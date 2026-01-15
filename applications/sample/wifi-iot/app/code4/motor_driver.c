/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * Description: Smart Conveyor Belt System - Motor Driver Implementation
 * 
 * TB6612FNG motor driver control
 */

#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_pwm.h"
#include "motor_driver.h"

/* GPIO Pin Definitions for TB6612FNG */
#define MOTOR_PWMA_PIN      WIFI_IOT_IO_NAME_GPIO_9     /* PWM speed control */
#define MOTOR_AIN1_PIN      WIFI_IOT_IO_NAME_GPIO_3     /* Direction control 1 */
#define MOTOR_AIN2_PIN      WIFI_IOT_IO_NAME_GPIO_4     /* Direction control 2 */
#define MOTOR_STBY_PIN      WIFI_IOT_IO_NAME_GPIO_5     /* Standby control */

#define MOTOR_PWMA_IDX      WIFI_IOT_GPIO_IDX_9
#define MOTOR_AIN1_IDX      WIFI_IOT_GPIO_IDX_3
#define MOTOR_AIN2_IDX      WIFI_IOT_GPIO_IDX_4
#define MOTOR_STBY_IDX      WIFI_IOT_GPIO_IDX_5

#define MOTOR_PWM_PORT      WIFI_IOT_PWM_PORT_PWM0

/* PWM frequency divisor for motor control */
#define MOTOR_PWM_FREQ_DIV  40000

/* Global variables */
static int g_isRunning = 0;
static uint8_t g_currentSpeed = 50;

void Motor_Init(void)
{
    GpioInit();
    
    /* Initialize PWMA pin for speed control */
    IoSetFunc(MOTOR_PWMA_PIN, WIFI_IOT_IO_FUNC_GPIO_9_PWM0_OUT);
    PwmInit(MOTOR_PWM_PORT);
    
    /* Initialize AIN1 as output */
    IoSetFunc(MOTOR_AIN1_PIN, WIFI_IOT_IO_FUNC_GPIO_3_GPIO);
    GpioSetDir(MOTOR_AIN1_IDX, WIFI_IOT_GPIO_DIR_OUT);
    
    /* Initialize AIN2 as output */
    IoSetFunc(MOTOR_AIN2_PIN, WIFI_IOT_IO_FUNC_GPIO_4_GPIO);
    GpioSetDir(MOTOR_AIN2_IDX, WIFI_IOT_GPIO_DIR_OUT);
    
    /* Initialize STBY as output (HIGH = active) */
    IoSetFunc(MOTOR_STBY_PIN, WIFI_IOT_IO_FUNC_GPIO_5_GPIO);
    GpioSetDir(MOTOR_STBY_IDX, WIFI_IOT_GPIO_DIR_OUT);
    
    /* Set initial state: stopped, standby mode */
    GpioSetOutputVal(MOTOR_AIN1_IDX, 0);
    GpioSetOutputVal(MOTOR_AIN2_IDX, 0);
    GpioSetOutputVal(MOTOR_STBY_IDX, 0);
    
    printf("[Motor] Motor driver initialized\n");
}

void Motor_Start(uint8_t speed)
{
    if (speed > 100) speed = 100;
    g_currentSpeed = speed;
    
    /* Set direction: AIN1=HIGH, AIN2=LOW for forward */
    GpioSetOutputVal(MOTOR_AIN1_IDX, 1);
    GpioSetOutputVal(MOTOR_AIN2_IDX, 0);
    
    /* Enable motor (exit standby) */
    GpioSetOutputVal(MOTOR_STBY_IDX, 1);
    
    /* Set speed via PWM */
    uint32_t duty = (MOTOR_PWM_FREQ_DIV * speed) / 100;
    PwmStart(MOTOR_PWM_PORT, duty, MOTOR_PWM_FREQ_DIV);
    
    g_isRunning = 1;
    printf("[Motor] Motor started at %d%% speed\n", speed);
}

void Motor_Stop(void)
{
    /* Stop PWM */
    PwmStop(MOTOR_PWM_PORT);
    
    /* Brake mode: both AIN1 and AIN2 LOW */
    GpioSetOutputVal(MOTOR_AIN1_IDX, 0);
    GpioSetOutputVal(MOTOR_AIN2_IDX, 0);
    
    /* Enter standby mode */
    GpioSetOutputVal(MOTOR_STBY_IDX, 0);
    
    g_isRunning = 0;
    printf("[Motor] Motor stopped\n");
}

void Motor_SetSpeed(uint8_t speed)
{
    if (speed > 100) speed = 100;
    g_currentSpeed = speed;
    
    if (g_isRunning) {
        uint32_t duty = (MOTOR_PWM_FREQ_DIV * speed) / 100;
        PwmStart(MOTOR_PWM_PORT, duty, MOTOR_PWM_FREQ_DIV);
        printf("[Motor] Speed changed to %d%%\n", speed);
    }
}

int Motor_IsRunning(void)
{
    return g_isRunning;
}

uint8_t Motor_GetSpeed(void)
{
    return g_currentSpeed;
}
