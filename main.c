/*
作者：alan
邮箱：3096141163@qq.com
*/
#include "FreeRTOS.h"
#include "task.h"
#include "bsp_led.h"
#include "bsp_usart.h"
#include "bsp_key.h"
#include "buzzer.h"
#include "bsp_dmziig.h"//电子秤双拼
#include "mylog.h"     //日志库等级版
#include "bsp_dht11.h"
#include "delay.h"
#include "sys.h"
#include "oled.h"
#include "bmp.h"
#include "queue.h"
static TaskHandle_t AppTaskCreate_Handle = NULL;
static TaskHandle_t DHT11_Task_Handle = NULL;
/* Weight_Task任务句柄 */
static TaskHandle_t Weight_Task_Handle = NULL;
/* 显示屏任务句柄 */
static TaskHandle_t display_Task_Handle = NULL;
/********************************** 内核对象句柄 *********************************/
/*
 * 信号量，消息队列，事件标志组，软件定时器这些都属于内核的对象，要想使用这些内核
 * 对象，必须先创建，创建成功之后会返回一个相应的句柄。实际上就是一个指针，后续我
 * 们就可以通过这个句柄操作这些内核对象。
 *
 * 内核对象说白了就是一种全局的数据结构，通过这些数据结构我们可以实现任务间的通信，
 * 任务间的事件同步等各种功能。至于这些功能的实现我们是通过调用这些内核对象的函数
 * 来完成的
 * 
 */
QueueHandle_t my_Queue =NULL;    //用的FIFO
#define  QUEUE_LEN    4   /* 队列的长度，最大可包含多少个消息 */
#define  QUEUE_SIZE   4   /* 队列中每个消息大小（字节） */
//刚好够用
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
static void display_Task(void* pvParameters);

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
    /* 创建my_Queue */
  my_Queue = xQueueCreate((UBaseType_t ) QUEUE_LEN,/* 消息队列的长度 */
                            (UBaseType_t ) QUEUE_SIZE);/* 消息的大小 */
  if(NULL != my_Queue)
    INFO("创建my_Queue消息队列成功!\r\n");
  /*DHT11先获取数据 然后vtaskdelay weight和 display_Task相同优先级*/
  /* 创建DHT11_Task任务 */
  xReturn = xTaskCreate((TaskFunction_t )DHT11_Task, /* 任务入口函数 */
                        (const char*    )"DHT11_Task",/* 任务名字 */
                        (uint16_t       )512,   /* 任务栈大小 */
                        (void*          )NULL,	/* 任务入口函数参数 */
                        (UBaseType_t    )3,	    /* 任务的优先级 */
                        (TaskHandle_t*  )&DHT11_Task_Handle);/* 任务控制块指针 */
  if(pdPASS == xReturn)
    INFO("创建DHT11_Task任务成功!\r\n");
  
	/* 创建Weight_Task任务 */
  xReturn = xTaskCreate((TaskFunction_t )Weight_Task, /* 任务入口函数 */
                        (const char*    )"Weight_Task",/* 任务名字 */
                        (uint16_t       )512,   /* 任务栈大小 */
                        (void*          )NULL,	/* 任务入口函数参数 */
                        (UBaseType_t    )2,	    /* 任务的优先级 */
                        (TaskHandle_t*  )&Weight_Task_Handle);/* 任务控制块指针 */
  if(pdPASS == xReturn)
    INFO("创建Weight_Task任务成功!\r\n");
  
   xReturn = xTaskCreate((TaskFunction_t )display_Task, /* 任务入口函数 */
                        (const char*    )"display_Task",/* 任务名字 */
                        (uint16_t       )512,   /* 任务栈大小 */
                        (void*          )NULL,	/* 任务入口函数参数 */
                        (UBaseType_t    )2,	    /* 任务的优先级 */
                        (TaskHandle_t*  )&display_Task_Handle);/* 任务控制块指针 */
  if(pdPASS == xReturn)
    INFO("创建display_Task任务成功!\r\n");


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
			vTaskDelay(1000);
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
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
    uint32_t wei_1 = 0;
    uint32_t wei_2 = 0;
    uint32_t wei_3 = 0;
    uint32_t wei_4 = 0;
    while (1)
    {
      Flag_ERROR =0;
      vTaskDelay(500);    //发送太快了
      Get_Weight();			//称重
      Scan_Key();
      //显示当前重量
      if( Flag_ERROR == 1)
      {
        printf("ERROR-->超重\n");
        //buzzer_on;   测试的时候太吵了 关了算了 led_red 替代！！                           
        LED_RED;                 WARN("这里涉及到互斥锁对资源的管理 在下一个else要设计但是还没写的");

      }		
      else
      {       /*  
              Usart_SendByte(DEBUG_USARTx,(Weight_Shiwu/1000 + 0X30));
              Usart_SendByte(DEBUG_USARTx,(Weight_Shiwu%1000/100 + 0X30));
              Usart_SendByte(DEBUG_USARTx,(Weight_Shiwu%100/10 + 0X30));
              Usart_SendByte(DEBUG_USARTx,(Weight_Shiwu%10 + 0X30));
        */
        wei_4=(Weight_Shiwu/1000);    
        wei_3=(Weight_Shiwu%1000/100);
        wei_2=(Weight_Shiwu%100/10);
        wei_1=(Weight_Shiwu%10);
        printf("%ld",wei_4);
        printf("%ld",wei_3);
        printf("%ld",wei_2);
        printf("%ld",wei_1);
        printf(" g\n");
        xReturn = xQueueSend( my_Queue, /* 消息队列的句柄 */
                              &wei_4,/* 发送的消息内容 */
                              0 );        /* 等待时间 0 */
                                                         if(pdPASS == xReturn)INFO("消息wei_4发送成功!\n");else INFO("消息wei_4发送失败!\n");
        xReturn = xQueueSend( my_Queue, &wei_3, 0 );     if(pdPASS == xReturn)INFO("消息wei_3发送成功!\n");else INFO("消息wei_3发送失败!\n");
        xReturn = xQueueSend( my_Queue, &wei_2, 0 );     if(pdPASS == xReturn)INFO("消息wei_2发送成功!\n");else INFO("消息wei_2发送失败!\n");
        xReturn = xQueueSend( my_Queue, &wei_1, 0 );     if(pdPASS == xReturn)INFO("消息wei_1发送成功!\n");else INFO("消息wei_1发送失败!\n");
        //buzzer_off;
        LED_RGBOFF;
			}
    }
}

