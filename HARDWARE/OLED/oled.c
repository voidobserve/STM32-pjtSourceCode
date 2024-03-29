/*************************************************
 Copyright ? 0010. All rights reserved.

// 文件名
@ File name: oled.c
// 作者、工号、版本及完成日期
@ Author:    ya
@ ID：       0011
@ Version:
@ Date:
@ Description:
	// 用于详细说明此程序文件完成的主要功能，与其他模块
	// 或函数的接口，输出值、取值范围、含义及参数间的控
	// 制、顺序、独立或依赖等关系
	----------------------------------------------------------------
		GND   电源地
		VCC   接5V或3.3v电源
		SCL   接PD6（SCL）
		SDA   接PB7（SDA）
	----------------------------------------------------------------
@ Others:  // 其它内容的说明
@ History: // 修改历史记录列表，每条修改记录应包括修改日期、修改
|		   // 者及修改内容简述
|--@ 1. Date:
|--@ Author:
|--@ ID:
|--@ Modification:
|--@ 2. ...
*************************************************/
#include "oled.h"
#include <stdlib.h>
#include <string.h>
#include "oledfont.h"
#include "delay.h"

#ifndef OLED_PIXEL_X
#define OLED_PIXEL_X 128 // OLED横向分辨率
#endif
#ifndef OLED_PIXEL_Y
#define OLED_PIXEL_Y 64 // OLED纵向分辨率
#endif

#ifndef OLED_PAGE_SIZE
#define OLED_PAGE_SIZE (OLED_PIXEL_Y / 8) // OLED分页
#endif

#ifndef BRIGHTNESS
#define BRIGHTNESS 0xFF // 要填充到OLED显存中的数据（全白或全黑）
#endif
#ifndef OLED_Y_BASE
#define OLED_Y_BASE 0xB0 // OLED页地址基准
#endif
#ifndef OLED_X_BASE_L
#define OLED_X_BASE_L 0x00 // OLEDx轴像素的基准（低位）
#endif
#ifndef OLED_X_BASE_H
#define OLED_X_BASE_H 0x10 // OLEDx轴像素的基准（高位）
#endif

// 要在OLED上显示的字符的大小（单位：像素）
#ifndef OLED_FONT_WIDTH
#define OLED_FONT_WIDTH 8
#endif
#ifndef OLED_FONT_HEIGHT
#define OLED_FONT_HEIGHT 16
#endif

#define OLED_CH_FONT_HEIGHT 16
#define OLED_CH_FONE_WIDTH 16

uint8_t OLED_Graphic_MEM[OLED_PIXEL_X][OLED_PAGE_SIZE]; // OLED显存

// OLED的显存
// 存放格式如下.
//[0]0 1 2 3 ... 127
//[1]0 1 2 3 ... 127
//[2]0 1 2 3 ... 127
//[3]0 1 2 3 ... 127
//[4]0 1 2 3 ... 127
//[5]0 1 2 3 ... 127
//[6]0 1 2 3 ... 127
//[7]0 1 2 3 ... 127
/**********************************************
//IIC Start
**********************************************/

