/*
 * Configuration block to be used by Bootloader
 * And the user application
 * By: Anistus H
 * 2023/07/13
 */

#include <bl_config_block.h>

// Initialize the config block
int bl_config_block_init(BL_Config_Block *q) {
	read_bl_config_block(q);
	if((q->block_start == 8) && (q->block_end == 17)){
		//Already available
		return 1;
	}else{
		//Not available
		memset(q, 0 , sizeof(BL_Config_Block));
		q->block_start = 8;
		q->block_end = 17;
		save_bl_config_block(q);
		return 2;
	}
}

// Store config block to flash
uint8_t save_bl_config_block(BL_Config_Block *q) {
	Flash_Erase_Page(BL_Config_Address, 2);
    Flash_Write_Data(BL_Config_Address , (uint32_t *) q, sizeof(BL_Config_Block));
    return 1;
}

// Read config block from flash
uint8_t read_bl_config_block(BL_Config_Block *q) {
	memcpy(q, (void *) BL_Config_Address , sizeof(BL_Config_Block));
	return 1;
}

// Delete config block
uint8_t delete_bl_config_block() {
	Flash_Erase_Page(BL_Config_Address, 2);
    return 1;
}
