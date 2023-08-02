/*
 * GSM Interface For SIMCom Modules
 * By: Anistus H
 * 2023/04/28
 */

#ifndef _NEWGSM_H_
#define _NEWGSM_H_

#define TIMEOUT_1s 1000
#define TIMEOUT_2s 2000
#define TIMEOUT_5s 5000
#define TIMEOUT_8s 8000
#define TIMEOUT_10s 10000

#include <stdint.h>

extern int GSM_Module_Ready;
extern int TCP_state;
extern int TCP_Ready;
extern char timeZone[5];
extern int signalStrength;

void PowerToggle(void);
void GSM_PowerControl(int input);
void GsmModuleReset(void);
void GSM_Check();
int GSM_ON();
void GSM_OFF();
void GSM_Init(const char* apn, const char* mqttserver, uint16_t port);
void ToCMDMode();
void ToDataMode();
#ifdef GSM3G
void ModuleReset();
int NetClose();
#endif
void GetCoverage();
void GetRTC();
void SendAT(uint8_t *p_string);
int SendAT_Reply1(uint8_t *data, char *reply, int timeout);
int SendAT_Reply2(uint8_t *data, char *reply1, char *reply2, int timeout);
int SendAT_Reply3(uint8_t *data, char *reply1, char *reply2, char *reply3, int timeout);
int SendAT_Reply_error(uint8_t *data, char *reply, char *error, int timeout);
int SendAT_Reply1_return(uint8_t *data, char *reply, int timeout, char *ret);
int Try_Send_AT1(uint8_t *data, char *reply, int timeout, int max_tries);
int Try_Send_AT2(uint8_t *data, char *reply1, char *reply2, int timeout, int max_tries);
int Try_Send_AT3(uint8_t *data, char *reply1, char *reply2, char *reply3, int timeout, int max_tries);
int Try_Send_AT_Err(uint8_t *data, char *reply1, char *error, int timeout, int max_tries);
int Try_Send_AT1_return(uint8_t *data, char *reply, int timeout, char *ret, int max_tries);
int TCP_Connect();

void gsm_test_function();

#endif /* INC_MQTT_H_ */
