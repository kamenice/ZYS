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

#ifndef BUZZER_H
#define BUZZER_H

/**
 * @brief Initialize active buzzer (low-level trigger)
 */
void Buzzer_Init(void);

/**
 * @brief Turn on the buzzer (by pulling GPIO low)
 */
void Buzzer_On(void);

/**
 * @brief Turn off the buzzer (by pulling GPIO high)
 */
void Buzzer_Off(void);

/**
 * @brief Check if buzzer is on
 * @return 1 if on, 0 if off
 */
int Buzzer_IsOn(void);

#endif // BUZZER_H
