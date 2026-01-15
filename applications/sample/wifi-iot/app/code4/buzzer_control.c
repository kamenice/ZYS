/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * Description: Smart Conveyor Belt System - Buzzer Control Implementation
 * 
 * Active buzzer: LOW level = ON, HIGH level = OFF
 * IMPORTANT: Set GPIO HIGH immediately to ensure buzzer is OFF on power-up
 */

#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "buzzer_control.h"

/* GPIO Pin Definition for Buzzer */
#define BUZZER_PIN      WIFI_IOT_IO_NAME_GPIO_1
#define BUZZER_IDX      WIFI_IOT_GPIO_IDX_1

/* Global variables (volatile for thread safety) */
static volatile int g_isMuted = 0;
static volatile int g_isIntermittent = 0;
static osThreadId_t g_intermittentTaskId = NULL;

void Buzzer_Init(void)
{
    GpioInit();
    
    /* CRITICAL: Set GPIO as output and HIGH FIRST to turn off buzzer */
    /* This prevents buzzer from sounding during initialization */
    IoSetFunc(BUZZER_PIN, WIFI_IOT_IO_FUNC_GPIO_1_GPIO);
    GpioSetDir(BUZZER_IDX, WIFI_IOT_GPIO_DIR_OUT);
    
    /* Turn off buzzer immediately (HIGH = OFF for active low buzzer) */
    GpioSetOutputVal(BUZZER_IDX, 1);
    
    /* Small delay to ensure GPIO state is stable */
    usleep(1000);
    
    /* Ensure buzzer is OFF again after delay */
    GpioSetOutputVal(BUZZER_IDX, 1);
    
    printf("[Buzzer] Buzzer initialized (OFF)\n");
}

void Buzzer_On(void)
{
    if (!g_isMuted) {
        /* LOW = ON for active low buzzer */
        GpioSetOutputVal(BUZZER_IDX, 0);
    }
}

void Buzzer_Off(void)
{
    /* HIGH = OFF for active low buzzer */
    GpioSetOutputVal(BUZZER_IDX, 1);
}

void Buzzer_Beep(int duration_ms)
{
    if (!g_isMuted) {
        Buzzer_On();
        usleep(duration_ms * 1000);
        Buzzer_Off();
    }
}

static void Buzzer_IntermittentTask(void *arg)
{
    (void)arg;
    
    while (g_isIntermittent) {
        if (!g_isMuted) {
            Buzzer_On();
            usleep(500000);  /* 500ms ON */
            Buzzer_Off();
            usleep(1000000); /* 1s OFF */
        } else {
            sleep(1);
        }
    }
}

void Buzzer_StartIntermittent(void)
{
    if (g_isIntermittent) return;  /* Already running */
    
    g_isIntermittent = 1;
    
    osThreadAttr_t attr;
    attr.name = "Buzzer_Intermittent";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024;
    attr.priority = osPriorityBelowNormal;
    
    g_intermittentTaskId = osThreadNew((osThreadFunc_t)Buzzer_IntermittentTask, NULL, &attr);
    printf("[Buzzer] Intermittent beeping started\n");
}

void Buzzer_StopIntermittent(void)
{
    g_isIntermittent = 0;
    Buzzer_Off();
    
    /* Thread will exit gracefully on next loop iteration */
    /* Wait briefly for thread to exit */
    usleep(200000);
    g_intermittentTaskId = NULL;
    printf("[Buzzer] Intermittent beeping stopped\n");
}

void Buzzer_Mute(void)
{
    g_isMuted = 1;
    Buzzer_Off();
    printf("[Buzzer] Muted\n");
}

void Buzzer_Unmute(void)
{
    g_isMuted = 0;
    printf("[Buzzer] Unmuted\n");
}

int Buzzer_IsMuted(void)
{
    return g_isMuted;
}
