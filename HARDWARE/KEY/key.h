#ifndef __KEY__H
#define __KEY__H

#include "sys.h"

extern volatile uint8_t key;

// 注意，下面的两种方式只能同时使用一种，否则会出现重复定义的警告

/*下面的方式是通过直接操作库函数方式读取IO*/
#define KEY0    GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) // PA0
#define KEY1    GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_2)  // PE2
#define KEY2    GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3)  // PE3
#define KEY3    GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4)  // PE4


/*下面的方式是通过位带操作方式读取IO*/

// #define KEY0		PEin(4)	//PE4
// #define KEY1		PEin(3)	//PE3
// #define KEY2		PEin(2)	//PE2
// #define WK_UP		PAin(0)	//PA0

/*下面是将按键转化为对应的键值*/
#define KEY0_PRES 1
#define KEY1_PRES 2
#define KEY2_PRES 3
#define KEY3_PRES 4

void KEY_Init(void); // 键盘对应的IO口初始化
void KEY_IT_Init(void); // 按键初始化，并且打开按键对应的中断
u8 KEY_Scan(u8);     // 按键扫描函数

#endif
