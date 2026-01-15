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

#include "buzzer.h"
#include <stdio.h>
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"

/*
 * Active Buzzer Pin Configuration
 * Using GPIO8 for buzzer control
 * Active buzzer is triggered by LOW level
 */
#define BUZZER_GPIO     WIFI_IOT_GPIO_IDX_8

void Buzzer_Init(void)
{
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_8, WIFI_IOT_IO_FUNC_GPIO_8_GPIO);
    GpioSetDir(BUZZER_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    /* Initially off (high level) */
    GpioSetOutputVal(BUZZER_GPIO, WIFI_IOT_GPIO_VALUE1);
    printf("[Buzzer] Initialized on GPIO8 (active low)\n");
}

void Buzzer_On(void)
{
    /* Active buzzer is triggered by LOW level */
    GpioSetOutputVal(BUZZER_GPIO, WIFI_IOT_GPIO_VALUE0);
    printf("[Buzzer] ON\n");
}

void Buzzer_Off(void)
{
    /* Turn off by setting HIGH level */
    GpioSetOutputVal(BUZZER_GPIO, WIFI_IOT_GPIO_VALUE1);
    printf("[Buzzer] OFF\n");
}
