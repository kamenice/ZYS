/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * Description: Smart Conveyor Belt System - HX711 Pressure Sensor
 * 
 * HX711 24-bit ADC for load cell/pressure sensor
 * Simplified version: Only detects presence of items (yes/no), no weight calculation
 * 
 * Pin Configuration:
 * - DT (Data): GPIO11
 * - SCK (Clock): GPIO12
 */

#ifndef HX711_SENSOR_H
#define HX711_SENSOR_H

#include <stdint.h>

/**
 * @brief Initialize HX711 sensor GPIO pins
 */
void HX711_Init(void);

/**
 * @brief Read raw 24-bit value from HX711
 * @return Raw ADC value
 */
unsigned long HX711_ReadRaw(void);

/**
 * @brief Get weight reading (disabled - always returns 0)
 * @return 0 (weight calculation disabled)
 */
float HX711_GetWeight(void);

/**
 * @brief Set baseline (tare) value
 */
void HX711_Tare(void);

/**
 * @brief Check if item is present on conveyor
 * @return 1 if item detected, 0 otherwise
 */
int HX711_IsItemPresent(void);

/**
 * @brief Check if current weight exceeds threshold (disabled)
 * @return 0 (overweight detection disabled)
 */
int HX711_IsOverweight(void);

/**
 * @brief Start HX711 sensor task
 */
void HX711_MainLoop(void);

#endif /* HX711_SENSOR_H */
