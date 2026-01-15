/*
 * HX711 压力传感器驱动实现
 * 用于检测传送带上是否有物品
 * 
 * GPIO引脚配置:
 * - DT (数据): GPIO11
 * - SCK (时钟): GPIO12
 */

#include <stdio.h>
#include <unistd.h>
#include "hx711.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"

// HX711引脚定义
#define HX711_DT_GPIO   WIFI_IOT_GPIO_IDX_11  // 数据引脚
#define HX711_SCK_GPIO  WIFI_IOT_GPIO_IDX_12  // 时钟引脚

// 检测阈值 - 当读数变化超过此值时认为有物品
#define HX711_THRESHOLD 5000

// 基准值
static double g_baselineValue = 0;

/**
 * @brief 初始化HX711传感器
 */
void HX711_Init(void)
{
    // 设置DT引脚为输入模式
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_IO_FUNC_GPIO_11_GPIO);
    GpioSetDir(HX711_DT_GPIO, WIFI_IOT_GPIO_DIR_IN);
    
    // 设置SCK引脚为输出模式
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_12, WIFI_IOT_IO_FUNC_GPIO_12_GPIO);
    GpioSetDir(HX711_SCK_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(HX711_SCK_GPIO, WIFI_IOT_GPIO_VALUE0);
    
    printf("[HX711] Initialized\n");
}

/**
 * @brief 读取传感器原始值
 * @return 传感器原始数据
 */
unsigned long HX711_Read(void)
{
    unsigned long value = 0;
    unsigned char i = 0;
    WifiIotGpioValue dtValue = WIFI_IOT_GPIO_VALUE0;
    
    // 等待AD转换完成（DT变为低电平）
    usleep(2);
    GpioSetOutputVal(HX711_SCK_GPIO, WIFI_IOT_GPIO_VALUE0);
    usleep(2);
    
    GpioGetInputVal(HX711_DT_GPIO, &dtValue);
    while (dtValue == WIFI_IOT_GPIO_VALUE1) {
        GpioGetInputVal(HX711_DT_GPIO, &dtValue);
        usleep(1);
    }
    
    // 读取24位数据
    for (i = 0; i < 24; i++) {
        // 时钟上升沿
        GpioSetOutputVal(HX711_SCK_GPIO, WIFI_IOT_GPIO_VALUE1);
        usleep(2);
        
        // 数据左移，准备接收新位
        value = value << 1;
        
        // 时钟下降沿
        GpioSetOutputVal(HX711_SCK_GPIO, WIFI_IOT_GPIO_VALUE0);
        usleep(2);
        
        // 读取数据位
        GpioGetInputVal(HX711_DT_GPIO, &dtValue);
        if (dtValue == WIFI_IOT_GPIO_VALUE1) {
            value++;
        }
    }
    
    // 第25个脉冲，设置增益为128
    GpioSetOutputVal(HX711_SCK_GPIO, WIFI_IOT_GPIO_VALUE1);
    usleep(2);
    // XOR 0x800000: HX711输出为24位补码，MSB为符号位
    // 此操作将补码转换为偏移二进制表示，便于后续计算
    value = value ^ 0x800000;
    GpioSetOutputVal(HX711_SCK_GPIO, WIFI_IOT_GPIO_VALUE0);
    usleep(2);
    
    return value;
}

/**
 * @brief 获取传感器平均值（10次采样）
 * @return 传感器平均值
 */
double HX711_GetAverageValue(void)
{
    double sum = 0;
    int i;
    
    for (i = 0; i < 10; i++) {
        sum += HX711_Read();
    }
    
    return sum / 10.0;
}

/**
 * @brief 设置基准值
 */
void HX711_SetBaseline(void)
{
    g_baselineValue = HX711_GetAverageValue();
    printf("[HX711] Baseline set: %.2f\n", g_baselineValue);
}

/**
 * @brief 检测是否有物品
 * @return 1: 有物品, 0: 无物品
 */
int HX711_HasObject(void)
{
    double currentValue = HX711_GetAverageValue();
    double diff = currentValue - g_baselineValue;
    
    // 如果差值超过阈值，认为有物品
    if (diff > HX711_THRESHOLD || diff < -HX711_THRESHOLD) {
        return 1;
    }
    
    return 0;
}
