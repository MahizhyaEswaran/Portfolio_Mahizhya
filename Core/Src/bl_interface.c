/*
 * Interfacing function for bootloader
 * By: Anistus H
 * 2023/08/07
 */

#include <bl_interface.h>
#include <rm_config_block.h>

BL_Config_Block bl_config = {0};
extern uint8_t deviceReset;
extern RM_ConfigBlock rm_config;

// to set the url to the bl config block
void set_fota_config(char *url, char *apn){
	if(strstr(url,"senzmate")){
		bl_config_block_init(&bl_config);
		bl_config.is_fw_available = 1;
		sprintf(bl_config.url, "%s", url);
		sprintf(bl_config.apn, "%s", apn);
		bl_config.remaining_tries = 3;

		save_bl_config_block(&bl_config);
		deviceReset = 1;
	}
}

void publish_firmware_status(){
	  bl_config_block_init(&bl_config);
	  char data[100] = {0};
	  sprintf(data, "Start: SW_%d, ErrFlag_%d, ErrState_%d, BlFlag_%d", (int)bl_config.current_fw_version, (int)bl_config.is_error_occurred,
			  (int)bl_config.error_state, (int)bl_config.bl_flag);

	  MQTT_Publish(rm_config.mqtt_pub_topic, data, 0);
}
