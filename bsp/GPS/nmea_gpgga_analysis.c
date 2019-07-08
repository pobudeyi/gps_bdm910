#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include"Operation.h"
#include"Type.h"

/******************************************************************************
 *  Function    -   NMEA GPGGA Analysis
 *
 *  Purpose     -  GPGGA命令解析
 *
 *  Description -  GPS定位信息
 *                 $GPGGA,(1),(2),(3),(4),(5),(6),(7),(8),(9),M,(10),M,(11),(12)*hh(CR)(LF)
 *
 $GPGGA，<1>，<2>，<3>，<4>，<5>，<6>，<7>，<8>，<9>，<10>，，<12>,，<14>

字段 $GPGGA语句意义——取值范围
<1> UTC时间：hhmmss.ss——000000.00~235959.99
<2> 纬度，格式：ddmm.mmmm ——0000．00000~8959.9999 （标准的度分格式）
<3> 南北半球——N北纬；S南纬
<4> 经度格式：dddmm.mmmm ——00000.0000~17959.9999 （标准的度分格式）
<5> 东西半球——E表示东经；W表示西经
<6> 质量因子——0=未定位，1=GPS单点定位固定解，2=差分定位，3=PPS解；4=RTK固定解；5=RTK浮点解；6=估计值；7=手工输入模式；8=模拟模式；
<7> 应用解算位置的卫星数——00~12
<8> HDOP，水平图形强度因子——0.500~99.000 ；大于6不可用
<9> 天线高程（海平面）——－9999.9～99999.9
<10> 线线高程单位(m) ——m
大地水准面起伏——地球椭球面相对大地水准面的高度
<12> 大地水准面起伏单位(m)   ——m
<13> 差分GPS数据期——差分时间（从最近一次接收到差分信号开始的秒数，如果不是差分定位将为空），不使用DGPS时为空
<14> 基准站号——0000~1023；不使用DGPS时为空
$GPGGA,074529.82,2429.6717,N,11804.6973,E,1,8,1.098,42.110,M,,M,,*76
 ******************************************************************************/

u8 *NMEA_GPGGA_Analysis(Nmea_msg *gpsx,u8 *buf)
{
	u8 *p1, *node;
    u8 posx;
	u32 temp;
	float rs;
	
    if(strlen((char*)buf) == 0)
    {
        return buf;
    }

	p1 = (u8*)strnstr((char *)buf, "$GPGGA", 6);

	if (p1 == NULL)
	{
		return buf;
	}

	posx = NMEA_Comma_Pos(p1, 1);/*UTC时间*/
    if (posx != 0XFF)
    {
		temp = NMEA_Str2num(p1 + posx) / NMEA_Pow(10, 2);/*UTC时间,hhmmss.ss去掉ms*/
		gpsx->utc.hour = temp / 10000;
		gpsx->utc.mint = (temp / 100) % 100;
		gpsx->utc.sec = temp % 100;
    }
	
	posx = NMEA_Comma_Pos(p1, 2);/*纬度 ddmm.mmmmm(度分)*/

	if (posx != 0XFF)
	{
		temp = NMEA_Str2num(p1 + posx);           /*ddmm.mmmmm*/
		gpsx->latitude = temp / NMEA_Pow(10, 7);  /*得到°*/
		rs = (temp / 10000) % 100;                /*得到'*/
		gpsx->latitude = gpsx->latitude + rs / 60;/*转换为°*/
	}

	posx = NMEA_Comma_Pos(p1, 3);/*南北纬*/

	if (posx != 0XFF)
		gpsx->nshemi = *(p1 + posx);
	
	posx = NMEA_Comma_Pos(p1, 4);/*经度*/
	if (posx != 0XFF)
	{
		temp = NMEA_Str2num(p1 + posx);             /*dddmm.mmmmm*/
		gpsx->longitude = temp / NMEA_Pow(10, 7);   /*得到°*/
		rs = (temp / 10000) % 100;                  /*得到'*/
		gpsx->longitude = gpsx->longitude + rs / 60;/*转换为°*/
	}

	posx = NMEA_Comma_Pos(p1, 5);/*东西经*/

	if (posx != 0XFF)
		gpsx->ewhemi = *(p1 + posx);
	posx = NMEA_Comma_Pos(p1, 6);/*得到GPS状态*/

	if (posx != 0XFF)
		gpsx->gpssta = NMEA_Str2num(p1 + posx);

	posx = NMEA_Comma_Pos(p1, 7);/*用于定位的卫星数*/

	if (posx != 0XFF)
		gpsx->posslnum = NMEA_Str2num(p1 + posx);

	posx = NMEA_Comma_Pos(p1, 8);/*HDOP位置精度因子*/

	if (posx != 0XFF)
		gpsx->hdop = (float)NMEA_Str2num(p1 + posx) / 100;

	posx = NMEA_Comma_Pos(p1, 9);/*海拔高度*/

	if (posx != 0XFF)
		gpsx->altitude = (float)NMEA_Str2num(p1 + posx) / 10;

	posx = NMEA_End_Pos(p1);/*14*/
	node = p1 + posx + 1;
	return node;
}
