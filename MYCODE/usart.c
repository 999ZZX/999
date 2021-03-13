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
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
int _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数   printf 是一个宏
int fputc(int ch, FILE *f)
{ 	
	USART_SendData(USART1,ch); //通过串口发送数据
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);      
	return ch;
}


/***********************************************
引脚说明：

PA9  -- TX
PA10 -- RX

************************************************/

void Usart1_Init(void)
{
	GPIO_InitTypeDef  	GPIO_InitStructure;
	USART_InitTypeDef 	USART_InitStruct;
	NVIC_InitTypeDef 	NVIC_InitStruct;
	
	
	//使能 USART1 时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	//B.  使用的是串口 1，串口 1 对应着芯片引脚 PA9,PA10 需要使能PA的时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); 


	//PA9 复用为 USART1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1); 
	//PA10 复用为 USART1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1);
	

	
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_9 | GPIO_Pin_10; //GPIOA9 与 GPIOA10
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF;				//配置IO口复用功能
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz; 		//速度 50MHz
	GPIO_InitStructure.GPIO_OType 	= GPIO_OType_PP; 			//推挽复用输出
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_UP; 			//上拉
	//初始化 PA9，PA10
	GPIO_Init(GPIOA,&GPIO_InitStructure); 
	
	
	
	USART_InitStruct.USART_BaudRate 			= 115200;							//一般设置为 115200;
	USART_InitStruct.USART_WordLength 			= USART_WordLength_8b;				//字长为 8 位数据格式
	USART_InitStruct.USART_StopBits				= USART_StopBits_1;					//一个停止位
	USART_InitStruct.USART_Parity 				= USART_Parity_No;					//无奇偶校验位
	USART_InitStruct.USART_HardwareFlowControl 	= USART_HardwareFlowControl_None;	//无硬件控制流
	USART_InitStruct.USART_Mode 				= USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	//初始化串口
	USART_Init(USART1, &USART_InitStruct); 
	

	NVIC_InitStruct.NVIC_IRQChannel 					= USART1_IRQn;  //中断通道
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority	=3;				//抢占优先级 3
	NVIC_InitStruct.NVIC_IRQChannelSubPriority 			=3;  			//响应优先级 3
	NVIC_InitStruct.NVIC_IRQChannelCmd 					= ENABLE; 		//IRQ 通道使能
	//根据指定的参数初始化 VIC 寄存器
	NVIC_Init(&NVIC_InitStruct);  

	//开启中断，接收到数据中断
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	
	//使能串口
	USART_Cmd(USART1, ENABLE); 

	
}

void Usart2_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	//使能 PA 时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); 
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG,ENABLE); 
	
	//使能USART2 时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	//串口 2 引脚复用映射//PA2复用为USART2
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2); 
	//PA3 复用为 USART2
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2);
	
	//GPIOA2 与 GPIOA3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;		//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; 	//速度 100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 		//推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 		//上拉
	GPIO_Init(GPIOA,&GPIO_InitStructure); 				//初始化 PA2，PA3
	
	//RE引脚接的是  所以设置PG8 推挽输出，485 模式控制
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; 			//GPIOG8
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		//输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; 	//速度 100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 		//推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	 	//上拉
	GPIO_Init(GPIOG,&GPIO_InitStructure); 				//初始化 PG8


	USART_InitTypeDef 	USART_InitStructure;
	//USART2 初始化设置
	USART_InitStructure.USART_BaudRate = 115200;					//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;		//字长为 8 位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;			//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;				//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl =USART_HardwareFlowControl_None;	//无硬件数据流控制								
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//收发
	USART_Init(USART2, &USART_InitStructure); 						//初始化串口 2
	

	NVIC_InitTypeDef 	NVIC_InitStructure;
	//Usart2 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//抢占优先级 3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;  //响应优先级 3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ 通道使能
	NVIC_Init(&NVIC_InitStructure);  //根据指定的参数初始化 NVIC 寄存器、
	
	USART_Cmd(USART2, ENABLE); 										//使能串口 2
	USART_ClearFlag(USART2, USART_FLAG_TC);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启接受中断

	PGout(8) = 0; //默认为接收模式
	
}

void Usart3_Init(void)
{
	GPIO_InitTypeDef  	GPIO_InitStructure;
	USART_InitTypeDef 	USART_InitStruct;
	NVIC_InitTypeDef 	NVIC_InitStruct;
	
	
	//使能 USART3 时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);
	//使用的是串口 1，串口 1 对应着芯片引脚 PA9,PA10 需要使能PB的时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);


	//PB10 复用为 USART3
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource10,GPIO_AF_USART3); 
	//PB11 复用为 USART3
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource11,GPIO_AF_USART3);
	

	
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_10 | GPIO_Pin_11; //GPIOB11 10
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF;				//配置IO口复用功能
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz; 		//速度 50MHz
	GPIO_InitStructure.GPIO_OType 	= GPIO_OType_PP; 			//推挽复用输出
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_UP; 			//上拉
	//初始化GPIO
	GPIO_Init(GPIOB,&GPIO_InitStructure); 
	
	
	
	USART_InitStruct.USART_BaudRate 			= 9600;							//一般设置为 115200;
	USART_InitStruct.USART_WordLength 			= USART_WordLength_8b;				//字长为 8 位数据格式
	USART_InitStruct.USART_StopBits				= USART_StopBits_1;					//一个停止位
	USART_InitStruct.USART_Parity 				= USART_Parity_No;					//无奇偶校验位
	USART_InitStruct.USART_HardwareFlowControl 	= USART_HardwareFlowControl_None;	//无硬件控制流
	USART_InitStruct.USART_Mode 				= USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	//初始化串口
	USART_Init(USART3, &USART_InitStruct); 
	 
	

	NVIC_InitStruct.NVIC_IRQChannel 					= USART3_IRQn;  //中断通道
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority	=3;				//抢占优先级 3
	NVIC_InitStruct.NVIC_IRQChannelSubPriority 			=3;  			//响应优先级 3
	NVIC_InitStruct.NVIC_IRQChannelCmd 					= ENABLE; 		//IRQ 通道使能
	//根据指定的参数初始化 VIC 寄存器
	NVIC_Init(&NVIC_InitStruct);  
	

	//开启中断，接收到数据中断
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	
	//使能串口
	USART_Cmd(USART3, ENABLE); 
}

void USART3_IRQHandler(void)
{
	
   //若是非空，则返回值为1，与RESET（0）判断，不相等则判断为真
   if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
   {	
	   //判断为真后，为下次中断做准备，则需要对中断的标志清零
		USART_ClearITPendingBit(USART3,USART_IT_RXNE);
	  
	    /* DR读取接受到的数据*/
		buffer[count++] = USART_ReceiveData(USART3);	
	   //判断是否接受结束符
	   if(buffer[count-1] == ':')
	   {
		   //重新赋值，过滤结束符
			for(rx_i = 0; rx_i<(count-1); rx_i++)
		   {
				rx_buffer[rx_i] = buffer[rx_i];
		   }
			rx_flag = 1;  //接受一帧数据
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
				USART_SendData(USART3,(temp+'0')); //通过串口发送数据
				while(USART_GetFlagStatus(USART3,USART_FLAG_TXE)==RESET);
			}
			
			rx_flag = 0;
			memset(rx_buffer,0, sizeof(rx_buffer));
		}
	}
	
}

