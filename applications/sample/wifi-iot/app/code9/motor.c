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

#include "motor.h"
#include <stdio.h>
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"

/*
 * TB6612 Motor Driver Pin Configuration
 * GPIO0/GPIO2 are reserved for system debugging, cannot use
 *
 * TB6612 Working Principle:
 * - AIN1/AIN2 same level: motor works
 * - AIN1/AIN2 different level: motor brakes
 * - PWMA high: motor works
 * - STBY high: chip works, low: chip doesn't work
 *
 * Pin Assignment:
 * - GPIO3: AIN1
 * - GPIO4: AIN2
 * - GPIO5: PWMA
 * - GPIO6: STBY
 */
#define MOTOR_AIN1_GPIO     WIFI_IOT_GPIO_IDX_3
#define MOTOR_AIN2_GPIO     WIFI_IOT_GPIO_IDX_4
#define MOTOR_PWMA_GPIO     WIFI_IOT_GPIO_IDX_5
#define MOTOR_STBY_GPIO     WIFI_IOT_GPIO_IDX_6

static MotorState g_motorState = MOTOR_STATE_STOPPED;

void Motor_Init(void)
{
    /* Configure AIN1 */
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_3, WIFI_IOT_IO_FUNC_GPIO_3_GPIO);
    GpioSetDir(MOTOR_AIN1_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(MOTOR_AIN1_GPIO, WIFI_IOT_GPIO_VALUE0);

    /* Configure AIN2 */
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_4, WIFI_IOT_IO_FUNC_GPIO_4_GPIO);
    GpioSetDir(MOTOR_AIN2_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(MOTOR_AIN2_GPIO, WIFI_IOT_GPIO_VALUE0);

    /* Configure PWMA */
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_5, WIFI_IOT_IO_FUNC_GPIO_5_GPIO);
    GpioSetDir(MOTOR_PWMA_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(MOTOR_PWMA_GPIO, WIFI_IOT_GPIO_VALUE0);

    /* Configure STBY */
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_6, WIFI_IOT_IO_FUNC_GPIO_6_GPIO);
    GpioSetDir(MOTOR_STBY_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(MOTOR_STBY_GPIO, WIFI_IOT_GPIO_VALUE0);

    g_motorState = MOTOR_STATE_STOPPED;
    printf("[Motor] TB6612 initialized\n");
}

void Motor_Start(void)
{
    /* Enable STBY (chip works) */
    GpioSetOutputVal(MOTOR_STBY_GPIO, WIFI_IOT_GPIO_VALUE1);

    /* Set AIN1 and AIN2 to same level (motor works) */
    GpioSetOutputVal(MOTOR_AIN1_GPIO, WIFI_IOT_GPIO_VALUE1);
    GpioSetOutputVal(MOTOR_AIN2_GPIO, WIFI_IOT_GPIO_VALUE1);

    /* Enable PWMA (motor works) */
    GpioSetOutputVal(MOTOR_PWMA_GPIO, WIFI_IOT_GPIO_VALUE1);

    g_motorState = MOTOR_STATE_RUNNING;
    printf("[Motor] Started\n");
}

void Motor_Stop(void)
{
    /* Disable PWMA */
    GpioSetOutputVal(MOTOR_PWMA_GPIO, WIFI_IOT_GPIO_VALUE0);

    /* Set AIN1 and AIN2 to different level (brake) */
    GpioSetOutputVal(MOTOR_AIN1_GPIO, WIFI_IOT_GPIO_VALUE1);
    GpioSetOutputVal(MOTOR_AIN2_GPIO, WIFI_IOT_GPIO_VALUE0);

    /* Disable STBY (chip doesn't work) */
    GpioSetOutputVal(MOTOR_STBY_GPIO, WIFI_IOT_GPIO_VALUE0);

    g_motorState = MOTOR_STATE_STOPPED;
    printf("[Motor] Stopped\n");
}

MotorState Motor_GetState(void)
{
    return g_motorState;
}
