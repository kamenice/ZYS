/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * Description: Smart Conveyor Belt System - I2C Common Interface
 */

#ifndef I2C_COMMON_H
#define I2C_COMMON_H

#include "cmsis_os2.h"

/* Global I2C mutex for thread-safe access */
extern osMutexId_t g_i2cMutex;

/**
 * @brief Initialize I2C bus with mutex protection
 * This should be called before any I2C operations
 */
void I2C_CommonInit(void);

#endif /* I2C_COMMON_H */
