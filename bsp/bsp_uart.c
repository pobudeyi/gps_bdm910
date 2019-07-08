#include "stm32f1xx_hal.h"
#include "bsp_uart.h"
#include "usart.h"
#include "cmsis_os.h"

USART_SENDTYPE UsartSendType1;
USART_SENDTYPE UsartSendType2;
USART_SENDTYPE UsartSendType3;


USART_RECEIVETYPE UsartRecvType1;  
USART_RECEIVETYPE UsartRecvType2;    
USART_RECEIVETYPE UsartRecvType3;  
   
  
extern QueueHandle_t xSendQueue1;
extern QueueHandle_t xSendQueue2;
extern QueueHandle_t xSendQueue3;

/////* USER CODE BEGIN 1 */  
//#ifdef __GNUC__  
//  
//  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf 
// set to 'Yes') calls __io_putchar() */  
//#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)  
//#else  
//  
//  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)  
//#endif /* __GNUC__ */  
//#if 1      
//PUTCHAR_PROTOTYPE  
//{  
//    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);  
//    return ch;  
//}  
//#endif

void uart_init(void)
{
	HAL_UART_Receive_DMA(&huart1, (uint8_t*)UsartRecvType1.usartDMA_rxBuf, sizeof(USART_RECEIVETYPE));
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
	HAL_UART_Receive_DMA(&huart2, (uint8_t*)UsartRecvType2.usartDMA_rxBuf, sizeof(USART_RECEIVETYPE));
	  __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE); 
	HAL_UART_Receive_DMA(&huart3, (uint8_t*)UsartRecvType3.usartDMA_rxBuf, sizeof(USART_RECEIVETYPE));
	  __HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);  
}
//DMA发送完成中断回调函数  
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)  
{  
     __HAL_DMA_DISABLE(huart->hdmatx);  
	if(huart->Instance==USART1)
		UsartSendType1.dmaSend_flag = USART_DMA_SENDOVER;  
	else if(huart->Instance==USART2)
		UsartSendType2.dmaSend_flag = USART_DMA_SENDOVER;  	
	else if(huart->Instance==USART3)
		UsartSendType3.dmaSend_flag = USART_DMA_SENDOVER; 
}  
  
//串口接收空闲中断  
void UsartReceive_IDLE(UART_HandleTypeDef *huart)  
{  
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if((__HAL_UART_GET_FLAG(huart,UART_FLAG_IDLE) != RESET))  
    {   
        __HAL_UART_CLEAR_IDLEFLAG(huart);  
        HAL_UART_DMAStop(huart);  
		if(huart->Instance==USART1)
		{
			if(RECEIVELEN - __HAL_DMA_GET_COUNTER(huart->hdmarx)<=sizeof(USART_RECEIVETYPE))
			{
				UsartRecvType1.rx_len =  RECEIVELEN - __HAL_DMA_GET_COUNTER(huart->hdmarx); //取接收长度
				UsartRecvType1.receive_flag=1; 
				UsartRecvType1.uart_no=1;								
				xQueueSendFromISR(xSendQueue1,(void *)&UsartRecvType1,&xHigherPriorityTaskWoken);  
			}
			
		}
		else if(huart->Instance==USART2)
		{
			if(RECEIVELEN - __HAL_DMA_GET_COUNTER(huart->hdmarx)<=sizeof(USART_RECEIVETYPE))
			{
				UsartRecvType2.rx_len =  RECEIVELEN - __HAL_DMA_GET_COUNTER(huart->hdmarx); //取接收长度
				UsartRecvType2.receive_flag=1; 
				UsartRecvType2.uart_no=2;								
				xQueueSendFromISR(xSendQueue2,(void *)&UsartRecvType2,&xHigherPriorityTaskWoken);  
			}
			//HAL_UART_Receive_DMA(huart,UsartRecvType2.usartDMA_rxBuf,RECEIVELEN);
		}
		else if(huart->Instance==USART3)
		{
			if(RECEIVELEN - __HAL_DMA_GET_COUNTER(huart->hdmarx)<=sizeof(USART_RECEIVETYPE))
			{
				UsartRecvType3.rx_len =  RECEIVELEN - __HAL_DMA_GET_COUNTER(huart->hdmarx); //取接收长度
				UsartRecvType3.receive_flag=1; 
				UsartRecvType3.uart_no=3;								
				xQueueSendFromISR(xSendQueue3,(void *)&UsartRecvType3,&xHigherPriorityTaskWoken);  
			}
			//HAL_UART_Receive_DMA(huart,UsartRecvType3.usartDMA_rxBuf,RECEIVELEN);
		}
    }  
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken); 
}  
