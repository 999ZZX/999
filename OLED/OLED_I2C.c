/************************************************************************************
*  Copyright (c), 2014, HelTec Automatic Technology co.,LTD.
*            All rights reserved.
*
* Http:    www.heltec.cn
* Email:   cn.heltec@gmail.com
* WebShop: heltec.taobao.com
*
* File name: OLED_I2C.c
* Project  : HelTec.uvprij
* Processor: STM32F103C8T6
* Compiler : MDK fo ARM
* 
* Author : 小林
* Version: 1.00
* Date   : 2014.4.8
* Email  : hello14blog@gmail.com
* Modification: none
* 
* Description:128*64点阵的OLED显示屏驱动文件，仅适用于惠特自动化(heltec.taobao.com)的SD1306驱动IIC通信方式显示屏
*
* Others: none;
*
* Function List:
*	1. void I2C_Configuration(void) -- 配置CPU的硬件I2C
* 2. void I2C_WriteByte(uint8_t addr,uint8_t data) -- 向寄存器地址写一个byte的数据
* 3. void WriteCmd(unsigned char I2C_Command) -- 写命令
* 4. void WriteDat(unsigned char I2C_Data) -- 写数据
* 5. void OLED_Init(void) -- OLED屏初始化
* 6. void OLED_SetPos(unsigned char x, unsigned char y) -- 设置起始点坐标
* 7. void OLED_Fill(unsigned char fill_Data) -- 全屏填充
* 8. void OLED_CLS(void) -- 清屏
* 9. void OLED_ON(void) -- 唤醒
* 10. void OLED_OFF(void) -- 睡眠
* 11. void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize) -- 显示字符串(字体大小有6*8和8*16两种)
* 12. void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N) -- 显示中文(中文需要先取模，然后放到codetab.h中)
* 13. void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]) -- BMP图片
*
* History: none;
*
*************************************************************************************/

#include "OLED_I2C.h"
#include "delay.h"
#include "codetab.h"
#include <string.h>

extern u8 OLED_GRAM[128][8];

void I2C_Configuration(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure; 

	//使能E 组时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	/*STM32F407芯片的硬件I2C: PE8 -- OLED_SCL; PE10 -- SDA */
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;  //上拉电阻
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD; //开漏输出
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//I2C必须开漏输出
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_1 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;  //上拉电阻
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //输出
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//I2C必须开漏输出
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	//总线空闲
	OLED_SCL = 1;
	OLED_SDA_OUT = 1;
	
	PDout(1) = 1;
	PDout(15) = 0;
}

//引脚模式变更--SDA
void OLED_Iic_Sda_Mode(GPIOMode_TypeDef mode)
{
	GPIO_InitTypeDef  	GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;				//第9号引脚
	GPIO_InitStructure.GPIO_Mode  = mode;					//输入/输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;			//推挽输出，增强驱动能力，引脚的输出电流更大
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//引脚的速度最大为100MHz
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;		//没有使用内部上拉电阻
	GPIO_Init(GPIOE, &GPIO_InitStructure);	
	
	
}

void OLED_Iic_Start(void)
{
	OLED_Iic_Sda_Mode(GPIO_Mode_OUT);
	//总线空闲
	OLED_SCL = 1;
	OLED_SDA_OUT = 1;
	delay_us(5);
	
	OLED_SDA_OUT = 0;
	delay_us(5);
	OLED_SCL = 0;     //钳住线
}

//停止信号
void OLED_Iic_Stop(void)
{
	OLED_Iic_Sda_Mode(GPIO_Mode_OUT);

	OLED_SCL = 0;
	OLED_SDA_OUT = 0;
	delay_us(5);
	
	OLED_SCL = 1;
	delay_us(5);
	OLED_SDA_OUT = 1;
}

//发送一位数据
void OLED_Iic_Send_Ack(u8 ack)
{
	OLED_Iic_Sda_Mode(GPIO_Mode_OUT);
	OLED_SCL = 0;
	//准备数据
	if(ack == 1)
	{
		OLED_SDA_OUT = 1;//引脚输出1
	}
	if(ack == 0)
	{
		OLED_SDA_OUT = 0;//引脚输出0
	}	
	
	delay_us(5);
	OLED_SCL = 1;
	delay_us(5);
	OLED_SCL = 0;

}

