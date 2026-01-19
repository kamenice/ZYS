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
#include "dht11.h"

/* Cached sensor values */
static float g_temperature = 0.0f;
static float g_humidity = 0.0f;

/* Helper function to set GPIO direction */
static void DHT11_SetOutput(void)
{
    GpioSetDir(DHT11_GPIO, WIFI_IOT_GPIO_DIR_OUT);
}

static void DHT11_SetInput(void)
{
    GpioSetDir(DHT11_GPIO, WIFI_IOT_GPIO_DIR_IN);
}

/* Wait for specified GPIO value with timeout
 * Uses larger sleep intervals initially and falls back to polling for precision
 * This reduces CPU usage while maintaining timing accuracy
 */
static int DHT11_WaitForValue(WifiIotGpioValue expectedValue, uint32_t timeoutUs)
{
    WifiIotGpioValue value;
    uint32_t elapsed = 0;
    const uint32_t pollInterval = 5; /* 5us polling interval */

    while (elapsed < timeoutUs) {
        GpioGetInputVal(DHT11_GPIO, &value);
        if (value == expectedValue) {
            return elapsed;
        }
        usleep(pollInterval);
        elapsed += pollInterval;
    }

    return -1; /* Timeout */
}

void DHT11_Init(void)
{
    /* Initialize GPIO for DHT11 data pin */
    IoSetFunc(DHT11_GPIO, WIFI_IOT_IO_FUNC_GPIO_7_GPIO);
    GpioSetDir(DHT11_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(DHT11_GPIO, WIFI_IOT_GPIO_VALUE1);

    printf("[DHT11] Initialized\r\n");
}

int DHT11_Read(float *temperature, float *humidity)
{
    uint8_t data[5] = {0};
    uint8_t i, j;
    int duration;

    /* Send start signal - pull low for 20ms */
    DHT11_SetOutput();
    GpioSetOutputVal(DHT11_GPIO, WIFI_IOT_GPIO_VALUE0);
    usleep(DHT11_START_LOW_TIME);

    /* Release bus and wait for sensor response */
    GpioSetOutputVal(DHT11_GPIO, WIFI_IOT_GPIO_VALUE1);
    usleep(DHT11_START_HIGH_TIME);

    DHT11_SetInput();

    /* Wait for sensor to pull low (response) */
    if (DHT11_WaitForValue(WIFI_IOT_GPIO_VALUE0, 100) < 0) {
        printf("[DHT11] No response from sensor\r\n");
        return -1;
    }

    /* Wait for sensor to release (80us low) */
    if (DHT11_WaitForValue(WIFI_IOT_GPIO_VALUE1, 100) < 0) {
        printf("[DHT11] Response timeout (low)\r\n");
        return -1;
    }

    /* Wait for sensor to pull low again (80us high) */
    if (DHT11_WaitForValue(WIFI_IOT_GPIO_VALUE0, 100) < 0) {
        printf("[DHT11] Response timeout (high)\r\n");
        return -1;
    }

    /* Read 40 bits (5 bytes) of data */
    for (i = 0; i < 5; i++) {
        for (j = 0; j < 8; j++) {
            /* Wait for bit start (50us low) */
            if (DHT11_WaitForValue(WIFI_IOT_GPIO_VALUE1, 100) < 0) {
                return -1;
            }

            /* Measure high duration to determine bit value */
            duration = DHT11_WaitForValue(WIFI_IOT_GPIO_VALUE0, 100);
            if (duration < 0) {
                return -1;
            }

            /* Shift in the bit - high duration > 50us means '1' */
            data[i] <<= 1;
            if (duration > DHT11_BIT_HIGH_TIME) {
                data[i] |= 1;
            }
        }
    }

    /* Verify checksum */
    if (data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
        printf("[DHT11] Checksum error\r\n");
        return -1;
    }

    /* Update cached values */
    g_humidity = data[0] + data[1] * 0.1f;
    g_temperature = data[2] + data[3] * 0.1f;

    if (humidity != NULL) {
        *humidity = g_humidity;
    }
    if (temperature != NULL) {
        *temperature = g_temperature;
    }

    return 0;
}

float DHT11_GetTemperature(void)
{
    return g_temperature;
}

float DHT11_GetHumidity(void)
{
    return g_humidity;
}

int DHT11_IsOverheated(float threshold)
{
    return (g_temperature > threshold) ? 1 : 0;
}
