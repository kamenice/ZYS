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

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_errno.h"

#include "conveyor_system.h"
#include "hx711.h"
#include "dht11.h"
#include "infrared.h"
#include "hall_sensor.h"
#include "motor.h"
#include "servo.h"
#include "buzzer.h"
#include "oled_display.h"
#include "wifi_mqtt.h"

/* System state */
static ConveyorState g_state = {
    .isRunning = 0,
    .isJammed = 0,
    .isOverweight = 0,
    .isOverheated = 0,
    .isBuzzerMuted = 0,
    .currentWeight = 0,
    .currentTemp = 0,
    .currentHumidity = 0,
    .currentSpeedRPM = 0,
    .runTimeSeconds = 0,
    .weightThreshold = DEFAULT_WEIGHT_THRESHOLD,
    .tempThreshold = DEFAULT_TEMP_THRESHOLD,
};

/* Previous alarm states for edge detection */
static int g_prevOverweight = 0;
static int g_prevOverheat = 0;
static int g_prevJam = 0;

void ConveyorSystem_Init(void)
{
    printf("[ConveyorSystem] Initializing...\r\n");

    /* Initialize GPIO */
    GpioInit();

    /* Initialize all sensors */
    HX711_Init();
    DHT11_Init();
    Infrared_Init();
    HallSensor_Init();

    /* Initialize actuators */
    Motor_Init();
    Servo_Init();
    Buzzer_Init();

    /* Initialize display */
    OLED_DisplayInit();

    /* Initialize button for manual control */
    IoSetFunc(BUTTON_GPIO, WIFI_IOT_IO_FUNC_GPIO_14_GPIO);
    GpioSetDir(BUTTON_GPIO, WIFI_IOT_GPIO_DIR_IN);
    GpioRegisterIsrFunc(BUTTON_GPIO,
                        WIFI_IOT_INT_TYPE_EDGE,
                        WIFI_IOT_GPIO_EDGE_FALL_LEVEL_LOW,
                        ConveyorSystem_ButtonCallback,
                        NULL);

    /* Calibrate weight sensor */
    HX711_Calibrate();

    /* Initialize WiFi and MQTT */
    if (WiFi_Init() == 0) {
        if (WiFi_Connect() == 0) {
            MQTT_Init();
            if (MQTT_Connect() == 0) {
                MQTT_SubscribeControl();
            }
        }
    }

    printf("[ConveyorSystem] Initialization complete\r\n");
}

void ConveyorSystem_Start(void)
{
    if (!g_state.isRunning) {
        Motor_Start();
        g_state.isRunning = 1;
        HallSensor_ResetPulseCount();
        printf("[ConveyorSystem] Started\r\n");
    }
}

void ConveyorSystem_Stop(void)
{
    if (g_state.isRunning) {
        Motor_Stop();
        g_state.isRunning = 0;
        printf("[ConveyorSystem] Stopped\r\n");
    }
}

void ConveyorSystem_Toggle(void)
{
    if (g_state.isRunning) {
        ConveyorSystem_Stop();
    } else {
        ConveyorSystem_Start();
    }
}

void ConveyorSystem_ClearJam(void)
{
    printf("[ConveyorSystem] Clearing jam...\r\n");

    /* Vibrate servo to shake loose jammed items */
    Servo_Vibrate();

    /* Reset jam detection */
    Infrared_ResetJamDetection();
    g_state.isJammed = 0;

    printf("[ConveyorSystem] Jam cleared\r\n");
}

void ConveyorSystem_MuteAlarm(void)
{
    Buzzer_Mute();
    g_state.isBuzzerMuted = 1;
}

void ConveyorSystem_UnmuteAlarm(void)
{
    Buzzer_Unmute();
    g_state.isBuzzerMuted = 0;
}

void ConveyorSystem_SetWeightThreshold(double threshold)
{
    g_state.weightThreshold = threshold;
    MQTT_SetWeightThreshold(threshold);
    printf("[ConveyorSystem] Weight threshold set to %.1fg\r\n", threshold);
}

void ConveyorSystem_SetTempThreshold(float threshold)
{
    g_state.tempThreshold = threshold;
    MQTT_SetTempThreshold(threshold);
    printf("[ConveyorSystem] Temperature threshold set to %.1fC\r\n", threshold);
}

