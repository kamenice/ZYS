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

#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <stdint.h>

/* OLED I2C Configuration */
#define OLED_I2C_IDX            0       /* I2C0 */
#define OLED_I2C_ADDR           0x78    /* SSD1306 I2C address */
#define OLED_WIDTH              128
#define OLED_HEIGHT             64

/* Display line definitions for status information */
#define DISPLAY_LINE_WEIGHT     0       /* Line for weight display */
#define DISPLAY_LINE_STATUS     2       /* Line for conveyor status */
#define DISPLAY_LINE_TEMP       4       /* Line for temperature */
#define DISPLAY_LINE_SPEED      6       /* Line for speed and time */

/**
 * @brief Initialize OLED display
 */
void OLED_DisplayInit(void);

/**
 * @brief Clear OLED display
 */
void OLED_DisplayClear(void);

/**
 * @brief Display weight information
 * @param weight Weight in grams
 * @param isOverweight 1 if overweight, 0 otherwise
 */
void OLED_DisplayWeight(double weight, int isOverweight);

/**
 * @brief Display conveyor belt status
 * @param isRunning 1 if running, 0 if stopped
 * @param isJammed 1 if jammed, 0 otherwise
 */
void OLED_DisplayConveyorStatus(int isRunning, int isJammed);

/**
 * @brief Display temperature information
 * @param temperature Temperature in Celsius
 * @param isOverheated 1 if overheated, 0 otherwise
 */
void OLED_DisplayTemperature(float temperature, int isOverheated);

/**
 * @brief Display speed and running time
 * @param speedRPM Speed in RPM
 * @param runTimeSeconds Running time in seconds
 */
void OLED_DisplaySpeedAndTime(float speedRPM, uint32_t runTimeSeconds);

/**
 * @brief Update all display information
 * @param weight Weight in grams
 * @param isOverweight 1 if overweight
 * @param isRunning 1 if motor running
 * @param isJammed 1 if jammed
 * @param temperature Temperature in Celsius
 * @param isOverheated 1 if overheated
 * @param speedRPM Speed in RPM
 * @param runTimeSeconds Running time in seconds
 */
void OLED_DisplayUpdate(double weight, int isOverweight,
                        int isRunning, int isJammed,
                        float temperature, int isOverheated,
                        float speedRPM, uint32_t runTimeSeconds);

#endif /* OLED_DISPLAY_H */
