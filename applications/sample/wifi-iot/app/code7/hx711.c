/*
 * hx711.c - HX711压力传感器驱动（检测有无物品）
 * 
 * HX711是一个24位ADC，用于称重传感器
 * 这里只检测有无物品，不需要换算具体重量
 */

#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_errno.h"

#include "hx711.h"

#define HX711_TASK_STACK_SIZE 2048

static volatile int32_t g_hx711_value = 0;
static volatile int g_has_object = 0;

void HX711_Init(void)
{
    GpioInit();
    
    // DOUT引脚设置为输入
    IoSetFunc(HX711_DOUT_IO_NAME, WIFI_IOT_IO_FUNC_GPIO_7_GPIO);
    GpioSetDir(HX711_DOUT_GPIO, WIFI_IOT_GPIO_DIR_IN);
    IoSetPull(HX711_DOUT_IO_NAME, WIFI_IOT_IO_PULL_UP);
    
    // SCK引脚设置为输出
    IoSetFunc(HX711_SCK_IO_NAME, WIFI_IOT_IO_FUNC_GPIO_8_GPIO);
    GpioSetDir(HX711_SCK_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(HX711_SCK_GPIO, WIFI_IOT_GPIO_VALUE0);
}

// 读取HX711的24位数据
int32_t HX711_ReadRaw(void)
{
    int32_t value = 0;
    WifiIotGpioValue dout_val;
    int i;
    
    // 等待DOUT变低（数据准备好）
    int timeout = 1000;
    do {
        GpioGetInputVal(HX711_DOUT_GPIO, &dout_val);
        if (dout_val == WIFI_IOT_GPIO_VALUE0) break;
        usleep(100);
        timeout--;
    } while (timeout > 0);
    
    if (timeout <= 0) {
        return 0; // 超时
    }
    
    // 读取24位数据
    for (i = 0; i < 24; i++) {
        GpioSetOutputVal(HX711_SCK_GPIO, WIFI_IOT_GPIO_VALUE1);
        usleep(1);
        value = value << 1;
        GpioSetOutputVal(HX711_SCK_GPIO, WIFI_IOT_GPIO_VALUE0);
        usleep(1);
        GpioGetInputVal(HX711_DOUT_GPIO, &dout_val);
        if (dout_val == WIFI_IOT_GPIO_VALUE1) {
            value++;
        }
    }
    
    // 第25个脉冲，设置增益为128
    GpioSetOutputVal(HX711_SCK_GPIO, WIFI_IOT_GPIO_VALUE1);
    usleep(1);
    GpioSetOutputVal(HX711_SCK_GPIO, WIFI_IOT_GPIO_VALUE0);
    usleep(1);
    
    // 处理符号位（24位补码）
    if (value & 0x800000) {
        value |= 0xFF000000;
    }
    
    return value;
}

int HX711_HasObject(void)
{
    return g_has_object;
}

int32_t HX711_GetLastValue(void)
{
    return g_hx711_value;
}

static void HX711_Task(void *arg)
{
    (void)arg;
    int32_t raw_value;
    int32_t baseline = 0;
    int calibrated = 0;
    
    HX711_Init();
    
    // 校准：读取初始基线值
    usleep(500000);
    for (int i = 0; i < 10; i++) {
        baseline += HX711_ReadRaw();
        usleep(100000);
    }
    baseline /= 10;
    calibrated = 1;
    printf("[HX711] Calibration done, baseline: %d\n", (int)baseline);
    
    while (1) {
        raw_value = HX711_ReadRaw();
        g_hx711_value = raw_value;
        
        if (calibrated) {
            // 检测物品：判断与基线的差值是否超过阈值
            int32_t diff = raw_value - baseline;
            if (diff < 0) diff = -diff;
            
            if (diff > HX711_OBJECT_THRESHOLD) {
                g_has_object = 1;
            } else {
                g_has_object = 0;
            }
        }
        
        printf("[HX711] raw:%d has_object:%d\n", (int)g_hx711_value, g_has_object);
        usleep(500000); // 500ms采样间隔
    }
}

void HX711_MainLoop(void)
{
    osThreadAttr_t attr;
    attr.name = "HX711_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = HX711_TASK_STACK_SIZE;
    attr.priority = osPriorityNormal;
    
    if (osThreadNew((osThreadFunc_t)HX711_Task, NULL, &attr) == NULL) {
        printf("[HX711] Failed to create HX711_Task!\n");
    }
}
