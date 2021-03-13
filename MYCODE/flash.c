#include "flash.h"



static uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;
  
  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_Sector_0;  
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_Sector_1;  
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_Sector_2;  
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_Sector_3;  
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_Sector_4;  
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_Sector_5;  
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_Sector_6;  
  }
  else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
  {
    sector = FLASH_Sector_7;  
  }
 
  return sector;
}




void Flash_Write(u32 addr, u8 *write_buff, u32 writetolen)
{
	
	u32 FLASH_USER_START_ADDR, FLASH_USER_END_ADDR, uwAddress; //开始与结束地址
	u32 uwStartSector, uwEndSector, uwSectorCounter;  //扇区变量
	
	/* Unlock the Flash *********************************************************/
	/* Enable the flash control register access */
	//解锁
	FLASH_Unlock();
	
	/* Erase the user Flash area ************************************************/
	/* area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR */
	
	/* Clear pending flags (if any) */  
	//清空标志位
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
					FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 
	
	
	//开始地址
	FLASH_USER_START_ADDR = addr;
	//结束地址
	FLASH_USER_END_ADDR = addr + writetolen;
	
	//通过地址获取要擦除的扇区
	/* Get the number of the start and end sectors */
	uwStartSector = GetSector(FLASH_USER_START_ADDR);
	uwEndSector = GetSector(FLASH_USER_END_ADDR);
	
	/* Strat the erase operation */
	uwSectorCounter = uwStartSector;
	while (uwSectorCounter <= uwEndSector)   //循环擦除扇区
	{
		/* Device voltage range supposed to be [2.7V to 3.6V], the operation will
			be done by word */ 
			//擦除扇区
		if (FLASH_EraseSector(uwSectorCounter, VoltageRange_3) != FLASH_COMPLETE)
		{ 
			/* Error occurred while sector erase. 
			User can add here some code to deal with this error  */
			printf("erase error\n");
			return ;
		}

		uwSectorCounter += 8;
		
	}
	
	//开始地址赋值
	uwAddress = FLASH_USER_START_ADDR;
	
	//循环写入数据
	while (uwAddress < FLASH_USER_END_ADDR)
	{
		//判断是否成功写入数据
		if (FLASH_ProgramByte(uwAddress, *write_buff) == FLASH_COMPLETE)
		{
			uwAddress = uwAddress + 1;  //FLASH地址要加1
			write_buff++;  //数据地址加1
		}
		else
		{ 

			printf("write failure\n");
			return ;
				
		}
	}
	
	//加锁
	FLASH_Lock();


}


void Flash_Read(u32 addr, u8 *read_buff, u32 readtolen)
{
	u32 FLASH_USER_START_ADDR, FLASH_USER_END_ADDR, uwAddress;

	//开始地址
	FLASH_USER_START_ADDR = addr;
	//结束地址
	FLASH_USER_END_ADDR = addr + readtolen;

	uwAddress = FLASH_USER_START_ADDR;
	
	while (uwAddress < FLASH_USER_END_ADDR)
	{
		*read_buff = *(__IO uint8_t*)uwAddress;

		uwAddress = uwAddress + 1;
		
		read_buff++;
	}  

	
}