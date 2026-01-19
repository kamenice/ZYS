/*
 * oled_task.c - OLED显示任务
 * 
 * 显示内容：
 * 1. 是否有物品（压力传感器）
 * 2. 温度
 * 3. 电机运行状态
 */

#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_errno.h"

#include "oled_ssd1306.h"
#include "hx711.h"
#include "dht11.h"
#include "motor.h"
#include "infrared.h"

#define OLED_TASK_STACK_SIZE 4096

static void OledTask_Init(void)
{
    GpioInit();
    OledInit();
}

static void OledTask_Run(void *arg)
{
    (void)arg;
    static char line[32] = {0};
    
    OledTask_Init();
    OledFillScreen(0x00); // 清屏
    
    // 显示标题
    OledShowString(0, 0, "Smart Conveyor", 1);
    sleep(1);
    
    while (1)
    {
        // 第1行：标题
        OledShowString(0, 0, "Smart Conveyor  ", 1);
        
        // 第2行：物品检测状态
        int has_object = HX711_HasObject();
        snprintf(line, sizeof(line), "Object: %s     ", has_object ? "YES" : "NO ");
        OledShowString(0, 1, line, 1);
        
        // 第3行：温度
        float temp = DHT11_GetTemperature();
        snprintf(line, sizeof(line), "Temp: %.1f C   ", temp);
        OledShowString(0, 2, line, 1);
        
        // 第4行：湿度
        float humi = DHT11_GetHumidity();
        snprintf(line, sizeof(line), "Humi: %.1f %%   ", humi);
        OledShowString(0, 3, line, 1);
        
        // 第5行：电机状态
        int motor_state = Motor_GetState();
        snprintf(line, sizeof(line), "Motor: %s    ", 
                 motor_state == MOTOR_STATE_RUNNING ? "RUN " : "STOP");
        OledShowString(0, 4, line, 1);
        
        // 第6行：红外检测状态
        int ir_detected = Infrared_GetLastState();
        snprintf(line, sizeof(line), "IR: %s        ", ir_detected ? "YES" : "NO ");
        OledShowString(0, 5, line, 1);
        
        // 第7行：超温警告
        if (DHT11_IsOverTemp()) {
            OledShowString(0, 6, "!! OVER TEMP !!", 1);
        } else {
            OledShowString(0, 6, "               ", 1);
        }
        
        usleep(500000); // 500ms刷新间隔
    }
}

void OledTask_MainLoop(void)
{
    osThreadAttr_t attr;
    attr.name = "OledTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = OLED_TASK_STACK_SIZE;
    attr.priority = osPriorityNormal;
    
    if (osThreadNew(OledTask_Run, NULL, &attr) == NULL)
    {
        printf("[OledTask] Failed to create OledTask!\n");
    }
}
