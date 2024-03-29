/*************************************************
 Copyright ? 0010. All rights reserved.
// 文件名
@ File name: ultraviolet_light.c -- 紫外线LED的驱动源程序
// 作者、工号、版本及完成日期
@ Author:    ya
@ ID：       0011
@ Version:
@ Date:
// 用于详细说明此程序文件完成的主要功能，与其他模块
// 或函数的接口，输出值、取值范围、含义及参数间的控
// 制、顺序、独立或依赖等关系
@ Description:
            紫外线LED的驱动源程序
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
#include "ultraviolet_light.h"
#include "stm32f4xx.h"

/**
 * @brief  紫外线LED控制端的初始化
 * 		 		紫外线LED使用5V供电，通过单片机控制继电器的开合来控制紫外线LED
 * 
 * @param  无
 * @retval 无
*/
void UlrtavioletLight_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

	// 时钟使能
	ULTRAVIOLET_LIGHT_RCC_CMD(ULTRAVIOLET_LIGHT_RCC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = ULTRAVIOLET_LIGHT_PIN ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(ULTRAVIOLET_LIGHT_PORT, &GPIO_InitStructure);

	// 设置为高电平，不亮
	ULTRAVIOLET_LIGHT_SET(1);
}


