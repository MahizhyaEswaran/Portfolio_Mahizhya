/*
 * Configuration block to be used for remote configuration
 * And the user application
 * By: Anistus H
 * 2023/07/31
 */

#include <rm_config_block.h>

// Initialize the config block
int config_rm_block_init(RM_ConfigBlock *q) {
	read_rm_config_block(q);
	if((q->block_start == 17) && (q->block_end == 8)){
		//Already available
		return 1;
	}else{
		//Not available
		memset(q, 0 , sizeof(RM_ConfigBlock));
		q->block_start = 17;
		q->block_end = 8;
		set_rm_default(q);
		save_rm_config_block(q);
		return 2;
	}
}

//Set the block with default Config
void set_rm_default(RM_ConfigBlock *q){
	sprintf(q->apn, "%s", APN);
	sprintf(q->mqtt_host, "%s", MQTT_HOST);
	sprintf(q->mqtt_username, "%s", MQTT_USER);
	sprintf(q->mqtt_password, "%s", MQTT_PASS);
#ifdef PublishTopic
	sprintf(q->mqtt_pub_topic, "%s", PublishTopic);
#endif
#ifdef SubscribeTopic
	sprintf(q->mqtt_sub_topic, "%s", SubscribeTopic);
#endif
#ifdef RmConfTopic
	sprintf(q->mqtt_rm_conf_topic, "%s", RmConfTopic);
#endif
	q->mqtt_port = MQTT_PORT;
	q->data_publish_period = WAKEUP_TIME;
	q->rm_config_check_period = CONFIG_PERIOD;
}

// Store config block to flash
uint8_t save_rm_config_block(RM_ConfigBlock *q) {
	Flash_Erase_Page(RM_Config_Address, 2);
    Flash_Write_Data(RM_Config_Address , (uint32_t *) q, sizeof(RM_ConfigBlock));
    return 1;
}

// Read config block from flash
uint8_t read_rm_config_block(RM_ConfigBlock *q) {
	memcpy(q, (void *) RM_Config_Address , sizeof(RM_ConfigBlock));
	return 1;
}
