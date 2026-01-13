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

#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

/* TB6612FNG Motor Driver GPIO Pin Definitions */
#define MOTOR_PWMA_GPIO         WIFI_IOT_GPIO_IDX_10  /* PWM pin (GPIO10/PWM1) */
#define MOTOR_AIN1_GPIO         WIFI_IOT_GPIO_IDX_6   /* Direction pin 1 (GPIO6) */
#define MOTOR_AIN2_GPIO         WIFI_IOT_GPIO_IDX_0   /* Direction pin 2 (GPIO0) */
#define MOTOR_STBY_GPIO         WIFI_IOT_GPIO_IDX_1   /* Standby pin (GPIO1) */

/* Motor PWM configuration */
#define MOTOR_PWM_PORT          WIFI_IOT_PWM_PORT_PWM1
#define MOTOR_PWM_FREQ          40000   /* PWM frequency divisor */
#define MOTOR_DEFAULT_SPEED     50      /* Default speed percentage (0-100) */

/**
 * @brief Initialize motor driver (TB6612FNG)
 */
void Motor_Init(void);

/**
 * @brief Start the conveyor belt motor
 */
void Motor_Start(void);

/**
 * @brief Stop the conveyor belt motor
 */
void Motor_Stop(void);

/**
 * @brief Set motor speed
 * @param speed Speed percentage (0-100)
 */
void Motor_SetSpeed(uint8_t speed);

/**
 * @brief Get current motor speed setting
 * @return Speed percentage (0-100)
 */
uint8_t Motor_GetSpeed(void);

/**
 * @brief Check if motor is running
 * @return 1 if running, 0 if stopped
 */
int Motor_IsRunning(void);

/**
 * @brief Enable/disable motor standby mode
 * @param enable 1 to enable standby, 0 to disable
 */
void Motor_SetStandby(int enable);

#endif /* MOTOR_H */
