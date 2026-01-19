/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * Description: Smart Conveyor Belt System - Servo Motor Control Implementation
 * 
 * Servo PWM control using GPIO bit-banging:
 * - 20ms period (50Hz)
 * - 0.5ms pulse = 0 degrees
 * - 1.5ms pulse = 90 degrees (center)
 * - 2.5ms pulse = 180 degrees
 * 
 * Note: Hardware PWM cannot achieve 50Hz with 16-bit divisor (160MHz/65535 > 2kHz),
 * so we use software GPIO control instead.
 */

#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "servo_control.h"

/* GPIO Pin Definition for Servo */
#define SERVO_PIN       WIFI_IOT_IO_NAME_GPIO_10
#define SERVO_IDX       WIFI_IOT_GPIO_IDX_10

/* Servo timing constants (microseconds) */
#define SERVO_PERIOD_US     20000   /* 20ms period */
#define SERVO_MIN_PULSE_US  500     /* 0.5ms for 0 degrees */
#define SERVO_MAX_PULSE_US  2500    /* 2.5ms for 180 degrees */

/* Global variable (volatile for thread safety) */
static volatile int g_isVibrating = 0;

/* Convert angle to pulse width in microseconds */
static uint32_t AngleToPulseUs(int angle)
{
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;
    
    /* Linear interpolation: 0° -> 500us, 180° -> 2500us */
    return SERVO_MIN_PULSE_US + (angle * (SERVO_MAX_PULSE_US - SERVO_MIN_PULSE_US) / 180);
}

void Servo_Init(void)
{
    GpioInit();
    IoSetFunc(SERVO_PIN, WIFI_IOT_IO_FUNC_GPIO_10_GPIO);
    GpioSetDir(SERVO_IDX, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(SERVO_IDX, 0);
}

/* Generate one PWM pulse for servo */
static void Servo_GeneratePulse(uint32_t pulseUs)
{
    /* High pulse */
    GpioSetOutputVal(SERVO_IDX, 1);
    usleep(pulseUs);
    
    /* Low for remainder of period */
    GpioSetOutputVal(SERVO_IDX, 0);
    usleep(SERVO_PERIOD_US - pulseUs);
}

void Servo_SetAngle(int angle)
{
    uint32_t pulseUs = AngleToPulseUs(angle);
    
    /* Generate a few pulses to move servo to position */
    for (int i = 0; i < 3; i++) {
        Servo_GeneratePulse(pulseUs);
    }
    
    printf("[Servo] Set angle to %d degrees (pulse: %u us)\n", angle, pulseUs);
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
    GpioSetOutputVal(SERVO_IDX, 0);
}

int Servo_IsVibrating(void)
{
    return g_isVibrating;
}
