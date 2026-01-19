/*
 * buzzer.c - 有源蜂鸣器驱动（低电平触发）
 * 
 * 有源蜂鸣器内部有振荡电路，只需要给一个低电平就能发声
 */

#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_errno.h"

#include "buzzer.h"

static volatile int g_buzzer_on = 0;

void Buzzer_Init(void)
{
    GpioInit();
    IoSetFunc(BUZZER_IO_NAME, WIFI_IOT_IO_FUNC_GPIO_9_GPIO);
    GpioSetDir(BUZZER_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    // 初始高电平（关闭蜂鸣器）
    GpioSetOutputVal(BUZZER_GPIO, WIFI_IOT_GPIO_VALUE1);
    g_buzzer_on = 0;
    printf("[Buzzer] Init done\n");
}

void Buzzer_On(void)
{
    // 低电平触发
    GpioSetOutputVal(BUZZER_GPIO, WIFI_IOT_GPIO_VALUE0);
    g_buzzer_on = 1;
}

void Buzzer_Off(void)
{
    // 高电平关闭
    GpioSetOutputVal(BUZZER_GPIO, WIFI_IOT_GPIO_VALUE1);
    g_buzzer_on = 0;
}

void Buzzer_Beep(int duration_ms)
{
    Buzzer_On();
    usleep(duration_ms * 1000);
    Buzzer_Off();
}

int Buzzer_IsOn(void)
{
    return g_buzzer_on;
}
