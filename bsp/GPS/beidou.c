
/******************************************************************************************************
    1、北斗协议和GPS协议不一样，不是以"\r\n"为一条协议的结束.
    2、printf函数中的打印信息是const字符串常量，放在cpu内部flash，北斗模块printf打印过多，导致scanf("%s",payload);输入的内容被改写.
    3、协议发送时不要使用结构体表示发送内容，接收时可以使用结构体表示接收内容.
    6、注意北斗协议净荷封装的是RTU的协议，由于RTU协议也是以'$'符号位为协议的开始，因此需要进行转义，本程序以'M'替代净荷中的'$'.
******************************************************************************************************/
//#pragma  diag_suppress 870
#include "beidou.h"
#include "string.h"
#include "stdio.h"
#include "usart.h" 
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "bsp_uart.h"
#include "bsp_crc.h"
#include "SEGGER_RTT.h"

unsigned char bd_buf_bitmap = 0;
//unsigned char dwxx_buf[todo];
//unsigned char txxx_buf[TXXX_MAX_SIZE];
//unsigned char icxx_buf[ICXX_FIRM_SIZE];
//unsigned char zjxx_buf[ZJXX_FIRM_SIZE];
//unsigned char sjxx_buf[todo];
//unsigned char bbxx_buf[todo];
//unsigned char fkxx_buf[FKXX_FIRM_SIZE];
unsigned int rx_packet_len = 0;
unsigned char bd_rx_char;
unsigned int bd_buf_pointer = 0;
//unsigned char bd_shared_rx_buf[RX_BD_MAX_DATA_SIZE];
unsigned char  g_src_user_addr[3];
unsigned char  g_dst_user_addr[3];
unsigned char g_transfer_format;

struct txxx_struct txxx;
struct fkxx_struct fkxx;

extern UART_HandleTypeDef huart3;

/* 异或校验和算法 */
unsigned char xor_checksum (unsigned char *buf, unsigned int len)
{
    unsigned int i;
    unsigned char checksum = 0;
    for (i = 0; i < len; ++i)
    {
        checksum ^= *(buf++);
    }
    return checksum;
}

void create_txsq(unsigned char *src_user_addr, unsigned char *dst_user_addr, 
                                unsigned char transfer_format, unsigned char *payload, 
                                unsigned int payload_len, unsigned char *send_txsq_data)
{    
    /* 1、通信申请指令初始化，不采用memcpy等库函数，提高指令执行效率,只有涉及到大量数据赋值拷贝时才考虑用库函数 */
    send_txsq_data[0] = '$';   
    send_txsq_data[1] = 'T';   
    send_txsq_data[2] = 'X';  
    send_txsq_data[3] = 'S';  
    send_txsq_data[4] = 'Q';  
    /* 2、包长度，先传高位，再传低位 */
    send_txsq_data[5] = (TXSQ_FIRM_SIZE +  payload_len) / 256;
    send_txsq_data[6] = (TXSQ_FIRM_SIZE +  payload_len) % 256;
    /* 3、源用户地址 */
    send_txsq_data[7] = *src_user_addr;
    send_txsq_data[8] = *(src_user_addr + 1);
    send_txsq_data[9] = *(src_user_addr + 2);
    /* 4.1、信息-信息类别 */
    if (transfer_format == 0) //汉字
    {
        send_txsq_data[10] = TXSQ_PAYLOAD_CHINESE;//0b01000100; 
    }
    else //代码/混发
    {
        send_txsq_data[10] = TXSQ_PAYLOAD_BCD;//0b01000110;
    }

    /* 4.2、信息-目的用户地址 */
    send_txsq_data[11] = *dst_user_addr;
    send_txsq_data[12] = *(dst_user_addr + 1);
    send_txsq_data[13] = *(dst_user_addr + 2);
    /* 4.3、信息-电文净荷长度-单位是bit */ 
     send_txsq_data[14] = (payload_len * 8) / 256;
     send_txsq_data[15]  = (payload_len * 8) % 256;  
    /* 4.4、信息-是否应答 */
     send_txsq_data[16]  = 0;
    /* 4.5、信息-电文内容 */
    memcpy(&send_txsq_data[17] , payload, payload_len);
    /* 5、校验和 */
    send_txsq_data[TXSQ_FIRM_SIZE + payload_len -1] = xor_checksum(send_txsq_data, (TXSQ_FIRM_SIZE +  payload_len -1));
    //SEGGER_RTT_printf(0,"\r\n    xor_checksum = 0x%x\r\n",  xor_checksum(send_txsq_data, (TXSQ_FIRM_SIZE +  payload_len -1)));
}

