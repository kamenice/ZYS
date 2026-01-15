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

#include "infrared.h"
#include <stdio.h>
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"

/*
 * YL-62 Infrared Sensor Pin Configuration
 * Using GPIO10 for infrared sensor input
 * YL-62 outputs LOW when obstacle is detected
 */
#define INFRARED_GPIO   WIFI_IOT_GPIO_IDX_10

void Infrared_Init(void)
{
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_10, WIFI_IOT_IO_FUNC_GPIO_10_GPIO);
    GpioSetDir(INFRARED_GPIO, WIFI_IOT_GPIO_DIR_IN);
    IoSetPull(WIFI_IOT_IO_NAME_GPIO_10, WIFI_IOT_IO_PULL_UP);
    printf("[Infrared] YL-62 initialized on GPIO10\n");
}

int Infrared_IsObstacleDetected(void)
{
    WifiIotGpioValue val;
    GpioGetInputVal(INFRARED_GPIO, &val);

    /* YL-62 outputs LOW when obstacle is detected */
    return (val == WIFI_IOT_GPIO_VALUE0) ? 1 : 0;
}
