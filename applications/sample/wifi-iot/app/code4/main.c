/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * Description: Smart Conveyor Belt System - Main Entry Point
 * 
 * System Overview:
 * ================
 * This is a smart conveyor belt system with the following features:
 * 
 * 1. Auto-Start Detection: Uses HX711 pressure sensor to detect items
 *    on the conveyor belt and automatically start the motor.
 *    Can also be controlled via APP (MQTT).
 * 
 * 2. Anti-Jam Function: Uses infrared sensor to detect item pile-up.
 *    When jam is detected, servo motor vibrates the belt to clear it.
 *    Can also be triggered manually via APP.
 * 
 * 3. Overweight Protection: Monitors weight via HX711 sensor.
 *    When items exceed 8kg, continuous buzzer alarm sounds.
 *    Alert is pushed to APP via MQTT.
 * 
 * 4. Overheat Protection: Uses DHT11 temperature sensor to monitor
 *    bearing temperature. When temp exceeds 30°C, stops conveyor
 *    and triggers intermittent buzzer. Alert pushed to APP.
 * 
 * 5. Speed Measurement: Hall effect sensor measures rotation speed.
 *    Magnet on shaft triggers sensor for RPM calculation.
 * 
 * 6. Status Display: OLED screen shows weight, jam/overweight status,
 *    conveyor state, temperature, speed, and run time.
 * 
 * 7. Remote Control: WiFi + MQTT enables APP control and monitoring.
 *    User can start/stop conveyor, mute buzzer, and view real-time data.
 * 
 * Hardware Connections:
 * =====================
 * - GPIO0, GPIO2: Reserved (System Debug, DO NOT USE)
 * - GPIO1: Active Buzzer (LOW = ON)
 * - GPIO3: TB6612FNG AIN1
 * - GPIO4: TB6612FNG AIN2
 * - GPIO5: TB6612FNG STBY
 * - GPIO6: Hall Effect Sensor (interrupt)
 * - GPIO7: DHT11 Temperature/Humidity Sensor
 * - GPIO8: Infrared Sensor (LOW = object detected)
 * - GPIO9: TB6612FNG PWMA (PWM0)
 * - GPIO10: Servo Motor PWM (PWM1)
 * - GPIO11: HX711 DT (Data)
 * - GPIO12: HX711 SCK (Clock)
 * - GPIO13: I2C0 SDA (OLED)
 * - GPIO14: I2C0 SCL (OLED)
 * 
 * WiFi Configuration:
 * - SSID: zys
 * - Password: 12345678
 * - PC IP: 192.168.43.230
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"

#include "i2c_common.h"
#include "hx711_sensor.h"
#include "dht11_sensor.h"
#include "infrared_sensor.h"
#include "hall_sensor.h"
#include "oled_display.h"
#include "conveyor_belt.h"
#include "mqtt_util.h"

#define MAIN_TASK_STACK_SIZE    4096

static void ConveyorSystem_Init(void *arg)
{
    (void)arg;
    
    printf("\n");
    printf("============================================\n");
    printf("    Smart Conveyor Belt System v1.0\n");
    printf("============================================\n");
    printf("\n");
    
    /* Initialize I2C for OLED display */
    I2C_CommonInit();
    
    /* Start sensor tasks */
    HX711_MainLoop();       /* Pressure sensor for weight */
    DHT11_MainLoop();       /* Temperature sensor for overheat */
    IR_MainLoop();          /* Infrared sensor for jam detection */
    Hall_MainLoop();        /* Hall sensor for speed measurement */
    
    /* Start display task */
    Display_MainLoop();
    
    /* Start main conveyor control task */
    Conveyor_MainLoop();
    
    /* Start MQTT task for remote control */
    mqtt_start_task();
    
    printf("[Main] All tasks started successfully\n");
}

static void MainTask(void)
{
    osThreadAttr_t attr;
    attr.name = "ConveyorSystem_Init";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = MAIN_TASK_STACK_SIZE;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)ConveyorSystem_Init, NULL, &attr) == NULL)
    {
        printf("[Main] Failed to create MainTask!\n");
    }
}

APP_FEATURE_INIT(MainTask);
