#ifndef __DUST_SENSOR_H
#define __DUST_SENSOR_H

#include "stm32f4xx.h"

extern volatile float pm2_5_mg_per_cubic_meter;

// =================================================================
// ====================================================
//-----------------粉尘传感器 模拟量输出端口相关定义----------------
// 粉尘传感器模拟量输出所连接的端口
#ifndef DUST_SENSOR_AO_PORT 
#define DUST_SENSOR_AO_PORT    GPIOA
#endif
// 粉尘传感器模拟量输出所连接的端口的引脚号
#ifndef DUST_SENSOR_AO_PIN 
#define DUST_SENSOR_AO_PIN    GPIO_Pin_6
#endif
// 粉尘传感器模拟量输出所连接的端口对应的时钟使能函数

// 粉尘传感器模拟量输出所连接的端口对应的时钟

// 控制LED，使用PE6

//-----------------粉尘传感器 模拟量输出端口相关定义----------------
// ====================================================
// =================================================================

#define DUST_LED_SET(N)  ( (N) ? (GPIO_SetBits(GPIOG, GPIO_Pin_12)) : (GPIO_ResetBits(GPIOG, GPIO_Pin_12)) )

void DustSensor_Init(void);
void DustSensor_GetVal(void);

void DustSensor_Test(void);

#endif

