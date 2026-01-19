/*
 * mqtt_util.c - MQTT连接工具
 * 
 * 将数据传到DevEco Studio所写的手机APP
 * 电脑连上WiFi的IP是192.168.43.230
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

#include "hx711.h"
#include "dht11.h"
#include "motor.h"
#include "infrared.h"

int mqtt_connect(void)
{
    // 1.定义连接参数
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    int mysock = 0;
    unsigned char buf[200];
    int buflen = sizeof(buf);

    MQTTString topicString = MQTTString_initializer;
    char payload[200];
    int payloadlen;
    int len = 0;

    // 2.配置连接参数
    char *host = "192.168.43.230"; // MQTT服务器地址（电脑IP）
    int port = 1883;               // MQTT服务器端口
    topicString.cstring = "conveyor"; // 主题名称

    data.clientID.cstring = "hi3861-conveyor"; // 客户端ID
    data.keepAliveInterval = 20;
    data.cleansession = 1;
    data.username.cstring = "hi3861_user";
    data.password.cstring = "conveyor123";

    // 3.连接服务器
    mysock = transport_open(host, port);
    if (mysock < 0) {
        printf("[MQTT] transport_open failed: %d\n", mysock);
        return mysock;
    }

    printf("[MQTT] Connecting to %s:%d\n", host, port);

    len = MQTTSerialize_connect(buf, buflen, &data);
    transport_sendPacketBuffer(mysock, buf, len);

    if (MQTTPacket_read(buf, buflen, transport_getdata) == CONNACK)
    {
        unsigned char sessionPresent, connack_rc;

        if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, buf, buflen) != 1 || connack_rc != 0)
        {
            printf("[MQTT] Unable to connect, return code %d\n", connack_rc);
            goto exit;
        }
        printf("[MQTT] Connected successfully\n");
    }
    else {
        printf("[MQTT] CONNACK not received\n");
        goto exit;
    }

    // 4.循环发送消息
    while (1)
    {
        // 获取传感器数据
        int has_object = HX711_HasObject();
        float temperature = DHT11_GetTemperature();
        float humidity = DHT11_GetHumidity();
        int motor_state = Motor_GetState();
        int ir_detected = Infrared_GetLastState();
        int over_temp = DHT11_IsOverTemp();

        // 封装为JSON字符串
        snprintf(payload, sizeof(payload), 
                 "{\"object\":%d,\"temp\":%.1f,\"humi\":%.1f,\"motor\":%d,\"ir\":%d,\"alarm\":%d}",
                 has_object, temperature, humidity, motor_state, ir_detected, over_temp);
        payloadlen = strlen(payload);
        
        len = MQTTSerialize_publish(buf, buflen, 0, 0, 0, 0, topicString, (unsigned char *)payload, payloadlen);
        transport_sendPacketBuffer(mysock, buf, len);
        
        printf("[MQTT] Published: %s\n", payload);

        sleep(2); // 每2秒发送一次
    }

exit:
    transport_close(mysock);
    return 0;
}
