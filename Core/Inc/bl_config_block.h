/*
 * Configuration block to be used by Bootloader
 * And the user application
 * By: Anistus H
 * 2023/07/13
 */

#ifndef INC_BL_CONFIG_BLOCK_H_
#define INC_BL_CONFIG_BLOCK_H_

#include <string.h>
#include <flash.h>

#define BL_Config_Address 0x08007C00

typedef struct BL_ConfigBlock {
	uint8_t block_start;
    char url[80];
    char apn[10];
    uint8_t bl_flag;
    uint8_t is_fw_available;
    uint8_t is_fw_downloaded;
    uint8_t is_fw_varified;
    uint8_t is_fw_copied;
    uint32_t bl_version;
    uint32_t current_fw_version;
    uint32_t fw_size;
    uint8_t is_error_occurred;
    uint8_t error_state;
    uint8_t remaining_tries;
    uint8_t block_end;
} BL_Config_Block;

int bl_config_block_init(BL_Config_Block *q);
uint8_t save_bl_config_block(BL_Config_Block *q);
uint8_t read_bl_config_block(BL_Config_Block *q);
uint8_t delete_bl_config_block();

#endif /* INC_BL_CONFIG_BLOCK_H_ */
