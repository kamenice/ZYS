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
#include "wifi_util.h"

void connect_wifi(void)
{
    // 1. Define WiFi variables
    WifiErrorCode errCode;
    WifiDeviceConfig apConfig = {};
    int netId = -1;

    // 2. Configure WiFi parameters
    // WiFi SSID: zys, Password: 12345678
    strcpy(apConfig.ssid, "zys");
    strcpy(apConfig.preSharedKey, "12345678");
    apConfig.securityType = WIFI_SEC_TYPE_PSK;

    // 3. Enable WiFi
    errCode = EnableWifi();
    printf("EnableWifi: %d\r\n", errCode);

    errCode = AddDeviceConfig(&apConfig, &netId);
    printf("AddDeviceConfig: %d, netId: %d\r\n", errCode, netId);

    // 4. Connect to WiFi
    errCode = ConnectTo(netId);
    printf("ConnectTo(%d): %d\r\n", netId, errCode);
    usleep(1000 * 1000);

    // 5. Get IP address via DHCP
    struct netif *iface = netifapi_netif_find("wlan0");
    if (iface) {
        // Start DHCP
        err_t ret = netifapi_dhcp_start(iface);
        printf("netifapi_dhcp_start: %d\r\n", ret);

        sleep(5);
        // Print IP address
        ret = netifapi_netif_common(iface, dhcp_clients_info_show, NULL);
        printf("netifapi_netif_common: %d\r\n", ret);
    }

    printf("[WiFi] Connected to SSID: zys\n");
}