void send_dwsq()
{
    //todo
}

/* 

    1、结构体不宜管理可变长度的数据协议,如通讯申请协议
    2、发送长度为6个字节("我爱你")，发送方式为中文，协议内容:
       txsq:24 54 58 53 51 00 18 02 ad f7 44 02 ad f7 00 30 00 ce d2 b0 ae c4 e3 63
       txxx:24 54 58 58 58 00 1a 02 ad f7 40 02 ad f7 00 00 00 30 ce d2 b0 ae c4 e3 00 67
 */
unsigned char send_txsq_data[TXSQ_FIRM_SIZE + MAX_PAYLOAD_LEN]; 
#if 0
void send_txsq(unsigned char cmd, unsigned char *src_user_addr, unsigned char *dst_user_addr, 
                             unsigned char transfer_format, unsigned char *send_txsq_payload, unsigned int send_txsq_payload_len)
{
    unsigned int i;
    unsigned char l_transfer_format=0;
    unsigned int payload_len;

    unsigned char l_src_user_addr[3];
    unsigned char l_dst_user_addr[3];
    unsigned char payload[MAX_PAYLOAD_LEN]={0x1,0x2,0x3,0x4,0x5,0x6};//{0xce,0xd2,0xb0,0xae,0xc4,0xe3};//{0x1,0x2,0x3,0x4,0x5,0x6};//
    static unsigned char cnt=0;
	#ifdef SEND
	unsigned long src_user_addr_long=411260;//318776
    unsigned long dst_user_addr_long=318776;
	
	#else
	unsigned long src_user_addr_long=318776;//318776
    unsigned long dst_user_addr_long=411260;
	#endif
	
    if (cmd == 1)
    {
		cnt++;
		payload[0]=cnt;
        //SEGGER_RTT_printf(0,"src_adr=%d ",src_user_addr_long);
        //SEGGER_RTT_printf(0,"dst_adr=%d",dst_user_addr_long);
//		if(l_transfer_format)
			//SEGGER_RTT_printf(0,"tyoe=code\r\n");
//		else
			//SEGGER_RTT_printf(0,"tyoe=chinese\r\n");
#if 1      
        //SEGGER_RTT_printf(0,"playload:%s\r\n",payload);
        payload_len = 6;//strlen((char const *)payload);
#else
        //SEGGER_RTT_printf(0,"\r\n    输入发送内容长度:");
        scanf("%d", &payload_len);
        //payload_len = 78;
        for (i = 0; i < payload_len; ++i)
        {
            payload[i] = 0x5a;
        }
#endif
        l_src_user_addr[0] = src_user_addr_long / 65536;
        l_src_user_addr[1] = (src_user_addr_long % 65536) / 256;
        l_src_user_addr[2] = (src_user_addr_long % 65536) % 256;
        
        l_dst_user_addr[0] = dst_user_addr_long / 65536;
        l_dst_user_addr[1] = (dst_user_addr_long % 65536) / 256;
        l_dst_user_addr[2] = (dst_user_addr_long % 65536) % 256;       

        for (i = 0; i < 3; ++i)
        {
            g_src_user_addr[i] = l_src_user_addr[i];
            g_dst_user_addr[i] = l_src_user_addr[i];
        }       
        g_transfer_format = l_transfer_format;      
        create_txsq(l_src_user_addr, l_dst_user_addr, l_transfer_format, payload, payload_len, send_txsq_data);
		HAL_UART_Transmit_DMA(&huart3, send_txsq_data, (TXSQ_FIRM_SIZE + payload_len));  
    }
    else
    {
        create_txsq(src_user_addr, dst_user_addr, transfer_format, send_txsq_payload, send_txsq_payload_len, send_txsq_data);
		HAL_UART_Transmit_DMA(&huart3,send_txsq_data, (TXSQ_FIRM_SIZE + send_txsq_payload_len));
    }
}
#endif
void send_txsq(unsigned int src_user_addr_long, unsigned int dst_user_addr_long, 
               unsigned char transfer_format, unsigned char *send_txsq_payload, unsigned int send_txsq_payload_len)
{
    unsigned int i;

    unsigned char l_src_user_addr[3];
    unsigned char l_dst_user_addr[3];
	
	l_src_user_addr[0] = src_user_addr_long / 65536;
	l_src_user_addr[1] = (src_user_addr_long % 65536) / 256;
	l_src_user_addr[2] = (src_user_addr_long % 65536) % 256;
	
	l_dst_user_addr[0] = dst_user_addr_long / 65536;
	l_dst_user_addr[1] = (dst_user_addr_long % 65536) / 256;
	l_dst_user_addr[2] = (dst_user_addr_long % 65536) % 256;       
      
	create_txsq(l_src_user_addr, l_dst_user_addr, transfer_format, send_txsq_payload, send_txsq_payload_len, send_txsq_data);
	HAL_UART_Transmit_DMA(&huart3, send_txsq_data, (TXSQ_FIRM_SIZE + send_txsq_payload_len));  
}
void send_cksc()
{
    //todo
}

