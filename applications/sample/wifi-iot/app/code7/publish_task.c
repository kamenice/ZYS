/*
 * publish_task.c - MQTT数据发布任务
 */

#include "cmsis_os2.h"
#include <unistd.h>
#include <stdio.h>
#include "wifi_util.h"
#include "mqtt_util.h"

static int g_wifi_connected = 0;

static void publish_thread(void *arg)
{
    (void)arg;

    while (1)
    {
        // 首次连接或重连WiFi
        if (!g_wifi_connected) {
            printf("[publish_task] Connecting to WiFi...\n");
            connect_wifi();
            sleep(2);  // 等待WiFi连接稳定
            g_wifi_connected = 1;
        }
        
        // 连接MQTT并发送数据，如果连接断开会返回
        int ret = mqtt_connect();
        if (ret < 0) {
            printf("[publish_task] MQTT connection failed, will retry...\n");
            g_wifi_connected = 0;  // 标记需要重连WiFi
        }
        
        sleep(5);  // 重试间隔
    }
}

void publish_task(void)
{
    osThreadAttr_t attr;

    attr.name = "publish_task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = 36;

    if (osThreadNew((osThreadFunc_t)publish_thread, NULL, &attr) == NULL)
    {
        printf("[publish_task] Failed to create publish_task!\n");
    }
}
