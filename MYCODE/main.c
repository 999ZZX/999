#include "stm32f4xx.h"
#include "exti.h"
#include "delay.h"
#include "usart.h"
#include "sys.h"
#include "string.h"
#include "myiic.h"
#include "max30102.h" 
#include "algorithm.h"
#include "OLED_I2C.h"
#include "mpu6050.h"
#include "func.h"
#include "show_pedometer_step_count.h"
#include "rtc.h"
#include "show_heart_rate.h"
#include "iic.h"
#include "led.h"

extern u8 flag;

int main(void)
{
	
	//NVIC分为第二组，抢占优先级范围:0~3, 响应优先级范围:0~3
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	Delay_Init();
	Usart1_Init();
	Usart3_Init();
	Exte_Init();
	Iic_Init();
	Led_Init();
	//闹钟初始化
	Rtc_Init();
	RTC_A_Alarm();
	//自动设置闹钟时间
	auto_set_Alarm();

	//MPU初始化
	MPU_Init();
	dmp_init();
	//OLED初始化
	I2C_Configuration();
	OLED_Init();
	OLED_CLS();
	
	//心率显示
	max30102_init();
	show_heart_rate_config();
	

	u8 i=0, clean = 0;
	while(1)
	{
		send_to_phone();
		stop_start_Alarm();
		if(flag==0)
		{
			if(clean!=0)
			{
				OLED_CLS();
				clean = 0;
			}
			//显示时间
			showtime_date();
		}
		else if(flag==1)
		{
			if(clean!=1)
			{
				OLED_CLS();
				clean = 1;
			}
			//显示计步数
			show_pedometer_step_count();
		}
		else if(flag==2)
		{
			if(clean!=2)
			{
				OLED_CLS();
				clean = 2;
			}
			//改变闹钟时间
			change();
			//显示闹钟时间
			Alarm_time();
			
		}
		else if(flag==3)
		{
			if(clean!=3)
			{
				OLED_CLS();
				clean = 3;
			}
			//显示心率图
			show_heart_rate();
		}
			
	}
	
	return 0;
}


