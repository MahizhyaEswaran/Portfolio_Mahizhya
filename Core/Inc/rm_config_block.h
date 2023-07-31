/*
 * Configuration block to be used for remote configuration
 * And the user application
 * By: Anistus H
 * 2023/07/31
 */

#ifndef INC_RM_CONFIG_BLOCK_H_
#define INC_RM_CONFIG_BLOCK_H_

#include <string.h>
#include <flash.h>

#define RM_Config_Address 0x08007C00 + 128*2

typedef struct RM_ConfigBlock {
	uint8_t block_start;
    uint8_t block_end;
} RM_ConfigBlock;

int config_rm_block_init(RM_ConfigBlock *q);
uint8_t save_rm_config_block(RM_ConfigBlock *q);
uint8_t read_rm_config_block(RM_ConfigBlock *q);

#endif /* INC_RM_CONFIG_BLOCK_H_ */
