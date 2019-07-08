#ifndef _PACKAGE_H_
#define _PACKAGE_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include "Type.h"
#include "Operation.h"

int Package(char *state)
{
    int signal;
    static u32 time;
    static u32 date;
    static u32 htime;
    static u32 hdate;
    static int avail = 0;/*数据包有效性*/
    static int packg = 0;/*封包标志*/
    u8 step;
    if(strstr(state,"$GNRMC"))
    {
        step = NMEA_Comma_Pos((u8*)state, 1);
        time = NMEA_Str2num((u8*)state + step) / 100;
        step = NMEA_Comma_Pos((u8*)state, 9);
        date = NMEA_Str2num((u8*)state + step);
    }
    if(strstr(state,"$GNVTG"))
    {
    }

    if(strstr(state,"$GNGGA"))
    {
        step = NMEA_Comma_Pos((u8*)state, 1);
        time = NMEA_Str2num((u8*)state + step) / 100;
    }

    if(strstr(state,"$GNGSA"))
    {
    }

    if(strstr(state,"$GPGSV"))
    {
    }

    if(strstr(state,"$GNGLL"))
    {
        step = NMEA_Comma_Pos((u8*)state, 5);
        time = NMEA_Str2num((u8*)state + step) / 100;
    }

    if(strstr(state,"$GNZDA"))
    {
        step = NMEA_Comma_Pos((u8*)state, 1);
        time = NMEA_Str2num((u8*)state + step) / 100;
        step = NMEA_Comma_Pos((u8*)state, 2);
		date = date + NMEA_Str2num((u8*)state + step)*10000;
	    step = NMEA_Comma_Pos((u8*)state, 3);
		date = date + NMEA_Str2num((u8*)state + step)*100;
	    step = NMEA_Comma_Pos((u8*)state, 4);
		date = date + NMEA_Str2num((u8*)state + step);
    }

 

    if ((time != 0) && (date != 0))/*存在有效时间信息*/
    {
        avail = 1;
        if((time != htime)||(date != hdate))/*存在时间信息更新*/
        {
            packg = 1;/*新包开始,旧包结束*/
            htime = time;
            hdate = date;
        }
        else/*时间信息没有更新*/
        {
            packg = 0;/*同数据包*/
        }
    }
    else
    {
        avail = 0;/*无效数据*/
    }

    if(!avail)
    {
        signal = 0;
    }

    else
    {
        if(!packg)/*0同数据包*/
        {
            signal = 1;
        }
        else/*1新数据包*/
        {
            signal = 2;
        }
    }
    return signal;
}

#endif // _PACKAGE_H_
