/*
 * Interfacing function for bootloader
 * By: Anistus H
 * 2023/08/07
 */

#ifndef INC_BL_INTERFACE_H_
#define INC_BL_INTERFACE_H_

#include <bl_config_block.h>

extern BL_Config_Block bl_config;

void set_fota_config(char *url, char *apn);
void publish_firmware_status();

#endif /* INC_BL_INTERFACE_H_ */
