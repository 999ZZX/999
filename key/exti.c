#include "exti.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "sys.h"
#include "OLED_I2C.h"

 u8 flag = 0;
 u8 Timer = 0;//ʱ�����
 u8 OLED_flag = 1;//��Ļ������־������Ϊ1���ر�Ϊ0
/*********************************************
S1(KEY1)���� PA0
����δ���£�PA0Ϊ�ߵ�ƽ
�������£�   PA0Ϊ�͵�ƽ

����������˲�䣬Ϊ�½��ء�
********************************************/
void Exte_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStruct;
	EXTI_InitTypeDef  EXTI_InitStruct;
	NVIC_InitTypeDef  NVIC_InitStruct;
	
	
	//ʹ��SYSCFGʱ�ӣ� 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	//ʹ��GPIO A��ʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_0;		//����0
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_IN;		//����ģʽ
	GPIO_InitStruct.GPIO_PuPd	= GPIO_PuPd_UP;		//����
	GPIO_Init(GPIOA, &GPIO_InitStruct);	
	
	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;		//����2 3 4
	GPIO_Init(GPIOE, &GPIO_InitStruct);	
	
	//����IO�����ж��ߵ�ӳ���ϵ�� PA0 -- EXTT0
   SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);
   SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource2);
   SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource3);
   SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource4);
	
	
	EXTI_InitStruct.EXTI_Line	= EXTI_Line0|EXTI_Line2|EXTI_Line3|EXTI_Line4;			//�ж���0
	EXTI_InitStruct.EXTI_Mode	= EXTI_Mode_Interrupt;	//�ж�ģʽ
	EXTI_InitStruct.EXTI_Trigger= EXTI_Trigger_Falling; //�½��ش���
	EXTI_InitStruct.EXTI_LineCmd= ENABLE;				//�ж���ʹ��
	//��ʼ�������жϣ����ô��������ȡ�
    EXTI_Init(&EXTI_InitStruct);
	
	NVIC_InitStruct.NVIC_IRQChannel						= EXTI0_IRQn;		//�ж�ͨ��
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority	= 2;				//��ռ���ȼ�
 	NVIC_InitStruct.NVIC_IRQChannelSubPriority			= 2;				//��Ӧ���ȼ�
	NVIC_InitStruct.NVIC_IRQChannelCmd					= ENABLE;			//�ж�ͨ��ʹ�� 
	//�����жϷ��飨NVIC������ʹ���жϡ�
    NVIC_Init(&NVIC_InitStruct);
	
	NVIC_InitStruct.NVIC_IRQChannel						= EXTI2_IRQn;		//�ж�ͨ��
	NVIC_Init(&NVIC_InitStruct);

	NVIC_InitStruct.NVIC_IRQChannel						= EXTI3_IRQn;		//�ж�ͨ��
	NVIC_Init(&NVIC_InitStruct);

	NVIC_InitStruct.NVIC_IRQChannel						= EXTI4_IRQn;		//�ж�ͨ��
	NVIC_Init(&NVIC_InitStruct);

}

//�жϷ����������е��жϺ�������startup_stm32f40_41xxx.s����
//�жϷ���������Ҫ����Ա���ã�������������CPU�Զ�����
void  EXTI0_IRQHandler(void)
{
	
	if(EXTI_GetITStatus(EXTI_Line0) == SET)
	{
		delay_ms(15);
		if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)==RESET)
		{
			Timer = 0;
			flag = 1;
			
			if(OLED_flag==0)
			{
				OLED_ON();
			}
			OLED_flag = 1;
		}
	
	}
	//����ж���0��־λ
    EXTI_ClearITPendingBit(EXTI_Line0);
	
}

void  EXTI2_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line2) == SET)
	{
		delay_ms(15);
		if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2)==RESET)
		{
			flag = 2;
			if(OLED_flag==0)
			{
				OLED_ON();
			}
			OLED_flag = 1;
			Timer = 0;
		}
	
	}
	//����ж���0��־λ
    EXTI_ClearITPendingBit(EXTI_Line2);
	
}

void  EXTI3_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line3) == SET)
	{
		delay_ms(15);
		if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_3)==RESET)
		{
			flag = 3;
			if(OLED_flag==0)
			{
				OLED_ON();
			}
			OLED_flag = 1;
			Timer = 0;
			
			
		}
	
	}
	//����ж���0��־λ
    EXTI_ClearITPendingBit(EXTI_Line3);
	
}

void  EXTI4_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line4) == SET)
	{
		delay_ms(15);
		if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)==RESET)
		{
			flag = 0;
			if(OLED_flag==0)
			{
				OLED_ON();
			}
			OLED_flag = 1;
			Timer = 0;
		}
	
	}
	//����ж���0��־λ
    EXTI_ClearITPendingBit(EXTI_Line4);
	
}
















