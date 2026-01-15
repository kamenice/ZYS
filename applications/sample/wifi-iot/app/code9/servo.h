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
 * @brief Initialize servo motor with PWM
 * Servo requires 20ms period with 0.5~2.5ms high pulse
 * 0.5ms -> 0 degrees
 * 1.5ms -> 90 degrees
 * 2.5ms -> 180 degrees
 */
void Servo_Init(void);

/**
 * @brief Set servo angle
 * @param angle Angle in degrees (0-180)
 */
void Servo_SetAngle(int angle);

/**
 * @brief Move servo to open position (90 degrees)
 */
void Servo_Open(void);

/**
 * @brief Move servo to closed position (0 degrees)
 */
void Servo_Close(void);

#endif /* SERVO_H */
