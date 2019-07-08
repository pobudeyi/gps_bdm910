//CRC
unsigned int crcUpdateCcitt(unsigned int crc, unsigned char dat)
{
    dat ^= ((unsigned char)crc) & 0xFF;
    dat ^= dat << 4;

    crc = (crc >> 8)^(((unsigned int) dat) << 8)^(((unsigned int) dat) << 3)^(((unsigned int) dat) >> 4);

    return crc;
}

unsigned int crcCalculateCcitt(unsigned int preloadValue, const unsigned char* buf, unsigned int length)
{
    unsigned int crc = preloadValue;
    unsigned int index;

    for (index = 0; index < length; index++)
    {
        crc = crcUpdateCcitt(crc, buf[index]);
    }

    return crc;
}
