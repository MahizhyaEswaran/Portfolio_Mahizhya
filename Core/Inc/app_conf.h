/*
 * app_conf.h
 *
 *  Created on: Jul 4, 2023
 *      Author: Anistus H
 */

#ifndef INC_APP_CONF_H_
#define INC_APP_CONF_H_

/*
 * Start of Configuration
 */

/*-----------------------MQTT ConFiguration-----------------------*/
#define APN ""
#define MQTT "52.24.122.165"
#define MQTT_ID "dev_anistus_t1"
#define USER "user"
#define PASS "user@pass"
#define KEEP_ALIVE 255
#define PING_TIME 0

#define PublishTopic "D2S/SA/V1/dev_anistus_t1/S"
/*--------------------END of MQTT ConFiguration-------------------*/


/*---------------------Main Flow ConFiguration--------------------*/
#define WAKEUP_TIME 3	//time in minutes

//If flash storage is required, please uncomment the line below.
#define FLASH_BACKUP

//If ACK is required for LoRa communication,
//please select the appropriate mode below.
#define ACK_CALLBACK	//send ACK from LoRa Receive Callback
//#define ACK_QUEUE		//send ACK in main loop using queue

//If CRC is sent from the slave, please enable CRC processing below
#define CRC_EN
/*-----------------END of Main Flow ConFiguration-----------------*/

/*
 * END of Configuration
 * Do not make any alterations or modifications below this line.
 */

#if defined(ACK_CALLBACK) + defined(ACK_QUEUE) > 1
    #error "Cannot select multiple ACK mode"
#endif

#if defined ACK_CALLBACK || defined ACK_QUEUE
#define LORA_ACK
#endif

#endif /* INC_APP_CONF_H_ */
