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
#include "hx711.h"

/* Base value for zero calibration */
static double g_baseValue = 0;

void HX711_Init(void)
{
    /* Initialize GPIO for HX711 DT pin (data input) */
    IoSetFunc(HX711_DT_GPIO, WIFI_IOT_IO_FUNC_GPIO_11_GPIO);
    GpioSetDir(HX711_DT_GPIO, WIFI_IOT_GPIO_DIR_IN);

    /* Initialize GPIO for HX711 SCK pin (clock output) */
    IoSetFunc(HX711_SCK_GPIO, WIFI_IOT_IO_FUNC_GPIO_12_GPIO);
    GpioSetDir(HX711_SCK_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(HX711_SCK_GPIO, WIFI_IOT_GPIO_VALUE0);

    printf("[HX711] Initialized\r\n");
}

unsigned long HX711_ReadRaw(void)
{
    unsigned long value = 0;
    unsigned char i = 0;
    WifiIotGpioValue input = WIFI_IOT_GPIO_VALUE0;

    usleep(2);

    /* Keep clock low in idle state */
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
        /* Clock high - start sending clock pulse */
        GpioSetOutputVal(HX711_SCK_GPIO, WIFI_IOT_GPIO_VALUE1);
        usleep(2);

        /* Shift left to make room for new bit */
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

    /* 25th pulse - set gain to 128 for channel A */
    GpioSetOutputVal(HX711_SCK_GPIO, WIFI_IOT_GPIO_VALUE1);
    usleep(2);

    /* Convert from 2's complement */
    value = value ^ 0x800000;

    /* End 25th pulse */
    GpioSetOutputVal(HX711_SCK_GPIO, WIFI_IOT_GPIO_VALUE0);
    usleep(2);

    return value;
}

double HX711_GetAveragedReading(void)
{
    double sum = 0;
    int samples = 10;

    /* Take multiple samples and average to reduce noise */
    for (int i = 0; i < samples; i++) {
        sum += HX711_ReadRaw();
    }

    return sum / samples;
}

void HX711_Calibrate(void)
{
    printf("[HX711] Calibrating... Please ensure scale is empty.\r\n");
    usleep(1000000); /* Wait 1 second for stable reading */
    g_baseValue = HX711_GetAveragedReading();
    printf("[HX711] Calibration complete. Base value: %.2f\r\n", g_baseValue);
}

double HX711_GetWeight(void)
{
    double currentValue = HX711_GetAveragedReading();
    double weight = (currentValue - g_baseValue) / HX711_GAP_VALUE;

    /* Ignore negative weights (noise) */
    if (weight < 0) {
        weight = 0;
    }

    return weight;
}

int HX711_IsItemPresent(void)
{
    double weight = HX711_GetWeight();
    /* Consider item present if weight > 10g */
    return (weight > 10.0) ? 1 : 0;
}

int HX711_IsOverweight(double threshold)
{
    double weight = HX711_GetWeight();
    return (weight > threshold) ? 1 : 0;
}
