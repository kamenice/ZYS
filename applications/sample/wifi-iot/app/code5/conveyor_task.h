/*
 * 传送带任务头文件
 * 定义各个子任务
 */

#ifndef CONVEYOR_TASK_H
#define CONVEYOR_TASK_H

/**
 * @brief 压力传感器任务 - 检测物品并控制电机
 */
void PressureSensor_Task(void);

/**
 * @brief 温度监测任务 - 监测温度并触发蜂鸣器
 */
void TemperatureMonitor_Task(void);

/**
 * @brief 红外传感器任务 - 检测物品并控制舵机
 */
void InfraredSensor_Task(void);

/**
 * @brief OLED显示任务 - 更新显示屏状态
 */
void OledDisplay_Task(void);

/**
 * @brief MQTT发布任务 - 上传数据到手机APP
 */
void MqttPublish_Task(void);

#endif // CONVEYOR_TASK_H
