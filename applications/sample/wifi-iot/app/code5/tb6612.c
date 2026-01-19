/*
 * TB6612电机驱动实现
 * 用于控制传送带电机
 * 
 * GPIO引脚配置:
 * - AIN1: GPIO0
 * - AIN2: GPIO1
 * - PWMA: GPIO2 (PWM2)
 * - STBY: GPIO10 (使能引脚)
 */

#include <stdio.h>
#include <unistd.h>
#include "tb6612.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_pwm.h"

// TB6612引脚定义
#define TB6612_AIN1_GPIO  WIFI_IOT_GPIO_IDX_0   // 方向控制引脚1
#define TB6612_AIN2_GPIO  WIFI_IOT_GPIO_IDX_1   // 方向控制引脚2
#define TB6612_PWMA_GPIO  WIFI_IOT_IO_NAME_GPIO_2  // PWM引脚
#define TB6612_STBY_GPIO  WIFI_IOT_GPIO_IDX_10  // 待机引脚

// PWM参数
#define MOTOR_PWM_PORT    WIFI_IOT_PWM_PORT_PWM2
#define MOTOR_PWM_FREQ    40000  // PWM频率分频值
#define MOTOR_PWM_DUTY    20000  // 占空比（50%）

// 电机状态
static int g_motorRunning = 0;

/**
 * @brief 初始化TB6612电机驱动
 */
void TB6612_Init(void)
{
    // 配置AIN1为GPIO输出
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_0, WIFI_IOT_IO_FUNC_GPIO_0_GPIO);
    GpioSetDir(TB6612_AIN1_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(TB6612_AIN1_GPIO, WIFI_IOT_GPIO_VALUE0);
    
    // 配置AIN2为GPIO输出
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_1, WIFI_IOT_IO_FUNC_GPIO_1_GPIO);
    GpioSetDir(TB6612_AIN2_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(TB6612_AIN2_GPIO, WIFI_IOT_GPIO_VALUE0);
    
    // 配置PWMA为PWM输出
    IoSetFunc(TB6612_PWMA_GPIO, WIFI_IOT_IO_FUNC_GPIO_2_PWM2_OUT);
    PwmInit(MOTOR_PWM_PORT);
    
    // 配置STBY为GPIO输出（高电平使能）
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_10, WIFI_IOT_IO_FUNC_GPIO_10_GPIO);
    GpioSetDir(TB6612_STBY_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(TB6612_STBY_GPIO, WIFI_IOT_GPIO_VALUE1);  // 使能电机驱动
    
    g_motorRunning = 0;
    printf("[TB6612] Motor driver initialized\n");
}

/**
 * @brief 启动电机 - 正转
 */
void TB6612_Forward(void)
{
    // 设置方向：AIN1=HIGH, AIN2=LOW
    GpioSetOutputVal(TB6612_AIN1_GPIO, WIFI_IOT_GPIO_VALUE1);
    GpioSetOutputVal(TB6612_AIN2_GPIO, WIFI_IOT_GPIO_VALUE0);
    
    // 启动PWM
    PwmStart(MOTOR_PWM_PORT, MOTOR_PWM_DUTY, MOTOR_PWM_FREQ);
    
    g_motorRunning = 1;
    printf("[TB6612] Motor forward\n");
}

/**
 * @brief 启动电机 - 反转
 */
void TB6612_Backward(void)
{
    // 设置方向：AIN1=LOW, AIN2=HIGH
    GpioSetOutputVal(TB6612_AIN1_GPIO, WIFI_IOT_GPIO_VALUE0);
    GpioSetOutputVal(TB6612_AIN2_GPIO, WIFI_IOT_GPIO_VALUE1);
    
    // 启动PWM
    PwmStart(MOTOR_PWM_PORT, MOTOR_PWM_DUTY, MOTOR_PWM_FREQ);
    
    g_motorRunning = 1;
    printf("[TB6612] Motor backward\n");
}

/**
 * @brief 停止电机
 */
void TB6612_Stop(void)
{
    // 停止PWM
    PwmStop(MOTOR_PWM_PORT);
    
    // 设置方向引脚为低电平
    GpioSetOutputVal(TB6612_AIN1_GPIO, WIFI_IOT_GPIO_VALUE0);
    GpioSetOutputVal(TB6612_AIN2_GPIO, WIFI_IOT_GPIO_VALUE0);
    
    g_motorRunning = 0;
    printf("[TB6612] Motor stopped\n");
}

/**
 * @brief 获取电机运行状态
 * @return 1: 运行中, 0: 停止
 */
int TB6612_IsRunning(void)
{
    return g_motorRunning;
}
