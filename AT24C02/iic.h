#ifndef __IIC_H
#define __IIC_H

#include "stm32f4xx.h"
#include "sys.h"


/*
Òý½ÅËµÃ÷
SCL -- PB8
SDA -- PB9

*/


#define AT_SCL  	PBout(8)
#define AT_SDA_IN	PBin(9)
#define AT_SDA_OUT	PBout(9)


void Iic_Init(void);
void AT24c02_Write(u8 addr, u8 *write_buff, u8 len);
void AT24c02_Read(u8 addr, u8 *read_buff, u8 len);

#endif