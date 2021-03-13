#ifndef __IIC_H
#define __IIC_H

#include "stm32f4xx.h"
#include "sys.h"


/*
Òý½ÅËµÃ÷
SCL -- PB8
SDA -- PB9

*/


#define SCL  	PBout(8)
#define SDA_IN	PBin(9)
#define SDA_OUT	PBout(9)


void Iic_Init(void);
void Iic_Sda_Mode(GPIOMode_TypeDef mode);
void Iic_Start(void);
void Iic_Stop(void);
void Iic_Send_Ack(u8 ack);
void Iic_Send_Byte(u8 data);
u8 Iic_Rcv_Ack(void);
u8 Iic_Rcv_Byte(void);
void AT24c02_Write(u8 addr, u8 *write_buff, u8 len);
void AT24c02_Read(u8 addr, u8 *read_buff, u8 len);

#endif