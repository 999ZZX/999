#include "rtc.h"
#include "delay.h"
#include <string.h>
#include "OLED_I2C.h"
#include "iic.h"
#include "led.h"

#define RBD		0x2020

extern u8 rx_flag;
extern u8 rx_buffer[64];
uint32_t Alarm_mode ;

u32 addr1 = 0x00; //闹钟时间存储地址，写入指令
u32 addr2 = 0x08; //闹钟响铃规则存储地址，写入指令
u32 addr3 = 0x16; //闹钟开启标志存储地址，开启写入1，关闭写入0

RTC_TimeTypeDef  RTC_TimeStruct;
RTC_DateTypeDef  RTC_DateStruct;

void Rtc_Init(void)
{
	RTC_InitTypeDef  RTC_InitStruct;
	
	//1、使能PWR时钟：
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	//2、使能后备寄存器访问,这个断电可保存数据的寄存器
	PWR_BackupAccessCmd(ENABLE);
	
	//读后备寄存器 后备寄存器断电可保存数据
	if(RTC_ReadBackupRegister(RTC_BKP_DR0) != RBD)
	{
	
		//3、配置RTC时钟源，使能RTC时钟：
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
		RCC_RTCCLKCmd(ENABLE);
		//如果使用LSE，要打开LSE：
		RCC_LSEConfig(RCC_LSE_ON);
		delay_ms(50);   //延时等待时钟源稳定。
		
		
		RTC_InitStruct.RTC_HourFormat	= RTC_HourFormat_24; //24小时
		RTC_InitStruct.RTC_AsynchPrediv	= 0x7F;				 //异步分频器 128分频
		RTC_InitStruct.RTC_SynchPrediv	= 0xFF;				 //同步分频器 256分频
		//4、 初始化RTC(同步/异步分频系数和时钟格式)：
		RTC_Init(&RTC_InitStruct);	
		
		
		RTC_TimeStruct.RTC_H12		= RTC_H12_PM;   //下午
		RTC_TimeStruct.RTC_Hours	= 15;			//时
		RTC_TimeStruct.RTC_Minutes	= 25;			//分
		RTC_TimeStruct.RTC_Seconds	= 0;			//秒
		//5、 设置时间：
		RTC_SetTime(RTC_Format_BIN,&RTC_TimeStruct);
		
		RTC_DateStruct.RTC_Year		= 20; 	//年
		RTC_DateStruct.RTC_Month	= 12;	//月	
		RTC_DateStruct.RTC_Date		= 2; 	//日
		RTC_DateStruct.RTC_WeekDay	= 0x03; //星期3
		//6、设置日期：
		RTC_SetDate(RTC_Format_BIN, &RTC_DateStruct);
		
		//将值写入后备寄存器
		RTC_WriteBackupRegister(RTC_BKP_DR0, RBD);
	}
	
}



//闹钟A设置
void RTC_A_Alarm(void)
{
	RTC_AlarmTypeDef  	RTC_AlarmStruct;
	RTC_TimeTypeDef 	RTC_AlarmTime;
	EXTI_InitTypeDef  	EXTI_InitStruct;
	NVIC_InitTypeDef 	NVIC_InitStruct;	
	
	//关闭闹钟：
	RTC_AlarmCmd(RTC_Alarm_A,DISABLE); 
	
	//闹钟时间设置
	RTC_AlarmTime.RTC_H12		= RTC_H12_PM;   //下午
	RTC_AlarmTime.RTC_Hours		= 15;			//时
	RTC_AlarmTime.RTC_Minutes	= 25;			//分
	RTC_AlarmTime.RTC_Seconds	= 30;			//秒
	
	RTC_AlarmStruct.RTC_AlarmTime			= RTC_AlarmTime;		//闹钟时间设置
	RTC_AlarmStruct.RTC_AlarmMask			= RTC_AlarmMask_None;	//无掩码位
	RTC_AlarmStruct.RTC_AlarmDateWeekDaySel	= RTC_AlarmDateWeekDaySel_Date; //按日期来设置闹钟
	RTC_AlarmStruct.RTC_AlarmDateWeekDay    = 2;

	//配置闹钟参数：
	RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStruct);
	//4、开启闹钟：
