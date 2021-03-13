#include "iic.h"
#include "delay.h"


/****************************************
����˵��
SCL -- PB8
SDA -- PB9

*****************************************/



void Iic_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;  //�ṹ��
	
	//ʹ��GPIO B��ʱ��

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);	

	
	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_8|GPIO_Pin_9;     	//����8 9
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_OUT;	//���ģʽ
	GPIO_InitStruct.GPIO_OType	= GPIO_OType_PP;    //�������
	GPIO_InitStruct.GPIO_PuPd	= GPIO_PuPd_UP;	    //����
	GPIO_InitStruct.GPIO_Speed	= GPIO_Speed_50MHz; //����ٶ�
	
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	//���߿���
	SCL = 1;
	SDA_OUT = 1;

}

//����ģʽ���--SDA
void Iic_Sda_Mode(GPIOMode_TypeDef mode)
{
	GPIO_InitTypeDef  	GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;				//��9������
	GPIO_InitStructure.GPIO_Mode  = mode;					//����/���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			//�����������ǿ�������������ŵ������������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//���ŵ��ٶ����Ϊ100MHz
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;		//û��ʹ���ڲ���������
	GPIO_Init(GPIOB, &GPIO_InitStructure);	
}

//�����ź�
void Iic_Start(void)
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

//ֹͣ�ź�
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

//����һλ����
void Iic_Send_Ack(u8 ack)
{
	Iic_Sda_Mode(GPIO_Mode_OUT);
	SCL = 0;
	//׼������
	if(ack == 1)
	{
		SDA_OUT = 1;//�������1
	}
	if(ack == 0)
	{
		SDA_OUT = 0;//�������0
	}	
	
	delay_us(5);
	SCL = 1;
	delay_us(5);
	SCL = 0;

}

//����һ���ֽ�����
void Iic_Send_Byte(u8 data)
{
	u8 i;
	Iic_Sda_Mode(GPIO_Mode_OUT);
	
	SCL = 0;
	for(i=0; i<8; i++)
	{
		//�����ݲ�λ���ȷ���λ  0x87  1000 0111
		
		if(data & (1<<(7-i)))
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

//����һλ���� STM32��Ϊ����
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

//����һ���ֽ�����
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
			data |= (1<<(7-i));  //���ݺϳ�
		}		
		
		
		SCL = 0;	
	}

	return data;
}

//��AT24C02д������
void AT24c02_Write(u8 addr, u8 *write_buff, u8 len)
{
	u8 ack;
	
	Iic_Start();
	//���͵�ַ����ִ��д����
	Iic_Send_Byte(0xA0);
	ack = Iic_Rcv_Ack();
	if(ack == 1)
	{
		printf("write failure\n");
		Iic_Stop();
		return;
	}
	
	//����д���ݵ���ʼ��ַ
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
		//��������
		Iic_Send_Byte(*write_buff);
		ack = Iic_Rcv_Ack();
		if(ack == 1)
		{
			printf("write failure\n");
			Iic_Stop();
			return;
		}			
		
		write_buff++;  //��ַƫ�Ƶ��¸�����
	}
	
	Iic_Stop();
	printf("write finish\n");
}

void AT24c02_Read(u8 addr, u8 *read_buff, u8 len)
{
	u8 ack;
	
	Iic_Start();
	//���͵�ַ����ִ��д����
	Iic_Send_Byte(0xA0);
	ack = Iic_Rcv_Ack();
	if(ack == 1)
	{
		printf("write failure\n");
		Iic_Stop();
		return;
	}
	
	//���Ͷ����ݵ���ʼ��ַ
	Iic_Send_Byte(addr);
	ack = Iic_Rcv_Ack();
	if(ack == 1)
	{
		printf("write failure\n");
		Iic_Stop();
		return;
	}		
	
	Iic_Start();

	//���͵�ַ����ִ�ж�����
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
			Iic_Send_Ack(0); //������ЧӦ��
		}
		
	}
	Iic_Send_Ack(1); //���ͷ�Ӧ��
	Iic_Stop();
}






