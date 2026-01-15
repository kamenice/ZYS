/*
 * dht11.h - DHT11温湿度传感器驱动
 */

#ifndef DHT11_H
#define DHT11_H

#include <stdint.h>

// DHT11引脚定义 - 使用GPIO3
#define DHT11_GPIO      WIFI_IOT_GPIO_IDX_3
#define DHT11_IO_NAME   WIFI_IOT_IO_NAME_GPIO_3

// 温度报警阈值（超过30度报警）
#define DHT11_TEMP_ALARM_THRESHOLD  30.0f

// 初始化DHT11
void DHT11_Init(void);

// 读取DHT11数据
// 返回值：0=成功，非0=失败
int DHT11_Read(float *temperature, float *humidity);

// 获取当前温度
float DHT11_GetTemperature(void);

// 获取当前湿度
float DHT11_GetHumidity(void);

// 检测是否超温报警
int DHT11_IsOverTemp(void);

// DHT11主任务入口
void DHT11_MainLoop(void);

#endif
