/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * Description: Smart Conveyor Belt System - Motor Driver (TB6612FNG)
 * 
 * TB6612FNG dual motor driver
 * Using only one motor channel (A)
 * 
 * Pin Configuration:
 * - PWMA: GPIO9 (PWM0) - Speed control
 * - AIN1: GPIO3 - Direction control 1
 * - AIN2: GPIO4 - Direction control 2
 * - STBY: GPIO5 - Standby control (HIGH=active, LOW=standby)
 * 
 * Operation:
 * - AIN1=HIGH, AIN2=LOW: Forward rotation
 * - AIN1=LOW, AIN2=HIGH: Reverse rotation
 * - AIN1=AIN2: Brake
 * - STBY=HIGH: Normal operation
 * - STBY=LOW: Standby mode
 */

#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include <stdint.h>

/**
 * @brief Initialize motor driver GPIOs
 */
void Motor_Init(void);

/**
 * @brief Start conveyor belt motor
 * @param speed Speed percentage (0-100)
 */
void Motor_Start(uint8_t speed);

/**
 * @brief Stop conveyor belt motor
 */
void Motor_Stop(void);

/**
 * @brief Set motor speed without changing running state
 * @param speed Speed percentage (0-100)
 */
void Motor_SetSpeed(uint8_t speed);

/**
 * @brief Check if motor is running
 * @return 1 if running, 0 otherwise
 */
int Motor_IsRunning(void);

/**
 * @brief Get current motor speed setting
 * @return Speed percentage (0-100)
 */
uint8_t Motor_GetSpeed(void);

#endif /* MOTOR_DRIVER_H */