// SDA设置为输出模式
static void IIC_SDASetOutputMode(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	OLED_SDA_RCC_CMD(OLED_SDA_RCC, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;	   // 输出模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // 输出速度
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	   // 推挽复用
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	   // 上拉电阻

	GPIO_InitStructure.GPIO_Pin = OLED_SDA_PIN; // 引脚编号
	GPIO_Init(OLED_SDA_PORT, &GPIO_InitStructure);
}

// SDA设置为输入模式
static void IIC_SDASetInputMode(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	OLED_SDA_RCC_CMD(OLED_SDA_RCC, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; // 输出模式
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; // 上拉电阻

	GPIO_InitStructure.GPIO_Pin = OLED_SDA_PIN; // 引脚编号
	GPIO_Init(OLED_SDA_PORT, &GPIO_InitStructure);
}

static void IIC_Start(void)
{
	// 1.SDA引脚设置为输出模式
	IIC_SDASetOutputMode();

	// 2.确保SDA和SCL为空闲状态--高电平
	IIC_SDA(1); // 把数据线拉高
	IIC_SCL(1); // 把时钟线拉高
	delay_us(4);

	// 3.在SCL高电平期间，把SDA电平拉低
	IIC_SDA(0);
	delay_us(4);

	// 4.把SCL时钟线的电平拉低
	IIC_SCL(0);
	delay_us(4);
}

/**********************************************
//IIC Stop
**********************************************/
static void IIC_Stop(void)
{
	// 1.SDA引脚设置为输出模式
	IIC_SDASetOutputMode();

	// 2.确保SDA和SCL都输出低电平
	IIC_SCL(0);
	IIC_SDA(0);
	delay_us(4);

	// 3.把SCL时钟线的电平拉高
	IIC_SCL(1);
	delay_us(4);

	// 4.把SDA数据线的电平拉高
	IIC_SDA(1);
	delay_us(4);
}

static bool IIC_Wait_Ack(void)
{
	// 定义变量存储应答状态
	bool status = false;

	// 1.设置SDA数据线为输入模式
	IIC_SDASetInputMode();

	// 2.把SCL时钟线的电平拉低，此时从机可以准备数据
	// 这一步可以省略，因为该函数的前一步是IIC发送一个字节的函数，
	// 在那个函数的最后已经把时钟线拉低了，并且做了延时
	// IIC_SCL(0);
	// delay_us(4);

	// 3.把SCL时钟线的电平拉高，此时主机可以读取应答数据
	IIC_SCL(1);
	delay_us(4);

	// 4.主器件读取SDA数据线的电平状态
	// 0--表示应答ACK，1--表示不应答NACK
	if (IIC_READ)
		status = false;
	else
		status = true;

	IIC_SCL(0);
	delay_us(4);
	return status;
}
/**********************************************
// IIC Write byte
**********************************************/

static void Write_IIC_Byte(unsigned char data)
{
	uint8_t i = 0;

	// 1.SDA引脚设置为输出模式
	IIC_SDASetOutputMode();

	// 2.把SCL时钟线电平拉低
	IIC_SCL(0);
	delay_us(4);

	// 3.循环发送8bit数据
	for (i = 0; i < 8; i++)
	{
		// 4.分析待发送数据的最高位 MSB
		if (data & 0x80)
			IIC_SDA(1);
		else
			IIC_SDA(0);

		data <<= 1; // 数据左移1bit
		delay_us(4);

		// 5.把SCL时钟线的电平拉高，此时从机可以读取
		IIC_SCL(1);
		delay_us(4);

		// 6.把SCL时钟线的电平拉低，此时主机可以准备数据
		// 在发送第8个bit后，把时钟线拉低，结束第8bit的时钟脉冲
		IIC_SCL(0);
		delay_us(4);
	}
}
/**********************************************
// IIC Write Command
**********************************************/
static void Write_IIC_Command(unsigned char IIC_Command)
{
	IIC_Start();
	Write_IIC_Byte(0x78); // Slave address,SA0=0
	IIC_Wait_Ack();
	Write_IIC_Byte(0x00); // write command
	IIC_Wait_Ack();
	Write_IIC_Byte(IIC_Command);
	IIC_Wait_Ack();
	IIC_Stop();
}
/**********************************************
// IIC Write Data
**********************************************/
static void Write_IIC_Data(unsigned char IIC_Data)
{
	IIC_Start();
	Write_IIC_Byte(0x78); // D/C#=0; R/W#=0
	IIC_Wait_Ack();
	Write_IIC_Byte(0x40); // write data
	IIC_Wait_Ack();
	Write_IIC_Byte(IIC_Data);
	IIC_Wait_Ack();
	IIC_Stop();
}

static void OLED_WR_Byte(unsigned dat, unsigned cmd)
{
	if (cmd)
	{

		Write_IIC_Data(dat);
	}
	else
	{
		Write_IIC_Command(dat);
	}
}

/********************************************
// fill_Picture
********************************************/
void fill_picture(unsigned char fill_Data)
{
	unsigned char m, n;
	for (m = 0; m < 8; m++)
	{
		OLED_WR_Byte(0xb0 + m, 0); // page0-page1
		OLED_WR_Byte(0x00, 0);	   // low column start address
		OLED_WR_Byte(0x10, 0);	   // high column start address
		for (n = 0; n < 128; n++)
		{
			OLED_WR_Byte(fill_Data, 1);
		}
	}
}

/**
 * @brief OLED定位函数
 *
 * @param x OLED横坐标 0 ~ OLED_PIXEL_X
 * @param y OLED页     0 ~ OLED_PAGE_SIZE
 */
void OLED_Set_Pos(unsigned char x, unsigned char y)
{
	x += OLED_X_BASE_L; // 要加上OLEDx轴像素的基准（低位），有进位就顺便进位
	// 写入页地址
	OLED_WR_Byte(OLED_Y_BASE + y, OLED_CMD);
	// 写入8位起始列地址的低4位
	OLED_WR_Byte(x & 0x0f, OLED_CMD);
	// 写入8位起始列地址的高4位（要加上OLEDx轴像素的基准（高位））
	OLED_WR_Byte((x & 0xf0) >> 4 | OLED_X_BASE_H, OLED_CMD);
}

/**
 * @brief OLED画点函数
 *
 * @param x 横坐标 0 ~ OLED_PIXEL_X
 * @param y 纵坐标 0 ~ OLED_PIXEL_Y
 * @param mode 是否反白显示
 */
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t mode)
{
	uint8_t pos; // 存放要画点的坐标对应的OLED页的位置

	if (x >= OLED_PIXEL_X || y >= OLED_PIXEL_Y)
	{
		// 如果要画点的坐标超出了屏幕
		return;
	}

	pos = y / 8;

	if (mode)
	{
		// 如果不是反白显示
		OLED_Graphic_MEM[x][pos] |= 1 << (y % 8);
	}
	else
	{
		OLED_Graphic_MEM[x][pos] &= (~(1 << (y % 8)));
	}
}

/**
 * @brief OLED刷新（将显存中的数据写入到屏幕中）
 */
void OLED_RefreshGraphic(void)
{
	uint8_t i, j;

	for (i = 0; i < OLED_PAGE_SIZE; i++)
	{
		// 定位
		OLED_Set_Pos(0, i);
		for (j = 0; j < OLED_PIXEL_X; j++)
		{
			// 填入数据
			OLED_WR_Byte(OLED_Graphic_MEM[j][i], OLED_DATA);
		}
	}
}

/**
 * @brief  开启显示.
 * @param  None
 * @retval None
 */
void OLED_Display_On(void)
{
	OLED_WR_Byte(0X8D, OLED_CMD); // SET DCDC命令
	OLED_WR_Byte(0X14, OLED_CMD); // DCDC ON
	OLED_WR_Byte(0XAF, OLED_CMD); // DISPLAY ON
}
/**
 * @brief  关闭显示.
 * @param  None
 * @retval None
 */
void OLED_Display_Off(void)
{
	OLED_WR_Byte(0X8D, OLED_CMD); // SET DCDC命令
	OLED_WR_Byte(0X10, OLED_CMD); // DCDC OFF
	OLED_WR_Byte(0XAE, OLED_CMD); // DISPLAY OFF
}

// 清屏函数,清完屏,整个屏幕是全白的
void OLED_Clear(void)
{
	uint8_t i, j;

	for (i = 0; i < OLED_PAGE_SIZE; i++)
	{
		for (j = 0; j < OLED_PIXEL_X; j++)
		{
			OLED_Graphic_MEM[j][i] = 0x00; // 黑屏，什么都不显示
		}
	}

	// 刷新OLED
	OLED_RefreshGraphic();
}

// 清空显存
void OLED_Clear_Graphic_MEM(void)
{
	memset(OLED_Graphic_MEM, 0, sizeof(OLED_Graphic_MEM) / sizeof(OLED_Graphic_MEM[0][0]));
}

/**
 * @brief OLED显示字符
 *
 * @param x x轴起始位置，0~127
 * @param y y轴起始位置，0~63
 * @param ch 要显示的字符
 * @param mode 是否要反白显示
 *             1--不反白显示
 *             0--反白显示
 */
void OLED_ShowChar(u8 x, u8 y, u8 ch, u8 mode)
{
	uint8_t i;				   // 循环计数值
	uint8_t j;				   // 循环计数值
	uint8_t y0 = y;			   // y0用来记录当前要画的y坐标，控制画点的列，使每一列的y轴起始位置都对齐
	uint8_t locate = ch - ' '; // 存放字符在字库中的位置

	// 字体的大小（所占多少个字节）
	// 通过三目运算符判断字体是跨 奇数个页 还是 偶数个页
	uint8_t size = OLED_FONT_WIDTH * ((OLED_FONT_HEIGHT % 8) ? (OLED_FONT_HEIGHT / 8 + 1) : (OLED_FONT_HEIGHT / 8));

	uint8_t temp; // 临时变量，存放要写入的字符

	for (i = 0; i < size; i++)
	{
		temp = ascii_0816[locate][i];

		for (j = 0; j < 8; j++)
		{
			// 通过循环，每次画 当前temp最低位的点

			if (temp & 0x01)
			{
				// 如果temp当前最低位是1，画点
				OLED_DrawPoint(x, y, mode);
			}
			else
			{
				// 如果temp当前最低位是0，不画点，向显存写入0
				OLED_DrawPoint(x, y, !mode);
			}

			temp >>= 1; // temp的次低位成为最低位，准备画点
			y++;		// 列，也就是纵坐标，加一

			if (y - y0 >= OLED_FONT_HEIGHT)
			{
				// 如果画完了一列，就要换下一列了
				y = y0;
				x++; // x轴加一
				break;
			}
		}
	}
}

/**
 * @brief OLED显示字符串（如果超出x轴会换行显示）
 *
 * @param x x轴起始位置
 * @param y y轴起始位置
 * @param str 要显示的字符串
 * @param mode 是否要反白显示
 *             1，不反白显示
 *             0，反白显示
 */
void OLED_ShowStr(uint8_t x, uint8_t y, char *str, uint8_t mode)
{
	// 判断字符是否符合ASCII的范围
	while ((*str) >= ' ' && (*str) <= '~')
	{
		if (x > OLED_PIXEL_X - OLED_FONT_WIDTH)
		{
			// 如果要显示的字符超出了屏幕的横向范围，换行显示
			x = 0;
			y += OLED_FONT_HEIGHT;
		}

		if (y > OLED_PIXEL_Y - OLED_FONT_HEIGHT)
		{
			// 如果要显示的字符超出了屏幕的纵向范围
			// 做清屏操作，从（0，0）坐标继续显示
			OLED_Clear();
			x = y = 0;
		}

		OLED_ShowChar(x, y, *str++, mode);
		x += OLED_FONT_WIDTH;
	}
}

/**
 * @brief 求一个数的多次方（n的m次方）
 *
 * @param n 底数
 * @param m 幂
 * @retval  返回这个底数的m次方
 */
static uint64_t oled_pow(uint32_t n, uint32_t m)
{
	uint64_t ret = 1;
	while (m--)
	{
		ret *= n;
	}

	return ret;
}

/**
 * @brief OLED显示数字（不考虑负数）
 *
 * @param x x轴起始位置
 * @param y y轴起始位置
 * @param num 要显示的数字
 * @param len 数字的长度
 * @param mode 是否要反白显示
 *             1，不反白显示
 *             0，反白显示
 */
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t mode)
{
	uint8_t i;			// 循环计数值
	uint8_t temp;		// 临时变量
	uint8_t enshow = 0; // 有效数字标志位，1：是有效数字，0，不是有效数字

	// 通过循环将每一个数位显示出来
	for (i = 0; i < len; i++)
	{
		// 通过计算得到当前最高位数的值
		temp = num / oled_pow(10, len - 1 - i) % 10;

		if (temp == 0)
		{
			if (enshow == 0)
			{
				// 如果最高位是0，并且不是这个数的有效数字
				OLED_ShowChar(x, y, ' ', mode);
			}
			else
			{
				// 如果当前位是0，并且是这个数的有效数字
				OLED_ShowChar(x, y, '0', mode);
			}
		}
		else
		{
			// 如果遇到非零的数位
			enshow = 1;
			OLED_ShowChar(x, y, '0' + temp, mode);
		}

		x += OLED_FONT_WIDTH;
		if (x > OLED_PIXEL_X - OLED_FONT_WIDTH)
		{
			// 如果要显示的字符超出了屏幕的横向范围，换行显示
			x = 0;
			y += OLED_FONT_HEIGHT;
		}
		if (y > OLED_PIXEL_Y - OLED_FONT_HEIGHT)
		{
			// 如果要显示的字符超出了屏幕的纵向范围
			// 做清屏操作，从（0，0）坐标继续显示
			OLED_Clear();
			x = y = 0;
		}
	}
}