//	RTC_AlarmCmd(RTC_Alarm_A,ENABLE);
	//5、开启配置闹钟中断：
	RTC_ITConfig(RTC_IT_ALRA, ENABLE);
	
	EXTI_InitStruct.EXTI_Line	= EXTI_Line17;			//中断线17
	EXTI_InitStruct.EXTI_Mode	= EXTI_Mode_Interrupt;	//中断模式
	EXTI_InitStruct.EXTI_Trigger= EXTI_Trigger_Rising;  //上升沿触发
	EXTI_InitStruct.EXTI_LineCmd= ENABLE;				//中断线使能
	//初始化线上中断，设置触发条件等。
    EXTI_Init(&EXTI_InitStruct);
	
	NVIC_InitStruct.NVIC_IRQChannel						= RTC_Alarm_IRQn;	//中断通道
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority	= 2;				//抢占优先级
 	NVIC_InitStruct.NVIC_IRQChannelSubPriority			= 2;				//响应优先级
	NVIC_InitStruct.NVIC_IRQChannelCmd					= ENABLE;			//中断通道使能 
	//配置中断分组（NVIC），并使能中断。
    NVIC_Init(&NVIC_InitStruct);

}


//编写中断服务函数：
void RTC_Alarm_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line17) == SET)
	{
		if(RTC_GetFlagStatus(RTC_FLAG_ALRAF) == SET)
		{
			LED0_ON;
			LED1_ON;
			LED2_ON;
			LED3_ON;
			RTC_ClearFlag(RTC_FLAG_ALRAF);
		}
	
	}
	//清除中断线0标志位
    EXTI_ClearITPendingBit(EXTI_Line17);

}

RTC_AlarmTypeDef  	RTC_AlarmStruct;
RTC_TimeTypeDef 	RTC_AlarmTime;

void change(void)
{
	if(rx_flag == 1)
	{
		if(rx_buffer[0]=='H'&&rx_buffer[1]=='T') 
		{
			//设置时间：
			RTC_TimeStruct.RTC_H12		= RTC_H12_PM;   //下午
			RTC_TimeStruct.RTC_Hours	= (rx_buffer[2]-'0')*10+(rx_buffer[3]-'0');			//时
			RTC_TimeStruct.RTC_Minutes	= (rx_buffer[4]-'0')*10+(rx_buffer[5]-'0');			//分
			RTC_TimeStruct.RTC_Seconds	= (rx_buffer[6]-'0')*10+(rx_buffer[7]-'0');			//秒
		
			RTC_SetTime(RTC_Format_BIN,&RTC_TimeStruct);
		}
		
		else if(rx_buffer[0]=='H'&&rx_buffer[1]=='D')
		{
			//设置日期：
			RTC_DateStruct.RTC_Year		= (rx_buffer[2]-'0')*10+(rx_buffer[3]-'0'); 	//年
			RTC_DateStruct.RTC_Month	= (rx_buffer[4]-'0')*10+(rx_buffer[5]-'0');		//月	
			RTC_DateStruct.RTC_Date		= (rx_buffer[6]-'0')*10+(rx_buffer[7]-'0'); 	//日
			RTC_DateStruct.RTC_WeekDay	= (rx_buffer[8]-'0'); 							//星期
		
			RTC_SetDate(RTC_Format_BIN, &RTC_DateStruct);
		}		

		else if(rx_buffer[0]=='H' && rx_buffer[1]=='A')
		{
			//设置闹钟时间
			//关闭闹钟：
			RTC_AlarmCmd(RTC_Alarm_A,DISABLE); 
			
			//闹钟时间设置
			RTC_AlarmTime.RTC_H12		= RTC_H12_PM;   //下午
			RTC_AlarmTime.RTC_Hours		= (rx_buffer[2]-'0')*10+(rx_buffer[3]-'0');			//时
			RTC_AlarmTime.RTC_Minutes	= (rx_buffer[4]-'0')*10+(rx_buffer[5]-'0');			//分
			RTC_AlarmTime.RTC_Seconds	= (rx_buffer[6]-'0')*10+(rx_buffer[7]-'0');			//秒
			
			RTC_AlarmStruct.RTC_AlarmTime			= RTC_AlarmTime;		//闹钟时间设置
			//配置闹钟参数：
			RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStruct);
			//4、开启闹钟：
			RTC_AlarmCmd(RTC_Alarm_A,ENABLE);
		    //将闹钟时间写入AT24C02
			AT24c02_Write(addr1,rx_buffer,8);
			
		}
		
		else if(rx_buffer[0]=='H'&&rx_buffer[1]=='C')
		{
			//将响铃规则写入AT24C02
			AT24c02_Write(addr2,rx_buffer,8);
			
			if(rx_buffer[2]=='D')
			{
				//设置闹钟按日期响
				
				//关闭闹钟：
				RTC_AlarmCmd(RTC_Alarm_A,DISABLE); 
				
				RTC_AlarmStruct.RTC_AlarmDateWeekDaySel	= RTC_AlarmDateWeekDaySel_Date; 	//按日期来设置闹钟
				RTC_AlarmStruct.RTC_AlarmDateWeekDay    = (rx_buffer[3]-'0')*10+(rx_buffer[4]-'0');
				
				//配置闹钟参数：
				RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStruct);
				//4、开启闹钟：
				RTC_AlarmCmd(RTC_Alarm_A,ENABLE);
				
			}
			
			else if(rx_buffer[2]=='W')
			{
				//设置闹钟按星期响
				
				//关闭闹钟：
				RTC_AlarmCmd(RTC_Alarm_A,DISABLE); 
				
				if((rx_buffer[3]-'0')==1)
				{
					RTC_AlarmStruct.RTC_AlarmDateWeekDay    = RTC_Weekday_Monday;
				}
				else if((rx_buffer[3]-'0')==2)
				{
					RTC_AlarmStruct.RTC_AlarmDateWeekDay    = RTC_Weekday_Tuesday;
				}
				
				else if((rx_buffer[3]-'0')==3)
				{
					RTC_AlarmStruct.RTC_AlarmDateWeekDay    = RTC_Weekday_Wednesday;
				}
				
				else if((rx_buffer[3]-'0')==4)
				{
					RTC_AlarmStruct.RTC_AlarmDateWeekDay    = RTC_Weekday_Thursday;
				}
				
				else if((rx_buffer[3]-'0')==5)
				{
					RTC_AlarmStruct.RTC_AlarmDateWeekDay    = RTC_Weekday_Friday;
				}
				
				else if((rx_buffer[3]-'0')==6)
				{
					RTC_AlarmStruct.RTC_AlarmDateWeekDay    = RTC_Weekday_Saturday;
				}
				
				else if((rx_buffer[3]-'0')==7)
				{
					RTC_AlarmStruct.RTC_AlarmDateWeekDay    = RTC_Weekday_Sunday;
				}
			
				RTC_AlarmStruct.RTC_AlarmDateWeekDaySel	= RTC_AlarmDateWeekDaySel_WeekDay; 	//按星期来设置闹钟
		
				//配置闹钟参数：
				RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStruct);
				//4、开启闹钟：
//				RTC_AlarmCmd(RTC_Alarm_A,ENABLE);
				//读取闹钟标志
				u8 read_buff[8] = {0};
				AT24c02_Read(addr3, read_buff, 8);
				if(read_buff[0]=='1')
				{
					//打开闹钟
					RTC_AlarmCmd(RTC_Alarm_A,ENABLE);
					u8 buf[8] = {"1"};
					AT24c02_Write(addr3,buf,8);
				}
			}
		}

		memset(rx_buffer,0, sizeof(rx_buffer));
		rx_flag = 0;
	}
}

