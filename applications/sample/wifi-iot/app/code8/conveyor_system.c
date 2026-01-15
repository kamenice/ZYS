/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_pwm.h"
#include "wifiiot_errno.h"
#include "conveyor_system.h"

/*
 * GPIO Pin Assignment (avoid GPIO0/2 - system debugging):
 * 
 * HX711 Pressure Sensor:
 *   - DT (Data): GPIO11
 *   - SCK (Clock): GPIO12
 * 
 * DHT11 Temperature Sensor:
 *   - DATA: GPIO5
 * 
 * TB6612 Motor Driver:
 *   - AIN1: GPIO3
 *   - AIN2: GPIO4
 *   - PWMA: GPIO6 (PWM3)
 *   - STBY: GPIO7
 * 
 * Active Buzzer (low-level trigger):
 *   - CTRL: GPIO8
 * 
 * YL-62 Infrared Sensor:
 *   - OUT: GPIO1
 * 
 * Servo Motor (PWM control):
 *   - PWM: GPIO9 (PWM0)
 * 
 * OLED Display (I2C):
 *   - SDA: GPIO13
 *   - SCL: GPIO14
 */

#define HX711_DT_PIN    WIFI_IOT_GPIO_IDX_11
#define HX711_SCK_PIN   WIFI_IOT_GPIO_IDX_12
#define DHT11_PIN       WIFI_IOT_GPIO_IDX_5
#define MOTOR_AIN1_PIN  WIFI_IOT_GPIO_IDX_3
#define MOTOR_AIN2_PIN  WIFI_IOT_GPIO_IDX_4
#define MOTOR_PWMA_PIN  WIFI_IOT_GPIO_IDX_6
#define MOTOR_STBY_PIN  WIFI_IOT_GPIO_IDX_7
#define BUZZER_PIN      WIFI_IOT_GPIO_IDX_8
#define IR_SENSOR_PIN   WIFI_IOT_GPIO_IDX_1
#define SERVO_PIN       WIFI_IOT_GPIO_IDX_9

#define TEMP_THRESHOLD  30.0f  /* Temperature threshold for alarm */

#define TASK_STACK_SIZE 2048

/* Global system state */
static ConveyorState g_state = {0};
static osMutexId_t g_stateMutex = NULL;

ConveyorState *GetConveyorState(void)
{
    return &g_state;
}

/* HX711 read function - returns 1 if item detected */
static int HX711_ReadItemDetected(void)
{
    unsigned long value = 0;
    unsigned char i;
    WifiIotGpioValue input;

    /* Set SCK low (idle state) */
    GpioSetOutputVal(HX711_SCK_PIN, WIFI_IOT_GPIO_VALUE0);
    usleep(2);

    /* Wait for DT to go low (conversion ready) */
    int timeout = 1000;
    do {
        GpioGetInputVal(HX711_DT_PIN, &input);
        if (--timeout <= 0) {
            return 0;  /* Timeout - no sensor or error */
        }
        usleep(10);
    } while (input == WIFI_IOT_GPIO_VALUE1);

    /* Read 24-bit data */
    for (i = 0; i < 24; i++) {
        GpioSetOutputVal(HX711_SCK_PIN, WIFI_IOT_GPIO_VALUE1);
        usleep(2);
        value = value << 1;
        GpioSetOutputVal(HX711_SCK_PIN, WIFI_IOT_GPIO_VALUE0);
        usleep(2);
        GpioGetInputVal(HX711_DT_PIN, &input);
        if (input == WIFI_IOT_GPIO_VALUE1) {
            value++;
        }
    }

    /* 25th pulse for gain setting (A channel, gain 128) */
    GpioSetOutputVal(HX711_SCK_PIN, WIFI_IOT_GPIO_VALUE1);
    usleep(2);
    value = value ^ 0x800000;
    GpioSetOutputVal(HX711_SCK_PIN, WIFI_IOT_GPIO_VALUE0);
    usleep(2);

    /* Simple threshold detection - if value exceeds baseline, item detected */
    /* Using a simple threshold approach since we just need presence detection */
    static unsigned long baseline = 0;
    static int calibrated = 0;
    
    if (!calibrated) {
        baseline = value;
        calibrated = 1;
        return 0;
    }

    /* If difference from baseline is significant, item is detected */
    long diff = (long)value - (long)baseline;
    if (diff < 0) diff = -diff;
    
    return (diff > 10000) ? 1 : 0;
}

