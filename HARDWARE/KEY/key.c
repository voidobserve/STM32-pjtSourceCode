#include "key.h"
#include "delay.h"

volatile uint8_t key = 0;

#if 0
// 按键扫描使用方法
// while (1)
// {
// 	key = KEY_Scan(0);
// 	if (key)
// 	{
// 		switch (key)
// 		{
// 		case KEY0_PRES:
// 			LED1 = !LED1;
// 			break;
// 		case KEY1_PRES:
// 			LED2 = !LED2;
// 			break;
// 		case KEY2_PRES:
// 			LED3 = !LED3;
// 			break;
// 		case KEY3_PRES:
// 			LED4 = !LED4;
// 			break;
// 		default:
// 			break;
// 		}
// 	}
// }
#endif // end 按键扫描使用方法

// 按键初始化，使用按键前要先初始化延时delay_init()
void KEY_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	// 使能GPIOA  GPIOE 时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4; // KEY1 KEY2 KEY3 对应引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_Init(GPIOE, &GPIO_InitStructure); // 初始化GPIOE 2,3,4

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; // KEY0
	GPIO_Init(GPIOA, &GPIO_InitStructure);	  // 初始化GPIOA0
}

// 按键初始化，并且打开按键对应的中断
void KEY_IT_Init(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	// 1.打开GPIOA端口的时钟
	// RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	// 2.打开SYSCFG控制器的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	// 3.配置GPIO引脚  输入模式
	// 使能GPIOA  GPIOE 时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4; // KEY1 KEY2 KEY3 对应引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_Init(GPIOE, &GPIO_InitStructure); // 初始化GPIOE 2,3,4

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; // KEY0
	GPIO_Init(GPIOA, &GPIO_InitStructure);	  // 初始化GPIOA0

	// 4.把IO口和外部中断线进行连接
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource2);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource3);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource4);

	// 5.配置EXTI外部中断线(模式+边沿)
	EXTI_InitStructure.EXTI_Line = EXTI_Line0 | EXTI_Line2 | EXTI_Line3 | EXTI_Line4;				// 中断线编号
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;		// 中断模式
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; // 下降沿触发
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;				// 使能中断线
	EXTI_Init(&EXTI_InitStructure);

	// 6.NVIC外设的配置
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;			 // 中断通道编号
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00; // 抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;		 // 子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				 // 使能中断通道
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;			 // 中断通道编号
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00; // 抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;		 // 子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				 // 使能中断通道
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;			 // 中断通道编号
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00; // 抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;		 // 子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				 // 使能中断通道
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;			 // 中断通道编号
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00; // 抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;		 // 子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				 // 使能中断通道
	NVIC_Init(&NVIC_InitStructure);
}


// 按键处理函数
// 返回按键值
// mode: 0,不支持连续按  1，支持连续按
// 0,没有任何按键按下
// 1，KEY0按下
// 2，KEY1按下
// 3，KEY2按下
// 4，WK_UP按下
// 注意此函数有响应优先级，KEY0>KEY1>KEY2>WK_UP !!
u8 KEY_Scan(u8 mode)
{
	static u8 key_up = 1; // 按键按松开标志
	// static,静态变量，不随函数的返回而清零，而是一直保存在静态存储区中

	if (mode)
		key_up = 1; // 支持连按

	if (key_up && (KEY0 == 0 || KEY1 == 0 || KEY2 == 0 || KEY3 == 0))
	{
		// 如果有按键按下
		delay_ms(10); // 延时去抖动
		key_up = 0;	  // 按键标志位置零

		/*下面通过判断按下的键值，返回键值*/
		if (KEY0 == 0)
			return 1;
		else if (KEY1 == 0)
			return 2;
		else if (KEY2 == 0)
			return 3;
		else if (KEY3 == 0)
			return 4;
	}
	else if (KEY0 == 1 && KEY1 == 1 && KEY2 == 1 && KEY3 == 1)
	{
		key_up = 1; // 按键标志位置一
	}
	return 0; // 无按键按下
}