void Alarm_time()
{
	u8 buffer[8] = {0};
	AT24c02_Read(addr3,buffer,8);
	
	char timestr[20] = {0}; 
	char datestr[20] = {0};
	RTC_AlarmTypeDef RTC_AlarmStruct;
	
	OLED_ShowCN(0,0,19);
	OLED_ShowCN(0,2,20);
	if(buffer[0]=='1')
	{
		OLED_ShowCN(0,5,21);
	}
	else
	{
		OLED_ShowCN(0,5,22);
	}
	
	RTC_GetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStruct);
	if(RTC_AlarmStruct.RTC_AlarmTime.RTC_Hours<=9||RTC_AlarmStruct.RTC_AlarmTime.RTC_Minutes<=9||
		RTC_AlarmStruct.RTC_AlarmTime.RTC_Seconds<=9)
	{
		if(RTC_AlarmStruct.RTC_AlarmTime.RTC_Hours<=9&&RTC_AlarmStruct.RTC_AlarmTime.RTC_Minutes>9&&
			RTC_AlarmStruct.RTC_AlarmTime.RTC_Seconds>9)
		{
			sprintf(timestr,"%d%d:%d:%d",0,RTC_AlarmStruct.RTC_AlarmTime.RTC_Hours,RTC_AlarmStruct.RTC_AlarmTime.RTC_Minutes,
			RTC_AlarmStruct.RTC_AlarmTime.RTC_Seconds);
		}
		else if(RTC_AlarmStruct.RTC_AlarmTime.RTC_Hours>9&&RTC_AlarmStruct.RTC_AlarmTime.RTC_Minutes<=9&&
			RTC_AlarmStruct.RTC_AlarmTime.RTC_Seconds>9)
		{
			sprintf(timestr,"%d:%d%d:%d",RTC_AlarmStruct.RTC_AlarmTime.RTC_Hours,0,RTC_AlarmStruct.RTC_AlarmTime.RTC_Minutes,
			RTC_AlarmStruct.RTC_AlarmTime.RTC_Seconds);
		}
		else if(RTC_AlarmStruct.RTC_AlarmTime.RTC_Hours>9&&RTC_AlarmStruct.RTC_AlarmTime.RTC_Minutes>9&&
			RTC_AlarmStruct.RTC_AlarmTime.RTC_Seconds<=9)
		{
			sprintf(timestr,"%d:%d:%d%d",RTC_AlarmStruct.RTC_AlarmTime.RTC_Hours,RTC_AlarmStruct.RTC_AlarmTime.RTC_Minutes,0,
			RTC_AlarmStruct.RTC_AlarmTime.RTC_Seconds);
		}
	}
	
	else
	{
		sprintf(timestr,"%d:%d:%d",RTC_AlarmStruct.RTC_AlarmTime.RTC_Hours,RTC_AlarmStruct.RTC_AlarmTime.RTC_Minutes,
			RTC_AlarmStruct.RTC_AlarmTime.RTC_Seconds);
	}

	OLED_ShowStr(35,1,timestr,2);
	u8 buf[1] = {0};
	if(RTC_AlarmStruct.RTC_AlarmDateWeekDaySel==RTC_AlarmDateWeekDaySel_Date)
	{
		if(RTC_AlarmStruct.RTC_AlarmDateWeekDay<10)
		{
			sprintf(buf," %d",RTC_AlarmStruct.RTC_AlarmDateWeekDay);
		}
		else
			sprintf(buf,"%d",RTC_AlarmStruct.RTC_AlarmDateWeekDay);
		
		OLED_ShowCN(35,5,13);
		OLED_ShowCN(51,5,14);
		OLED_ShowStr(66,5,buf,2);
		OLED_ShowCN(84,5,15);
		OLED_ShowCN(100,5,16);
	}
	
	if(RTC_AlarmStruct.RTC_AlarmDateWeekDaySel==RTC_AlarmDateWeekDaySel_WeekDay)
	{
		
		if(RTC_AlarmStruct.RTC_AlarmDateWeekDay==RTC_Weekday_Monday)
		{
			
			OLED_ShowCN(35,5,13);
			OLED_ShowCN(51,5,17);
			OLED_ShowCN(66,5,0);
			OLED_ShowCN(84,5,16);
			OLED_ShowCN(100,5,18);
		}
		else if(RTC_AlarmStruct.RTC_AlarmDateWeekDay==RTC_Weekday_Tuesday)
		{
			
			OLED_ShowCN(35,5,13);
			OLED_ShowCN(51,5,17);
			OLED_ShowCN(66,5,1);
			OLED_ShowCN(84,5,16);
			OLED_ShowCN(100,5,18);
		}
		else if(RTC_AlarmStruct.RTC_AlarmDateWeekDay==RTC_Weekday_Wednesday)
		{
			
			OLED_ShowCN(35,5,13);
			OLED_ShowCN(51,5,17);
			OLED_ShowCN(66,5,2);
			OLED_ShowCN(84,5,16);
			OLED_ShowCN(100,5,18);
		}
		else if(RTC_AlarmStruct.RTC_AlarmDateWeekDay==RTC_Weekday_Thursday)
		{
			
			OLED_ShowCN(35,5,13);
			OLED_ShowCN(51,5,17);
			OLED_ShowCN(66,5,3);
			OLED_ShowCN(84,5,16);
			OLED_ShowCN(100,5,18);
		}
		else if(RTC_AlarmStruct.RTC_AlarmDateWeekDay==RTC_Weekday_Friday)
		{
			
			OLED_ShowCN(35,5,13);
			OLED_ShowCN(51,5,17);
			OLED_ShowCN(66,5,4);
			OLED_ShowCN(84,5,16);
			OLED_ShowCN(100,5,18);
		}
		else if(RTC_AlarmStruct.RTC_AlarmDateWeekDay==RTC_Weekday_Saturday)
		{
			
			OLED_ShowCN(35,5,13);
			OLED_ShowCN(51,5,17);
			OLED_ShowCN(66,5,5);
			OLED_ShowCN(84,5,16);
			OLED_ShowCN(100,5,18);
		}
		else if(RTC_AlarmStruct.RTC_AlarmDateWeekDay==RTC_Weekday_Sunday)
		{
			
			OLED_ShowCN(35,5,13);
			OLED_ShowCN(51,5,17);
			OLED_ShowCN(66,5,6);
			OLED_ShowCN(84,5,16);
			OLED_ShowCN(100,5,18);
		}
		
	}
	delay_s(1);
}

