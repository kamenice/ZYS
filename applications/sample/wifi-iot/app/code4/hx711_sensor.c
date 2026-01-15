/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * Description: Smart Conveyor Belt System - HX711 Pressure Sensor Implementation
 * 
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

/* Calibration factor - adjust based on sensor characteristics */
/* Increase if measured weight is high, decrease if low */
#define HX711_CALIBRATION_FACTOR    1300.0f

#define HX711_TASK_STACK_SIZE       4096

/* Global variables (volatile for thread safety) */
static volatile double g_baseValue = 0.0;
static volatile float g_currentWeight = 0.0f;

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

static double HX711_GetAverageReading(int samples)
{
    double sum = 0.0;
    for (int i = 0; i < samples; i++) {
        sum += (double)HX711_ReadRaw();
    }
    return sum / samples;
}

void HX711_Tare(void)
{
    g_baseValue = HX711_GetAverageReading(10);
    printf("[HX711] Tare complete, base value: %.2f\n", g_baseValue);
}

float HX711_GetWeight(void)
{
    return g_currentWeight;
}

int HX711_IsItemPresent(void)
{
    return (g_currentWeight > WEIGHT_PRESENCE_THRESHOLD * 1000.0f) ? 1 : 0;
}

int HX711_IsOverweight(void)
{
    return (g_currentWeight > WEIGHT_OVERLOAD_THRESHOLD * 1000.0f) ? 1 : 0;
}

static void HX711_Task(void *arg)
{
    (void)arg;
    
    HX711_Init();
    usleep(100000); /* Wait for sensor to stabilize */
    HX711_Tare();   /* Set baseline weight */
    
    while (1) {
        double currentReading = HX711_GetAverageReading(10);
        g_currentWeight = (float)((currentReading - g_baseValue) / HX711_CALIBRATION_FACTOR);
        
        if (g_currentWeight < 0) {
            g_currentWeight = 0;
        }
        
        printf("[HX711] Weight: %.2f g\n", g_currentWeight);
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
