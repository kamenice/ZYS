/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * Description: Smart Conveyor Belt System - OLED Display Module
 * 
 * Displays system status:
 * - Current weight on conveyor belt
 * - Jam/overweight status
 * - Conveyor belt running status
 * - Shaft temperature
 * - Belt speed (RPM)
 * - Run time
 */

#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

/**
 * @brief Initialize OLED display
 */
void Display_Init(void);

/**
 * @brief Update OLED display with current status
 */
void Display_Update(void);

/**
 * @brief Start OLED display task
 */
void Display_MainLoop(void);

#endif /* OLED_DISPLAY_H */