/**
 * @brief OLED显示汉字
 *
 * @param x x轴起始位置
 * @param y y轴起始位置
 * @param pHZ 要显示的汉字
 * @param mode 是否要反白显示
 *             1，不反白显示
 *             0，反白显示
 */
void OLED_ShowChinese(uint8_t x, uint8_t y, char *ChineseStr, uint8_t mode)
{
	uint8_t i; // 循环计数值
	uint8_t j; // 循环计数值
	uint8_t k;
	uint8_t temp; // 临时变量
	uint8_t y0 = y;

	// 计算出汉字字库中数组元素的个数
	uint8_t len = sizeof(Chinese) / sizeof(Chinese[0]);

	// 字体的大小（所占多少个字节）
	// 通过三目运算符判断字体是跨 奇数个页 还是 偶数个页
	uint8_t size = OLED_CH_FONE_WIDTH * ((OLED_CH_FONT_HEIGHT % 8) ? (OLED_CH_FONT_HEIGHT / 8 + 1) : (OLED_CH_FONT_HEIGHT / 8));

	while (*ChineseStr != '\0')
	{
		// 当前要显示的汉字不为'\0'时，继续循环

		for (i = 0; i < len; i++)
		{
			if (ChineseStr[0] == Chinese[i].Index[0] && ChineseStr[1] == Chinese[i].Index[1])
			{
				// 如果在字库中找到了该汉字

				// 判断当前汉字有没有超出屏幕显示
				if (x > OLED_PIXEL_X - OLED_CH_FONE_WIDTH)
				{
					// 如果要显示的字符超出了屏幕的横向范围，换行显示
					x = 0;
					y += OLED_CH_FONT_HEIGHT;
					y0 = y;
				}
				if (y > OLED_PIXEL_Y - OLED_CH_FONT_HEIGHT)
				{
					// 如果要显示的字符超出了屏幕的纵向范围
					// 做清屏操作，从（0，0）坐标继续显示
					OLED_Clear();
					x = y = 0;
				}

				for (j = 0; j < size; j++)
				{
					temp = Chinese[i].Msk[j];

					for (k = 0; k < 8; k++)
					{
						if (temp & 0x01)
						{
							// 如果temp当前最低位是1，画点
							OLED_DrawPoint(x, y, mode);
						}
						else
						{
							// 如果temp当前最低位是0，不画点，向显存写入0
							OLED_DrawPoint(x, y, !mode);
						}

						temp >>= 1; // temp的次低位成为最低位，准备画点
						y++;		// 列，也就是纵坐标，加一

						if (y - y0 >= OLED_CH_FONT_HEIGHT)
						{
							// 如果画完了一列，就要换下一列了
							y = y0;
							x++; // x轴加一
							break;
						}
					}
				}

				break; // 如果在字库中找不到汉字
			}
		}

		ChineseStr += 2;
	}
}
/***********功能描述：显示显示BMP图片128×64起始点坐标(x,y),x的范围0～127，y为页的范围0～7*****************/
void OLED_DrawBMP(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char BMP[])
{
	unsigned int j = 0;
	unsigned char x, y;

	if (y1 % 8 == 0)
		y = y1 / 8;
	else
		y = y1 / 8 + 1;
	for (y = y0; y < y1; y++)
	{
		OLED_Set_Pos(x0, y);
		for (x = x0; x < x1; x++)
		{
			OLED_WR_Byte(BMP[j++], OLED_DATA);
		}
	}
}

