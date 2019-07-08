/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */     
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bsp_uart.h"
#include "usart.h" 
#include "nmea_gprmc_analysis.h"
#include "type.h"
#include "operation.h"
#include "bsp_crc.h"
#include "SEGGER_RTT.h"
#include "beidou.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
//任务优先级
#define START_TASK_PRIO		1
//任务堆栈大小	
#define START_STK_SIZE 		512  
//任务句柄
TaskHandle_t StartTask_Handler;
//任务函数
void start_task(void *pvParameters);

////任务优先级
//#define UART0_TASK_PRIO		4
////任务堆栈大小	
//#define UART0_STK_SIZE 		256  
////任务句柄
//TaskHandle_t UART0Task_Handler;
////任务函数
//void uart0_task(void *pvParameters);

//任务优先级
#define UART3_TASK_PRIO		4
//任务堆栈大小	
#define UART3_STK_SIZE 		512  
//任务句柄
TaskHandle_t UART3Task_Handler;
//任务函数
void bsp_beidou_task(void *pvParameters);

//任务优先级
#define UART2_TASK_PRIO		2
//任务堆栈大小	
#define UART2_STK_SIZE 		512  
//任务句柄
TaskHandle_t UART2Task_Handler;
//任务函数
void bsp_gps_task(void *pvParameters);

//任务优先级
#define UART1_TASK_PRIO		3
//任务堆栈大小	
#define UART1_STK_SIZE 		512  
//任务句柄
TaskHandle_t UART1Task_Handler;
//任务函数
void bsp_debug_task(void *pvParameters);
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
const char SET_BD_DW[]="$CFGSYS,h10\n";
const char SET_BD_MSG[]="$CFGMSG,0,1,1\n";
Nmea_msg mea_msg;
extern QueueHandle_t xSendQueue1;//485
extern QueueHandle_t xSendQueue2;//GNSS
extern QueueHandle_t xSendQueue3;//GDSS
/* USER CODE END Variables */
osThreadId defaultTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
//开始任务任务函数
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //进入临界区
    //创建UART0任务
//    xTaskCreate((TaskFunction_t )uart0_task,     	
//                (const char*    )"uart0_task",   	
//                (uint16_t       )UART0_STK_SIZE, 
//                (void*          )NULL,				
//                (UBaseType_t    )UART0_TASK_PRIO,	
//                (TaskHandle_t*  )&UART0Task_Handler);
	
	//gps任务//UART5 
	xTaskCreate((TaskFunction_t )bsp_gps_task,    	
                (const char*    )"bsp_gps_task",   	
                (uint16_t       )UART2_STK_SIZE, 
                (void*          )NULL,				
                (UBaseType_t    )UART2_TASK_PRIO,	
                (TaskHandle_t*  )&UART2Task_Handler);
				
	//beidou任务//UART3
	xTaskCreate((TaskFunction_t )bsp_beidou_task,    	
                (const char*    )"bsp_beidou_task",   	
                (uint16_t       )UART3_STK_SIZE, 
                (void*          )NULL,				
                (UBaseType_t    )UART3_TASK_PRIO,	
                (TaskHandle_t*  )&UART3Task_Handler);
				
	xTaskCreate((TaskFunction_t )bsp_debug_task,    	
                (const char*    )"bsp_debug_task",   	
                (uint16_t       )UART1_STK_SIZE, 
                (void*          )NULL,				
                (UBaseType_t    )UART1_TASK_PRIO,	
                (TaskHandle_t*  )&UART2Task_Handler);
				
	vTaskDelete(StartTask_Handler); //删除开始任务
    taskEXIT_CRITICAL();            //退出临界区
}

////uart任务函数 （集中测试用，后续每个串口单独任务出来）
//void uart0_task(void *pvParameters)
//{
//	USART_RECEIVETYPE ucQueueMsgValue;
//    while(1)
//    {
//		xQueueReceive(xSendQueue1,(USART_RECEIVETYPE *)&ucQueueMsgValue,(TickType_t)portMAX_DELAY); 
//		if(ucQueueMsgValue.uart_no==1)
//			HAL_UART_Transmit_DMA(&huart1, ucQueueMsgValue.usartDMA_rxBuf,ucQueueMsgValue.rx_len);     
//		else if(ucQueueMsgValue.uart_no==2)
//			HAL_UART_Transmit_DMA(&huart2, ucQueueMsgValue.usartDMA_rxBuf,ucQueueMsgValue.rx_len);//收到Lora的信号直接打印出来，没有做任何控制处理，
//		else if(ucQueueMsgValue.uart_no==3)
//			HAL_UART_Transmit_DMA(&huart3, ucQueueMsgValue.usartDMA_rxBuf,ucQueueMsgValue.rx_len);      
//		vTaskDelay(10);
//    }
//}



