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

/*
 * Smart Conveyor Belt System
 *
 * Features:
 * 1. HX711 pressure sensor - detects if object is present, starts TB6612 motor
 * 2. DHT11 temperature sensor - if temp > 30°C, activate buzzer and stop motor
 * 3. YL-62 infrared sensor - when object detected (low level), activate servo
 * 4. OLED display - shows object status, temperature, motor status
 * 5. WiFi + MQTT - sends data to mobile app via MQTT
 *
 * TB6612 Motor Driver:
 * - AIN1/AIN2 same level = motor runs
 * - AIN1/AIN2 different level = motor brakes
 * - PWMA high = motor works
 * - STBY high = chip enabled
 *
 * Active Buzzer:
 * - Low level triggers the buzzer
 */

#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_i2c.h"

#include "conveyor_data.h"
#include "hx711.h"
#include "dht11.h"
#include "tb6612.h"
#include "buzzer.h"
#include "infrared.h"
#include "servo.h"
#include "oled_ssd1306.h"
#include "wifi_util.h"
#include "mqtt_util.h"

// Task stack sizes
#define SENSOR_TASK_STACK_SIZE  4096
#define OLED_TASK_STACK_SIZE    4096
#define MQTT_TASK_STACK_SIZE    4096
#define MAIN_TASK_STACK_SIZE    4096

// I2C configuration
#define I2C_BAUDRATE (400 * 1000)

/**
 * @brief Initialize all hardware components
 */
static void Hardware_Init(void)
{
    // Initialize GPIO
    GpioInit();

    // Initialize I2C for OLED
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_13, WIFI_IOT_IO_FUNC_GPIO_13_I2C0_SDA);
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_14, WIFI_IOT_IO_FUNC_GPIO_14_I2C0_SCL);
    I2cInit(WIFI_IOT_I2C_IDX_0, I2C_BAUDRATE);

    // Initialize all modules
    HX711_Init();
    DHT11_Init();
    TB6612_Init();
    Buzzer_Init();
    Infrared_Init();
    Servo_Init();
    OledInit();

    printf("[Main] Hardware initialized\n");
}

/**
 * @brief Sensor reading and control task
 * Reads sensors and controls motor, buzzer, servo based on sensor data
 */
static void Sensor_Task(void *arg)
{
    (void)arg;
    ConveyorData *data = GetConveyorData();
    float temperature = 0.0f;
    float humidity = 0.0f;

    printf("[Sensor Task] Started\n");

    while (1) {
        // 1. Read HX711 pressure sensor (object detection)
        data->object_detected = HX711_IsObjectDetected();

        // 2. Read DHT11 temperature sensor
        if (DHT11_Read(&temperature, &humidity) == 0) {
            data->temperature = temperature;
            data->humidity = humidity;
            data->over_temperature = DHT11_IsOverTemperature();
        }

        // 3. Read YL-62 infrared sensor
        data->infrared_detected = Infrared_IsObjectDetected();

        // 4. Control logic

        // Check temperature first - if over 30°C, stop everything and alarm
        if (data->over_temperature) {
            // Temperature too high - stop motor and activate buzzer
            if (data->motor_running) {
                TB6612_Stop();
                data->motor_running = 0;
            }
            Buzzer_On();
        } else {
            // Temperature normal - turn off buzzer
            Buzzer_Off();

            // Control motor based on HX711 pressure sensor
            if (data->object_detected) {
                // Object detected - start motor if not running
                if (!data->motor_running) {
                    TB6612_Start();
                    data->motor_running = 1;
                }
            } else {
                // No object - stop motor if running
                if (data->motor_running) {
                    TB6612_Stop();
                    data->motor_running = 0;
                }
            }
        }

        // Control servo based on YL-62 infrared sensor
        if (data->infrared_detected) {
            // Object detected by infrared - activate servo
            Servo_Activate();
        } else {
            // No object - deactivate servo
            Servo_Deactivate();
        }

        // Update motor status
        data->motor_running = TB6612_IsRunning();

        // Print status
        printf("[Sensor] Object:%d Temp:%.1f Hum:%.1f Motor:%d IR:%d OverTemp:%d\n",
               data->object_detected, data->temperature, data->humidity,
               data->motor_running, data->infrared_detected, data->over_temperature);

        sleep(1);
    }
}

