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
#include <app_conf.h>

#define RM_Config_Address 0x08007d00

typedef struct RM_ConfigBlock {
	uint8_t block_start;
    char mqtt_host[50];
    char mqtt_id[20];
    uint16_t mqtt_port;
    char apn[20];
    char mqtt_username[20];
    char mqtt_password[20];
    uint16_t data_publish_period;
    uint16_t rm_config_check_period;
    char mqtt_pub_topic[50];
    char mqtt_sub_topic[50];
    char mqtt_rm_conf_topic[50];
    uint8_t is_topic_imei;
    uint8_t block_end;
} RM_ConfigBlock;

int config_rm_block_init(RM_ConfigBlock *q);
void set_rm_default(RM_ConfigBlock *q);
uint8_t save_rm_config_block(RM_ConfigBlock *q);
uint8_t read_rm_config_block(RM_ConfigBlock *q);

#endif /* INC_RM_CONFIG_BLOCK_H_ */
