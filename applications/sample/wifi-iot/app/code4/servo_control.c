/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * Description: Smart Conveyor Belt System - Servo Motor Control Implementation
 * 
 * PWM frequency for servo: 50Hz (20ms period)
 * Duty cycle range: 2.5% (0.5ms) to 12.5% (2.5ms)
 */

#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_pwm.h"
#include "servo_control.h"

/* GPIO/PWM Pin Definition for Servo */
#define SERVO_PIN       WIFI_IOT_IO_NAME_GPIO_10
#define SERVO_PWM_PORT  WIFI_IOT_PWM_PORT_PWM1

/* PWM clock frequency (Hz) */
#define PWM_CLK_FREQ    160000000
/* PWM frequency for servo (50Hz = 20ms period) */
#define SERVO_FREQ      50
/* PWM period divisor */
#define SERVO_PERIOD_DIVISOR    (PWM_CLK_FREQ / SERVO_FREQ)

/* Global variable (volatile for thread safety) */
static volatile int g_isVibrating = 0;

/* Convert angle to PWM duty cycle
 * 0 degrees = 0.5ms = 2.5% duty
 * 180 degrees = 2.5ms = 12.5% duty
 * Duty = (angle / 180) * 10% + 2.5%
 */
static uint16_t AngleToDuty(int angle)
{
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;
    
    /* Calculate duty cycle in microseconds (500us to 2500us) */
    uint32_t pulseWidth = 500 + (angle * 2000 / 180);
    
    /* Convert to PWM divisor (20ms period) */
    uint32_t duty = (pulseWidth * SERVO_PERIOD_DIVISOR) / 20000;
    
    return (uint16_t)duty;
}

void Servo_Init(void)
{
    GpioInit();
    IoSetFunc(SERVO_PIN, WIFI_IOT_IO_FUNC_GPIO_10_PWM1_OUT);
    PwmInit(SERVO_PWM_PORT);
}

void Servo_SetAngle(int angle)
{
    uint16_t duty = AngleToDuty(angle);
    PwmStart(SERVO_PWM_PORT, duty, SERVO_PERIOD_DIVISOR);
    printf("[Servo] Set angle to %d degrees\n", angle);
}

void Servo_Vibrate(void)
{
    g_isVibrating = 1;
    printf("[Servo] Vibration started\n");
    
    /* Vibrate by alternating between 60 and 120 degrees */
    for (int i = 0; i < 10 && g_isVibrating; i++) {
        Servo_SetAngle(60);
        usleep(100000);  /* 100ms */
        Servo_SetAngle(120);
        usleep(100000);  /* 100ms */
    }
    
    /* Return to center position */
    Servo_SetAngle(90);
    g_isVibrating = 0;
    printf("[Servo] Vibration stopped\n");
}

void Servo_Stop(void)
{
    g_isVibrating = 0;
    PwmStop(SERVO_PWM_PORT);
}

int Servo_IsVibrating(void)
{
    return g_isVibrating;
}
