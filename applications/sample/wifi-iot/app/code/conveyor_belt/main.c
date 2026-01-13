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
 * @file main.c
 * @brief Main entry point for the Conveyor Belt Intelligent System
 *
 * This system implements a smart conveyor belt controller for the Hi3861 chip with:
 * - Auto-start via HX711 pressure sensor when items are placed on belt
 * - Button and APP control for manual start/stop
 * - Jam detection using YL-62 infrared sensor with SG90 servo vibration clearing
 * - Overweight detection with buzzer alarm and MQTT notification
 * - Overheat protection using DHT11 temperature sensor
 * - Speed measurement using 3411 Hall effect sensor
 * - OLED status display
 * - WiFi/MQTT remote monitoring and control
 */

#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"

#include "conveyor_system.h"

#define MAIN_TASK_STACK_SIZE    8192
#define MAIN_TASK_PRIORITY      osPriorityNormal
#define TASK_INTERVAL_MS        500

/**
 * @brief Main conveyor system task
 *
 * This task runs continuously to:
 * - Initialize all system components
 * - Periodically read sensors
 * - Update display
 * - Handle alarms
 * - Process MQTT messages
 */
static void ConveyorMainTask(void *arg)
{
    (void)arg;

    printf("==========================================\r\n");
    printf("  Conveyor Belt Intelligent System\r\n");
    printf("  Platform: Hi3861\r\n");
    printf("==========================================\r\n");

    /* Initialize the conveyor belt system */
    ConveyorSystem_Init();

    printf("[Main] System ready. Starting main loop...\r\n");

    /* Main loop */
    while (1) {
        /* Execute main system task */
        ConveyorSystem_Task();

        /* Task interval delay */
        usleep(TASK_INTERVAL_MS * 1000);
    }
}

/**
 * @brief Application entry point
 *
 * Creates the main conveyor system task thread
 */
static void ConveyorApp(void)
{
    osThreadAttr_t attr;

    attr.name = "ConveyorMainTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = MAIN_TASK_STACK_SIZE;
    attr.priority = MAIN_TASK_PRIORITY;

    if (osThreadNew((osThreadFunc_t)ConveyorMainTask, NULL, &attr) == NULL) {
        printf("[Main] Failed to create ConveyorMainTask!\r\n");
    }
}

APP_FEATURE_INIT(ConveyorApp);
