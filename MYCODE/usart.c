#include "usart.h"
#include "stdio.h"
#include "sys.h"
#include <string.h>
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"

u8 rx_flag = 0;
u8 buffer[64] = {0};
u8 rx_buffer[64] = {0};
u8 rx_i , count=0;

#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
int _sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc����   printf ��һ����
int fputc(int ch, FILE *f)
{ 	
	USART_SendData(USART1,ch); //ͨ�����ڷ�������
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);      
	return ch;
}


/***********************************************
����˵����

PA9  -- TX
PA10 -- RX

************************************************/

void Usart1_Init(void)
{
	GPIO_InitTypeDef  	GPIO_InitStructure;
	USART_InitTypeDef 	USART_InitStruct;
	NVIC_InitTypeDef 	NVIC_InitStruct;
	
	
	//ʹ�� USART1 ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	//B.  ʹ�õ��Ǵ��� 1������ 1 ��Ӧ��оƬ���� PA9,PA10 ��Ҫʹ��PA��ʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); 


	//PA9 ����Ϊ USART1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1); 
	//PA10 ����Ϊ USART1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1);
	

	
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_9 | GPIO_Pin_10; //GPIOA9 �� GPIOA10
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF;				//����IO�ڸ��ù���
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz; 		//�ٶ� 50MHz
	GPIO_InitStructure.GPIO_OType 	= GPIO_OType_PP; 			//���츴�����
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_UP; 			//����
	//��ʼ�� PA9��PA10
	GPIO_Init(GPIOA,&GPIO_InitStructure); 
	
	
	
	USART_InitStruct.USART_BaudRate 			= 115200;							//һ������Ϊ 115200;
	USART_InitStruct.USART_WordLength 			= USART_WordLength_8b;				//�ֳ�Ϊ 8 λ���ݸ�ʽ
	USART_InitStruct.USART_StopBits				= USART_StopBits_1;					//һ��ֹͣλ
	USART_InitStruct.USART_Parity 				= USART_Parity_No;					//����żУ��λ
	USART_InitStruct.USART_HardwareFlowControl 	= USART_HardwareFlowControl_None;	//��Ӳ��������
	USART_InitStruct.USART_Mode 				= USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	//��ʼ������
	USART_Init(USART1, &USART_InitStruct); 
	

	NVIC_InitStruct.NVIC_IRQChannel 					= USART1_IRQn;  //�ж�ͨ��
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority	=3;				//��ռ���ȼ� 3
	NVIC_InitStruct.NVIC_IRQChannelSubPriority 			=3;  			//��Ӧ���ȼ� 3
	NVIC_InitStruct.NVIC_IRQChannelCmd 					= ENABLE; 		//IRQ ͨ��ʹ��
	//����ָ���Ĳ�����ʼ�� VIC �Ĵ���
	NVIC_Init(&NVIC_InitStruct);  

	//�����жϣ����յ������ж�
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	
	//ʹ�ܴ���
	USART_Cmd(USART1, ENABLE); 

	
}

void Usart2_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	//ʹ�� PA ʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); 
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG,ENABLE); 
	
	//ʹ��USART2 ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	//���� 2 ���Ÿ���ӳ��//PA2����ΪUSART2
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2); 
	//PA3 ����Ϊ USART2
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2);
	
	//GPIOA2 �� GPIOA3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;		//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; 	//�ٶ� 100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 		//���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 		//����
	GPIO_Init(GPIOA,&GPIO_InitStructure); 				//��ʼ�� PA2��PA3
	
	//RE���Žӵ���  ��������PG8 ���������485 ģʽ����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; 			//GPIOG8
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		//���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; 	//�ٶ� 100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 		//�������
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	 	//����
	GPIO_Init(GPIOG,&GPIO_InitStructure); 				//��ʼ�� PG8


	USART_InitTypeDef 	USART_InitStructure;
	//USART2 ��ʼ������
	USART_InitStructure.USART_BaudRate = 115200;					//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;		//�ֳ�Ϊ 8 λ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;			//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;				//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl =USART_HardwareFlowControl_None;	//��Ӳ������������								
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//�շ�
	USART_Init(USART2, &USART_InitStructure); 						//��ʼ������ 2
	

	NVIC_InitTypeDef 	NVIC_InitStructure;
	//Usart2 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//��ռ���ȼ� 3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;  //��Ӧ���ȼ� 3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ ͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);  //����ָ���Ĳ�����ʼ�� NVIC �Ĵ�����
	
	USART_Cmd(USART2, ENABLE); 										//ʹ�ܴ��� 2
	USART_ClearFlag(USART2, USART_FLAG_TC);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//���������ж�

	PGout(8) = 0; //Ĭ��Ϊ����ģʽ
	
}