/* DHT11 read function */
static int DHT11_ReadTemperature(float *temp)
{
    uint8_t data[5] = {0};
    uint8_t i, j;
    WifiIotGpioValue level;
    int timeout;

    /* Start signal: pull low for 18ms, then high for 20-40us */
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_5, WIFI_IOT_IO_FUNC_GPIO_5_GPIO);
    GpioSetDir(DHT11_PIN, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(DHT11_PIN, WIFI_IOT_GPIO_VALUE0);
    usleep(20000);  /* 20ms low */
    GpioSetOutputVal(DHT11_PIN, WIFI_IOT_GPIO_VALUE1);
    usleep(30);     /* 30us high */

    /* Switch to input mode and wait for DHT11 response */
    GpioSetDir(DHT11_PIN, WIFI_IOT_GPIO_DIR_IN);

    /* Wait for DHT11 to pull low (response signal) */
    timeout = 100;
    while (timeout-- > 0) {
        GpioGetInputVal(DHT11_PIN, &level);
        if (level == WIFI_IOT_GPIO_VALUE0) break;
        usleep(1);
    }
    if (timeout <= 0) return -1;

    /* Wait for DHT11 to pull high */
    timeout = 100;
    while (timeout-- > 0) {
        GpioGetInputVal(DHT11_PIN, &level);
        if (level == WIFI_IOT_GPIO_VALUE1) break;
        usleep(1);
    }
    if (timeout <= 0) return -1;

    /* Wait for DHT11 to pull low again (start data transmission) */
    timeout = 100;
    while (timeout-- > 0) {
        GpioGetInputVal(DHT11_PIN, &level);
        if (level == WIFI_IOT_GPIO_VALUE0) break;
        usleep(1);
    }
    if (timeout <= 0) return -1;

    /* Read 40 bits of data (5 bytes) */
    for (i = 0; i < 5; i++) {
        for (j = 0; j < 8; j++) {
            /* Wait for high level */
            timeout = 100;
            while (timeout-- > 0) {
                GpioGetInputVal(DHT11_PIN, &level);
                if (level == WIFI_IOT_GPIO_VALUE1) break;
                usleep(1);
            }
            if (timeout <= 0) return -1;

            /* Measure high level duration */
            usleep(30);  /* Wait 30us */
            GpioGetInputVal(DHT11_PIN, &level);
            
            data[i] <<= 1;
            if (level == WIFI_IOT_GPIO_VALUE1) {
                data[i] |= 1;
                /* Wait for low level if high */
                timeout = 100;
                while (timeout-- > 0) {
                    GpioGetInputVal(DHT11_PIN, &level);
                    if (level == WIFI_IOT_GPIO_VALUE0) break;
                    usleep(1);
                }
            }
        }
    }

    /* Checksum verification */
    if (data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
        return -1;
    }

    /* data[2] is integer part of temperature, data[3] is decimal part */
    *temp = (float)data[2] + (float)data[3] * 0.1f;
    return 0;
}