/*  
    1、IC检测协议内容:
       icjc:24 49 43 4A 43 00 0C 00 00 00 00 2B 	
       icxx:24 49 43 58 58 00 16 02 AD F7 00 00 00 0B 06 00 3C 03 00 00 00 52 
 */
void send_icjc()
{
    unsigned char send_icjc_data[XTZJ_FIRM_SIZE];
    send_icjc_data[0] = '$';  
    send_icjc_data[1] = 'I';
    send_icjc_data[2] = 'C'; 
    send_icjc_data[3] = 'J';
    send_icjc_data[4] = 'C';
    send_icjc_data[5] = ICJC_FIRM_SIZE / 256;  //先传高位
    send_icjc_data[6] = ICJC_FIRM_SIZE % 256; //再传低位
    send_icjc_data[7] = 0x00;
    send_icjc_data[8] = 0x00;
    send_icjc_data[9] = 0x00;
    send_icjc_data[10] = 0x00;
    send_icjc_data[11] = xor_checksum(send_icjc_data, (XTZJ_FIRM_SIZE - 1)); 
	HAL_UART_Transmit_DMA(&huart3,send_icjc_data, XTZJ_FIRM_SIZE);
}
/*  
    1、系统自检协议内容:
       xtzj:24  58 54 5A 4A 00 0D 02 AD FB 00 00 61 	
       zjxx:24 5a 4a 58 58 00 15 02 AD FB 01 00 64 02 00 00 03 00 02 00 13 
 */
void send_xtzj()
{
    unsigned long user_addr;
    unsigned int frequency;
    unsigned char send_xtzj_data[XTZJ_FIRM_SIZE];

    //SEGGER_RTT_printf(0,"\r\n    input user addr(02adf7):");
    scanf("%lx", &user_addr);

    //SEGGER_RTT_printf(0,"\r\n    input freq:");
    scanf("%d", &frequency);
    send_xtzj_data[0] = '$';
    send_xtzj_data[1] = 'X';
    send_xtzj_data[2] = 'T';
    send_xtzj_data[3] = 'Z';
    send_xtzj_data[4] = 'J';
    send_xtzj_data[5] = XTZJ_FIRM_SIZE / 256; //先传高位
    send_xtzj_data[6] = XTZJ_FIRM_SIZE % 256; //再传低位
    send_xtzj_data[7] = user_addr / 65536;
    send_xtzj_data[8] = (user_addr % 65536) / 256;
    send_xtzj_data[9] = (user_addr % 65536) % 256;
    send_xtzj_data[10] = frequency / 256;
    send_xtzj_data[11] = frequency % 256;
    send_xtzj_data[12] = xor_checksum(send_xtzj_data, XTZJ_FIRM_SIZE-1);
	HAL_UART_Transmit_DMA(&huart3,send_xtzj_data, XTZJ_FIRM_SIZE);
}

