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
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "buzzer.h"

/*
 * Active Buzzer Configuration:
 * - Uses GPIO8 for control
 * - Low level triggers the buzzer (active low)
 * - High level turns off the buzzer
 */

#define BUZZER_GPIO     WIFI_IOT_GPIO_IDX_8
#define BUZZER_IO_NAME  WIFI_IOT_IO_NAME_GPIO_8

static int g_buzzer_on = 0;

void Buzzer_Init(void)
{
    // Initialize GPIO8 as output
    IoSetFunc(BUZZER_IO_NAME, WIFI_IOT_IO_FUNC_GPIO_8_GPIO);
    GpioSetDir(BUZZER_GPIO, WIFI_IOT_GPIO_DIR_OUT);

    // Turn off buzzer initially (high level = off for active low buzzer)
    GpioSetOutputVal(BUZZER_GPIO, WIFI_IOT_GPIO_VALUE1);
    g_buzzer_on = 0;

    printf("[Buzzer] Initialized (active low)\n");
}

void Buzzer_On(void)
{
    // Low level triggers active buzzer
    GpioSetOutputVal(BUZZER_GPIO, WIFI_IOT_GPIO_VALUE0);
    g_buzzer_on = 1;
    printf("[Buzzer] ON\n");
}

void Buzzer_Off(void)
{
    // High level turns off active buzzer
    GpioSetOutputVal(BUZZER_GPIO, WIFI_IOT_GPIO_VALUE1);
    g_buzzer_on = 0;
    printf("[Buzzer] OFF\n");
}

int Buzzer_IsOn(void)
{
    return g_buzzer_on;
}
