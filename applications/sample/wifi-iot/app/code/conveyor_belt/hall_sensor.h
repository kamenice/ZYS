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

#ifndef HALL_SENSOR_H
#define HALL_SENSOR_H

#include <stdint.h>

/* 3411 Hall Effect Sensor GPIO Pin Definition */
#define HALL_SENSOR_GPIO        WIFI_IOT_GPIO_IDX_8   /* Digital output pin (GPIO8) */

/* Speed calculation parameters */
#define HALL_PULSES_PER_REV     1       /* Number of magnets on the shaft */
#define SPEED_CALC_INTERVAL     1000    /* Speed calculation interval in milliseconds */

/**
 * @brief Initialize Hall effect sensor (3411)
 */
void HallSensor_Init(void);

/**
 * @brief Get current pulse count
 * @return Number of pulses detected
 */
uint32_t HallSensor_GetPulseCount(void);

/**
 * @brief Reset pulse counter
 */
void HallSensor_ResetPulseCount(void);

/**
 * @brief Get conveyor belt speed in RPM
 * @return Speed in rotations per minute
 */
float HallSensor_GetSpeedRPM(void);

/**
 * @brief Update speed calculation (call periodically)
 */
void HallSensor_UpdateSpeed(void);

#endif /* HALL_SENSOR_H */
