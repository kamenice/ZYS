/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OLED_SSD1306_H
#define OLED_SSD1306_H

#include <stdint.h>

/**
 * @brief Font size enumeration
 */
typedef enum {
    FONT6x8 = 1,
    FONT8x16
} Font;

/**
 * @brief Initialize SSD1306 OLED display via I2C
 * @return 0 on success, error code otherwise
 */
uint32_t OledInit(void);

/**
 * @brief Set cursor position
 * @param x Horizontal position (0-127)
 * @param y Vertical position (0-7, page number)
 */
void OledSetPosition(uint8_t x, uint8_t y);

/**
 * @brief Fill entire screen with data
 * @param fillData Data to fill (0x00 for clear, 0xFF for fill)
 */
void OledFillScreen(uint8_t fillData);

/**
 * @brief Display a single character
 * @param x Horizontal position
 * @param y Vertical position (page)
 * @param ch Character to display
 * @param font Font size
 */
void OledShowChar(uint8_t x, uint8_t y, uint8_t ch, Font font);

/**
 * @brief Display a string
 * @param x Horizontal position
 * @param y Vertical position (page)
 * @param str String to display
 * @param font Font size
 */
void OledShowString(uint8_t x, uint8_t y, const char* str, Font font);

#endif /* OLED_SSD1306_H */
