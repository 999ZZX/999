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
* Author : С��
* Version: 1.00
* Date   : 2014.4.8
* Email  : hello14blog@gmail.com
* Modification: none
* 
* Description:128*64�����OLED��ʾ�������ļ����������ڻ����Զ���(heltec.taobao.com)��SD1306����IICͨ�ŷ�ʽ��ʾ��
*
* Others: none;
*
* Function List:
*	1. void I2C_Configuration(void) -- ����CPU��Ӳ��I2C
* 2. void I2C_WriteByte(uint8_t addr,uint8_t data) -- ��Ĵ�����ַдһ��byte������
* 3. void WriteCmd(unsigned char I2C_Command) -- д����
* 4. void WriteDat(unsigned char I2C_Data) -- д����
* 5. void OLED_Init(void) -- OLED����ʼ��
* 6. void OLED_SetPos(unsigned char x, unsigned char y) -- ������ʼ������
* 7. void OLED_Fill(unsigned char fill_Data) -- ȫ�����
* 8. void OLED_CLS(void) -- ����
* 9. void OLED_ON(void) -- ����
* 10. void OLED_OFF(void) -- ˯��
* 11. void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize) -- ��ʾ�ַ���(�����С��6*8��8*16����)
* 12. void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N) -- ��ʾ����(������Ҫ��ȡģ��Ȼ��ŵ�codetab.h��)
* 13. void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]) -- BMPͼƬ
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

	//ʹ��E ��ʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	/*STM32F407оƬ��Ӳ��I2C: PE8 -- OLED_SCL; PE10 -- SDA */
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;  //��������
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD; //��©���
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//I2C���뿪©���
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_1 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;  //��������
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//I2C���뿪©���
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	//���߿���
	OLED_SCL = 1;
	OLED_SDA_OUT = 1;
	
	PDout(1) = 1;
	PDout(15) = 0;
}

//����ģʽ���--SDA
void OLED_Iic_Sda_Mode(GPIOMode_TypeDef mode)
{
	GPIO_InitTypeDef  	GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;				//��9������
	GPIO_InitStructure.GPIO_Mode  = mode;					//����/���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;			//�����������ǿ�������������ŵ������������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//���ŵ��ٶ����Ϊ100MHz
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;		//û��ʹ���ڲ���������
	GPIO_Init(GPIOE, &GPIO_InitStructure);	
	
	
}

void OLED_Iic_Start(void)
{
	OLED_Iic_Sda_Mode(GPIO_Mode_OUT);
	//���߿���
	OLED_SCL = 1;
	OLED_SDA_OUT = 1;
	delay_us(5);
	
	OLED_SDA_OUT = 0;
	delay_us(5);
	OLED_SCL = 0;     //ǯס��
}

//ֹͣ�ź�
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

//����һλ����
void OLED_Iic_Send_Ack(u8 ack)
{
	OLED_Iic_Sda_Mode(GPIO_Mode_OUT);
	OLED_SCL = 0;
	//׼������
	if(ack == 1)
	{
		OLED_SDA_OUT = 1;//�������1
	}
	if(ack == 0)
	{
		OLED_SDA_OUT = 0;//�������0
	}	
	
	delay_us(5);
	OLED_SCL = 1;
	delay_us(5);
	OLED_SCL = 0;

}

//����һ���ֽ�����
void OLED_Iic_Send_Byte(u8 data)
{
	u8 i;
	OLED_Iic_Sda_Mode(GPIO_Mode_OUT);
	
	OLED_SCL = 0;
	for(i=0; i<8; i++)
	{
		//�����ݲ�λ���ȷ���λ  0x87  1000 0111
		
		if(data & (1<<(7-i)))
		{
			OLED_SDA_OUT = 1;//�������1
		}
		else
		{
			OLED_SDA_OUT = 0;//�������0		
		}
		delay_us(5);
		OLED_SCL = 1;
		delay_us(5);
		OLED_SCL = 0;	
	}
	
}
//����һλ���� STM32��Ϊ����
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
	//���͵�ַ����ִ��д����
	OLED_Iic_Send_Byte(OLED_ADDRESS);
	ack = OLED_Iic_Rcv_Ack();
	if(ack == 1)
	{
		printf("write failure\n");
		OLED_Iic_Stop();
		return;
	}
	
	//����д���ݵ���ʼ��ַ
	OLED_Iic_Send_Byte(addr);
	ack = OLED_Iic_Rcv_Ack();
	if(ack == 1)
	{
		printf("write failure\n");
		OLED_Iic_Stop();
		return;
	}	
	
	//��������
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

