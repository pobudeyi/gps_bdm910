#ifndef __BSP_CRC
#define __BSP_CRC
unsigned int crcUpdateCcitt(unsigned int crc, unsigned char dat);
unsigned int crcCalculateCcitt(unsigned int preloadValue, const unsigned char* buf, unsigned int length);
#endif