/* Motor control functions (TB6612) */
static void Motor_Init(void)
{
    /* Initialize GPIO for motor control */
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_3, WIFI_IOT_IO_FUNC_GPIO_3_GPIO);
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_4, WIFI_IOT_IO_FUNC_GPIO_4_GPIO);
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_6, WIFI_IOT_IO_FUNC_GPIO_6_PWM3_OUT);
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_7, WIFI_IOT_IO_FUNC_GPIO_7_GPIO);

    GpioSetDir(MOTOR_AIN1_PIN, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetDir(MOTOR_AIN2_PIN, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetDir(MOTOR_STBY_PIN, WIFI_IOT_GPIO_DIR_OUT);

    /* Initialize PWM for motor speed control */
    PwmInit(WIFI_IOT_PWM_PORT_PWM3);

    /* Enable TB6612 (STBY high) */
    GpioSetOutputVal(MOTOR_STBY_PIN, WIFI_IOT_GPIO_VALUE1);
}

static void Motor_Start(void)
{
    /* AIN1 and AIN2 same level = forward rotation */
    GpioSetOutputVal(MOTOR_AIN1_PIN, WIFI_IOT_GPIO_VALUE1);
    GpioSetOutputVal(MOTOR_AIN2_PIN, WIFI_IOT_GPIO_VALUE1);
    /* PWMA high - start motor (duty cycle ~80%) */
    PwmStart(WIFI_IOT_PWM_PORT_PWM3, 40000, 50000);
    g_state.motor_running = 1;
}

static void Motor_Stop(void)
{
    /* Stop PWM */
    PwmStop(WIFI_IOT_PWM_PORT_PWM3);
    /* AIN1 and AIN2 different level = brake */
    GpioSetOutputVal(MOTOR_AIN1_PIN, WIFI_IOT_GPIO_VALUE1);
    GpioSetOutputVal(MOTOR_AIN2_PIN, WIFI_IOT_GPIO_VALUE0);
    g_state.motor_running = 0;
}

/* Buzzer control (active buzzer, low-level trigger) */
static void Buzzer_Init(void)
{
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_8, WIFI_IOT_IO_FUNC_GPIO_8_GPIO);
    GpioSetDir(BUZZER_PIN, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(BUZZER_PIN, WIFI_IOT_GPIO_VALUE1);  /* Off (high) */
}

static void Buzzer_On(void)
{
    GpioSetOutputVal(BUZZER_PIN, WIFI_IOT_GPIO_VALUE0);  /* Low-level trigger */
}

static void Buzzer_Off(void)
{
    GpioSetOutputVal(BUZZER_PIN, WIFI_IOT_GPIO_VALUE1);  /* High = off */
}

/* Servo control (20ms period, 0.5~2.5ms high pulse) */
static void Servo_Init(void)
{
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_IO_FUNC_GPIO_9_PWM0_OUT);
    PwmInit(WIFI_IOT_PWM_PORT_PWM0);
}

/* Set servo angle (0-180 degrees) */
static void Servo_SetAngle(int angle)
{
    /*
     * 20ms period with 160MHz clock:
     * freq = 160MHz / 20ms = 8000
     * 0.5ms duty = 0.5/20 * 8000 = 200 (0 degrees)
     * 2.5ms duty = 2.5/20 * 8000 = 1000 (180 degrees)
     * 
     * Using 40MHz external crystal for better timing:
     * freq = 40MHz / 20ms = 2000
     * 0.5ms = 50, 2.5ms = 250
     */
    uint16_t freq = 8000;  /* 20ms period */
    uint16_t duty_min = 200;   /* 0.5ms = 0 degrees */
    uint16_t duty_max = 1000;  /* 2.5ms = 180 degrees */
    
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;
    
    uint16_t duty = duty_min + (duty_max - duty_min) * angle / 180;
    PwmStart(WIFI_IOT_PWM_PORT_PWM0, duty, freq);
}

static void Servo_Stop(void)
{
    PwmStop(WIFI_IOT_PWM_PORT_PWM0);
}

/* Infrared sensor (YL-62) */
static void IR_Sensor_Init(void)
{
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_1, WIFI_IOT_IO_FUNC_GPIO_1_GPIO);
    GpioSetDir(IR_SENSOR_PIN, WIFI_IOT_GPIO_DIR_IN);
}

static int IR_Sensor_ItemDetected(void)
{
    WifiIotGpioValue level;
    GpioGetInputVal(IR_SENSOR_PIN, &level);
    /* YL-62 outputs low when detecting items */
    return (level == WIFI_IOT_GPIO_VALUE0) ? 1 : 0;
}