// 图片要用 阴码 + 逐列式 + 逆向 + 十六进制 + C51格式取模生成
void OLED_ShowBMP(const uint8_t x, const uint8_t y, uint8_t width, uint8_t height, const uint8_t BMP[])
{
	uint8_t i;			// 循环计数值
	uint8_t j;			// 循环计数值
	uint8_t k;			// 循环计数值
	uint8_t cur_x = x;	// cur_x用来记录当前要画的x坐标，控制画点的行，使每一行的x轴起始位置都对齐
	uint8_t cur_y = y;	// cur_y用来记录当前要画的y坐标，控制画点的列，使每一列的y轴起始位置都对齐
	uint8_t locate = 0; // 记录当前读取到图片的第几个字节
	uint8_t temp;		// 存放一次读取图片的一个字节数据的变量

	for (i = 0; i < width; i++)
	{
		for (j = 0; j < height; j++)
		{
			temp = BMP[locate++]; // 一次读取图片的一个字节数据
			for (k = 0; k < 8; k++)
			{
				// 在循环内，向显存写入一个字节的数据
				// 通过循环，每次画 当前temp最低位的点
				if (temp & 0x01)
				{
					// 如果temp当前最低位是1，画点
					OLED_DrawPoint(cur_x, cur_y, 1);
				}
				else
				{
					// 如果temp当前最低位是0，不画点，向显存写入0
					OLED_DrawPoint(cur_x, cur_y, 0);
				}

				temp >>= 1; // temp的次低位成为最低位，准备画点
				cur_y++;	// 列，也就是纵坐标，加一
			}

			if (cur_y - y >= height)
			{
				// 如果画完了一列，就要换下一列了
				cur_y = y;
				cur_x++; // x轴加一
				break;
			}
		}

		if (cur_x - x >= width)
		{
			// 如果当前的x轴坐标超出了图片宽度，说明图片的数据已经全部写入到显存了
			break;
		}
	}
}

