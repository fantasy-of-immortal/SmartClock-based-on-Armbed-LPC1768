#ifndef MP3TF16_H
#define MP3TF16_H

#include "mbed.h"

// MP3 模块 UART 默认设置
#define MP3_BAUDRATE 9600

// MP3 模块命令定义
#define MP3_CMD_NEXT        0x01
#define MP3_CMD_PREV        0x02
#define MP3_CMD_PLAY_TRACK  0x03
#define MP3_CMD_VOL_UP      0x04
#define MP3_CMD_VOL_DOWN    0x05
#define MP3_CMD_SET_VOL     0x06
#define MP3_CMD_PLAY        0x0D
#define MP3_CMD_PAUSE       0x0E
#define MP3_CMD_STOP        0x16

// 全局串口对象声明
extern Serial mp3Serial;  // 用于MP3模块通信

// 初始化 MP3 控制
void MP3_Init();

// 播放控制
void MP3_Play();
void MP3_Pause();
void MP3_Stop();
void MP3_NextTrack();
void MP3_PreviousTrack();

// 音量控制
void MP3_VolumeUp();
void MP3_VolumeDown();
void MP3_SetVolume(uint8_t volume);

// 播放指定曲目
void MP3_PlayTrack(uint16_t track);

// 查询模块状态
void MP3_QueryStatus();
void MP3_QueryCurrentTrack();

#endif // MP3_CONTROL_H
