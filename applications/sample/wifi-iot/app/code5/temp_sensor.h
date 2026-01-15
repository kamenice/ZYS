/*
 * 温度传感器头文件
 * 使用AHT20温湿度传感器读取温度
 */

#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H

#include <stdint.h>

/**
 * @brief 初始化温度传感器
 */
void TempSensor_Init(void);

/**
 * @brief 读取温度值
 * @return 当前温度（摄氏度）
 */
float TempSensor_ReadTemperature(void);

/**
 * @brief 读取湿度值
 * @return 当前湿度（%）
 */
float TempSensor_ReadHumidity(void);

/**
 * @brief 检查温度是否超过阈值
 * @param threshold 温度阈值
 * @return 1: 超过阈值, 0: 未超过
 */
int TempSensor_IsOverThreshold(float threshold);

#endif // TEMP_SENSOR_H