// 初始化SSD1306
void OLED_Init(void)
{
	// 配置好OLED的引脚

	// 引脚配置
	GPIO_InitTypeDef GPIO_InitStructure;

	OLED_SCL_RCC_CMD(OLED_SCL_RCC, ENABLE);
	OLED_SDA_RCC_CMD(OLED_SDA_RCC, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;	   // 输出模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // 输出速度
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	   // 推挽复用
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	   // 上拉电阻

	GPIO_InitStructure.GPIO_Pin = OLED_SCL_PIN; // 引脚编号
	GPIO_Init(OLED_SCL_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = OLED_SDA_PIN; // 引脚编号
	GPIO_Init(OLED_SDA_PORT, &GPIO_InitStructure);

	delay_ms(200);

	// SSD1306初始化序列

	OLED_WR_Byte(0xAE, OLED_CMD); //--display off 						 关闭显示(0xAE/0xAF 关闭/开启)
	OLED_WR_Byte(0x00, OLED_CMD); //---set low column address 			 默认为 0
	OLED_WR_Byte(0X02, OLED_CMD); //									 [1:0]00:列地址模式;01:行地址;10:页地址模式;
	OLED_WR_Byte(0x10, OLED_CMD); //---set high column address
	OLED_WR_Byte(0X20, OLED_CMD); // 设置内存地址模式
	OLED_WR_Byte(0x40, OLED_CMD); //--set start line address			 设置显示开始行[5:0]
	OLED_WR_Byte(0xB0, OLED_CMD); //--set page address
	OLED_WR_Byte(0x81, OLED_CMD); // contract control					 设置对比度
	OLED_WR_Byte(0xFF, OLED_CMD); //--128
	OLED_WR_Byte(0xA1, OLED_CMD); // set segment remap					 设置左右反置 (0xa0/0xa1 开/关)
	OLED_WR_Byte(0XA4, OLED_CMD); // 全局显示开启;bit0:1,开启;0,关闭;(白屏/黑屏)
	OLED_WR_Byte(0xA6, OLED_CMD); //--normal / reverse					 设置正常显示(0xa6/0xa7  正常/不正常)
	OLED_WR_Byte(0xA8, OLED_CMD); //--set multiplex ratio(1 to 64) 		 设置驱动路数(1~64)
	OLED_WR_Byte(0x3F, OLED_CMD); //--1/32 duty 						 默认 0X3F	(1~64)
	OLED_WR_Byte(0xC8, OLED_CMD); // Com scan direction					 设置扫描方向 (0xc0/0xc8 上下翻转/正常)
	OLED_WR_Byte(0xD3, OLED_CMD); //-set display offset					 设置显示偏移
	OLED_WR_Byte(0x00, OLED_CMD); //

	OLED_WR_Byte(0xD5, OLED_CMD); // set osc division 					 设置时钟分频因子,震荡频率
	OLED_WR_Byte(0x80, OLED_CMD); // [3:0],								 分频因子;[7:4],震荡频率

	OLED_WR_Byte(0xD8, OLED_CMD); // set area color mode off
	OLED_WR_Byte(0x05, OLED_CMD); //

	OLED_WR_Byte(0xD9, OLED_CMD); // Set Pre-Charge Period				 设置预充电周期
	OLED_WR_Byte(0xF1, OLED_CMD); //									 [3:0],PHASE 1;[7:4],PHASE 2;

	OLED_WR_Byte(0xDA, OLED_CMD); // set com pin configuartion			 设置硬件引脚配置
	OLED_WR_Byte(0x12, OLED_CMD); //									 [5:4]配置

	OLED_WR_Byte(0xDB, OLED_CMD); // set Vcomh							 设置VCOMH电压倍率
	OLED_WR_Byte(0x30, OLED_CMD); // [6:4] 000,0.65*vcc;001,0.77*vcc;011,0.83*vcc;

	OLED_WR_Byte(0x8D, OLED_CMD); // set charge pump enable				 电荷泵设置
	OLED_WR_Byte(0x14, OLED_CMD); // 									 开启/关闭 0x14/0x10

	OLED_WR_Byte(0XEF, OLED_CMD); // 亮度调节0x00~0xFF (亮度设置,越大越亮)

	OLED_WR_Byte(0xAF, OLED_CMD); //--turn on oled panel				 开启显示

	OLED_Clear(); // 清屏

	// 定位到坐标为 0，0 的地方，方便下次写入
	OLED_Set_Pos(0, 0);
}
