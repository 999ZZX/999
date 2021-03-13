#include "stm32f4xx.h"
#include "sys.h"
#include "i2c.h"
#include "delay.h"

//��ʼ��IIC
void MPU_IIC_Init(void)
{			
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//ʹ��GPIOBʱ��

	//GPIOB8,B9��ʼ������
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
	GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��
	MPU_SCL=1;
	MPU_SDA=1;
	
}
//����IIC��ʼ�ź�
void MPU_IIC_Start(void)
{
	SDA_OUT();     //sda�����
	MPU_SDA=1;	  	  
	MPU_SCL=1;
	delay_us(4);
 	MPU_SDA=0;//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	MPU_SCL=0;//ǯסI2C���ߣ�׼�����ͻ�������� 
}	  
//����IICֹͣ�ź�
void MPU_IIC_Stop(void)
{
	SDA_OUT();//sda�����
	MPU_SCL=0;
	MPU_SDA=0;//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	MPU_SCL=1; 
	MPU_SDA=1;//����I2C���߽����ź�
	delay_us(4);							   	
}
//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
u8 MPU_IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
	SDA_IN();      //SDA����Ϊ����  
	MPU_SDA=1;delay_us(1);	   
	MPU_SCL=1;delay_us(1);	 
	while(READ_SDA)
	{
		ucErrTime++;
		//delay_us(1);	
		if(ucErrTime>250)
		{
			MPU_IIC_Stop();
			return 1;
		}
	}
	MPU_SCL=0;//ʱ�����0 	   
	return 0;  
} 
//����ACKӦ��
void MPU_IIC_Ack(void)
{
	MPU_SCL=0;
	SDA_OUT();
	MPU_SDA=0;
	delay_us(2);
	MPU_SCL=1;
	delay_us(2);
	MPU_SCL=0;
}
//������ACKӦ��		    
void MPU_IIC_NAck(void)
{
	MPU_SCL=0;
	SDA_OUT();
	MPU_SDA=1;
	delay_us(2);
	MPU_SCL=1;
	delay_us(2);
	MPU_SCL=0;
}					 				     
//IIC����һ���ֽ�
//���شӻ�����Ӧ��
//1����Ӧ��
//0����Ӧ��			  
void MPU_IIC_Send_Byte(u8 txd)
{                        
    u8 t;   
	SDA_OUT(); 	    
    MPU_SCL=0;//����ʱ�ӿ�ʼ���ݴ���
    for(t=0;t<8;t++)
    {              
        MPU_SDA=(txd&0x80)>>7;
        txd<<=1; 	  
		delay_us(2);   //��TEA5767��������ʱ���Ǳ����
		MPU_SCL=1;
		delay_us(2); 
		MPU_SCL=0;	
		delay_us(2);
    }	 
} 	    
//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK   
u8 MPU_IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_IN();//SDA����Ϊ����
    for(i=0;i<8;i++ )
	{
        MPU_SCL=0; 
        delay_us(2);
		MPU_SCL=1;
        receive<<=1;
        if(READ_SDA)receive++;   
		delay_us(1); 
    }					 
    if (!ack)
        MPU_IIC_NAck();//����nACK
    else
        MPU_IIC_Ack(); //����ACK   
    return receive;
}

