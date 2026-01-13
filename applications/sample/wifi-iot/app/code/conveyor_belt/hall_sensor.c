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
#include "hall_sensor.h"

/* Pulse counter - updated by interrupt */
static volatile uint32_t g_pulseCount = 0;
static volatile uint32_t g_lastPulseCount = 0;
static float g_speedRPM = 0.0f;

/* Hall sensor interrupt callback */
static void HallSensor_ISR(char *arg)
{
    (void)arg;
    g_pulseCount++;
}

void HallSensor_Init(void)
{
    /* Initialize GPIO for 3411 Hall effect sensor */
    IoSetFunc(HALL_SENSOR_GPIO, WIFI_IOT_IO_FUNC_GPIO_8_GPIO);
    GpioSetDir(HALL_SENSOR_GPIO, WIFI_IOT_GPIO_DIR_IN);

    /* Register interrupt for rising edge (magnet passes) */
    GpioRegisterIsrFunc(HALL_SENSOR_GPIO,
                        WIFI_IOT_INT_TYPE_EDGE,
                        WIFI_IOT_GPIO_EDGE_RISE_LEVEL_HIGH,
                        HallSensor_ISR,
                        NULL);

    g_pulseCount = 0;
    g_lastPulseCount = 0;
    g_speedRPM = 0.0f;

    printf("[Hall Sensor] 3411 sensor initialized\r\n");
}

uint32_t HallSensor_GetPulseCount(void)
{
    return g_pulseCount;
}

void HallSensor_ResetPulseCount(void)
{
    g_pulseCount = 0;
    g_lastPulseCount = 0;
}

float HallSensor_GetSpeedRPM(void)
{
    return g_speedRPM;
}

void HallSensor_UpdateSpeed(void)
{
    /* Calculate speed based on pulse count difference
     * RPM = (pulses per second) * 60 / (pulses per revolution)
     * Assuming this function is called every SPEED_CALC_INTERVAL ms
     */
    uint32_t currentPulses = g_pulseCount;
    uint32_t pulseDiff = currentPulses - g_lastPulseCount;
    g_lastPulseCount = currentPulses;

    /* Calculate RPM: pulses * (1000/interval) * 60 / PULSES_PER_REV */
    g_speedRPM = (float)pulseDiff * (1000.0f / SPEED_CALC_INTERVAL) * 60.0f / HALL_PULSES_PER_REV;
}
