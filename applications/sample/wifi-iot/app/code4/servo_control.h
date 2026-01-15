/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * Description: Smart Conveyor Belt System - Servo Motor Control
 * 
 * Servo motor for anti-jam vibration function
 * PWM control: 20ms period, 0.5ms-2.5ms high pulse for angle control
 * - 0.5ms = 0 degrees
 * - 1.5ms = 90 degrees (center)
 * - 2.5ms = 180 degrees
 * 
 * Pin Configuration:
 * - PWM: GPIO10 (PWM1)
 */

#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

/**
 * @brief Initialize servo motor PWM
 */
void Servo_Init(void);

/**
 * @brief Set servo angle
 * @param angle Angle in degrees (0-180)
 */
void Servo_SetAngle(int angle);

/**
 * @brief Vibrate servo to shake conveyor belt
 * Alternates between two angles rapidly
 */
void Servo_Vibrate(void);

/**
 * @brief Stop servo vibration
 */
void Servo_Stop(void);

/**
 * @brief Check if servo is currently vibrating
 * @return 1 if vibrating, 0 otherwise
 */
int Servo_IsVibrating(void);

#endif /* SERVO_CONTROL_H */
