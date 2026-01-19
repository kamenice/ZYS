/*
 * main.c - 智能传送带系统主程序
 * 
 * 功能：
 * 1. HX711压力传感器检测物品 -> TB6612电机驱动启动电机
 * 2. DHT11温度传感器检测温度 -> 超过30度启动蜂鸣器并停止电机
 * 3. YL-62红外传感器检测物品 -> 启动舵机并启动蜂鸣器
 * 4. OLED显示物品状态、温度、电机状态
 * 5. WiFi+MQTT传输数据到手机APP
 * 
 * 硬件配置：
 * - HX711: DOUT=GPIO7, SCK=GPIO8
 * - DHT11: DATA=GPIO3
 * - TB6612: AIN1=GPIO10, AIN2=GPIO11, PWMA=GPIO12(PWM3), STBY=GPIO6
 * - 舵机: PWM=GPIO5(PWM2)
 * - 蜂鸣器: GPIO9（低电平触发）
 * - YL-62: OUT=GPIO4
 * - OLED: I2C0 (GPIO13=SDA, GPIO14=SCL)
 * 
 * WiFi配置：
 * - SSID: zys
 * - Password: 12345678
 * - MQTT服务器: 192.168.43.230:1883
 * 
 * 注意：GPIO0/2不能使用
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_i2c.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_errno.h"

#include "i2c_common.h"
#include "hx711.h"
#include "dht11.h"
#include "motor.h"
#include "buzzer.h"
#include "servo.h"
#include "infrared.h"
#include "oled_ssd1306.h"
#include "oled_task.h"
#include "control_task.h"
#include "publish_task.h"

#define MAIN_TASK_STACK_SIZE 4096

// 主任务函数
static void MainTask_Run(void *arg)
{
    (void)arg;
    
    printf("\n");
    printf("========================================\n");
    printf("  Smart Conveyor Belt System Starting  \n");
    printf("========================================\n");
    printf("\n");
    
    // 启动各个传感器任务
    printf("[Main] Starting HX711 pressure sensor task...\n");
    HX711_MainLoop();       // HX711压力传感器任务
    
    printf("[Main] Starting DHT11 temperature sensor task...\n");
    DHT11_MainLoop();       // DHT11温度传感器任务
    
    printf("[Main] Starting infrared sensor task...\n");
    Infrared_MainLoop();    // YL-62红外传感器任务
    
    printf("[Main] Starting control task...\n");
    ControlTask_MainLoop(); // 系统控制任务
    
    printf("[Main] Starting OLED display task...\n");
    OledTask_MainLoop();    // OLED显示任务
    
    printf("[Main] Starting MQTT publish task...\n");
    publish_task();         // MQTT发布任务
    
    printf("\n");
    printf("[Main] All tasks started successfully!\n");
    printf("\n");
}

static void MainTask(void)
{
    osThreadAttr_t attr;
    attr.name = "MainTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = MAIN_TASK_STACK_SIZE;
    attr.priority = osPriorityNormal;

    // 初始化I2C（OLED和部分传感器共用）
    I2C_CommonInit();

    if (osThreadNew((osThreadFunc_t)MainTask_Run, NULL, &attr) == NULL)
    {
        printf("[Main] Failed to create MainTask!\n");
    }
}

APP_FEATURE_INIT(MainTask);
