/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */     
#include "bsp_uart.h"
#include "sys.h"
#include "delay.h"
#include "usart.h" 
//#include "bsp_timer.h"
#include "bsp_malloc.h"
//#include "adc.h"
#include  "stdarg.h"
#include  "stdio.h"
#include  "stdlib.h"
#include  "string.h"
#include  "bsp_gps.h"
#include "bsp_ms5837_task.h"
#include "delay.h"
#include "bsp_lora_task.h"
#include "bsp_crc.h"
#include "bsp_rtc.h"
#include "ff.h"
#include "exfuns.h"

extern QueueHandle_t xSendQueue3;
extern QueueHandle_t xSendQueue2;
extern QueueHandle_t xSendQueue8;
extern disp_status_t env_status;
extern motor_set_t motor_set;
extern uint8_t FILE_USE;

void InitLoRa(void)
{
	//set lora
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_SET);//POWER
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_4,GPIO_PIN_RESET);//M1
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_3,GPIO_PIN_RESET);//M0	
	
}
#if(DEBUG_LORA_SEND)
//Lora任务	
void Lora_send_task(void *pvParameters)
{
	up_load_t *RecLoraSendReq;  
	USART_RECEIVETYPE ucQueueMsgValue;
    while(1)
    {
		xQueueReceive(xSendQueue8,(USART_RECEIVETYPE *)&ucQueueMsgValue,(TickType_t)portMAX_DELAY); 
		RecLoraSendReq=(up_load_t *)&ucQueueMsgValue.usartDMA_rxBuf;
		if(memcmp(RecLoraSendReq,"WCGF",4)==0 && RecLoraSendReq->protocol_head.crc == crcCalculateCcitt(0x2018, (uint8_t *)&RecLoraSendReq->br_body, RecLoraSendReq->protocol_head.len - 16))
			HAL_UART_Transmit_DMA(&huart2,(uint8_t *)RecLoraSendReq,RecLoraSendReq->protocol_head.len);
		vTaskDelay(20);
    } 
}
#endif


