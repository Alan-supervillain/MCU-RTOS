#include "FreeRTOS.h"
#include "task.h"
#include "bsp_led.h"
#include "bsp_usart.h"
#include "bsp_key.h"
#include "buzzer.h"
#include "bsp_dmziig.h"//电子秤双拼
#include "mylog.h"     //日志库开满了
#include "bsp_dht11.h"

 /* 创建任务句柄 */
static TaskHandle_t AppTaskCreate_Handle = NULL;
/* 任务句柄 */
static TaskHandle_t DHT11_Task_Handle = NULL;
/* Weight_Task任务句柄 */
static TaskHandle_t Weight_Task_Handle = NULL;
/* 显示屏任务句柄 */
static TaskHandle_t dispaly_Handle = NULL;

	//扫描按键
void Scan_Key()   //豪华_硬件去抖
{
	
		if( GPIO_ReadInputDataBit(KEY1_GPIO_PORT, KEY1_GPIO_PIN) == KEY_ON )
				{
					// 松手检测
					while( GPIO_ReadInputDataBit(KEY1_GPIO_PORT, KEY1_GPIO_PIN) == KEY_ON )
							;
					Get_Maopi();			//去皮
				}	
}

/******************************* 全局变量声明 ************************************/
/*
 * 当我们在写应用程序的时候，可能需要用到一些全局变量。
 */
DHT11_Data_TypeDef DHT11_Data;

/*
*************************************************************************
*                             函数声明
*************************************************************************
*/
static void AppTaskCreate(void);/* 用于创建任务 */

static void DHT11_Task(void* pvParameters);/* TaskDHT11任务实现 */
static void Weight_Task(void* pvParameters);/* Weight_Task_Task任务实现 */
static void dispaly_Task(void* pvParameters);

static void BSP_Init(void);/* 用于初始化板载相关资源 */

/*****************************************************************
  * @brief  主函数
  * @param  无
  * @retval 无
  * @note   第一步：开发板硬件初始化 
            第二步：创建APP应用任务
            第三步：启动FreeRTOS，开始多任务调度
  ****************************************************************/
int main(void)
{	
  BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */

  /* 开发板硬件初始化 */
	BSP_Init();
   /* 创建AppTaskCreate任务 */
  xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,  /* 任务入口函数 */
                        (const char*    )"AppTaskCreate",/* 任务名字 */
                        (uint16_t       )512,  /* 任务栈大小 */
                        (void*          )NULL,/* 任务入口函数参数 */
                        (UBaseType_t    )1, /* 任务的优先级 */
                        (TaskHandle_t*  )&AppTaskCreate_Handle);/* 任务控制块指针 */ 
  /* 启动任务调度 */           
  if(pdPASS == xReturn)
    vTaskStartScheduler();   /* 启动任务，开启调度 */
  else
    return -1;  
  
  while(1);   /* 正常不会执行到这里 */    
}


/***********************************************************************
  * @ 函数名  ： AppTaskCreate
  * @ 功能说明： 为了方便管理，所有的任务创建函数都放在这个函数里面
  * @ 参数    ： 无  
  * @ 返回值  ： 无
  **********************************************************************/
static void AppTaskCreate(void)
{
  BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
  
  taskENTER_CRITICAL();           //进入临界区
  
  /* 创建DHT11_Task任务 */
  xReturn = xTaskCreate((TaskFunction_t )DHT11_Task, /* 任务入口函数 */
                        (const char*    )"DHT11_Task",/* 任务名字 */
                        (uint16_t       )512,   /* 任务栈大小 */
                        (void*          )NULL,	/* 任务入口函数参数 */
                        (UBaseType_t    )2,	    /* 任务的优先级 */
                        (TaskHandle_t*  )&DHT11_Task_Handle);/* 任务控制块指针 */
  if(pdPASS == xReturn)
    INFO("创建DHT11_Task任务成功!\r\n");
  
	/* 创建Task任务 */
  xReturn = xTaskCreate((TaskFunction_t )Weight_Task, /* 任务入口函数 */
                        (const char*    )"Weight_Task",/* 任务名字 */
                        (uint16_t       )512,   /* 任务栈大小 */
                        (void*          )NULL,	/* 任务入口函数参数 */
                        (UBaseType_t    )3,	    /* 任务的优先级 */
                        (TaskHandle_t*  )&Weight_Task_Handle);/* 任务控制块指针 */
  if(pdPASS == xReturn)
    INFO("创建Weight_Task任务成功!\r\n");
  
   xReturn = xTaskCreate((TaskFunction_t )dispaly_Task, /* 任务入口函数 */
                        (const char*    )"dispaly_Task",/* 任务名字 */
                        (uint16_t       )512,   /* 任务栈大小 */
                        (void*          )NULL,	/* 任务入口函数参数 */
                        (UBaseType_t    )1,	    /* 任务的优先级 */
                        (TaskHandle_t*  )&dispaly_Task_Handle);/* 任务控制块指针 */
  if(pdPASS == xReturn)
    INFO("创建Weight_Task任务成功!\r\n");


  vTaskDelete(AppTaskCreate_Handle); //删除AppTaskCreate任务
  
  taskEXIT_CRITICAL();            //退出临界区
}


static void DHT11_Task(void* parameter)
{	
    while (1)
    {
        if  ( DHT11_Read_TempAndHumidity ( & DHT11_Data ) == SUCCESS)
			{
				printf("湿度为%d.%d ％RH ，温度为 %d.%d℃ \r\n",\
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
  * @ 函数名  ： Weight_Task
  * @ 功能说明： Weight_Task任务主体
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/
static void Weight_Task(void* parameter)
{	
    while (1)
    {
      vTaskDelay(2000);    //发送太快了
      Get_Weight();			//称重
      Scan_Key();
      //显示当前重量
      if( Flag_ERROR == 1)
      {
        printf("ERROR-->超重\n");
        buzzer_on;                        //还可以改一下，一直叫，下面else才关--------------------------------
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
			}
    }
}


static void display_Task(void* parameter)
{	
    while (1)
    {
      //--------------------
    }
}
/***********************************************************************
  * @ 函数名  ： BSP_Init
  * @ 功能说明： 板级外设初始化，所有板子上的初始化均可放在这个函数里面
  * @ 参数    ：   
  * @ 返回值  ： 无
  *********************************************************************/
static void BSP_Init(void)
{
	/*
	 * STM32中断优先级分组为4，即4bit都用来表示抢占优先级，范围为：0~15
	 * 优先级分组只需要分组一次即可，以后如果有其他的任务需要用到中断，
	 * 都统一用这个优先级分组，千万不要再分组，切忌。
	 */
	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );

	LED_GPIO_Config();
	USART_Config();
	Key_GPIO_Config();    
	dmziig_GPIO_Config();
	dmziig_init();	
	buzzer_GPIO_Config();
	DHT11_Init();
	printf("硬件初始化成功\n");
	printf("你  好  兰  哥\n");
	printf("---------------------------------");
	printf("\n");
}

/********************************END OF FILE****************************/
