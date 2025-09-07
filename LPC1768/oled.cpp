#include "oled.h"
#include "font.h" 


// 写入 OLED 字节
void OLED_WR_Byte(I2C &i2c, uint8_t dat, uint8_t cmd) {
    char data[2];
    data[0] = cmd;
    data[1] = dat;
    i2c.write(OLED_I2C_ADDRESS, data, 2);
}

// OLED 清屏函数
void OLED_Clear(I2C &i2c) {
    for (uint8_t i = 0; i < 8; i++) {
        OLED_WR_Byte(i2c, 0xB0 + i, OLED_CMD);
        OLED_WR_Byte(i2c, 0x00, OLED_CMD);
        OLED_WR_Byte(i2c, 0x10, OLED_CMD);
        for (uint8_t j = 0; j < 128; j++) {
            OLED_WR_Byte(i2c, 0x00, OLED_DATA);
        }
    }
}

// OLED 初始化函数
void OLED_Init(I2C &i2c) {
    wait_ms(200); // 等待 OLED 复位完成

    OLED_WR_Byte(i2c, 0xAE, OLED_CMD); // 关闭显示
    OLED_WR_Byte(i2c, 0xD5, OLED_CMD); // 设置时钟分频因子, 震荡频率
    OLED_WR_Byte(i2c, 0x80, OLED_CMD); //[3:0], 分频因子; [7:4], 震荡频率
    OLED_WR_Byte(i2c, 0xA8, OLED_CMD); // 设置驱动路数
    OLED_WR_Byte(i2c, 0x3F, OLED_CMD); // 默认0X3F(1/64)
    OLED_WR_Byte(i2c, 0xD3, OLED_CMD); // 设置显示偏移
    OLED_WR_Byte(i2c, 0x00, OLED_CMD); // 默认为0
    OLED_WR_Byte(i2c, 0x40, OLED_CMD); // 设置显示开始行
    OLED_WR_Byte(i2c, 0x8D, OLED_CMD); // 电荷泵设置
    OLED_WR_Byte(i2c, 0x14, OLED_CMD); // 开启电荷泵
    OLED_WR_Byte(i2c, 0x20, OLED_CMD); // 设置内存地址模式
    OLED_WR_Byte(i2c, 0x00, OLED_CMD); // 水平地址模式
    OLED_WR_Byte(i2c, 0xA1, OLED_CMD); // 段重定义设置
    OLED_WR_Byte(i2c, 0xC8, OLED_CMD); // 设置COM扫描方向
    OLED_WR_Byte(i2c, 0xDA, OLED_CMD); // 设置COM硬件引脚配置
    OLED_WR_Byte(i2c, 0x12, OLED_CMD); // 默认设置
    OLED_WR_Byte(i2c, 0x81, OLED_CMD); // 对比度设置
    OLED_WR_Byte(i2c, 0xCF, OLED_CMD); // 对比度值
    OLED_WR_Byte(i2c, 0xD9, OLED_CMD); // 设置预充电周期
    OLED_WR_Byte(i2c, 0xF1, OLED_CMD); // 预充电周期
    OLED_WR_Byte(i2c, 0xDB, OLED_CMD); // 设置VCOMH电压倍率
    OLED_WR_Byte(i2c, 0x40, OLED_CMD); // 默认设置
    OLED_WR_Byte(i2c, 0xA4, OLED_CMD); // 全局显示开启
    OLED_WR_Byte(i2c, 0xA6, OLED_CMD); // 设置显示方式
    OLED_WR_Byte(i2c, 0xAF, OLED_CMD); // 开启显示

    OLED_Clear(i2c); // 清屏
}

void OLED_ShowSmallChar(I2C &i2c, uint8_t x, uint8_t y, char chr) {
    uint8_t c = chr - ' '; // 获取字符的索引
    if (c >= 95) return; // 检查字符是否在支持范围内

    OLED_WR_Byte(i2c, 0xB0 + y, OLED_CMD); // 设置页地址
    OLED_WR_Byte(i2c, 0x00 + (x & 0x0F), OLED_CMD); // 设置低列地址
    OLED_WR_Byte(i2c, 0x10 + ((x >> 4) & 0x0F), OLED_CMD); // 设置高列地址

    for (int i = 0; i < 6; i++) {
        OLED_WR_Byte(i2c, asc2_0806[c][i], OLED_DATA); // 写入字符数据
    }
}

void OLED_ShowSmallString(I2C &i2c, uint8_t x, uint8_t y, const char* str) {
    while (*str) {
        OLED_ShowSmallChar(i2c, x, y, *str);
        x += 6; // 每个小字体字符占用 6 列
        if (x > 122) {
            x = 0;
            y++;
        }
        str++;
    }
}


void OLED_ShowLargeChar(I2C &i2c, uint8_t x, uint8_t y, char chr) {
    uint8_t c = chr - ' '; // 获取字符的索引
    if (c < 0 || c > 94) return; // 检查字符是否在支持范围内
    OLED_WR_Byte(i2c, 0xB0 + y, OLED_CMD); // 设置页地址
    OLED_WR_Byte(i2c, 0x00 + (x & 0x0F), OLED_CMD); // 设置低列地址
    OLED_WR_Byte(i2c, 0x10 + ((x >> 4) & 0x0F), OLED_CMD); // 设置高列地址
    // 显示字符的前 8 行
    for (int i = 0; i < 8; i++) {
        OLED_WR_Byte(i2c, asc2_1608[c][i], OLED_DATA); // 发送字符数据
    }

    // 设置页地址和列地址到下一行
    OLED_WR_Byte(i2c, 0xB0 + y + 1, OLED_CMD); // 设置下一页地址
    OLED_WR_Byte(i2c, 0x00 + (x & 0x0F), OLED_CMD); // 设置低列地址
    OLED_WR_Byte(i2c, 0x10 + ((x >> 4) & 0x0F), OLED_CMD); // 设置高列地址

    // 显示字符的后 8 行
    for (int i = 8; i < 16; i++) {
        OLED_WR_Byte(i2c, asc2_1608[c][i], OLED_DATA); // 发送字符数据
    }
}




void OLED_ShowLargeString(I2C &i2c, uint8_t x, uint8_t y, const char* str) {
    while (*str) {
        OLED_ShowLargeChar(i2c, x, y, *str); 
        x += 8; // 每个大字体字符占用 12 列
        if (x > 122) { // 如果 x 超过最大宽度，则换行
            x = 0;
            y ++; // y 增加，换行

        }
        str++;
    }
}



