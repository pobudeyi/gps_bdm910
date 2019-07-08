#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "type.h"
/******************************************************************************
 *  Function    -  Power calculation
 *
 *  Purpose     -  十进制转换倍率
 *
 *  Description -  计算m的n次方
 *
 ******************************************************************************/

u32 NMEA_Pow(u8 m,u8 n)
{
    u32 result=1;
    while(n--){result*=m;}
    return result;
}

/******************************************************************************
 *  Function    -  Relative address offset
 *
 *  Purpose     -  计算第cx个逗号的相对地址偏移量

 *  Description -  buf为字段的起始地址
 *
 ******************************************************************************/

u8 NMEA_Comma_Pos(u8 *buf,u8 cx)
{
    u8 *p=buf;
    while(cx)
    {
        if(*buf=='*'||*buf<' '||*buf>'z')/*排除*和非法字符*/
        {
            return 0XFF;
        }
        if(*buf==','){cx--;}
        buf++;
    }
    return buf-p;/*相对地址偏移量*/
}

/******************************************************************************
 *  Function    -  Relative address offset
 *
 *  Purpose     -  计算每个语句结束符*的相对地址偏移量
 *
 *  Description -  buf为字段的起始地址
 ******************************************************************************/

u8 NMEA_End_Pos(u8 *buf)
{
	u8 *p = buf;
	while (*buf != '\n')
	{
		if (*buf<' ' || *buf>'z')/*排除非法字符*/
		{
			return 0XFF;
		}
		buf++;
	}
	return buf-p;/*相对地址偏移量*/
}

/******************************************************************************
 *  Function    -  Numeric character to digit
 *
 *  Purpose     -  数字字符转数字
 *
 *  Description -  buf为数字起始地址
 *
 ******************************************************************************/
int NMEA_Str2num(u8 *buf)
{
    u8 *p=buf;/*buf为数字起始地址*/
    u8 i,j,mask=0;/*mask作为负数号与小数点的验证标志*/
    u8 ilen=0,flen=0;/*整数长和小数长*/
    u32 ires=0,fres=0;/*整数和小数*/
    int res;
    while((*p!=',')&&(*p!='*')&&(*p!=' '))
    {
		/*if(*p=='-'){mask|=0x02;p++;}*//*进行按位或,mask=0x02是负数*/
        if(*p=='.'){mask|=0x01;p++;}/*小数点*/
        else if(*p>'9'||(*p<'0'))
        {
            ilen = 0;
            flen = 0;
            break;
        }
        if(mask&0x01)
		{
			flen++;
			p++;
		}/*计数小数的位数*/
        else
		{
			ilen++;
			p++;
		}/*起始检测存在数字*/
    }

	/*if(mask&0x02){flen++;}*//*负数符号位*/
    for(i=0;i<ilen;i++)
    {
        ires+=NMEA_Pow(10,ilen-1-i)*(buf[i]-'0');/*buf为数字起始地址*/
    }

    if(flen>6)
		flen=6;/*限定6位小数*/
    for(j=0;j<flen;j++)
    {
        fres+=NMEA_Pow(10,flen-1-j)*(buf[ilen+1+j]-'0');/*加多1位小数点偏移量*/
    }
    res = ires*NMEA_Pow(10,flen)+fres;
    /*if(mask&0x02)res=-res;*//*负数的情况*/
    return res;
}

/******************************************************************************
 *  Function    -  Search comparison string
 *
 *  Purpose     -  搜索对比s1与s2字符串前n位
 *
 *  Description -  搜索对比s1与s2字符串前n位
 *                 相同则返回s2在s1中出现的首地址
 *                 不同则返回NULL
 *
 ******************************************************************************/
char *strnstr(char *s1, const char *s2, int pos)
{
	int len1, len2;
	len1 = strlen(s1);
	len2 = strlen(s2);
	if (!len2)
		return (char *)s1;
	pos = (pos > len1) ? len1 : pos;
	while (pos > len2)
	{
		pos--;
	}

	if (!memcmp(s1, s2, pos))
		return (char *)s1;
	else
		return NULL;
}

 

/******************************************************************************
 *  Function    -  character to hex to integer
 *
 *  Purpose     -  字符转十六进制转整型
 *
 *  Description -  字符转整型
 *
 ******************************************************************************/
int NMEA_Str2hex2num(char *tail)
{
	int digit, step, value;
	int num = 0;

	digit = strlen(tail);

    if(digit > 3)
    {
        digit = 2;
    }

	for (step = 0; step < digit; step++)
	{
		switch (tail[step])
		{
			case 'A':value = 10; break;
			case 'B':value = 11; break;
			case 'C':value = 12; break;
			case 'D':value = 13; break;
			case 'E':value = 14; break;
			case 'F':value = 15; break;
			default :value = tail[step] - '0'; break;
		}
		num = num + value * NMEA_Pow(16, (digit - step - 1));
	}
	return num;
}

 

/*****************************************************************************
 *  Function    -  NMEA Check bit calculation check
 *
 *  Purpose     -  NMEA校验位计算检验
 *
 *  Description -  按位异或计算'$'至'*'之间的字符
 *
 ******************************************************************************/
int NMEA_Checkout(char *str)
{
	int i, result, value, avail;
	char *tail;
	tail = strstr(str, "*") + 1;
	value = NMEA_Str2hex2num(tail);

	for (result = str[1], i = 2; str[i] != '*'; i++)
	{
		result ^= str[i];
	}

	if (result == value)
		avail = 1;
	else
		avail = 0;
	return avail;
}

