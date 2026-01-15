/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * Description: Smart Conveyor Belt System - OLED Display Implementation
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"

#include "oled_ssd1306.h"
#include "oled_display.h"
#include "hx711_sensor.h"
#include "dht11_sensor.h"
#include "infrared_sensor.h"
#include "hall_sensor.h"
#include "motor_driver.h"
#include "buzzer_control.h"
#include "conveyor_belt.h"

#define DISPLAY_TASK_STACK_SIZE     4096

void Display_Init(void)
{
    GpioInit();
    OledInit();
    OledFillScreen(0x00);
    OledShowString(0, 0, "Conveyor System", 1);
    printf("[Display] OLED initialized\n");
}

void Display_Update(void)
{
    char line[32] = {0};
    
    /* Line 0: Title */
    OledShowString(0, 0, "Conveyor System", 1);
    
    /* Line 1: Weight */
    float weight = HX711_GetWeight();
    snprintf(line, sizeof(line), "Wt:%.0fg %s", weight, 
             HX711_IsOverweight() ? "OVR!" : "    ");
    OledShowString(0, 1, line, 1);
    
    /* Line 2: Status */
    int jam = IR_IsJamDetected();
    int running = Motor_IsRunning();
    snprintf(line, sizeof(line), "St:%s Jam:%s", 
             running ? "RUN " : "STOP",
             jam ? "YES" : "NO ");
    OledShowString(0, 2, line, 1);
    
    /* Line 3: Temperature */
    float temp = DHT11_GetTemperature();
    snprintf(line, sizeof(line), "Temp:%.1fC %s", temp,
             DHT11_IsOverheating() ? "HOT!" : "    ");
    OledShowString(0, 3, line, 1);
    
    /* Line 4: Speed (RPM) */
    uint32_t rpm = Hall_GetRPM();
    snprintf(line, sizeof(line), "Speed:%u RPM    ", rpm);
    OledShowString(0, 4, line, 1);
    
    /* Line 5: Run time */
    uint32_t runTime = Conveyor_GetRunTime();
    uint32_t hours = runTime / 3600;
    uint32_t mins = (runTime % 3600) / 60;
    uint32_t secs = runTime % 60;
    snprintf(line, sizeof(line), "Time:%02u:%02u:%02u   ", hours, mins, secs);
    OledShowString(0, 5, line, 1);
    
    /* Line 6: Mute status */
    snprintf(line, sizeof(line), "Mute:%s         ", 
             Buzzer_IsMuted() ? "ON " : "OFF");
    OledShowString(0, 6, line, 1);
}

static void Display_Task(void *arg)
{
    (void)arg;
    
    Display_Init();
    sleep(1);
    
    while (1) {
        Display_Update();
        usleep(500000);  /* Update every 500ms */
    }
}

void Display_MainLoop(void)
{
    osThreadAttr_t attr;
    attr.name = "Display_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = DISPLAY_TASK_STACK_SIZE;
    attr.priority = osPriorityNormal;
    
    if (osThreadNew((osThreadFunc_t)Display_Task, NULL, &attr) == NULL) {
        printf("[Display] Failed to create Display task!\n");
    }
}
