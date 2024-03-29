#include "usart2.h"
#include "stm32f4xx.h"

uint8_t USART2_RX_BUF[128]; // 接收缓冲
uint8_t USART2_RX_CNT = 0;
uint8_t USART2_RX_FALG = 0; // 接收状态标记

// baud:波特率
void USART2_Init(u32 baud)
{
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	// 1.打开GPIO端口  PA2 PA3
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	// 2.打开串口时钟  USART2 -- APB1
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	// 3.选择引脚的复用功能
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

	// 4.配置GPIO引脚参数并初始化
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;		   // 复用模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	   // 输出速度
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		   // 推挽复用
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;		   // 上拉电阻
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3; // 引脚编号
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// 5.配置USART1的参数并初始化
	USART_InitStructure.USART_BaudRate = baud;										// 波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						// 数据位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							// 停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;								// 检验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件流控
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					// 收发模式
	USART_Init(USART2, &USART_InitStructure);

	// 6.配置中断参数并初始化
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;		  // 中断通道编号
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; // 抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  // 子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // 打开中断通道
	NVIC_Init(&NVIC_InitStructure);

	// 7.选择中断源   接收到数据则触发中断
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

	// 8.打开USART2
	USART_Cmd(USART2, ENABLE);
}

// 接收到 '\r''\n'结束一次接收，接收完成标志位置一
void USART2_IRQHandler(void) // 串口2中断服务程序
{
	u8 recv;

	if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) // 接收中断(接收到的数据必须是0x0d 0x0a结尾，即\r\n结尾)
	{
		recv = USART_ReceiveData(USART2); // 读取接收到的数据

		if (0 == USART2_RX_FALG) // 接收未完成
		{
			USART2_RX_BUF[USART2_RX_CNT++] = recv;

			if (recv == '\n')
			{
				USART2_RX_CNT = 0;
				USART2_RX_FALG = 1; // 标记一次接收完成
			}
		}
	}
}