static void display_Task(void* parameter)
{	
      BaseType_t xReturn = pdTRUE;/* 定义一个创建信息返回值，默认为pdTRUE */
      uint32_t r_wei;	/* 定义一个接收消息的变量 */
    //启动画面   字的大小弄了16*16
#define line_1  0
      OLED_ShowCHinese(0,line_1,0);//你
      OLED_ShowCHinese(16,line_1,1);//好
      OLED_ShowString(35,line_1,"alan",5);
      OLED_ShowString(50,line_1+2,"/( >,<)/",9);
      OLED_ShowString(0,line_1+3,"https://github.com/Alan-supervillain/MCU-RTOS",46);
      vTaskDelay(1500);
      OLED_Clear();

#define line_2  2
#define line_3  4
#define line_4  6
      //这里是不需要一直刷新的数据   
      OLED_ShowCHinese(0,0,2);//湿
      OLED_ShowCHinese(16,0,3);//度
      OLED_ShowCHinese(32,0,4);//为
      OLED_ShowChar(70,0,46,16);//  . aiisc 是 46

      OLED_ShowCHinese(0,line_2,5);//温
      OLED_ShowCHinese(16,line_2,3);//度
      OLED_ShowCHinese(32,line_2,4);//为*/
      OLED_ShowChar(70,line_2,46,16);//.

      OLED_ShowCHinese(0,line_3,6);//重
      OLED_ShowCHinese(16,line_3,7);//量
      OLED_ShowCHinese(32,line_3,4);//为
      OLED_ShowChar(88,line_3,103,16);  //g的ascii

      OLED_ShowCHinese(0,line_4,8);//作者兰政
      OLED_ShowCHinese(16,line_4,9);
      OLED_ShowChar(32,line_4,58,16);  //：的ascii
      OLED_ShowCHinese(48,line_4,10);
      OLED_ShowCHinese(64,line_4,11);
      u8 x=0;
    while (1)
    {
      OLED_ShowNum(48,0,DHT11_Data.humi_int,2,16);  //湿度整数
      OLED_ShowNum(83,0,DHT11_Data.humi_deci,1,16);  //湿度小数
//这个x是0-127的  而这个y的大小表示下一个行号  牢记而不是demo给的0-63！！！！！
      OLED_ShowNum(48,line_2,DHT11_Data.temp_int,2,16);
      OLED_ShowNum(83,line_2,DHT11_Data.temp_deci,1,16);
      xReturn = xQueueReceive( my_Queue,    /* 消息队列的句柄 */
                                  &r_wei,      /* 发送的消息内容 */
                                  portMAX_DELAY); /* 等待时间 一直等 */                   
      if(pdTRUE == xReturn)
        DBG("本次接收到的数据是%d\n",r_wei);
      else
        DBG("数据接收出错,错误代码0x%lx\n",xReturn);
      OLED_ShowNum(48+x,line_3,r_wei,1,16);
      x+=10;
      if(x>30)x=0;
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
  delay_init();	    	 
  OLED_Init();
  OLED_Clear();
	INFO("硬件初始化成功\n");
	INFO("你  好  兰  哥\n");
	INFO("---------------------------------\n");
}

/********************************END OF FILE****************************/
