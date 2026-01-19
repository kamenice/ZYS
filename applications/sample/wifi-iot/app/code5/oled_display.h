/*
 * OLED显示头文件
 * 用于显示传送带系统状态
 */

#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <stdint.h>

/**
 * @brief 初始化OLED显示屏
 * @return 0: 成功, 其他: 失败
 */
uint32_t OledDisplay_Init(void);

/**
 * @brief 清除屏幕
 */
void OledDisplay_Clear(void);

/**
 * @brief 显示字符串
 * @param x X坐标
 * @param y Y坐标
 * @param str 要显示的字符串
 */
void OledDisplay_ShowString(uint8_t x, uint8_t y, const char *str);

/**
 * @brief 更新传送带系统状态显示
 * @param hasObject 是否有物品 (1: 有, 0: 无)
 * @param temperature 当前温度
 * @param motorRunning 电机状态 (1: 运行, 0: 停止)
 */
void OledDisplay_UpdateStatus(int hasObject, float temperature, int motorRunning);

#endif // OLED_DISPLAY_H