void auto_set_Alarm()
{
	u8 read_buff[8] = {0};
	
	//读取闹钟时间
	AT24c02_Read(addr1, read_buff, 8);
	//关闭闹钟：
	RTC_AlarmCmd(RTC_Alarm_A,DISABLE); 
	
	//闹钟时间设置
	RTC_AlarmTime.RTC_H12		= RTC_H12_PM;   //下午
	RTC_AlarmTime.RTC_Hours		= (read_buff[2]-'0')*10+(read_buff[3]-'0');			//时
	RTC_AlarmTime.RTC_Minutes	= (read_buff[4]-'0')*10+(read_buff[5]-'0');			//分
	RTC_AlarmTime.RTC_Seconds	= (read_buff[6]-'0')*10+(read_buff[7]-'0');			//秒
	
	RTC_AlarmStruct.RTC_AlarmTime			= RTC_AlarmTime;		//闹钟时间设置
	//配置闹钟参数：
	RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStruct);
	
	delay_ms(10);
	
	//读取闹钟时间
	AT24c02_Read(addr2, read_buff, 8);
	if(read_buff[2]=='D')
	{
		//设置闹钟按日期响
		
		//关闭闹钟：
		RTC_AlarmCmd(RTC_Alarm_A,DISABLE); 
		
		RTC_AlarmStruct.RTC_AlarmDateWeekDaySel	= RTC_AlarmDateWeekDaySel_Date; 	//按日期来设置闹钟
		RTC_AlarmStruct.RTC_AlarmDateWeekDay    = (read_buff[3]-'0')*10+(read_buff[4]-'0');
		
		//配置闹钟参数：
		RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStruct);
	}
	
	else if(read_buff[2]=='W')
	{
		//设置闹钟按星期响
		
		//关闭闹钟：
		RTC_AlarmCmd(RTC_Alarm_A,DISABLE); 
		if((read_buff[3]-'0')==1)
		{
			RTC_AlarmStruct.RTC_AlarmDateWeekDay    = RTC_Weekday_Monday;
		}
		else if((read_buff[3]-'0')==2)
		{
			RTC_AlarmStruct.RTC_AlarmDateWeekDay    = RTC_Weekday_Tuesday;
		}
		
		else if((read_buff[3]-'0')==3)
		{
			RTC_AlarmStruct.RTC_AlarmDateWeekDay    = RTC_Weekday_Wednesday;
		}
		
		else if((read_buff[3]-'0')==4)
		{
			RTC_AlarmStruct.RTC_AlarmDateWeekDay    = RTC_Weekday_Thursday;
		}
		
		else if((read_buff[3]-'0')==5)
		{
			RTC_AlarmStruct.RTC_AlarmDateWeekDay    = RTC_Weekday_Friday;
		}
		
		else if((read_buff[3]-'0')==6)
		{
			RTC_AlarmStruct.RTC_AlarmDateWeekDay    = RTC_Weekday_Saturday;
		}
		
		else if((read_buff[3]-'0')==7)
		{
			RTC_AlarmStruct.RTC_AlarmDateWeekDay    = RTC_Weekday_Sunday;
		}
		
		RTC_AlarmStruct.RTC_AlarmDateWeekDaySel	= RTC_AlarmDateWeekDaySel_WeekDay; 	//按星期来设置闹钟

		//配置闹钟参数：
		RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStruct);
	}
	//读取闹钟标志
	AT24c02_Read(addr3, read_buff, 1);
	if(read_buff[0]=='1')
	{
		//打开闹钟
		RTC_AlarmCmd(RTC_Alarm_A,ENABLE);
		u8 buf[8] = {"1"};
		AT24c02_Write(addr3,buf,8);
	}
	
}

void stop_start_Alarm()
{
	if(rx_flag==1)
	{
		if(rx_buffer[0]=='S'&&rx_buffer[1]=='T'&&rx_buffer[2]=='O'&&rx_buffer[3]=='P')
		{
			//关闭闹钟
			RTC_AlarmCmd(RTC_Alarm_A,DISABLE);
			//写入标志
			u8 buf[8] = {"0"};
			AT24c02_Write(addr3,buf,8);
		}
		else if(rx_buffer[0]=='S'&&rx_buffer[1]=='T'&&rx_buffer[2]=='A'&&rx_buffer[3]=='R'&&rx_buffer[4]=='T')
		{
			//开启闹钟
			RTC_AlarmCmd(RTC_Alarm_A,ENABLE);
			//写入标志
			u8 buf[8] = {"1"};
			AT24c02_Write(addr3,buf,8);
		}
	}
}

