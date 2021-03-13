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
	
	//NVIC��Ϊ�ڶ��飬��ռ���ȼ���Χ:0~3, ��Ӧ���ȼ���Χ:0~3
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	Delay_Init();
	Usart1_Init();
	Usart3_Init();
	Exte_Init();
	Iic_Init();
	Led_Init();
	//���ӳ�ʼ��
	Rtc_Init();
	RTC_A_Alarm();
	//�Զ���������ʱ��
	auto_set_Alarm();

	//MPU��ʼ��
	MPU_Init();
	dmp_init();
	//OLED��ʼ��
	I2C_Configuration();
	OLED_Init();
	OLED_CLS();
	
	//������ʾ
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
			//��ʾʱ��
			showtime_date();
		}
		else if(flag==1)
		{
			if(clean!=1)
			{
				OLED_CLS();
				clean = 1;
			}
			//��ʾ�Ʋ���
			show_pedometer_step_count();
		}
		else if(flag==2)
		{
			if(clean!=2)
			{
				OLED_CLS();
				clean = 2;
			}
			//�ı�����ʱ��
			change();
			//��ʾ����ʱ��
			Alarm_time();
			
		}
		else if(flag==3)
		{
			if(clean!=3)
			{
				OLED_CLS();
				clean = 3;
			}
			//��ʾ����ͼ
			show_heart_rate();
		}
			
	}
	
	return 0;
}


