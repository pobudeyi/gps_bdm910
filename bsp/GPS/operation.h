#ifndef _OPERATION_H_
#define _OPERATION_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "type.h"

u32 NMEA_Pow(u8 m,u8 n);
u8 NMEA_Comma_Pos(u8 *buf,u8 cx);
u8 NMEA_End_Pos(u8 *buf);
int NMEA_Str2num(u8 *buf);
char *strnstr(char *s1, const char *s2, int pos);
int NMEA_Str2hex2num(char *tail);
int NMEA_Checkout(char *str);
#endif // _OPERATION_H_
