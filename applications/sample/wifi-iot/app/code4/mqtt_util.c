/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * Description: Smart Conveyor Belt System - MQTT Utility Implementation
 * 
 * Note: MQTT server configuration uses PC IP when connected to zys WiFi.
 * For production deployment, consider using a configuration interface.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "hi_wifi_api.h"
#include "lwip/ip_addr.h"
#include "lwip/netifapi.h"
#include "lwip/sockets.h"
#include "MQTTPacket.h"
#include "transport.h"

#include "mqtt_util.h"
#include "wifi_util.h"
#include "hx711_sensor.h"
#include "dht11_sensor.h"
#include "infrared_sensor.h"
#include "hall_sensor.h"
#include "motor_driver.h"
#include "buzzer_control.h"
#include "conveyor_belt.h"

/* MQTT Configuration - PC IP when connected to zys WiFi */
#define MQTT_SERVER_HOST    "192.168.43.230"
#define MQTT_SERVER_PORT    1883
#define MQTT_CLIENT_ID      "hi3861-conveyor"
#define MQTT_USERNAME       "conveyor_user"
#define MQTT_PASSWORD       "conveyor_pass"

/* MQTT Topics */
#define MQTT_TOPIC_STATUS   "conveyor/status"
#define MQTT_TOPIC_ALERT    "conveyor/alert"
#define MQTT_TOPIC_CONTROL  "conveyor/control"

/* Global variable for MQTT socket (volatile for thread safety) */
static volatile int g_mqttSocket = -1;

int mqtt_connect(void)
{
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    MQTTString topicFilter = MQTTString_initializer;
    unsigned char buf[200];
    int buflen = sizeof(buf);
    int len = 0;
    int req_qos = 0;
    int msgid = 1;

    /* Configure connection parameters */
    data.clientID.cstring = MQTT_CLIENT_ID;
    data.keepAliveInterval = 60;
    data.cleansession = 1;
    data.username.cstring = MQTT_USERNAME;
    data.password.cstring = MQTT_PASSWORD;

    /* Connect to MQTT broker */
    g_mqttSocket = transport_open((char *)MQTT_SERVER_HOST, MQTT_SERVER_PORT);
    if (g_mqttSocket < 0) {
        printf("[MQTT] Failed to connect to %s:%d\n", MQTT_SERVER_HOST, MQTT_SERVER_PORT);
        return -1;
    }

    printf("[MQTT] Connecting to %s:%d\n", MQTT_SERVER_HOST, MQTT_SERVER_PORT);

    len = MQTTSerialize_connect(buf, buflen, &data);
    transport_sendPacketBuffer(g_mqttSocket, buf, len);

    if (MQTTPacket_read(buf, buflen, transport_getdata) == CONNACK)
    {
        unsigned char sessionPresent, connack_rc;

        if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, buf, buflen) != 1 || connack_rc != 0)
        {
            printf("[MQTT] Connection failed, return code %d\n", connack_rc);
            transport_close(g_mqttSocket);
            g_mqttSocket = -1;
            return -1;
        }
        printf("[MQTT] Connected successfully\n");
    }
    else
    {
        printf("[MQTT] CONNACK not received\n");
        transport_close(g_mqttSocket);
        g_mqttSocket = -1;
        return -1;
    }

    /* Subscribe to control topic */
    topicFilter.cstring = MQTT_TOPIC_CONTROL;
    
    len = MQTTSerialize_subscribe(buf, buflen, 0, msgid, 1, &topicFilter, &req_qos);
    transport_sendPacketBuffer(g_mqttSocket, buf, len);
    
    printf("[MQTT] Subscribed to %s\n", MQTT_TOPIC_CONTROL);

    return 0;
}

