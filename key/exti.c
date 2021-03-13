#include "exti.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "sys.h"
#include "OLED_I2C.h"

 u8 flag = 0;
 u8 Timer = 0;//时间计数
 u8 OLED_flag = 1;//屏幕开启标志，开启为1，关闭为0
/*********************************************
S1(KEY1)连接 PA0
按键未按下，PA0为高电平
按键按下，   PA0为低电平

当按键按下瞬间，为下降沿。
********************************************/
void Exte_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStruct;
	EXTI_InitTypeDef  EXTI_InitStruct;
	NVIC_InitTypeDef  NVIC_InitStruct;
	
	
	//使能SYSCFG时钟： 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	//使能GPIO A组时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_0;		//引脚0
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_IN;		//输入模式
	GPIO_InitStruct.GPIO_PuPd	= GPIO_PuPd_UP;		//上拉
	GPIO_Init(GPIOA, &GPIO_InitStruct);	
	
	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;		//引脚2 3 4
	GPIO_Init(GPIOE, &GPIO_InitStruct);	
	
	//设置IO口与中断线的映射关系。 PA0 -- EXTT0
   SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);
   SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource2);
   SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource3);
   SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource4);
	
	
	EXTI_InitStruct.EXTI_Line	= EXTI_Line0|EXTI_Line2|EXTI_Line3|EXTI_Line4;			//中断线0
	EXTI_InitStruct.EXTI_Mode	= EXTI_Mode_Interrupt;	//中断模式
	EXTI_InitStruct.EXTI_Trigger= EXTI_Trigger_Falling; //下降沿触发
	EXTI_InitStruct.EXTI_LineCmd= ENABLE;				//中断线使能
	//初始化线上中断，设置触发条件等。
    EXTI_Init(&EXTI_InitStruct);
	
	NVIC_InitStruct.NVIC_IRQChannel						= EXTI0_IRQn;		//中断通道
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority	= 2;				//抢占优先级
 	NVIC_InitStruct.NVIC_IRQChannelSubPriority			= 2;				//响应优先级
	NVIC_InitStruct.NVIC_IRQChannelCmd					= ENABLE;			//中断通道使能 
	//配置中断分组（NVIC），并使能中断。
    NVIC_Init(&NVIC_InitStruct);
	
	NVIC_InitStruct.NVIC_IRQChannel						= EXTI2_IRQn;		//中断通道
	NVIC_Init(&NVIC_InitStruct);

	NVIC_InitStruct.NVIC_IRQChannel						= EXTI3_IRQn;		//中断通道
	NVIC_Init(&NVIC_InitStruct);

	NVIC_InitStruct.NVIC_IRQChannel						= EXTI4_IRQn;		//中断通道
	NVIC_Init(&NVIC_InitStruct);

}

//中断服务函数，所有的中断函数可在startup_stm32f40_41xxx.s查找
//中断服务函数不需要程序员调用，当满足条件后，CPU自动调用
void  EXTI0_IRQHandler(void)
{
	
	if(EXTI_GetITStatus(EXTI_Line0) == SET)
	{
		delay_ms(15);
		if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)==RESET)
		{
			Timer = 0;
			flag = 1;
			
			if(OLED_flag==0)
			{
				OLED_ON();
			}
			OLED_flag = 1;
		}
	
	}
	//清除中断线0标志位
    EXTI_ClearITPendingBit(EXTI_Line0);
	
}

void  EXTI2_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line2) == SET)
	{
		delay_ms(15);
		if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2)==RESET)
		{
			flag = 2;
			if(OLED_flag==0)
			{
				OLED_ON();
			}
			OLED_flag = 1;
			Timer = 0;
		}
	
	}
	//清除中断线0标志位
    EXTI_ClearITPendingBit(EXTI_Line2);
	
}

void  EXTI3_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line3) == SET)
	{
		delay_ms(15);
		if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_3)==RESET)
		{
			flag = 3;
			if(OLED_flag==0)
			{
				OLED_ON();
			}
			OLED_flag = 1;
			Timer = 0;
			
			
		}
	
	}
	//清除中断线0标志位
    EXTI_ClearITPendingBit(EXTI_Line3);
	
}

void  EXTI4_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line4) == SET)
	{
		delay_ms(15);
		if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)==RESET)
		{
			flag = 0;
			if(OLED_flag==0)
			{
				OLED_ON();
			}
			OLED_flag = 1;
			Timer = 0;
		}
	
	}
	//清除中断线0标志位
    EXTI_ClearITPendingBit(EXTI_Line4);
	
}
















