/*************************************************
 Copyright ? 0010. All rights reserved.

// 文件名
@ File name: dht11.c -- dht11温湿度传感器驱动的源程序
// 作者、工号、版本及完成日期
@ Author:    ya
@ ID：       0011
@ Version:
@ Date:
// 用于详细说明此程序文件完成的主要功能，与其他模块
// 或函数的接口，输出值、取值范围、含义及参数间的控
// 制、顺序、独立或依赖等关系
@ Description:
		   dht11温湿度传感器驱动的源程序

@ Others:  // 其它内容的说明
@ History: // 修改历史记录列表，每条修改记录应包括修改日期、修改
|		   // 者及修改内容简述
|--@ 1. Date:
|--@ Author:
|--@ ID:
|--@ Modification:
|--@ 2. ...
*************************************************/
#include <stdbool.h>

#include "stm32f4xx.h"
#include "dht11.h"
#include "delay.h" // 包含了延时

// ==========================================
// ================================
// DHT11测试使用到的函数接口
#include "usart.h"
#include <stdio.h>
// ================================
// ==========================================

uint8_t dhtbuf[5] = {0};

/**
 * @brief   DHT11相应端口的初始化函数
 *
 * @param   无
 * @retval  无
 */
void DHT11_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	// 打开GPIOG外设的时钟
	DHT11_DATA_RCC_CMD(DHT11_DATA_RCC, ENABLE);

	// 配置相应的GPIO并初始化
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;	   // 输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	   // 推挽输出
	GPIO_InitStructure.GPIO_Pin = DHT11_DATA_PIN;	   // 与DHT11的DATA线相连
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // 输出速度
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(DHT11_DATA_PORT, &GPIO_InitStructure);

	// 默认拉高引脚电平
	DHT11_SET(1);
}

/**
 * @brief  将DHT11相应端口设置为输入模式
 *
 * @param  无
 * @retval 无
 */
void DHT11_SetInputMode(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	DHT11_DATA_RCC_CMD(DHT11_DATA_RCC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = DHT11_DATA_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(DHT11_DATA_PORT, &GPIO_InitStructure);
}

/**
 * @brief  将DHT11相应端口设置为输出模式
 *
 * @param  无
 * @retval 无
 */
void DHT11_SetOutputMode(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	DHT11_DATA_RCC_CMD(DHT11_DATA_RCC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = DHT11_DATA_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(DHT11_DATA_PORT, &GPIO_InitStructure);
}


/**
 * @brief  给DHT11发送开始信号
 * 
 * @param  无
 * @retval 无
*/
void DHT11_SendStart(void)
{
	// 无论上一次引脚是什么状态，都先配置成输出模式
	DHT11_SetOutputMode();

	// 确保引脚为空闲状态
	DHT11_SET(1);
	delay_us(5);

	// 拉低数据线20ms（要求大于18ms）
	DHT11_SET(0);
	delay_ms(20);

	// 拉高数据线30us（要求20~40us）
	DHT11_SET(1);
	delay_us(30);
}


/**
 * @brief  等待DHT11的应答信号（含超时判断）
 * 
 * @param  无
 * @retval true--DHT11响应了，false--DHT11在超时间隔内未作出响应
*/ 
bool DHT11_GetAck(void)
{
	int cnt = 0; // 用于超时的计数

	// 1.设置引脚为输入模式
	DHT11_SetInputMode();

	// 2.等待DHT11模块把引脚拉低  为了提高程序可靠性，应该人为使用超时处理  100us
	while (DHT11_READ == SET && cnt < 100)
	{
		delay_us(1);
		cnt++;
	}

	// 如果在超时间隔内都没有响应，说明超时了
	if (cnt >= 100)
		return false;

	cnt = 0;

	// 3.如果DHT11把引脚电平拉低，则需要分析是否拉低时间为80us左右
	while (DHT11_READ == RESET && cnt < 100)
	{
		delay_us(1);
		cnt++;
	}

	if (cnt >= 100)
		return false; // 说明未响应，原因是超时了
	return true; // 说明响应了
}


/** 
 * @brief  从DHT11获取一位数据（含超时判断）
 * 
 * @param  databit，要存放一位二进制数据的容器，在函数内，无论这个容器的值先前是多少，
 * 		只要不是错误退出，它的值就肯定是0或1
 * @retval true--DHT11响应了，false--DHT11在超时间隔内未作出响应
*/
bool DHT11_GetDataBit(u8 *databit)
{
	int cnt = 0; // 用于计数

	// 1.等待DHT11把引脚电平拉低
	while (DHT11_READ == SET && cnt < 100)
	{
		delay_us(1);
		cnt++;
	}

	// 如果在超时间隔内都没有响应，说明超时了
	if (cnt >= 100)
		return false; 

	cnt = 0;

	// 2.等待DHT11把引脚电平拉高
	while (DHT11_READ == RESET && cnt < 100)
	{
		delay_us(1);
		cnt++;
	}

	// 如果在超时间隔内都没有响应，说明超时了
	if (cnt >= 100)
		return false; 

	// 3.延时28us < us < 70us
	delay_us(40);

	*databit = DHT11_READ; // 返回读取到的一位数据

	return true;
}


/**
 * @brief  获取DHT11的温湿度数据
 * 
 * @param  buf 存放DHT11返回的数据，
 * 		由于DHT11返回的是5个字节的数据，这个容器要至少有5个字节的大小
 * @retval true--成功获取到了数据，false--DHT11超时未响应或是DHT11发送的数据有误
*/ 
bool DHT11_GetData(u8 buf[5])
{
	int i, j;
	u8 databit = 0; // 用来存放一位二进制数据
	uint8_t data = 0; // 用于存储1字节  8bit

	// 1.发送开始信号
	DHT11_SendStart();

	// 2.等待DHT11响应
	if (DHT11_GetAck())
	{
		// 3.循环读取40bit
		for (i = 0; i < 5; i++)
		{
			for (j = 0; j < 8; j++)
			{
				data <<= 1;
				DHT11_GetDataBit(&databit); // 这里没有进行错误处理，错误处理可以选择在调试阶段进行
				data |= databit;
			}

			buf[i] = data;
			data = 0;
		}

		// 4.进行校验
		if (buf[0] + buf[1] + buf[2] + buf[3] == buf[4])
		{
			return true;
		}
		else
			return false;
	}
	else
	{
		return false; // 获取数据失败，原因是未响应
	}
}

// DHT11测试函数，内部是个死循环
// 函数内部已经自己初始化了使用到的外设，包括DHT11本身
// 需要用到串口1和printf函数
void DHT11_Test(void)
{
	char buf[128] = {0};

	uint8_t dhtbuf[5] = {0};

	// 1.硬件初始化
	uart_init(9600);
	DHT11_Init();
	
	while (1)
	{
		DHT11_GetData(dhtbuf);
		// 在GB2312编码下，℃这个符号会乱码
		sprintf(buf, "温度 = %d ℃,湿度=%d %%RH \n\n", dhtbuf[2], dhtbuf[0]);
		printf("%s\r\n", buf);
		delay_ms(500);
		delay_ms(500);
	}
}
