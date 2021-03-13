#include "delay.h"

u32 my_us = 21;
u32 my_ms = 21000;

void Delay_Init(void)
{
	//systick定时器时钟频率：168MHZ/8 = 21MHZ  1us可以21个数
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);

}


//nus取值范围：0~798,915
void delay_us(u32 nus)
{
	u32 temp;
	
	//重装值寄存器
	SysTick->LOAD = my_us * nus - 1;
	//计数器值为 0 
	SysTick->VAL = 0x00;
	//启动定时器
	SysTick->CTRL |= (0x01<<0);
	
	do
	{
		temp = SysTick->CTRL;
		
	}while((temp & (0x01<<0)) && !(temp & (0x01<<16)));

	//关闭定时器
	SysTick->CTRL &= ~(0x01<<0);
}


//nus取值范围：0~798
void delay_ms(u32 nms)
{
	u32 temp;
	
	//重装值寄存器
	SysTick->LOAD = my_ms * nms - 1;
	//计数器值为 0 
	SysTick->VAL = 0x00;
	//启动定时器
	SysTick->CTRL |= (0x01<<0);
	
	do
	{
		temp = SysTick->CTRL;
		
	}while((temp & (0x01<<0)) && !(temp & (0x01<<16)));

	//关闭定时器
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

