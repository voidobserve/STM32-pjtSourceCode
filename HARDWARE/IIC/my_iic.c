#include <stdbool.h>
#include "my_iic.h"
#include "stm32f4xx.h"
#include "delay.h"

// 使用IO口来模拟IIC时序，从而控制从器件
#define IIC_SDA(n) ((n) ? GPIO_SetBits(GPIOB, GPIO_Pin_9) : GPIO_ResetBits(GPIOB, GPIO_Pin_9))
#define IIC_SCL(n) ((n) ? GPIO_SetBits(GPIOB, GPIO_Pin_8) : GPIO_ResetBits(GPIOB, GPIO_Pin_8))

#define IIC_READ (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9))

// IIC时序： 起始信号 + 发送字节 + 从机应答 + 读取字节 + 主机应答 + 停止信号

// IIC的初始化
void IIC_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;      // 输出模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // 输出速度
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     // 开漏输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;       // 上拉电阻

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; // 引脚编号
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; // 引脚编号
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 确保SDA和SCL都处于空闲状态
    IIC_SDA(1);
    IIC_SCL(1);
}

// SDA设置为输出模式
void IIC_SDASetOutputMode(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;      // 输出模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // 输出速度
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     // 开漏输出
    // GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;       // 上拉电阻

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; // 引脚编号
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

// SDA设置为输入模式
void IIC_SDASetInputMode(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; // 输入模式
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; // 引脚编号
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

// IIC的开始信号
void IIC_Start(void)
{
    // 1.SDA引脚设置为输出模式
    IIC_SDASetOutputMode();

    // 2.确保SDA和SCL为空闲状态--高电平
    IIC_SDA(1);
    IIC_SCL(1);
    delay_us(5);

    // 3.在SCL高电平期间，把SDA电平拉低
    IIC_SDA(0);
    delay_us(5);

    // 4.把SCL时钟线的电平拉低
    IIC_SCL(0);
    // delay_us(5);
}

// IIC发送字节
void IIC_SendByte(uint8_t data)
{
    uint8_t i = 0;

    // 1.SDA引脚设置为输出模式
    IIC_SDASetOutputMode();

    // 2.把SCL时钟线电平拉低
    IIC_SCL(0);
    delay_us(5);

    // 3.循环发送8bit数据
    for (i = 0; i < 8; i++)
    {
        // 4.分析待发送数据的最高位 MSB
        if (data & 0x80)
            IIC_SDA(1);
        else
            IIC_SDA(0);

        data <<= 1; // 数据左移1bit
        delay_us(5);

        // 5.把SCL时钟线的电平拉高，此时从机可以读取
        IIC_SCL(1);
        delay_us(5);

        // 6.把SCL时钟线的电平拉低，此时主机可以准备数据
        IIC_SCL(0);
        delay_us(5);
    }
}

// IIC的等待从器件应答
bool IIC_WaitSlaveAck(void)
{
    // 1.设置SDA数据线为输入模式
    IIC_SDASetInputMode();

    // 2.把SCL时钟线的电平拉低，此时从机可以准备数据
    IIC_SCL(0);
    delay_us(5);

    // 3.把SCL时钟线的电平拉高，此时主机可以读取数据
    IIC_SCL(1);
    delay_us(5);

    // 4.主器件读取SDA数据线的电平状态
    if (IIC_READ)
        return false;
    else
        return true;
}

// IIC读取字节
uint8_t IIC_ReadByte(void)
{
    uint8_t data = 0;
    uint8_t i = 0;

    // 1.设置SDA数据线为输入模式
    IIC_SDASetInputMode();

    // 2.把SCL时钟线的电平拉低，此时从机可以准备数据
    IIC_SCL(0);
    delay_us(5);

    // 3.循环读取8bit数据并存储在变量
    for (i = 0; i < 8; i++)
    {
        // 4.把SCL时钟线的电平拉高，此时主机可以读取数据
        IIC_SCL(1);
        delay_us(5);

        // 5.读取SDA数据线的电平状态  MSB
        data <<= 1;
        data |= IIC_READ;

        // 6.把SCL时钟线的电平拉低，此时从机可以准备数据
        IIC_SCL(0);
        delay_us(5);
    }

    return data;
}

// IIC主机发送应答
// 在IIC协议中：
// ack == 1，表示不应答
// ack == 0，表示应答
void IIC_MasterSendAck(uint8_t ack)
{
    // 1.SDA引脚设置为输出模式
    IIC_SDASetOutputMode();

    // 2.把SCL时钟线的电平拉低，此时主机可以准备数据
    IIC_SCL(0);
    delay_us(5);

    // 3.主机进行应答
    if (ack)
        IIC_SDA(1); // 表示不应答
    else
        IIC_SDA(0); // 表示应答了

    delay_us(5);

    // 4.把SCL时钟线的电平拉高，此时从机可以读取
    IIC_SCL(1);
    delay_us(5);
}

// IIC发送停止信号
void IIC_Stop(void)
{
    // 1.SDA引脚设置为输出模式
    IIC_SDASetOutputMode();

    // 2.确保SDA和SCL都输出低电平
    IIC_SCL(0);
    IIC_SDA(0);
    delay_us(5);

    // 3.把SCL时钟线的电平拉高
    IIC_SCL(1);
    delay_us(5);

    // 4.把SDA数据线的电平拉高
    IIC_SDA(1);
    delay_us(5);
}
