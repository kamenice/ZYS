/*
 * servo.c - 舵机PWM驱动
 * 
 * 舵机需要20ms周期的PWM信号（50Hz）
 * 高电平部分0.5ms~2.5ms控制角度
 * 
 * 使用160MHz时钟：
 * 周期 = 160MHz / freq = 20ms => freq = 8000
 * 但是freq是16位最大65535
 * 
 * 实际计算：
 * PWM周期 = 160MHz / freq_div
 * 20ms = 160,000,000 / freq_div
 * freq_div = 3,200,000 (超出16位范围)
 * 
 * 使用分频方式：先降频再计算
 * 这里使用GPIO模拟PWM方式实现20ms周期
 */

#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_pwm.h"
#include "wifiiot_errno.h"

#include "servo.h"

static volatile int g_servo_angle = 0;
static volatile int g_servo_running = 0;
static osThreadId_t g_servo_thread = NULL;

void Servo_Init(void)
{
    GpioInit();
    IoSetFunc(SERVO_IO_NAME, WIFI_IOT_IO_FUNC_GPIO_5_GPIO);
    GpioSetDir(SERVO_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(SERVO_GPIO, WIFI_IOT_GPIO_VALUE0);
    
    g_servo_angle = 0;
    printf("[Servo] Init done\n");
}

// 生成舵机PWM脉冲（软件模拟）
// angle: 0-180度
// 对应高电平时间：0.5ms-2.5ms
static void Servo_GeneratePulse(int angle)
{
    // 限制角度范围
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;
    
    // 计算高电平时间（微秒）
    // 0度 = 500us, 180度 = 2500us
    int high_time_us = 500 + (angle * 2000 / 180);
    int low_time_us = 20000 - high_time_us; // 20ms周期
    
    // 输出PWM脉冲
    GpioSetOutputVal(SERVO_GPIO, WIFI_IOT_GPIO_VALUE1);
    usleep(high_time_us);
    GpioSetOutputVal(SERVO_GPIO, WIFI_IOT_GPIO_VALUE0);
    usleep(low_time_us);
}

void Servo_SetAngle(int angle)
{
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;
    
    g_servo_angle = angle;
    
    // 生成若干个脉冲使舵机转到指定角度
    for (int i = 0; i < 25; i++) { // 约500ms
        Servo_GeneratePulse(angle);
    }
    
    printf("[Servo] Set angle to %d\n", angle);
}

void Servo_Open(void)
{
    Servo_SetAngle(SERVO_ANGLE_90);
}

void Servo_Close(void)
{
    Servo_SetAngle(SERVO_ANGLE_0);
}

int Servo_GetAngle(void)
{
    return g_servo_angle;
}
