#include "oled.h" 
#include "DHT11.h"
#include "MP3TF16.h"

I2C i2c(p9, p10); // SDA, SCL
Serial pc(USBTX, USBRX); // 串口通信
DigitalOut VCC(p18);
DigitalOut VCCMP3(p16);

Ticker timer;
Ticker dht11Timer; 
Ticker MP3Timer;
PinName dht11_pin = p17; // DHT11连接的引脚

DHT11_Data dht11Data;

// 当前时间
time_t currentTime;
// 闹钟相关变量
time_t alarmTime = 0; // 闹钟时间
bool alarmSet = false; // 闹钟是否设置
bool alarmRepeat = false; // 是否重复闹钟


// 更新 OLED 上的时间显示
void UpdateOLED() {
    struct tm *ltm = localtime(&currentTime);
    char dateBuffer[11]; // YYYY-MM-DD
    char timeBuffer[9];  // HH:MM:SS

    strftime(dateBuffer, sizeof(dateBuffer), "%Y-%m-%d", ltm);
    strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", ltm);


    // 显示日期，使用小字体
    uint8_t dateStartX = (128 - strlen(dateBuffer) * 6) / 2; // 居中小字体
    OLED_ShowSmallString(i2c, dateStartX, 0, dateBuffer);

    // 显示时间，使用大字体
    uint8_t timeStartX = (128 - strlen(timeBuffer) * 8) / 2; // 居中大字体
    OLED_ShowLargeString(i2c, timeStartX, 2, timeBuffer); // 大字体占用多行

    // 显示温湿度数据
    char tempHumBuffer[32];
    sprintf(tempHumBuffer, "T:%d.%02dC H:%d.%02d%%",
            dht11Data.temperature_int, dht11Data.temperature_dec,
            dht11Data.humidity_int, dht11Data.humidity_dec);

    OLED_ShowSmallString(i2c, 0, 6, tempHumBuffer); // 温湿度数据位于第6行

}


// 处理串口接收的命令
void SerialReceived() {
    static char buffer[100];
    static int index = 0;
    while (pc.readable()) {
        char c = pc.getc();
        if (c == ';') { // 结束符
            buffer[index] = '\0'; // 结束字符串
            index = 0; // 重置索引

            // 解析同步命令
            if (strncmp(buffer, "SYNC:", 5) == 0) {
                struct tm t;
                if (sscanf(buffer + 5, "%d-%d-%dT%d:%d:%d", &t.tm_year, &t.tm_mon, &t.tm_mday, &t.tm_hour, &t.tm_min, &t.tm_sec) == 6) {
                    t.tm_year -= 1900; // 年份从1900开始
                    t.tm_mon -= 1; // 月份从0开始
                    currentTime = mktime(&t);
                    OLED_Clear(i2c);
                    UpdateOLED(); // 更新显示
                }
            }
            // 解析播放命令
            else if (strncmp(buffer, "PLAY:", 5) == 0) {
                int trackNumber = 0;
                if (sscanf(buffer + 5, "%d", &trackNumber) == 1) {
                    if (trackNumber >= 1 && trackNumber <= 255) {
                        MP3_PlayTrack(trackNumber);
                        pc.printf("Playing track: %d\n", trackNumber);
                    } else {
                        pc.printf("Error: Invalid track number\n");
                    }
                }
            }
            // 解析音量增加命令
            else if (strcmp(buffer, "VOL_UP") == 0) {
                MP3_VolumeUp();
                pc.printf("Volume increased\n");
            }
            // 解析音量降低命令
            else if (strcmp(buffer, "VOL_DOWN") == 0) {
                MP3_VolumeDown();
                pc.printf("Volume decreased\n");
            }
            // 解析暂停命令
            else if (strcmp(buffer, "PAUSE") == 0) {
                MP3_Pause();
                pc.printf("Playback paused\n");
            }
            // 解析播放命令
            else if (strcmp(buffer, "RESUME") == 0) {
                MP3_Play();
                pc.printf("Playback resumed\n");
            }
            // 解析停止命令
            else if (strcmp(buffer, "STOP") == 0) {
                MP3_Stop();
                pc.printf("Playback stopped\n");
            }
            // 解析下一首命令
            else if (strcmp(buffer, "NEXT") == 0) {
                MP3_NextTrack();
                pc.printf("Next track\n");
            }
            // 解析上一首命令
            else if (strcmp(buffer, "PREV") == 0) {
                MP3_PreviousTrack();
                pc.printf("Previous track\n");
            }
            else if (strncmp(buffer, "SETALARM:", 9) == 0) {
                int hours, minutes, repeat;
                if (sscanf(buffer + 9, "%d:%d:%d", &hours, &minutes, &repeat) == 3) {
                // 设置闹钟时间
                struct tm t;
                time(&currentTime);
                localtime_r(&currentTime, &t); // 获取当前时间
                t.tm_hour = hours;
             t.tm_min = minutes;
                t.tm_sec = 0;
                alarmTime = mktime(&t);
                alarmSet = true;
                alarmRepeat = repeat == 1; // 设置重复状态
                pc.printf("Alarm set for %02d:%02d, Repeat: %d\n", hours, minutes, repeat);
         } else {
              pc.printf("Error: Invalid alarm format\n");
             }
            }
        } else {
            if (index < sizeof(buffer) - 1) { // 确保不溢出
                buffer[index++] = c;
            }
        }
    }
}


void TimerCallback() {
    currentTime += 1; // 每次调用时更新当前时间
    UpdateOLED();

    // 检查闹钟时间
    if (alarmSet && currentTime >= alarmTime) {
        pc.printf("Alarm ringing!\n");
        MP3_PlayTrack(1);
        // 如果设置为不重复，则取消闹钟
        if (!alarmRepeat) {
            alarmSet = false; // 取消闹钟
        } else {
            // 如果是重复闹钟，重新设置闹钟时间
            alarmTime += 24 * 3600; // 设置为次日同一时间
        }
    }
}


// DHT11中断回调：读取温湿度数据
void DHT11InterruptHandler() {
    if (DHT11_Read(dht11_pin, &dht11Data)) {
        pc.printf("T:%d.%d;H:%d.%d;\n", dht11Data.temperature_int, dht11Data.temperature_dec,
                  dht11Data.humidity_int, dht11Data.humidity_dec);
    } else {
        pc.printf("Error: Failed to read from DHT11\n");
    }
}

void MP3TF16InterruptHandler(){

    MP3_QueryStatus();
    MP3_QueryCurrentTrack();

}


int main() {
    pc.baud(9600);
    mp3Serial.baud(9600);            // 设置波特率为 9600
    pc.attach(&SerialReceived, Serial::RxIrq);
    VCC = 1;
    VCCMP3 = 1;
    i2c.frequency(400000); // 设置 I2C 频率为 400kHz
    OLED_Init(i2c); // 初始化 OLED

    // 初始化当前时间为系统时间
    currentTime = time(NULL);
    UpdateOLED();
    // 设置定时器，每 1 秒调用一次 TimerCallback 函数
    timer.attach(&TimerCallback, 1.0); // 每秒更新当前时间
    dht11Timer.attach(&DHT11InterruptHandler, 2.0);
    MP3_Init();
    MP3Timer.attach(&MP3TF16InterruptHandler,5.0);

    while (true) {
    }
}