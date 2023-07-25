/*
 * Interfacing Functions to store data in STM32 internal FLASH
 * By: Anistus H
 * 2023/06/19
 */

#ifndef __FLASH_H__
#define __FLASH_H__

//Includes
#include "main.h"

//Prototyping
uint32_t Flash_Read_Data(uint32_t address, uint8_t* data, uint32_t length);
uint32_t Flash_Write_Data(uint32_t address, uint32_t *fdata, uint32_t length);
uint32_t Flash_Erase_Page(uint32_t address, uint32_t numOfPages);

#endif
