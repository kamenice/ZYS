/*
 * 智能传送带系统 - 主程序
 * 
 * 功能说明:
 * 1. 压力传感器(HX711)检测是否有物品，有则启动TB6612电机驱动
 * 2. 温度传感器检测温度，超过30度启动有源蜂鸣器(低电平触发)
 * 3. 红外传感器检测物品(低电平)，检测到时启动舵机(PWM: 20ms周期，0.5-2.5ms高电平)
 * 4. OLED显示：物品状态、温度、电机状态
 * 5. WiFi(SSID:zys, PWD:12345678)和MQTT(192.168.43.230)传输数据到手机APP
 */

#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "conveyor_task.h"

/**
 * @brief 主任务线程
 */
static void Conveyor_MainThread(void *arg)
{
    (void)arg;
    
    printf("========================================\n");
    printf("  Smart Conveyor Belt System Starting\n");
    printf("========================================\n");
    
    sleep(2);  // 等待系统稳定
    
    // 启动各个子任务
    printf("[Main] Starting pressure sensor task...\n");
    PressureSensor_Task();      // 压力传感器任务 - 检测物品并控制电机
    
    printf("[Main] Starting temperature monitor task...\n");
    TemperatureMonitor_Task();  // 温度监测任务 - 监测温度并触发蜂鸣器
    
    printf("[Main] Starting infrared sensor task...\n");
    InfraredSensor_Task();      // 红外传感器任务 - 检测物品并控制舵机
    
    printf("[Main] Starting OLED display task...\n");
    OledDisplay_Task();         // OLED显示任务 - 更新显示屏状态
    
    printf("[Main] Starting MQTT publish task...\n");
    MqttPublish_Task();         // MQTT发布任务 - 上传数据到手机APP
    
    printf("[Main] All tasks started successfully!\n");
    printf("========================================\n");
}

/**
 * @brief 系统入口函数
 */
void Conveyor_Entry(void)
{
    osThreadAttr_t attr;
    
    attr.name = "ConveyorMain";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = 36;
    
    if (osThreadNew((osThreadFunc_t)Conveyor_MainThread, NULL, &attr) == NULL) {
        printf("[Main] Failed to create main thread!\n");
    }
}

SYS_RUN(Conveyor_Entry);
