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

/**
 * @brief Initialize servo motor
 */
void Servo_Init(void);

/**
 * @brief Set servo to a specific angle
 * @param angle Angle in degrees (0-180)
 *
 * PWM signal for servo:
 * - Period: 20ms (50Hz)
 * - 0.5ms high = 0 degrees
 * - 1.5ms high = 90 degrees
 * - 2.5ms high = 180 degrees
 */
void Servo_SetAngle(int angle);

/**
 * @brief Activate servo (move to operational position)
 */
void Servo_Activate(void);

/**
 * @brief Deactivate servo (move to rest position)
 */
void Servo_Deactivate(void);

#endif // SERVO_H
