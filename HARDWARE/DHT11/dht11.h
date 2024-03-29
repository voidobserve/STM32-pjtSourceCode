/*************************************************
 Copyright ? 0010. All rights reserved.
// 文件名
@ File name: dht11.h -- dht11温湿度传感器的头文件
// 作者、工号、版本及完成日期
@ Author:    ya
@ ID：       0011
@ Version:
@ Date:
// 用于详细说明此程序文件完成的主要功能，与其他模块
// 或函数的接口，输出值、取值范围、含义及参数间的控
// 制、顺序、独立或依赖等关系
@ Description:
            dht11温湿度传感器的头文件

@ Others:  // 其它内容的说明
@ History: // 修改历史记录列表，每条修改记录应包括修改日期、修改
|		   // 者及修改内容简述
|--@ 1. Date:
|--@ Author:
|--@ ID:
|--@ Modification:
|--@ 2. ...
*************************************************/
#ifndef __DHT11_H
#define __DHT11_H

#include <stdbool.h>
#include "stm32f4xx.h"

#include "my_config.h"

extern uint8_t dhtbuf[5];

// =================================================================
// ====================================================
//-----------------温湿度传感器DHT11端口相关定义----------------

// 温湿度传感器DHT11的DATA数据引脚对应的端口
#ifndef DHT11_DATA_PORT 
#define DHT11_DATA_PORT             GPIOG
#endif 
// 温湿度传感器DHT11的DATA数据引脚对应的端口的引脚号
#ifndef DHT11_DATA_PIN
#define DHT11_DATA_PIN              GPIO_Pin_9
#endif
// 温湿度传感器DHT11的DATA数据引脚的时钟使能函数
#ifndef DHT11_DATA_RCC_CMD
#define DHT11_DATA_RCC_CMD          RCC_AHB1PeriphClockCmd
#endif
// 温湿度传感器DHT11的DATA数据引脚的时钟
#ifndef DHT11_DATA_RCC
#define DHT11_DATA_RCC              RCC_AHB1Periph_GPIOG
#endif

//-----------------温湿度传感器DHT11端口相关定义----------------
// ====================================================
// =================================================================

#define DHT11_SET(n) \
       ((n) ? GPIO_SetBits(DHT11_DATA_PORT, DHT11_DATA_PIN) : GPIO_ResetBits(DHT11_DATA_PORT, DHT11_DATA_PIN))

#define DHT11_READ          GPIO_ReadInputDataBit(DHT11_DATA_PORT, DHT11_DATA_PIN)

void DHT11_Init(void); // DHT11相应端口的初始化函数

void DHT11_SetInputMode(void);  // 将DHT11相应端口设置为输入模式
void DHT11_SetOutputMode(void); // 将DHT11相应端口设置为输出模式

void DHT11_SendStart(void); // 给DHT11发送开始信号
bool DHT11_GetAck(void);    // 等待DHT11的应答信号（含超时判断）

bool DHT11_GetDataBit(u8 *databit); //从DHT11获取一位数据（含超时判断）
bool DHT11_GetData(u8 buf[5]);      // 获取DHT11的温湿度数据

void DHT11_Test(void); // DHT11测试函数，内部是个死循环，一般只在调试的时候使用

#endif
