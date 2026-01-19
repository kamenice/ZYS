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

#ifndef WIFI_MQTT_H
#define WIFI_MQTT_H

#include <stdint.h>

/* WiFi Configuration - Modify as needed */
#define WIFI_SSID               "your_wifi_ssid"
#define WIFI_PASSWORD           "your_wifi_password"

/* MQTT Configuration */
#define MQTT_HOST               "192.168.1.100"
#define MQTT_PORT               1883
#define MQTT_CLIENT_ID          "hi3861_conveyor"
#define MQTT_USERNAME           "conveyor"
#define MQTT_PASSWORD           "conveyor123"

/* MQTT Topics */
#define MQTT_TOPIC_STATUS       "conveyor/status"
#define MQTT_TOPIC_CONTROL      "conveyor/control"
#define MQTT_TOPIC_ALERT        "conveyor/alert"
#define MQTT_TOPIC_CONFIG       "conveyor/config"

/* Alert types for MQTT messages */
#define ALERT_OVERWEIGHT        "overweight"
#define ALERT_OVERHEAT          "overheat"
#define ALERT_JAM               "jam"

/* Control commands from APP */
#define CMD_START               "start"
#define CMD_STOP                "stop"
#define CMD_MUTE                "mute"
#define CMD_UNMUTE              "unmute"
#define CMD_CLEAR_JAM           "clear_jam"
#define CMD_SET_WEIGHT_THRESHOLD    "set_weight"
#define CMD_SET_TEMP_THRESHOLD      "set_temp"

/**
 * @brief Initialize WiFi connection
 * @return 0 on success, -1 on failure
 */
int WiFi_Init(void);

/**
 * @brief Connect to WiFi network
 * @return 0 on success, -1 on failure
 */
int WiFi_Connect(void);

/**
 * @brief Check if WiFi is connected
 * @return 1 if connected, 0 otherwise
 */
int WiFi_IsConnected(void);

/**
 * @brief Initialize MQTT client
 * @return 0 on success, -1 on failure
 */
int MQTT_Init(void);

/**
 * @brief Connect to MQTT broker
 * @return 0 on success, -1 on failure
 */
int MQTT_Connect(void);

/**
 * @brief Publish conveyor status to MQTT
 * @param weight Current weight
 * @param temperature Current temperature
 * @param speedRPM Current speed
 * @param isRunning Motor running status
 * @param runTimeSeconds Running time
 */
void MQTT_PublishStatus(double weight, float temperature, 
                        float speedRPM, int isRunning, 
                        uint32_t runTimeSeconds);

/**
 * @brief Publish alert message to MQTT
 * @param alertType Type of alert (overweight/overheat/jam)
 * @param value Associated value
 */
void MQTT_PublishAlert(const char *alertType, float value);

/**
 * @brief Subscribe to control topic
 * @return 0 on success, -1 on failure
 */
int MQTT_SubscribeControl(void);

/**
 * @brief Process incoming MQTT messages
 * This function should be called periodically
 */
void MQTT_ProcessMessages(void);

/**
 * @brief Disconnect from MQTT broker
 */
void MQTT_Disconnect(void);

/**
 * @brief Set weight threshold via MQTT callback
 * @param threshold New threshold value
 */
void MQTT_SetWeightThreshold(double threshold);

/**
 * @brief Set temperature threshold via MQTT callback
 * @param threshold New threshold value
 */
void MQTT_SetTempThreshold(float threshold);

/**
 * @brief Get current weight threshold
 * @return Weight threshold in grams
 */
double MQTT_GetWeightThreshold(void);

/**
 * @brief Get current temperature threshold
 * @return Temperature threshold in Celsius
 */
float MQTT_GetTempThreshold(void);

#endif /* WIFI_MQTT_H */
