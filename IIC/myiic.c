#include "myiic.h"
#include "delay.h"

/****************************************
����˵��
VCC -- 5V 
SCL -- PA3
SDA -- PA2
GND -- GND

*****************************************/

//��ʼ��IIC
void IIC_Init(void)
{					     
	GPIO_InitTypeDef GPIO_InitStruct;  //�ṹ��
	
	//ʹ��GPIO A��ʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_2|GPIO_Pin_3;     	//����
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_OUT;	//���ģʽ
	GPIO_InitStruct.GPIO_OType	= GPIO_OType_PP;    //�����©
	GPIO_InitStruct.GPIO_PuPd	= GPIO_PuPd_NOPULL;	    //����
	GPIO_InitStruct.GPIO_Speed	= GPIO_Speed_50MHz; //����ٶ�
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	SCL=1;
	SDA_OUT=1;
}

//����ģʽ���--SDA
void Iic_Sda_Mode(GPIOMode_TypeDef mode)
{
	GPIO_InitTypeDef  	GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2;				//��9������
	GPIO_InitStructure.GPIO_Mode  = mode;					//����/���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			//�����������ǿ�������������ŵ������������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//���ŵ��ٶ����Ϊ100MHz
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;		//û��ʹ���ڲ���������
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
}

//����IIC��ʼ�ź�
void IIC_Start(void)
{
	Iic_Sda_Mode(GPIO_Mode_OUT);
	//���߿���
	SCL = 1;
	SDA_OUT = 1;
	delay_us(5);
	SDA_OUT = 0;
	delay_us(5);
	SCL = 0;     //ǯס��
}	  
//����IICֹͣ�ź�
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
//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
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
//����ACKӦ��
void IIC_Ack(void)
{
	Iic_Sda_Mode(GPIO_Mode_OUT);
	SCL = 0;
	SDA_OUT = 0;//�������0
	delay_us(5);
	SCL = 1;
	delay_us(5);
	SCL = 0;

}
//������ACKӦ��		    
void IIC_NAck(void)
{
	Iic_Sda_Mode(GPIO_Mode_OUT);
	SCL = 0;
	SDA_OUT = 1;//�������0
	delay_us(5);
	SCL = 1;
	delay_us(5);
	SCL = 0;
}					 				     
//IIC����һ���ֽ�
//���شӻ�����Ӧ��
//1����Ӧ��
//0����Ӧ��			  
void IIC_Send_Byte(u8 txd)
{    
	
	u8 t;
	Iic_Sda_Mode(GPIO_Mode_OUT);
	
	SCL = 0;
	for(t=0;t<8;t++)
    {              
        if(txd & (1<<(7-t)))
		{
			SDA_OUT = 1;//�������1
		}
		else
		{
			SDA_OUT = 0;//�������0		
		}
		delay_us(5);
		SCL = 1;
		delay_us(5);
		SCL = 0;
    }	 	 
} 	    
//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK   
u8 IIC_Read_Byte(unsigned char ack)
{
	u8 i,receive=0;
	Iic_Sda_Mode(GPIO_Mode_IN);//SDA����Ϊ����
	
	SCL = 0;
    for(i=0;i<8;i++ )
	{
		
        delay_us(5);
		SCL = 1;
		delay_us(5);
		
		if(SDA_IN == 1)
		{
			receive |= (1<<(7-i));  //���ݺϳ�
		}
		SCL = 0;		
    }					 
    if (!ack)
        IIC_NAck();//����nACK
    else
        IIC_Ack(); //����ACK   
    return receive;
}


void IIC_WriteBytes(u8 WriteAddr,u8* data,u8 dataLength)
{		
	
	u8 i,ack;	
    IIC_Start();  

	IIC_Send_Byte(WriteAddr);	    //����д����
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
    IIC_Stop();//����һ��ֹͣ���� 
	delay_ms(10);	 
}

void IIC_ReadBytes(u8 deviceAddr, u8 writeAddr,u8* data,u8 dataLength)
{		
	u8 i;	
    IIC_Start();  

	IIC_Send_Byte(deviceAddr);	    //����д����
	IIC_Wait_Ack();
	IIC_Send_Byte(writeAddr);
	IIC_Wait_Ack();
	IIC_Send_Byte(deviceAddr|0X01);//�������ģʽ			   
	IIC_Wait_Ack();
	
	for(i=0;i<dataLength-1;i++)
	{
		data[i] = IIC_Read_Byte(1);
	}		
	data[dataLength-1] = IIC_Read_Byte(0);	
    IIC_Stop();//����һ��ֹͣ���� 
	delay_ms(10);	 
}

void IIC_Read_One_Byte(u8 daddr,u8 addr,u8* data)
{			
	u8 ack;

    IIC_Start();  
	
	IIC_Send_Byte(daddr);	   //����д����
	ack = IIC_Wait_Ack();
	if(ack == 1)
	{
		printf("IIC_Read_One_Byte failure\n");
		IIC_Stop();
		return;
	}
	IIC_Send_Byte(addr);//���͵�ַ
	ack = IIC_Wait_Ack();
	if(ack == 1)
	{
		printf("IIC_Read_One_Byte failure\n");
		IIC_Stop();
		return;
	}	 
	IIC_Start();  	 	   
	IIC_Send_Byte(daddr|0X01);//�������ģʽ			   
	ack = IIC_Wait_Ack();
	if(ack == 1)
	{
		printf("IIC_Read_One_Byte failure\n");
		IIC_Stop();
		return;
	}
		 
    *data = IIC_Read_Byte(0);		   
    IIC_Stop();//����һ��ֹͣ����	    
}

void IIC_Write_One_Byte(u8 daddr,u8 addr,u8 data)
{	
	u8 ack;
	
    IIC_Start();  
	
	IIC_Send_Byte(daddr);	    //����д����
	ack = IIC_Wait_Ack();
	if(ack == 1)
	{
		printf("write failure\n");
		IIC_Stop();
		return;
	}
	IIC_Send_Byte(addr);//���͵�ַ
	ack = IIC_Wait_Ack();
	if(ack == 1)
	{
		printf("write failure\n");
		IIC_Stop();
		return;
	}  	 										  		   
	IIC_Send_Byte(data);     //�����ֽ�							   
	ack = IIC_Wait_Ack();
	if(ack == 1)
	{
		printf("write failure\n");
		IIC_Stop();
		return;
	}	    	   
    IIC_Stop();//����һ��ֹͣ���� 
	//delay_ms(10);	 
}



























