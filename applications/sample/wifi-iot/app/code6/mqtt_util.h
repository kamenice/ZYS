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

#ifndef MQTT_UTIL_H
#define MQTT_UTIL_H

/**
 * @brief Connect to MQTT broker and publish data
 * MQTT Server: 192.168.43.230
 * MQTT Port: 1883
 * Topic: conveyor_belt
 */
int mqtt_connect(void);

#endif // MQTT_UTIL_H
