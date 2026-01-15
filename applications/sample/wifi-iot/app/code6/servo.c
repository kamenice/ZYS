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
#include "servo.h"

/*
 * Servo Motor Configuration:
 * - Uses GPIO9 with PWM0 for control
 * - PWM signal requirements:
 *   - Period: 20ms (50Hz frequency)
 *   - High pulse: 0.5ms~2.5ms
 *   - 0.5ms = 0 degrees
 *   - 1.5ms = 90 degrees
 *   - 2.5ms = 180 degrees
 *
 * Using 160MHz clock:
 * - For 20ms period, freq divisor = 160000000 * 0.02 = 3200000 (too large)
 * - We need to use a smaller divisor and adjust duty cycle accordingly
 *
 * Alternative calculation:
 * - Using smaller values that fit in uint16_t
 * - PWM frequency setting: 160MHz / freq_divisor
 * - For 50Hz (20ms): divisor = 160MHz / 50Hz = 3.2M (doesn't fit)
 *
 * Using 40MHz XTAL clock source would work better
 * But we'll use software PWM simulation for precise control
 */

#define SERVO_GPIO      WIFI_IOT_GPIO_IDX_9
#define SERVO_IO_NAME   WIFI_IOT_IO_NAME_GPIO_9

// Software PWM timing in microseconds
#define SERVO_PERIOD_US     20000   // 20ms period
#define SERVO_MIN_PULSE_US  500     // 0.5ms for 0 degrees
#define SERVO_MAX_PULSE_US  2500    // 2.5ms for 180 degrees

void Servo_Init(void)
{
    // Initialize GPIO9 as output for software PWM
    IoSetFunc(SERVO_IO_NAME, WIFI_IOT_IO_FUNC_GPIO_9_GPIO);
    GpioSetDir(SERVO_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(SERVO_GPIO, WIFI_IOT_GPIO_VALUE0);

    printf("[Servo] Initialized\n");
}

void Servo_SetAngle(int angle)
{
    // Clamp angle to 0-180
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    // Calculate pulse width for the angle
    // Linear interpolation: 0.5ms at 0 degrees, 2.5ms at 180 degrees
    int pulse_us = SERVO_MIN_PULSE_US +
                   (angle * (SERVO_MAX_PULSE_US - SERVO_MIN_PULSE_US)) / 180;

    // Generate PWM signal using software timing
    // Send multiple pulses for smooth movement (reduced to minimize blocking)
    for (int i = 0; i < 10; i++) {  // 10 cycles = 200ms of PWM
        // High pulse
        GpioSetOutputVal(SERVO_GPIO, WIFI_IOT_GPIO_VALUE1);
        usleep(pulse_us);

        // Low pulse (remainder of period)
        GpioSetOutputVal(SERVO_GPIO, WIFI_IOT_GPIO_VALUE0);
        usleep(SERVO_PERIOD_US - pulse_us);
    }

    printf("[Servo] Set angle to %d degrees (pulse: %d us)\n", angle, pulse_us);
}

void Servo_Activate(void)
{
    // Move servo to 90 degrees (operational position)
    Servo_SetAngle(90);
    printf("[Servo] Activated\n");
}

void Servo_Deactivate(void)
{
    // Move servo to 0 degrees (rest position)
    Servo_SetAngle(0);
    printf("[Servo] Deactivated\n");
}
