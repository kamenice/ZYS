/*
 * servo.h - 舵机PWM驱动
 * 
 * 舵机需要20ms周期的PWM信号
 * 高电平部分0.5ms~2.5ms控制角度
 * - 0.5ms: 0度
 * - 1.5ms: 90度
 * - 2.5ms: 180度
 */

#ifndef SERVO_H
#define SERVO_H

// 舵机引脚定义 - 使用GPIO5 (PWM2)
#define SERVO_GPIO      WIFI_IOT_GPIO_IDX_5
#define SERVO_IO_NAME   WIFI_IOT_IO_NAME_GPIO_5
#define SERVO_PWM_PORT  WIFI_IOT_PWM_PORT_PWM2

// 舵机角度常量
#define SERVO_ANGLE_0    0
#define SERVO_ANGLE_90   90
#define SERVO_ANGLE_180  180

// 初始化舵机
void Servo_Init(void);

// 设置舵机角度(0-180度)
void Servo_SetAngle(int angle);

// 舵机开启(转到90度)
void Servo_Open(void);

// 舵机关闭(转到0度)
void Servo_Close(void);

// 获取舵机当前角度
int Servo_GetAngle(void);

#endif
