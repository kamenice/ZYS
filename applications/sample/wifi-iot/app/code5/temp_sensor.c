/*
 * 温度传感器实现
 * 使用AHT20温湿度传感器读取温度
 * 
 * I2C配置:
 * - SDA: GPIO13
 * - SCL: GPIO14
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "temp_sensor.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_i2c.h"
#include "wifiiot_errno.h"

// AHT20 I2C配置
#define AHT20_I2C_IDX           WIFI_IOT_I2C_IDX_0
#define AHT20_BAUDRATE          (400 * 1000)  // 400kHz

// AHT20地址和命令
#define AHT20_DEVICE_ADDR       0x38
#define AHT20_READ_ADDR         ((0x38 << 1) | 0x1)
#define AHT20_WRITE_ADDR        ((0x38 << 1) | 0x0)

#define AHT20_CMD_CALIBRATION   0xBE
#define AHT20_CMD_CALIBRATION_ARG0 0x08
#define AHT20_CMD_CALIBRATION_ARG1 0x00

#define AHT20_CMD_TRIGGER       0xAC
#define AHT20_CMD_TRIGGER_ARG0  0x33
#define AHT20_CMD_TRIGGER_ARG1  0x00

#define AHT20_CMD_STATUS        0x71
#define AHT20_CMD_RESET         0xBA

// 时间定义
#define AHT20_STARTUP_TIME      (20 * 1000)
#define AHT20_CALIBRATION_TIME  (40 * 1000)
#define AHT20_MEASURE_TIME      (75 * 1000)

#define AHT20_STATUS_BUSY_SHIFT 7
#define AHT20_STATUS_BUSY_MASK  (0x1 << AHT20_STATUS_BUSY_SHIFT)
#define AHT20_STATUS_BUSY(status) ((status & AHT20_STATUS_BUSY_MASK) >> AHT20_STATUS_BUSY_SHIFT)

#define AHT20_STATUS_CALI_SHIFT 3
#define AHT20_STATUS_CALI_MASK  (0x1 << AHT20_STATUS_CALI_SHIFT)
#define AHT20_STATUS_CALI(status) ((status & AHT20_STATUS_CALI_MASK) >> AHT20_STATUS_CALI_SHIFT)

#define AHT20_STATUS_RESPONSE_MAX 6
#define AHT20_RESLUTION         (1 << 20)  // 2^20
#define AHT20_MAX_RETRY         10

// 当前温湿度值
static float g_currentTemperature = 0.0f;
static float g_currentHumidity = 0.0f;

static uint32_t AHT20_Read(uint8_t *buffer, uint32_t buffLen)
{
    WifiIotI2cData data = {0};
    data.receiveBuf = buffer;
    data.receiveLen = buffLen;
    uint32_t retval = I2cRead(AHT20_I2C_IDX, AHT20_READ_ADDR, &data);
    if (retval != WIFI_IOT_SUCCESS) {
        printf("[TempSensor] I2cRead failed: %0X\n", retval);
        return retval;
    }
    return WIFI_IOT_SUCCESS;
}

static uint32_t AHT20_Write(uint8_t *buffer, uint32_t buffLen)
{
    WifiIotI2cData data = {0};
    data.sendBuf = buffer;
    data.sendLen = buffLen;
    uint32_t retval = I2cWrite(AHT20_I2C_IDX, AHT20_WRITE_ADDR, &data);
    if (retval != WIFI_IOT_SUCCESS) {
        printf("[TempSensor] I2cWrite failed: %0X\n", retval);
        return retval;
    }
    return WIFI_IOT_SUCCESS;
}

static uint32_t AHT20_StatusCommand(void)
{
    uint8_t statusCmd[] = {AHT20_CMD_STATUS};
    return AHT20_Write(statusCmd, sizeof(statusCmd));
}

static uint32_t AHT20_ResetCommand(void)
{
    uint8_t resetCmd[] = {AHT20_CMD_RESET};
    return AHT20_Write(resetCmd, sizeof(resetCmd));
}

static uint32_t AHT20_CalibrateCommand(void)
{
    uint8_t clibrateCmd[] = {AHT20_CMD_CALIBRATION, AHT20_CMD_CALIBRATION_ARG0, AHT20_CMD_CALIBRATION_ARG1};
    return AHT20_Write(clibrateCmd, sizeof(clibrateCmd));
}

static uint32_t AHT20_Calibrate(void)
{
    uint32_t retval = 0;
    uint8_t buffer[AHT20_STATUS_RESPONSE_MAX] = {0};

    retval = AHT20_StatusCommand();
    if (retval != WIFI_IOT_SUCCESS) {
        return retval;
    }

    retval = AHT20_Read(buffer, sizeof(buffer));
    if (retval != WIFI_IOT_SUCCESS) {
        return retval;
    }

    if (AHT20_STATUS_BUSY(buffer[0]) || !AHT20_STATUS_CALI(buffer[0])) {
        retval = AHT20_ResetCommand();
        if (retval != WIFI_IOT_SUCCESS) {
            return retval;
        }
        usleep(AHT20_STARTUP_TIME);
        retval = AHT20_CalibrateCommand();
        usleep(AHT20_CALIBRATION_TIME);
        return retval;
    }

    return WIFI_IOT_SUCCESS;
}

static uint32_t AHT20_StartMeasure(void)
{
    uint8_t triggerCmd[] = {AHT20_CMD_TRIGGER, AHT20_CMD_TRIGGER_ARG0, AHT20_CMD_TRIGGER_ARG1};
    return AHT20_Write(triggerCmd, sizeof(triggerCmd));
}

static uint32_t AHT20_GetMeasureResult(float *temp, float *humi)
{
    uint32_t retval = 0;
    uint32_t i = 0;
    
    if (temp == NULL || humi == NULL) {
        return WIFI_IOT_FAILURE;
    }

    uint8_t buffer[AHT20_STATUS_RESPONSE_MAX] = {0};
    retval = AHT20_Read(buffer, sizeof(buffer));
    if (retval != WIFI_IOT_SUCCESS) {
        return retval;
    }

    for (i = 0; AHT20_STATUS_BUSY(buffer[0]) && i < AHT20_MAX_RETRY; i++) {
        usleep(AHT20_MEASURE_TIME);
        retval = AHT20_Read(buffer, sizeof(buffer));
        if (retval != WIFI_IOT_SUCCESS) {
            return retval;
        }
    }
    if (i >= AHT20_MAX_RETRY) {
        printf("[TempSensor] AHT20 device always busy\n");
        return WIFI_IOT_FAILURE;
    }

    uint32_t humiRaw = buffer[1];
    humiRaw = (humiRaw << 8) | buffer[2];
    humiRaw = (humiRaw << 4) | ((buffer[3] & 0xF0) >> 4);
    *humi = humiRaw / (float)AHT20_RESLUTION * 100;

    uint32_t tempRaw = buffer[3] & 0x0F;
    tempRaw = (tempRaw << 8) | buffer[4];
    tempRaw = (tempRaw << 8) | buffer[5];
    *temp = tempRaw / (float)AHT20_RESLUTION * 200 - 50;

    return WIFI_IOT_SUCCESS;
}

/**
 * @brief 初始化温度传感器
 */
