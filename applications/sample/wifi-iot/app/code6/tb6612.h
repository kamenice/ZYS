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

#ifndef TB6612_H
#define TB6612_H

/**
 * @brief Initialize TB6612 motor driver
 */
void TB6612_Init(void);

/**
 * @brief Start the motor
 */
void TB6612_Start(void);

/**
 * @brief Stop the motor (brake)
 */
void TB6612_Stop(void);

/**
 * @brief Check if motor is running
 * @return 1 if running, 0 if stopped
 */
int TB6612_IsRunning(void);

#endif // TB6612_H
