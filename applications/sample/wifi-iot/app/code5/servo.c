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
 * Hi3861 PWM时钟为160MHz
 * 20ms周期 = 50Hz
 * 分频值 = 160MHz / 50Hz = 3200000 (超出uint16范围)
 * 使用预分频: 160MHz / 4000 = 40000Hz
 * 周期 = 40000 / 50 = 800 (即freq参数为800)
 * 
 * 高电平时间:
 * - 0.5ms = 2.5% 占空比 = 20
 * - 1.5ms = 7.5% 占空比 = 60
 * - 2.5ms = 12.5% 占空比 = 100
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

// PWM参数 (基于160MHz时钟)
// 160MHz / 40000 = 4000Hz时钟
// 4000Hz / 80 = 50Hz (20ms周期)
#define SERVO_PWM_FREQ   80  // 产生约20ms周期

// 占空比对应的脉宽
// 0度: 0.5ms / 20ms = 2.5% -> 80 * 2.5% = 2
// 90度: 1.5ms / 20ms = 7.5% -> 80 * 7.5% = 6
// 180度: 2.5ms / 20ms = 12.5% -> 80 * 12.5% = 10
#define SERVO_DUTY_0DEG    2   // 0度对应的占空比
#define SERVO_DUTY_90DEG   6   // 90度对应的占空比
#define SERVO_DUTY_180DEG  10  // 180度对应的占空比

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
