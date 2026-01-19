/*
 * HX711 压力传感器驱动头文件
 * 用于检测传送带上是否有物品
 */

#ifndef HX711_H
#define HX711_H

#include <stdint.h>

/**
 * @brief 初始化HX711传感器
 */
void HX711_Init(void);

/**
 * @brief 读取传感器原始值
 * @return 传感器原始数据
 */
unsigned long HX711_Read(void);

/**
 * @brief 获取传感器平均值
 * @return 传感器平均值
 */
double HX711_GetAverageValue(void);

/**
 * @brief 检测是否有物品
 * @return 1: 有物品, 0: 无物品
 */
int HX711_HasObject(void);

/**
 * @brief 设置基准值
 */
void HX711_SetBaseline(void);

#endif // HX711_H
