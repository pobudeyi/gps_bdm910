#ifndef _TYPE_H_
#define _TYPE_H_

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
 
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

#define MSG_SEND_REQ 1
#define MSG_TIME_SYNC 2
#define MSG_MOTOR_SET 3
#define MSG_MOTOR_READ 4
#define MSG_READ_DIR 5
#define MSG_READ_FILE 6
#define MSG_READ_DATA 7
#define MSG_SEND_END 8
#define MSG_GPS_REQ 11

#if 0
typedef struct NMEA_MSG
{
    struct UTC/*utc时间日期*/
    {
        int year;
        int month;
        int date;
        int hour;
        int mint;
        int sec;
    }utc;
    float speed;/*地面速率*/
    float latitude;/*纬度*/
    float longitude;/*经度*/
    float altitude;/*海拔高度*/
    float pdop;/*综合位置精度因子*/
    float hdop;/*水平精度因子*/
    float vdop;/*垂直精度因子*/
    char nshemi;/*南北纬*/
    char ewhemi;/*东西经*/
    char avhemi;/*定位状态//A有效定位、V无效定位*/
	char pattern;/*定位模式指示//A自主定位、D差分、E估算、N数据无效*/
    char mode;/*模式//M手动、A自动*/
	int mntude;/*磁北航向角度*/
	int entude;/*地北航向角度*/
    int svnum;/*可见卫星数//至多12颗*/
    int gpssta;/*GPS状态//0未定位、1非差分定位、2差分定位*/
    int posslnum;/*定位卫星数目*/   
    int fixmode;/*定位类型//1未定位、2D定位、3D定位*/
    int possl[32];/*定位卫星编号*/
    struct SLMSG/*定位卫星信息//卫星编号、卫星仰角、卫星方位角、信号强度*/
    {
        int num;
        int eledeg;
        int azideg;
        int sn;
    }slmsg[32];
}Nmea_msg;
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

typedef struct NMEA_MSG
{
    struct UTC/*utc时间日期*/
    {
        unsigned char year;
        unsigned char month;
        unsigned char date;
        unsigned char hour;
        unsigned char mint;
        unsigned char sec;
		unsigned char reserve[2];//只是位了对齐
    }utc;
	double latitude;/*纬度*/
	double longitude;/*经度*/
	char nshemi;/*南北纬*/
	char ewhemi;/*东西经*/
	char avhemi;/*定位状态//A有效定位、V无效定位*/
	char reserve;//预留
	int entude;/*地北航向角度*/
	float speed;/*地面速率*/
	int reserve2;
}Nmea_msg;

typedef struct 							 //中继包体
{
	protocol_head_t protocol_head;
	union br_body_u
	{	
		Nmea_msg mea_msg;
		unsigned char data[256];
	}br_body;
}up_load_t;
 
#endif // _TYPE_H_
