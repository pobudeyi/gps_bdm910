#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "Operation.h"
#include "type.h"

/******************************************************************************
 字段 $GPRMC语句意义——取值范围
<1> UTC时间：hhmmss.ss——000000.00~235959.99
<2> 状态，有效性 ——A表示有效；V表示无效
<3> 纬度格式：ddmm.mmmm——0000．00000~8959.9999 （标准的度分格式）
<4> 南北半球——N北纬；S南纬
<5> 经度格式：dddmm.mmmm——00000.0000~17959.9999 （标准的度分格式）
<6> 东西半球——E表示东经；W表示西经
<7> 地面速度——000.00~999.999
<8> 速度方向——000.00~359.99
<9> 日期格式，月日年——010100~123199
<10> 磁偏角，单位：度——00.00~99.99
磁偏角方向——E表示东；W表示西
<12> 模式指示及校验和—— A=自主定位，D=差分，E=估算，N=数据无效
例如：$GPRMC,074529.82,A,2429.6717,N,11804.6973,E,12.623,32.122,010806,,W,A*08
	  $BDRMC,064457.90,A,3110.4691241,N,12123.2667666,E,0.157,63.0,300713,0.0,W,A*05 
 ******************************************************************************/

u8 *NMEA_GPRMC_Analysis(Nmea_msg *gpsx,u8 *buf)
{
	u8 *p1, *node;
    u8 posx,posy;
    u32 temp;/*32位变量读取时间数据*/
	char temp_buff[16];
    float rs;

    if(strlen((char*)buf) == 0)
    {
        return buf;
    }

	p1 = (u8*)strnstr((char *)buf, "$BDRMC", 6);
	if (p1 == NULL)
	{
		return buf;
	}
	posx = NMEA_Comma_Pos(p1, 1);/*UTC时间*/
    if (posx != 0XFF)
    {
		temp = NMEA_Str2num(p1 + posx) / NMEA_Pow(10, 3);/*UTC时间,hhmmss.ss去掉ms*/
		gpsx->utc.hour = temp / 10000;
		gpsx->utc.mint = (temp / 100) % 100;
		gpsx->utc.sec = temp % 100;
    }

	posx = NMEA_Comma_Pos(p1, 2);/*有无效用定位*/
	if (posx != 0XFF)
		gpsx->avhemi = *(p1 + posx);
	
	posx = NMEA_Comma_Pos(p1, 3);/*纬度 ddmm.mmmm(度分)*/
	
	posy = NMEA_Comma_Pos(p1, 4);
	if(posy-posx>0)
	{
		memset(temp_buff,0,sizeof(temp_buff));
		memcpy(temp_buff,p1+posx,posy-posx-1);
		//gpsx->latitude=atof(temp_buff);
		sscanf((char *)temp_buff, "%lf", &gpsx->latitude);
	}
//    if (posx != 0XFF)
//    {
//		temp = NMEA_Str2num(p1 + posx);           /*ddmm.mmmmm*/
//		gpsx->latitude = temp / NMEA_Pow(10, 6);  /*得到°*/
//		rs = (temp / 10000) % 100;                /*得到'*/
//		gpsx->latitude = gpsx->latitude + rs / 60;/*转换为°*/
//    }

	posx = NMEA_Comma_Pos(p1, 4);/*南北纬*/
	if (posx != 0XFF)
		gpsx->nshemi = *(p1 + posx);
	
	posx = NMEA_Comma_Pos(p1, 5);/*经度*/
	posy = NMEA_Comma_Pos(p1, 6);
	if(posy-posx>0)
	{
		memset(temp_buff,0,sizeof(temp_buff));
		memcpy(temp_buff,p1+posx,posy-posx-1);
		//gpsx->longitude=atof(temp_buff);
		sscanf((char *)temp_buff, "%lf", &gpsx->longitude);
	}
//    if (posx != 0XFF)
//    {
//		temp = NMEA_Str2num(p1 + posx);             /*dddmm.mmmmm*/
//		gpsx->longitude = temp / NMEA_Pow(10, 6);   /*得到°*/
//		rs = (temp / 10000) % 100;                  /*得到'*/
//		gpsx->longitude = gpsx->longitude + rs / 60;/*转换为°*/
//    }

	posx = NMEA_Comma_Pos(p1, 6);/*东西经*/
	if (posx != 0XFF)
		gpsx->ewhemi = *(p1 + posx);
	
	posx = NMEA_Comma_Pos(p1, 7);/*地面速率(航节)*/
	posy = NMEA_Comma_Pos(p1, 8);
//    if (posx != 0XFF)

//    {
//		temp = NMEA_Str2num(p1 + posx);
//		gpsx->speed = (float)temp / 1000 * 1.852;
//    }
	if(posy-posx>0)
	{
		memset(temp_buff,0,sizeof(temp_buff));
		memcpy(temp_buff,p1+posx,posy-posx-1);
		//gpsx->longitude=atof(temp_buff);
		sscanf((char *)temp_buff, "%f", &gpsx->speed);
	}

	posx = NMEA_Comma_Pos(p1, 8);/*地北航向角度*/

	if (posx != 0XFF)
		gpsx->entude = NMEA_Str2num(p1 + posx);

	posx = NMEA_Comma_Pos(p1, 9);/*UTC日期*/
    if (posx != 0XFF)
    {
		temp = NMEA_Str2num(p1 + posx);
		gpsx->utc.date = temp / 10000;
		gpsx->utc.month = (temp / 100) % 100;
		gpsx->utc.year = temp % 100;
    }

	posx = NMEA_End_Pos(p1);/*12*/
	node = p1 + posx + 1;
	return node;
}
