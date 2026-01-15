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
#include "cmsis_os2.h"
#include "wifi_device.h"
#include "lwip/netifapi.h"
#include "lwip/api_shell.h"
#include "MQTTPacket.h"
#include "transport.h"
#include "conveyor_system.h"
#include "wifi_mqtt.h"

/* WiFi Configuration */
#define WIFI_SSID           "zys"
#define WIFI_PASSWORD       "12345678"

/* MQTT Configuration */
#define MQTT_HOST           "192.168.43.230"
#define MQTT_PORT           1883
#define MQTT_TOPIC          "conveyor"
#define MQTT_CLIENT_ID      "hi3861-conveyor"

#define TASK_STACK_SIZE     4096

static int g_wifiConnected = 0;

static void ConnectWifi(void)
{
    WifiErrorCode errCode;
    WifiDeviceConfig apConfig = {};
    int netId = -1;

    /* Configure WiFi */
    strcpy(apConfig.ssid, WIFI_SSID);
    strcpy(apConfig.preSharedKey, WIFI_PASSWORD);
    apConfig.securityType = WIFI_SEC_TYPE_PSK;

    /* Enable WiFi */
    errCode = EnableWifi();
    printf("[WiFi] EnableWifi: %d\n", errCode);

    /* Clear old configurations */
    WifiDeviceConfig configs[WIFI_MAX_CONFIG_SIZE] = {0};
    unsigned int size = WIFI_MAX_CONFIG_SIZE;
    if (GetDeviceConfigs(configs, &size) == WIFI_SUCCESS) {
        for (unsigned int i = 0; i < size; i++) {
            RemoveDevice(configs[i].netId);
        }
    }

    /* Add new configuration */
    errCode = AddDeviceConfig(&apConfig, &netId);
    printf("[WiFi] AddDeviceConfig: %d, netId: %d\n", errCode, netId);

    /* Connect to WiFi */
    errCode = ConnectTo(netId);
    printf("[WiFi] ConnectTo(%d): %d\n", netId, errCode);
    sleep(3);

    /* Get IP address via DHCP */
    struct netif *iface = netifapi_netif_find("wlan0");
    if (iface) {
        err_t ret = netifapi_dhcp_start(iface);
        printf("[WiFi] netifapi_dhcp_start: %d\n", ret);
        sleep(5);
        ret = netifapi_netif_common(iface, dhcp_clients_info_show, NULL);
        printf("[WiFi] netifapi_netif_common: %d\n", ret);
        g_wifiConnected = 1;
    }
}

static void MqttPublishLoop(void)
{
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    int sock = 0;
    unsigned char buf[256];
    int buflen = sizeof(buf);
    MQTTString topicString = MQTTString_initializer;
    char payload[256];
    int len;
    ConveyorState *state = GetConveyorState();

    /* Setup MQTT connection parameters */
    topicString.cstring = MQTT_TOPIC;
    data.clientID.cstring = MQTT_CLIENT_ID;
    data.keepAliveInterval = 20;
    data.cleansession = 1;
    data.username.cstring = "conveyor_user";
    data.password.cstring = "conveyor_pass";

    /* Connect to MQTT broker */
    sock = transport_open((char *)MQTT_HOST, MQTT_PORT);
    if (sock < 0) {
        printf("[MQTT] Failed to connect to broker\n");
        return;
    }

    printf("[MQTT] Connecting to %s:%d\n", MQTT_HOST, MQTT_PORT);

    len = MQTTSerialize_connect(buf, buflen, &data);
    transport_sendPacketBuffer(sock, buf, len);

    if (MQTTPacket_read(buf, buflen, transport_getdata) == CONNACK) {
        unsigned char sessionPresent, connack_rc;
        if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, buf, buflen) != 1 || connack_rc != 0) {
            printf("[MQTT] Unable to connect, return code %d\n", connack_rc);
            goto mqtt_exit;
        }
    } else {
        printf("[MQTT] Failed to receive CONNACK\n");
        goto mqtt_exit;
    }

    printf("[MQTT] Connected, publishing data...\n");

    /* Publish data loop */
    while (1) {
        /* Create JSON payload */
        snprintf(payload, sizeof(payload),
                 "{\"item\":%d,\"temp\":%.1f,\"motor\":%d,\"alarm\":%d,\"servo\":%d}",
                 state->item_detected,
                 state->temperature,
                 state->motor_running,
                 state->overheat_alarm,
                 state->servo_activated);

        int payloadlen = strlen(payload);
        len = MQTTSerialize_publish(buf, buflen, 0, 0, 0, 0, topicString,
                                    (unsigned char *)payload, payloadlen);
        transport_sendPacketBuffer(sock, buf, len);

        printf("[MQTT] Published: %s\n", payload);
        sleep(2);
    }

mqtt_exit:
    transport_close(sock);
}

void WifiMqttInit(void)
{
    /* WiFi initialization is deferred to task */
    printf("[WiFi/MQTT] Module ready\n");
}

static void MqttTask(void *arg)
{
    (void)arg;

    while (1) {
        if (!g_wifiConnected) {
            ConnectWifi();
        }
        
        if (g_wifiConnected) {
            MqttPublishLoop();
        }
        
        /* If disconnected, wait and retry */
        g_wifiConnected = 0;
        sleep(5);
    }
}

void StartMqttTask(void)
{
    osThreadAttr_t attr = {0};
    attr.name = "MqttTask";
    attr.stack_size = TASK_STACK_SIZE;
    attr.priority = 36;

    if (osThreadNew(MqttTask, NULL, &attr) == NULL) {
        printf("[MQTT] Failed to create MqttTask\n");
    }
}
