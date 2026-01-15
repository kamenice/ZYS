/*
 * wifi_util.c - WiFi连接工具
 * 
 * WiFi名称：zys
 * WiFi密码：12345678
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "wifi_device.h"
#include "lwip/netifapi.h"
#include "lwip/api_shell.h"

void connect_wifi(void)
{
    // 1.定义WiFi相关变量
    WifiErrorCode errCode;
    WifiDeviceConfig apConfig = {};
    int netId = -1;

    // 2.配置WiFi参数
    strcpy(apConfig.ssid, "zys");           // WiFi名称：zys
    strcpy(apConfig.preSharedKey, "12345678"); // WiFi密码：12345678
    apConfig.securityType = WIFI_SEC_TYPE_PSK;  // 加密方式：PSK

    // 3.启用WiFi
    errCode = EnableWifi();
    printf("[WiFi] EnableWifi: %d\r\n", errCode);
    
    // 清除旧配置，防止冲突
    WifiDeviceConfig configs[WIFI_MAX_CONFIG_SIZE] = {0};
    unsigned int size = WIFI_MAX_CONFIG_SIZE;
    if (GetDeviceConfigs(configs, &size) == WIFI_SUCCESS) {
        for (unsigned int i = 0; i < size; i++) {
            RemoveDevice(configs[i].netId);
        }
    }

    errCode = AddDeviceConfig(&apConfig, &netId);
    printf("[WiFi] AddDeviceConfig: %d, netId: %d\r\n", errCode, netId);

    // 4.连接WiFi
    errCode = ConnectTo(netId);
    printf("[WiFi] ConnectTo(%d): %d\r\n", netId, errCode);
    sleep(3);

    // 5.获取IP地址
    struct netif *iface = netifapi_netif_find("wlan0");
    if (iface)
    {
        // 启动DHCP
        err_t ret = netifapi_dhcp_start(iface);
        printf("[WiFi] netifapi_dhcp_start: %d\r\n", ret);

        sleep(5);
        // 打印IP地址
        ret = netifapi_netif_common(iface, dhcp_clients_info_show, NULL);
        printf("[WiFi] netifapi_netif_common: %d\r\n", ret);
    }
}
