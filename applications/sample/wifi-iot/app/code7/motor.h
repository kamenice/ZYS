/*
 * motor.h - TB6612电机驱动
 * 
 * TB6612工作原理：
 * - AIN1/AIN2电平相同时工作，不同时制动
 * - PWMA高电平时工作
 * - STBY高电平芯片才工作
 * 不需要考虑旋转方向
 */

#ifndef MOTOR_H
#define MOTOR_H

// TB6612引脚定义 - 避免使用GPIO0/2
// AIN1: GPIO10
// AIN2: GPIO11  
// PWMA: GPIO12 (PWM3)
// STBY: GPIO6

#define MOTOR_AIN1_GPIO     WIFI_IOT_GPIO_IDX_10
#define MOTOR_AIN1_IO_NAME  WIFI_IOT_IO_NAME_GPIO_10
#define MOTOR_AIN2_GPIO     WIFI_IOT_GPIO_IDX_11
#define MOTOR_AIN2_IO_NAME  WIFI_IOT_IO_NAME_GPIO_11
#define MOTOR_PWMA_GPIO     WIFI_IOT_GPIO_IDX_12
#define MOTOR_PWMA_IO_NAME  WIFI_IOT_IO_NAME_GPIO_12
#define MOTOR_STBY_GPIO     WIFI_IOT_GPIO_IDX_6
#define MOTOR_STBY_IO_NAME  WIFI_IOT_IO_NAME_GPIO_6

// 电机状态
#define MOTOR_STATE_STOPPED  0
#define MOTOR_STATE_RUNNING  1

// 初始化电机
void Motor_Init(void);

// 启动电机
void Motor_Start(void);

// 停止电机
void Motor_Stop(void);

// 获取电机状态
int Motor_GetState(void);

#endif
