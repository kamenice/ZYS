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
#include "dht11.h"

// DHT11 Pin definition - GPIO7
#define DHT11_GPIO      WIFI_IOT_GPIO_IDX_7
#define DHT11_IO_NAME   WIFI_IOT_IO_NAME_GPIO_7

// Temperature threshold (30 degrees Celsius)
#define TEMPERATURE_THRESHOLD 30.0f

static float g_temperature = 0.0f;
static float g_humidity = 0.0f;

static void DHT11_SetOutput(void)
{
    IoSetFunc(DHT11_IO_NAME, WIFI_IOT_IO_FUNC_GPIO_7_GPIO);
    GpioSetDir(DHT11_GPIO, WIFI_IOT_GPIO_DIR_OUT);
}

static void DHT11_SetInput(void)
{
    IoSetFunc(DHT11_IO_NAME, WIFI_IOT_IO_FUNC_GPIO_7_GPIO);
    GpioSetDir(DHT11_GPIO, WIFI_IOT_GPIO_DIR_IN);
}

static int DHT11_WaitForLevel(WifiIotGpioValue level, int timeout_us)
{
    WifiIotGpioValue val;
    int count = 0;

    while (count < timeout_us) {
        GpioGetInputVal(DHT11_GPIO, &val);
        if (val == level) {
            return count;
        }
        usleep(1);
        count++;
    }

    return -1;
}

void DHT11_Init(void)
{
    GpioInit();
    DHT11_SetOutput();
    GpioSetOutputVal(DHT11_GPIO, WIFI_IOT_GPIO_VALUE1);
    printf("[DHT11] Initialized\n");
}

int DHT11_Read(float *temperature, float *humidity)
{
    uint8_t data[5] = {0};
    int i, j;

    // Start signal: pull low for at least 18ms
    DHT11_SetOutput();
    GpioSetOutputVal(DHT11_GPIO, WIFI_IOT_GPIO_VALUE0);
    usleep(20000); // 20ms

    // Pull high and switch to input
    GpioSetOutputVal(DHT11_GPIO, WIFI_IOT_GPIO_VALUE1);
    usleep(30);
    DHT11_SetInput();

    // Wait for DHT11 response (pull low for 80us)
    if (DHT11_WaitForLevel(WIFI_IOT_GPIO_VALUE0, 100) < 0) {
        printf("[DHT11] No response (low)\n");
        return -1;
    }

    // Wait for DHT11 to release (pull high for 80us)
    if (DHT11_WaitForLevel(WIFI_IOT_GPIO_VALUE1, 100) < 0) {
        printf("[DHT11] No response (high)\n");
        return -1;
    }

    // Wait for data transmission start
    if (DHT11_WaitForLevel(WIFI_IOT_GPIO_VALUE0, 100) < 0) {
        printf("[DHT11] No data start\n");
        return -1;
    }

    // Read 40 bits (5 bytes) of data
    for (i = 0; i < 5; i++) {
        for (j = 0; j < 8; j++) {
            // Wait for start of bit (high level)
            if (DHT11_WaitForLevel(WIFI_IOT_GPIO_VALUE1, 100) < 0) {
                return -1;
            }

            // Measure high level duration
            usleep(30);
            WifiIotGpioValue val;
            GpioGetInputVal(DHT11_GPIO, &val);

            data[i] <<= 1;
            if (val == WIFI_IOT_GPIO_VALUE1) {
                data[i] |= 1;
                // Wait for end of bit
                DHT11_WaitForLevel(WIFI_IOT_GPIO_VALUE0, 100);
            }
        }
    }

    // Verify checksum
    if (data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
        printf("[DHT11] Checksum error\n");
        return -1;
    }

    // Parse data (DHT11 format: integer only)
    g_humidity = (float)data[0] + (float)data[1] / 10.0f;
    g_temperature = (float)data[2] + (float)data[3] / 10.0f;

    if (temperature) {
        *temperature = g_temperature;
    }
    if (humidity) {
        *humidity = g_humidity;
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

int DHT11_IsOverTemperature(void)
{
    return (g_temperature >= TEMPERATURE_THRESHOLD) ? 1 : 0;
}
