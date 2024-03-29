#ifndef __USART2_H
#define __USART2_H

#include "stm32f4xx.h"
	
extern uint8_t USART2_RX_BUF[128]; // 接收缓冲
extern uint8_t USART2_RX_FALG; // 接收状态标记


void USART2_Init(u32 baud);


#endif


