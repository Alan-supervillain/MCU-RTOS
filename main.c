/*
���ߣ�alan
���䣺3096141163@com
*/
#include "FreeRTOS.h"
#include "task.h"
#include "bsp_led.h"
#include "bsp_usart.h"
#include "bsp_key.h"
#include "buzzer.h"
#include "bsp_dmziig.h"//���ӳ�˫ƴ
#include "mylog.h"     //��־�⿪����
#include "bsp_dht11.h"

#include "draw_api.h"  //����������
#include "test.h"
#include "lcd.h"
#include "i2c_soft.h"
 /* ���������� */
static TaskHandle_t AppTaskCreate_Handle = NULL;
/* ������ */
static TaskHandle_t DHT11_Task_Handle = NULL;
/* Weight_Task������ */
static TaskHandle_t Weight_Task_Handle = NULL;
/* ��ʾ�������� */
static TaskHandle_t display_Task_Handle = NULL;

/******************************* ȫ�ֱ������� ************************************/
/*
 * ��������дӦ�ó����ʱ�򣬿�����Ҫ�õ�һЩȫ�ֱ�����
 */
DHT11_Data_TypeDef DHT11_Data;

/*
*************************************************************************
*                             ��������
*************************************************************************
*/
static void AppTaskCreate(void);/* ���ڴ������� */

static void DHT11_Task(void* pvParameters);/* TaskDHT11����ʵ�� */
static void Weight_Task(void* pvParameters);/* Weight_Task_Task����ʵ�� */
static void display_Task(void* pvParameters);

static void BSP_Init(void);/* ���ڳ�ʼ�����������Դ */

/*****************************************************************
  * @brief  ������
  * @param  ��
  * @retval ��
  * @note   ��һ����������Ӳ����ʼ�� 
            �ڶ���������APPӦ������
            ������������FreeRTOS����ʼ���������
  ****************************************************************/
int main(void)
{	
  BaseType_t xReturn = pdPASS;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */

  /* ������Ӳ����ʼ�� */
	BSP_Init();
   /* ����AppTaskCreate���� */
  xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,  /* ������ں��� */
                        (const char*    )"AppTaskCreate",/* �������� */
                        (uint16_t       )512,  /* ����ջ��С */
                        (void*          )NULL,/* ������ں������� */
                        (UBaseType_t    )1, /* ��������ȼ� */
                        (TaskHandle_t*  )&AppTaskCreate_Handle);/* ������ƿ�ָ�� */ 
  /* ����������� */           
  if(pdPASS == xReturn)
    vTaskStartScheduler();   /* �������񣬿������� */
  else
    return -1;  
  
  while(1);   /* ��������ִ�е����� */    
}


/***********************************************************************
  * @ ������  �� AppTaskCreate
  * @ ����˵���� Ϊ�˷���������е����񴴽����������������������
  * @ ����    �� ��  
  * @ ����ֵ  �� ��
  **********************************************************************/
static void AppTaskCreate(void)
{
  BaseType_t xReturn = pdPASS;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
  
  taskENTER_CRITICAL();           //�����ٽ���
  
  /* ����DHT11_Task���� */
  xReturn = xTaskCreate((TaskFunction_t )DHT11_Task, /* ������ں��� */
                        (const char*    )"DHT11_Task",/* �������� */
                        (uint16_t       )512,   /* ����ջ��С */
                        (void*          )NULL,	/* ������ں������� */
                        (UBaseType_t    )2,	    /* ��������ȼ� */
                        (TaskHandle_t*  )&DHT11_Task_Handle);/* ������ƿ�ָ�� */
  if(pdPASS == xReturn)
    INFO("����DHT11_Task����ɹ�!\r\n");
  
	/* ����Task���� */
  xReturn = xTaskCreate((TaskFunction_t )Weight_Task, /* ������ں��� */
                        (const char*    )"Weight_Task",/* �������� */
                        (uint16_t       )512,   /* ����ջ��С */
                        (void*          )NULL,	/* ������ں������� */
                        (UBaseType_t    )3,	    /* ��������ȼ� */
                        (TaskHandle_t*  )&Weight_Task_Handle);/* ������ƿ�ָ�� */
  if(pdPASS == xReturn)
    INFO("����Weight_Task����ɹ�!\r\n");
  
   xReturn = xTaskCreate((TaskFunction_t )display_Task, /* ������ں��� */
                        (const char*    )"display_Task",/* �������� */
                        (uint16_t       )512,   /* ����ջ��С */
                        (void*          )NULL,	/* ������ں������� */
                        (UBaseType_t    )1,	    /* ��������ȼ� */
                        (TaskHandle_t*  )&display_Task_Handle);/* ������ƿ�ָ�� */
  if(pdPASS == xReturn)
    INFO("����display_Task����ɹ�!\r\n");


  vTaskDelete(AppTaskCreate_Handle); //ɾ��AppTaskCreate����
  
  taskEXIT_CRITICAL();            //�˳��ٽ���
}


