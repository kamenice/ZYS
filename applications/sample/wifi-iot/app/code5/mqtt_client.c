/*
 * MQTT通信工具实现
 * 服务器IP: 192.168.43.230
 * 端口: 1883
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "mqtt_client.h"
#include "hi_wifi_api.h"
#include "lwip/ip_addr.h"
#include "lwip/netifapi.h"
#include "lwip/sockets.h"
#include "MQTTPacket.h"
#include "transport.h"

// MQTT服务器配置
#define MQTT_SERVER_HOST    "192.168.43.230"
#define MQTT_SERVER_PORT    1883
#define MQTT_TOPIC          "conveyor"
#define MQTT_CLIENT_ID      "hi3861_conveyor"
#define MQTT_USERNAME       "hi3861"
#define MQTT_PASSWORD       "password"

// MQTT连接socket
static int g_mqttSocket = -1;

/**
 * @brief 连接MQTT服务器
 * @return 0: 成功, 其他: 失败
 */
int MQTT_Connect(void)
{
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    unsigned char buf[200];
    int buflen = sizeof(buf);
    int len = 0;

    // 配置连接参数
    data.clientID.cstring = MQTT_CLIENT_ID;
    data.keepAliveInterval = 20;
    data.cleansession = 1;
    data.username.cstring = MQTT_USERNAME;
    data.password.cstring = MQTT_PASSWORD;

    // 连接服务器
    g_mqttSocket = transport_open((char *)MQTT_SERVER_HOST, MQTT_SERVER_PORT);
    if (g_mqttSocket < 0) {
        printf("[MQTT] Failed to connect to server %s:%d\n", MQTT_SERVER_HOST, MQTT_SERVER_PORT);
        return -1;
    }

    printf("[MQTT] Connecting to %s:%d\n", MQTT_SERVER_HOST, MQTT_SERVER_PORT);

    len = MQTTSerialize_connect(buf, buflen, &data);
    transport_sendPacketBuffer(g_mqttSocket, buf, len);

    if (MQTTPacket_read(buf, buflen, transport_getdata) == CONNACK) {
        unsigned char sessionPresent, connack_rc;

        if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, buf, buflen) != 1 || connack_rc != 0) {
            printf("[MQTT] Unable to connect, return code %d\n", connack_rc);
            transport_close(g_mqttSocket);
            g_mqttSocket = -1;
            return -1;
        }
    } else {
        printf("[MQTT] Connection failed - no CONNACK received\n");
        transport_close(g_mqttSocket);
        g_mqttSocket = -1;
        return -1;
    }

    printf("[MQTT] Connected successfully\n");
    return 0;
}

/**
 * @brief 发布传送带系统数据
 * @param hasObject 是否有物品
 * @param temperature 当前温度
 * @param motorRunning 电机状态
 * @param infraredDetected 红外检测状态
 */
void MQTT_PublishData(int hasObject, float temperature, int motorRunning, int infraredDetected)
{
    if (g_mqttSocket < 0) {
        printf("[MQTT] Not connected\n");
        return;
    }

    unsigned char buf[200];
    int buflen = sizeof(buf);
    char payload[200];
    MQTTString topicString = MQTTString_initializer;
    int len = 0;

    topicString.cstring = MQTT_TOPIC;

    // 封装为JSON字符串
    snprintf(payload, sizeof(payload), 
             "{\"hasObject\":%d,\"temperature\":%.1f,\"motorRunning\":%d,\"infraredDetected\":%d}",
             hasObject, temperature, motorRunning, infraredDetected);
    
    int payloadlen = strlen(payload);

    len = MQTTSerialize_publish(buf, buflen, 0, 0, 0, 0, topicString, (unsigned char *)payload, payloadlen);
    transport_sendPacketBuffer(g_mqttSocket, buf, len);

    printf("[MQTT] Published: %s\n", payload);
}

/**
 * @brief 断开MQTT连接
 */
void MQTT_Disconnect(void)
{
    if (g_mqttSocket >= 0) {
        transport_close(g_mqttSocket);
        g_mqttSocket = -1;
        printf("[MQTT] Disconnected\n");
    }
}
