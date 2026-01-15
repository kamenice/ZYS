/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * Description: Smart Conveyor Belt System - WiFi Utility Implementation
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "wifi_device.h"
#include "lwip/netifapi.h"
#include "lwip/api_shell.h"

/* WiFi Configuration */
#define WIFI_SSID       "zys"
#define WIFI_PASSWORD   "12345678"

void connect_wifi(void)
{
    WifiErrorCode errCode;
    WifiDeviceConfig apConfig = {};
    int netId = -1;

    /* Configure WiFi parameters */
    strcpy(apConfig.ssid, WIFI_SSID);
    strcpy(apConfig.preSharedKey, WIFI_PASSWORD);
    apConfig.securityType = WIFI_SEC_TYPE_PSK;

    /* Enable WiFi */
    errCode = EnableWifi();
    printf("[WiFi] EnableWifi: %d\r\n", errCode);
    
    /* Clear existing configurations */
    WifiDeviceConfig configs[WIFI_MAX_CONFIG_SIZE] = {0};
    unsigned int size = WIFI_MAX_CONFIG_SIZE;
    if (GetDeviceConfigs(configs, &size) == WIFI_SUCCESS) {
        for (unsigned int i = 0; i < size; i++) {
            RemoveDevice(configs[i].netId);
        }
    }

    errCode = AddDeviceConfig(&apConfig, &netId);
    printf("[WiFi] AddDeviceConfig: %d, netId: %d\r\n", errCode, netId);

    /* Connect to WiFi */
    errCode = ConnectTo(netId);
    printf("[WiFi] ConnectTo(%d): %d\r\n", netId, errCode);
    sleep(3);

    /* Get IP address via DHCP */
    struct netif *iface = netifapi_netif_find("wlan0");
    if (iface)
    {
        err_t ret = netifapi_dhcp_start(iface);
        printf("[WiFi] netifapi_dhcp_start: %d\r\n", ret);

        sleep(5);
        ret = netifapi_netif_common(iface, dhcp_clients_info_show, NULL);
        printf("[WiFi] netifapi_netif_common: %d\r\n", ret);
    }
}