void send_sjsc()
{
    //todo
}

void send_bbdq()
{
    //todo
}

void parse_txxx(struct txxx_struct *p_txxx,unsigned char *p_txxx_buf)
{
    unsigned int i;
    unsigned int payload_len;
	memset(p_txxx,0,sizeof(struct txxx_struct));
//    unsigned char send_data[104];//用途有2
    /* 1、指令内容 */
    for (i = 0; i < 5; ++i)
    {
        (*p_txxx).instruction[i] = p_txxx_buf[i];
    }
    /* 2、接收包长 */
    (*p_txxx).packet_len = p_txxx_buf[5] * 256 + p_txxx_buf[6];
    /* 3、目的用户地址 */
    for (i = 0; i < 3; ++i)
    {
        (*p_txxx).user_addr[i] = p_txxx_buf[i + 7];
    }
    /* 4.1、信息-信息类别 */
    memcpy(&((*p_txxx).txxx_info.txxx_info_type), (p_txxx_buf + 10), 1);   
    /* 4.2、信息-发送方地址 */
    for (i = 0; i < 3; ++i)
    {
        (*p_txxx).txxx_info.src_user_addr[i] = p_txxx_buf[i + 11];
    }
    /* 4.3、信息-发送时间 */
    (*p_txxx).txxx_info.send_time.hour = p_txxx_buf[14];
    (*p_txxx).txxx_info.send_time.minute = p_txxx_buf[15];
    /* 4.4、信息-电文长度 */
    (*p_txxx).txxx_info.payload_len = p_txxx_buf[16] * 256 + p_txxx_buf[17];
    payload_len = (*p_txxx).txxx_info.payload_len / 8;
    /* 4.5、信息-电文内容 */
    memcpy((*p_txxx).txxx_info.payload, (p_txxx_buf + 18), payload_len);
    /* 4.6、信息-CRC */
    (*p_txxx).txxx_info.crc = p_txxx_buf[18 + payload_len];
    /* 5、校验和 */
    (*p_txxx).checksum = p_txxx_buf[18 + payload_len + 1];

}

void parse_icxx(struct icxx_struct *icxx)
{
//    unsigned int i;
//    for (i = 0; i < 5; ++i)
//    {
//        (*icxx).instruction[i] = icxx_buf[i];
//    }
//    (*icxx).packet_len = icxx_buf[5] * 256 + icxx_buf[6];
//    for (i = 0; i < 3; ++i)
//    {
//        (*icxx).user_addr[i] = icxx_buf[i + 7];
//    }
//    (*icxx).icxx_info.frame_id = icxx_buf[10];
//    for (i = 0; i < 3; ++i)
//    {
//        (*icxx).icxx_info.broadcast_id[i] = icxx_buf[i + 11];
//    }
//    (*icxx).icxx_info.user_feature = icxx_buf[14];
//    (*icxx).icxx_info.service_frequency = icxx_buf[15] * 256 + icxx_buf[16];
//    (*icxx).icxx_info.comm_level = icxx_buf[17];
//    (*icxx).icxx_info.encryption_flag = icxx_buf[18];
//    (*icxx).icxx_info.user_num = icxx_buf[19] * 256 + icxx_buf[20];
//    (*icxx).checksum = icxx_buf[21];
}

