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

#ifndef INFRARED_H
#define INFRARED_H

#include <stdint.h>

/* YL-62 Infrared Obstacle Sensor GPIO Pin Definition */
/* Note: GPIO4 conflicts with UART0_RXD (debug serial), so we use GPIO8 instead */
#define INFRARED_GPIO           WIFI_IOT_GPIO_IDX_8   /* Digital input pin (GPIO8) */

/* Jam detection parameters */
#define JAM_DETECTION_COUNT     3       /* Number of consecutive detections to confirm jam */
#define JAM_DETECTION_INTERVAL  500     /* Detection interval in milliseconds */

/**
 * @brief Initialize infrared sensor (YL-62)
 */
void Infrared_Init(void);

/**
 * @brief Check if obstacle is detected
 * @return 1 if obstacle detected, 0 otherwise
 */
int Infrared_IsObstacleDetected(void);

/**
 * @brief Check if items are jammed/piled up
 * @return 1 if jam detected, 0 otherwise
 */
int Infrared_IsJammed(void);

/**
 * @brief Reset jam detection counter
 */
void Infrared_ResetJamDetection(void);

#endif /* INFRARED_H */
