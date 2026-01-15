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

#include "mqtt_client.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "lwip/ip_addr.h"
#include "lwip/netifapi.h"
#include "lwip/sockets.h"
#include "MQTTPacket.h"
#include "transport.h"

/* MQTT Broker Configuration
 * Server: 192.168.43.230 (PC connected to WiFi)
 * Port: 1883
 */
#define MQTT_HOST       "192.168.43.230"
#define MQTT_PORT       1883
#define MQTT_CLIENT_ID  "hi3861_conveyor"
#define MQTT_TOPIC      "conveyor"
#define MQTT_USERNAME   "hi3861"
#define MQTT_PASSWORD   "password"

static int g_mqttSocket = -1;

int MQTT_Connect(void)
{
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    unsigned char buf[200];
    int buflen = sizeof(buf);
    int len = 0;

    printf("[MQTT] Connecting to %s:%d...\n", MQTT_HOST, MQTT_PORT);

    /* Connect to MQTT broker */
    g_mqttSocket = transport_open((char *)MQTT_HOST, MQTT_PORT);
    if (g_mqttSocket < 0) {
        printf("[MQTT] Failed to connect to broker\n");
        return g_mqttSocket;
    }

    printf("[MQTT] Socket connected, sending CONNECT packet\n");

    /* Configure MQTT connection parameters */
    data.clientID.cstring = MQTT_CLIENT_ID;
    data.keepAliveInterval = 60;
    data.cleansession = 1;
    data.username.cstring = MQTT_USERNAME;
    data.password.cstring = MQTT_PASSWORD;

    len = MQTTSerialize_connect(buf, buflen, &data);
    transport_sendPacketBuffer(g_mqttSocket, buf, len);

    /* Wait for CONNACK */
    if (MQTTPacket_read(buf, buflen, transport_getdata) == CONNACK) {
        unsigned char sessionPresent, connack_rc;

        if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, buf, buflen) != 1 || connack_rc != 0) {
            printf("[MQTT] Connection rejected, return code: %d\n", connack_rc);
            transport_close(g_mqttSocket);
            g_mqttSocket = -1;
            return -1;
        }
        printf("[MQTT] Connected successfully\n");
    } else {
        printf("[MQTT] Failed to receive CONNACK\n");
        transport_close(g_mqttSocket);
        g_mqttSocket = -1;
        return -1;
    }

    return g_mqttSocket;
}

void MQTT_PublishStatus(int hasItem, float temperature, int motorState)
{
    unsigned char buf[200];
    int buflen = sizeof(buf);
    char payload[200];
    int payloadlen;
    int len;
    MQTTString topicString = MQTTString_initializer;

    if (g_mqttSocket < 0) {
        printf("[MQTT] Not connected\n");
        return;
    }

    topicString.cstring = MQTT_TOPIC;

    /* Format JSON payload */
    snprintf(payload, sizeof(payload),
             "{\"hasItem\":%d,\"temperature\":%.1f,\"motorState\":%d}",
             hasItem, temperature, motorState);
    payloadlen = strlen(payload);

    len = MQTTSerialize_publish(buf, buflen, 0, 0, 0, 0, topicString,
                                (unsigned char *)payload, payloadlen);
    transport_sendPacketBuffer(g_mqttSocket, buf, len);

    printf("[MQTT] Published: %s\n", payload);
}

void MQTT_Disconnect(void)
{
    unsigned char buf[50];
    int len;

    if (g_mqttSocket >= 0) {
        len = MQTTSerialize_disconnect(buf, sizeof(buf));
        transport_sendPacketBuffer(g_mqttSocket, buf, len);
        transport_close(g_mqttSocket);
        g_mqttSocket = -1;
        printf("[MQTT] Disconnected\n");
    }
}
