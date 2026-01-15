/*
 * TB6612电机驱动头文件
 * 用于控制传送带电机
 */

#ifndef TB6612_H
#define TB6612_H

/**
 * @brief 初始化TB6612电机驱动
 */
void TB6612_Init(void);

/**
 * @brief 启动电机 - 正转
 */
void TB6612_Forward(void);

/**
 * @brief 启动电机 - 反转
 */
void TB6612_Backward(void);

/**
 * @brief 停止电机
 */
void TB6612_Stop(void);

/**
 * @brief 获取电机运行状态
 * @return 1: 运行中, 0: 停止
 */
int TB6612_IsRunning(void);

#endif // TB6612_H
