/*
 * 红外线传感器实现
 * 用于检测物品，检测到物品时输出低电平
 * 
 * GPIO配置:
 * - 红外传感器输出: GPIO7
 */

#include <stdio.h>
#include <unistd.h>
#include "infrared.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"

// 红外传感器引脚定义
#define INFRARED_GPIO  WIFI_IOT_GPIO_IDX_7

/**
 * @brief 初始化红外传感器
 */
void Infrared_Init(void)
{
    // 配置红外传感器引脚为GPIO输入
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_7, WIFI_IOT_IO_FUNC_GPIO_7_GPIO);
    GpioSetDir(INFRARED_GPIO, WIFI_IOT_GPIO_DIR_IN);
    
    // 设置上拉电阻
    IoSetPull(WIFI_IOT_IO_NAME_GPIO_7, WIFI_IOT_IO_PULL_UP);
    
    printf("[Infrared] Initialized\n");
}

/**
 * @brief 检测是否有物品
 * 红外传感器检测到物品时输出低电平
 * @return 1: 检测到物品（低电平）, 0: 未检测到物品
 */
int Infrared_DetectObject(void)
{
    WifiIotGpioValue value = WIFI_IOT_GPIO_VALUE1;
    
    GpioGetInputVal(INFRARED_GPIO, &value);
    
    // 低电平表示检测到物品
    if (value == WIFI_IOT_GPIO_VALUE0) {
        return 1;
    }
    
    return 0;
}
