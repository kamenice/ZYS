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

#ifndef DHT11_H
#define DHT11_H

#include <stdint.h>

/**
 * @brief Initialize DHT11 temperature sensor
 */
void DHT11_Init(void);

/**
 * @brief Read temperature and humidity from DHT11
 * @param temperature Pointer to store temperature value
 * @param humidity Pointer to store humidity value
 * @return 0 on success, -1 on failure
 */
int DHT11_Read(float *temperature, float *humidity);

/**
 * @brief Get the last read temperature value
 * @return Temperature in Celsius
 */
float DHT11_GetTemperature(void);

/**
 * @brief Get the last read humidity value
 * @return Humidity in percentage
 */
float DHT11_GetHumidity(void);

/**
 * @brief Check if temperature exceeds threshold (30 degrees)
 * @return 1 if over temperature, 0 otherwise
 */
int DHT11_IsOverTemperature(void);

#endif // DHT11_H
