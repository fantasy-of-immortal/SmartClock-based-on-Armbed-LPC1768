#include "MP3TF16.h"

PinName tx = p13;
PinName rx = p14;
extern Serial pc;  // USB串口通信
Serial mp3Serial(tx,rx); 

// 发送命令
// 发送命令到 MP3 模块
static void MP3_SendCommand(uint8_t cmd, uint8_t feedback, uint16_t dat) {
    uint8_t Send_buf[8];

    // 构造数据帧
    Send_buf[0] = 0x7E;                   // 起始位
    Send_buf[1] = 0xFF;                   // 版本号
    Send_buf[2] = 0x06;                   // 数据长度
    Send_buf[3] = cmd;                    // 控制指令
    Send_buf[4] = feedback;               // 是否需要反馈
    Send_buf[5] = (uint8_t)(dat >> 8);    // 数据高字节
    Send_buf[6] = (uint8_t)(dat);         // 数据低字节
    Send_buf[7] = 0xEF;                   // 结束位

    // 发送数据
    for (int i = 0; i < 8; i++) {
        mp3Serial.putc(Send_buf[i]);
    }
}

// 接收响应数据
uint8_t MP3_ReceiveResponse() {
    uint8_t response[10] = {0};
    int bytesRead = 0;
    int attempts = 0; // 读取尝试次数

    // 最多尝试10次读取响应
    while (attempts < 10) {
        if (mp3Serial.readable()) {
            response[bytesRead++] = mp3Serial.getc();
            // 检查是否接收到完整的指令
            if (bytesRead >= 10) {
                if (response[0] == 0x7E && response[9] == 0xEF) {
                    uint8_t status = response[6]; // 状态码位于第7字节
                    pc.printf("Response received: %02X\n", status);
                    return status;
                }
                // 清空读取到的数据，准备接收新的数据
                bytesRead = 0; 
            }
        }
        // 延时以防止过快的循环
        wait_ms(100); 
        attempts++;
    }

    // 超过最大尝试次数未接收到完整的响应
    pc.printf("Error: No response received after 10 attempts\n");
    return 0xFF; // 返回错误状态
}

// 初始化 MP3 模块
void MP3_Init() {
    pc.printf("Initializing MP3 module...\n");

    // 初始化延时，确保模块稳定
    wait_ms(3000);
    MP3_SendCommand(0x0C, 0x00, 0x00); 
    wait_ms(3000);
    // 查询设备状态
    MP3_SendCommand(0x42, 0x00, 0x00); // 查询设备状态命令
    wait_ms(30);
    // 读取响应
    uint8_t response = MP3_ReceiveResponse();

    // 根据响应状态输出结果
    switch (response) {
        case 0x01: pc.printf("Device: USB\n"); break;
        case 0x02: pc.printf("Device: TF Card\n"); break;
        case 0x04: pc.printf("Device: PC\n"); break;
        case 0x08: pc.printf("Device: FLASH\n"); break;
        case 0x03: pc.printf("Device: USB and TF Card\n"); break;
        default:   pc.printf("Error: %d\n", response); break;
    }

    wait_ms(1000); // 确保模块初始化完成
}
// 播放
void MP3_Play() {
    MP3_SendCommand(MP3_CMD_PLAY, 0, 0);
}

// 暂停
void MP3_Pause() {
    MP3_SendCommand(MP3_CMD_PAUSE, 0, 0);
}

// 停止
void MP3_Stop() {
    MP3_SendCommand(MP3_CMD_STOP, 0, 0);
}

// 下一曲
void MP3_NextTrack() {
    MP3_SendCommand(MP3_CMD_NEXT, 0, 0);
}

// 上一曲
void MP3_PreviousTrack() {
    MP3_SendCommand(MP3_CMD_PREV, 0, 0);
}

// 音量增加
void MP3_VolumeUp() {
    MP3_SendCommand(MP3_CMD_VOL_UP, 0, 0);
}

// 音量减少
void MP3_VolumeDown() {
    MP3_SendCommand(MP3_CMD_VOL_DOWN, 0, 0);
}

// 设置音量
void MP3_SetVolume(uint8_t volume) {
    if (volume > 30) {
        volume = 30;
    }
    MP3_SendCommand(MP3_CMD_SET_VOL, 0, volume);
}

// 播放指定曲目
void MP3_PlayTrack(uint16_t track) {
    uint8_t param1 = (track >> 8) & 0xFF; // 高字节
    uint8_t param2 = track & 0xFF;       // 低字节
    MP3_SendCommand(MP3_CMD_PLAY_TRACK, param1, param2);
}

// 查询播放状态
void MP3_QueryStatus() {
    MP3_SendCommand(0x42, 0, 0); // 0x42 为查询状态命令
    uint8_t status = MP3_ReceiveResponse();
    switch (status) {
    case 0x00:
        pc.printf("Model: %d\n", 0);
        break;
    case 0x01:
        pc.printf("Model: %d\n", 1);
        break;
    case 0x02:
        pc.printf("Model: %d\n", 2);
        break;
    case 0x08:
        pc.printf("Device in sleep mode\n");
        break;
    default:
        pc.printf("Unknown state: %d\n", status);
        break;
    }
}

// 查询当前曲目
void MP3_QueryCurrentTrack() {
    MP3_SendCommand(0x4C, 0, 0); // 0x4C 为查询当前TF卡曲目的命令
    uint8_t status = MP3_ReceiveResponse();
    pc.printf("Song: %d\n", status);
}
