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

#include "dht11.h"
#include <stdio.h>
#include <unistd.h>
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"

/* DHT11 Pin Configuration
 * Using GPIO7 for DHT11 data line
 */
#define DHT11_GPIO      WIFI_IOT_GPIO_IDX_7
#define DHT11_IO_NAME   WIFI_IOT_IO_NAME_GPIO_7

/* Temperature threshold for alarm */
#define TEMPERATURE_THRESHOLD 30.0f

static float g_temperature = 0.0f;
static float g_humidity = 0.0f;

/* Set GPIO as output mode */
static void DHT11_SetOutput(void)
{
    GpioSetDir(DHT11_GPIO, WIFI_IOT_GPIO_DIR_OUT);
}

/* Set GPIO as input mode */
static void DHT11_SetInput(void)
{
    GpioSetDir(DHT11_GPIO, WIFI_IOT_GPIO_DIR_IN);
}

/* Write value to GPIO */
static void DHT11_Write(WifiIotGpioValue val)
{
    GpioSetOutputVal(DHT11_GPIO, val);
}

/* Read value from GPIO */
static WifiIotGpioValue DHT11_ReadPin(void)
{
    WifiIotGpioValue val;
    GpioGetInputVal(DHT11_GPIO, &val);
    return val;
}

void DHT11_Init(void)
{
    IoSetFunc(DHT11_IO_NAME, WIFI_IOT_IO_FUNC_GPIO_7_GPIO);
    GpioSetDir(DHT11_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(DHT11_GPIO, WIFI_IOT_GPIO_VALUE1);
    printf("[DHT11] Initialized on GPIO7\n");
}

int DHT11_Read(float *temperature, float *humidity)
{
    uint8_t data[5] = {0};
    uint8_t i, j;
    uint32_t timeout;

    /* Start signal: pull low for at least 18ms */
    DHT11_SetOutput();
    DHT11_Write(WIFI_IOT_GPIO_VALUE0);
    usleep(20000); /* 20ms */

    /* Pull high for 20-40us */
    DHT11_Write(WIFI_IOT_GPIO_VALUE1);
    usleep(30);

    /* Switch to input mode to read response */
    DHT11_SetInput();

    /* Wait for DHT11 response (low for 80us) */
    timeout = 100;
    while (DHT11_ReadPin() == WIFI_IOT_GPIO_VALUE1 && timeout > 0) {
        usleep(1);
        timeout--;
    }
    if (timeout == 0) {
        printf("[DHT11] No response (timeout waiting for low)\n");
        return -1;
    }

    timeout = 100;
    while (DHT11_ReadPin() == WIFI_IOT_GPIO_VALUE0 && timeout > 0) {
        usleep(1);
        timeout--;
    }
    if (timeout == 0) {
        printf("[DHT11] Response low timeout\n");
        return -1;
    }

    /* Wait for response high (80us) to end */
    timeout = 100;
    while (DHT11_ReadPin() == WIFI_IOT_GPIO_VALUE1 && timeout > 0) {
        usleep(1);
        timeout--;
    }
    if (timeout == 0) {
        printf("[DHT11] Response high timeout\n");
        return -1;
    }

    /* Read 40 bits (5 bytes) of data */
    for (i = 0; i < 5; i++) {
        for (j = 0; j < 8; j++) {
            /* Wait for low level to end (50us) */
            timeout = 100;
            while (DHT11_ReadPin() == WIFI_IOT_GPIO_VALUE0 && timeout > 0) {
                usleep(1);
                timeout--;
            }

            /* Measure high level duration */
            usleep(30); /* 30us: if still high, it's '1', otherwise '0' */

            data[i] <<= 1;
            if (DHT11_ReadPin() == WIFI_IOT_GPIO_VALUE1) {
                data[i] |= 1;
                /* Wait for high level to end */
                timeout = 100;
                while (DHT11_ReadPin() == WIFI_IOT_GPIO_VALUE1 && timeout > 0) {
                    usleep(1);
                    timeout--;
                }
            }
        }
    }

    /* Verify checksum */
    if (data[4] != (uint8_t)(data[0] + data[1] + data[2] + data[3])) {
        printf("[DHT11] Checksum error\n");
        return -1;
    }

    /* Parse data: DHT11 returns integer values only */
    g_humidity = (float)data[0];
    g_temperature = (float)data[2];

    if (humidity != NULL) {
        *humidity = g_humidity;
    }
    if (temperature != NULL) {
        *temperature = g_temperature;
    }

    return 0;
}

int DHT11_IsOverTemperature(void)
{
    return (g_temperature >= TEMPERATURE_THRESHOLD) ? 1 : 0;
}

float DHT11_GetTemperature(void)
{
    return g_temperature;
}
