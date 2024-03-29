#ifndef __INTERFACE_H
#define __INTERFACE_H

#include "stm32f4xx.h"

// #define CUR_MENU
// #define CUR_FUNCTION_SHOW_HUMITURE
// #define CUR_FUNCTION_SHOW_HCHO
// #define CUR_FUNCTION_SHOW_PM2_5
// #define CUR_FUNCTION_SHOW_FAN_SPEED

enum cur_page
{
    CUR_MENU = 0,
    CUR_FUNCTION_SHOW_HUMITURE,
    CUR_FUNCTION_SHOW_HCHO,
    CUR_FUNCTION_SHOW_PM2_5,
    CUR_FUNCTION_SHOW_FAN_SPEED
};

typedef enum cur_page cur_index_t;

extern cur_index_t index;

void Show_Humiture(void);

// 根据目录索引下标来切换要显示的界面
void OLED_Show_Interface(void);

#endif
