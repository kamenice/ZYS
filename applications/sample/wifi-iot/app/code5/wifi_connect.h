/*
 * WiFi连接工具头文件
 * SSID: zys, Password: 12345678
 */

#ifndef WIFI_CONNECT_H
#define WIFI_CONNECT_H

/**
 * @brief 连接WiFi
 * WiFi名称: zys
 * WiFi密码: 12345678
 */
void WiFi_Connect(void);

/**
 * @brief 检查WiFi连接状态
 * @return 1: 已连接, 0: 未连接
 */
int WiFi_IsConnected(void);

#endif // WIFI_CONNECT_H