void parse_zjxx(struct zjxx_struct *zjxx)
{
//    unsigned int i;
//    for (i = 0; i < 5; ++i)
//    {
//        (*zjxx).instruction[i] = zjxx_buf[i];
//    }
//    (*zjxx).packet_len = zjxx_buf[5] * 256 + zjxx_buf[6];
//    for (i = 0; i < 3; ++i)
//    {
//        (*zjxx).user_addr[i] = zjxx_buf[i + 7];
//    }
//    (*zjxx).zjxx_info.ic_status = zjxx_buf[10];
//    (*zjxx).zjxx_info.hw_status = zjxx_buf[11];
//    (*zjxx).zjxx_info.battery_quantity = zjxx_buf[12];
//    (*zjxx).zjxx_info.in_station_status = zjxx_buf[13];
//    for (i = 0; i < 6; ++i)
//    {
//        (*zjxx).zjxx_info.power_status[i] = zjxx_buf[14 + i];
//    }
//    (*zjxx).checksum = zjxx_buf[20];
}

void parse_fkxx(struct fkxx_struct *fkxx)
{
//    unsigned int i;
//    for (i = 0; i < 5; ++i)
//    {
//        (*fkxx).instruction[i] = fkxx_buf[i];
//    }
//    (*fkxx).packet_len = fkxx_buf[5] * 256 + fkxx_buf[6];
//    for (i = 0; i < 3; ++i)
//    {
//        (*fkxx).user_addr[i] = fkxx_buf[i + 7];
//    }
//    (*fkxx).fkxx_info.fk_flag = fkxx_buf[10];
//    for (i = 0; i < 4; ++i)
//    {
//        (*fkxx).fkxx_info.extra_info[i] = fkxx_buf[11 + i];
//    }
//    (*fkxx).checksum = fkxx_buf[15];
}

void print_txxx(struct txxx_struct *txxx)
{
//    unsigned int i;
//    SEGGER_RTT_printf(0,"\r\n    ========= TXXX_START=========\r\n");
//    SEGGER_RTT_printf(0,"\r\n    TXXX_LEN:%d", (*txxx).packet_len);
//    SEGGER_RTT_printf(0,"\r\n    TXXX_USR_ADDR:0x%02x%02x%02x", (*txxx).user_addr[0], (*txxx).user_addr[1], (*txxx).user_addr[2]);
//    SEGGER_RTT_printf(0,"\r\n    TXXX_ADDR:0x%02x%02x%02x", (*txxx).txxx_info.src_user_addr[0], (*txxx).txxx_info.src_user_addr[1], (*txxx).txxx_info.src_user_addr[2]);
//    SEGGER_RTT_printf(0,"\r\n    TXXX_TIME:%02d:%02d",  (*txxx).txxx_info.send_time.hour,  (*txxx).txxx_info.send_time.minute);
//    SEGGER_RTT_printf(0,"\r\n    TXXX_LEN:%d", (*txxx).txxx_info.payload_len / 8);
//    SEGGER_RTT_printf(0,"\r\n    TXXX_PLAYLOAD:");
//    for (i = 0; i < ((*txxx).txxx_info.payload_len / 8); ++i)
//    {
//        SEGGER_RTT_printf(0,"%02x ", (*txxx).txxx_info.payload[i]);
//    }
//    SEGGER_RTT_printf(0,"\r\n    TXXX_crc:0x%02x", (*txxx).txxx_info.crc);
//    SEGGER_RTT_printf(0,"\r\n    TXXX_checksum:0x%02x", (*txxx).checksum);
//    SEGGER_RTT_printf(0,"\r\n    ========= TXXX_END=========\r\n");
}

