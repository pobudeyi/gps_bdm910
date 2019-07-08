#ifndef __LORA_H
#define __LORA_H

#define SEND_FRAM_LEN 512

#define MSG_SEND_REQ 1
#define MSG_TIME_SYNC 2
#define MSG_MOTOR_SET 3
#define MSG_MOTOR_READ 4
#define MSG_READ_DIR 5
#define MSG_READ_FILE 6
#define MSG_READ_DATA 7
#define MSG_SEND_END 8

//三种组合，第一种：本地调试
#if 1
#define DEBUG_LOCAL //是否为本地调试
#define DEBUG_LORA_SEND 0 ///0：调试板1：主板
#endif

//第二种 lora 调试 发送端 当上位机
#if 0
//#define DEBUG_LOCAL //是否为本地调试
#define DEBUG_LORA_SEND 1 ///0：调试板1：主板
#endif

//第三种 lora调试 接收端 当下位机
#if 0
//#define DEBUG_LOCAL //是否为本地调试
#define DEBUG_LORA_SEND 0 //0：调试板1：主板
#endif

typedef struct 						//协议
{
	unsigned char head[4];					//协议头
	unsigned int id;						//设备ID
	unsigned short index;					//包编号
	unsigned short len;						//包长度
	unsigned short crc;						//crc校验
	unsigned char ack;						//应答
	unsigned char msg_type;					//包体类型       
}protocol_head_t;

typedef struct _disp_status_t
{
	unsigned int hight;//高度
	unsigned int depth;//深度
	unsigned int heading;//艘向
	unsigned int heeling;//横倾
	unsigned int trim;//纵倾
	unsigned int temp;//温度
	unsigned int pressure;//压力
	unsigned int resevre1;//预留
	unsigned int resevre2;//预留
	unsigned int resevre3;//预留
	unsigned int resevre4;//预留
	unsigned int resevre5;//预留
	unsigned int resevre6;//预留
	unsigned int resevre7;//预留
	unsigned int resevre8;//预留
	unsigned int resevre9;//预留
	unsigned int resevre10;//预留
}disp_status_t;

typedef struct 						//时间同步
{
	unsigned char year;						//年
	unsigned char mon;						//月
	unsigned char date;						//日
	unsigned char hour;						//时
	unsigned char min;						//分
	unsigned char sec;						//秒
}msg_sync_time_t;

typedef struct 						
{
	short motor1;						//1号舵机角度
	short motor2;						//2号舵机角度
	short motor3;						//3号舵机角度
	short motor4;						//4号舵机角度
	int motor_speed;					//电机速度
}motor_set_t;

typedef struct 						
{
	long size;
	char name[16];
}file_info_t;

typedef struct 						
{
	int num;
	file_info_t file_info[50];
}dir_read_t;

typedef struct 							 //中继包体
{
	protocol_head_t protocol_head;
	union br_body_u
	{	
		disp_status_t disp_status;
		msg_sync_time_t msg_sync_time;
		motor_set_t motor_set;
		dir_read_t dir_read;
		unsigned char file_data[512];
	}br_body;
}up_load_t;

void InitLoRa(void);
void Lora_send_task(void *pvParameters);
void Lora_recv_task(void *pvParameters);
#endif
