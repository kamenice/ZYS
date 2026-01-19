/*
 * OLED显示实现
 * 用于显示传送带系统状态
 */

#include <stdio.h>
#include <string.h>
#include "oled_display.h"
#include "oled_ssd1306.h"
#include "wifiiot_gpio.h"

/**
 * @brief 初始化OLED显示屏
 * @return 0: 成功, 其他: 失败
 */
uint32_t OledDisplay_Init(void)
{
    GpioInit();
    uint32_t ret = OledInit();
    if (ret == 0) {
        OledDisplay_Clear();
        printf("[OledDisplay] Initialized\n");
    }
    return ret;
}

/**
 * @brief 清除屏幕
 */
void OledDisplay_Clear(void)
{
    OledFillScreen(0x00);
}

/**
 * @brief 显示字符串
 * @param x X坐标
 * @param y Y坐标
 * @param str 要显示的字符串
 */
void OledDisplay_ShowString(uint8_t x, uint8_t y, const char *str)
{
    OledShowString(x, y, str, FONT6x8);
}

/**
 * @brief 更新传送带系统状态显示
 * @param hasObject 是否有物品 (1: 有, 0: 无)
 * @param temperature 当前温度
 * @param motorRunning 电机状态 (1: 运行, 0: 停止)
 */
void OledDisplay_UpdateStatus(int hasObject, float temperature, int motorRunning)
{
    char line[32] = {0};
    
    // 第一行：标题
    OledShowString(8, 0, "Smart Conveyor", FONT6x8);
    
    // 第二行：物品检测状态
    if (hasObject) {
        snprintf(line, sizeof(line), "Object: YES     ");
    } else {
        snprintf(line, sizeof(line), "Object: NO      ");
    }
    OledShowString(0, 2, line, FONT6x8);
    
    // 第三行：温度
    snprintf(line, sizeof(line), "Temp: %.1f C    ", temperature);
    OledShowString(0, 4, line, FONT6x8);
    
    // 第四行：电机状态
    if (motorRunning) {
        snprintf(line, sizeof(line), "Motor: RUNNING  ");
    } else {
        snprintf(line, sizeof(line), "Motor: STOPPED  ");
    }
    OledShowString(0, 6, line, FONT6x8);
}
