/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "wifi_device.h"
#include "lwip/netifapi.h"
#include "lwip/api_shell.h"
#include "lwip/ip_addr.h"
#include "lwip/sockets.h"
#include "MQTTPacket.h"
#include "transport.h"
#include "wifi_mqtt.h"

/* WiFi connection state */
static int g_wifiConnected = 0;
static int g_mqttSocket = -1;

/* Configurable thresholds */
static double g_weightThreshold = 5000.0;   /* Default 5kg */
static float g_tempThreshold = 60.0f;       /* Default 60°C */

/* External callback declarations */
extern void ConveyorSystem_Start(void);
extern void ConveyorSystem_Stop(void);
extern void ConveyorSystem_MuteAlarm(void);
extern void ConveyorSystem_UnmuteAlarm(void);
extern void ConveyorSystem_ClearJam(void);
extern void ConveyorSystem_SetWeightThreshold(double threshold);
extern void ConveyorSystem_SetTempThreshold(float threshold);

int WiFi_Init(void)
{
    WifiErrorCode errCode;

    errCode = EnableWifi();
    if (errCode != WIFI_SUCCESS) {
        printf("[WiFi] EnableWifi failed: %d\r\n", errCode);
        return -1;
    }

    printf("[WiFi] Initialized\r\n");
    return 0;
}

int WiFi_Connect(void)
{
    WifiErrorCode errCode;
    WifiDeviceConfig apConfig = {0};
    int netId = -1;

    printf("[WiFi] Connecting to %s...\r\n", WIFI_SSID);

    /* Configure WiFi connection */
    strcpy(apConfig.ssid, WIFI_SSID);
    strcpy(apConfig.preSharedKey, WIFI_PASSWORD);
    apConfig.securityType = WIFI_SEC_TYPE_PSK;

    /* Clear existing configurations */
    WifiDeviceConfig configs[WIFI_MAX_CONFIG_SIZE] = {0};
    unsigned int size = WIFI_MAX_CONFIG_SIZE;
    if (GetDeviceConfigs(configs, &size) == WIFI_SUCCESS) {
        for (unsigned int i = 0; i < size; i++) {
            RemoveDevice(configs[i].netId);
        }
    }

    errCode = AddDeviceConfig(&apConfig, &netId);
    if (errCode != WIFI_SUCCESS) {
        printf("[WiFi] AddDeviceConfig failed: %d\r\n", errCode);
        return -1;
    }
    printf("[WiFi] AddDeviceConfig: %d, netId: %d\r\n", errCode, netId);

    errCode = ConnectTo(netId);
    if (errCode != WIFI_SUCCESS) {
        printf("[WiFi] ConnectTo failed: %d\r\n", errCode);
        return -1;
    }
    printf("[WiFi] ConnectTo(%d): %d\r\n", netId, errCode);

    /* Wait for WiFi connection (following reference code pattern) */
    sleep(3);

    /* Get IP address via DHCP */
    struct netif *iface = netifapi_netif_find("wlan0");
    if (iface) {
        err_t ret = netifapi_dhcp_start(iface);
        printf("[WiFi] netifapi_dhcp_start: %d\r\n", ret);

        /* Wait for DHCP (following reference code pattern) */
        sleep(5);

        /* Print IP address */
        ret = netifapi_netif_common(iface, dhcp_clients_info_show, NULL);
        printf("[WiFi] netifapi_netif_common: %d\r\n", ret);
    }

    g_wifiConnected = 1;
    printf("[WiFi] Connected to %s\r\n", WIFI_SSID);
    return 0;
}

int WiFi_IsConnected(void)
{
    return g_wifiConnected;
}

int MQTT_Init(void)
{
    /* MQTT initialization happens during connect */
    return 0;
}

