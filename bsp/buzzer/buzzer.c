#include "buzzer.h"
void buzzer_GPIO_Config(void)
{		
		
		/*	����gpio*/
		/*����һ��GPIO_InitTypeDef���͵Ľṹ��*/
		GPIO_InitTypeDef GPIO_InitStructure;
		/*������ص�GPIO����ʱ��*/
		RCC_APB2PeriphClockCmd( buzzer_GPIO_CLK , ENABLE);
		/*ѡ��Ҫ���Ƶ�GPIO����*/
		GPIO_InitStructure.GPIO_Pin = buzzer_GPIO_PIN;
		/*��������ģʽΪͨ���������*/
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
		/*������������Ϊ50MHz */   
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
		/*���ÿ⺯������ʼ��GPIO*/
		GPIO_Init(buzzer_GPIO_PORT, &GPIO_InitStructure);	
		
}



