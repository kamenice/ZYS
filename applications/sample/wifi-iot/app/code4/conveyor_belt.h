/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * Description: Smart Conveyor Belt System - Main Control Logic
 * 
 * Features:
 * 1. Auto-start when item detected via pressure sensor
 * 2. Anti-jam vibration using servo motor
 * 3. Overweight detection and alarm
 * 4. Overheat protection with temperature monitoring
 * 5. Speed measurement via Hall sensor
 * 6. Remote control via MQTT/WiFi
 */

#ifndef CONVEYOR_BELT_H
#define CONVEYOR_BELT_H

#include <stdint.h>

/**
 * @brief Initialize all conveyor belt subsystems
 */
void Conveyor_Init(void);

/**
 * @brief Start conveyor belt
 */
void Conveyor_Start(void);

/**
 * @brief Stop conveyor belt
 */
void Conveyor_Stop(void);

/**
 * @brief Manually trigger vibration to clear jam
 */
void Conveyor_ManualVibrate(void);

/**
 * @brief Get total run time in seconds
 * @return Run time in seconds
 */
uint32_t Conveyor_GetRunTime(void);

/**
 * @brief Start main control loop task
 */
void Conveyor_MainLoop(void);

#endif /* CONVEYOR_BELT_H */