int MQTT_Connect(void)
{
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    unsigned char buf[256];
    int buflen = sizeof(buf);
    int len;

    /* Connect to MQTT broker */
    g_mqttSocket = transport_open((char *)MQTT_HOST, MQTT_PORT);
    if (g_mqttSocket < 0) {
        printf("[MQTT] Failed to connect to %s:%d\r\n", MQTT_HOST, MQTT_PORT);
        return -1;
    }

    /* Configure connection parameters */
    data.clientID.cstring = (char *)MQTT_CLIENT_ID;
    data.keepAliveInterval = 60;
    data.cleansession = 1;
    data.username.cstring = (char *)MQTT_USERNAME;
    data.password.cstring = (char *)MQTT_PASSWORD;

    /* Send CONNECT packet */
    len = MQTTSerialize_connect(buf, buflen, &data);
    transport_sendPacketBuffer(g_mqttSocket, buf, len);

    /* Wait for CONNACK with timeout */
    int packetType = MQTTPacket_read(buf, buflen, transport_getdata);
    if (packetType == CONNACK) {
        unsigned char sessionPresent, connack_rc;
        if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, buf, buflen) != 1 ||
            connack_rc != 0) {
            printf("[MQTT] Connection refused: %d\r\n", connack_rc);
            transport_close(g_mqttSocket);
            g_mqttSocket = -1;
            return -1;
        }
    } else if (packetType < 0) {
        printf("[MQTT] Connection timeout or error\r\n");
        transport_close(g_mqttSocket);
        g_mqttSocket = -1;
        return -1;
    } else {
        printf("[MQTT] Unexpected packet type: %d\r\n", packetType);
        transport_close(g_mqttSocket);
        g_mqttSocket = -1;
        return -1;
    }

    printf("[MQTT] Connected to broker\r\n");
    return 0;
}

void MQTT_PublishStatus(double weight, float temperature,
                        float speedRPM, int isRunning,
                        uint32_t runTimeSeconds)
{
    if (g_mqttSocket < 0) {
        return;
    }

    unsigned char buf[256];
    int buflen = sizeof(buf);
    char payload[200];
    int payloadlen;
    int len;
    MQTTString topicString = MQTTString_initializer;

    topicString.cstring = (char *)MQTT_TOPIC_STATUS;

    /* Build JSON payload */
    snprintf(payload, sizeof(payload),
             "{\"weight\":%.1f,\"temp\":%.1f,\"speed\":%.1f,\"running\":%d,\"time\":%u}",
             weight, temperature, speedRPM, isRunning, runTimeSeconds);
    payloadlen = strlen(payload);

    /* Publish message */
    len = MQTTSerialize_publish(buf, buflen, 0, 0, 0, 0,
                                topicString, (unsigned char *)payload, payloadlen);
    transport_sendPacketBuffer(g_mqttSocket, buf, len);
}

void MQTT_PublishAlert(const char *alertType, float value)
{
    if (g_mqttSocket < 0) {
        return;
    }

    unsigned char buf[256];
    int buflen = sizeof(buf);
    char payload[100];
    int payloadlen;
    int len;
    MQTTString topicString = MQTTString_initializer;

    topicString.cstring = (char *)MQTT_TOPIC_ALERT;

    /* Build JSON alert payload */
    snprintf(payload, sizeof(payload),
             "{\"alert\":\"%s\",\"value\":%.1f}", alertType, value);
    payloadlen = strlen(payload);

    /* Publish alert */
    len = MQTTSerialize_publish(buf, buflen, 0, 0, 0, 0,
                                topicString, (unsigned char *)payload, payloadlen);
    transport_sendPacketBuffer(g_mqttSocket, buf, len);

    printf("[MQTT] Alert sent: %s (%.1f)\r\n", alertType, value);
}

