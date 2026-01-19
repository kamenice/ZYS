/*
 * 有源蜂鸣器驱动实现
 * 低电平触发
 * 
 * GPIO配置:
 * - 蜂鸣器: GPIO8 (低电平触发)
 */

#include <stdio.h>
#include <unistd.h>
#include "buzzer.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"

// 蜂鸣器引脚定义 (有源蜂鸣器，低电平触发)
#define BUZZER_GPIO  WIFI_IOT_GPIO_IDX_8

/**
 * @brief 初始化蜂鸣器
 */
void Buzzer_Init(void)
{
    // 配置蜂鸣器引脚为GPIO输出
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_8, WIFI_IOT_IO_FUNC_GPIO_8_GPIO);
    GpioSetDir(BUZZER_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    
    // 初始状态为高电平（蜂鸣器关闭）
    GpioSetOutputVal(BUZZER_GPIO, WIFI_IOT_GPIO_VALUE1);
    
    printf("[Buzzer] Initialized (active low trigger)\n");
}

/**
 * @brief 打开蜂鸣器（低电平触发）
 */
void Buzzer_On(void)
{
    // 低电平触发蜂鸣器
    GpioSetOutputVal(BUZZER_GPIO, WIFI_IOT_GPIO_VALUE0);
}

/**
 * @brief 关闭蜂鸣器
 */
void Buzzer_Off(void)
{
    // 高电平关闭蜂鸣器
    GpioSetOutputVal(BUZZER_GPIO, WIFI_IOT_GPIO_VALUE1);
}

/**
 * @brief 蜂鸣器报警（持续响）
 */
void Buzzer_Alarm(void)
{
    Buzzer_On();
    printf("[Buzzer] Alarm triggered!\n");
}