/**
 * @brief OLED display task
 * Shows object status, temperature, and motor status on OLED screen
 */
static void OLED_Task(void *arg)
{
    (void)arg;
    ConveyorData *data = GetConveyorData();
    char line[32] = {0};

    printf("[OLED Task] Started\n");

    // Clear screen
    OledFillScreen(0x00);

    // Show title
    OledShowString(8, 0, "Smart Conveyor", FONT6x8);

    while (1) {
        // Line 1: Object status
        if (data->object_detected) {
            OledShowString(0, 2, "Object: YES    ", FONT6x8);
        } else {
            OledShowString(0, 2, "Object: NO     ", FONT6x8);
        }

        // Line 2: Temperature
        snprintf(line, sizeof(line), "Temp: %.1f C   ", data->temperature);
        OledShowString(0, 3, line, FONT6x8);

        // Line 3: Humidity
        snprintf(line, sizeof(line), "Humi: %.1f %%  ", data->humidity);
        OledShowString(0, 4, line, FONT6x8);

        // Line 4: Motor status
        if (data->motor_running) {
            OledShowString(0, 5, "Motor: RUNNING ", FONT6x8);
        } else {
            OledShowString(0, 5, "Motor: STOPPED ", FONT6x8);
        }

        // Line 5: Temperature alarm
        if (data->over_temperature) {
            OledShowString(0, 6, "ALARM: OVERHEAT", FONT6x8);
        } else {
            OledShowString(0, 6, "Status: NORMAL ", FONT6x8);
        }

        // Line 6: Infrared status
        if (data->infrared_detected) {
            OledShowString(0, 7, "IR: DETECTED   ", FONT6x8);
        } else {
            OledShowString(0, 7, "IR: CLEAR      ", FONT6x8);
        }

        usleep(500000); // Update every 500ms
    }
}

/**
 * @brief MQTT publishing task
 * Connects to WiFi and publishes data via MQTT
 */
static void MQTT_Task(void *arg)
{
    (void)arg;

    printf("[MQTT Task] Started\n");

    // Wait for system to stabilize
    sleep(5);

    // Connect to WiFi
    printf("[MQTT Task] Connecting to WiFi...\n");
    connect_wifi();

    // Wait for WiFi connection
    sleep(3);

    // Connect to MQTT and start publishing
    printf("[MQTT Task] Starting MQTT connection...\n");
    mqtt_connect();
}

/**
 * @brief Main entry task
 */
static void Main_Task(void *arg)
{
    (void)arg;
    osThreadAttr_t attr;

    printf("\n========================================\n");
    printf("   Smart Conveyor Belt System\n");
    printf("========================================\n\n");

    // Wait for system startup
    sleep(2);

    // Initialize hardware
    Hardware_Init();

    // Create sensor task
    attr.name = "sensor_task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = SENSOR_TASK_STACK_SIZE;
    attr.priority = osPriorityNormal;

    if (osThreadNew(Sensor_Task, NULL, &attr) == NULL) {
        printf("[Main] Failed to create Sensor_Task!\n");
    }

    // Create OLED task
    attr.name = "oled_task";
    attr.stack_size = OLED_TASK_STACK_SIZE;
    attr.priority = osPriorityBelowNormal;

    if (osThreadNew(OLED_Task, NULL, &attr) == NULL) {
        printf("[Main] Failed to create OLED_Task!\n");
    }

    // Create MQTT task
    attr.name = "mqtt_task";
    attr.stack_size = MQTT_TASK_STACK_SIZE;
    attr.priority = osPriorityBelowNormal;

    if (osThreadNew(MQTT_Task, NULL, &attr) == NULL) {
        printf("[Main] Failed to create MQTT_Task!\n");
    }

    printf("[Main] All tasks created successfully\n");
}

/**
 * @brief System entry point
 */
static void Conveyor_Entry(void)
{
    osThreadAttr_t attr;

    attr.name = "main_task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = MAIN_TASK_STACK_SIZE;
    attr.priority = osPriorityAboveNormal;

    if (osThreadNew(Main_Task, NULL, &attr) == NULL) {
        printf("[Conveyor] Failed to create Main_Task!\n");
    }
}

SYS_RUN(Conveyor_Entry);