void TempSensor_Init(void)
{
    // 配置I2C引脚
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_13, WIFI_IOT_IO_FUNC_GPIO_13_I2C0_SDA);
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_14, WIFI_IOT_IO_FUNC_GPIO_14_I2C0_SCL);
    
    I2cInit(AHT20_I2C_IDX, AHT20_BAUDRATE);
    
    // 校准传感器
    while (WIFI_IOT_SUCCESS != AHT20_Calibrate()) {
        printf("[TempSensor] AHT20 calibration failed, retrying...\n");
        usleep(1000);
    }
    
    printf("[TempSensor] Initialized\n");
}

/**
 * @brief 读取温度值
 * @return 当前温度（摄氏度）
 */
float TempSensor_ReadTemperature(void)
{
    uint32_t retval = AHT20_StartMeasure();
    if (retval == WIFI_IOT_SUCCESS) {
        usleep(AHT20_MEASURE_TIME);
        retval = AHT20_GetMeasureResult(&g_currentTemperature, &g_currentHumidity);
    }
    return g_currentTemperature;
}

/**
 * @brief 读取湿度值
 * @return 当前湿度（%）
 */
float TempSensor_ReadHumidity(void)
{
    return g_currentHumidity;
}

/**
 * @brief 检查温度是否超过阈值
 * @param threshold 温度阈值
 * @return 1: 超过阈值, 0: 未超过
 */
int TempSensor_IsOverThreshold(float threshold)
{
    float temp = TempSensor_ReadTemperature();
    return (temp > threshold) ? 1 : 0;
}
