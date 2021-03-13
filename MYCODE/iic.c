#include "iic.h"
#include "delay.h"


/****************************************
引脚说明
SCL -- PB8
SDA -- PB9

*****************************************/



void Iic_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;  //结构体
	
	//使能GPIO B组时钟

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);	

	
	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_8|GPIO_Pin_9;     	//引脚8 9
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_OUT;	//输出模式
	GPIO_InitStruct.GPIO_OType	= GPIO_OType_PP;    //输出推挽
	GPIO_InitStruct.GPIO_PuPd	= GPIO_PuPd_UP;	    //上拉
	GPIO_InitStruct.GPIO_Speed	= GPIO_Speed_50MHz; //输出速度
	
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	//总线空闲
	SCL = 1;
	SDA_OUT = 1;

}

//引脚模式变更--SDA
void Iic_Sda_Mode(GPIOMode_TypeDef mode)
{
	GPIO_InitTypeDef  	GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;				//第9号引脚
	GPIO_InitStructure.GPIO_Mode  = mode;					//输入/输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			//推挽输出，增强驱动能力，引脚的输出电流更大
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//引脚的速度最大为100MHz
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;		//没有使用内部上拉电阻
	GPIO_Init(GPIOB, &GPIO_InitStructure);	
}

//启动信号
void Iic_Start(void)
{
	Iic_Sda_Mode(GPIO_Mode_OUT);
	//总线空闲
	SCL = 1;
	SDA_OUT = 1;
	delay_us(5);
	
	SDA_OUT = 0;
	delay_us(5);
	SCL = 0;     //钳住线
}

//停止信号
void Iic_Stop(void)
{
	Iic_Sda_Mode(GPIO_Mode_OUT);

	SCL = 0;
	SDA_OUT = 0;
	delay_us(5);
	
	SCL = 1;
	delay_us(5);
	SDA_OUT = 1;
}

//发送一位数据
void Iic_Send_Ack(u8 ack)
{
	Iic_Sda_Mode(GPIO_Mode_OUT);
	SCL = 0;
	//准备数据
	if(ack == 1)
	{
		SDA_OUT = 1;//引脚输出1
	}
	if(ack == 0)
	{
		SDA_OUT = 0;//引脚输出0
	}	
	
	delay_us(5);
	SCL = 1;
	delay_us(5);
	SCL = 0;

}

//发送一个字节数据
void Iic_Send_Byte(u8 data)
{
	u8 i;
	Iic_Sda_Mode(GPIO_Mode_OUT);
	
	SCL = 0;
	for(i=0; i<8; i++)
	{
		//将数据拆位，先发高位  0x87  1000 0111
		
		if(data & (1<<(7-i)))
		{
			SDA_OUT = 1;//引脚输出1
		}
		else
		{
			SDA_OUT = 0;//引脚输出0		
		}
		delay_us(5);
		SCL = 1;
		delay_us(5);
		SCL = 0;	
	}
	
}

//接受一位数据 STM32做为输入
u8 Iic_Rcv_Ack(void)
{
	u8  ack;
	Iic_Sda_Mode(GPIO_Mode_IN);
	
	SCL = 0;
	delay_us(5);
	SCL = 1;
	delay_us(5);
	
	if(SDA_IN == 1)
	{
		ack = 1;
	}
	else
	{
		ack = 0;
	}
	
	SCL = 0;	

	return ack;
}

//接受一个字节数据
u8 Iic_Rcv_Byte(void)
{
	u8 i, data = 0; //data == 0000 0000
	Iic_Sda_Mode(GPIO_Mode_IN);
	
	SCL = 0;
	for(i=0; i<8; i++)
	{
		delay_us(5);
		SCL = 1;
		delay_us(5);
		
		if(SDA_IN == 1)
		{
			data |= (1<<(7-i));  //数据合成
		}		
		
		
		SCL = 0;	
	}

	return data;
}

//往AT24C02写入数据
void AT24c02_Write(u8 addr, u8 *write_buff, u8 len)
{
	u8 ack;
	
	Iic_Start();
	//发送地址，并执行写操作
	Iic_Send_Byte(0xA0);
	ack = Iic_Rcv_Ack();
	if(ack == 1)
	{
		printf("write failure\n");
		Iic_Stop();
		return;
	}
	
	//发送写数据的起始地址
	Iic_Send_Byte(addr);
	ack = Iic_Rcv_Ack();
	if(ack == 1)
	{
		printf("write failure\n");
		Iic_Stop();
		return;
	}	
	
	while(len--)
	{
		//发送数据
		Iic_Send_Byte(*write_buff);
		ack = Iic_Rcv_Ack();
		if(ack == 1)
		{
			printf("write failure\n");
			Iic_Stop();
			return;
		}			
		
		write_buff++;  //地址偏移到下个数据
	}
	
	Iic_Stop();
	printf("write finish\n");
}

void AT24c02_Read(u8 addr, u8 *read_buff, u8 len)
{
	u8 ack;
	
	Iic_Start();
	//发送地址，并执行写操作
	Iic_Send_Byte(0xA0);
	ack = Iic_Rcv_Ack();
	if(ack == 1)
	{
		printf("write failure\n");
		Iic_Stop();
		return;
	}
	
	//发送读数据的起始地址
	Iic_Send_Byte(addr);
	ack = Iic_Rcv_Ack();
	if(ack == 1)
	{
		printf("write failure\n");
		Iic_Stop();
		return;
	}		
	
	Iic_Start();

	//发送地址，并执行读操作
	Iic_Send_Byte(0xA1);
	ack = Iic_Rcv_Ack();
	if(ack == 1)
	{
		printf("write failure\n");
		Iic_Stop();
		return;
	}
	
	while(len--)   //5
	{
		*read_buff = Iic_Rcv_Byte();
		read_buff++;
		
		if(len > 0)
		{
			Iic_Send_Ack(0); //发送有效应答
		}
		
	}
	Iic_Send_Ack(1); //发送非应答
	Iic_Stop();
}






