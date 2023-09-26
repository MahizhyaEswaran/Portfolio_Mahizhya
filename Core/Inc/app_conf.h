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
#define MQTT_HOST "52.24.122.165"
#define MQTT_ID "dev_anistus_t2"
#define MQTT_PORT 1883
#define MQTT_USER "user"
#define MQTT_PASS "user@pass"
#define KEEP_ALIVE 255
#define PING_TIME 0

//#define IMEI_Topic
#define PublishTopic "D2S/SA/V1/dev_anistus_t2/S"
//#define SubscribeTopic "sample_topic/#"
#define RmConfTopic "S2D/SA/V1/dev_anistus_t2/C/#"
/*--------------------END of MQTT ConFiguration-------------------*/


/*----------------------Sensor ConFiguration----------------------*/
// Disable the sensors by commenting out the
// corresponding sensor definition
#define SHT2x_Temp_EN
#define SHT2x_RH_EN
#define Moist_EN
#define EC_EN
#define Irro_EN
#define Light_EN
#define DS18B20_Temp_EN
#define Power_status_EN
/*-------------------END of Sensor ConFiguration------------------*/


/*-------------------Sensor Label ConFiguration-------------------*/
// Please don't comment any of the definitions. Just edit the values
#define SHT2x_Temp_LB "T"
#define SHT2x_RH_LB "H"
#define Moist_EC_LB "MEA4"
#define Irro_LB "IRO"
#define Light_LB "LIA1"
#define DS18B20_Temp_LB "ST"
#define Device_Battery_LB "B"
#define Internal_temp_LB "IT"
#define Signal_Strength_LB "SS"
#define Power_Status_LB "PS"
/*----------------END of Sensor Label ConFiguration---------------*/


/*---------------------Main Flow ConFiguration--------------------*/
#define WAKEUP_TIME 5	//time in minutes

//Set it if you need an extra connection attempt after the initial
//connection attempt fails.
//#define EXTRA_MQTT_CON_TRY

#define CONFIG_PERIOD 10
/*-----------------END of Main Flow ConFiguration-----------------*/

/*
 * END of Configuration
 * Do not make any alterations or modifications below this line.
 */

#if defined(Moist_EN) + defined(EC_EN)  == 1
    #error "Must select the moisture and EC sensors together."
#endif

#if defined(Moist_EN) && defined(EC_EN)
#define Moist_EC_EN
#endif

#if defined(SHT2x_Temp_EN) || defined(SHT2x_RH_EN)
#define SHT2x_EN
#endif

#if defined(Irro_EN) && !defined(SHT2x_Temp_EN)
    #error "Must select the SHT2x temperature sensors together with Irrometer."
#endif

#if defined(SHT2x_EN) + defined(Moist_EC_EN) + defined(Irro_EN) + defined(Light_EN) + defined(DS18B20_Temp_EN)  == 0
    #error "Choose at least one sensor."
#endif

#if defined(Moist_EC_EN) && !defined(SHT2x_EN) && !defined(Irro_EN) && !defined(Light_EN) && !defined(DS18B20_Temp_EN)
#define Moist_EC_ONLY
#endif

#endif /* INC_APP_CONF_H_ */