//GPS任务
void bsp_gps_task(void *pvParameters)
{
	uint8_t time_cnt=5;
	USART_RECEIVETYPE ucQueueMsgValue;

	while(time_cnt--)
	{
		HAL_UART_Transmit_DMA(&huart2, (unsigned char *)SET_BD_DW,strlen(SET_BD_DW));
		vTaskDelay(100);
	}
	
    while(1)
    {
		xQueueReceive(xSendQueue2,(USART_RECEIVETYPE *)&ucQueueMsgValue,(TickType_t)portMAX_DELAY); 
		HAL_UART_Receive_DMA(&huart2,UsartRecvType2.usartDMA_rxBuf,RECEIVELEN);
		if(NMEA_Checkout((char *)ucQueueMsgValue.usartDMA_rxBuf))
		{
			NMEA_GPRMC_Analysis(&mea_msg,ucQueueMsgValue.usartDMA_rxBuf);
//			SEGGER_RTT_printf(0,"%s\r\n",ucQueueMsgValue.usartDMA_rxBuf);
//			SEGGER_RTT_printf(0,"hour:%d min:%d sec:%d\r\n", mea_msg.utc.hour, mea_msg.utc.mint, mea_msg.utc.sec);
//			SEGGER_RTT_printf(0,"avhemi:%c\r\n", mea_msg.avhemi);
//			SEGGER_RTT_printf(0,"latitude:%f\r\n", mea_msg.latitude);
//			SEGGER_RTT_printf(0,"nshemi:%c\r\n", mea_msg.nshemi);
//			SEGGER_RTT_printf(0,"longitude:%f\r\n", mea_msg.longitude);
//			SEGGER_RTT_printf(0,"ewhemi:%c\r\n", mea_msg.ewhemi);
//			SEGGER_RTT_printf(0,"speed:%f\r\n", mea_msg.speed);
//			SEGGER_RTT_printf(0,"entude:%f\r\n", mea_msg.entude);
//			SEGGER_RTT_printf(0,"year:%d mon:%d date:%d\r\n", mea_msg.utc.year, mea_msg.utc.month, mea_msg.utc.date);
		}
			
		vTaskDelay(10);
    }
}  
extern struct txxx_struct txxx;
//北斗任务
void bsp_beidou_task(void *pvParameters)
{
	USART_RECEIVETYPE ucQueueMsgValue;
	up_load_t *RecLoraSendReq;
    while(1)
    {
		xQueueReceive(xSendQueue3,(USART_RECEIVETYPE *)&ucQueueMsgValue,(TickType_t)portMAX_DELAY); 
		HAL_UART_Receive_DMA(&huart3,UsartRecvType3.usartDMA_rxBuf,RECEIVELEN);//如果收到北斗通信数据，解析得到数据后转发给串口1
		if(ucQueueMsgValue.usartDMA_rxBuf[0]=='$'&& (ucQueueMsgValue.usartDMA_rxBuf[5] * 256 + ucQueueMsgValue.usartDMA_rxBuf[6])<=(TXSQ_FIRM_SIZE+MAX_PAYLOAD_LEN) &&\
		ucQueueMsgValue.usartDMA_rxBuf[(ucQueueMsgValue.usartDMA_rxBuf[5] * 256 + ucQueueMsgValue.usartDMA_rxBuf[6])-1]==xor_checksum(ucQueueMsgValue.usartDMA_rxBuf,(ucQueueMsgValue.usartDMA_rxBuf[5] * 256 + ucQueueMsgValue.usartDMA_rxBuf[6])-1))
		{
			
			if(copy_packet_from_shared_buf(ucQueueMsgValue.usartDMA_rxBuf)==1)
			{
				read_bd_rx_info(ucQueueMsgValue.usartDMA_rxBuf);
				RecLoraSendReq=(up_load_t *)&txxx.txxx_info.payload;
				if(RecLoraSendReq->protocol_head.msg_type==MSG_GPS_REQ && RecLoraSendReq->protocol_head.crc == crcCalculateCcitt(0x2018, (uint8_t *)&RecLoraSendReq->br_body, RecLoraSendReq->protocol_head.len - 16))
					HAL_UART_Transmit_DMA(&huart1,(uint8_t *)&txxx.txxx_info.payload,txxx.txxx_info.payload_len);
			}
				
		}
		vTaskDelay(10);
    }
}  

