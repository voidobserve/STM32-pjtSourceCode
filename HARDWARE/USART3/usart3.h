#ifndef __USART3_H
#define __USART3_H

#include "stm32f4xx.h"

#ifndef USART_REC_LEN
#define USART_REC_LEN 512 // 定义最大接收字节数
#endif                    // end def USART_REC_LEN

extern u8 USART3_RX_DATA;               // 串口3接收到的数据--1字节
extern u8 USART3_RX_BUF[USART_REC_LEN]; // 接收缓冲,最大USART_REC_LEN个字节.末字节为换行符
extern u8 USART3_RX_STA;                // 接收状态标记
extern u8 USART3_RX_CNT;                // 记录接收缓冲区中当前接收了多少个字节

extern uint8_t Tx3Buffer[512];
extern volatile uint32_t Rx3Counter;
extern volatile uint8_t Rx3Data;
extern volatile uint8_t Rx3End;
extern volatile uint8_t Rx3Buffer[512];

void USART3_Init(u32 bound);
void USART3_SendByte(u8 Byte);
void USART3_SendString(char *str);
void USART_SendBytes(USART_TypeDef *pUSARTx, uint8_t *buf, uint32_t len);

#endif
