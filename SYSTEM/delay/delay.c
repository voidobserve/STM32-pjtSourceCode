#include "delay.h"
#include "stm32f4xx.h"

// 延时微秒  非阻塞延时
void delay_us(u32 nus)
{
	int cnt = 0;  // 用于存储计数的总次数
	int load = 0; // 用于记录Systick的自动重载寄存器的值
	int told = 0; // 用于记录Systick的当前数值寄存器的初值
	int tnew = 0; // 用于记录Systick的当前数值寄存器的数值
	int sum = 0;  // 记录Systick的计数次数

	// 1.计算延时时间对应的计数次数  Systick的时钟源是168MHZ 所以1us计数168次
	cnt = nus * 168;

	// 2.记录Systick的自动重载寄存器的值
	load = SysTick->LOAD;

	// 3.记录Systick的当前数值寄存器的初值
	told = SysTick->VAL;

	// 4.循环记录当前数值寄存器的计数次数并和要延时的计数总数进行比较即可
	while (1)
	{
		// 5.获取Systick的当前数值寄存器的值
		tnew = SysTick->VAL;

		// 6.判断是否可以一轮数完
		if (told != tnew)
		{
			if (told < tnew)
				sum += load - tnew + told;
			else
				sum += told - tnew;

			told = tnew;

			if (sum >= cnt) // 说明时间到达
				break;
		}
	}
}

// 延时毫秒  非阻塞延时
void delay_ms(u32 nms)
{
	int i = 0;
	for (i = 0; i < nms; i++)
		delay_us(1000);
}
