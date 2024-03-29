#include "usart3.h"
#include "stm32f4xx.h"

uint8_t Tx3Buffer[512];
volatile uint32_t Rx3Counter = 0;
volatile uint8_t Rx3Data = 0;
volatile uint8_t Rx3End = 0;
volatile uint8_t Rx3Buffer[512] = {0};


// 初始化IO 串口3
// bound:波特率
void USART3_Init(u32 bound)
{
    // GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);  // 使能GPIOB时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE); // 使能USART3时钟

    // 串口3对应引脚复用映射
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3); // GPIOB10复用为USART3
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3); // GPIOB11复用为USART3

    // USART1端口配置
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11; // GPIOB10与GPIOB11
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;             // 复用功能
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;       // 速度
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;           // 推挽复用输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;             // 上拉
    GPIO_Init(GPIOB, &GPIO_InitStructure);                   // 初始化PB10、PB11

    // USART1 初始化设置
    USART_InitStructure.USART_BaudRate = bound;                                     // 波特率设置
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                     // 字长为8位数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1;                          // 一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;                             // 无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                 // 收发模式
    USART_Init(USART3, &USART_InitStructure);                                       // 初始化串口3

    USART_Cmd(USART3, ENABLE); // 使能串口3

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE); // 开启相关中断

    // Usart1 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;         // 串口3中断通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; // 抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;        // 子优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);                           // 根据指定的参数初始化NVIC寄存器
}

void USART3_IRQHandler(void) // 串口3中断服务程序
{

    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        /* Read one byte from the receive data register */
        Rx3Data = USART_ReceiveData(USART3);

        Rx3Buffer[Rx3Counter++] = Rx3Data;

        if (Rx3Counter >= sizeof(Rx3Buffer))
        {
            Rx3Counter = 0;
            Rx3End = 1;
        }
    }

    USART_ClearITPendingBit(USART3, USART_IT_RXNE); // 清除中断标志
}

// 发送一个字节的函数
void USART3_SendByte(u8 Byte)
{
    USART_SendData(USART3, Byte);
    while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET)
        ;
}

// 发送字符串的函数
void USART3_SendString(char *str)
{
    // 循环发送字符
    while (*str != '\0')
    {
        USART_SendData(USART3, *str);
        while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET)
            ;
        str++;
    }
}

/*****************  发送指定长度的字节 **********************/
void USART_SendBytes(USART_TypeDef *pUSARTx, uint8_t *buf, uint32_t len)
{
    uint8_t *p = buf;

    while (len--)
    {
        USART_SendData(pUSARTx, *p);

        p++;

        // 等待数据发送成功
        while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET)
            ;
        USART_ClearFlag(pUSARTx, USART_FLAG_TXE);
    }
}
