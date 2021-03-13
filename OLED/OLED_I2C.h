#ifndef __OLED_I2C_H
#define	__OLED_I2C_H

#include "stm32f4xx.h"
#include "sys.h"


#define OLED_SCL  		PEout(8)
#define OLED_SDA_IN		PEin(10)
#define OLED_SDA_OUT	PEout(10)
#define OLED_ADDRESS	0x78 //通过调整0R电阻,屏可以0x78和0x7A两个地址 -- 默认0x78

void OLED_DrawPoint(u8 x,u8 y,u8 t);
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 size,u8 mode);
void I2C_Configuration(void);
void OLED_Refresh_Gram(void);


void OLED_I2C_WriteByte(uint8_t addr,uint8_t data);
void OLED_WriteCmd(unsigned char I2C_Command);
void OLED_WriteDat(unsigned char I2C_Data);
void OLED_Init(void);
void OLED_SetPos(unsigned char x, unsigned char y);
void OLED_Fill(unsigned char fill_Data);
void OLED_CLS(void);
void OLED_ON(void);
void OLED_OFF(void);
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize);
void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N);
void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]);

#endif
