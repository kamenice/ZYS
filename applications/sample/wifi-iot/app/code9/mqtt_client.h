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

#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

/**
 * @brief Initialize and connect to MQTT broker
 * Server: 192.168.43.230
 * Port: 1883
 * @return Socket descriptor on success, negative value on failure
 */
int MQTT_Connect(void);

/**
 * @brief Publish conveyor belt status data
 * @param hasItem Whether item is detected (1 or 0)
 * @param temperature Current temperature in Celsius
 * @param motorState Motor state (0: stopped, 1: running)
 */
void MQTT_PublishStatus(int hasItem, float temperature, int motorState);

/**
 * @brief Disconnect from MQTT broker
 */
void MQTT_Disconnect(void);

#endif /* MQTT_CLIENT_H */
