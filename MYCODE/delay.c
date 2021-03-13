#include "delay.h"

u32 my_us = 21;
u32 my_ms = 21000;

void Delay_Init(void)
{
	//systick��ʱ��ʱ��Ƶ�ʣ�168MHZ/8 = 21MHZ  1us����21����
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);

}


//nusȡֵ��Χ��0~798,915
void delay_us(u32 nus)
{
	u32 temp;
	
	//��װֵ�Ĵ���
	SysTick->LOAD = my_us * nus - 1;
	//������ֵΪ 0 
	SysTick->VAL = 0x00;
	//������ʱ��
	SysTick->CTRL |= (0x01<<0);
	
	do
	{
		temp = SysTick->CTRL;
		
	}while((temp & (0x01<<0)) && !(temp & (0x01<<16)));

	//�رն�ʱ��
	SysTick->CTRL &= ~(0x01<<0);
}


//nusȡֵ��Χ��0~798
void delay_ms(u32 nms)
{
	u32 temp;
	
	//��װֵ�Ĵ���
	SysTick->LOAD = my_ms * nms - 1;
	//������ֵΪ 0 
	SysTick->VAL = 0x00;
	//������ʱ��
	SysTick->CTRL |= (0x01<<0);
	
	do
	{
		temp = SysTick->CTRL;
		
	}while((temp & (0x01<<0)) && !(temp & (0x01<<16)));

	//�رն�ʱ��
	SysTick->CTRL &= ~(0x01<<0);
}



void delay_s(u32 ns)
{
	int i;
	for(i=0; i<ns; i++)
	{
		delay_ms(500);
		delay_ms(500);
	}
}

