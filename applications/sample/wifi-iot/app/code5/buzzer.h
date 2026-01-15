/*
 * 有源蜂鸣器驱动头文件
 * 低电平触发
 */

#ifndef BUZZER_H
#define BUZZER_H

/**
 * @brief 初始化蜂鸣器
 */
void Buzzer_Init(void);

/**
 * @brief 打开蜂鸣器（低电平触发）
 */
void Buzzer_On(void);

/**
 * @brief 关闭蜂鸣器
 */
void Buzzer_Off(void);

/**
 * @brief 蜂鸣器报警（持续响）
 */
void Buzzer_Alarm(void);

#endif // BUZZER_H
