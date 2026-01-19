/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * Description: Smart Conveyor Belt System - Main Control Logic Implementation
 * 
 * Simplified: No overweight detection (pressure sensor only detects presence)
 */

#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"

#include "conveyor_belt.h"
#include "hx711_sensor.h"
#include "dht11_sensor.h"
#include "infrared_sensor.h"
#include "hall_sensor.h"
#include "servo_control.h"
#include "motor_driver.h"
#include "buzzer_control.h"
#include "mqtt_util.h"

#define CONVEYOR_TASK_STACK_SIZE    4096
#define DEFAULT_MOTOR_SPEED         70      /* Default motor speed percentage */

/* Alert flags to prevent repeated alerts (volatile for thread safety) */
static volatile int g_overheatAlertSent = 0;
static volatile int g_jamAlertSent = 0;

/* Run time tracking (volatile for thread safety) */
static volatile uint32_t g_runTime = 0;
static volatile int g_isRunning = 0;

/* Control flags (volatile for thread safety) */
static volatile int g_autoStartEnabled = 1;
static volatile int g_manualVibrate = 0;

void Conveyor_Init(void)
{
    printf("[Conveyor] Initializing conveyor belt system...\n");
    
    Motor_Init();
    Servo_Init();
    Buzzer_Init();
    
    printf("[Conveyor] Conveyor belt system initialized\n");
}

void Conveyor_Start(void)
{
    if (!g_isRunning) {
        Motor_Start(DEFAULT_MOTOR_SPEED);
        g_isRunning = 1;
        printf("[Conveyor] Conveyor belt started\n");
    }
}

void Conveyor_Stop(void)
{
    if (g_isRunning) {
        Motor_Stop();
        g_isRunning = 0;
        printf("[Conveyor] Conveyor belt stopped\n");
    }
}

void Conveyor_ManualVibrate(void)
{
    g_manualVibrate = 1;
}

uint32_t Conveyor_GetRunTime(void)
{
    return g_runTime;
}

static void Conveyor_HandleAutoStart(void)
{
    /* Feature 1: Auto-start when item detected via pressure sensor */
    if (g_autoStartEnabled) {
        if (HX711_IsItemPresent()) {
            if (!g_isRunning) {
                printf("[Conveyor] Item detected, auto-starting...\n");
                Conveyor_Start();
            }
        }
    }
}

static void Conveyor_HandleOverheat(void)
{
    /* Feature: Overheat protection */
    if (DHT11_IsOverheating()) {
        /* Stop conveyor belt to prevent damage */
        if (g_isRunning) {
            Conveyor_Stop();
            printf("[Conveyor] Overheating! Stopping conveyor belt.\n");
        }
        
        /* Intermittent buzzer for overheat warning */
        Buzzer_StartIntermittent();
        
        /* Send alert via MQTT (only once per event) */
        if (!g_overheatAlertSent) {
            mqtt_publish_alert("OVERHEAT");
            g_overheatAlertSent = 1;
            printf("[Conveyor] ALERT: Temperature too high (>30C)!\n");
        }
    } else {
        /* Clear overheat state */
        if (g_overheatAlertSent) {
            Buzzer_StopIntermittent();
            g_overheatAlertSent = 0;
        }
    }
}

static void Conveyor_HandleJam(void)
{
    /* Feature 2: Anti-jam vibration using servo motor */
    if (IR_IsJamDetected() || g_manualVibrate) {
        printf("[Conveyor] Jam detected or manual vibrate requested\n");
        
        /* Trigger servo vibration to clear jam */
        Servo_Vibrate();
        
        /* Send alert via MQTT (only once per event) */
        if (!g_jamAlertSent && !g_manualVibrate) {
            mqtt_publish_alert("JAM");
            g_jamAlertSent = 1;
        }
        
        g_manualVibrate = 0;  /* Clear manual request */
    } else {
        /* Clear jam state */
        if (g_jamAlertSent) {
            g_jamAlertSent = 0;
        }
    }
}

static void Conveyor_Task(void *arg)
{
    (void)arg;
    
    Conveyor_Init();
    
    /* Wait for sensors to initialize */
    sleep(3);
    
    printf("[Conveyor] Starting main control loop\n");
    
    while (1) {
        /* Check and handle auto-start */
        Conveyor_HandleAutoStart();
        
        /* Check and handle overheat */
        Conveyor_HandleOverheat();
        
        /* Check and handle jam */
        Conveyor_HandleJam();
        
        /* Update run time */
        if (g_isRunning) {
            g_runTime++;
        }
        
        sleep(1);
    }
}

void Conveyor_MainLoop(void)
{
    osThreadAttr_t attr;
    attr.name = "Conveyor_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = CONVEYOR_TASK_STACK_SIZE;
    attr.priority = osPriorityAboveNormal;
    
    if (osThreadNew((osThreadFunc_t)Conveyor_Task, NULL, &attr) == NULL) {
        printf("[Conveyor] Failed to create Conveyor task!\n");
    }
}
