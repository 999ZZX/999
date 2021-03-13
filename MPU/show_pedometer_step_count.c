#include "show_pedometer_step_count.h"
#include "OLED_I2C.h"
#include "mpu6050.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "stm32f4xx.h"
#include "delay.h"
#include "usart.h"
#include <string.h>

unsigned long Count = 0;
unsigned char buf[10] = {0};

void show_pedometer_step_count()
{	
	dmp_get_pedometer_step_count(&Count);
	printf("count=%ld\n",Count);
	
	sprintf(buf,"%ld",Count);
	OLED_ShowCN(0,3,9);
	OLED_ShowCN(16,3,10);
	OLED_ShowCN(32,3,11);
	OLED_ShowCN(48,3,12);
	OLED_ShowStr(70,3,buf,2);
	delay_ms(400);
}


void dmp_init()
{
	while(mpu_dmp_init())
		delay_ms(500);
	
	dmp_set_pedometer_step_count(Count);
}
