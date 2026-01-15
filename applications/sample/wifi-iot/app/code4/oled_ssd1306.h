/*
 * Copyright (c) 2020 HiHope Community.
 * Description: OLED SSD1306 Display Driver Header
 */

#ifndef OLED_SSD1306_H
#define OLED_SSD1306_H

#include <stdint.h>

#define ARRAY_SIZE(a) sizeof(a) / sizeof(a[0])

#define OLED_I2C_IDX WIFI_IOT_I2C_IDX_0

#define OLED_WIDTH (128)
#define OLED_I2C_ADDR 0x78             /* Default address 0x78 */
#define OLED_I2C_CMD 0x00              /* Write command */
#define OLED_I2C_DATA 0x40             /* Write data */
#define OLED_I2C_BAUDRATE (400 * 1000) /* 400kHz */

#define DELAY_100_MS (100 * 1000)

/**
 * @brief Initialize SSD1306 OLED display
 * @return WIFI_IOT_SUCCESS on success
 */
uint32_t OledInit(void);

/**
 * @brief Set cursor position
 * @param x Horizontal position
 * @param y Vertical position (page)
 */
void OledSetPosition(uint8_t x, uint8_t y);

/**
 * @brief Fill entire screen with specified data
 * @param fillData Data to fill (0x00 for clear, 0xFF for all on)
 */
void OledFillScreen(uint8_t fillData);

enum Font
{
    FONT6x8 = 1,
    FONT8x16
};
typedef enum Font Font;

/**
 * @brief Display a single character
 * @param x X position
 * @param y Y position (page)
 * @param ch Character to display
 * @param font Font size
 */
void OledShowChar(uint8_t x, uint8_t y, uint8_t ch, Font font);

/**
 * @brief Display a string
 * @param x X position
 * @param y Y position (page)
 * @param str String to display
 * @param font Font size
 */
void OledShowString(uint8_t x, uint8_t y, const char *str, Font font);

#endif /* OLED_SSD1306_H */
