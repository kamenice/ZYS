/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * Description: Smart Conveyor Belt System - DHT11 Temperature/Humidity Sensor
 * 
 * DHT11 digital temperature and humidity sensor
 * Used for conveyor belt overheating protection
 * Temperature threshold: 30°C
 * 
 * Pin Configuration:
 * - DATA: GPIO7
 */

#ifndef DHT11_SENSOR_H
#define DHT11_SENSOR_H

#include <stdint.h>

/* Temperature threshold for overheating alert (Celsius) */
#define TEMP_OVERHEAT_THRESHOLD     30.0f

/**
 * @brief Initialize DHT11 sensor GPIO pin
 */
void DHT11_Init(void);

/**
 * @brief Read temperature and humidity from DHT11
 * @param temperature Pointer to store temperature value
 * @param humidity Pointer to store humidity value
 * @return 0 on success, -1 on failure
 */
int DHT11_Read(float *temperature, float *humidity);

/**
 * @brief Get current temperature reading
 * @return Temperature in Celsius
 */
float DHT11_GetTemperature(void);

/**
 * @brief Get current humidity reading
 * @return Humidity in percentage
 */
float DHT11_GetHumidity(void);

/**
 * @brief Check if temperature exceeds threshold
 * @return 1 if overheating, 0 otherwise
 */
int DHT11_IsOverheating(void);

/**
 * @brief Start DHT11 sensor task
 */
void DHT11_MainLoop(void);

#endif /* DHT11_SENSOR_H */
