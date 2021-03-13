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

u32 addr1 = 0x00; //����ʱ��洢��ַ��д��ָ��
u32 addr2 = 0x08; //�����������洢��ַ��д��ָ��
u32 addr3 = 0x16; //���ӿ�����־�洢��ַ������д��1���ر�д��0

RTC_TimeTypeDef  RTC_TimeStruct;
RTC_DateTypeDef  RTC_DateStruct;

void Rtc_Init(void)
{
	RTC_InitTypeDef  RTC_InitStruct;
	
	//1��ʹ��PWRʱ�ӣ�
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	//2��ʹ�ܺ󱸼Ĵ�������,����ϵ�ɱ������ݵļĴ���
	PWR_BackupAccessCmd(ENABLE);
	
	//���󱸼Ĵ��� �󱸼Ĵ����ϵ�ɱ�������
	if(RTC_ReadBackupRegister(RTC_BKP_DR0) != RBD)
	{
	
		//3������RTCʱ��Դ��ʹ��RTCʱ�ӣ�
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
		RCC_RTCCLKCmd(ENABLE);
		//���ʹ��LSE��Ҫ��LSE��
		RCC_LSEConfig(RCC_LSE_ON);
		delay_ms(50);   //��ʱ�ȴ�ʱ��Դ�ȶ���
		
		
		RTC_InitStruct.RTC_HourFormat	= RTC_HourFormat_24; //24Сʱ
		RTC_InitStruct.RTC_AsynchPrediv	= 0x7F;				 //�첽��Ƶ�� 128��Ƶ
		RTC_InitStruct.RTC_SynchPrediv	= 0xFF;				 //ͬ����Ƶ�� 256��Ƶ
		//4�� ��ʼ��RTC(ͬ��/�첽��Ƶϵ����ʱ�Ӹ�ʽ)��
		RTC_Init(&RTC_InitStruct);	
		
		
		RTC_TimeStruct.RTC_H12		= RTC_H12_PM;   //����
		RTC_TimeStruct.RTC_Hours	= 15;			//ʱ
		RTC_TimeStruct.RTC_Minutes	= 25;			//��
		RTC_TimeStruct.RTC_Seconds	= 0;			//��
		//5�� ����ʱ�䣺
		RTC_SetTime(RTC_Format_BIN,&RTC_TimeStruct);
		
		RTC_DateStruct.RTC_Year		= 20; 	//��
		RTC_DateStruct.RTC_Month	= 12;	//��	
		RTC_DateStruct.RTC_Date		= 2; 	//��
		RTC_DateStruct.RTC_WeekDay	= 0x03; //����3
		//6���������ڣ�
		RTC_SetDate(RTC_Format_BIN, &RTC_DateStruct);
		
		//��ֵд��󱸼Ĵ���
		RTC_WriteBackupRegister(RTC_BKP_DR0, RBD);
	}
	
}



//����A����
void RTC_A_Alarm(void)
{
	RTC_AlarmTypeDef  	RTC_AlarmStruct;
	RTC_TimeTypeDef 	RTC_AlarmTime;
	EXTI_InitTypeDef  	EXTI_InitStruct;
	NVIC_InitTypeDef 	NVIC_InitStruct;	
	
	//�ر����ӣ�
	RTC_AlarmCmd(RTC_Alarm_A,DISABLE); 
	
	//����ʱ������
	RTC_AlarmTime.RTC_H12		= RTC_H12_PM;   //����
	RTC_AlarmTime.RTC_Hours		= 15;			//ʱ
	RTC_AlarmTime.RTC_Minutes	= 25;			//��
	RTC_AlarmTime.RTC_Seconds	= 30;			//��
	
	RTC_AlarmStruct.RTC_AlarmTime			= RTC_AlarmTime;		//����ʱ������
	RTC_AlarmStruct.RTC_AlarmMask			= RTC_AlarmMask_None;	//������λ
	RTC_AlarmStruct.RTC_AlarmDateWeekDaySel	= RTC_AlarmDateWeekDaySel_Date; //����������������
	RTC_AlarmStruct.RTC_AlarmDateWeekDay    = 2;

	//�������Ӳ�����
	RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStruct);
	//4���������ӣ�
//	RTC_AlarmCmd(RTC_Alarm_A,ENABLE);
	//5���������������жϣ�
	RTC_ITConfig(RTC_IT_ALRA, ENABLE);
	
	EXTI_InitStruct.EXTI_Line	= EXTI_Line17;			//�ж���17
	EXTI_InitStruct.EXTI_Mode	= EXTI_Mode_Interrupt;	//�ж�ģʽ
	EXTI_InitStruct.EXTI_Trigger= EXTI_Trigger_Rising;  //�����ش���
	EXTI_InitStruct.EXTI_LineCmd= ENABLE;				//�ж���ʹ��
	//��ʼ�������жϣ����ô��������ȡ�
    EXTI_Init(&EXTI_InitStruct);
	
	NVIC_InitStruct.NVIC_IRQChannel						= RTC_Alarm_IRQn;	//�ж�ͨ��
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority	= 2;				//��ռ���ȼ�
 	NVIC_InitStruct.NVIC_IRQChannelSubPriority			= 2;				//��Ӧ���ȼ�
	NVIC_InitStruct.NVIC_IRQChannelCmd					= ENABLE;			//�ж�ͨ��ʹ�� 
	//�����жϷ��飨NVIC������ʹ���жϡ�
    NVIC_Init(&NVIC_InitStruct);

}


//��д�жϷ�������
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
	//����ж���0��־λ
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
			//����ʱ�䣺
			RTC_TimeStruct.RTC_H12		= RTC_H12_PM;   //����
			RTC_TimeStruct.RTC_Hours	= (rx_buffer[2]-'0')*10+(rx_buffer[3]-'0');			//ʱ
			RTC_TimeStruct.RTC_Minutes	= (rx_buffer[4]-'0')*10+(rx_buffer[5]-'0');			//��
			RTC_TimeStruct.RTC_Seconds	= (rx_buffer[6]-'0')*10+(rx_buffer[7]-'0');			//��
		
			RTC_SetTime(RTC_Format_BIN,&RTC_TimeStruct);
		}
		
		else if(rx_buffer[0]=='H'&&rx_buffer[1]=='D')
		{
			//�������ڣ�
			RTC_DateStruct.RTC_Year		= (rx_buffer[2]-'0')*10+(rx_buffer[3]-'0'); 	//��
			RTC_DateStruct.RTC_Month	= (rx_buffer[4]-'0')*10+(rx_buffer[5]-'0');		//��	
			RTC_DateStruct.RTC_Date		= (rx_buffer[6]-'0')*10+(rx_buffer[7]-'0'); 	//��
			RTC_DateStruct.RTC_WeekDay	= (rx_buffer[8]-'0'); 							//����
		
			RTC_SetDate(RTC_Format_BIN, &RTC_DateStruct);
		}		

		else if(rx_buffer[0]=='H' && rx_buffer[1]=='A')
		{
			//��������ʱ��
			//�ر����ӣ�
			RTC_AlarmCmd(RTC_Alarm_A,DISABLE); 
			
			//����ʱ������
			RTC_AlarmTime.RTC_H12		= RTC_H12_PM;   //����
			RTC_AlarmTime.RTC_Hours		= (rx_buffer[2]-'0')*10+(rx_buffer[3]-'0');			//ʱ
			RTC_AlarmTime.RTC_Minutes	= (rx_buffer[4]-'0')*10+(rx_buffer[5]-'0');			//��
			RTC_AlarmTime.RTC_Seconds	= (rx_buffer[6]-'0')*10+(rx_buffer[7]-'0');			//��
			
			RTC_AlarmStruct.RTC_AlarmTime			= RTC_AlarmTime;		//����ʱ������
			//�������Ӳ�����
			RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStruct);
			//4���������ӣ�
			RTC_AlarmCmd(RTC_Alarm_A,ENABLE);
		    //������ʱ��д��AT24C02
			AT24c02_Write(addr1,rx_buffer,8);
			
		}
		
		else if(rx_buffer[0]=='H'&&rx_buffer[1]=='C')
		{
			//���������д��AT24C02
			AT24c02_Write(addr2,rx_buffer,8);
			
			if(rx_buffer[2]=='D')
			{
				//�������Ӱ�������
				
				//�ر����ӣ�
				RTC_AlarmCmd(RTC_Alarm_A,DISABLE); 
				
				RTC_AlarmStruct.RTC_AlarmDateWeekDaySel	= RTC_AlarmDateWeekDaySel_Date; 	//����������������
				RTC_AlarmStruct.RTC_AlarmDateWeekDay    = (rx_buffer[3]-'0')*10+(rx_buffer[4]-'0');
				
				//�������Ӳ�����
				RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStruct);
				//4���������ӣ�
				RTC_AlarmCmd(RTC_Alarm_A,ENABLE);
				
			}
			
			else if(rx_buffer[2]=='W')
			{
				//�������Ӱ�������
				
				//�ر����ӣ�
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
			
				RTC_AlarmStruct.RTC_AlarmDateWeekDaySel	= RTC_AlarmDateWeekDaySel_WeekDay; 	//����������������
		
				//�������Ӳ�����
				RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStruct);
				//4���������ӣ�
//				RTC_AlarmCmd(RTC_Alarm_A,ENABLE);
				//��ȡ���ӱ�־
				u8 read_buff[8] = {0};
				AT24c02_Read(addr3, read_buff, 8);
				if(read_buff[0]=='1')
				{
					//������
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
	
	//��ȡ����ʱ��
	AT24c02_Read(addr1, read_buff, 8);
	//�ر����ӣ�
	RTC_AlarmCmd(RTC_Alarm_A,DISABLE); 
	
	//����ʱ������
	RTC_AlarmTime.RTC_H12		= RTC_H12_PM;   //����
	RTC_AlarmTime.RTC_Hours		= (read_buff[2]-'0')*10+(read_buff[3]-'0');			//ʱ
	RTC_AlarmTime.RTC_Minutes	= (read_buff[4]-'0')*10+(read_buff[5]-'0');			//��
	RTC_AlarmTime.RTC_Seconds	= (read_buff[6]-'0')*10+(read_buff[7]-'0');			//��
	
	RTC_AlarmStruct.RTC_AlarmTime			= RTC_AlarmTime;		//����ʱ������
	//�������Ӳ�����
	RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStruct);
	
	delay_ms(10);
	
	//��ȡ����ʱ��
	AT24c02_Read(addr2, read_buff, 8);
	if(read_buff[2]=='D')
	{
		//�������Ӱ�������
		
		//�ر����ӣ�
		RTC_AlarmCmd(RTC_Alarm_A,DISABLE); 
		
		RTC_AlarmStruct.RTC_AlarmDateWeekDaySel	= RTC_AlarmDateWeekDaySel_Date; 	//����������������
		RTC_AlarmStruct.RTC_AlarmDateWeekDay    = (read_buff[3]-'0')*10+(read_buff[4]-'0');
		
		//�������Ӳ�����
		RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStruct);
	}
	
	else if(read_buff[2]=='W')
	{
		//�������Ӱ�������
		
		//�ر����ӣ�
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
		
		RTC_AlarmStruct.RTC_AlarmDateWeekDaySel	= RTC_AlarmDateWeekDaySel_WeekDay; 	//����������������

		//�������Ӳ�����
		RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStruct);
	}
	//��ȡ���ӱ�־
	AT24c02_Read(addr3, read_buff, 1);
	if(read_buff[0]=='1')
	{
		//������
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
			//�ر�����
			RTC_AlarmCmd(RTC_Alarm_A,DISABLE);
			//д���־
			u8 buf[8] = {"0"};
			AT24c02_Write(addr3,buf,8);
		}
		else if(rx_buffer[0]=='S'&&rx_buffer[1]=='T'&&rx_buffer[2]=='A'&&rx_buffer[3]=='R'&&rx_buffer[4]=='T')
		{
			//��������
			RTC_AlarmCmd(RTC_Alarm_A,ENABLE);
			//д���־
			u8 buf[8] = {"1"};
			AT24c02_Write(addr3,buf,8);
		}
	}
}

