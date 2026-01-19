/*
 * dht11.c - DHT11温湿度传感器驱动
 * 
 * DHT11是单总线数字温湿度传感器
 * 数据格式：8bit湿度整数 + 8bit湿度小数 + 8bit温度整数 + 8bit温度小数 + 8bit校验和
 */

#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_errno.h"

#include "dht11.h"

#define DHT11_TASK_STACK_SIZE 2048

static volatile float g_temperature = 0.0f;
static volatile float g_humidity = 0.0f;
static volatile int g_over_temp = 0;

void DHT11_Init(void)
{
    GpioInit();
    IoSetFunc(DHT11_IO_NAME, WIFI_IOT_IO_FUNC_GPIO_3_GPIO);
    GpioSetDir(DHT11_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(DHT11_GPIO, WIFI_IOT_GPIO_VALUE1);
}

static void DHT11_SetOutput(void)
{
    GpioSetDir(DHT11_GPIO, WIFI_IOT_GPIO_DIR_OUT);
}

static void DHT11_SetInput(void)
{
    GpioSetDir(DHT11_GPIO, WIFI_IOT_GPIO_DIR_IN);
    IoSetPull(DHT11_IO_NAME, WIFI_IOT_IO_PULL_UP);
}

static WifiIotGpioValue DHT11_ReadPin(void)
{
    WifiIotGpioValue val;
    GpioGetInputVal(DHT11_GPIO, &val);
    return val;
}

static void DHT11_WritePin(WifiIotGpioValue val)
{
    GpioSetOutputVal(DHT11_GPIO, val);
}

// 读取一个字节
static uint8_t DHT11_ReadByte(void)
{
    uint8_t byte = 0;
    int i;
    
    for (i = 0; i < 8; i++) {
        // 等待低电平结束（数据位开始）
        int timeout = 100;
        while (DHT11_ReadPin() == WIFI_IOT_GPIO_VALUE0 && timeout > 0) {
            usleep(1);
            timeout--;
        }
        
        // 延时30us后判断高电平持续时间
        usleep(30);
        
        byte <<= 1;
        if (DHT11_ReadPin() == WIFI_IOT_GPIO_VALUE1) {
            byte |= 0x01; // 高电平持续时间长表示数据1
        }
        
        // 等待高电平结束
        timeout = 100;
        while (DHT11_ReadPin() == WIFI_IOT_GPIO_VALUE1 && timeout > 0) {
            usleep(1);
            timeout--;
        }
    }
    
    return byte;
}

int DHT11_Read(float *temperature, float *humidity)
{
    uint8_t data[5] = {0};
    int timeout;
    
    // 主机拉低至少18ms
    DHT11_SetOutput();
    DHT11_WritePin(WIFI_IOT_GPIO_VALUE0);
    usleep(20000); // 20ms
    
    // 主机拉高20-40us
    DHT11_WritePin(WIFI_IOT_GPIO_VALUE1);
    usleep(30);
    
    // 切换为输入模式
    DHT11_SetInput();
    
    // 等待DHT11响应（拉低80us）
    timeout = 100;
    while (DHT11_ReadPin() == WIFI_IOT_GPIO_VALUE1 && timeout > 0) {
        usleep(1);
        timeout--;
    }
    if (timeout <= 0) return -1;
    
    // 等待响应低电平结束
    timeout = 100;
    while (DHT11_ReadPin() == WIFI_IOT_GPIO_VALUE0 && timeout > 0) {
        usleep(1);
        timeout--;
    }
    if (timeout <= 0) return -2;
    
    // 等待响应高电平结束
    timeout = 100;
    while (DHT11_ReadPin() == WIFI_IOT_GPIO_VALUE1 && timeout > 0) {
        usleep(1);
        timeout--;
    }
    if (timeout <= 0) return -3;
    
    // 读取5字节数据
    for (int i = 0; i < 5; i++) {
        data[i] = DHT11_ReadByte();
    }
    
    // 恢复为输出高电平
    DHT11_SetOutput();
    DHT11_WritePin(WIFI_IOT_GPIO_VALUE1);
    
    // 校验和检查
    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if (checksum != data[4]) {
        return -4;
    }
    
    // 解析数据
    *humidity = (float)data[0] + (float)data[1] * 0.1f;
    *temperature = (float)data[2] + (float)data[3] * 0.1f;
    
    return 0;
}

float DHT11_GetTemperature(void)
{
    return g_temperature;
}

float DHT11_GetHumidity(void)
{
    return g_humidity;
}

int DHT11_IsOverTemp(void)
{
    return g_over_temp;
}

static void DHT11_Task(void *arg)
{
    (void)arg;
    float temp, humi;
    int ret;
    
    DHT11_Init();
    usleep(1000000); // DHT11上电后等待1秒稳定
    
    while (1) {
        ret = DHT11_Read(&temp, &humi);
        if (ret == 0) {
            g_temperature = temp;
            g_humidity = humi;
            
            // 检测超温
            if (temp > DHT11_TEMP_ALARM_THRESHOLD) {
                g_over_temp = 1;
            } else {
                g_over_temp = 0;
            }
            
            printf("[DHT11] temp:%.1f humi:%.1f overtemp:%d\n", temp, humi, g_over_temp);
        } else {
            printf("[DHT11] Read failed: %d\n", ret);
        }
        
        sleep(2); // DHT11采样间隔至少1秒
    }
}

void DHT11_MainLoop(void)
{
    osThreadAttr_t attr;
    attr.name = "DHT11_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = DHT11_TASK_STACK_SIZE;
    attr.priority = osPriorityNormal;
    
    if (osThreadNew((osThreadFunc_t)DHT11_Task, NULL, &attr) == NULL) {
        printf("[DHT11] Failed to create DHT11_Task!\n");
    }
}
