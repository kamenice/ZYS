/*
 * infrared.h - YL-62红外线避障传感器驱动
 * 
 * YL-62当检测到物品时输出低电平
 */

#ifndef INFRARED_H
#define INFRARED_H

// YL-62引脚定义 - 使用GPIO4
#define INFRARED_GPIO     WIFI_IOT_GPIO_IDX_4
#define INFRARED_IO_NAME  WIFI_IOT_IO_NAME_GPIO_4

// 初始化红外传感器
void Infrared_Init(void);

// 检测是否有物品（低电平=有物品）
// 返回值：1=检测到物品，0=未检测到
int Infrared_DetectObject(void);

// 获取最近一次检测结果
int Infrared_GetLastState(void);

// 红外传感器主任务入口
void Infrared_MainLoop(void);

#endif