void print_icxx(struct icxx_struct *icxx)
{
    //SEGGER_RTT_printf(0,"\r\n    ========= ICXX_START=========\r\n");
    //SEGGER_RTT_printf(0,"\r\n    ICXX_packet_len:%d", (*icxx).packet_len);
    //SEGGER_RTT_printf(0,"\r\n    ICXX_addr:0x%02x%02x%02x", (*icxx).user_addr[0], (*icxx).user_addr[1], (*icxx).user_addr[2]);   
    //SEGGER_RTT_printf(0,"\r\n    ICXX_frame_id:%d", (*icxx).icxx_info.frame_id);   
    //SEGGER_RTT_printf(0,"\r\n    ICXX_broadcast_id:0x%02x%02x%02x", (*icxx).icxx_info.broadcast_id[0], (*icxx).icxx_info.broadcast_id[1], (*icxx).icxx_info.broadcast_id[2]);
    //SEGGER_RTT_printf(0,"\r\n    ICXX_user_feature:%d", (*icxx).icxx_info.user_feature);      
    //SEGGER_RTT_printf(0,"\r\n    ICXX_frequency:%d", (*icxx).icxx_info.service_frequency);   
    //SEGGER_RTT_printf(0,"\r\n    ICXX_comm_level:%d", (*icxx).icxx_info.comm_level);   
    //SEGGER_RTT_printf(0,"\r\n    ICXX_encryption_flag:%d", (*icxx).icxx_info.encryption_flag);   
    //SEGGER_RTT_printf(0,"\r\n    ICXX_user_num:%d", (*icxx).icxx_info.user_num);   
    //SEGGER_RTT_printf(0,"\r\n    ICXX_checksum:0x%02x\r\n", (*icxx).checksum);
    //SEGGER_RTT_printf(0,"\r\n    ========= ICXX_END=========\r\n");
}

void print_zjxx(struct zjxx_struct *zjxx)
{
    //SEGGER_RTT_printf(0,"\r\n    ========= ZJXX_START=========\r\n");
    //SEGGER_RTT_printf(0,"\r\n    ZJXX_LEN:%d", (*zjxx).packet_len);
    //SEGGER_RTT_printf(0,"\r\n    ZJXX_ADDR:0x%02x%02x%02x", (*zjxx).user_addr[0], (*zjxx).user_addr[1], (*zjxx).user_addr[2]);   
    //SEGGER_RTT_printf(0,"\r\n    ZJXX_PLAYLOAD-IC_STAUS:0x%02x", (*zjxx).zjxx_info.ic_status);      
    //SEGGER_RTT_printf(0,"\r\n    ZJXX_PLAYLOAD-HARD:0x%02x", (*zjxx).zjxx_info.hw_status);   
    //SEGGER_RTT_printf(0,"\r\n    ZJXX_PLAYLOAD-DC_POWER:0x%02x", (*zjxx).zjxx_info.battery_quantity);   
    //SEGGER_RTT_printf(0,"\r\n    ZJXX_PLAYLOAD-RZ_STATUS:0x%02x", (*zjxx).zjxx_info.in_station_status);      
    //SEGGER_RTT_printf(0,"\r\n    ZJXX_PLAYLOAD-POWER_STATUS:%d-%d-%d-%d-%d-%d", (*zjxx).zjxx_info.power_status[0], (*zjxx).zjxx_info.power_status[1],\
                          (*zjxx).zjxx_info.power_status[2], (*zjxx).zjxx_info.power_status[3],(*zjxx).zjxx_info.power_status[4], (*zjxx).zjxx_info.power_status[5]);

    //SEGGER_RTT_printf(0,"\r\n    ZJXX_CHECK:0x%02x\r\n", (*zjxx).checksum);
    //SEGGER_RTT_printf(0,"\r\n    ========= ZJXX_END=========\r\n");
}

 

void print_fkxx(struct fkxx_struct *fkxx)
{
//    SEGGER_RTT_printf(0,"\r\n    ========= FKXX_START=========\r\n");
//    SEGGER_RTT_printf(0,"\r\n    FKXX_LEN:%d", (*fkxx).packet_len);
//    SEGGER_RTT_printf(0,"\r\n    FKXX_ADDR:0x%02x%02x%02x", (*fkxx).user_addr[0], (*fkxx).user_addr[1], (*fkxx).user_addr[2]);   
//    SEGGER_RTT_printf(0,"\r\n    FKXX_FLAG:0x%02x", (*fkxx).fkxx_info.fk_flag);    
//    SEGGER_RTT_printf(0,"\r\n    FKXX_INFO:0x%02x-0x%02x-0x%02x-0x%02x", (*fkxx).fkxx_info.extra_info[0], (*fkxx).fkxx_info.extra_info[1], (*fkxx).fkxx_info.extra_info[2], (*fkxx).fkxx_info.extra_info[3]);   
//    SEGGER_RTT_printf(0,"\r\n    FKXX_CHECK:0x%02x\r\n", (*fkxx).checksum);
//    SEGGER_RTT_printf(0,"\r\n    ========= FKXX_END=========\r\n");
 }

