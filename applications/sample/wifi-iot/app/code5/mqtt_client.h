/*
 * MQTT通信工具头文件
 * 服务器IP: 192.168.43.230
 */

#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

/**
 * @brief 连接MQTT服务器
 * 服务器地址: 192.168.43.230
 * 端口: 1883
 * @return 0: 成功, 其他: 失败
 */
int MQTT_Connect(void);

/**
 * @brief 发布传送带系统数据
 * @param hasObject 是否有物品
 * @param temperature 当前温度
 * @param motorRunning 电机状态
 * @param infraredDetected 红外检测状态
 */
void MQTT_PublishData(int hasObject, float temperature, int motorRunning, int infraredDetected);

/**
 * @brief 断开MQTT连接
 */
void MQTT_Disconnect(void);

#endif // MQTT_CLIENT_H
