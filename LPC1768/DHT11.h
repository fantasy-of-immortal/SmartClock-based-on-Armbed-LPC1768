#ifndef DHT11_H
#define DHT11_H

#include "mbed.h"

// 温湿度数据结构体
typedef struct {
    uint8_t humidity_int;   // 湿度整数部分
    uint8_t humidity_dec;   // 湿度小数部分
    uint8_t temperature_int; // 温度整数部分
    uint8_t temperature_dec; // 温度小数部分
} DHT11_Data;

// 初始化DHT11
void DHT11_Init(PinName pin);

// 读取DHT11数据
bool DHT11_Read(PinName pin, DHT11_Data *data);

#endif // DHT11_H
