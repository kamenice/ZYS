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
#include "wifiiot_errno.h"
#include "infrared.h"

/* Jam detection counter */
static int g_jamCounter = 0;

void Infrared_Init(void)
{
    /* Initialize GPIO for YL-62 infrared sensor (digital input) */
    IoSetFunc(INFRARED_GPIO, WIFI_IOT_IO_FUNC_GPIO_8_GPIO);
    GpioSetDir(INFRARED_GPIO, WIFI_IOT_GPIO_DIR_IN);

    g_jamCounter = 0;

    printf("[Infrared] YL-62 sensor initialized\r\n");
}

int Infrared_IsObstacleDetected(void)
{
    WifiIotGpioValue value;
    GpioGetInputVal(INFRARED_GPIO, &value);

    /* YL-62 outputs LOW when obstacle is detected */
    return (value == WIFI_IOT_GPIO_VALUE0) ? 1 : 0;
}

int Infrared_IsJammed(void)
{
    /* Check if obstacle is continuously detected */
    if (Infrared_IsObstacleDetected()) {
        g_jamCounter++;
        if (g_jamCounter >= JAM_DETECTION_COUNT) {
            return 1; /* Jam confirmed */
        }
    } else {
        /* Reset counter if no obstacle */
        g_jamCounter = 0;
    }

    return 0;
}

void Infrared_ResetJamDetection(void)
{
    g_jamCounter = 0;
}
