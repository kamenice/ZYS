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
#include "hx711.h"

// HX711 Pin definitions
// DT (Data) pin - GPIO11
// SCK (Clock) pin - GPIO12
#define HX711_DT_GPIO   WIFI_IOT_GPIO_IDX_11
#define HX711_SCK_GPIO  WIFI_IOT_GPIO_IDX_12

// Detection threshold - difference from base value indicates object presence
#define HX711_DETECTION_THRESHOLD 50000

static double g_hx711_base_value = 0;
static int g_hx711_initialized = 0;

void HX711_Init(void)
{
    // Initialize GPIO11 as input for DT (data) pin
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_IO_FUNC_GPIO_11_GPIO);
    GpioSetDir(HX711_DT_GPIO, WIFI_IOT_GPIO_DIR_IN);

    // Initialize GPIO12 as output for SCK (clock) pin
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_12, WIFI_IOT_IO_FUNC_GPIO_12_GPIO);
    GpioSetDir(HX711_SCK_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(HX711_SCK_GPIO, WIFI_IOT_GPIO_VALUE0);

    // Wait for sensor to stabilize
    usleep(100000);

    // Get base value (no object on sensor)
    g_hx711_base_value = HX711_GetAverageRead();
    g_hx711_initialized = 1;

    printf("[HX711] Initialized, base value: %.2f\n", g_hx711_base_value);
}

unsigned long HX711_Read(void)
{
    unsigned long value = 0;
    unsigned char i = 0;
    WifiIotGpioValue input = 0;

    usleep(2);
    // Clock line low - idle state
    GpioSetOutputVal(HX711_SCK_GPIO, WIFI_IOT_GPIO_VALUE0);
    usleep(2);

    // Wait for AD conversion to complete (DT goes low)
    GpioGetInputVal(HX711_DT_GPIO, &input);
    while (input) {
        GpioGetInputVal(HX711_DT_GPIO, &input);
        usleep(1);
    }

    // Read 24 bits of data
    for (i = 0; i < 24; i++) {
        // Clock high - start sending clock pulse
        GpioSetOutputVal(HX711_SCK_GPIO, WIFI_IOT_GPIO_VALUE1);
        usleep(2);

        // Shift left to make room for new bit
        value = value << 1;

        // Clock low
        GpioSetOutputVal(HX711_SCK_GPIO, WIFI_IOT_GPIO_VALUE0);
        usleep(2);

        // Read one bit of data
        GpioGetInputVal(HX711_DT_GPIO, &input);
        if (input) {
            value++;
        }
    }

    // 25th pulse - set gain for next conversion (gain = 128, channel A)
    GpioSetOutputVal(HX711_SCK_GPIO, WIFI_IOT_GPIO_VALUE1);
    usleep(2);
    value = value ^ 0x800000;
    GpioSetOutputVal(HX711_SCK_GPIO, WIFI_IOT_GPIO_VALUE0);
    usleep(2);

    return value;
}

double HX711_GetAverageRead(void)
{
    double sum = 0;
    int count = 10;

    for (int i = 0; i < count; i++) {
        sum += HX711_Read();
    }

    return sum / count;
}

int HX711_IsObjectDetected(void)
{
    if (!g_hx711_initialized) {
        return 0;
    }

    double current_value = HX711_GetAverageRead();
    double diff = current_value - g_hx711_base_value;

    // If difference exceeds threshold, object is detected
    if (diff < 0) {
        diff = -diff; // absolute value
    }

    return (diff > HX711_DETECTION_THRESHOLD) ? 1 : 0;
}
