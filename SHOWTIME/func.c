#include "func.h"
#include "OLED_I2C.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "delay.h"
#include "led.h"

char timestr[20] = {0}; 
char datestr[20] = {0};
extern u8 Timer;//时间计数
extern u8 OLED_flag;//屏幕开启标志，开启为1，关闭为0；

void showtime_date(void)
{
		uint32_t RTC_Format;
		RTC_DateTypeDef RTC_DateStruct;
		RTC_TimeTypeDef RTC_TimeStruct;
	
		RTC_GetDate(RTC_Format,&RTC_DateStruct);
		RTC_GetTime(RTC_Format,&RTC_TimeStruct);
		
		sprintf(datestr,"20%d-%d-%d",RTC_DateStruct.RTC_Year,
						RTC_DateStruct.RTC_Month,RTC_DateStruct.RTC_Date);
	
		if(RTC_TimeStruct.RTC_Seconds<=9||RTC_TimeStruct.RTC_Minutes<=9||RTC_TimeStruct.RTC_Hours<=9)
		{
			if(RTC_TimeStruct.RTC_Seconds<=9&&RTC_TimeStruct.RTC_Minutes>9&&RTC_TimeStruct.RTC_Hours>9)
			{
				sprintf(timestr,"%d:%d:%d%d",RTC_TimeStruct.RTC_Hours,
						RTC_TimeStruct.RTC_Minutes,0,RTC_TimeStruct.RTC_Seconds);
			}
			
			else if(RTC_TimeStruct.RTC_Seconds>9&&RTC_TimeStruct.RTC_Minutes<=9&&RTC_TimeStruct.RTC_Hours>9)
			{
				sprintf(timestr,"%d:%d%d:%d",RTC_TimeStruct.RTC_Hours,0,
						RTC_TimeStruct.RTC_Minutes,RTC_TimeStruct.RTC_Seconds);
			}
			else if(RTC_TimeStruct.RTC_Seconds>9&&RTC_TimeStruct.RTC_Minutes>9&&RTC_TimeStruct.RTC_Hours<=9)
			{
				sprintf(timestr," %d:%d:%d",RTC_TimeStruct.RTC_Hours,
						RTC_TimeStruct.RTC_Minutes,RTC_TimeStruct.RTC_Seconds);
			}
				
		}
		else
		{
			sprintf(timestr,"%d:%d:%d",RTC_TimeStruct.RTC_Hours,
						RTC_TimeStruct.RTC_Minutes,RTC_TimeStruct.RTC_Seconds);
		}
		
		OLED_ShowStr(0,0,datestr,2);
		OLED_ShowCN(79,6,7);
		OLED_ShowCN(95,6,8);
		OLED_ShowCN(111,6,(RTC_DateStruct.RTC_WeekDay)-1);
		OLED_ShowStr(35,3,timestr,2);
		
		//自动关闭屏幕
		Timer++;
		if(Timer==30)
		{
			OLED_OFF();
			OLED_flag = 0;
		}
		
		delay_s(1);
}