void mqtt_publish_status(void)
{
    MQTTString topicString = MQTTString_initializer;
    unsigned char buf[256];
    int buflen = sizeof(buf);
    char payload[200];
    int len = 0;
    int payloadlen = 0;
    float weight = 0;
    float temp = 0;
    float humidity = 0;
    int jam = 0;
    unsigned int rpm = 0;
    int running = 0;
    unsigned int runTime = 0;
    int overweight = 0;
    int overheat = 0;

    if (g_mqttSocket < 0) return;

    topicString.cstring = MQTT_TOPIC_STATUS;

    /* Get sensor data */
    weight = HX711_GetWeight();
    temp = DHT11_GetTemperature();
    humidity = DHT11_GetHumidity();
    jam = IR_IsJamDetected();
    rpm = (unsigned int)Hall_GetRPM();
    running = Motor_IsRunning();
    runTime = (unsigned int)Conveyor_GetRunTime();
    overweight = HX711_IsOverweight();
    overheat = DHT11_IsOverheating();

    /* Format JSON payload */
    snprintf(payload, sizeof(payload),
             "{\"weight\":%.1f,\"temp\":%.1f,\"humidity\":%.1f,"
             "\"jam\":%d,\"rpm\":%u,\"running\":%d,"
             "\"runTime\":%u,\"overweight\":%d,\"overheat\":%d}",
             weight, temp, humidity, jam, rpm, running,
             runTime, overweight, overheat);

    payloadlen = strlen(payload);
    len = MQTTSerialize_publish(buf, buflen, 0, 0, 0, 0, topicString, 
                                 (unsigned char *)payload, payloadlen);
    transport_sendPacketBuffer(g_mqttSocket, buf, len);

    printf("[MQTT] Published: %s\n", payload);
}

void mqtt_publish_alert(const char *alertType)
{
    MQTTString topicString = MQTTString_initializer;
    unsigned char buf[200];
    int buflen = sizeof(buf);
    char payload[100];
    int len = 0;
    int payloadlen = 0;

    if (g_mqttSocket < 0) return;

    topicString.cstring = MQTT_TOPIC_ALERT;

    /* Format alert message */
    snprintf(payload, sizeof(payload), "{\"alert\":\"%s\"}", alertType);

    payloadlen = strlen(payload);
    len = MQTTSerialize_publish(buf, buflen, 0, 0, 0, 0, topicString,
                                 (unsigned char *)payload, payloadlen);
    transport_sendPacketBuffer(g_mqttSocket, buf, len);

    printf("[MQTT] Alert: %s\n", payload);
}

static void mqtt_process_command(const char *cmd)
{
    printf("[MQTT] Received command: %s\n", cmd);

    if (strstr(cmd, "start") != NULL) {
        Conveyor_Start();
    } else if (strstr(cmd, "stop") != NULL) {
        Conveyor_Stop();
    } else if (strstr(cmd, "mute") != NULL) {
        Buzzer_Mute();
    } else if (strstr(cmd, "unmute") != NULL) {
        Buzzer_Unmute();
    } else if (strstr(cmd, "vibrate") != NULL) {
        Conveyor_ManualVibrate();
    }
}

static void mqtt_receive_loop(void)
{
    unsigned char buf[200];
    int buflen = sizeof(buf);
    int rc = 0;
    unsigned char dup = 0;
    unsigned char retained = 0;
    unsigned char *payload_in = NULL;
    unsigned short packetid = 0;
    int qos = 0;
    int payloadlen_in = 0;
    MQTTString receivedTopic = MQTTString_initializer;
    char cmd[100] = {0};

    if (g_mqttSocket < 0) return;

    /* Check for incoming messages (non-blocking) */
    rc = MQTTPacket_read(buf, buflen, transport_getdata);
    
    if (rc == PUBLISH) {
        MQTTDeserialize_publish(&dup, &qos, &retained, &packetid, &receivedTopic,
                                &payload_in, &payloadlen_in, buf, buflen);

        /* Suppress unused variable warnings */
        (void)dup;
        (void)qos;
        (void)retained;
        (void)packetid;
        (void)receivedTopic;

        /* Process received command */
        if (payloadlen_in < (int)sizeof(cmd) - 1) {
            memcpy(cmd, payload_in, payloadlen_in);
            mqtt_process_command(cmd);
        }
    }
}

static void mqtt_task(void *arg)
{
    (void)arg;

    /* Wait for system to initialize */
    sleep(5);

    while (1) {
        /* Connect to WiFi if not connected */
        connect_wifi();
        sleep(3);

        /* Connect to MQTT broker */
        if (mqtt_connect() == 0) {
            /* Main MQTT loop */
            while (g_mqttSocket >= 0) {
                /* Publish status every 2 seconds */
                mqtt_publish_status();
                
                /* Check for incoming commands */
                mqtt_receive_loop();
                
                sleep(2);
            }
        }

        /* Retry after failure */
        sleep(5);
    }
}

void mqtt_start_task(void)
{
    osThreadAttr_t attr;
    attr.name = "mqtt_task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = osPriorityBelowNormal;

    if (osThreadNew((osThreadFunc_t)mqtt_task, NULL, &attr) == NULL) {
        printf("[MQTT] Failed to create MQTT task!\n");
    }
}
