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
#include "wifiiot_errno.h"
#include "motor.h"

/* Motor state */
static int g_isRunning = 0;
static uint8_t g_speed = MOTOR_DEFAULT_SPEED;

void Motor_Init(void)
{
    /* Initialize PWM pin for motor speed control */
    IoSetFunc(MOTOR_PWMA_GPIO, WIFI_IOT_IO_FUNC_GPIO_10_PWM1_OUT);
    PwmInit(MOTOR_PWM_PORT);

    /* Initialize direction control pins */
    IoSetFunc(MOTOR_AIN1_GPIO, WIFI_IOT_IO_FUNC_GPIO_6_GPIO);
    GpioSetDir(MOTOR_AIN1_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(MOTOR_AIN1_GPIO, WIFI_IOT_GPIO_VALUE1);

    IoSetFunc(MOTOR_AIN2_GPIO, WIFI_IOT_IO_FUNC_GPIO_0_GPIO);
    GpioSetDir(MOTOR_AIN2_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(MOTOR_AIN2_GPIO, WIFI_IOT_GPIO_VALUE0);

    /* Initialize standby pin (active low) */
    IoSetFunc(MOTOR_STBY_GPIO, WIFI_IOT_IO_FUNC_GPIO_1_GPIO);
    GpioSetDir(MOTOR_STBY_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(MOTOR_STBY_GPIO, WIFI_IOT_GPIO_VALUE0); /* Start in standby */

    g_isRunning = 0;
    g_speed = MOTOR_DEFAULT_SPEED;

    printf("[Motor] TB6612FNG driver initialized\r\n");
}

void Motor_Start(void)
{
    if (g_isRunning) {
        return;
    }

    /* Exit standby mode */
    GpioSetOutputVal(MOTOR_STBY_GPIO, WIFI_IOT_GPIO_VALUE1);

    /* Set direction (forward only) */
    GpioSetOutputVal(MOTOR_AIN1_GPIO, WIFI_IOT_GPIO_VALUE1);
    GpioSetOutputVal(MOTOR_AIN2_GPIO, WIFI_IOT_GPIO_VALUE0);

    /* Start PWM with configured speed */
    uint16_t duty = (MOTOR_PWM_FREQ * g_speed) / 100;
    PwmStart(MOTOR_PWM_PORT, duty, MOTOR_PWM_FREQ);

    g_isRunning = 1;
    printf("[Motor] Started at %d%% speed\r\n", g_speed);
}

void Motor_Stop(void)
{
    if (!g_isRunning) {
        return;
    }

    /* Stop PWM */
    PwmStop(MOTOR_PWM_PORT);

    /* Enter standby mode */
    GpioSetOutputVal(MOTOR_STBY_GPIO, WIFI_IOT_GPIO_VALUE0);

    g_isRunning = 0;
    printf("[Motor] Stopped\r\n");
}

void Motor_SetSpeed(uint8_t speed)
{
    if (speed > 100) {
        speed = 100;
    }

    g_speed = speed;

    /* If motor is running, update PWM duty cycle */
    if (g_isRunning) {
        uint16_t duty = (MOTOR_PWM_FREQ * g_speed) / 100;
        PwmStop(MOTOR_PWM_PORT);
        PwmStart(MOTOR_PWM_PORT, duty, MOTOR_PWM_FREQ);
    }

    printf("[Motor] Speed set to %d%%\r\n", speed);
}

uint8_t Motor_GetSpeed(void)
{
    return g_speed;
}

int Motor_IsRunning(void)
{
    return g_isRunning;
}

void Motor_SetStandby(int enable)
{
    if (enable) {
        GpioSetOutputVal(MOTOR_STBY_GPIO, WIFI_IOT_GPIO_VALUE0);
    } else {
        GpioSetOutputVal(MOTOR_STBY_GPIO, WIFI_IOT_GPIO_VALUE1);
    }
}