//发送一个字节数据
void OLED_Iic_Send_Byte(u8 data)
{
	u8 i;
	OLED_Iic_Sda_Mode(GPIO_Mode_OUT);
	
	OLED_SCL = 0;
	for(i=0; i<8; i++)
	{
		//将数据拆位，先发高位  0x87  1000 0111
		
		if(data & (1<<(7-i)))
		{
			OLED_SDA_OUT = 1;//引脚输出1
		}
		else
		{
			OLED_SDA_OUT = 0;//引脚输出0		
		}
		delay_us(5);
		OLED_SCL = 1;
		delay_us(5);
		OLED_SCL = 0;	
	}
	
}
//接受一位数据 STM32做为输入
u8 OLED_Iic_Rcv_Ack(void)
{
	u8  ack;
	OLED_Iic_Sda_Mode(GPIO_Mode_IN);
	
	OLED_SCL = 0;
	delay_us(5);
	OLED_SCL = 1;
	delay_us(5);
	
	if(OLED_SDA_IN == 1)
	{
		ack = 1;
	}
	else
	{
		ack = 0;
	}
	
	OLED_SCL = 0;	

	return ack;
}



void OLED_I2C_WriteByte(uint8_t addr,uint8_t data)
{
	u8 ack;
	while(OLED_SCL !=1 || OLED_SDA_OUT !=1);
	
	OLED_Iic_Start();
	//发送地址，并执行写操作
	OLED_Iic_Send_Byte(OLED_ADDRESS);
	ack = OLED_Iic_Rcv_Ack();
	if(ack == 1)
	{
		printf("write failure\n");
		OLED_Iic_Stop();
		return;
	}
	
	//发送写数据的起始地址
	OLED_Iic_Send_Byte(addr);
	ack = OLED_Iic_Rcv_Ack();
	if(ack == 1)
	{
		printf("write failure\n");
		OLED_Iic_Stop();
		return;
	}	
	
	//发送数据
	OLED_Iic_Send_Byte(data);
	ack = OLED_Iic_Rcv_Ack();
	if(ack == 1)
	{
		printf("write failure\n");
		OLED_Iic_Stop();
		return;
	}			

	OLED_Iic_Stop();
}

void OLED_WriteCmd(unsigned char I2C_Command)//写命令
{
	OLED_I2C_WriteByte(0x00, I2C_Command);
}

void OLED_WriteDat(unsigned char I2C_Data)//写数据
{
	OLED_I2C_WriteByte(0x40, I2C_Data);
}

void OLED_Init(void)
{
	
	delay_ms(100); //这里的延时很重要
	
	OLED_WriteCmd(0xAE); //display off
	OLED_WriteCmd(0x20);	//Set Memory Addressing Mode	
	OLED_WriteCmd(0x10);	//00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	OLED_WriteCmd(0xb0);	//Set Page Start Address for Page Addressing Mode,0-7
	OLED_WriteCmd(0xc8);	//Set COM Output Scan Direction
	OLED_WriteCmd(0x00); //---set low column address
	OLED_WriteCmd(0x10); //---set high column address
	OLED_WriteCmd(0x40); //--set start line address
	OLED_WriteCmd(0x81); //--set contrast control register
	OLED_WriteCmd(0xff); //亮度调节 0x00~0xff
	OLED_WriteCmd(0xa1); //--set segment re-map 0 to 127
	OLED_WriteCmd(0xa6); //--set normal display
	OLED_WriteCmd(0xa8); //--set multiplex ratio(1 to 64)
	OLED_WriteCmd(0x3F); //
	OLED_WriteCmd(0xa4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	OLED_WriteCmd(0xd3); //-set display offset
	OLED_WriteCmd(0x00); //-not offset
	OLED_WriteCmd(0xd5); //--set display clock divide ratio/oscillator frequency
	OLED_WriteCmd(0xf0); //--set divide ratio
	OLED_WriteCmd(0xd9); //--set pre-charge period
	OLED_WriteCmd(0x22); //
	OLED_WriteCmd(0xda); //--set com pins hardware configuration
	OLED_WriteCmd(0x12);
	OLED_WriteCmd(0xdb); //--set vcomh
	OLED_WriteCmd(0x20); //0x20,0.77xVcc
	OLED_WriteCmd(0x8d); //--set DC-DC enable
	OLED_WriteCmd(0x14); //
	OLED_WriteCmd(0xaf); //--turn on oled panel
}

void OLED_SetPos(unsigned char x, unsigned char y) //设置起始点坐标
{ 
	OLED_WriteCmd(0xb0+y);
	OLED_WriteCmd(((x&0xf0)>>4)|0x10);
	OLED_WriteCmd((x&0x0f)|0x01);
}

void OLED_Fill(unsigned char fill_Data)//全屏填充
{
	unsigned char m,n;
	for(m=0;m<8;m++)
	{
		OLED_WriteCmd(0xb0+m);		//page0-page1
		OLED_WriteCmd(0x00);		//low column start address
		OLED_WriteCmd(0x10);		//high column start address
		for(n=0;n<128;n++)
			{
				OLED_WriteDat(fill_Data);
			}
	}
}

void OLED_CLS(void)//清屏
{
	OLED_Fill(0x00);
}

//--------------------------------------------------------------
// Prototype      : void OLED_ON(void)
// Calls          : 
// Parameters     : none
// Description    : 将OLED从休眠中唤醒
//--------------------------------------------------------------
void OLED_ON(void)
{
	OLED_WriteCmd(0X8D);  //设置电荷泵
	OLED_WriteCmd(0X14);  //开启电荷泵
	OLED_WriteCmd(0XAF);  //OLED唤醒
}

//--------------------------------------------------------------
// Prototype      : void OLED_OFF(void)
// Calls          : 
// Parameters     : none
// Description    : 让OLED休眠 -- 休眠模式下,OLED功耗不到10uA
//--------------------------------------------------------------
void OLED_OFF(void)
{
	OLED_WriteCmd(0X8D);  //设置电荷泵
	OLED_WriteCmd(0X10);  //关闭电荷泵
	OLED_WriteCmd(0XAE);  //OLED休眠
}

//--------------------------------------------------------------
// Prototype      : void OLED_ShowChar(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
// Calls          : 
// Parameters     : x,y -- 起始点坐标(x:0~127, y:0~7); ch[] -- 要显示的字符串; TextSize -- 字符大小(1:6*8 ; 2:8*16)
// Description    : 显示codetab.h中的ASCII字符,有6*8和8*16可选择
//--------------------------------------------------------------
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
{
	unsigned char c = 0,i = 0,j = 0;
	switch(TextSize)
	{
		case 1:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 126)
				{
					x = 0;
					y++;
				}
				OLED_SetPos(x,y);
				for(i=0;i<6;i++)
					OLED_WriteDat(F6x8[c][i]);
				x += 6;
				j++;
			}
		}break;
		case 2:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 120)
				{
					x = 0;
					y++;
				}
				OLED_SetPos(x,y);
				for(i=0;i<8;i++)
					OLED_WriteDat(F8X16[c*16+i]);
				OLED_SetPos(x,y+1);
				for(i=0;i<8;i++)
					OLED_WriteDat(F8X16[c*16+i+8]);
				x += 8;
				j++;
			}
		}break;
	}
}