void OLED_WriteCmd(unsigned char I2C_Command)//д����
{
	OLED_I2C_WriteByte(0x00, I2C_Command);
}

void OLED_WriteDat(unsigned char I2C_Data)//д����
{
	OLED_I2C_WriteByte(0x40, I2C_Data);
}

void OLED_Init(void)
{
	
	delay_ms(100); //�������ʱ����Ҫ
	
	OLED_WriteCmd(0xAE); //display off
	OLED_WriteCmd(0x20);	//Set Memory Addressing Mode	
	OLED_WriteCmd(0x10);	//00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	OLED_WriteCmd(0xb0);	//Set Page Start Address for Page Addressing Mode,0-7
	OLED_WriteCmd(0xc8);	//Set COM Output Scan Direction
	OLED_WriteCmd(0x00); //---set low column address
	OLED_WriteCmd(0x10); //---set high column address
	OLED_WriteCmd(0x40); //--set start line address
	OLED_WriteCmd(0x81); //--set contrast control register
	OLED_WriteCmd(0xff); //���ȵ��� 0x00~0xff
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

void OLED_SetPos(unsigned char x, unsigned char y) //������ʼ������
{ 
	OLED_WriteCmd(0xb0+y);
	OLED_WriteCmd(((x&0xf0)>>4)|0x10);
	OLED_WriteCmd((x&0x0f)|0x01);
}

void OLED_Fill(unsigned char fill_Data)//ȫ�����
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

void OLED_CLS(void)//����
{
	OLED_Fill(0x00);
}

//--------------------------------------------------------------
// Prototype      : void OLED_ON(void)
// Calls          : 
// Parameters     : none
// Description    : ��OLED�������л���
//--------------------------------------------------------------
void OLED_ON(void)
{
	OLED_WriteCmd(0X8D);  //���õ�ɱ�
	OLED_WriteCmd(0X14);  //������ɱ�
	OLED_WriteCmd(0XAF);  //OLED����
}

//--------------------------------------------------------------
// Prototype      : void OLED_OFF(void)
// Calls          : 
// Parameters     : none
// Description    : ��OLED���� -- ����ģʽ��,OLED���Ĳ���10uA
//--------------------------------------------------------------
void OLED_OFF(void)
{
	OLED_WriteCmd(0X8D);  //���õ�ɱ�
	OLED_WriteCmd(0X10);  //�رյ�ɱ�
	OLED_WriteCmd(0XAE);  //OLED����
}

//--------------------------------------------------------------
// Prototype      : void OLED_ShowChar(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
// Calls          : 
// Parameters     : x,y -- ��ʼ������(x:0~127, y:0~7); ch[] -- Ҫ��ʾ���ַ���; TextSize -- �ַ���С(1:6*8 ; 2:8*16)
// Description    : ��ʾcodetab.h�е�ASCII�ַ�,��6*8��8*16��ѡ��
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
// Parameters     : x,y -- ��ʼ������(x:0~127, y:0~7); N:������codetab.h�е�����
// Description    : ��ʾcodetab.h�еĺ���,16*16����
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
// Parameters     : x0,y0 -- ��ʼ������(x0:0~127, y0:0~7); x1,y1 -- ���Խ���(������)������(x1:1~128,y1:1~8)
// Description    : ��ʾBMPλͼ
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
		OLED_WriteCmd (0xb0+i);    //����ҳ��ַ��0~7��
		OLED_WriteCmd (0x00);      //������ʾλ�á��е͵�ַ
		OLED_WriteCmd (0x10);      //������ʾλ�á��иߵ�ַ   
		for(n=0;n<128;n++)OLED_WriteDat(OLED_GRAM[n][i]); 
	}   
}


void OLED_DrawPoint(u8 x,u8 y,u8 t)
{
	u8 pos,bx,temp=0;
	if(x>127||y>63)return;//������Χ��.
	pos=7-y/8;
	bx=y%8;
	temp=1<<(7-bx);
	if(t)OLED_GRAM[x][pos]|=temp;
	else OLED_GRAM[x][pos]&=~temp;	 
}

