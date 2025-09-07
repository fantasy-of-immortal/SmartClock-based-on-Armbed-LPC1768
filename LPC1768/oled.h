#ifndef __OLED_H
#define __OLED_H

#include "mbed.h"

// OLED I2C 地址
#define OLED_I2C_ADDRESS 0x78

// OLED 命令和数据模式
#define OLED_CMD 0x00
#define OLED_DATA 0x40

// 函数声明
void OLED_Init(I2C &i2c);
void OLED_Clear(I2C &i2c);
void OLED_ShowSmallChar(I2C &i2c, uint8_t x, uint8_t y, char chr);
void OLED_ShowSmallString(I2C &i2c, uint8_t x, uint8_t y, const char* str);
void OLED_ShowLargeChar(I2C &i2c, uint8_t x, uint8_t y, char chr);
void OLED_ShowLargeString(I2C &i2c, uint8_t x, uint8_t y, const char* str);

#endif // __OLED_H
