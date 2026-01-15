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
#include "wifiiot_pwm.h"
#include "tb6612.h"

/*
 * TB6612 Motor Driver Pin Configuration:
 * - AIN1: GPIO2 - Motor A direction control 1
 * - AIN2: GPIO3 - Motor A direction control 2
 * - PWMA: GPIO5 (PWM2) - Motor A PWM speed control
 * - STBY: GPIO6 - Standby control (HIGH = work, LOW = standby)
 *
 * Working principle:
 * - AIN1/AIN2 same level = motor runs
 * - AIN1/AIN2 different level = motor brakes
 * - PWMA high = motor works
 * - STBY high = chip works
 */

#define TB6612_AIN1_GPIO    WIFI_IOT_GPIO_IDX_2
#define TB6612_AIN2_GPIO    WIFI_IOT_GPIO_IDX_3
#define TB6612_PWMA_GPIO    WIFI_IOT_GPIO_IDX_5
#define TB6612_STBY_GPIO    WIFI_IOT_GPIO_IDX_6

#define TB6612_PWM_PORT     WIFI_IOT_PWM_PORT_PWM2

// PWM parameters for motor control
#define TB6612_PWM_DUTY     40000
#define TB6612_PWM_FREQ     40000

static int g_motor_running = 0;

void TB6612_Init(void)
{
    // Initialize AIN1 (GPIO2) as output
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_2, WIFI_IOT_IO_FUNC_GPIO_2_GPIO);
    GpioSetDir(TB6612_AIN1_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(TB6612_AIN1_GPIO, WIFI_IOT_GPIO_VALUE0);

    // Initialize AIN2 (GPIO3) as output
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_3, WIFI_IOT_IO_FUNC_GPIO_3_GPIO);
    GpioSetDir(TB6612_AIN2_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(TB6612_AIN2_GPIO, WIFI_IOT_GPIO_VALUE0);

    // Initialize STBY (GPIO6) as output
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_6, WIFI_IOT_IO_FUNC_GPIO_6_GPIO);
    GpioSetDir(TB6612_STBY_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(TB6612_STBY_GPIO, WIFI_IOT_GPIO_VALUE0);

    // Initialize PWMA (GPIO5) as PWM output
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_5, WIFI_IOT_IO_FUNC_GPIO_5_PWM2_OUT);
    PwmInit(TB6612_PWM_PORT);

    g_motor_running = 0;
    printf("[TB6612] Initialized\n");
}

void TB6612_Start(void)
{
    // Set STBY high to enable chip
    GpioSetOutputVal(TB6612_STBY_GPIO, WIFI_IOT_GPIO_VALUE1);

    // Set AIN1 and AIN2 to same level (both HIGH) for motor to run
    GpioSetOutputVal(TB6612_AIN1_GPIO, WIFI_IOT_GPIO_VALUE1);
    GpioSetOutputVal(TB6612_AIN2_GPIO, WIFI_IOT_GPIO_VALUE1);

    // Start PWM for motor speed control
    PwmStart(TB6612_PWM_PORT, TB6612_PWM_DUTY, TB6612_PWM_FREQ);

    g_motor_running = 1;
    printf("[TB6612] Motor started\n");
}

void TB6612_Stop(void)
{
    // Stop PWM
    PwmStop(TB6612_PWM_PORT);

    // Set AIN1 and AIN2 to different levels for brake
    GpioSetOutputVal(TB6612_AIN1_GPIO, WIFI_IOT_GPIO_VALUE1);
    GpioSetOutputVal(TB6612_AIN2_GPIO, WIFI_IOT_GPIO_VALUE0);

    // Set STBY low to disable chip
    GpioSetOutputVal(TB6612_STBY_GPIO, WIFI_IOT_GPIO_VALUE0);

    g_motor_running = 0;
    printf("[TB6612] Motor stopped\n");
}

int TB6612_IsRunning(void)
{
    return g_motor_running;
}
