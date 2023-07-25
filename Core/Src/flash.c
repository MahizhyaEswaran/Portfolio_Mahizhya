/*
 * Interfacing Functions to store data in STM32 internal FLASH
 * By: Anistus H
 * 2023/06/19
 */

#include "flash.h"
#include <string.h>

uint32_t Flash_Read_Data(uint32_t address, uint8_t* data, uint32_t length)
{
	memcpy(data,(void *)address,length);
	return 0;
}

uint32_t Flash_Write_Data(uint32_t address, uint32_t *fdata, uint32_t length){
	uint32_t idx = 0;

	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR );

	while(idx < length){
		if((HAL_FLASH_Program(FLASH_TYPEPROGRAMDATA_WORD, address, fdata[idx/4])) == HAL_OK){
			address += 4;
			idx += 4;

		}else{
			HAL_FLASH_Lock();
			return HAL_FLASH_GetError();
		}
	}
	HAL_FLASH_Lock();
	return 0;
}

uint32_t Flash_Erase_Page(uint32_t address, uint32_t numOfPages){
	FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t PageError;

	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR );

	EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.PageAddress = address;
	EraseInitStruct.NbPages = numOfPages;

	if((HAL_FLASHEx_Erase(&EraseInitStruct, &PageError)) != HAL_OK){
		HAL_FLASH_Lock();
		return HAL_FLASH_GetError();
	}
	HAL_FLASH_Lock();
	return 0;
}
