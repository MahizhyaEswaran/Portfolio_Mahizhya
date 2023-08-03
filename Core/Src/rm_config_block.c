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
	sprintf(q->mqtt_id, "%s", MQTT_ID);
	sprintf(q->mqtt_username, "%s", MQTT_USER);
	sprintf(q->mqtt_password, "%s", MQTT_PASS);
#ifdef PublishTopic
	sprintf(q->mqtt_pub_topic, "%s", PublishTopic);
	q->is_topic_imei = 0;
#else
	q->is_topic_imei = 1;
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

void set_rm_config_via_remote(RM_ConfigBlock *rm, mqtt_queue_t *mq){
	while(mqtt_queue_count(mq)){
		Mqtt_str data = {0};
		mqtt_dequeue(mq, &data);
		if(strstr(data.data,"RM") && strstr(data.data,"END")){
			split_rm_config(rm, data.data);
			MQTT_Publish(data.topic, "", 1);
		}
	}
	save_rm_config_block(rm);
}

void split_rm_config(RM_ConfigBlock *rm, char *msg){
	char *token;
	char data[100] = {0};
	char* outer_saveptr = NULL;
	char* inner_saveptr = NULL;
	memcpy(data, &msg[3], strlen(msg) - 7);

	token = strtok_r(data, ";",&outer_saveptr);
	while( token != NULL ) {
		volatile int index = 0;
		char conf_data[50] = {0};
		char index_str[5] = {0};

		sprintf(index_str, strtok_r(token, "-",&inner_saveptr) );
		sprintf(conf_data, strtok_r(NULL, "-",&inner_saveptr) );
		index = (int)strtoul(index_str,0,10);

		if(index > 0){
			set_rm_config(index, conf_data, rm);
		}

		token = strtok_r(NULL, ";", &outer_saveptr);
	}
}

void set_rm_config(int index, char * data, RM_ConfigBlock *rm){
	switch(index){
	case 1:
		memset(rm->mqtt_host,0,sizeof(rm->mqtt_host));
		memcpy(rm->mqtt_host,data,strlen(data));
		break;
	case 2:
		memset(rm->mqtt_id,0,sizeof(rm->mqtt_id));
		memcpy(rm->mqtt_id,data,strlen(data));
		break;
	case 3:
		memset(rm->apn,0,sizeof(rm->apn));
		memcpy(rm->apn,data,strlen(data));
		break;
	case 4:
		rm->mqtt_port = (int)strtoul(data,0,10);
		break;
	case 5:
		memset(rm->mqtt_username,0,sizeof(rm->mqtt_username));
		memcpy(rm->mqtt_username,data,strlen(data));
		break;
	case 6:
		memset(rm->mqtt_password,0,sizeof(rm->mqtt_password));
		memcpy(rm->mqtt_password,data,strlen(data));
		break;
	case 7:
		rm->data_publish_period = (int)strtoul(data,0,10);
		break;
	case 8:
		rm->rm_config_check_period = (int)strtoul(data,0,10);
		break;
	case 9:
		memset(rm->mqtt_pub_topic,0,sizeof(rm->mqtt_pub_topic));
		memcpy(rm->mqtt_pub_topic,data,strlen(data));
		break;
	case 10:
		memset(rm->mqtt_sub_topic,0,sizeof(rm->mqtt_sub_topic));
		memcpy(rm->mqtt_sub_topic,data,strlen(data));
		break;
	case 11:
		memset(rm->mqtt_rm_conf_topic,0,sizeof(rm->mqtt_rm_conf_topic));
		memcpy(rm->mqtt_rm_conf_topic,data,strlen(data));
		break;
	case 12:
		rm->is_topic_imei = (int)strtoul(data,0,10);
		break;
	}
}

// Store config block to flash
uint8_t save_rm_config_block(RM_ConfigBlock *q) {
	Flash_Erase_Page(RM_Config_Address, 4);
    Flash_Write_Data(RM_Config_Address , (uint32_t *) q, sizeof(RM_ConfigBlock));
    return 1;
}

// Read config block from flash
uint8_t read_rm_config_block(RM_ConfigBlock *q) {
	memcpy(q, (void *) RM_Config_Address , sizeof(RM_ConfigBlock));
	return 1;
}
