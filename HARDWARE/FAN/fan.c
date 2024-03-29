/*************************************************
 Copyright ? 0010. All rights reserved.
// 文件名
@ File name: fan.c -- 风扇的驱动源程序
// 作者、工号、版本及完成日期
@ Author:    ya
@ ID：       0011
@ Version:
@ Date:
// 用于详细说明此程序文件完成的主要功能，与其他模块
// 或函数的接口，输出值、取值范围、含义及参数间的控
// 制、顺序、独立或依赖等关系
@ Description:
			风扇的驱动源程序

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
#include "fan.h"
#include "stm32f4xx.h"

// 记录风扇转速的全局变量
volatile int cur_fan_speed = CUR_FAN_SPEED_CLOSE;

/**
 * @brief  定时器通过PWM控制风扇的初始化函数
 *
 * @param  无
 * @retval 无
 */
void FAN_PWM_Init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;

	// 1.打开定时器的时钟
	FAN_TIM_RCC_CMD(FAN_TIM_RCC, ENABLE);

	// 2.打开GPIO端口时钟
	FAN_RCC_CMD(FAN_RCC, ENABLE);

	// 3.配置GPIO引脚并初始化
	GPIO_InitStructure.GPIO_Pin = FAN_PIN;			   // 引脚编号
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;	   // 复用模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // 输出速率
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	   // 推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	   // 上拉电阻
	GPIO_Init(FAN_PORT, &GPIO_InitStructure);

	FAN_SET_CLOSE();

	// 4.需要选择引脚要复用的功能  PB3 -- TIM2_CH2
	GPIO_PinAFConfig(FAN_PORT, FAN_PIN_SOURCE, FAN_AF_PERIPHERAL);

	// 5.配置定时器的基本参数 + 初始化   注意：脉冲信号的周期尽可能短一点  比如10ms
	TIM_TimeBaseStructure.TIM_Prescaler = 84 - 1;				// TIM2 84MHZ / 84 = 1000000HZ  1us计数1次
	TIM_TimeBaseStructure.TIM_Period = 100 - 1;					// 计数周期，计数100次
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;				// 不分频
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //
	TIM_TimeBaseInit(FAN_TIM, &TIM_TimeBaseStructure);

	// 6.配置定时器的通道 + 初始化定时器通道
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;			  // PWM模式1  CNT < CCR1 通道有效
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // 使能输出比较
	// TIM_OCInitStructure.TIM_Pulse = 50;							  // CCR寄存器的初值  默认占空比50%
	TIM_OCInitStructure.TIM_Pulse = 0;						  // CCR寄存器的初值  默认占空比0%，风扇不会转
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; // 高电平有效
	FAN_TIM_OC_INIT(FAN_TIM, &TIM_OCInitStructure);			  // TIM2_CH2初始化

	// 7.使能定时器通道对的预装载寄存器
	FAN_TIM_PRELOAD_CONFIG(FAN_TIM, TIM_OCPreload_Enable);

	// 8.使能自动重载预装载寄存器
	TIM_ARRPreloadConfig(FAN_TIM, ENABLE);

	// 9.打开定时器
	TIM_Cmd(FAN_TIM, ENABLE);

	FAN_SET_CLOSE();
}


// 根据记录风扇转速的变量调节风扇转速
void FAN_Speed_Adjust(void)
{
	switch (cur_fan_speed)
	{
	case CUR_FAN_SPEED_CLOSE:
		FAN_SET_CLOSE();
		break;
	case CUR_FAN_SPEED_MIN:
		FAN_SET_MIN_SPEED();
		break;
	case CUR_FAN_SPEED_LOW:
		FAN_SET_LOW_SPEED();
		break;
	case CUR_FAN_SPEED_MID:
		FAN_SET_MID_SPEED();
		break;
	case CUR_FAN_SPEED_MAX:
		FAN_SET_MAX_SPEED();
		break;
	default:
		break;
	}
}

void FAN_Speed_Inc(void)
{
	cur_fan_speed++;

	if (CUR_FAN_SPEED_MAX < cur_fan_speed)
	{
		cur_fan_speed = CUR_FAN_SPEED_MAX;
	}

	FAN_Speed_Adjust();
}


void FAN_Speed_dec(void)
{
	cur_fan_speed--;

	if (CUR_FAN_SPEED_CLOSE > cur_fan_speed)
	{
		cur_fan_speed = CUR_FAN_SPEED_CLOSE;
	}

	FAN_Speed_Adjust();
}
