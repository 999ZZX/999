#include "beep.h"

/*******************************************
引脚说明:
BEEP连接PF8

PF8输出高电平（1），蜂鸣器不响
PF8输出低电平（0），蜂鸣器响

********************************************/

void Beep_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStruct;
		
	//使能GPIO F组时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
	
	//BEEP
	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_8;		//引脚8
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_OUT;	//输出模式
	GPIO_InitStruct.GPIO_OType	= GPIO_OType_PP;	//推挽
	GPIO_InitStruct.GPIO_PuPd	= GPIO_PuPd_DOWN;	//下拉
	GPIO_InitStruct.GPIO_Speed	= GPIO_Speed_50MHz; //快速
	GPIO_Init(GPIOF, &GPIO_InitStruct);
}