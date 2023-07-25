/*
 * Configuration File for MEW_MQTT library
 * By: Anistus H
 * 2023/05/02
 */

#ifndef INC_MQTT_CONF_H_
#define INC_MQTT_CONF_H_

/*---------------------------------------------Configuration*/
//if Watch dog is enabled, set WDOG to 1.
#define WDog 1

//select the appropriate GSM module
#define SIM808
//#define SIM5320E
//#define SIM5320A
//#define SIM5320J
/*--------------------------------------End of Configuration*/

#if defined(SIM808) + defined(SIM5320E) + defined(SIM5320A) + defined(SIM5320J) > 1
    #error "Cannot select multiple GSM modules simultaneously!"
#endif

#if defined(SIM808) + defined(SIM5320E) + defined(SIM5320A) + defined(SIM5320J) == 0
    #error "Must select one of the GSM module!"
#endif

#ifdef SIM808
#define GSM2G
#endif

#if defined SIM5320E || defined SIM5320A || defined SIM5320J
#define GSM3G

#if defined SIM5320E
#define GSMNEW
#endif

#if defined SIM5320A || defined SIM5320J
#define GSMOLD
#endif

#endif

#endif /* INC_MQTT_CONF_H_ */
