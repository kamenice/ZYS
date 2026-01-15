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
#include "infrared.h"

/*
 * YL-62 Infrared Sensor Configuration:
 * - Uses GPIO10 for detection signal
 * - Outputs LOW level when object is detected
 * - Outputs HIGH level when no object
 */

#define INFRARED_GPIO       WIFI_IOT_GPIO_IDX_10
#define INFRARED_IO_NAME    WIFI_IOT_IO_NAME_GPIO_10

void Infrared_Init(void)
{
    // Initialize GPIO10 as input
    IoSetFunc(INFRARED_IO_NAME, WIFI_IOT_IO_FUNC_GPIO_10_GPIO);
    GpioSetDir(INFRARED_GPIO, WIFI_IOT_GPIO_DIR_IN);

    printf("[Infrared YL-62] Initialized\n");
}

int Infrared_IsObjectDetected(void)
{
    WifiIotGpioValue val;
    GpioGetInputVal(INFRARED_GPIO, &val);

    // Low level = object detected
    return (val == WIFI_IOT_GPIO_VALUE0) ? 1 : 0;
}
