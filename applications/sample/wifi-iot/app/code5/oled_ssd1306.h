/*
 * OLED SSD1306驱动头文件
 */

#ifndef OLED_SSD1306_H
#define OLED_SSD1306_H

#include <stdint.h>

#define OLED_I2C_IDX WIFI_IOT_I2C_IDX_0

#define OLED_WIDTH    (128)
#define OLED_I2C_ADDR 0x78      // 默认地址为 0x78
#define OLED_I2C_CMD 0x00       // 0000 0000 写命令
#define OLED_I2C_DATA 0x40      // 0100 0000(0x40) 写数据
#define OLED_I2C_BAUDRATE (400*1000) // 400k

/**
 * @brief ssd1306 OLED初始化
 */
uint32_t OledInit(void);

/**
 * @brief 设置光标位置
 * @param x 水平位置
 * @param y 垂直位置 
 */
void OledSetPosition(uint8_t x, uint8_t y);

/**
 * @brief 全屏填充
 * @param fillData 填充数据
 */
void OledFillScreen(uint8_t fillData);

/**
 * @brief 字体枚举
 */
typedef enum {
    FONT6x8 = 1,
    FONT8x16
} Font;

/**
 * @brief 显示单个字符
 * @param x X坐标
 * @param y Y坐标
 * @param ch 字符
 * @param font 字体
 */
void OledShowChar(uint8_t x, uint8_t y, uint8_t ch, Font font);

/**
 * @brief 显示字符串
 * @param x X坐标
 * @param y Y坐标
 * @param str 字符串
 * @param font 字体
 */
void OledShowString(uint8_t x, uint8_t y, const char* str, Font font);

#endif // OLED_SSD1306_H
