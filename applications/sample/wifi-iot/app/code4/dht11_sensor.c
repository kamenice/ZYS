/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * Description: Smart Conveyor Belt System - DHT11 Temperature/Humidity Sensor Implementation
 * 
 * DHT11 uses single-wire communication protocol
 * Reference: DHT11 datasheet
 */

#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "dht11_sensor.h"

/* GPIO Pin Definition for DHT11 */
#define DHT11_PIN       WIFI_IOT_IO_NAME_GPIO_7
#define DHT11_IDX       WIFI_IOT_GPIO_IDX_7

#define DHT11_TASK_STACK_SIZE   4096

/* Global variables */
static float g_temperature = 0.0f;
static float g_humidity = 0.0f;

/* Set GPIO as output */
static void DHT11_SetOutput(void)
{
    IoSetFunc(DHT11_PIN, WIFI_IOT_IO_FUNC_GPIO_7_GPIO);
    GpioSetDir(DHT11_IDX, WIFI_IOT_GPIO_DIR_OUT);
}

/* Set GPIO as input */
static void DHT11_SetInput(void)
{
    IoSetFunc(DHT11_PIN, WIFI_IOT_IO_FUNC_GPIO_7_GPIO);
    GpioSetDir(DHT11_IDX, WIFI_IOT_GPIO_DIR_IN);
    IoSetPull(DHT11_PIN, WIFI_IOT_IO_PULL_UP);
}

void DHT11_Init(void)
{
    GpioInit();
    DHT11_SetOutput();
    GpioSetOutputVal(DHT11_IDX, 1);
}

int DHT11_Read(float *temperature, float *humidity)
{
    uint8_t data[5] = {0};
    uint8_t i, j;
    WifiIotGpioValue val;
    int timeout;
    
    /* Send start signal */
    DHT11_SetOutput();
    GpioSetOutputVal(DHT11_IDX, 0);
    usleep(20000);  /* Pull low for at least 18ms */
    GpioSetOutputVal(DHT11_IDX, 1);
    usleep(30);     /* Pull high for 20-40us */
    
    /* Switch to input mode to receive response */
    DHT11_SetInput();
    
    /* Wait for DHT11 response (low for 80us) */
    timeout = 100;
    GpioGetInputVal(DHT11_IDX, &val);
    while (val == WIFI_IOT_GPIO_VALUE1 && timeout > 0) {
        usleep(1);
        timeout--;
        GpioGetInputVal(DHT11_IDX, &val);
    }
    if (timeout == 0) return -1;
    
    /* DHT11 pulls low for 80us */
    timeout = 100;
    GpioGetInputVal(DHT11_IDX, &val);
    while (val == WIFI_IOT_GPIO_VALUE0 && timeout > 0) {
        usleep(1);
        timeout--;
        GpioGetInputVal(DHT11_IDX, &val);
    }
    if (timeout == 0) return -1;
    
    /* DHT11 pulls high for 80us */
    timeout = 100;
    GpioGetInputVal(DHT11_IDX, &val);
    while (val == WIFI_IOT_GPIO_VALUE1 && timeout > 0) {
        usleep(1);
        timeout--;
        GpioGetInputVal(DHT11_IDX, &val);
    }
    if (timeout == 0) return -1;
    
    /* Read 40 bits (5 bytes) of data */
    for (i = 0; i < 5; i++) {
        for (j = 0; j < 8; j++) {
            /* Wait for low signal to end (start of bit) */
            timeout = 100;
            GpioGetInputVal(DHT11_IDX, &val);
            while (val == WIFI_IOT_GPIO_VALUE0 && timeout > 0) {
                usleep(1);
                timeout--;
                GpioGetInputVal(DHT11_IDX, &val);
            }
            if (timeout == 0) return -1;
            
            /* Measure high signal duration to determine bit value */
            usleep(30);  /* Wait 30us, if still high it's a '1' */
            GpioGetInputVal(DHT11_IDX, &val);
            
            data[i] <<= 1;
            if (val == WIFI_IOT_GPIO_VALUE1) {
                data[i] |= 1;
                /* Wait for remaining high time */
                timeout = 50;
                while (val == WIFI_IOT_GPIO_VALUE1 && timeout > 0) {
                    usleep(1);
                    timeout--;
                    GpioGetInputVal(DHT11_IDX, &val);
                }
            }
        }
    }
    
    /* Verify checksum */
    if (data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
        printf("[DHT11] Checksum error\n");
        return -1;
    }
    
    /* Parse data: data[0]=humidity int, data[1]=humidity dec, data[2]=temp int, data[3]=temp dec */
    *humidity = (float)data[0] + (float)data[1] / 10.0f;
    *temperature = (float)data[2] + (float)data[3] / 10.0f;
    
    return 0;
}

float DHT11_GetTemperature(void)
{
    return g_temperature;
}

float DHT11_GetHumidity(void)
{
    return g_humidity;
}

int DHT11_IsOverheating(void)
{
    return (g_temperature >= TEMP_OVERHEAT_THRESHOLD) ? 1 : 0;
}

static void DHT11_Task(void *arg)
{
    (void)arg;
    float temp, humi;
    
    DHT11_Init();
    sleep(1);  /* Wait for sensor to stabilize */
    
    while (1) {
        if (DHT11_Read(&temp, &humi) == 0) {
            g_temperature = temp;
            g_humidity = humi;
            printf("[DHT11] Temp: %.1f C, Humidity: %.1f%%\n", g_temperature, g_humidity);
        } else {
            printf("[DHT11] Read failed\n");
        }
        sleep(2);  /* DHT11 requires at least 1 second between reads */
    }
}

void DHT11_MainLoop(void)
{
    osThreadAttr_t attr;
    attr.name = "DHT11_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = DHT11_TASK_STACK_SIZE;
    attr.priority = osPriorityNormal;
    
    if (osThreadNew((osThreadFunc_t)DHT11_Task, NULL, &attr) == NULL) {
        printf("[DHT11] Failed to create DHT11 task!\n");
    }
}
