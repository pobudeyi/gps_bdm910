#ifndef __BEIDOU_H__
#define __BEIDOU_H__

//#define SEND
#define MAX_PAYLOAD_LEN 210 //即(1680/8)
#define INSTRUCTION_SIZE 5
#define PACKET_LEN_SIZE 2
#define USER_ADDR_SIZE 3
#define CHECKSUM_SIZE 1
#define IPUC (INSTRUCTION_SIZE + PACKET_LEN_SIZE + USER_ADDR_SIZE + CHECKSUM_SIZE)
#define TXSQ_INFO_FIRM_SIZE 7 //即(1 个信息类别 + 3 个用户地址 + 2个电文长度 + 1个应答字节) 
#define TXSQ_FIRM_SIZE (IPUC + TXSQ_INFO_FIRM_SIZE)
#define ICJC_INFO_FIRM_SIZE 1 //即帧号，占一个字节
#define ICJC_FIRM_SIZE (IPUC + ICJC_INFO_FIRM_SIZE)
#define XTZJ_INFO_FIRM_SIZE 2 //即自检频度，占二个字节
#define XTZJ_FIRM_SIZE (IPUC + XTZJ_INFO_FIRM_SIZE)
#define ICXX_INFO_FIRM_SIZE 11 //即(1个帧号+3个通播ID+1个用户特征+2个服务频度+1个通信等级+1个加密标志+2个下属用户总数)
#define ICXX_FIRM_SIZE (IPUC + ICXX_INFO_FIRM_SIZE)
#define TXXX_INFO_FIRM_SIZE 9 //即(1个信息类别+3个发信方地址+2个发信时间+2个电文长度+1个CRC标志
#define TXXX_FIRM_SIZE (IPUC + TXXX_INFO_FIRM_SIZE)
#define TXXX_MAX_SIZE (TXXX_FIRM_SIZE + MAX_PAYLOAD_LEN)//TXXX由固定长度和净长度构成
#define FKXX_INFO_FIRM_SIZE 5//即(1个反馈标志+4个附加信息)
#define FKXX_FIRM_SIZE (IPUC + FKXX_INFO_FIRM_SIZE)
#define ZJXX_INFO_FRIM_SIZE 10 //即(1个IC卡状态+1个硬件状态+1个电池电量+1个入站状态+6个功率状态)
#define ZJXX_FIRM_SIZE (IPUC + ZJXX_INFO_FRIM_SIZE)
#define RX_BD_MAX_DATA_SIZE TXXX_MAX_SIZE 
#define TXSQ_PAYLOAD_CHINESE 0x44
#define TXSQ_PAYLOAD_BCD 0x46

enum {
    DWXX_BUF = (1 << 0),
    TXXX_BUF = (1 << 1),
    ICXX_BUF = (1 << 2),
    ZJXX_BUF = (1 << 3),
    SJXX_BUF = (1 << 4),
    BBXX_BUF = (1 << 5),
    FKXX_BUF = (1 << 6),
};
/* =================== RTU到用户机 ============================ */
/* 
    注意:因为发送协议中通信申请(txsq)协议有可变数据内容，使用结构体来表示时，因为要通过串口发送出去，
    无法控制长度，所以发送协议不宜采用结构体表示!
struct peri_to_user_struct
{
    struct dwsq_struct dwsq;
    struct txsq_struct txsq;
    struct cksc_struct cksc;
    struct icjc_struct icjc;
    struct xtzj_struct xtzj;
    struct sjsc_struct sjsc;
    struct bbdq_struct bbdq;
};
*/

/* =================== 用户机到RTU ============================*/
/* 定位信息 */
struct dwxx_struct
{
    unsigned int todo;
};

/* 通信信息 */
struct txxx_info_type_struct
{
    unsigned char packet_comm:2;
    unsigned char transfer_format:1;
    unsigned char ack:1;
    unsigned char comm_way:1;
    unsigned char has_key:1;
    unsigned char rest:2;
};

struct send_time_struct
{
    unsigned char hour;
    unsigned char minute;
};

struct txxx_info_struct
{
    struct txxx_info_type_struct  txxx_info_type;
    unsigned char src_user_addr[3];
    struct send_time_struct send_time;
    unsigned int payload_len;
    unsigned char payload[MAX_PAYLOAD_LEN];
    unsigned char crc;  
};

struct txxx_struct 
{
    unsigned char instruction[5];
    unsigned int packet_len; //解析结构体时以整形数据表示其长度
    unsigned char user_addr[3];
    struct txxx_info_struct txxx_info;
    unsigned char checksum;
};

struct icxx_info_struct
{
    unsigned char frame_id;
    unsigned char broadcast_id[3];
    unsigned char user_feature;
    unsigned int service_frequency;
    unsigned char comm_level;
    unsigned char encryption_flag;
    unsigned int user_num;
};

/* IC信息 */
struct icxx_struct
{
    unsigned char instruction[5];
    unsigned int packet_len;
    unsigned char user_addr[3];
    struct icxx_info_struct icxx_info;
    unsigned char checksum;
};

struct zjxx_info_struct
{
    unsigned char ic_status;
    unsigned char hw_status;
    unsigned char battery_quantity;
    unsigned char in_station_status;
    unsigned char power_status[6];
};

struct zjxx_struct
{
    unsigned char instruction[5];
    unsigned int packet_len;
    unsigned char user_addr[3];
    struct zjxx_info_struct zjxx_info;
    unsigned char checksum;
};

struct sjxx_struct
{
    unsigned int todo;
};

struct bbxx_struct
{
    unsigned int todo;
};

struct fkxx_info_struct
{
    unsigned char fk_flag;
    unsigned char extra_info[4];
};

struct fkxx_struct
{
    unsigned char instruction[5];
    unsigned int packet_len;  
    unsigned char user_addr[3];
    struct fkxx_info_struct fkxx_info;
    unsigned char checksum;
};

/*
struct user_to_peri_struct
{
    struct dwxx_struct dwxx;
    struct txxx_struct txxx;
    struct icxx_struct icxx;
    struct zjxx_struct zjxx;
    struct sjxx_struct sjxx;
    struct bbxx_struct bbxx;
    struct fkxx_struct fkxx;
};

*/
//extern void send_dwsq();
extern void send_txsq(unsigned int src_user_addr_long, unsigned int dst_user_addr_long, 
               unsigned char transfer_format, unsigned char *send_txsq_payload, unsigned int send_txsq_payload_len);
//extern void send_cksc();
//extern void send_icjc();
//extern void send_xtzj();
//extern void send_sjsc();
//extern void send_bbdq();
extern void read_bd_rx_info(unsigned char *txxx_buf);
unsigned char copy_packet_from_shared_buf(unsigned char *bd_shared_rx_buf);
unsigned char xor_checksum (unsigned char *buf, unsigned int len);
#endif
