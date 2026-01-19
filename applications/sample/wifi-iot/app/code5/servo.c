/*
 * 舵机驱动实现
 * PWM控制：20ms周期（50Hz），高电平0.5-2.5ms
 * 
 * PWM配置:
 * - 舵机: GPIO6 (PWM3)
 * 
 * 舵机角度与脉宽对应关系:
 * - 0度:   0.5ms 高电平
 * - 90度:  1.5ms 高电平
 * - 180度: 2.5ms 高电平
 * 
 * Hi3861 PWM说明:
 * PwmStart(port, duty, freq) 参数说明:
 * - freq: 频率分频值，PWM周期 = freq个时钟周期
 * - duty: 占空比，高电平时间 = duty个时钟周期
 * 
 * 使用较大的freq值来实现20ms周期:
 * freq = 60000 时，周期约为20ms（基于160MHz/4000预分频后的40kHz）
 * 
 * 占空比计算 (基于20ms周期):
 * - 0.5ms 高电平: duty = 60000 * 0.5 / 20 = 1500
 * - 1.5ms 高电平: duty = 60000 * 1.5 / 20 = 4500
 * - 2.5ms 高电平: duty = 60000 * 2.5 / 20 = 7500
 */

#include <stdio.h>
#include <unistd.h>
#include "servo.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_pwm.h"

// 舵机PWM引脚定义
#define SERVO_PWM_PORT   WIFI_IOT_PWM_PORT_PWM3
#define SERVO_GPIO       WIFI_IOT_IO_NAME_GPIO_6

// PWM参数 - 用于产生约20ms周期的PWM信号
#define SERVO_PWM_FREQ   60000  // 频率分频值，产生约20ms周期

// 占空比对应的脉宽 (基于60000的周期)
// 0度: 0.5ms / 20ms * 60000 = 1500
// 90度: 1.5ms / 20ms * 60000 = 4500
// 180度: 2.5ms / 20ms * 60000 = 7500
#define SERVO_DUTY_0DEG    1500   // 0度对应的占空比 (0.5ms)
#define SERVO_DUTY_90DEG   4500   // 90度对应的占空比 (1.5ms)
#define SERVO_DUTY_180DEG  7500   // 180度对应的占空比 (2.5ms)

/**
 * @brief 初始化舵机
 */
void Servo_Init(void)
{
    // 配置舵机引脚为PWM输出
    IoSetFunc(SERVO_GPIO, WIFI_IOT_IO_FUNC_GPIO_6_PWM3_OUT);
    PwmInit(SERVO_PWM_PORT);
    
    printf("[Servo] Initialized (PWM3 on GPIO6)\n");
}

/**
 * @brief 设置舵机角度
 * @param angle 角度值 (0-180度)
 */
void Servo_SetAngle(int angle)
{
    // 限制角度范围
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;
    
    // 计算占空比
    // 从0度(2)到180度(10)线性映射
    uint16_t duty = SERVO_DUTY_0DEG + (SERVO_DUTY_180DEG - SERVO_DUTY_0DEG) * angle / 180;
    
    // 启动PWM
    PwmStart(SERVO_PWM_PORT, duty, SERVO_PWM_FREQ);
    
    printf("[Servo] Set angle to %d degrees (duty=%d)\n", angle, duty);
    
    // 等待舵机转到位置
    usleep(500000);  // 500ms
}

/**
 * @brief 舵机转到初始位置 (0度)
 */
void Servo_Reset(void)
{
    Servo_SetAngle(0);
    printf("[Servo] Reset to 0 degrees\n");
}

/**
 * @brief 舵机转到90度位置
 */
void Servo_Middle(void)
{
    Servo_SetAngle(90);
    printf("[Servo] Set to 90 degrees\n");
}

/**
 * @brief 舵机转到180度位置
 */
void Servo_Full(void)
{
    Servo_SetAngle(180);
    printf("[Servo] Set to 180 degrees\n");
}

/**
 * @brief 停止舵机PWM输出
 */
void Servo_Stop(void)
{
    PwmStop(SERVO_PWM_PORT);
    printf("[Servo] PWM stopped\n");
}
