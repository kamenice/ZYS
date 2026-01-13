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

#include <stdint.h>

/* Buzzer GPIO Pin Definition */
#define BUZZER_GPIO             WIFI_IOT_GPIO_IDX_9   /* GPIO9 for active buzzer */

/* Active buzzer configuration
 * Active buzzer has built-in oscillator, only needs DC power
 * This buzzer is LOW level triggered (low = ON, high = OFF)
 */
#define BUZZER_ACTIVE_LOW       1       /* 1 = low level trigger, 0 = high level trigger */

/* Alarm patterns */
#define ALARM_CONTINUOUS        0       /* Continuous beep */
#define ALARM_INTERMITTENT      1       /* On-off pattern */
#define ALARM_OVERWEIGHT_PATTERN    2   /* Rapid beeping for overweight */
#define ALARM_OVERHEAT_PATTERN      3   /* Slow beeping for overheat */

/**
 * @brief Initialize buzzer
 */
void Buzzer_Init(void);

/**
 * @brief Turn buzzer on
 */
void Buzzer_On(void);

/**
 * @brief Turn buzzer off
 */
void Buzzer_Off(void);

/**
 * @brief Play alarm with specific pattern
 * @param pattern Alarm pattern type
 */
void Buzzer_Alarm(uint8_t pattern);

/**
 * @brief Stop any ongoing alarm
 */
void Buzzer_StopAlarm(void);

/**
 * @brief Mute buzzer (silent mode)
 */
void Buzzer_Mute(void);

/**
 * @brief Unmute buzzer
 */
void Buzzer_Unmute(void);

/**
 * @brief Check if buzzer is muted
 * @return 1 if muted, 0 otherwise
 */
int Buzzer_IsMuted(void);

/**
 * @brief Beep once for short duration
 * @param duration Duration in milliseconds
 */
void Buzzer_Beep(uint16_t duration);

#endif /* BUZZER_H */
