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

/* DHT11 GPIO Pin Definition */
#define DHT11_GPIO              WIFI_IOT_GPIO_IDX_7   /* Data pin (GPIO7) */

/* DHT11 Timing Constants (microseconds) */
#define DHT11_START_LOW_TIME    20000   /* 20ms start signal low */
#define DHT11_START_HIGH_TIME   30      /* 30us release time */
#define DHT11_RESPONSE_WAIT     80      /* 80us response time */
#define DHT11_BIT_HIGH_TIME     50      /* >50us indicates '1' */

/* Default temperature threshold for overheating alert (Celsius) */
#define DHT11_DEFAULT_TEMP_THRESHOLD    60.0f

/**
 * @brief Initialize DHT11 sensor
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
 * @brief Get current temperature
 * @return Temperature in Celsius
 */
float DHT11_GetTemperature(void);

/**
 * @brief Get current humidity
 * @return Humidity in percentage
 */
float DHT11_GetHumidity(void);

/**
 * @brief Check if temperature exceeds threshold
 * @param threshold Temperature threshold in Celsius
 * @return 1 if overheated, 0 otherwise
 */
int DHT11_IsOverheated(float threshold);

#endif /* DHT11_H */
