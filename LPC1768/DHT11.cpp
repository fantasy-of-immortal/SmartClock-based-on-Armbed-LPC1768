#include "DHT11.h"

// DHT11初始化函数
void DHT11_Init(PinName pin) {
    DigitalInOut dht11_pin(pin);
    dht11_pin.output();
    dht11_pin = 1; // 初始拉高
    wait_ms(20);   // 上电稳定时间
}

// 微秒级延时函数
static void delay_us(int us) {
    wait_us(us);
}

// 读取DHT11数据
bool DHT11_Read(PinName pin, DHT11_Data *data) {
    uint8_t raw_data[5] = {0};
    uint8_t bit_idx = 7, byte_idx = 0;
    bool response = false;

    DigitalInOut dht11_pin(pin);

    // 发送开始信号
    dht11_pin.output();
    dht11_pin = 0;
    wait_ms(18);  // 拉低18ms
    dht11_pin = 1;
    wait_us(30);  // 拉高20-40us
    dht11_pin.input();

    // 检测DHT11响应信号
    wait_us(40);
    if (dht11_pin == 0) {
        wait_us(80);
        if (dht11_pin == 1) {
            response = true;
            wait_us(80);
        }
    }

    // 读取40位数据
    if (response) {
        for (int i = 0; i < 40; i++) {
            while (dht11_pin == 0); // 等待信号开始
            wait_us(30);           // 等待信号中间状态
            if (dht11_pin == 1) {
                raw_data[byte_idx] |= (1 << bit_idx); // 存储高位
            }
            while (dht11_pin == 1); // 等待信号结束

            if (bit_idx == 0) {
                bit_idx = 7;
                byte_idx++;
            } else {
                bit_idx--;
            }
        }
    }

    // 数据校验
    if (raw_data[4] != ((raw_data[0] + raw_data[1] + raw_data[2] + raw_data[3]) & 0xFF)) {
        return false; // 校验失败
    }

    // 解析数据
    data->humidity_int = raw_data[0];
    data->humidity_dec = raw_data[1];
    data->temperature_int = raw_data[2];
    data->temperature_dec = raw_data[3];

    return true;
}