//--------------------------------------------------------------
// Prototype      : void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N)
// Calls          : 
// Parameters     : x,y -- 起始点坐标(x:0~127, y:0~7); N:汉字在codetab.h中的索引
// Description    : 显示codetab.h中的汉字,16*16点阵
//--------------------------------------------------------------
void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N)
{
	unsigned char wm=0;
	unsigned int  adder=32*N;
	OLED_SetPos(x , y);
	for(wm = 0;wm < 16;wm++)
	{
		OLED_WriteDat(F16x16[adder]);
		adder += 1;
	}
	OLED_SetPos(x,y + 1);
	for(wm = 0;wm < 16;wm++)
	{
		OLED_WriteDat(F16x16[adder]);
		adder += 1;
	}
}

//--------------------------------------------------------------
// Prototype      : void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]);
// Calls          : 
// Parameters     : x0,y0 -- 起始点坐标(x0:0~127, y0:0~7); x1,y1 -- 起点对角线(结束点)的坐标(x1:1~128,y1:1~8)
// Description    : 显示BMP位图
//--------------------------------------------------------------
void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[])
{
	unsigned int j=0;
	unsigned char x,y;

  if(y1%8==0)
		y = y1/8;
  else
		y = y1/8 + 1;
	for(y=y0;y<y1;y++)
	{
		OLED_SetPos(x0,y);
    for(x=x0;x<x1;x++)
		{
			OLED_WriteDat(BMP[j++]);
		}
	}
}
void OLED_Refresh_Gram(void)
{
	u8 i,n;		    
	for(i=0;i<8;i++)  
	{  
		OLED_WriteCmd (0xb0+i);    //设置页地址（0~7）
		OLED_WriteCmd (0x00);      //设置显示位置—列低地址
		OLED_WriteCmd (0x10);      //设置显示位置—列高地址   
		for(n=0;n<128;n++)OLED_WriteDat(OLED_GRAM[n][i]); 
	}   
}


void OLED_DrawPoint(u8 x,u8 y,u8 t)
{
	u8 pos,bx,temp=0;
	if(x>127||y>63)return;//超出范围了.
	pos=7-y/8;
	bx=y%8;
	temp=1<<(7-bx);
	if(t)OLED_GRAM[x][pos]|=temp;
	else OLED_GRAM[x][pos]&=~temp;	 
}