void read_bd_rx_info(unsigned char *txxx_buf)
{
//    struct txxx_struct txxx;
//    struct icxx_struct icxx;
//    struct zjxx_struct zjxx;
//    struct fkxx_struct fkxx;

    if (bd_buf_bitmap & FKXX_BUF) 
    {
        parse_fkxx(&fkxx);
      //  print_fkxx(&fkxx);
        bd_buf_bitmap &= ~FKXX_BUF;
    }

//    if (bd_buf_bitmap & ICXX_BUF)
//    {
//        parse_icxx(&icxx);
//        print_icxx(&icxx);
//        bd_buf_bitmap &= ~ICXX_BUF;
//    }
// 
//    if (bd_buf_bitmap & ZJXX_BUF)
//    {
//        parse_zjxx(&zjxx);
//        print_zjxx(&zjxx);
//        bd_buf_bitmap &= ~ZJXX_BUF;
//    }   

    if (bd_buf_bitmap & TXXX_BUF)
    {
        parse_txxx(&txxx,txxx_buf);
//        print_txxx(&txxx);
        bd_buf_bitmap &= ~TXXX_BUF;
    } 
}

unsigned char copy_packet_from_shared_buf(unsigned char *bd_shared_rx_buf)
{
//	if ((bd_shared_rx_buf[1] == 'D') && (bd_shared_rx_buf[2] == 'W')) //收到定位信息$DWXX
//	{
//	   bd_buf_bitmap |= DWXX_BUF;
//	   //memcpy(dwxx_buf, bd_shared_rx_buf, sizeof(dwxx_buf));
//	}
    if (memcmp(bd_shared_rx_buf,"$TXXX",5)==0) //收到通信信息$TXXX
	{
	   bd_buf_bitmap |= TXXX_BUF;
		return 1;
	//	memset(txxx_buf,0,sizeof(txxx_buf));
	//   memcpy(txxx_buf, bd_shared_rx_buf, sizeof(txxx_buf));
	}
//    else if ((bd_shared_rx_buf[1] == 'I') && (bd_shared_rx_buf[2] == 'C')) //收到IC信息$ICXX
//    {
//		bd_buf_bitmap |= ICXX_BUF;
//        memcpy(icxx_buf, bd_shared_rx_buf, sizeof(icxx_buf));
//    }

//	else if ((bd_shared_rx_buf[1] == 'Z') && (bd_shared_rx_buf[2] == 'J')) //收到自检信息$ZJXX
//	{
//		bd_buf_bitmap |= ZJXX_BUF;
//		memcpy(zjxx_buf, bd_shared_rx_buf, sizeof(zjxx_buf));
//	}

//	else if ((bd_shared_rx_buf[1] == 'S') && (bd_shared_rx_buf[2] == 'J')) //收到时间信息$SJXX
//	{
//		bd_buf_bitmap |= SJXX_BUF;
//		//memcpy(sjxx_buf, bd_shared_rx_buf, sizeof(sjxx_buf));
//	}
//	else if ((bd_shared_rx_buf[1] == 'B') && (bd_shared_rx_buf[2] == 'B')) //收到版本信息$BBXX
//	{
//		bd_buf_bitmap |= BBXX_BUF;
//		//memcpy(bbxx_buf, bd_shared_rx_buf, sizeof(bbxx_buf));
//	}
	else if(memcmp(bd_shared_rx_buf,"$FKXX",5)==0) //收到反馈信息$FKXX
	{
		bd_buf_bitmap |= FKXX_BUF;
		return 2;
	//	memcpy(fkxx_buf, bd_shared_rx_buf, sizeof(fkxx_buf));
	}
	return 0;
}