//$BDRMC,000133.000,V,3104.465517,N,12121.549250,E,0.000,0.000,,,E,N*2F
//$BDGGA,000133.000,3104.465517,N,12121.549250,E,0,00,127.000,30.744,M,0,M,,*5F
//$BDGLL,3104.465517,N,12121.549250,E,000133.000,V,N*5D
//$BDGSA,A,1,,,,,,,,,,,,,127.000,127.000,127.000*25 
void bsp_debug_task(void *pvParameters)
{
//	uint8_t time_cnt=5;
	USART_RECEIVETYPE ucQueueMsgValue;
	up_load_t *RecLoraSendReq,SendLoraData;  
    while(1)
    {
		xQueueReceive(xSendQueue1,(USART_RECEIVETYPE *)&ucQueueMsgValue,(TickType_t)portMAX_DELAY); 
		HAL_UART_Receive_DMA(&huart1,UsartRecvType1.usartDMA_rxBuf,RECEIVELEN);
		RecLoraSendReq=(up_load_t *)&ucQueueMsgValue.usartDMA_rxBuf;
		if(RecLoraSendReq->protocol_head.msg_type==MSG_GPS_REQ && RecLoraSendReq->protocol_head.crc == crcCalculateCcitt(0x2018, (uint8_t *)&RecLoraSendReq->br_body, RecLoraSendReq->protocol_head.len - 16))
		{
			memcpy(&SendLoraData.protocol_head,&RecLoraSendReq->protocol_head,sizeof(protocol_head_t));
			SendLoraData.protocol_head.ack=1;
			SendLoraData.protocol_head.len=sizeof(protocol_head_t)+sizeof(Nmea_msg);
			SendLoraData.br_body.mea_msg=mea_msg;
			SendLoraData.protocol_head.crc=crcCalculateCcitt(0x2018, (uint8_t *)&SendLoraData.br_body, SendLoraData.protocol_head.len - 16);
			HAL_UART_Transmit_DMA(&huart1,(uint8_t *)&SendLoraData,SendLoraData.protocol_head.len);
		}
		else if(RecLoraSendReq->protocol_head.len<=sizeof(up_load_t) && RecLoraSendReq->protocol_head.crc == crcCalculateCcitt(0x2018, (uint8_t *)&RecLoraSendReq->br_body, RecLoraSendReq->protocol_head.len - 16))
		{
			send_txsq(411260,318776,0,(uint8_t *)&RecLoraSendReq->br_body,RecLoraSendReq->protocol_head.len);
		}
			
		vTaskDelay(10);
    }
}  
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
       
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
	xTaskCreate((TaskFunction_t )start_task,          //任务函数
		(const char*    )"start_task",          //任务名称
		(uint16_t       )START_STK_SIZE,        //任务堆栈大小
		(void*          )NULL,                  //传递给任务函数的参数
		(UBaseType_t    )START_TASK_PRIO,       //任务优先级
		(TaskHandle_t*  )&StartTask_Handler);   //任务句柄   
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{

  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
//	uint8_t temp_data[]={0x24,0x54,0x58,0x58,0x58,0x00,0x1a,0x02,0xad,0xf7,0x40,0x02,0xad,0xf7,0x00,0x00,0x00,0x30,0xce,0xd2,0xb0,0xae,0xc4,0xe3,0x00,0x67};
  for(;;)
  {
#if 0	
		if(temp_data[0]=='$'&& \
		temp_data[(temp_data[5] * 256 + temp_data[6])-1]==xor_checksum(temp_data,(temp_data[5] * 256 + temp_data[6])-1))
		{
			copy_packet_from_shared_buf(temp_data);
			read_bd_rx_info(temp_data);
		}
#endif		
	//HAL_UART_Transmit_DMA(&huart3,"aaaaaaaaa",strlen("aaaaaaaaa"));
//	#ifdef SEND
//	send_txsq(1,"11","22",0,"aa",10);  
//    osDelay(5000);
//	#else
//	send_txsq(1,"11","22",0,"aa",10); 
	osDelay(10000);
//	#endif
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
