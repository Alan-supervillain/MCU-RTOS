#include "bsp_dmziig.h" 
#include "buzzer.h"
#include "FreeRTOS.h"
#include "task.h"
#include "bsp_usart.h"
#include "core_delay.h" 
//#include  <stdbool.h>
//static unsigned long HX711_Buffer = 0;
unsigned long Weight_Maopi = 0;
long Weight_Shiwu = 0;  //4���ֽ�
//static unsigned char flag = 0;
//static bool Flag_ERROR = 0;//  ԭ����bit Flag_ERROR = 0;�ò��������滻51 c��bit
char Flag_ERROR = 0;

void dmziig_init(void)
{
		CPU_TS_Tmr_Delay_MS(1000);//�ӳ�1s�ȴ�ϵͳ�ȶ�  //û��ʱ��
		Get_Maopi();				//��ëƤ����		
}
void dmziig_GPIO_Config(void)
{		
		
		/*	����gpio*/
		/*����һ��GPIO_InitTypeDef���͵Ľṹ��*/
		GPIO_InitTypeDef GPIO_InitStructure;
		/*������ص�GPIO����ʱ��*/
		RCC_APB2PeriphClockCmd( HX711_DOUT_GPIO_CLK | HX711_SCK_GPIO_CLK, ENABLE);
		/*ѡ��Ҫ���Ƶ�GPIO����*/
		GPIO_InitStructure.GPIO_Pin = HX711_DOUT_GPIO_PIN;	
		/*��������ģʽΪxxxxx*/
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;   
		/*������������Ϊ50MHz */   
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
		/*���ÿ⺯������ʼ��GPIO*/
		GPIO_Init(HX711_DOUT_GPIO_PORT, &GPIO_InitStructure);	
	
		/*ѡ��Ҫ���Ƶ�GPIO����*/
		GPIO_InitStructure.GPIO_Pin = HX711_SCK_GPIO_PIN;
	/*��������ģʽΪxxxx*/
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		/*���ÿ⺯������ʼ��GPIO*/
		GPIO_Init(HX711_SCK_GPIO_PORT, &GPIO_InitStructure);
}


//****************************************************
//��ȡëƤ����
//****************************************************
void Get_Maopi()
{
	Weight_Maopi = HX711_Read();	
} 


//****************************************************
//����
//****************************************************
void Get_Weight()
{
	Weight_Shiwu = HX711_Read();
	Weight_Shiwu = Weight_Shiwu - Weight_Maopi;		//��ȡ����
	if(Weight_Shiwu > 0)			
	{	
		Weight_Shiwu = (unsigned int)((float)Weight_Shiwu/GapValue); 	//����ʵ���ʵ������
																		
																		
		if(Weight_Shiwu > 5000)		//���ر���
		{
			Flag_ERROR = 1;
		}
		else
		{
			Flag_ERROR = 0;
		}
	}
	else
	{
		Weight_Shiwu = 0;
	//	Flag_ERROR = 1;				//���ر���
	}
	
}


unsigned long HX711_Read(void)	//A����128
{
	unsigned long count; 
	unsigned char i;  

	GPIO_ResetBits(HX711_SCK_GPIO_PORT, HX711_SCK_GPIO_PIN);
	count=0;
	while(GPIO_ReadInputDataBit(GPIOC, HX711_DOUT_GPIO_PIN))
		; 
	for(i=0;i<24;i++)
	{ 
		GPIO_SetBits(HX711_SCK_GPIO_PORT, HX711_SCK_GPIO_PIN);
		count=count<<1; 
		GPIO_ResetBits(HX711_SCK_GPIO_PORT, HX711_SCK_GPIO_PIN);
		if(GPIO_ReadInputDataBit(GPIOC, HX711_DOUT_GPIO_PIN)==1)
		{
			count++; 
		}
	} 
	GPIO_SetBits(HX711_SCK_GPIO_PORT, HX711_SCK_GPIO_PIN);
	count=count^0x800000;//��25�������½�����ʱ��ת������

	GPIO_ResetBits(HX711_SCK_GPIO_PORT, HX711_SCK_GPIO_PIN);
	return(count);
}

/*********************************************END OF FILE**********************/
