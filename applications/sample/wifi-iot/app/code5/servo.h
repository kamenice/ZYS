/*
 * 舵机驱动头文件
 * PWM控制：20ms周期，高电平0.5-2.5ms
 */

#ifndef SERVO_H
#define SERVO_H

/**
 * @brief 初始化舵机
 */
void Servo_Init(void);

/**
 * @brief 设置舵机角度
 * @param angle 角度值 (0-180度)
 */
void Servo_SetAngle(int angle);

/**
 * @brief 舵机转到初始位置 (0度)
 */
void Servo_Reset(void);

/**
 * @brief 舵机转到90度位置
 */
void Servo_Middle(void);

/**
 * @brief 舵机转到180度位置
 */
void Servo_Full(void);

/**
 * @brief 停止舵机PWM输出
 */
void Servo_Stop(void);

#endif // SERVO_H
