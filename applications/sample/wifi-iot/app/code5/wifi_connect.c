/*
 * WiFi连接工具实现
 * SSID: zys, Password: 12345678
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "wifi_connect.h"
#include "wifi_device.h"
#include "lwip/netifapi.h"
#include "lwip/api_shell.h"

// WiFi连接状态
static int g_wifiConnected = 0;

/**
 * @brief 连接WiFi
 * WiFi名称: zys
 * WiFi密码: 12345678
 */
void WiFi_Connect(void)
{
    WifiErrorCode errCode;
    WifiDeviceConfig apConfig = {};
    int netId = -1;

    // 配置WiFi参数
    strcpy(apConfig.ssid, "zys");              // WiFi名称
    strcpy(apConfig.preSharedKey, "12345678"); // WiFi密码
    apConfig.securityType = WIFI_SEC_TYPE_PSK; // 加密方式

    // 启动WiFi
    errCode = EnableWifi();
    printf("[WiFi] EnableWifi: %d\n", errCode);
    
    errCode = AddDeviceConfig(&apConfig, &netId);
    printf("[WiFi] AddDeviceConfig: %d, netId: %d\n", errCode, netId);

    // 连接WiFi
    errCode = ConnectTo(netId);
    printf("[WiFi] ConnectTo(%d): %d\n", netId, errCode);
    usleep(1000 * 1000);

    // 获取IP地址
    struct netif *iface = netifapi_netif_find("wlan0");
    if (iface) {
        // 启动DHCP
        err_t ret = netifapi_dhcp_start(iface);
        printf("[WiFi] netifapi_dhcp_start: %d\n", ret);

        sleep(5);  // 等待获取IP地址
        
        // 打印IP地址
        ret = netifapi_netif_common(iface, dhcp_clients_info_show, NULL);
        printf("[WiFi] netifapi_netif_common: %d\n", ret);
        
        g_wifiConnected = 1;
        printf("[WiFi] Connected to: zys\n");
    } else {
        printf("[WiFi] Failed to find wlan0 interface\n");
        g_wifiConnected = 0;
    }
}

/**
 * @brief 检查WiFi连接状态
 * @return 1: 已连接, 0: 未连接
 */
int WiFi_IsConnected(void)
{
    return g_wifiConnected;
}
