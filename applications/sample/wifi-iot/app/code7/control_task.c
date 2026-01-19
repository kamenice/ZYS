/*
 * control_task.c - 系统控制任务
 * 
 * 控制逻辑：
 * 1. HX711压力传感器检测有物品 -> 启动电机
 * 2. DHT11温度超过30度 -> 停止电机并启动蜂鸣器
 * 3. YL-62红外传感器检测到物品 -> 启动舵机并启动蜂鸣器（在infrared.c中处理）
 */

#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"

#include "hx711.h"
#include "dht11.h"
#include "motor.h"
#include "buzzer.h"
#include "servo.h"
#include "infrared.h"

#define CONTROL_TASK_STACK_SIZE 2048

static void ControlTask_Run(void *arg)
{
    (void)arg;
    int last_object_state = 0;
    int alarm_active = 0;
    
    // 初始化各模块
    Motor_Init();
    Buzzer_Init();
    Servo_Init();
    
    printf("[Control] Control task started\n");
    
    while (1)
    {
        // 获取传感器状态
        int has_object = HX711_HasObject();
        int over_temp = DHT11_IsOverTemp();
        float temp = DHT11_GetTemperature();
        
        printf("[Control] object:%d temp:%.1f overtemp:%d\n", has_object, temp, over_temp);
        
        // 逻辑1：温度超过30度 -> 停止电机并报警
        if (over_temp)
        {
            if (!alarm_active) {
                printf("[Control] ALARM: Temperature over 30C! Stopping motor.\n");
                Motor_Stop();
                alarm_active = 1;
            }
            // 蜂鸣器报警（间歇性）
            Buzzer_On();
            usleep(200000);
            Buzzer_Off();
            usleep(200000);
        }
        else
        {
            // 温度正常
            if (alarm_active) {
                printf("[Control] Temperature back to normal.\n");
                Buzzer_Off();
                alarm_active = 0;
            }
            
            // 逻辑2：压力传感器检测到物品 -> 启动电机
            if (has_object && !last_object_state)
            {
                printf("[Control] Object detected! Starting motor.\n");
                Motor_Start();
            }
            else if (!has_object && last_object_state)
            {
                printf("[Control] Object removed. Stopping motor.\n");
                Motor_Stop();
            }
        }
        
        last_object_state = has_object;
        
        usleep(200000); // 200ms控制周期
    }
}

void ControlTask_MainLoop(void)
{
    osThreadAttr_t attr;
    attr.name = "ControlTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = CONTROL_TASK_STACK_SIZE;
    attr.priority = osPriorityAboveNormal; // 较高优先级
    
    if (osThreadNew((osThreadFunc_t)ControlTask_Run, NULL, &attr) == NULL)
    {
        printf("[Control] Failed to create ControlTask!\n");
    }
}