void Lora_recv_task(void *pvParameters)
{
	up_load_t *RecLoraSendReq,SendLoraData;  
	FRESULT res; 
	FIL SD_File;
	uint32_t bytesread,file_len,file_cnt,send_len;  
	DIR dir;
	FILINFO fileinfo;
	uint16_t dir_num=0,time_out,flag=0;
	USART_RECEIVETYPE ucQueueMsgValue;
    while(1)
    {
		#ifdef DEBUG_LOCAL
		xQueueReceive(xSendQueue8,(USART_RECEIVETYPE *)&ucQueueMsgValue,(TickType_t)portMAX_DELAY); 
		#else
		xQueueReceive(xSendQueue2,(USART_RECEIVETYPE *)&ucQueueMsgValue,(TickType_t)portMAX_DELAY); 
		#endif
		RecLoraSendReq=(up_load_t *)&ucQueueMsgValue.usartDMA_rxBuf;
		if(RecLoraSendReq->protocol_head.msg_type==MSG_SEND_REQ && RecLoraSendReq->protocol_head.crc == crcCalculateCcitt(0x2018, (uint8_t *)&RecLoraSendReq->br_body, RecLoraSendReq->protocol_head.len - 16))
		{
			memcpy(&SendLoraData.protocol_head,&RecLoraSendReq->protocol_head,sizeof(protocol_head_t));
			SendLoraData.protocol_head.ack=1;
			SendLoraData.protocol_head.len=sizeof(protocol_head_t)+sizeof(disp_status_t);
			SendLoraData.br_body.disp_status=env_status;
			SendLoraData.protocol_head.crc=crcCalculateCcitt(0x2018, (uint8_t *)&SendLoraData.br_body, SendLoraData.protocol_head.len - 16);
			#ifdef DEBUG_LOCAL
				HAL_UART_Transmit_DMA(&huart8,(uint8_t *)&SendLoraData,SendLoraData.protocol_head.len);
			#else
			if(DEBUG_LORA_SEND)
			{
				memcpy(&SendLoraData.protocol_head,&RecLoraSendReq->protocol_head,RecLoraSendReq->protocol_head.len);
				HAL_UART_Transmit_DMA(&huart8,(uint8_t *)&SendLoraData,SendLoraData.protocol_head.len);
			}
			else
			{
				HAL_UART_Transmit_DMA(&huart2,(uint8_t *)&SendLoraData,SendLoraData.protocol_head.len);
			}
			#endif
		}	
		else if(RecLoraSendReq->protocol_head.msg_type==MSG_TIME_SYNC && RecLoraSendReq->protocol_head.crc == crcCalculateCcitt(0x2018, (uint8_t *)&RecLoraSendReq->br_body, RecLoraSendReq->protocol_head.len - 16))
		{
			RTC_Set_Time(RecLoraSendReq->br_body.msg_sync_time.hour,RecLoraSendReq->br_body.msg_sync_time.min,RecLoraSendReq->br_body.msg_sync_time.sec ,RTC_HOURFORMAT12_AM);	        //设置时间 ,时分秒
			RTC_Set_Date(RecLoraSendReq->br_body.msg_sync_time.year,RecLoraSendReq->br_body.msg_sync_time.mon,RecLoraSendReq->br_body.msg_sync_time.date,7);
			//printf("20%d:%d:%d-%d:%d:%d\r\n",RecLoraSendReq->br_body.msg_sync_time.year,RecLoraSendReq->br_body.msg_sync_time.mon,RecLoraSendReq->br_body.msg_sync_time.date,RecLoraSendReq->br_body.msg_sync_time.hour,RecLoraSendReq->br_body.msg_sync_time.min,RecLoraSendReq->br_body.msg_sync_time.sec);
			//设置日期，年月日
			memcpy(&SendLoraData.protocol_head,&RecLoraSendReq->protocol_head,sizeof(protocol_head_t));
			SendLoraData.protocol_head.ack=1;
			SendLoraData.protocol_head.len=sizeof(protocol_head_t);
			SendLoraData.protocol_head.crc=crcCalculateCcitt(0x2018, (uint8_t *)&SendLoraData.br_body, SendLoraData.protocol_head.len - 16);
			#ifdef DEBUG_LOCAL
				HAL_UART_Transmit_DMA(&huart8,(uint8_t *)&SendLoraData,SendLoraData.protocol_head.len);
			#else
			if(DEBUG_LORA_SEND)
			{
				memcpy(&SendLoraData.protocol_head,&RecLoraSendReq->protocol_head,RecLoraSendReq->protocol_head.len);
				HAL_UART_Transmit_DMA(&huart8,(uint8_t *)&SendLoraData,SendLoraData.protocol_head.len);
			}
			else
			{
				HAL_UART_Transmit_DMA(&huart2,(uint8_t *)&SendLoraData,SendLoraData.protocol_head.len);
			}
			#endif
		}
		else if(RecLoraSendReq->protocol_head.msg_type==MSG_MOTOR_SET && RecLoraSendReq->protocol_head.crc == crcCalculateCcitt(0x2018, (uint8_t *)&RecLoraSendReq->br_body, RecLoraSendReq->protocol_head.len - 16))
		{
			memcpy(&SendLoraData.protocol_head,&RecLoraSendReq->protocol_head,RecLoraSendReq->protocol_head.len);
			motor_set=SendLoraData.br_body.motor_set;
			SendLoraData.protocol_head.ack=1;
			SendLoraData.protocol_head.len=sizeof(protocol_head_t);
			SendLoraData.protocol_head.crc=crcCalculateCcitt(0x2018, (uint8_t *)&SendLoraData.br_body, SendLoraData.protocol_head.len - 16);
			#ifdef DEBUG_LOCAL
				HAL_UART_Transmit_DMA(&huart8,(uint8_t *)&SendLoraData,SendLoraData.protocol_head.len);
			#else
			if(DEBUG_LORA_SEND)
			{
				memcpy(&SendLoraData.protocol_head,&RecLoraSendReq->protocol_head,RecLoraSendReq->protocol_head.len);
				HAL_UART_Transmit_DMA(&huart8,(uint8_t *)&SendLoraData,SendLoraData.protocol_head.len);
			}
			else
			{
				HAL_UART_Transmit_DMA(&huart2,(uint8_t *)&SendLoraData,SendLoraData.protocol_head.len);
			}
			#endif
		}
		else if(RecLoraSendReq->protocol_head.msg_type==MSG_MOTOR_READ && RecLoraSendReq->protocol_head.crc == crcCalculateCcitt(0x2018, (uint8_t *)&RecLoraSendReq->br_body, RecLoraSendReq->protocol_head.len - 16))
		{
			memcpy(&SendLoraData.protocol_head,&RecLoraSendReq->protocol_head,RecLoraSendReq->protocol_head.len);
			SendLoraData.br_body.motor_set=motor_set;
			SendLoraData.protocol_head.ack=1;
			SendLoraData.protocol_head.len=sizeof(protocol_head_t)+sizeof(motor_set_t);
			SendLoraData.protocol_head.crc=crcCalculateCcitt(0x2018, (uint8_t *)&SendLoraData.br_body, SendLoraData.protocol_head.len - 16);
			#ifdef DEBUG_LOCAL
				HAL_UART_Transmit_DMA(&huart8,(uint8_t *)&SendLoraData,SendLoraData.protocol_head.len);
			#else
			if(DEBUG_LORA_SEND)
			{
				memcpy(&SendLoraData.protocol_head,&RecLoraSendReq->protocol_head,RecLoraSendReq->protocol_head.len);
				HAL_UART_Transmit_DMA(&huart8,(uint8_t *)&SendLoraData,SendLoraData.protocol_head.len);
			}
			else
			{
				HAL_UART_Transmit_DMA(&huart2,(uint8_t *)&SendLoraData,SendLoraData.protocol_head.len);
			}
			#endif
		}
		else if(RecLoraSendReq->protocol_head.msg_type==MSG_READ_DIR && RecLoraSendReq->protocol_head.crc == crcCalculateCcitt(0x2018, (uint8_t *)&RecLoraSendReq->br_body, RecLoraSendReq->protocol_head.len - 16))
		{	
			dir_num=0;
			res = f_opendir(&dir, "/"); //从当前目录开始
			if (res == FR_OK) 
			{
				time_out=100;
				while(FILE_USE&&--time_out)//确保sd卡此时没有在写入数据
					vTaskDelay(10);
				if(time_out)
				{
					flag=1;
					FILE_USE=1;
				}	
				memset(&SendLoraData,0,sizeof(SendLoraData));
				time_out=200;
				while(flag&&time_out--)
				{
					res = f_readdir(&dir, &fileinfo); //读取文件夹
					if(res != FR_OK || fileinfo.fname[0] == 0) 
						break;
					else
					{
						if(fileinfo.fsize>0&&fileinfo.fsize<500000&&fileinfo.fname[0]>0x2f&&fileinfo.fname[0]<0x3a)//保证文件是数字开头，保证文件长度不要有误
						{
							if(dir_num<50)
							{
								memcpy(&SendLoraData.br_body.dir_read.file_info[dir_num].name,&fileinfo.fname,14);
								SendLoraData.br_body.dir_read.file_info[dir_num].size=fileinfo.fsize;
								SendLoraData.br_body.dir_read.num=dir_num+1;
								dir_num++;
							}
							else//如果大于50个文件就分包传输
							{
		
								memcpy(&SendLoraData.protocol_head,&RecLoraSendReq->protocol_head,sizeof(protocol_head_t));
								SendLoraData.protocol_head.ack=1;
								SendLoraData.protocol_head.len=sizeof(protocol_head_t)+sizeof(dir_read_t);
								SendLoraData.protocol_head.crc=crcCalculateCcitt(0x2018, (uint8_t *)&SendLoraData.br_body, SendLoraData.protocol_head.len - 16);
								#ifdef DEBUG_LOCAL
								HAL_UART_Transmit_DMA(&huart8,(uint8_t *)&SendLoraData,SendLoraData.protocol_head.len);
								#endif
								dir_num=0;	
								memcpy(&SendLoraData.br_body.dir_read.file_info[dir_num].name,&fileinfo.fname,14);
								SendLoraData.br_body.dir_read.file_info[dir_num].size=fileinfo.fsize;
								SendLoraData.br_body.dir_read.num=dir_num+1;
								dir_num++;
							}
						}	
					}
					vTaskDelay(20);
				}
				if(SendLoraData.br_body.dir_read.num>0)
				{
					memcpy(&SendLoraData.protocol_head,&RecLoraSendReq->protocol_head,sizeof(protocol_head_t));
					SendLoraData.protocol_head.ack=1;
					SendLoraData.protocol_head.len=sizeof(protocol_head_t)+sizeof(file_info_t)*SendLoraData.br_body.dir_read.num+4;
					SendLoraData.protocol_head.crc=crcCalculateCcitt(0x2018, (uint8_t *)&SendLoraData.br_body, SendLoraData.protocol_head.len - 16);
					#ifdef DEBUG_LOCAL
					HAL_UART_Transmit_DMA(&huart8,(uint8_t *)&SendLoraData,SendLoraData.protocol_head.len);
					#endif
				}
				FILE_USE=0;
			}
			else
			{
				memcpy(&SendLoraData.protocol_head,&RecLoraSendReq->protocol_head,sizeof(protocol_head_t));
				SendLoraData.protocol_head.ack=1;
				SendLoraData.br_body.dir_read.num=0;
				SendLoraData.protocol_head.len=sizeof(protocol_head_t)+4;
				SendLoraData.protocol_head.crc=crcCalculateCcitt(0x2018, (uint8_t *)&SendLoraData.br_body, SendLoraData.protocol_head.len - 16);
			}

		}
		else if(RecLoraSendReq->protocol_head.msg_type==MSG_READ_FILE && RecLoraSendReq->protocol_head.crc == crcCalculateCcitt(0x2018, (uint8_t *)&RecLoraSendReq->br_body, RecLoraSendReq->protocol_head.len - 16))
		{	
			if(f_open(&SD_File, (char *)RecLoraSendReq->br_body.file_data, FA_READ) == FR_OK)
			{
				file_len=f_size(&SD_File);
				if(file_len%SEND_FRAM_LEN)
					file_cnt=file_len/SEND_FRAM_LEN+1;
				else
					file_cnt=file_len/SEND_FRAM_LEN;
				
				printf("len=%d\r\n",file_cnt);
				memcpy(&SendLoraData.br_body.file_data,&file_len,4);
				memcpy(&SendLoraData.br_body.file_data[4],&file_cnt,4);
				memcpy(&SendLoraData.protocol_head,&RecLoraSendReq->protocol_head,sizeof(protocol_head_t));
				SendLoraData.protocol_head.ack=1;
				SendLoraData.protocol_head.len=sizeof(protocol_head_t)+8;
				SendLoraData.protocol_head.crc=crcCalculateCcitt(0x2018, (uint8_t *)&SendLoraData.br_body, SendLoraData.protocol_head.len - 16);
			}
			else
			{
				memcpy(&SendLoraData.protocol_head,&RecLoraSendReq->protocol_head,sizeof(protocol_head_t));
				SendLoraData.protocol_head.ack=1;
				SendLoraData.br_body.dir_read.num=0xffffffff;//表示读取失败
				SendLoraData.protocol_head.crc=crcCalculateCcitt(0x2018, (uint8_t *)&SendLoraData.br_body, SendLoraData.protocol_head.len - 16);
			}
			#ifdef DEBUG_LOCAL
				HAL_UART_Transmit_DMA(&huart8,(uint8_t *)&SendLoraData,SendLoraData.protocol_head.len);
			#endif
		}
		else if(RecLoraSendReq->protocol_head.msg_type==MSG_READ_DATA && RecLoraSendReq->protocol_head.crc == crcCalculateCcitt(0x2018, (uint8_t *)&RecLoraSendReq->br_body, RecLoraSendReq->protocol_head.len - 16))
		{	
			bytesread=0;
			send_len=0;
			if(RecLoraSendReq->protocol_head.index==(file_cnt-1))
			{
				if(file_len%SEND_FRAM_LEN)
				{
					f_lseek(&SD_File,RecLoraSendReq->protocol_head.index*SEND_FRAM_LEN);
					while(file_len%SEND_FRAM_LEN-send_len)//保证读完所有字节
					{
						f_read(&SD_File, &SendLoraData.br_body.file_data[send_len], file_len%512-send_len, (void *)&bytesread);
						send_len+=bytesread;
					}
					memcpy(&SendLoraData.protocol_head,&RecLoraSendReq->protocol_head,sizeof(protocol_head_t));
					SendLoraData.protocol_head.ack=1;
					SendLoraData.protocol_head.len=sizeof(protocol_head_t)+file_len%SEND_FRAM_LEN;
					SendLoraData.protocol_head.crc=crcCalculateCcitt(0x2018, (uint8_t *)&SendLoraData.br_body, SendLoraData.protocol_head.len - 16);
					HAL_UART_Transmit_DMA(&huart8,(uint8_t *)&SendLoraData,SendLoraData.protocol_head.len);
				}
				else
				{
					f_lseek(&SD_File,RecLoraSendReq->protocol_head.index*SEND_FRAM_LEN);
					while(SEND_FRAM_LEN-send_len)//保证读完512字节
					{
						f_read(&SD_File, &SendLoraData.br_body.file_data[send_len], SEND_FRAM_LEN-send_len, (void *)&bytesread);
						send_len+=bytesread;
					}
					memcpy(&SendLoraData.protocol_head,&RecLoraSendReq->protocol_head,sizeof(protocol_head_t));
					SendLoraData.protocol_head.ack=1;
					SendLoraData.protocol_head.len=sizeof(protocol_head_t)+SEND_FRAM_LEN;
					SendLoraData.protocol_head.crc=crcCalculateCcitt(0x2018, (uint8_t *)&SendLoraData.br_body, SendLoraData.protocol_head.len - 16);
					HAL_UART_Transmit_DMA(&huart8,(uint8_t *)&SendLoraData,SendLoraData.protocol_head.len);
				}
			
			}
			else if(RecLoraSendReq->protocol_head.index<file_cnt)
			{
				f_lseek(&SD_File,RecLoraSendReq->protocol_head.index*SEND_FRAM_LEN);
				while(SEND_FRAM_LEN-send_len)
				{
					f_read(&SD_File, &SendLoraData.br_body.file_data[send_len], SEND_FRAM_LEN-send_len, (void *)&bytesread);
					send_len+=bytesread;
				}
				memcpy(&SendLoraData.protocol_head,&RecLoraSendReq->protocol_head,sizeof(protocol_head_t));
				SendLoraData.protocol_head.ack=1;
				SendLoraData.protocol_head.len=sizeof(protocol_head_t)+SEND_FRAM_LEN;
				SendLoraData.protocol_head.crc=crcCalculateCcitt(0x2018, (uint8_t *)&SendLoraData.br_body, SendLoraData.protocol_head.len - 16);
				HAL_UART_Transmit_DMA(&huart8,(uint8_t *)&SendLoraData,SendLoraData.protocol_head.len);
			}
			else//发送结束标志
			{
				memcpy(&SendLoraData.protocol_head,&RecLoraSendReq->protocol_head,sizeof(protocol_head_t));	
				SendLoraData.protocol_head.ack=1;
				SendLoraData.protocol_head.msg_type=MSG_SEND_END;	
				SendLoraData.protocol_head.crc=crcCalculateCcitt(0x2018, (uint8_t *)&SendLoraData.br_body, SendLoraData.protocol_head.len - 16);
				HAL_UART_Transmit_DMA(&huart8,(uint8_t *)&SendLoraData,SendLoraData.protocol_head.len);
				f_close(&SD_File);
				printf("send over \n");
			}
		}
	}
		
}
