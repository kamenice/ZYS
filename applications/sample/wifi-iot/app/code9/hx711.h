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

/**
 * @brief Initialize HX711 pressure sensor
 */
void HX711_Init(void);

/**
 * @brief Read HX711 sensor value
 * @return Raw sensor value
 */
unsigned long HX711_Read(void);

/**
 * @brief Check if item is detected on the sensor
 * @return 1 if item detected, 0 otherwise
 */
int HX711_IsItemDetected(void);

/**
 * @brief Get averaged sensor reading
 * @return Averaged sensor value
 */
double HX711_GetAverageReading(void);

#endif /* HX711_H */
