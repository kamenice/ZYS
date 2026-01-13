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

#ifndef HX711_H
#define HX711_H

#include <stdint.h>

/* HX711 GPIO Pin Definitions */
#define HX711_DT_GPIO           WIFI_IOT_GPIO_IDX_11   /* Data pin (GPIO11) */
#define HX711_SCK_GPIO          WIFI_IOT_GPIO_IDX_12   /* Clock pin (GPIO12) */

/* Calibration factor - adjust based on your load cell */
#define HX711_GAP_VALUE         1300

/**
 * @brief Initialize HX711 pressure sensor
 */
void HX711_Init(void);

/**
 * @brief Read raw value from HX711
 * @return Raw 24-bit value from sensor
 */
unsigned long HX711_ReadRaw(void);

/**
 * @brief Get averaged sensor reading
 * @return Averaged raw value
 */
double HX711_GetAveragedReading(void);

/**
 * @brief Calibrate the sensor (set zero point)
 */
void HX711_Calibrate(void);

/**
 * @brief Get weight in grams
 * @return Weight in grams
 */
double HX711_GetWeight(void);

/**
 * @brief Check if item is present on conveyor
 * @return 1 if item detected, 0 otherwise
 */
int HX711_IsItemPresent(void);

/**
 * @brief Check if item is overweight
 * @param threshold Weight threshold in grams
 * @return 1 if overweight, 0 otherwise
 */
int HX711_IsOverweight(double threshold);

#endif /* HX711_H */
