/*
 * 传送带任务实现
 * 定义各个子任务
 */

#include <stdio.h>
#include <unistd.h>
#include "cmsis_os2.h"
#include "conveyor_task.h"

#include "hx711.h"
#include "tb6612.h"
#include "temp_sensor.h"
#include "buzzer.h"
#include "infrared.h"
#include "servo.h"
#include "oled_display.h"
#include "wifi_connect.h"
#include "mqtt_client.h"

// 全局状态变量
static int g_hasObject = 0;          // HX711检测到的物品状态
static float g_temperature = 0.0f;    // 当前温度
static int g_motorRunning = 0;        // 电机运行状态
static int g_infraredDetected = 0;    // 红外检测状态

// 温度阈值
#define TEMP_THRESHOLD  30.0f

/**
 * @brief 压力传感器任务 - 检测物品并控制电机
 * HX711压力传感器检测是否有物品
 * 有物品时启动TB6612电机驱动
 */
static void PressureSensor_TaskRun(void *arg)
{
    (void)arg;
    
    printf("[PressureTask] Starting...\n");
    
    // 初始化HX711和TB6612
    HX711_Init();
    TB6612_Init();
    
    // 等待稳定后设置基准值
    sleep(2);
    HX711_SetBaseline();
    
    while (1) {
        // 检测是否有物品
        g_hasObject = HX711_HasObject();
        
        if (g_hasObject) {
            // 有物品，启动电机
            if (!TB6612_IsRunning()) {
                TB6612_Forward();
            }
        } else {
            // 无物品，停止电机
            if (TB6612_IsRunning()) {
                TB6612_Stop();
            }
        }
        
        g_motorRunning = TB6612_IsRunning();
        
        usleep(500000);  // 500ms
    }
}

void PressureSensor_Task(void)
{
    osThreadAttr_t attr;
    
    attr.name = "PressureSensorTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 2048;
    attr.priority = osPriorityNormal;
    
    if (osThreadNew(PressureSensor_TaskRun, NULL, &attr) == NULL) {
        printf("[PressureTask] Failed to create task!\n");
    }
}

/**
 * @brief 温度监测任务 - 监测温度并触发蜂鸣器
 * 温度超过30度时启动有源蜂鸣器（低电平触发）
 */
static void TemperatureMonitor_TaskRun(void *arg)
{
    (void)arg;
    
    printf("[TempTask] Starting...\n");
    
    // 初始化温度传感器和蜂鸣器
    TempSensor_Init();
    Buzzer_Init();
    
    while (1) {
        // 读取温度
        g_temperature = TempSensor_ReadTemperature();
        
        printf("[TempTask] Temperature: %.1f C\n", g_temperature);
        
        // 检查是否超过阈值
        if (g_temperature > TEMP_THRESHOLD) {
            // 温度超过30度，触发蜂鸣器报警
            Buzzer_On();
            printf("[TempTask] Temperature alarm! %.1f > %.1f\n", g_temperature, TEMP_THRESHOLD);
        } else {
            // 温度正常，关闭蜂鸣器
            Buzzer_Off();
        }
        
        sleep(1);  // 每秒检测一次
    }
}

void TemperatureMonitor_Task(void)
{
    osThreadAttr_t attr;
    
    attr.name = "TempMonitorTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 2048;
    attr.priority = osPriorityNormal;
    
    if (osThreadNew(TemperatureMonitor_TaskRun, NULL, &attr) == NULL) {
        printf("[TempTask] Failed to create task!\n");
    }
}

/**
 * @brief 红外传感器任务 - 检测物品并控制舵机
 * 红外传感器检测到物品时（低电平），启动舵机
 * 舵机使用PWM信号：20ms周期，高电平0.5-2.5ms
 */
static void InfraredSensor_TaskRun(void *arg)
{
    (void)arg;
    
    printf("[InfraredTask] Starting...\n");
    
    // 初始化红外传感器和舵机
    Infrared_Init();
    Servo_Init();
    
    // 舵机复位
    Servo_Reset();
    usleep(500000);
    Servo_Stop();
    
    while (1) {
        // 检测红外传感器
        g_infraredDetected = Infrared_DetectObject();
        
        if (g_infraredDetected) {
            // 检测到物品（低电平），启动舵机
            printf("[InfraredTask] Object detected! Activating servo...\n");
            
            // 舵机转到90度位置（分拣动作）
            Servo_SetAngle(90);
            usleep(1000000);  // 保持1秒
            
            // 舵机复位
            Servo_Reset();
            usleep(500000);
            Servo_Stop();
        }
        
        usleep(200000);  // 200ms检测间隔
    }
}

void InfraredSensor_Task(void)
{
    osThreadAttr_t attr;
    
    attr.name = "InfraredTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 2048;
    attr.priority = osPriorityNormal;
    
    if (osThreadNew(InfraredSensor_TaskRun, NULL, &attr) == NULL) {
        printf("[InfraredTask] Failed to create task!\n");
    }
}

/**
 * @brief OLED显示任务 - 更新显示屏状态
 * 显示：是否有物品、温度、电机运行状态
 */
static void OledDisplay_TaskRun(void *arg)
{
    (void)arg;
    
    printf("[OledTask] Starting...\n");
    
    // 初始化OLED
    OledDisplay_Init();
    
    // 显示欢迎信息
    OledDisplay_Clear();
    OledDisplay_ShowString(0, 0, "Smart Conveyor");
    OledDisplay_ShowString(0, 2, "System Starting...");
    sleep(2);
    
    while (1) {
        // 更新显示状态
        OledDisplay_UpdateStatus(g_hasObject, g_temperature, g_motorRunning);
        
        usleep(500000);  // 500ms刷新间隔
    }
}

void OledDisplay_Task(void)
{
    osThreadAttr_t attr;
    
    attr.name = "OledDisplayTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;
    
    if (osThreadNew(OledDisplay_TaskRun, NULL, &attr) == NULL) {
        printf("[OledTask] Failed to create task!\n");
    }
}

/**
 * @brief MQTT发布任务 - 上传数据到手机APP
 * WiFi名称: zys, 密码: 12345678
 * 服务器IP: 192.168.43.230
 */
static void MqttPublish_TaskRun(void *arg)
{
    (void)arg;
    
    printf("[MqttTask] Starting...\n");
    
    // 连接WiFi
    WiFi_Connect();
    
    // 等待WiFi连接成功
    sleep(5);
    
    if (!WiFi_IsConnected()) {
        printf("[MqttTask] WiFi not connected, task exiting\n");
        return;
    }
    
    // 连接MQTT服务器
    if (MQTT_Connect() != 0) {
        printf("[MqttTask] MQTT connection failed, task exiting\n");
        return;
    }
    
    while (1) {
        // 发布数据
        MQTT_PublishData(g_hasObject, g_temperature, g_motorRunning, g_infraredDetected);
        
        sleep(1);  // 每秒发送一次
    }
}

void MqttPublish_Task(void)
{
    osThreadAttr_t attr;
    
    attr.name = "MqttPublishTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 8192;
    attr.priority = osPriorityNormal;
    
    if (osThreadNew(MqttPublish_TaskRun, NULL, &attr) == NULL) {
        printf("[MqttTask] Failed to create task!\n");
    }
}
