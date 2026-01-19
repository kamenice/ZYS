/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * Description: Smart Conveyor Belt System - Buzzer Control
 * 
 * Active buzzer with low-level trigger
 * Used for alerts: overweight, overheating, jam
 * 
 * Pin Configuration:
 * - OUT: GPIO1 (avoiding GPIO0,2 which are system debug pins)
 */

#ifndef BUZZER_CONTROL_H
#define BUZZER_CONTROL_H

/**
 * @brief Initialize buzzer GPIO
 */
void Buzzer_Init(void);

/**
 * @brief Turn buzzer on continuously
 */
void Buzzer_On(void);

/**
 * @brief Turn buzzer off
 */
void Buzzer_Off(void);

/**
 * @brief Beep buzzer for specified duration (blocking)
 * @param duration_ms Duration in milliseconds
 */
void Buzzer_Beep(int duration_ms);

/**
 * @brief Start intermittent beeping (non-blocking)
 * Used for overheating warning
 */
void Buzzer_StartIntermittent(void);

/**
 * @brief Stop intermittent beeping
 */
void Buzzer_StopIntermittent(void);

/**
 * @brief Mute all buzzer sounds
 */
void Buzzer_Mute(void);

/**
 * @brief Unmute buzzer
 */
void Buzzer_Unmute(void);

/**
 * @brief Check if buzzer is muted
 * @return 1 if muted, 0 otherwise
 */
int Buzzer_IsMuted(void);

#endif /* BUZZER_CONTROL_H */
