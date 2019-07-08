/* USER CODE BEGIN Private defines */  
#include "stm32f1xx_hal.h"
#define RECEIVELEN 256  
#define SENDLEN 256  
#define USART_DMA_SENDING 1//发送未完成  
#define USART_DMA_SENDOVER 0//发送完成  

typedef struct  
{  
	uint8_t receive_flag;//空闲接收标记  
	uint8_t uart_no;//串口号
	uint16_t rx_len;//接收长度  
	uint8_t usartDMA_rxBuf[RECEIVELEN];//DMA接收缓存  
}USART_RECEIVETYPE;  

typedef struct  
{  
	uint8_t dmaSend_flag;//发送完成标记 
	uint8_t uart_no;//串口号
	uint16_t tx_len;//接收长度  
	uint8_t usartDMA_txBuf[SENDLEN];//DMA接收缓存    
}USART_SENDTYPE;  

extern USART_SENDTYPE UsartSendType1;
extern USART_SENDTYPE UsartSendType2;
extern USART_SENDTYPE UsartSendType3;


extern USART_RECEIVETYPE UsartRecvType1;  
extern USART_RECEIVETYPE UsartRecvType2;    
extern USART_RECEIVETYPE UsartRecvType3;  


void uart_init(void);
void Usart1SendData_DMA(UART_HandleTypeDef *huart,uint8_t *pdata, uint16_t Length);  
//void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart); 
void UsartReceive_IDLE(UART_HandleTypeDef *huart); 
