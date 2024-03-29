#ifndef __TIM_H
#define __TIM_H

#include "stm32f4xx.h"

extern volatile uint8_t tim6_update_cnt; // 基本定时器TIM6更新事件的计数器（TIM6每5s产生一次更新中断）

// 基本定时器6初始化，用来给MQTT协议定时50s，每50秒让标志位置一
// 检测到这个标志位置一后，给服务器发送心跳包
// 初始化后，基本定时器每5s产生一次中断
void TIM6_Init(void);

void TIM14_PWMConfig(void);

#endif
