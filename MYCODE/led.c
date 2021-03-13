#include "led.h"

/*******************************************
����˵��:
LED0����PF9
LED1����PF10

LED2����PE13
LED3����PE14


PF9����ߵ�ƽ��1��������
PF9����͵�ƽ��0��������




********************************************/

void Led_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStruct;
		
	//ʹ��GPIO F��ʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	
	//LED0
	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_9;		//����9
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_OUT;	//���ģʽ
	GPIO_InitStruct.GPIO_OType	= GPIO_OType_PP;	//����
	GPIO_InitStruct.GPIO_PuPd	= GPIO_PuPd_UP;		//����
	GPIO_InitStruct.GPIO_Speed	= GPIO_Speed_50MHz; //����
	GPIO_Init(GPIOF, &GPIO_InitStruct);
	//LED1
	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_10;		//����10
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_OUT;	//���ģʽ
	GPIO_InitStruct.GPIO_OType	= GPIO_OType_PP;	//����
	GPIO_InitStruct.GPIO_PuPd	= GPIO_PuPd_UP;		//����
	GPIO_InitStruct.GPIO_Speed	= GPIO_Speed_50MHz; //����
	GPIO_Init(GPIOF, &GPIO_InitStruct);

	
	//FSMA_D10
	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_13|GPIO_Pin_14;	//����13	14
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_OUT;			//���ģʽ
	GPIO_InitStruct.GPIO_OType	= GPIO_OType_PP;			//����
	GPIO_InitStruct.GPIO_PuPd	= GPIO_PuPd_UP;				//����
	GPIO_InitStruct.GPIO_Speed	= GPIO_Speed_50MHz; 		//����
	GPIO_Init(GPIOE, &GPIO_InitStruct);
	
	GPIO_SetBits(GPIOF, GPIO_Pin_9);	//���1,LED0����

	GPIO_SetBits(GPIOF, GPIO_Pin_10);	//���1,LED1����

	GPIO_SetBits(GPIOE, GPIO_Pin_13);	//���1,LED2����

	GPIO_SetBits(GPIOE, GPIO_Pin_14);	//���1,LED3����
	
	
}