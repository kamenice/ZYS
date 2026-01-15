/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * Description: Smart Conveyor Belt System - MQTT Utility Implementation
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

/* MQTT Configuration */
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
    unsigned char buf[200];
    int buflen = sizeof(buf);
    int len = 0;

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
    MQTTString topicFilter = MQTTString_initializer;
    topicFilter.cstring = MQTT_TOPIC_CONTROL;
    int req_qos = 0;
    int msgid = 1;
    
    len = MQTTSerialize_subscribe(buf, buflen, 0, msgid, 1, &topicFilter, &req_qos);
    transport_sendPacketBuffer(g_mqttSocket, buf, len);
    
    printf("[MQTT] Subscribed to %s\n", MQTT_TOPIC_CONTROL);

    return 0;
}

void mqtt_publish_status(void)
{
    if (g_mqttSocket < 0) return;

    unsigned char buf[256];
    int buflen = sizeof(buf);
    char payload[200];
    int len = 0;

    MQTTString topicString = MQTTString_initializer;
    topicString.cstring = MQTT_TOPIC_STATUS;

    /* Get sensor data */
    float weight = HX711_GetWeight();
    float temp = DHT11_GetTemperature();
    float humidity = DHT11_GetHumidity();
    int jam = IR_IsJamDetected();
    uint32_t rpm = Hall_GetRPM();
    int running = Motor_IsRunning();
    uint32_t runTime = Conveyor_GetRunTime();
    int overweight = HX711_IsOverweight();
    int overheat = DHT11_IsOverheating();

    /* Format JSON payload */
    snprintf(payload, sizeof(payload),
             "{\"weight\":%.1f,\"temp\":%.1f,\"humidity\":%.1f,"
             "\"jam\":%d,\"rpm\":%u,\"running\":%d,"
             "\"runTime\":%u,\"overweight\":%d,\"overheat\":%d}",
             weight, temp, humidity, jam, rpm, running,
             runTime, overweight, overheat);

    int payloadlen = strlen(payload);
    len = MQTTSerialize_publish(buf, buflen, 0, 0, 0, 0, topicString, 
                                 (unsigned char *)payload, payloadlen);
    transport_sendPacketBuffer(g_mqttSocket, buf, len);

    printf("[MQTT] Published: %s\n", payload);
}

void mqtt_publish_alert(const char *alertType)
{
    if (g_mqttSocket < 0) return;

    unsigned char buf[200];
    int buflen = sizeof(buf);
    char payload[100];
    int len = 0;

    MQTTString topicString = MQTTString_initializer;
    topicString.cstring = MQTT_TOPIC_ALERT;

    /* Format alert message */
    snprintf(payload, sizeof(payload), "{\"alert\":\"%s\"}", alertType);

    int payloadlen = strlen(payload);
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
    if (g_mqttSocket < 0) return;

    unsigned char buf[200];
    int buflen = sizeof(buf);

    /* Check for incoming messages (non-blocking) */
    int rc = MQTTPacket_read(buf, buflen, transport_getdata);
    
    if (rc == PUBLISH) {
        unsigned char dup, retained;
        unsigned short msgid;
        int payloadlen_in;
        unsigned char *payload_in;
        MQTTString receivedTopic;

        MQTTDeserialize_publish(&dup, &retained, &msgid, &receivedTopic,
                                &payload_in, &payloadlen_in, buf, buflen);

        /* Process received command */
        char cmd[100] = {0};
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
    attr.priority = 36;

    if (osThreadNew((osThreadFunc_t)mqtt_task, NULL, &attr) == NULL) {
        printf("[MQTT] Failed to create MQTT task!\n");
    }
}
