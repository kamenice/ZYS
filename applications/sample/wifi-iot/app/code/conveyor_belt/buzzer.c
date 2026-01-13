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

#include <stdio.h>
#include <unistd.h>
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_pwm.h"
#include "wifiiot_errno.h"
#include "buzzer.h"

/* Buzzer state */
static int g_isMuted = 0;
static int g_alarmActive = 0;

void Buzzer_Init(void)
{
    /* Initialize PWM pin for buzzer */
    IoSetFunc(BUZZER_GPIO, WIFI_IOT_IO_FUNC_GPIO_9_PWM0_OUT);
    PwmInit(BUZZER_PWM_PORT);

    g_isMuted = 0;
    g_alarmActive = 0;

    printf("[Buzzer] Initialized\r\n");
}

void Buzzer_On(void)
{
    if (g_isMuted) {
        return;
    }

    PwmStart(BUZZER_PWM_PORT, BUZZER_DUTY, BUZZER_FREQ);
}

void Buzzer_Off(void)
{
    PwmStop(BUZZER_PWM_PORT);
}

void Buzzer_Alarm(uint8_t pattern)
{
    if (g_isMuted) {
        return;
    }

    g_alarmActive = 1;

    switch (pattern) {
        case ALARM_CONTINUOUS:
            Buzzer_On();
            break;

        case ALARM_INTERMITTENT:
            /* Simple on-off pattern */
            Buzzer_On();
            usleep(500000);
            Buzzer_Off();
            usleep(500000);
            break;

        case ALARM_OVERWEIGHT_PATTERN:
            /* Rapid beeping for overweight alert */
            for (int i = 0; i < 3 && !g_isMuted; i++) {
                Buzzer_On();
                usleep(100000);
                Buzzer_Off();
                usleep(100000);
            }
            break;

        case ALARM_OVERHEAT_PATTERN:
            /* Slow beeping for overheat alert */
            Buzzer_On();
            usleep(1000000);
            Buzzer_Off();
            usleep(1000000);
            break;

        default:
            Buzzer_On();
            break;
    }
}

void Buzzer_StopAlarm(void)
{
    g_alarmActive = 0;
    Buzzer_Off();
}

void Buzzer_Mute(void)
{
    g_isMuted = 1;
    Buzzer_Off();
    printf("[Buzzer] Muted\r\n");
}

void Buzzer_Unmute(void)
{
    g_isMuted = 0;
    printf("[Buzzer] Unmuted\r\n");
}

int Buzzer_IsMuted(void)
{
    return g_isMuted;
}

void Buzzer_Beep(uint16_t duration)
{
    if (g_isMuted) {
        return;
    }

    Buzzer_On();
    usleep(duration * 1000);
    Buzzer_Off();
}
