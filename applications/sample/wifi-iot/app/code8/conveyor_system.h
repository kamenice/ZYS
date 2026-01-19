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

#ifndef CONVEYOR_SYSTEM_H
#define CONVEYOR_SYSTEM_H

#include <stdint.h>

/* Global system state structure */
typedef struct {
    int item_detected;       /* Whether item is detected by HX711 */
    float temperature;       /* Temperature from DHT11 */
    int motor_running;       /* Motor status: 0=stopped, 1=running */
    int overheat_alarm;      /* Overheat alarm status */
    int servo_activated;     /* Servo activation status */
} ConveyorState;

/* Get current system state */
ConveyorState *GetConveyorState(void);

/* Initialize all sensors and actuators */
void ConveyorSystemInit(void);

/* Start sensor monitoring task */
void StartSensorTask(void);

/* Start motor control task */
void StartMotorTask(void);

/* Start servo control task */
void StartServoTask(void);

#endif /* CONVEYOR_SYSTEM_H */
