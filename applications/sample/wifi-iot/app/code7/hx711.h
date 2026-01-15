/*
 * hx711.h - HX711压力传感器驱动（检测有无物品）
 */

#ifndef HX711_H
#define HX711_H

#include <stdint.h>

// HX711引脚定义 - 避免使用GPIO0/2
// DOUT: GPIO7, SCK: GPIO8
#define HX711_DOUT_GPIO     WIFI_IOT_GPIO_IDX_7
#define HX711_DOUT_IO_NAME  WIFI_IOT_IO_NAME_GPIO_7
#define HX711_SCK_GPIO      WIFI_IOT_GPIO_IDX_8
#define HX711_SCK_IO_NAME   WIFI_IOT_IO_NAME_GPIO_8

// 物品检测阈值（ADC值超过此值表示有物品）
#define HX711_OBJECT_THRESHOLD 500

// 初始化HX711
void HX711_Init(void);

// 读取HX711原始值
int32_t HX711_ReadRaw(void);

// 检测是否有物品（1=有物品，0=无物品）
int HX711_HasObject(void);

// 获取最近一次读取的原始值
int32_t HX711_GetLastValue(void);

// HX711主任务入口
void HX711_MainLoop(void);

#endif
