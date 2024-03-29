#ifndef __MY_IIC_H
#define __MY_IIC_H

#include <stdbool.h>
#include "stm32f4xx.h"

void IIC_Config(void);
void IIC_SDASetOutputMode(void);
void IIC_SDASetInputMode(void);
void IIC_Start(void);
void IIC_SendByte(uint8_t data);
bool IIC_WaitSlaveAck(void);
uint8_t IIC_ReadByte(void);
void IIC_MasterSendAck(uint8_t ack);
void IIC_Stop(void);

#endif
