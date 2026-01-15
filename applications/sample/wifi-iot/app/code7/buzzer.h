/*
 * buzzer.h - 有源蜂鸣器驱动（低电平触发）
 */

#ifndef BUZZER_H
#define BUZZER_H

// 蜂鸣器引脚定义 - 使用GPIO9 (PWM0也可以)
#define BUZZER_GPIO     WIFI_IOT_GPIO_IDX_9
#define BUZZER_IO_NAME  WIFI_IOT_IO_NAME_GPIO_9

// 初始化蜂鸣器
void Buzzer_Init(void);

// 开启蜂鸣器（低电平触发）
void Buzzer_On(void);

// 关闭蜂鸣器（高电平关闭）
void Buzzer_Off(void);

// 蜂鸣器响一段时间（毫秒）
void Buzzer_Beep(int duration_ms);

// 获取蜂鸣器状态
int Buzzer_IsOn(void);

#endif
