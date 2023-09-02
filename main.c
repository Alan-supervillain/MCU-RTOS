/*
���ߣ�alan
���䣺3096141163@qq.com
*/
#include "FreeRTOS.h"
#include "task.h"
#include "bsp_led.h"
#include "bsp_usart.h"
#include "bsp_key.h"
#include "buzzer.h"
#include "bsp_dmziig.h"//���ӳ�(˫ƴ���뷨��ƴ��)
#include "mylog.h"     //��־��ȼ���  INFO һ�����ڴ�ӡЩ״̬��Ϣ�����Ķ�  ��WARN�ǵ����õ� ���漶���
#include "bsp_dht11.h"
#include "delay.h"
#include "sys.h"
#include "oled.h"
#include "bmp.h"
#include "queue.h"
#include "ADC_drv.h"
#include "event_groups.h"
#include "bsp_iwdg.h" 

#define display_Task_EVENT  (0x01 << 0)//�����¼������λ0
#define Weight_Task_EVENT   (0x01 << 1)//�����¼������λ1
#define MQ2_Task_EVENT      (0x01 << 2)//�����¼������λ2

#define waring_numb   900           //�������趨ֵ
#define waring_temp   40            //���±�������ֵ

static TaskHandle_t AppTaskCreate_Handle = NULL;
static TaskHandle_t DHT11_Task_Handle = NULL;      /* Weight_Task������ */
static TaskHandle_t Weight_Task_Handle = NULL;     /* ��ʾ�������� */
static TaskHandle_t display_Task_Handle = NULL;
static TaskHandle_t MQ2_Task_Handle = NULL;        //ADC����  ������������
/********************************** �ں˶����� *********************************/

QueueHandle_t my_Queue =NULL;    //�õ�FIFO
#define  QUEUE_LEN    4   /* ���еĳ��ȣ����ɰ������ٸ���Ϣ */
#define  QUEUE_SIZE   4   /* ������ÿ����Ϣ��С���ֽڣ� */ //�պù���
//�¼���
static EventGroupHandle_t Event_Handle =NULL;

/******************************* ȫ�ֱ������� ************************************/

DHT11_Data_TypeDef DHT11_Data;
uint16_t somke ;//����Ũ��

/*
*************************************************************************
*                             ��������
*************************************************************************
*/
static void AppTaskCreate(void);/* ���ڴ������� */

static void DHT11_Task(void* pvParameters);/* TaskDHT11����ʵ�� */
static void Weight_Task(void* pvParameters);/* Weight_Task_Task����ʵ�� */
static void display_Task(void* pvParameters);
static void MQ2_Task(void* pvParameters);

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
  /* ���� Event_Handle */
  Event_Handle = xEventGroupCreate();
    /* ����my_Queue */
  my_Queue = xQueueCreate((UBaseType_t ) QUEUE_LEN,/* ��Ϣ���еĳ��� */
                            (UBaseType_t ) QUEUE_SIZE);/* ��Ϣ�Ĵ�С */
  if(NULL != my_Queue)
    INFO("����my_Queue��Ϣ���гɹ�!\r\n");
  /*DHT11�Ȼ�ȡ���� Ȼ��vtaskdelay weight�� display_Task��ͬ���ȼ�*/
  /* ����DHT11_Task���� */
  xReturn = xTaskCreate((TaskFunction_t )DHT11_Task, /* ������ں��� */
                        (const char*    )"DHT11_Task",/* �������� */
                        (uint16_t       )512,   /* ����ջ��С */
                        (void*          )NULL,	/* ������ں������� */
                        (UBaseType_t    )4,	    /* ��������ȼ� */
                        (TaskHandle_t*  )&DHT11_Task_Handle);/* ������ƿ�ָ�� */
  if(pdPASS == xReturn)
    INFO("����DHT11_Task����ɹ�!\r\n");
  
	/* ����Weight_Task���� */
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

   xReturn = xTaskCreate((TaskFunction_t )MQ2_Task, /* ������ں��� */
                        (const char*    )"MQ2_Task",/* �������� */
                        (uint16_t       )512,   /* ����ջ��С */
                        (void*          )NULL,	/* ������ں������� */
                        (UBaseType_t    )2,	    /* ��������ȼ� */
                        (TaskHandle_t*  )&MQ2_Task_Handle);/* ������ƿ�ָ�� */
  if(pdPASS == xReturn)
    INFO("����MQ2_Task����ɹ�!\r\n");


  vTaskDelete(AppTaskCreate_Handle); //ɾ��AppTaskCreate����
  
  taskEXIT_CRITICAL();            //�˳��ٽ���
}

