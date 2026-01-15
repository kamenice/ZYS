/*
 * i2c_common.h - I2C共享资源和互斥锁管理
 */

#ifndef I2C_COMMON_H
#define I2C_COMMON_H

#include "cmsis_os2.h"

// 全局I2C互斥锁
extern osMutexId_t g_i2cMutex;

// I2C初始化（含互斥锁创建）
void I2C_CommonInit(void);

#endif
