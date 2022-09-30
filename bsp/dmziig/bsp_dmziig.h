#ifndef   DMZIIG_H
#define	  DMZIIG_H

#include "stm32f10x.h"

/*���ӳ�˵��*/                          //---------------------------------------------------------
//У׼����
//��Ϊ��ͬ�Ĵ������������߲��Ǻ�һ�£���ˣ�ÿһ����������Ҫ�������������������ʹ����ֵ��׼ȷ��
//�����ֲ��Գ���������ƫ��ʱ�����Ӹ���ֵ��
//������Գ���������ƫСʱ����С����ֵ��
//��ֵ����ΪС��
#define GapValue 400


#define HX711_DOUT_GPIO_PORT    				GPIOC			              /* GPIO�˿� */
#define HX711_DOUT_GPIO_CLK 	    			RCC_APB2Periph_GPIOC		/* GPIO�˿�ʱ�� */
#define HX711_DOUT_GPIO_PIN							GPIO_Pin_8			       

#define HX711_SCK_GPIO_PORT    					GPIOC			              /* GPIO�˿� */
#define HX711_SCK_GPIO_CLK 	    				RCC_APB2Periph_GPIOC		/* GPIO�˿�ʱ�� */
#define HX711_SCK_GPIO_PIN							GPIO_Pin_9			        

extern unsigned long Weight_Maopi;
extern long Weight_Shiwu;
extern char Flag_ERROR;

unsigned long HX711_Read(void);
void dmziig_GPIO_Config(void);
void Get_Maopi(void);
void Get_Weight(void);
unsigned long HX711_Read(void);
void dmziig_init(void);


#endif 
