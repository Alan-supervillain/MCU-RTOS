#ifndef __LCD_H
#define __LCD_H		
#include "sys.h"	 
#include "stdlib.h" 
#include <stdbool.h>



void LCD_Init(void);													   	//初始化													//关显示
void LCD_Clear(u16 Color);	 												//清屏
void oled_flush(void);
void OLED_WR_Byte(unsigned dat,unsigned cmd);





#endif  
	 
	 



