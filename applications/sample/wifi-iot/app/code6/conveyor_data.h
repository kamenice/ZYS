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

#ifndef CONVEYOR_DATA_H
#define CONVEYOR_DATA_H

/**
 * @brief Shared data structure for conveyor belt system
 */
typedef struct {
    int object_detected;        // HX711 pressure sensor: 1 = object detected, 0 = no object
    float temperature;          // DHT11 temperature reading (Celsius)
    float humidity;             // DHT11 humidity reading (%)
    int motor_running;          // TB6612 motor status: 1 = running, 0 = stopped
    int infrared_detected;      // YL-62 infrared sensor: 1 = object detected, 0 = no object
    int over_temperature;       // Temperature alarm: 1 = over 30°C, 0 = normal
} ConveyorData;

/**
 * @brief Get pointer to shared conveyor data
 * @return Pointer to ConveyorData structure
 */
ConveyorData* GetConveyorData(void);

#endif // CONVEYOR_DATA_H