static void DHT11_Task(void* parameter)
{	
    while (1)
    {
        if  ( DHT11_Read_TempAndHumidity ( & DHT11_Data ) == SUCCESS)
			{
				printf("ʪ��Ϊ%d.%d ��RH ���¶�Ϊ %d.%d�� \r\n",\
				DHT11_Data.humi_int,DHT11_Data.humi_deci,DHT11_Data.temp_int,DHT11_Data.temp_deci);
			}			
		else
			{
				printf("Read DHT11 ERROR!\r\n");
			}
			  vTaskDelay(2000);
    }
}

/**********************************************************************
  * @ ������  �� Weight_Task
  * @ ����˵���� Weight_Task��������
  * @ ����    ��   
  * @ ����ֵ  �� ��
  ********************************************************************/
static void Weight_Task(void* parameter)
{	
    while (1)
    {
      vTaskDelay(2000);    //����̫����
      Get_Weight();			//����
      Scan_Key();
      //��ʾ��ǰ����
      if( Flag_ERROR == 1)
      {
        printf("ERROR-->����\n");
        buzzer_on;                        
        LED_RED;
      }		
      else
      {       /*  
              Usart_SendByte(DEBUG_USARTx,(Weight_Shiwu/1000 + 0X30));
              Usart_SendByte(DEBUG_USARTx,(Weight_Shiwu%1000/100 + 0X30));
              Usart_SendByte(DEBUG_USARTx,(Weight_Shiwu%100/10 + 0X30));
              Usart_SendByte(DEBUG_USARTx,(Weight_Shiwu%10 + 0X30));
        */
        printf("%ld",(Weight_Shiwu/1000));
        printf("%ld",(Weight_Shiwu%1000/100));
        printf("%ld",(Weight_Shiwu%100/10));
        printf("%ld",(Weight_Shiwu%10));
        printf(" g\n");
        buzzer_off;
        LED_RGBOFF;
			}
    }
}

static void display_Task(void* parameter)
{	
    while (1)
    {
      INFO("test\n");//--------------------����
    }
}
/***********************************************************************
  * @ ������  �� BSP_Init
  * @ ����˵���� �弶�����ʼ�������а����ϵĳ�ʼ�����ɷ��������������
  * @ ����    ��   
  * @ ����ֵ  �� ��
  *********************************************************************/
static void BSP_Init(void)
{
	/*
	 * STM32�ж����ȼ�����Ϊ4����4bit��������ʾ��ռ���ȼ�����ΧΪ��0~15
	 * ���ȼ�����ֻ��Ҫ����һ�μ��ɣ��Ժ������������������Ҫ�õ��жϣ�
	 * ��ͳһ��������ȼ����飬ǧ��Ҫ�ٷ��飬�мɡ�
	 */
	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );

	LED_GPIO_Config();
	USART_Config();
	Key_GPIO_Config();    
	dmziig_GPIO_Config();
	dmziig_init();	
	buzzer_GPIO_Config();
	DHT11_Init();

  //delay_init();	    	 //���������
  I2C_GPIO_Config();
  LCD_Init();
	INFO("Ӳ����ʼ���ɹ�\n");
	INFO("��  ��  ��  ��\n");
	INFO("---------------------------------\n");
	
}

/********************************END OF FILE****************************/
