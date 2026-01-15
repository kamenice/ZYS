/*
 * 红外线传感器头文件
 * 用于检测物品，检测到物品时输出低电平
 */

#ifndef INFRARED_H
#define INFRARED_H

/**
 * @brief 初始化红外传感器
 */
void Infrared_Init(void);

/**
 * @brief 检测是否有物品
 * @return 1: 检测到物品（低电平）, 0: 未检测到物品
 */
int Infrared_DetectObject(void);

#endif // INFRARED_H