ConveyorState* ConveyorSystem_GetState(void)
{
    return &g_state;
}

void ConveyorSystem_ButtonCallback(char *arg)
{
    (void)arg;
    /* Toggle conveyor on button press */
    ConveyorSystem_Toggle();
}

void ConveyorSystem_Task(void)
{
    static uint32_t lastSpeedUpdate = 0;
    static uint32_t lastStatusPublish = 0;
    static uint32_t taskCounter = 0;

    taskCounter++;

    /* Read weight sensor */
    g_state.currentWeight = HX711_GetWeight();
    g_state.isOverweight = (g_state.currentWeight > g_state.weightThreshold) ? 1 : 0;

    /* Read temperature/humidity sensor (every 2 seconds - DHT11 minimum interval) */
    if (taskCounter % 4 == 0) {
        float temp, hum;
        if (DHT11_Read(&temp, &hum) == 0) {
            g_state.currentTemp = temp;
            g_state.currentHumidity = hum;
        }
    }
    g_state.isOverheated = (g_state.currentTemp > g_state.tempThreshold) ? 1 : 0;

    /* Check for jam */
    g_state.isJammed = Infrared_IsJammed();

    /* Update speed calculation (every second) */
    if (taskCounter - lastSpeedUpdate >= 2) {
        HallSensor_UpdateSpeed();
        g_state.currentSpeedRPM = HallSensor_GetSpeedRPM();
        lastSpeedUpdate = taskCounter;
    }

    /* Update running time if motor is running */
    if (g_state.isRunning && taskCounter % 2 == 0) {
        g_state.runTimeSeconds++;
    }

    /* Auto-start when item detected */
    if (!g_state.isRunning && !g_state.isOverheated &&
        g_state.currentWeight > ITEM_DETECT_THRESHOLD) {
        ConveyorSystem_Start();
    }

    /* Handle overweight condition */
    if (g_state.isOverweight) {
        /* Continuous buzzer alarm for overweight */
        Buzzer_Alarm(ALARM_OVERWEIGHT_PATTERN);

        /* Send MQTT alert on rising edge */
        if (!g_prevOverweight) {
            MQTT_PublishAlert(ALERT_OVERWEIGHT, g_state.currentWeight);
        }
    }
    g_prevOverweight = g_state.isOverweight;

    /* Handle overheat condition */
    if (g_state.isOverheated) {
        /* Stop conveyor on overheat */
        ConveyorSystem_Stop();

        /* Intermittent buzzer alarm for overheat */
        Buzzer_Alarm(ALARM_OVERHEAT_PATTERN);

        /* Send MQTT alert on rising edge */
        if (!g_prevOverheat) {
            MQTT_PublishAlert(ALERT_OVERHEAT, g_state.currentTemp);
        }
    }
    g_prevOverheat = g_state.isOverheated;

    /* Handle jam condition */
    if (g_state.isJammed) {
        /* Automatically try to clear jam with vibration */
        ConveyorSystem_ClearJam();

        /* Send MQTT alert on rising edge */
        if (!g_prevJam) {
            MQTT_PublishAlert(ALERT_JAM, 1.0f);
        }
    }
    g_prevJam = g_state.isJammed;

    /* Stop buzzer if no alarms and not muted */
    if (!g_state.isOverweight && !g_state.isOverheated && !g_state.isJammed) {
        Buzzer_StopAlarm();
    }

    /* Update OLED display */
    OLED_DisplayUpdate(g_state.currentWeight, g_state.isOverweight,
                       g_state.isRunning, g_state.isJammed,
                       g_state.currentTemp, g_state.isOverheated,
                       g_state.currentSpeedRPM, g_state.runTimeSeconds);

    /* Publish status via MQTT (every 2 seconds) */
    if (taskCounter - lastStatusPublish >= 4) {
        MQTT_PublishStatus(g_state.currentWeight, g_state.currentTemp,
                          g_state.currentSpeedRPM, g_state.isRunning,
                          g_state.runTimeSeconds);
        lastStatusPublish = taskCounter;
    }

    /* Process incoming MQTT messages */
    MQTT_ProcessMessages();
}
