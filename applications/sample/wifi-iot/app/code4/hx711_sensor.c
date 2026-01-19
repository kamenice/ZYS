/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * Description: Smart Conveyor Belt System - HX711 Pressure Sensor Implementation
 * 
 * Simplified version: Only detects presence of items (yes/no), no weight calculation
 * Reference: HX711 24-bit ADC datasheet and existing app_demo_hx711.c
 */

#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "hx711_sensor.h"

/* GPIO Pin Definitions for HX711 */
#define HX711_DT_PIN    WIFI_IOT_IO_NAME_GPIO_11   /* Data pin */
#define HX711_SCK_PIN   WIFI_IOT_IO_NAME_GPIO_12   /* Clock pin */
#define HX711_DT_IDX    WIFI_IOT_GPIO_IDX_11
#define HX711_SCK_IDX   WIFI_IOT_GPIO_IDX_12

/* Threshold for detecting item presence (raw ADC value difference) */
#define HX711_PRESENCE_THRESHOLD    50000

#define HX711_TASK_STACK_SIZE       4096

/* Global variables (volatile for thread safety) */
static volatile unsigned long g_baseValue = 0;
static volatile unsigned long g_currentReading = 0;
static volatile int g_itemPresent = 0;

void HX711_Init(void)
{
    GpioInit();
    
    /* Initialize DT pin as input */
    IoSetFunc(HX711_DT_PIN, WIFI_IOT_IO_FUNC_GPIO_11_GPIO);
    GpioSetDir(HX711_DT_IDX, WIFI_IOT_GPIO_DIR_IN);
    
    /* Initialize SCK pin as output */
    IoSetFunc(HX711_SCK_PIN, WIFI_IOT_IO_FUNC_GPIO_12_GPIO);
    GpioSetDir(HX711_SCK_IDX, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(HX711_SCK_IDX, 0);
}

unsigned long HX711_ReadRaw(void)
{
    unsigned long value = 0;
    unsigned char i = 0;
    WifiIotGpioValue input = 0;
    
    usleep(2);
    
    /* Keep clock low during idle */
    GpioSetOutputVal(HX711_SCK_IDX, 0);
    usleep(2);
    
    /* Wait for HX711 to be ready (DOUT goes low) */
    GpioGetInputVal(HX711_DT_IDX, &input);
    while (input) {
        GpioGetInputVal(HX711_DT_IDX, &input);
    }
    
    /* Read 24-bit data */
    for (i = 0; i < 24; i++) {
        /* Clock high - start pulse */
        GpioSetOutputVal(HX711_SCK_IDX, 1);
        usleep(2);
        
        /* Shift left to make room for new bit */
        value = value << 1;
        
        /* Clock low */
        GpioSetOutputVal(HX711_SCK_IDX, 0);
        usleep(2);
        
        /* Read data bit */
        GpioGetInputVal(HX711_DT_IDX, &input);
        if (input) {
            value++;
        }
    }
    
    /* 25th clock pulse - set gain to 128 for channel A */
    GpioSetOutputVal(HX711_SCK_IDX, 1);
    usleep(2);
    value = value ^ 0x800000;  /* Convert to signed */
    GpioSetOutputVal(HX711_SCK_IDX, 0);
    usleep(2);
    
    return value;
}

static unsigned long HX711_GetAverageReading(int samples)
{
    unsigned long sum = 0;
    int i = 0;
    for (i = 0; i < samples; i++) {
        sum += HX711_ReadRaw();
    }
    return sum / samples;
}

void HX711_Tare(void)
{
    g_baseValue = HX711_GetAverageReading(10);
    printf("[HX711] Tare complete, base value: %lu\n", g_baseValue);
}

float HX711_GetWeight(void)
{
    /* Weight calculation disabled - just return 0 */
    /* This function kept for API compatibility */
    return 0.0f;
}

int HX711_IsItemPresent(void)
{
    return g_itemPresent;
}

int HX711_IsOverweight(void)
{
    /* Overweight detection disabled - always return 0 */
    /* This function kept for API compatibility */
    return 0;
}

static void HX711_Task(void *arg)
{
    (void)arg;
    unsigned long diff = 0;
    
    HX711_Init();
    usleep(100000); /* Wait for sensor to stabilize */
    HX711_Tare();   /* Set baseline value */
    
    while (1) {
        g_currentReading = HX711_GetAverageReading(5);
        
        /* Calculate difference from baseline */
        if (g_currentReading > g_baseValue) {
            diff = g_currentReading - g_baseValue;
        } else {
            diff = g_baseValue - g_currentReading;
        }
        
        /* Determine if item is present based on threshold */
        g_itemPresent = (diff > HX711_PRESENCE_THRESHOLD) ? 1 : 0;
        
        printf("[HX711] Item present: %s\n", g_itemPresent ? "YES" : "NO");
        sleep(1);
    }
}

void HX711_MainLoop(void)
{
    osThreadAttr_t attr;
    attr.name = "HX711_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = HX711_TASK_STACK_SIZE;
    attr.priority = osPriorityNormal;
    
    if (osThreadNew((osThreadFunc_t)HX711_Task, NULL, &attr) == NULL) {
        printf("[HX711] Failed to create HX711 task!\n");
    }
}
