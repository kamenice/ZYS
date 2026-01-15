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

#include "wifi_connect.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "wifi_device.h"
#include "lwip/netifapi.h"
#include "lwip/api_shell.h"

/* WiFi Configuration
 * SSID: zys
 * Password: 12345678
 */
#define WIFI_SSID       "zys"
#define WIFI_PASSWORD   "12345678"

static int g_wifiConnected = 0;

void WiFi_Connect(void)
{
    WifiErrorCode errCode;
    WifiDeviceConfig apConfig = {};
    int netId = -1;

    /* Configure WiFi parameters */
    strcpy(apConfig.ssid, WIFI_SSID);
    strcpy(apConfig.preSharedKey, WIFI_PASSWORD);
    apConfig.securityType = WIFI_SEC_TYPE_PSK;

    printf("[WiFi] Connecting to %s...\n", WIFI_SSID);

    /* Enable WiFi */
    errCode = EnableWifi();
    if (errCode != WIFI_SUCCESS) {
        printf("[WiFi] EnableWifi failed: %d\n", errCode);
        return;
    }

    errCode = AddDeviceConfig(&apConfig, &netId);
    if (errCode != WIFI_SUCCESS) {
        printf("[WiFi] AddDeviceConfig failed: %d\n", errCode);
        return;
    }

    /* Connect to WiFi */
    errCode = ConnectTo(netId);
    printf("[WiFi] ConnectTo(%d): %d\n", netId, errCode);
    usleep(1000 * 1000);

    /* Get IP address via DHCP */
    struct netif *iface = netifapi_netif_find("wlan0");
    if (iface) {
        err_t ret = netifapi_dhcp_start(iface);
        printf("[WiFi] DHCP start: %d\n", ret);

        sleep(5);

        ret = netifapi_netif_common(iface, dhcp_clients_info_show, NULL);
        printf("[WiFi] DHCP info: %d\n", ret);

        g_wifiConnected = 1;
        printf("[WiFi] Connected successfully\n");
    } else {
        printf("[WiFi] Failed to find wlan0 interface\n");
    }
}

int WiFi_IsConnected(void)
{
    return g_wifiConnected;
}
