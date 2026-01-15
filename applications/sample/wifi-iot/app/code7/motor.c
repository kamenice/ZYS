/*
 * motor.c - TB6612电机驱动
 * 
 * TB6612工作原理：
 * - AIN1/AIN2电平相同时工作，不同时制动
 * - PWMA高电平时工作
 * - STBY高电平芯片才工作
 */

#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_pwm.h"
#include "wifiiot_errno.h"

#include "motor.h"

static volatile int g_motor_state = MOTOR_STATE_STOPPED;

void Motor_Init(void)
{
    GpioInit();
    
    // AIN1设置为GPIO输出
    IoSetFunc(MOTOR_AIN1_IO_NAME, WIFI_IOT_IO_FUNC_GPIO_10_GPIO);
    GpioSetDir(MOTOR_AIN1_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(MOTOR_AIN1_GPIO, WIFI_IOT_GPIO_VALUE0);
    
    // AIN2设置为GPIO输出
    IoSetFunc(MOTOR_AIN2_IO_NAME, WIFI_IOT_IO_FUNC_GPIO_11_GPIO);
    GpioSetDir(MOTOR_AIN2_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(MOTOR_AIN2_GPIO, WIFI_IOT_GPIO_VALUE0);
    
    // PWMA设置为PWM3输出
    IoSetFunc(MOTOR_PWMA_IO_NAME, WIFI_IOT_IO_FUNC_GPIO_12_PWM3_OUT);
    PwmInit(WIFI_IOT_PWM_PORT_PWM3);
    
    // STBY设置为GPIO输出
    IoSetFunc(MOTOR_STBY_IO_NAME, WIFI_IOT_IO_FUNC_GPIO_6_GPIO);
    GpioSetDir(MOTOR_STBY_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(MOTOR_STBY_GPIO, WIFI_IOT_GPIO_VALUE0); // 初始待机
    
    g_motor_state = MOTOR_STATE_STOPPED;
    printf("[Motor] Init done\n");
}

void Motor_Start(void)
{
    // STBY高电平使能芯片
    GpioSetOutputVal(MOTOR_STBY_GPIO, WIFI_IOT_GPIO_VALUE1);
    
    // AIN1=AIN2=高电平，电机工作（正转）
    GpioSetOutputVal(MOTOR_AIN1_GPIO, WIFI_IOT_GPIO_VALUE1);
    GpioSetOutputVal(MOTOR_AIN2_GPIO, WIFI_IOT_GPIO_VALUE1);
    
    // PWMA输出PWM信号（50%占空比）
    // 频率分频系数：160MHz / 40000 = 4kHz
    PwmStart(WIFI_IOT_PWM_PORT_PWM3, 20000, 40000);
    
    g_motor_state = MOTOR_STATE_RUNNING;
    printf("[Motor] Started\n");
}

void Motor_Stop(void)
{
    // 停止PWM
    PwmStop(WIFI_IOT_PWM_PORT_PWM3);
    
    // AIN1!=AIN2制动
    GpioSetOutputVal(MOTOR_AIN1_GPIO, WIFI_IOT_GPIO_VALUE1);
    GpioSetOutputVal(MOTOR_AIN2_GPIO, WIFI_IOT_GPIO_VALUE0);
    
    // STBY低电平使芯片待机
    GpioSetOutputVal(MOTOR_STBY_GPIO, WIFI_IOT_GPIO_VALUE0);
    
    g_motor_state = MOTOR_STATE_STOPPED;
    printf("[Motor] Stopped\n");
}

int Motor_GetState(void)
{
    return g_motor_state;
}
