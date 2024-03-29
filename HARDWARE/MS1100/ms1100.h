/*************************************************
 Copyright ? 0010. All rights reserved.

// 文件名
@ File name: ms1100.h -- ms1100甲醛检测模块的头文件
// 作者、工号、版本及完成日期
@ Author:    ya
@ ID：       0011
@ Version:
@ Date:
// 用于详细说明此程序文件完成的主要功能，与其他模块
// 或函数的接口，输出值、取值范围、含义及参数间的控
// 制、顺序、独立或依赖等关系
@ Description:
            ms1100.c -- ms1100甲醛检测模块的头文件
        注意：MS1100甲醛检测模块输出的模拟电压值是0~5V，
            要先确定MCU对应的IO口是否有5V容忍

@ Others:  // 其它内容的说明
@ History: // 修改历史记录列表，每条修改记录应包括修改日期、修改
|		   // 者及修改内容简述
|--@ 1. Date:
|--@ Author:
|--@ ID:
|--@ Modification:
|--@ 2. ...
*************************************************/
#ifndef __MS1100_H
#define __MS1100_H

#include "stm32f4xx.h"
#include "my_config.h"

extern volatile float hcho_mg_per_cubic_meter;

// ============================================================================================
// ===============================================================================
//-----------------MS1100甲醛浓度检测模块的相关定义----------------
// 与甲醛浓度检测模块ADC输出相连接的GPIO端口
#ifndef MS1100_PROT
#define MS1100_PROT              GPIOA
#endif
// 与甲醛浓度检测模块ADC输出相连接的GPIO端口的引脚号
#ifndef MS1100_PIN
#define MS1100_PIN               GPIO_Pin_6
#endif
// 与甲醛浓度检测模块ADC输出相连接的GPIO端口的时钟使能函数
#ifndef MS1100_RCC_CMD
#define MS1100_RCC_CMD           RCC_AHB1PeriphClockCmd
#endif
// 与甲醛浓度检测模块ADC输出相连接的GPIO端口的时钟
#ifndef MS1100_RCC
#define MS1100_RCC               RCC_AHB1Periph_GPIOA
#endif
// 与甲醛浓度检测模块ADC输出相连接的GPIO端口复用到的ADC
#ifndef MS1100_ADC               
#define MS1100_ADC          ADC1
#endif
// ADC的时钟使能函数
#ifndef MS1100_ADC_RCC_CMD
#define MS1100_ADC_RCC_CMD  RCC_APB2PeriphClockCmd
#endif
// ADC的时钟
#ifndef MS1100_ADC_RCC
#define MS1100_ADC_RCC  RCC_APB2Periph_ADC1
#endif
// ADC的规则通道
#ifndef MS1100_ADC_REGULARCHANNEL
#define MS1100_ADC_REGULARCHANNEL ADC_Channel_6
#endif

//-----------------MS1100甲醛检测模块的相关定义----------------
// ===============================================================================
// ============================================================================================

void MS1100_Init(void);                // MS1100甲醛传感器初始化函数
float MS1100_ADCValToPPM(u16 adc_val); // 将采集到的电压值转换成以ppm为单位表示的浮点数值

void MS1100_Get_Mg_per_cubic_meter(void); // 获取一次甲醛浓度的数值，存放到全局变量中

void MS1100_Test(void);                // MS1100甲醛模块的测试函数（里面是个死循环）

#endif
