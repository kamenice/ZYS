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
 * 1. HX711 pressure sensor - detects item presence on conveyor
 * 2. TB6612 motor driver - controls conveyor belt motor
 *    - AIN1/AIN2 same level: forward, different: brake
 *    - PWMA high: motor runs
 *    - STBY high: chip enabled
 * 3. DHT11 temperature sensor - monitors temperature
 *    - Triggers buzzer and stops motor when temp > 30°C
 * 4. Active buzzer (low-level trigger) - overheat alarm
 * 5. YL-62 infrared sensor - detects items at specific location
 *    - Outputs low when item detected
 *    - Activates servo when triggered
 * 6. Servo motor - controlled by PWM (20ms period, 0.5~2.5ms pulse)
 * 7. OLED display - shows item status, temperature, motor state
 * 8. WiFi + MQTT - sends data to mobile app
 *    - WiFi SSID: zys, Password: 12345678
 *    - MQTT Server: 192.168.43.230
 * 
 * Note: GPIO0 and GPIO2 are reserved for system debugging
 */

#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "conveyor_system.h"
#include "oled_display.h"
#include "wifi_mqtt.h"

#define MAIN_TASK_STACK_SIZE 4096

static void MainTask(void *arg)
{
    (void)arg;

    printf("\n========================================\n");
    printf("   Smart Conveyor Belt System v1.0\n");
    printf("========================================\n\n");

    /* Initialize all subsystems */
    ConveyorSystemInit();
    OledDisplayInit();
    WifiMqttInit();

    /* Start all tasks */
    StartSensorTask();    /* HX711 + DHT11 + Buzzer monitoring */
    StartMotorTask();     /* TB6612 motor control */
    StartServoTask();     /* YL-62 + Servo control */
    StartOledTask();      /* OLED display update */
    StartMqttTask();      /* WiFi + MQTT data publishing */

    printf("[Main] All tasks started\n");
}

static void ConveyorEntry(void)
{
    osThreadAttr_t attr = {0};
    attr.name = "MainTask";
    attr.stack_size = MAIN_TASK_STACK_SIZE;
    attr.priority = osPriorityNormal;

    if (osThreadNew(MainTask, NULL, &attr) == NULL) {
        printf("[Main] Failed to create MainTask!\n");
    }
}

APP_FEATURE_INIT(ConveyorEntry);
