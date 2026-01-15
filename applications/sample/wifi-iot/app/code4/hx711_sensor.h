/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * Description: Smart Conveyor Belt System - HX711 Pressure Sensor
 * 
 * HX711 24-bit ADC for load cell/pressure sensor
 * Used for weight measurement (0-10kg range) and conveyor start detection
 * 
 * Pin Configuration:
 * - DT (Data): GPIO11
 * - SCK (Clock): GPIO12
 */

#ifndef HX711_SENSOR_H
#define HX711_SENSOR_H

#include <stdint.h>

/* Weight threshold for overweight detection (kg) */
#define WEIGHT_OVERLOAD_THRESHOLD   8.0f
/* Weight threshold to detect item presence on conveyor (kg) */
#define WEIGHT_PRESENCE_THRESHOLD   0.1f

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
 * @brief Get calibrated weight reading
 * @return Weight in grams
 */
float HX711_GetWeight(void);

/**
 * @brief Set baseline (tare) weight
 */
void HX711_Tare(void);

/**
 * @brief Check if item is present on conveyor
 * @return 1 if item detected, 0 otherwise
 */
int HX711_IsItemPresent(void);

/**
 * @brief Check if current weight exceeds threshold
 * @return 1 if overweight, 0 otherwise
 */
int HX711_IsOverweight(void);

/**
 * @brief Start HX711 sensor task
 */
void HX711_MainLoop(void);

#endif /* HX711_SENSOR_H */
