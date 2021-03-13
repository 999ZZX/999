#include "myiic.h"
#include "delay.h"

/****************************************
引脚说明
VCC -- 5V 
SCL -- PA3
SDA -- PA2
GND -- GND

*****************************************/

//初始化IIC
void IIC_Init(void)
{					     
	GPIO_InitTypeDef GPIO_InitStruct;  //结构体
	
	//使能GPIO A组时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_2|GPIO_Pin_3;     	//引脚
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_OUT;	//输出模式
	GPIO_InitStruct.GPIO_OType	= GPIO_OType_PP;    //输出开漏
	GPIO_InitStruct.GPIO_PuPd	= GPIO_PuPd_NOPULL;	    //上拉
	GPIO_InitStruct.GPIO_Speed	= GPIO_Speed_50MHz; //输出速度
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	SCL=1;
	SDA_OUT=1;
}

//引脚模式变更--SDA
void Iic_Sda_Mode(GPIOMode_TypeDef mode)
{
	GPIO_InitTypeDef  	GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2;				//第9号引脚
	GPIO_InitStructure.GPIO_Mode  = mode;					//输入/输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			//推挽输出，增强驱动能力，引脚的输出电流更大
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//引脚的速度最大为100MHz
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;		//没有使用内部上拉电阻
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
}

//产生IIC起始信号
void IIC_Start(void)
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
//产生IIC停止信号
void IIC_Stop(void)
{
	Iic_Sda_Mode(GPIO_Mode_OUT);

	SCL = 0;
	SDA_OUT = 0;
	delay_us(5);
	
	SCL = 1;
	delay_us(5);
	SDA_OUT = 1;						   	
}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 IIC_Wait_Ack(void)
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
//产生ACK应答
void IIC_Ack(void)
{
	Iic_Sda_Mode(GPIO_Mode_OUT);
	SCL = 0;
	SDA_OUT = 0;//引脚输出0
	delay_us(5);
	SCL = 1;
	delay_us(5);
	SCL = 0;

}
//不产生ACK应答		    
void IIC_NAck(void)
{
	Iic_Sda_Mode(GPIO_Mode_OUT);
	SCL = 0;
	SDA_OUT = 1;//引脚输出0
	delay_us(5);
	SCL = 1;
	delay_us(5);
	SCL = 0;
}					 				     
//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
void IIC_Send_Byte(u8 txd)
{    
	
	u8 t;
	Iic_Sda_Mode(GPIO_Mode_OUT);
	
	SCL = 0;
	for(t=0;t<8;t++)
    {              
        if(txd & (1<<(7-t)))
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
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
u8 IIC_Read_Byte(unsigned char ack)
{
	u8 i,receive=0;
	Iic_Sda_Mode(GPIO_Mode_IN);//SDA设置为输入
	
	SCL = 0;
    for(i=0;i<8;i++ )
	{
		
        delay_us(5);
		SCL = 1;
		delay_us(5);
		
		if(SDA_IN == 1)
		{
			receive |= (1<<(7-i));  //数据合成
		}
		SCL = 0;		
    }					 
    if (!ack)
        IIC_NAck();//发送nACK
    else
        IIC_Ack(); //发送ACK   
    return receive;
}


void IIC_WriteBytes(u8 WriteAddr,u8* data,u8 dataLength)
{		
	
	u8 i,ack;	
    IIC_Start();  

	IIC_Send_Byte(WriteAddr);	    //发送写命令
	ack = IIC_Wait_Ack();
	if(ack == 1)
	{
		printf("write failure\n");
		IIC_Stop();
		return;
	}
	
	for(i=0;i<dataLength;i++)
	{
		IIC_Send_Byte(data[i]);
		ack = IIC_Wait_Ack();
		if(ack == 1)
		{
			printf("IIC_Send_Byte failure\n");
			IIC_Stop();
			return;
		}
	}				    	   
    IIC_Stop();//产生一个停止条件 
	delay_ms(10);	 
}

void IIC_ReadBytes(u8 deviceAddr, u8 writeAddr,u8* data,u8 dataLength)
{		
	u8 i;	
    IIC_Start();  

	IIC_Send_Byte(deviceAddr);	    //发送写命令
	IIC_Wait_Ack();
	IIC_Send_Byte(writeAddr);
	IIC_Wait_Ack();
	IIC_Send_Byte(deviceAddr|0X01);//进入接收模式			   
	IIC_Wait_Ack();
	
	for(i=0;i<dataLength-1;i++)
	{
		data[i] = IIC_Read_Byte(1);
	}		
	data[dataLength-1] = IIC_Read_Byte(0);	
    IIC_Stop();//产生一个停止条件 
	delay_ms(10);	 
}

void IIC_Read_One_Byte(u8 daddr,u8 addr,u8* data)
{			
	u8 ack;

    IIC_Start();  
	
	IIC_Send_Byte(daddr);	   //发送写命令
	ack = IIC_Wait_Ack();
	if(ack == 1)
	{
		printf("IIC_Read_One_Byte failure\n");
		IIC_Stop();
		return;
	}
	IIC_Send_Byte(addr);//发送地址
	ack = IIC_Wait_Ack();
	if(ack == 1)
	{
		printf("IIC_Read_One_Byte failure\n");
		IIC_Stop();
		return;
	}	 
	IIC_Start();  	 	   
	IIC_Send_Byte(daddr|0X01);//进入接收模式			   
	ack = IIC_Wait_Ack();
	if(ack == 1)
	{
		printf("IIC_Read_One_Byte failure\n");
		IIC_Stop();
		return;
	}
		 
    *data = IIC_Read_Byte(0);		   
    IIC_Stop();//产生一个停止条件	    
}

void IIC_Write_One_Byte(u8 daddr,u8 addr,u8 data)
{	
	u8 ack;
	
    IIC_Start();  
	
	IIC_Send_Byte(daddr);	    //发送写命令
	ack = IIC_Wait_Ack();
	if(ack == 1)
	{
		printf("write failure\n");
		IIC_Stop();
		return;
	}
	IIC_Send_Byte(addr);//发送地址
	ack = IIC_Wait_Ack();
	if(ack == 1)
	{
		printf("write failure\n");
		IIC_Stop();
		return;
	}  	 										  		   
	IIC_Send_Byte(data);     //发送字节							   
	ack = IIC_Wait_Ack();
	if(ack == 1)
	{
		printf("write failure\n");
		IIC_Stop();
		return;
	}	    	   
    IIC_Stop();//产生一个停止条件 
	//delay_ms(10);	 
}



