/* System initialization */
void ConveyorSystemInit(void)
{
    /* Create state mutex */
    osMutexAttr_t mutexAttr = {0};
    g_stateMutex = osMutexNew(&mutexAttr);

    /* Initialize GPIO */
    GpioInit();

    /* Initialize HX711 pins */
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_IO_FUNC_GPIO_11_GPIO);
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_12, WIFI_IOT_IO_FUNC_GPIO_12_GPIO);
    GpioSetDir(HX711_DT_PIN, WIFI_IOT_GPIO_DIR_IN);
    GpioSetDir(HX711_SCK_PIN, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(HX711_SCK_PIN, WIFI_IOT_GPIO_VALUE0);

    /* Initialize DHT11 */
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_5, WIFI_IOT_IO_FUNC_GPIO_5_GPIO);

    /* Initialize motor */
    Motor_Init();

    /* Initialize buzzer */
    Buzzer_Init();

    /* Initialize IR sensor */
    IR_Sensor_Init();

    /* Initialize servo */
    Servo_Init();

    printf("[ConveyorSystem] Initialized\n");
}

/* Sensor monitoring task */
static void SensorTask(void *arg)
{
    (void)arg;
    float temperature = 0.0f;

    while (1) {
        /* Read HX711 pressure sensor */
        g_state.item_detected = HX711_ReadItemDetected();

        /* Read DHT11 temperature sensor */
        if (DHT11_ReadTemperature(&temperature) == 0) {
            g_state.temperature = temperature;
        }

        /* Check overheat condition */
        if (g_state.temperature > TEMP_THRESHOLD) {
            g_state.overheat_alarm = 1;
            Buzzer_On();
        } else {
            g_state.overheat_alarm = 0;
            Buzzer_Off();
        }

        printf("[Sensor] Item: %d, Temp: %.1fC, Alarm: %d\n",
               g_state.item_detected, g_state.temperature, g_state.overheat_alarm);

        sleep(1);
    }
}

/* Motor control task */
static void MotorTask(void *arg)
{
    (void)arg;

    while (1) {
        if (g_state.overheat_alarm) {
            /* Stop motor on overheat */
            Motor_Stop();
        } else if (g_state.item_detected) {
            /* Start motor when item detected */
            Motor_Start();
        } else {
            /* Stop motor when no item */
            Motor_Stop();
        }

        printf("[Motor] Status: %s\n", g_state.motor_running ? "RUNNING" : "STOPPED");
        usleep(500000);  /* 500ms */
    }
}

/* Servo control task */
static void ServoTask(void *arg)
{
    (void)arg;
    int ir_detected = 0;

    while (1) {
        ir_detected = IR_Sensor_ItemDetected();

        if (ir_detected) {
            g_state.servo_activated = 1;
            /* Rotate servo to 90 degrees */
            Servo_SetAngle(90);
            printf("[Servo] Item detected, rotating to 90 degrees\n");
            sleep(2);  /* Hold position for 2 seconds */
            /* Return to 0 degrees */
            Servo_SetAngle(0);
            g_state.servo_activated = 0;
        } else {
            Servo_Stop();
        }

        usleep(200000);  /* 200ms */
    }
}

void StartSensorTask(void)
{
    osThreadAttr_t attr = {0};
    attr.name = "SensorTask";
    attr.stack_size = TASK_STACK_SIZE;
    attr.priority = osPriorityNormal;

    if (osThreadNew(SensorTask, NULL, &attr) == NULL) {
        printf("[ConveyorSystem] Failed to create SensorTask\n");
    }
}

void StartMotorTask(void)
{
    osThreadAttr_t attr = {0};
    attr.name = "MotorTask";
    attr.stack_size = TASK_STACK_SIZE;
    attr.priority = osPriorityNormal;

    if (osThreadNew(MotorTask, NULL, &attr) == NULL) {
        printf("[ConveyorSystem] Failed to create MotorTask\n");
    }
}

void StartServoTask(void)
{
    osThreadAttr_t attr = {0};
    attr.name = "ServoTask";
    attr.stack_size = TASK_STACK_SIZE;
    attr.priority = osPriorityNormal;

    if (osThreadNew(ServoTask, NULL, &attr) == NULL) {
        printf("[ConveyorSystem] Failed to create ServoTask\n");
    }
}
