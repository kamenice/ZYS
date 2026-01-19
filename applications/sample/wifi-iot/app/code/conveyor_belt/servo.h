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

#ifndef SERVO_H
#define SERVO_H

#include <stdint.h>

/* SG90 Servo GPIO Pin Definition */
#define SERVO_GPIO              WIFI_IOT_GPIO_IDX_5   /* PWM pin (GPIO5/PWM3) */
#define SERVO_PWM_PORT          WIFI_IOT_PWM_PORT_PWM3

/* SG90 Servo PWM Parameters
 * SG90 operates at 50Hz (20ms period)
 * Pulse width: 0.5ms-2.5ms for 0-180 degrees
 * Using 160MHz clock with divisor
 */
#define SERVO_PWM_FREQ          64000   /* 160MHz / 64000 = 2500Hz base, then div by 50 = 50Hz */
#define SERVO_MIN_DUTY          1600    /* 0.5ms pulse (0 degrees) */
#define SERVO_MAX_DUTY          8000    /* 2.5ms pulse (180 degrees) */
#define SERVO_CENTER_DUTY       4800    /* 1.5ms pulse (90 degrees) */

/* Vibration parameters */
#define SERVO_VIBRATION_COUNT   5       /* Number of vibration cycles */
#define SERVO_VIBRATION_DELAY   200     /* Delay between vibrations in ms */

/**
 * @brief Initialize SG90 servo motor
 */
void Servo_Init(void);

/**
 * @brief Set servo angle
 * @param angle Angle in degrees (0-180)
 */
void Servo_SetAngle(uint8_t angle);

/**
 * @brief Perform vibration to clear jammed items
 */
void Servo_Vibrate(void);

/**
 * @brief Move servo to center position
 */
void Servo_Center(void);

/**
 * @brief Stop servo PWM output
 */
void Servo_Stop(void);

#endif /* SERVO_H */
