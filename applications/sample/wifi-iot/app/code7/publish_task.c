/*
 * publish_task.c - MQTT数据发布任务
 */

#include "cmsis_os2.h"
#include <unistd.h>
#include <stdio.h>
#include "wifi_util.h"
#include "mqtt_util.h"

static void publish_thread(void *arg)
{
    (void)arg;

    while (1)
    {
        connect_wifi(); // 连接WiFi
        sleep(2);       // 等待WiFi连接
        mqtt_connect(); // 连接MQTT并发送数据
        sleep(5);       // 重试间隔
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
