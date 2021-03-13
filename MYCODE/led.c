#include "led.h"

/*******************************************
引脚说明:
LED0连接PF9
LED1连接PF10

LED2连接PE13
LED3连接PE14


PF9输出高电平（1），灯灭
PF9输出低电平（0），灯亮




********************************************/

void Led_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStruct;
		
	//使能GPIO F组时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	
	//LED0
	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_9;		//引脚9
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_OUT;	//输出模式
	GPIO_InitStruct.GPIO_OType	= GPIO_OType_PP;	//推挽
	GPIO_InitStruct.GPIO_PuPd	= GPIO_PuPd_UP;		//上拉
	GPIO_InitStruct.GPIO_Speed	= GPIO_Speed_50MHz; //快速
	GPIO_Init(GPIOF, &GPIO_InitStruct);
	//LED1
	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_10;		//引脚10
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_OUT;	//输出模式
	GPIO_InitStruct.GPIO_OType	= GPIO_OType_PP;	//推挽
	GPIO_InitStruct.GPIO_PuPd	= GPIO_PuPd_UP;		//上拉
	GPIO_InitStruct.GPIO_Speed	= GPIO_Speed_50MHz; //快速
	GPIO_Init(GPIOF, &GPIO_InitStruct);

	
	//FSMA_D10
	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_13|GPIO_Pin_14;	//引脚13	14
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_OUT;			//输出模式
	GPIO_InitStruct.GPIO_OType	= GPIO_OType_PP;			//推挽
	GPIO_InitStruct.GPIO_PuPd	= GPIO_PuPd_UP;				//上拉
	GPIO_InitStruct.GPIO_Speed	= GPIO_Speed_50MHz; 		//快速
	GPIO_Init(GPIOE, &GPIO_InitStruct);
	
	GPIO_SetBits(GPIOF, GPIO_Pin_9);	//输出1,LED0灯灭

	GPIO_SetBits(GPIOF, GPIO_Pin_10);	//输出1,LED1灯灭

	GPIO_SetBits(GPIOE, GPIO_Pin_13);	//输出1,LED2灯灭

	GPIO_SetBits(GPIOE, GPIO_Pin_14);	//输出1,LED3灯灭
	
	
}