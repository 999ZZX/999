#include "beep.h"

/*******************************************
����˵��:
BEEP����PF8

PF8����ߵ�ƽ��1��������������
PF8����͵�ƽ��0������������

********************************************/

void Beep_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStruct;
		
	//ʹ��GPIO F��ʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
	
	//BEEP
	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_8;		//����8
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_OUT;	//���ģʽ
	GPIO_InitStruct.GPIO_OType	= GPIO_OType_PP;	//����
	GPIO_InitStruct.GPIO_PuPd	= GPIO_PuPd_DOWN;	//����
	GPIO_InitStruct.GPIO_Speed	= GPIO_Speed_50MHz; //����
	GPIO_Init(GPIOF, &GPIO_InitStruct);
}