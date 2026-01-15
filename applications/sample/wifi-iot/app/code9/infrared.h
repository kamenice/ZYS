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

/**
 * @brief Initialize YL-62 infrared sensor
 * YL-62 outputs LOW when obstacle is detected
 */
void Infrared_Init(void);

/**
 * @brief Check if obstacle is detected by infrared sensor
 * @return 1 if obstacle detected (LOW signal), 0 otherwise
 */
int Infrared_IsObstacleDetected(void);

#endif /* INFRARED_H */
