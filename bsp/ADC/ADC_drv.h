#ifndef _ADC_DRV_H_
#define _ADC_DRV_H_

#include "stm32f10x.h"
#include "delay.h"


extern void ADC_InitConfig(void);
extern u16 Get_Adc(void); 
extern u16 Count_AdcVal(u8 ch,u8 times);
extern uint16_t GET_RG_Value(void);


#endif
