/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * Description: Smart Conveyor Belt System - Infrared Sensor
 * 
 * IR sensor for detecting item pile-up/jam on conveyor belt
 * Low level output when object is detected
 * 
 * Pin Configuration:
 * - OUT: GPIO8
 */

#ifndef INFRARED_SENSOR_H
#define INFRARED_SENSOR_H

/**
 * @brief Initialize infrared sensor GPIO pin
 */
void IR_Init(void);

/**
 * @brief Check if item pile-up/jam is detected
 * @return 1 if jam detected (low level), 0 otherwise
 */
int IR_IsJamDetected(void);

/**
 * @brief Start infrared sensor monitoring task
 */
void IR_MainLoop(void);

#endif /* INFRARED_SENSOR_H */
