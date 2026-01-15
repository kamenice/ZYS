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

#include "hx711.h"
#include <stdio.h>
#include <unistd.h>
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"

/* HX711 Pin Configuration
 * GPIO0/GPIO2 are reserved for system debugging, cannot use
 * Using GPIO11 as DT (Data), GPIO12 as SCK (Clock)
 */
#define HX711_DT_GPIO   WIFI_IOT_GPIO_IDX_11
#define HX711_SCK_GPIO  WIFI_IOT_GPIO_IDX_12

/* Threshold for item detection (raw value difference from baseline) */
#define ITEM_DETECTION_THRESHOLD 5000

static double g_baselineValue = 0;
static int g_initialized = 0;

void HX711_Init(void)
{
    /* Set DT pin as input */
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_IO_FUNC_GPIO_11_GPIO);
    GpioSetDir(HX711_DT_GPIO, WIFI_IOT_GPIO_DIR_IN);

    /* Set SCK pin as output */
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_12, WIFI_IOT_IO_FUNC_GPIO_12_GPIO);
    GpioSetDir(HX711_SCK_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(HX711_SCK_GPIO, WIFI_IOT_GPIO_VALUE0);

    /* Get baseline value (empty scale) */
    usleep(100000); /* Wait for sensor stabilization */
    g_baselineValue = HX711_GetAverageReading();
    g_initialized = 1;

    printf("[HX711] Initialized, baseline: %.2f\n", g_baselineValue);
}

unsigned long HX711_Read(void)
{
    unsigned long value = 0;
    unsigned char i = 0;
    WifiIotGpioValue input = WIFI_IOT_GPIO_VALUE0;

    usleep(2);

    /* Keep clock line low when idle */
    GpioSetOutputVal(HX711_SCK_GPIO, WIFI_IOT_GPIO_VALUE0);
    usleep(2);

    /* Wait for ADC conversion to complete (DT goes low) */
    GpioGetInputVal(HX711_DT_GPIO, &input);
    while (input == WIFI_IOT_GPIO_VALUE1) {
        GpioGetInputVal(HX711_DT_GPIO, &input);
        usleep(1);
    }

    /* Read 24 bits of data */
    for (i = 0; i < 24; i++) {
        /* Clock high - start sending pulse */
        GpioSetOutputVal(HX711_SCK_GPIO, WIFI_IOT_GPIO_VALUE1);
        usleep(2);

        /* Shift left, prepare to receive data */
        value = value << 1;

        /* Clock low */
        GpioSetOutputVal(HX711_SCK_GPIO, WIFI_IOT_GPIO_VALUE0);
        usleep(2);

        /* Read one bit of data */
        GpioGetInputVal(HX711_DT_GPIO, &input);
        if (input == WIFI_IOT_GPIO_VALUE1) {
            value++;
        }
    }

    /* 25th pulse for gain selection (128 gain) */
    GpioSetOutputVal(HX711_SCK_GPIO, WIFI_IOT_GPIO_VALUE1);
    usleep(2);
    value = value ^ 0x800000;
    GpioSetOutputVal(HX711_SCK_GPIO, WIFI_IOT_GPIO_VALUE0);
    usleep(2);

    return value;
}

double HX711_GetAverageReading(void)
{
    double sum = 0;
    int i;

    /* Take 10 readings and average for better accuracy */
    for (i = 0; i < 10; i++) {
        sum += HX711_Read();
    }

    return sum / 10;
}

int HX711_IsItemDetected(void)
{
    double currentValue;
    double diff;

    if (!g_initialized) {
        return 0;
    }

    currentValue = HX711_GetAverageReading();
    diff = currentValue - g_baselineValue;

    /* Absolute difference */
    if (diff < 0) {
        diff = -diff;
    }

    return (diff > ITEM_DETECTION_THRESHOLD) ? 1 : 0;
}