int MQTT_SubscribeControl(void)
{
    if (g_mqttSocket < 0) {
        return -1;
    }

    unsigned char buf[256];
    int buflen = sizeof(buf);
    int len;
    MQTTString topicString = MQTTString_initializer;
    int msgid = 1;
    int qos = 0;

    topicString.cstring = (char *)MQTT_TOPIC_CONTROL;

    len = MQTTSerialize_subscribe(buf, buflen, 0, msgid, 1, &topicString, &qos);
    transport_sendPacketBuffer(g_mqttSocket, buf, len);

    printf("[MQTT] Subscribed to %s\r\n", MQTT_TOPIC_CONTROL);
    return 0;
}

void MQTT_ProcessMessages(void)
{
    if (g_mqttSocket < 0) {
        return;
    }

    unsigned char buf[256];
    int buflen = sizeof(buf);
    int rc;
    MQTTString topicName;
    unsigned char *payload;
    int payloadlen;
    unsigned char dup;
    int qos;
    unsigned char retained;
    unsigned short msgid;

    /* Check for incoming messages (transport_getdata has 1s timeout) */
    rc = MQTTPacket_read(buf, buflen, transport_getdata);

    /* Handle error or timeout cases */
    if (rc <= 0) {
        /* Timeout or error - no data available, return immediately */
        return;
    }

    if (rc == PUBLISH) {
        MQTTDeserialize_publish(&dup, &qos, &retained, &msgid,
                               &topicName, &payload, &payloadlen, buf, buflen);

        /* Process control commands */
        if (strncmp((char *)payload, CMD_START, strlen(CMD_START)) == 0) {
            ConveyorSystem_Start();
        } else if (strncmp((char *)payload, CMD_STOP, strlen(CMD_STOP)) == 0) {
            ConveyorSystem_Stop();
        } else if (strncmp((char *)payload, CMD_MUTE, strlen(CMD_MUTE)) == 0) {
            ConveyorSystem_MuteAlarm();
        } else if (strncmp((char *)payload, CMD_UNMUTE, strlen(CMD_UNMUTE)) == 0) {
            ConveyorSystem_UnmuteAlarm();
        } else if (strncmp((char *)payload, CMD_CLEAR_JAM, strlen(CMD_CLEAR_JAM)) == 0) {
            ConveyorSystem_ClearJam();
        } else if (strncmp((char *)payload, CMD_SET_WEIGHT_THRESHOLD,
                          strlen(CMD_SET_WEIGHT_THRESHOLD)) == 0) {
            /* Parse threshold value from command */
            double threshold;
            if (sscanf((char *)payload + strlen(CMD_SET_WEIGHT_THRESHOLD) + 1,
                       "%lf", &threshold) == 1) {
                ConveyorSystem_SetWeightThreshold(threshold);
            }
        } else if (strncmp((char *)payload, CMD_SET_TEMP_THRESHOLD,
                          strlen(CMD_SET_TEMP_THRESHOLD)) == 0) {
            /* Parse threshold value from command */
            float threshold;
            if (sscanf((char *)payload + strlen(CMD_SET_TEMP_THRESHOLD) + 1,
                       "%f", &threshold) == 1) {
                ConveyorSystem_SetTempThreshold(threshold);
            }
        }
    }
}

void MQTT_Disconnect(void)
{
    if (g_mqttSocket >= 0) {
        unsigned char buf[16];
        int len = MQTTSerialize_disconnect(buf, sizeof(buf));
        transport_sendPacketBuffer(g_mqttSocket, buf, len);
        transport_close(g_mqttSocket);
        g_mqttSocket = -1;
    }
    printf("[MQTT] Disconnected\r\n");
}

void MQTT_SetWeightThreshold(double threshold)
{
    g_weightThreshold = threshold;
    printf("[MQTT] Weight threshold set to %.1fg\r\n", threshold);
}

void MQTT_SetTempThreshold(float threshold)
{
    g_tempThreshold = threshold;
    printf("[MQTT] Temperature threshold set to %.1fC\r\n", threshold);
}

double MQTT_GetWeightThreshold(void)
{
    return g_weightThreshold;
}

float MQTT_GetTempThreshold(void)
{
    return g_tempThreshold;
}
