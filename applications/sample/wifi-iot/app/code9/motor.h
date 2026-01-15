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

/**
 * @brief Motor state enumeration
 */
typedef enum {
    MOTOR_STATE_STOPPED = 0,
    MOTOR_STATE_RUNNING = 1
} MotorState;

/**
 * @brief Initialize TB6612 motor driver
 * TB6612 working principle:
 * - AIN1/AIN2 same level: motor works
 * - AIN1/AIN2 different level: motor brakes
 * - PWMA high: motor works
 * - STBY high: chip works, low: chip doesn't work
 */
void Motor_Init(void);

/**
 * @brief Start the motor
 */
void Motor_Start(void);

/**
 * @brief Stop the motor
 */
void Motor_Stop(void);

/**
 * @brief Get current motor state
 * @return Current motor state (MOTOR_STATE_STOPPED or MOTOR_STATE_RUNNING)
 */
MotorState Motor_GetState(void);

#endif /* MOTOR_H */
