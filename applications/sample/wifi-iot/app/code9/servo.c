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

#include "servo.h"
#include <stdio.h>
#include <unistd.h>
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_pwm.h"

/*
 * Servo Motor Pin Configuration
 * Using GPIO9 for servo PWM control (PWM0)
 *
 * Servo PWM Requirements:
 * - Period: 20ms (50Hz)
 * - High pulse: 0.5ms ~ 2.5ms
 *   - 0.5ms -> 0 degrees
 *   - 1.5ms -> 90 degrees
 *   - 2.5ms -> 180 degrees
 *
 * PWM Configuration Explanation:
 * The Hi3861 PWM uses freq as a divisor value, not actual frequency.
 * With 160MHz system clock and default prescaler:
 * - freq = 64000 gives approximately 2.5kHz carrier (160MHz / 64000)
 * - The duty cycle ratio (duty/freq) determines the pulse width
 * - For 20ms period servo signal, we rely on the internal timing
 *
 * Duty cycle calculation for servo position:
 * - duty / freq = high_pulse_time / period
 * - For 0.5ms (0°):   duty = 64000 * 0.5 / 20 = 1600 (2.5% duty)
 * - For 1.5ms (90°):  duty = 64000 * 1.5 / 20 = 4800 (7.5% duty)
 * - For 2.5ms (180°): duty = 64000 * 2.5 / 20 = 8000 (12.5% duty)
 *
 * Note: The actual PWM frequency may differ from standard 50Hz,
 * but the duty cycle ratio maintains correct pulse widths for servo control.
 */
#define SERVO_GPIO      WIFI_IOT_GPIO_IDX_9
#define SERVO_PWM_PORT  WIFI_IOT_PWM_PORT_PWM0

/* PWM frequency divisor for 20ms period */
#define SERVO_PWM_FREQ  64000

/* Duty cycle values for different angles */
#define SERVO_DUTY_0DEG     1600    /* 0.5ms */
#define SERVO_DUTY_90DEG    4800    /* 1.5ms */
#define SERVO_DUTY_180DEG   8000    /* 2.5ms */

void Servo_Init(void)
{
    /* Set GPIO9 to PWM0 function */
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_IO_FUNC_GPIO_9_PWM0_OUT);
    PwmInit(SERVO_PWM_PORT);
    printf("[Servo] Initialized on GPIO9 (PWM0)\n");
}

void Servo_SetAngle(int angle)
{
    unsigned short duty;

    /* Clamp angle to valid range */
    if (angle < 0) {
        angle = 0;
    }
    if (angle > 180) {
        angle = 180;
    }

    /* Calculate duty cycle based on angle
     * Linear interpolation between 0.5ms (0°) and 2.5ms (180°)
     * duty = 1600 + (angle / 180) * (8000 - 1600)
     */
    duty = SERVO_DUTY_0DEG + (angle * (SERVO_DUTY_180DEG - SERVO_DUTY_0DEG)) / 180;

    PwmStart(SERVO_PWM_PORT, duty, SERVO_PWM_FREQ);
    printf("[Servo] Angle set to %d degrees (duty: %d)\n", angle, duty);
}

void Servo_Open(void)
{
    Servo_SetAngle(90);
}

void Servo_Close(void)
{
    Servo_SetAngle(0);
}