//���ȼ�4��1�����˵�
static void DHT11_Task(void* parameter)
{	
    EventBits_t r_event;
    while (1)
    {
      if  ( DHT11_Read_TempAndHumidity ( & DHT11_Data ) == SUCCESS)
			{
				printf("ʪ��Ϊ%d.%d ��RH ���¶�Ϊ %d.%d�� \r\n",\
				DHT11_Data.humi_int,DHT11_Data.humi_deci,DHT11_Data.temp_int,DHT11_Data.temp_deci);
        if(DHT11_Data.temp_int > waring_temp)
        {
            buzzer_on;
            LED_RED;
        }
			}			
		  else
			{
				printf("Read DHT11 ERROR!\r\n");
			}

      //�����������ȼ����������ռ����������Ƿ������е������� ���е����ȴ�����Ȼ������
      r_event = xEventGroupWaitBits(Event_Handle,  /* �¼������� */
                                MQ2_Task_EVENT|Weight_Task_EVENT|display_Task_EVENT,/* �����̸߳���Ȥ���¼� */
                                pdTRUE,   /* �����˳�ʱ����¼�λ */
                                pdTRUE,   /* �������Ȥ�������¼� */
                                portMAX_DELAY);/* ָ����ʱʱ�� */
      //������¼�λ�����ã�����������������������������򷵻ص�ֵ��������������˳�����̬���¼����ֵ����������ڳ�ʱ�˳����򷵻ص�ֵ����������������˳�����̬���¼����ֵ��
      if((r_event & (MQ2_Task_EVENT|Weight_Task_EVENT|display_Task_EVENT)) == (MQ2_Task_EVENT|Weight_Task_EVENT|display_Task_EVENT)) 
      {
        /* ���������ɲ�����ȷ */
        printf( "MQ2_Task_EVENT|Weight_Task_EVENT|display_Task_EVENT is ����\n");
        INFO("---ι��---");
        IWDG_ReloadCounter();
      }
      else{
        printf( "�¼�����\n");
      }
      vTaskDelay(1000);
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
    BaseType_t xReturn = pdPASS;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
    uint32_t wei_1 = 0;
    uint32_t wei_2 = 0;
    uint32_t wei_3 = 0;
    uint32_t wei_4 = 0;
    while (1)
    {
      Flag_ERROR =0;
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
      {       
        wei_4=(Weight_Shiwu/1000);    
        wei_3=(Weight_Shiwu%1000/100);
        wei_2=(Weight_Shiwu%100/10);
        wei_1=(Weight_Shiwu%10);
        printf("%ld%ld%ld%ld g\n",wei_4,wei_3,wei_2,wei_1);
        xReturn = xQueueSend( my_Queue, /* ��Ϣ���еľ�� */
                              &wei_4,/* ���͵���Ϣ���� */
                              0 );        /* �ȴ�ʱ�� 0 */
                                                         if(pdPASS == xReturn){INFO("��Ϣwei_4���ͳɹ�!\n");}else {INFO("��Ϣwei_4����ʧ��!\n");}
        xReturn = xQueueSend( my_Queue, &wei_3, 0 );     if(pdPASS == xReturn){INFO("��Ϣwei_3���ͳɹ�!\n");}else {INFO("��Ϣwei_3����ʧ��!\n");}
        xReturn = xQueueSend( my_Queue, &wei_2, 0 );     if(pdPASS == xReturn){INFO("��Ϣwei_2���ͳɹ�!\n");}else {INFO("��Ϣwei_2����ʧ��!\n");}
        xReturn = xQueueSend( my_Queue, &wei_1, 0 );     if(pdPASS == xReturn){INFO("��Ϣwei_1���ͳɹ�!\n");}else {INFO("��Ϣwei_1����ʧ��!\n");}
        //ҵ���߼� �����������������������ر�buzzer �˴����֮ǰ��ѹ���������Ѿ�������
        if((somke<=waring_numb)&&(DHT11_Data.temp_int<waring_temp)){
            buzzer_off;
            LED_RGBOFF;
        }

      INFO("��task ���һ�����У��������飡\n" );
			/* ����һ���¼� */
			xEventGroupSetBits(Event_Handle,Weight_Task_EVENT);

        vTaskDelay(500);    //����̫���� �������һ���������л���
			}
    }
}
static void MQ2_Task(void* pvParameters){
    for(;;)
    {
      somke = Get_Adc();
		  printf("����Ũ����%d\n",somke);
      if(somke > waring_numb){
          buzzer_on;
          LED_RED;
      }
      
      INFO("��task ���һ�����У��������飡\n" );
			/* ����һ���¼� */
			xEventGroupSetBits(Event_Handle,MQ2_Task_EVENT);

      vTaskDelay(500);
    }
}
static void display_Task(void* parameter)
{	
      BaseType_t xReturn = pdTRUE;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdTRUE */
      uint32_t r_wei;	/* ����һ��������Ϣ�ı��� */
    //��������   �ֵĴ�СŪ��16*16
#define line_1  0
      OLED_ShowCHinese(0,line_1,0);//��
      OLED_ShowCHinese(16,line_1,1);//��
      OLED_ShowString(35,line_1,"alan",5);
      OLED_ShowString(50,line_1+2,"/( >,<)/",9);
      OLED_ShowString(0,line_1+3,"https://github.com/Alan-supervillain/MCU-RTOS",46);
      vTaskDelay(1500);
      OLED_Clear();

#define line_2  2
#define line_3  4
#define line_4  6
      //�����ǲ���Ҫһֱˢ�µ�����   
      OLED_ShowCHinese(0,0,2);//ʪ
      OLED_ShowCHinese(16,0,3);//��
      OLED_ShowCHinese(32,0,4);//Ϊ
      OLED_ShowChar(70,0,46,16);//  . aiisc �� 46

      OLED_ShowCHinese(0,line_2,5);//��
      OLED_ShowCHinese(16,line_2,3);//��
      OLED_ShowCHinese(32,line_2,4);//Ϊ*/
      OLED_ShowChar(70,line_2,46,16);//.

      OLED_ShowCHinese(0,line_3,6);//��
      OLED_ShowCHinese(16,line_3,7);//��
      OLED_ShowCHinese(32,line_3,4);//Ϊ
      OLED_ShowChar(88,line_3,103,16);  //g��ascii

      OLED_ShowCHinese(0,line_4,8);//��������
      OLED_ShowCHinese(16,line_4,9);
      OLED_ShowChar(32,line_4,58,16);  //����ascii
      OLED_ShowCHinese(48,line_4,10);
      OLED_ShowCHinese(64,line_4,11);
      u8 x=0;
    while (1)
    {
      OLED_ShowNum(48,0,DHT11_Data.humi_int,2,16);  //ʪ������
      OLED_ShowNum(83,0,DHT11_Data.humi_deci,1,16);  //ʪ��С��
//���x��0-127��  �����y�Ĵ�С��ʾ��һ���к�  �μǶ�����demo����0-63����������
      OLED_ShowNum(48,line_2,DHT11_Data.temp_int,2,16);
      OLED_ShowNum(83,line_2,DHT11_Data.temp_deci,1,16);
      xReturn = xQueueReceive( my_Queue,    /* ��Ϣ���еľ�� */
                                  &r_wei,      /* ���͵���Ϣ���� */
                                  portMAX_DELAY); /* �ȴ�ʱ�� һֱ�� */                   
      if(pdTRUE == xReturn){
        printf("��Ϣ���н���data is %d\n",r_wei);
      }
      else{
        DBG("���ݽ��ճ���,�������0x%lx",xReturn);
      }
      OLED_ShowNum(48+x,line_3,r_wei,1,16);
      x+=10;
      if(x>30)x=0;

      INFO("��task ���һ�����У���������!" );
			/* ����һ���¼� */
			xEventGroupSetBits(Event_Handle,display_Task_EVENT);
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
	delay_init();	    	 
	OLED_Init();
	ADC_InitConfig();
	OLED_Clear();
  /* ����Ƿ�Ϊ�������Ź���λ */
  if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)
  {
    INFO("�������Ź���λ");
    /* �����־ */
    RCC_ClearFlag();
  }
  else
  {
    /*���Ƕ������Ź���λ(����Ϊ�ϵ縴λ�����ֶ�������λ֮���) */
    INFO("���Ƕ������Ź���λ");
  }
  IWDG_Config(IWDG_Prescaler_64 ,625);// IWDG 1s ��ʱ���
	INFO("Ӳ����ʼ���ɹ�\n");
	INFO("��  ��  ��  ��\n");
	INFO("---------------------------------\n");
  WARN("����ɶ��Թ淶--�´��ύʵ��\n");
}

/********************************END OF FILE****************************/
