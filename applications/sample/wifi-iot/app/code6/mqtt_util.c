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

#include "hi_wifi_api.h"
#include "lwip/ip_addr.h"
#include "lwip/netifapi.h"
#include "lwip/sockets.h"
#include "MQTTPacket.h"
#include "transport.h"
#include "mqtt_util.h"
#include "conveyor_data.h"

int mqtt_connect(void)
{
    // 1. Initialize variables
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    int mysock = 0;
    unsigned char buf[200];
    int buflen = sizeof(buf);

    MQTTString topicString = MQTTString_initializer;
    char payload[200];
    int payloadlen = 0;
    int len = 0;

    // 2. Configure connection parameters
    // MQTT Server: 192.168.43.230 (PC connected to same WiFi)
    char *host = "192.168.43.230";
    int port = 1883;
    topicString.cstring = "conveyor_belt";

    data.clientID.cstring = "hi3861_conveyor";
    data.keepAliveInterval = 20;
    data.cleansession = 1;
    data.username.cstring = "hi3861_conveyor";
    data.password.cstring = "conveyor123";

    // 3. Connect to MQTT server
    mysock = transport_open(host, port);
    if (mysock < 0) {
        printf("[MQTT] Failed to connect to server\n");
        return mysock;
    }

    printf("[MQTT] Sending to hostname %s port %d\n", host, port);

    len = MQTTSerialize_connect(buf, buflen, &data);
    transport_sendPacketBuffer(mysock, buf, len);

    if (MQTTPacket_read(buf, buflen, transport_getdata) == CONNACK) {
        unsigned char sessionPresent, connack_rc;

        if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, buf, buflen) != 1 || connack_rc != 0) {
            printf("[MQTT] Unable to connect, return code %d\n", connack_rc);
            goto exit;
        }
    } else {
        printf("[MQTT] Failed to receive CONNACK\n");
        goto exit;
    }

    printf("[MQTT] Connected successfully\n");

    // 4. Publish data in a loop
    while (1) {
        // Get current conveyor belt status
        ConveyorData *conveyor = GetConveyorData();

        // Format data as JSON string for mobile app
        snprintf(payload, sizeof(payload),
                 "{\"object_detected\":%d,\"temperature\":%.1f,\"humidity\":%.1f,\"motor_running\":%d,\"infrared_detected\":%d,\"over_temp\":%d}",
                 conveyor->object_detected,
                 conveyor->temperature,
                 conveyor->humidity,
                 conveyor->motor_running,
                 conveyor->infrared_detected,
                 conveyor->over_temperature);

        payloadlen = strlen(payload);
        len = MQTTSerialize_publish(buf, buflen, 0, 0, 0, 0, topicString, (unsigned char *)payload, payloadlen);
        transport_sendPacketBuffer(mysock, buf, len);

        printf("[MQTT] Published: %s\n", payload);

        sleep(1); // Publish every 1 second
    }

exit:
    transport_close(mysock);
    return 0;
}
