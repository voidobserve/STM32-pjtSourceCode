/*************************************************
 Copyright ? 0010. All rights reserved.
// 文件名
@ File name: fan.h -- 风扇的驱动程序的头文件
// 作者、工号、版本及完成日期
@ Author:    ya
@ ID：       0011
@ Version:
@ Date:
// 用于详细说明此程序文件完成的主要功能，与其他模块
// 或函数的接口，输出值、取值范围、含义及参数间的控
// 制、顺序、独立或依赖等关系
@ Description:
            风扇的驱动程序的头文件

			注意: 风扇的PWM控制需要根据驱动定时器的时钟频率和调试获得，
				 因此具体的使用中可能还要修改其中的初始化代码
                     
@ Others:  // 其它内容的说明
@ History: // 修改历史记录列表，每条修改记录应包括修改日期、修改
|		   // 者及修改内容简述
|--@ 1. Date:
|--@ Author:
|--@ ID:
|--@ Modification:
|--@ 2. ...
*************************************************/
#ifndef __FAN_H
#define __FAN_H

#include "my_config.h"

enum 
{
	CUR_FAN_SPEED_CLOSE = 0,
	CUR_FAN_SPEED_MIN,
	CUR_FAN_SPEED_LOW,
	CUR_FAN_SPEED_MID,
	CUR_FAN_SPEED_MAX
};

extern volatile int cur_fan_speed; 

// =================================================================
// ====================================================
//-----------------风扇控制端口相关定义----------------

// 风扇控制引脚对应的端口
#ifndef FAN_PORT 
#define FAN_PORT             GPIOB
#endif 
// 风扇控制引脚对应的端口的引脚号
#ifndef FAN_PIN
#define FAN_PIN              GPIO_Pin_3
#endif
// 风扇控制引脚对应的端口的时钟使能函数
#ifndef FAN_RCC_CMD
#define FAN_RCC_CMD          RCC_AHB1PeriphClockCmd
#endif
// 风扇控制引脚对应的端口的时钟
#ifndef FAN_RCC
#define FAN_RCC              RCC_AHB1Periph_GPIOB
#endif
// 风扇控制引脚复用的引脚资源
#ifndef FAN_PIN_SOURCE
#define FAN_PIN_SOURCE       GPIO_PinSource3
#endif
// 风扇控制引脚复用的外设资源
#ifndef FAN_AF_PERIPHERAL
#define FAN_AF_PERIPHERAL    GPIO_AF_TIM2
#endif
// 风扇控制引脚对应的定时器
#ifndef FAN_TIM
#define FAN_TIM              TIM2
#endif
// 风扇控制引脚对应的定时器的时钟使能函数
#ifndef FAN_TIM_RCC_CMD
#define FAN_TIM_RCC_CMD      RCC_APB1PeriphClockCmd
#endif
// 风扇控制引脚对应的定时器的时钟
#ifndef FAN_TIM_RCC
#define FAN_TIM_RCC          RCC_APB1Periph_TIM2
#endif
// 风扇控制引脚对应的定时器的输出通道使能函数
#ifndef FAN_TIM_OC_INIT
#define FAN_TIM_OC_INIT      TIM_OC2Init
#endif
// 风扇控制引脚对应的定时器通道的预装载寄存器使能函数
#ifndef FAN_TIM_PRELOAD_CONFIG
#define FAN_TIM_PRELOAD_CONFIG TIM_OC2PreloadConfig
#endif


//-----------------风扇控制端口相关定义----------------
// ====================================================
// =================================================================

#define FAN_SET_CLOSE()              TIM_SetCompare2(FAN_TIM, 0)  // 关闭风扇
#define FAN_SET_MIN_SPEED()          TIM_SetCompare2(FAN_TIM, 65) // 靠近听才有声音
#define FAN_SET_LOW_SPEED()          TIM_SetCompare2(FAN_TIM, 75) // 有些许声音
#define FAN_SET_MID_SPEED()          TIM_SetCompare2(FAN_TIM, 80) // 有连续的声音
#define FAN_SET_MAX_SPEED()          TIM_SetCompare2(FAN_TIM, 99) // 风扇最大转速

// void FAN_Init(void); // 测试能不能通过继电器控制风扇的程序
void FAN_PWM_Init(void); // 定时器通过PWM控制风扇的初始化函数


// 根据记录风扇转速的变量调节风扇转速
void FAN_Speed_Adjust(void);
void FAN_Speed_Inc(void);
void FAN_Speed_dec(void);

#endif

