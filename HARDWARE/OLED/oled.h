/*************************************************
 Copyright ? 0010. All rights reserved.

// 文件名
@ File name: oled.h
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
#ifndef __OLED_H
#define __OLED_H
// #include "sys.h"
#include "stm32f4xx.h"
#include <stdlib.h>
#include <stdbool.h>

#include "my_config.h" // 自定义的配置文件

#define OLED_MODE       0
#define SIZE            8
#define XLevelL         0x00
#define XLevelH         0x10
#define Max_Column      128
#define Max_Row         64
#define Brightness      0xFF
#define X_WIDTH         128
#define Y_WIDTH         64



//-----------------OLED IIC端口定义----------------
// =================================================================
// ====================================================
// OLED SCL 端口定义
#ifndef OLED_SCL_PORT 
#define OLED_SCL_PORT           GPIOD
#endif // end OLED_SCL_PORT
// OLED SCL 引脚号定义
#ifndef OLED_SCL_PIN
#define OLED_SCL_PIN            GPIO_Pin_6
#endif // end OLED_SCL_PIN
// OLED SCL GPIO时钟使能函数 定义
#ifndef OLED_SCL_RCC_CMD 
#define OLED_SCL_RCC_CMD        RCC_AHB1PeriphClockCmd
#endif // end OLED_SCL_RCC_CMD
// OLED SCL 填入GPIO时钟使能函数的参数 定义
#ifndef OLED_SCL_RCC 
#define OLED_SCL_RCC            RCC_AHB1Periph_GPIOD
#endif // end OLED_SCL_RCC

// OLED SDA 端口定义
#ifndef OLED_SDA_PORT 
#define OLED_SDA_PORT           GPIOB
#endif // end OLED_SDA_PORT
// OLED SDA 引脚号定义
#ifndef OLED_SDA_PIN
#define OLED_SDA_PIN            GPIO_Pin_7
#endif // end OLED_SDA_PIN
// OLED SDA GPIO时钟使能函数 定义
#ifndef OLED_SDA_RCC_CMD 
#define OLED_SDA_RCC_CMD        RCC_AHB1PeriphClockCmd
#endif // end OLED_SDA_RCC_CMD
// OLED SDA 填入GPIO时钟使能函数的参数 定义
#ifndef OLED_SDA_RCC 
#define OLED_SDA_RCC            RCC_AHB1Periph_GPIOB
#endif // end OLED_SDA_RCC

// ====================================================
// =================================================================


// 使用IO口来模拟IIC时序，从而控制从器件
#define IIC_SDA(n)      ((n) ? GPIO_SetBits(OLED_SDA_PORT, OLED_SDA_PIN) : GPIO_ResetBits(OLED_SDA_PORT, OLED_SDA_PIN))
#define IIC_SCL(n)      ((n) ? GPIO_SetBits(OLED_SCL_PORT, OLED_SCL_PIN) : GPIO_ResetBits(OLED_SCL_PORT, OLED_SCL_PIN))

#define IIC_READ        (GPIO_ReadInputDataBit(OLED_SDA_PORT, OLED_SDA_PIN)) 

#define OLED_CMD        0  // 写命令
#define OLED_DATA       1  // 写数据

#ifndef OLED_PIXEL_X
#define OLED_PIXEL_X 128 // OLED横向分辨率
#endif
#ifndef OLED_PIXEL_Y
#define OLED_PIXEL_Y 64 // OLED纵向分辨率
#endif
#ifndef OLED_PAGE_SIZE
#define OLED_PAGE_SIZE (OLED_PIXEL_Y / 8) // OLED分页
#endif
extern uint8_t OLED_Graphic_MEM[OLED_PIXEL_X][OLED_PAGE_SIZE]; // OLED显存

// OLED控制用函数
void OLED_Display_On(void);
void OLED_Display_Off(void);
void OLED_Init(void);
void OLED_Clear(void);
void OLED_Clear_Graphic_MEM(void); // 清空显存
void OLED_RefreshGraphic(void);

void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t mode);

void OLED_ShowChar(u8 x, u8 y, u8 ch, u8 mode);
void OLED_ShowStr(uint8_t x, uint8_t y, char *str, uint8_t mode);
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t mode);
void OLED_ShowChinese(uint8_t x, uint8_t y, char *ChineseStr, uint8_t mode);

void OLED_ShowBMP(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t BMP[]);

void OLED_Set_Pos(unsigned char x, unsigned char y);
void OLED_DrawBMP(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char BMP[]);

void fill_picture(unsigned char fill_Data);
void Picture(void);


#endif
