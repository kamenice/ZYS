/*
 * infrared.c - YL-62红外线避障传感器驱动
 * 
 * YL-62当检测到物品时输出低电平信号
 * 检测到低电平时启动舵机并启动蜂鸣器
 */

#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_errno.h"

#include "infrared.h"
#include "servo.h"
#include "buzzer.h"

#define INFRARED_TASK_STACK_SIZE 2048

static volatile int g_infrared_detected = 0;

void Infrared_Init(void)
{
    GpioInit();
    IoSetFunc(INFRARED_IO_NAME, WIFI_IOT_IO_FUNC_GPIO_4_GPIO);
    GpioSetDir(INFRARED_GPIO, WIFI_IOT_GPIO_DIR_IN);
    IoSetPull(INFRARED_IO_NAME, WIFI_IOT_IO_PULL_UP);
    printf("[Infrared] Init done\n");
}

int Infrared_DetectObject(void)
{
    WifiIotGpioValue val;
    GpioGetInputVal(INFRARED_GPIO, &val);
    // YL-62检测到物品时输出低电平
    return (val == WIFI_IOT_GPIO_VALUE0) ? 1 : 0;
}

int Infrared_GetLastState(void)
{
    return g_infrared_detected;
}

static void Infrared_Task(void *arg)
{
    (void)arg;
    int last_state = 0;
    
    Infrared_Init();
    
    while (1) {
        int detected = Infrared_DetectObject();
        g_infrared_detected = detected;
        
        // 检测到物品时（上升沿触发）
        if (detected && !last_state) {
            printf("[Infrared] Object detected! Activating servo and buzzer\n");
            
            // 启动舵机（转到90度）
            Servo_Open();
            
            // 启动蜂鸣器响500ms
            Buzzer_Beep(500);
            
            // 延时后舵机复位
            usleep(1000000); // 1秒
            Servo_Close();
        }
        
        last_state = detected;
        
        printf("[Infrared] detected:%d\n", detected);
        usleep(200000); // 200ms采样间隔
    }
}

void Infrared_MainLoop(void)
{
    osThreadAttr_t attr;
    attr.name = "Infrared_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = INFRARED_TASK_STACK_SIZE;
    attr.priority = osPriorityNormal;
    
    if (osThreadNew((osThreadFunc_t)Infrared_Task, NULL, &attr) == NULL) {
        printf("[Infrared] Failed to create Infrared_Task!\n");
    }
}
