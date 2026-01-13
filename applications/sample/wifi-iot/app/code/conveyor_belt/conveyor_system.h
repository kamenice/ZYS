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

/* Button GPIO Pin Definition */
#define BUTTON_GPIO             WIFI_IOT_GPIO_IDX_3   /* User button (GPIO3) - GPIO14 is used for I2C SCL */

/* System default thresholds */
#define DEFAULT_WEIGHT_THRESHOLD    5000.0  /* 5kg default weight threshold */
#define DEFAULT_TEMP_THRESHOLD      60.0f   /* 60°C default temperature threshold */
#define ITEM_DETECT_THRESHOLD       10.0    /* 10g threshold to detect item presence */

/* System state flags */
typedef struct {
    int isRunning;              /* Motor running state */
    int isJammed;               /* Jam detected state */
    int isOverweight;           /* Overweight detected state */
    int isOverheated;           /* Overheat detected state */
    int isBuzzerMuted;          /* Buzzer mute state */
    double currentWeight;       /* Current weight on conveyor */
    float currentTemp;          /* Current temperature */
    float currentHumidity;      /* Current humidity */
    float currentSpeedRPM;      /* Current speed in RPM */
    uint32_t runTimeSeconds;    /* Total running time */
    double weightThreshold;     /* Weight alert threshold */
    float tempThreshold;        /* Temperature alert threshold */
} ConveyorState;

/**
 * @brief Initialize the entire conveyor belt system
 */
void ConveyorSystem_Init(void);

/**
 * @brief Start the conveyor belt
 */
void ConveyorSystem_Start(void);

/**
 * @brief Stop the conveyor belt
 */
void ConveyorSystem_Stop(void);

/**
 * @brief Toggle conveyor belt state (start/stop)
 */
void ConveyorSystem_Toggle(void);

/**
 * @brief Trigger jam clearing procedure (vibration)
 */
void ConveyorSystem_ClearJam(void);

/**
 * @brief Mute all alarms
 */
void ConveyorSystem_MuteAlarm(void);

/**
 * @brief Unmute alarms
 */
void ConveyorSystem_UnmuteAlarm(void);

/**
 * @brief Set weight threshold
 * @param threshold Weight threshold in grams
 */
void ConveyorSystem_SetWeightThreshold(double threshold);

/**
 * @brief Set temperature threshold
 * @param threshold Temperature threshold in Celsius
 */
void ConveyorSystem_SetTempThreshold(float threshold);

/**
 * @brief Get current system state
 * @return Pointer to current state structure
 */
ConveyorState* ConveyorSystem_GetState(void);

/**
 * @brief Main system task - call periodically
 * Reads sensors, updates display, handles alarms, publishes MQTT
 */
void ConveyorSystem_Task(void);

/**
 * @brief Button callback handler
 */
void ConveyorSystem_ButtonCallback(char *arg);

#endif /* CONVEYOR_SYSTEM_H */
