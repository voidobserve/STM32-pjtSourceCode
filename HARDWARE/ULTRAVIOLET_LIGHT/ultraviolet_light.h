/*************************************************
 Copyright ? 0010. All rights reserved.
// 文件名
@ File name: ultraviolet_light.h -- 紫外线LED的头文件
// 作者、工号、版本及完成日期
@ Author:    ya
@ ID：       0011
@ Version:
@ Date:
// 用于详细说明此程序文件完成的主要功能，与其他模块
// 或函数的接口，输出值、取值范围、含义及参数间的控
// 制、顺序、独立或依赖等关系
@ Description:
            紫外线LED的头文件
          紫外线LED使用5V供电，通过单片机控制继电器的开合来控制紫外线LED，
          继电器的控制段配置为低电平使能。

@ Others:  // 其它内容的说明
@ History: // 修改历史记录列表，每条修改记录应包括修改日期、修改
|		   // 者及修改内容简述
|--@ 1. Date:
|--@ Author:
|--@ ID:
|--@ Modification:
|--@ 2. ...
*************************************************/
#ifndef __ULTRAVIOLET_H
#define __ULTRAVIOLET_H

#include "stm32f4xx.h"
#include "my_config.h" 

// =================================================================
// ====================================================
//-----------------紫外线LED控制端口相关定义----------------

// 紫外线LED控制引脚对应的端口
#ifndef ULTRAVIOLET_LIGHT_PORT 
#define ULTRAVIOLET_LIGHT_PORT             GPIOC
#endif 
// 紫外线LED控制引脚对应的端口的引脚号
#ifndef ULTRAVIOLET_LIGHT_PIN
#define ULTRAVIOLET_LIGHT_PIN              GPIO_Pin_7
#endif
// 紫外线LED控制引脚对应的端口的时钟使能函数
#ifndef ULTRAVIOLET_LIGHT_RCC_CMD
#define ULTRAVIOLET_LIGHT_RCC_CMD          RCC_AHB1PeriphClockCmd
#endif
// 紫外线LED控制引脚对应的端口的时钟
#ifndef ULTRAVIOLET_LIGHT_RCC
#define ULTRAVIOLET_LIGHT_RCC              RCC_AHB1Periph_GPIOC
#endif

//-----------------紫外线LED控制端口相关定义----------------
// ====================================================
// =================================================================

// 紫外线LED控制引脚输出高低电平的宏函数，0--输出低电平，1--输出高电平
#define ULTRAVIOLET_LIGHT_SET(n)   \
     ((n) ? GPIO_SetBits(ULTRAVIOLET_LIGHT_PORT, ULTRAVIOLET_LIGHT_PIN) : GPIO_ResetBits(ULTRAVIOLET_LIGHT_PORT, ULTRAVIOLET_LIGHT_PIN))

void UlrtavioletLight_Init(void); // 紫外线LED控制端的初始化

#endif

