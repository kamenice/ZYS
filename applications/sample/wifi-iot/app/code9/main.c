/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * Smart Conveyor Belt System
 *
 * Features:
 * 1. HX711 pressure sensor detects items, activates TB6612 motor driver
 * 2. DHT11 temperature sensor monitors temperature, activates buzzer and stops motor when >30°C
 * 3. YL-62 infrared sensor detects items, activates servo motor with PWM
 * 4. OLED display shows: item status, temperature, motor state
 * 5. WiFi + MQTT sends data to mobile app (WiFi: zys/12345678, Server: 192.168.43.230)
 *
 * GPIO Assignment (GPIO0/2 reserved for debugging):
 * - GPIO3:  TB6612 AIN1
 * - GPIO4:  TB6612 AIN2
 * - GPIO5:  TB6612 PWMA
 * - GPIO6:  TB6612 STBY
 * - GPIO7:  DHT11 Data
 * - GPIO8:  Buzzer (active low)
 * - GPIO9:  Servo PWM (PWM0)
 * - GPIO10: YL-62 Infrared
 * - GPIO11: HX711 DT (Data)
 * - GPIO12: HX711 SCK (Clock)
 * - GPIO13: OLED SDA (I2C0)
 * - GPIO14: OLED SCL (I2C0)
 */

#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"

#include "hx711.h"
#include "dht11.h"
#include "motor.h"
#include "buzzer.h"
#include "infrared.h"
#include "servo.h"
#include "oled_ssd1306.h"
#include "wifi_connect.h"
#include "mqtt_client.h"

/* Global state variables */
static int g_hasItem = 0;           /* Item detected by pressure sensor */
static float g_temperature = 0.0f;  /* Current temperature */
static int g_motorRunning = 0;      /* Motor state: 0=stopped, 1=running */
static int g_overTemp = 0;          /* Over temperature flag */
static int g_infraredItem = 0;      /* Item detected by infrared sensor */

/* Update OLED display with current status */
static void UpdateDisplay(void)
{
    char buf[32];

    /* Line 0: Item status */
    if (g_hasItem) {
        OledShowString(0, 0, "Item: YES   ", FONT8x16);
    } else {
        OledShowString(0, 0, "Item: NO    ", FONT8x16);
    }

    /* Line 2: Temperature */
    snprintf(buf, sizeof(buf), "Temp: %.1fC  ", g_temperature);
    OledShowString(0, 2, buf, FONT8x16);

    /* Line 4: Motor status */
    if (g_motorRunning) {
        OledShowString(0, 4, "Motor: RUN  ", FONT8x16);
    } else {
        OledShowString(0, 4, "Motor: STOP ", FONT8x16);
    }

    /* Line 6: Alarm status */
    if (g_overTemp) {
        OledShowString(0, 6, "ALARM: HIGH!", FONT8x16);
    } else {
        OledShowString(0, 6, "Alarm: OK   ", FONT8x16);
    }
}

/* Sensor monitoring task */
static void SensorTask(void *arg)
{
    (void)arg;
    float humidity;

    printf("[SensorTask] Starting sensor monitoring\n");

    while (1) {
        /* Read HX711 pressure sensor - detect items */
        g_hasItem = HX711_IsItemDetected();

        /* Read DHT11 temperature sensor */
        if (DHT11_Read(&g_temperature, &humidity) == 0) {
            g_overTemp = DHT11_IsOverTemperature();
        }

        /* Read YL-62 infrared sensor */
        g_infraredItem = Infrared_IsObstacleDetected();

        /* Control logic */

        /* 1. Motor control based on pressure sensor and temperature */
        if (g_overTemp) {
            /* Temperature too high: stop motor and activate buzzer */
            if (g_motorRunning) {
                Motor_Stop();
                g_motorRunning = 0;
            }
            Buzzer_On();
        } else {
            Buzzer_Off();
            /* Start motor if item detected */
            if (g_hasItem && !g_motorRunning) {
                Motor_Start();
                g_motorRunning = 1;
            } else if (!g_hasItem && g_motorRunning) {
                Motor_Stop();
                g_motorRunning = 0;
            }
        }

        /* 2. Servo control based on infrared sensor */
        if (g_infraredItem) {
            /* Item detected by infrared: open servo */
            Servo_Open();
        } else {
            /* No item: close servo */
            Servo_Close();
        }

        /* Update display */
        UpdateDisplay();

        printf("[SensorTask] Item:%d, Temp:%.1f, Motor:%d, IR:%d\n",
               g_hasItem, g_temperature, g_motorRunning, g_infraredItem);

        sleep(1);
    }
}

/* MQTT publishing task */
static void MqttTask(void *arg)
{
    (void)arg;

    printf("[MqttTask] Starting MQTT task\n");

    /* Wait for WiFi connection to complete
     * WiFi connection typically takes 5-8 seconds (including DHCP)
     * We wait 10 seconds to ensure stable connection before MQTT
     */
    while (!WiFi_IsConnected()) {
        printf("[MqttTask] Waiting for WiFi connection...\n");
        sleep(2);
    }
    sleep(2); /* Additional delay for connection stabilization */

    /* Connect to MQTT broker */
    if (MQTT_Connect() < 0) {
        printf("[MqttTask] Failed to connect to MQTT broker\n");
        return;
    }

    while (1) {
        /* Publish status every 2 seconds */
        MQTT_PublishStatus(g_hasItem, g_temperature, g_motorRunning);
        sleep(2);
    }
}

/* WiFi connection task */
static void WiFiTask(void *arg)
{
    (void)arg;

    printf("[WiFiTask] Starting WiFi connection\n");
    sleep(3);  /* Wait for system initialization */

    WiFi_Connect();
}

/* Main initialization function */
static void ConveyorInit(void)
{
    osThreadAttr_t attr;

    printf("========================================\n");
    printf("  Smart Conveyor Belt System Starting\n");
    printf("========================================\n");

    /* Initialize GPIO */
    GpioInit();

    /* Initialize all sensors and actuators */
    HX711_Init();
    DHT11_Init();
    Motor_Init();
    Buzzer_Init();
    Infrared_Init();
    Servo_Init();
    OledInit();

    /* Clear and initialize OLED display */
    OledFillScreen(0x00);
    OledShowString(0, 0, "Conveyor Belt", FONT8x16);
    OledShowString(0, 2, "System Init...", FONT8x16);

    /* Create WiFi task */
    attr.name = "WiFiTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)WiFiTask, NULL, &attr) == NULL) {
        printf("[Main] Failed to create WiFiTask\n");
    }

    /* Create sensor monitoring task */
    attr.name = "SensorTask";
    attr.stack_size = 4096;
    attr.priority = osPriorityAboveNormal;

    if (osThreadNew((osThreadFunc_t)SensorTask, NULL, &attr) == NULL) {
        printf("[Main] Failed to create SensorTask\n");
    }

    /* Create MQTT task */
    attr.name = "MqttTask";
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)MqttTask, NULL, &attr) == NULL) {
        printf("[Main] Failed to create MqttTask\n");
    }

    printf("[Main] All tasks created successfully\n");
}

SYS_RUN(ConveyorInit);
