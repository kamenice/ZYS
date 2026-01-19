/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * Description: Smart Conveyor Belt System - Hall Effect Sensor Implementation
 * 
 * Measures rotation speed by counting magnet passes per second
 * RPM = (pulse_count / magnets_per_revolution) * 60
 */

#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "hall_sensor.h"

/* GPIO Pin Definition for Hall Sensor */
#define HALL_PIN    WIFI_IOT_IO_NAME_GPIO_6
#define HALL_IDX    WIFI_IOT_GPIO_IDX_6

#define HALL_TASK_STACK_SIZE    2048

/* Assume 1 magnet per revolution */
#define MAGNETS_PER_REVOLUTION  1

/* Global variables (volatile for thread safety) */
static volatile uint32_t g_pulseCount = 0;
static volatile uint32_t g_rpm = 0;

/* GPIO interrupt callback for Hall sensor */
static void Hall_InterruptHandler(char *arg)
{
    (void)arg;
    g_pulseCount++;
}

void Hall_Init(void)
{
    GpioInit();
    IoSetFunc(HALL_PIN, WIFI_IOT_IO_FUNC_GPIO_6_GPIO);
    GpioSetDir(HALL_IDX, WIFI_IOT_GPIO_DIR_IN);
    IoSetPull(HALL_PIN, WIFI_IOT_IO_PULL_UP);
    
    /* Register interrupt on falling edge (magnet detected) */
    GpioRegisterIsrFunc(HALL_IDX, WIFI_IOT_INT_TYPE_EDGE, 
                        WIFI_IOT_GPIO_EDGE_FALL_LEVEL_LOW, 
                        Hall_InterruptHandler, NULL);
}

uint32_t Hall_GetRPM(void)
{
    return g_rpm;
}

static void Hall_Task(void *arg)
{
    (void)arg;
    
    Hall_Init();
    
    while (1) {
        /* Reset pulse counter */
        g_pulseCount = 0;
        
        /* Wait 1 second to count pulses */
        sleep(1);
        
        /* Calculate RPM: pulses per second * 60 / magnets per revolution */
        g_rpm = (g_pulseCount * 60) / MAGNETS_PER_REVOLUTION;
        
        printf("[Hall] RPM: %u\n", g_rpm);
    }
}

void Hall_MainLoop(void)
{
    osThreadAttr_t attr;
    attr.name = "Hall_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = HALL_TASK_STACK_SIZE;
    attr.priority = osPriorityNormal;
    
    if (osThreadNew((osThreadFunc_t)Hall_Task, NULL, &attr) == NULL) {
        printf("[Hall] Failed to create Hall task!\n");
    }
}
