/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * Description: Smart Conveyor Belt System - Infrared Sensor Implementation
 * 
 * IR sensor outputs low level when object is detected
 */

#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "infrared_sensor.h"

/* GPIO Pin Definition for IR Sensor */
#define IR_PIN      WIFI_IOT_IO_NAME_GPIO_8
#define IR_IDX      WIFI_IOT_GPIO_IDX_8

#define IR_TASK_STACK_SIZE  2048

/* Global variable for jam state */
static int g_jamDetected = 0;

void IR_Init(void)
{
    GpioInit();
    IoSetFunc(IR_PIN, WIFI_IOT_IO_FUNC_GPIO_8_GPIO);
    GpioSetDir(IR_IDX, WIFI_IOT_GPIO_DIR_IN);
    IoSetPull(IR_PIN, WIFI_IOT_IO_PULL_UP);
}

int IR_IsJamDetected(void)
{
    return g_jamDetected;
}

static void IR_Task(void *arg)
{
    (void)arg;
    WifiIotGpioValue val;
    
    IR_Init();
    
    while (1) {
        GpioGetInputVal(IR_IDX, &val);
        
        /* IR sensor outputs LOW when object is detected */
        if (val == WIFI_IOT_GPIO_VALUE0) {
            if (!g_jamDetected) {
                printf("[IR] Jam/pile-up detected!\n");
            }
            g_jamDetected = 1;
        } else {
            if (g_jamDetected) {
                printf("[IR] Jam cleared\n");
            }
            g_jamDetected = 0;
        }
        
        usleep(100000);  /* Check every 100ms */
    }
}

void IR_MainLoop(void)
{
    osThreadAttr_t attr;
    attr.name = "IR_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = IR_TASK_STACK_SIZE;
    attr.priority = osPriorityNormal;
    
    if (osThreadNew((osThreadFunc_t)IR_Task, NULL, &attr) == NULL) {
        printf("[IR] Failed to create IR task!\n");
    }
}
