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
#include "servo.h"

void Servo_Init(void)
{
    /* Initialize PWM pin for servo control */
    IoSetFunc(SERVO_GPIO, WIFI_IOT_IO_FUNC_GPIO_2_PWM2_OUT);
    PwmInit(SERVO_PWM_PORT);

    printf("[Servo] SG90 servo initialized\r\n");
}

void Servo_SetAngle(uint8_t angle)
{
    if (angle > 180) {
        angle = 180;
    }

    /* Calculate duty cycle based on angle
     * Linear interpolation between MIN_DUTY (0°) and MAX_DUTY (180°)
     */
    uint16_t duty = SERVO_MIN_DUTY + ((SERVO_MAX_DUTY - SERVO_MIN_DUTY) * angle) / 180;

    PwmStop(SERVO_PWM_PORT);
    PwmStart(SERVO_PWM_PORT, duty, SERVO_PWM_FREQ);

    /* Brief delay for servo to reach position */
    usleep(50000);
}

void Servo_Vibrate(void)
{
    printf("[Servo] Starting vibration to clear jam\r\n");

    /* Perform rapid back-and-forth movements to shake items loose */
    for (int i = 0; i < SERVO_VIBRATION_COUNT; i++) {
        /* Quick movement to one side */
        Servo_SetAngle(60);
        usleep(SERVO_VIBRATION_DELAY * 1000);

        /* Quick movement to other side */
        Servo_SetAngle(120);
        usleep(SERVO_VIBRATION_DELAY * 1000);
    }

    /* Return to center */
    Servo_Center();

    printf("[Servo] Vibration complete\r\n");
}

void Servo_Center(void)
{
    PwmStop(SERVO_PWM_PORT);
    PwmStart(SERVO_PWM_PORT, SERVO_CENTER_DUTY, SERVO_PWM_FREQ);
    usleep(100000);
}

void Servo_Stop(void)
{
    PwmStop(SERVO_PWM_PORT);
}
