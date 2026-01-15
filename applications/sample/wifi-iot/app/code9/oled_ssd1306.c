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

#include "oled_ssd1306.h"
#include <stddef.h>
#include <stdio.h>
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_i2c.h"
#include "wifiiot_errno.h"
#include "oled_fonts.h"

#define ARRAY_SIZE(a) sizeof(a)/sizeof(a[0])

#define OLED_I2C_IDX WIFI_IOT_I2C_IDX_0

#define OLED_WIDTH    (128)
#define OLED_I2C_ADDR 0x78
#define OLED_I2C_CMD 0x00
#define OLED_I2C_DATA 0x40
#define OLED_I2C_BAUDRATE (400*1000)

static uint32_t I2cWriteByte(uint8_t regAddr, uint8_t byte)
{
    WifiIotI2cIdx id = OLED_I2C_IDX;
    uint8_t buffer[] = {regAddr, byte};
    WifiIotI2cData i2cData = {0};

    i2cData.sendBuf = buffer;
    i2cData.sendLen = sizeof(buffer)/sizeof(buffer[0]);

    return I2cWrite(id, OLED_I2C_ADDR, &i2cData);
}

static uint32_t WriteCmd(uint8_t cmd)
{
    return I2cWriteByte(OLED_I2C_CMD, cmd);
}

static uint32_t WriteData(uint8_t data)
{
    return I2cWriteByte(OLED_I2C_DATA, data);
}

uint32_t OledInit(void)
{
    static const uint8_t initCmds[] = {
        0xAE, // display off
        0x00, // set low column address
        0x10, // set high column address
        0x40, // set start line address
        0xB0, // set page address
        0x81, // contract control
        0xFF, // 128
        0xA1, // set segment remap
        0xA6, // normal / reverse
        0xA8, // set multiplex ratio(1 to 64)
        0x3F, // 1/32 duty
        0xC8, // Com scan direction
        0xD3, // set display offset
        0x00,
        0xD5, // set osc division
        0x80,
        0xD8, // set area color mode off
        0x05,
        0xD9, // Set Pre-Charge Period
        0xF1,
        0xDA, // set com pin configuration
        0x12,
        0xDB, // set Vcomh
        0x30,
        0x8D, // set charge pump enable
        0x14,
        0xAF, // turn on oled panel
    };

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_13, WIFI_IOT_IO_FUNC_GPIO_13_I2C0_SDA);
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_14, WIFI_IOT_IO_FUNC_GPIO_14_I2C0_SCL);

    I2cInit(WIFI_IOT_I2C_IDX_0, OLED_I2C_BAUDRATE);

    for (size_t i = 0; i < ARRAY_SIZE(initCmds); i++) {
        uint32_t status = WriteCmd(initCmds[i]);
        if (status != WIFI_IOT_SUCCESS) {
            return status;
        }
    }
    printf("[OLED] Initialized\n");
    return WIFI_IOT_SUCCESS;
}

void OledSetPosition(uint8_t x, uint8_t y)
{
    WriteCmd(0xb0 + y);
    WriteCmd(((x & 0xf0) >> 4) | 0x10);
    WriteCmd(x & 0x0f);
}

void OledFillScreen(uint8_t fillData)
{
    uint8_t m = 0;
    uint8_t n = 0;

    for (m = 0; m < 8; m++) {
        WriteCmd(0xb0 + m);
        WriteCmd(0x00);
        WriteCmd(0x10);

        for (n = 0; n < 128; n++) {
            WriteData(fillData);
        }
    }
}

void OledShowChar(uint8_t x, uint8_t y, uint8_t ch, Font font)
{
    uint8_t c = 0;
    uint8_t i = 0;

    c = ch - ' ';
    if (x > OLED_WIDTH - 1) {
        x = 0;
        y = y + 2;
    }

    if (font == FONT8x16) {
        OledSetPosition(x, y);
        for (i = 0; i < 8; i++) {
            WriteData(F8X16[c*16 + i]);
        }

        OledSetPosition(x, y+1);
        for (i = 0; i < 8; i++) {
            WriteData(F8X16[c*16 + i + 8]);
        }
    } else {
        OledSetPosition(x, y);
        for (i = 0; i < 6; i++) {
            WriteData(F6x8[c][i]);
        }
    }
}

void OledShowString(uint8_t x, uint8_t y, const char* str, Font font)
{
    uint8_t j = 0;
    if (str == NULL) {
        printf("param is NULL,Please check!!!\r\n");
        return;
    }

    while (str[j]) {
        OledShowChar(x, y, str[j], font);
        x += 8;
        if (x > 120) {
            x = 0;
            y += 2;
        }
        j++;
    }
}
