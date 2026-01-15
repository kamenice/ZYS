/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * Description: Smart Conveyor Belt System - Hall Effect Sensor
 * 
 * Hall effect sensor (ES3144) for measuring conveyor belt rotation speed
 * Magnet on rotation shaft triggers sensor when passing by
 * Reference: 1.c file for hall sensor with 51 MCU
 * 
 * Pin Configuration:
 * - OUT: GPIO6 (with interrupt capability)
 */

#ifndef HALL_SENSOR_H
#define HALL_SENSOR_H

#include <stdint.h>

/**
 * @brief Initialize Hall effect sensor
 */
void Hall_Init(void);

/**
 * @brief Get current rotation speed
 * @return Rotation speed in RPM
 */
uint32_t Hall_GetRPM(void);

/**
 * @brief Start Hall sensor monitoring task
 */
void Hall_MainLoop(void);

#endif /* HALL_SENSOR_H */
