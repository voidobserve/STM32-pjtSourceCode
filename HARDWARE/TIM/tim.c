#include "tim.h"
#include "stm32f4xx.h"

volatile uint8_t tim6_update_cnt = 0; // 基本定时器TIM6更新事件的计数器（TIM6每5s产生一次更新中断）

// TIM14的初始化
void TIM14_PWMConfig(void)
{

	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;

	// 1.打开TIM14的时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);

	// 2.打开GPIO端口时钟  PF9
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

	// 3.配置GPIO引脚并初始化
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;		   // 引脚编号
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;	   // 复用模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // 输出速率
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	   // 推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	   // 上拉电阻
	GPIO_Init(GPIOF, &GPIO_InitStructure);

	// 4.需要选择引脚要复用的功能  PF9 -- TIM14_CH1
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource9, GPIO_AF_TIM14);

	// 5.配置定时器的基本参数 + 初始化   注意：脉冲信号的周期尽可能短一点  比如10ms
	TIM_TimeBaseStructure.TIM_Prescaler = 8400 - 1;				// TIM14 84MHZ / 8400 = 10000HZ  100us计数1次
	TIM_TimeBaseStructure.TIM_Period = 100 - 1;					// 10ms * 1000 / 100 = 100次
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;				// 不分频
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // TIM14只能递增计数
	TIM_TimeBaseInit(TIM14, &TIM_TimeBaseStructure);

	// 6.配置定时器的通道 + 初始化定时器通道
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;			  // PWM模式1  CNT < CCR1 通道有效
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // 使能输出比较
	TIM_OCInitStructure.TIM_Pulse = 50;							  // CCR寄存器的初值  默认占空比50%
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;	  // 高电平有效
	TIM_OC1Init(TIM14, &TIM_OCInitStructure);

	// 7.使能定时器通道对的预装载寄存器
	TIM_OC1PreloadConfig(TIM14, TIM_OCPreload_Enable);

	// 8.使能自动重载预装载寄存器
	TIM_ARRPreloadConfig(TIM14, ENABLE);

	// 9.打开定时器
	TIM_Cmd(TIM14, ENABLE);
}


// 基本定时器6初始化，用来给MQTT协议定时50s，每50秒让标志位置一
// 检测到这个标志位置一后，给服务器发送心跳包
// 初始化后，基本定时器每5s产生一次中断
void TIM6_Init(void)
{
		// 定义初始化结构体变量
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	// 打开定时器外设的时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

	// 给定时器初始化结构体变量的成员赋值
	TIM_TimeBaseStructure.TIM_Period = 50000 - 1;				// 重装载值
	TIM_TimeBaseStructure.TIM_Prescaler = 8400 - 1;				// 预分频值  84MHz / 8400 = 10000Hz，即1000ms有10000个信号波形，即1ms计数10次
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;				// 不分频
	// 初始化定时器6
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);

	// 配置嵌套向量中断控制器
	NVIC_InitStructure.NVIC_IRQChannel = TIM6_DAC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// 选择定时器的中断源，使用向上溢出中断
	TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);

	// 打开定时器6
	TIM_Cmd(TIM6, ENABLE);
}