void Usart3_Init(void)
{
	GPIO_InitTypeDef  	GPIO_InitStructure;
	USART_InitTypeDef 	USART_InitStruct;
	NVIC_InitTypeDef 	NVIC_InitStruct;
	
	
	//ʹ�� USART3 ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);
	//ʹ�õ��Ǵ��� 1������ 1 ��Ӧ��оƬ���� PA9,PA10 ��Ҫʹ��PB��ʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);


	//PB10 ����Ϊ USART3
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource10,GPIO_AF_USART3); 
	//PB11 ����Ϊ USART3
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource11,GPIO_AF_USART3);
	

	
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_10 | GPIO_Pin_11; //GPIOB11 10
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF;				//����IO�ڸ��ù���
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz; 		//�ٶ� 50MHz
	GPIO_InitStructure.GPIO_OType 	= GPIO_OType_PP; 			//���츴�����
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_UP; 			//����
	//��ʼ��GPIO
	GPIO_Init(GPIOB,&GPIO_InitStructure); 
	
	
	
	USART_InitStruct.USART_BaudRate 			= 9600;							//һ������Ϊ 115200;
	USART_InitStruct.USART_WordLength 			= USART_WordLength_8b;				//�ֳ�Ϊ 8 λ���ݸ�ʽ
	USART_InitStruct.USART_StopBits				= USART_StopBits_1;					//һ��ֹͣλ
	USART_InitStruct.USART_Parity 				= USART_Parity_No;					//����żУ��λ
	USART_InitStruct.USART_HardwareFlowControl 	= USART_HardwareFlowControl_None;	//��Ӳ��������
	USART_InitStruct.USART_Mode 				= USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	//��ʼ������
	USART_Init(USART3, &USART_InitStruct); 
	 
	

	NVIC_InitStruct.NVIC_IRQChannel 					= USART3_IRQn;  //�ж�ͨ��
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority	=3;				//��ռ���ȼ� 3
	NVIC_InitStruct.NVIC_IRQChannelSubPriority 			=3;  			//��Ӧ���ȼ� 3
	NVIC_InitStruct.NVIC_IRQChannelCmd 					= ENABLE; 		//IRQ ͨ��ʹ��
	//����ָ���Ĳ�����ʼ�� VIC �Ĵ���
	NVIC_Init(&NVIC_InitStruct);  
	

	//�����жϣ����յ������ж�
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	
	//ʹ�ܴ���
	USART_Cmd(USART3, ENABLE); 
}

void USART3_IRQHandler(void)
{
	
   //���Ƿǿգ��򷵻�ֵΪ1����RESET��0���жϣ���������ж�Ϊ��
   if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
   {	
	   //�ж�Ϊ���Ϊ�´��ж���׼��������Ҫ���жϵı�־����
		USART_ClearITPendingBit(USART3,USART_IT_RXNE);
	  
	    /* DR��ȡ���ܵ�������*/
		buffer[count++] = USART_ReceiveData(USART3);	
	   //�ж��Ƿ���ܽ�����
	   if(buffer[count-1] == ':')
	   {
		   //���¸�ֵ�����˽�����
			for(rx_i = 0; rx_i<(count-1); rx_i++)
		   {
				rx_buffer[rx_i] = buffer[rx_i];
		   }
			rx_flag = 1;  //����һ֡����
		   count = 0;
		   
		   memset(buffer,0, sizeof(buffer));
	   }
   }
   
}

void send_to_phone()
{
	if(rx_flag==1)
	{
		unsigned long Count = 0;
		u8 i = 10;
		u8 temp = 0;
		if(rx_buffer[0]=='G'&&rx_buffer[1]=='E'&&rx_buffer[2]=='T')
		{
			dmp_get_pedometer_step_count(&Count);
			while(Count/i)
			{
				i *= 10;
			}
			while(i>1)
			{
				i /= 10;
				temp = Count/i;
				Count = Count%i;
				USART_SendData(USART3,(temp+'0')); //ͨ�����ڷ�������
				while(USART_GetFlagStatus(USART3,USART_FLAG_TXE)==RESET);
			}
			
			rx_flag = 0;
			memset(rx_buffer,0, sizeof(rx_buffer));
		}
	}
	
}

