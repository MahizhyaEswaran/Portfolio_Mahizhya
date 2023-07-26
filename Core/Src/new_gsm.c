/*
 * GSM Interface For SIMCom Modules
 * By: Anistus H
 * 2023/04/28
 */

/*Includes*/
#include <new_gsm.h>
#include <stdio.h>
#include "gpio.h"
#include "usart.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "iwdg.h"
#include <rtc.h>
#include "mqtt_conf.h"
/*End of Includes*/

/*Defines*/
#define GSM_ReplySize 200
#define rxBuffSize 46
/*End of Defines*/

/*User Variables*/
char gsmreply[GSM_ReplySize];

int GSM_Module_Ready = 0;
int GSM_Module_Mode = 0;
int TCP_state = 0;
int TCP_Ready = 0;
int signalStrength = 0;
char timeZone[5] = {0};

char APN[20] = {0};
char MQTTServer[50] = {0};
char AtCom[100] = {0};
/*End ofUser Variables*/

/*GSM Functions*/
void PowerToggle(void)
{
	HAL_GPIO_WritePin(MODEM_PWR_GPIO_Port, MODEM_PWR_Pin, GPIO_PIN_RESET);
	HAL_Delay(1000);
	HAL_GPIO_WritePin(MODEM_PWR_GPIO_Port, MODEM_PWR_Pin, GPIO_PIN_SET);
	HAL_Delay(2000);
	HAL_GPIO_WritePin(MODEM_PWR_GPIO_Port, MODEM_PWR_Pin, GPIO_PIN_RESET);
	if(WDog){
		HAL_IWDG_Refresh(&hiwdg);
	}
}

void GSM_PowerControl(int input)
{
	if (input == 1)
	{
		HAL_GPIO_WritePin(GSM_POWER_CTRL_GPIO_Port, GSM_POWER_CTRL_Pin, GPIO_PIN_SET);
	}
	else if (input == 2)
	{
		HAL_GPIO_WritePin(GSM_POWER_CTRL_GPIO_Port, GSM_POWER_CTRL_Pin, GPIO_PIN_RESET);
	}
}

void GsmModuleReset(void)
{
	HAL_UART_DeInit(&huart1);
	PowerToggle();
	HAL_Delay(1000);
	PowerToggle();
	HAL_UART_Init(&huart1);
	if(WDog){
		HAL_IWDG_Refresh(&hiwdg);
	}
	GSM_Module_Ready = 0;
	GSM_Check();
}

void GSM_Check()
{
	int tries_check = 0;
	HAL_UART_AbortReceive(&huart1);
	while ((GSM_Module_Ready == 0) && (tries_check < 2) )
	{
		uint32_t start_tick = HAL_GetTick();
		while (HAL_GetTick() - start_tick < TIMEOUT_10s){
			int reply = SendAT_Reply1((uint8_t*) "AT\r\n","OK\r\n",TIMEOUT_1s);
			if(WDog){
				HAL_IWDG_Refresh(&hiwdg);
			}
			if (reply)
			{
				memset(gsmreply, 0, GSM_ReplySize);
				GSM_Module_Ready = 1;
				break;
			}
		}

		if(!GSM_Module_Ready){
			memset(gsmreply, 0, GSM_ReplySize);
			HAL_UART_DeInit(&huart1);
			PowerToggle();
			HAL_UART_Init(&huart1);
			HAL_Delay(100);
			if(WDog){
				HAL_IWDG_Refresh(&hiwdg);
			}
			tries_check++;
		}
	}
}

int GSM_ON(){
	GSM_PowerControl(2);
	PowerToggle();
	GSM_Check();
	return GSM_Module_Ready;
}

void GSM_OFF(){
	PowerToggle();
	GSM_PowerControl(1);
}

void GSM_Init(const char* apn, const char* mqttserver){
	strcpy(APN, apn);
	strcpy(MQTTServer, mqttserver);
}

void ToCMDMode(){
	HAL_Delay(1000);
	int reply = SendAT_Reply1((uint8_t*) "+++","OK\r\n",TIMEOUT_1s);
	if (reply){
		memset(gsmreply, 0, GSM_ReplySize);
		GSM_Module_Mode = 0;
	}
	HAL_Delay(1000);
}

void ToDataMode(){
	HAL_Delay(1000);
#ifdef GSM3G
	int reply = SendAT_Reply1((uint8_t*) "ATO\r\n","CONNECT 9600\r\n",TIMEOUT_1s);
#endif
#ifdef GSM2G
	int reply = SendAT_Reply1((uint8_t*) "ATO\r\n","CONNECT\r\n",TIMEOUT_1s);
#endif
	if (reply){
		memset(gsmreply, 0, GSM_ReplySize);
		GSM_Module_Mode = 1;
	}
	HAL_Delay(1000);
}

#ifdef GSM3G
void ModuleReset(){
	int reply = Try_Send_AT1((uint8_t*) "AT+CRESET\r\n","OK\r\n",TIMEOUT_1s,3);
	if (reply){
		memset(gsmreply, 0, GSM_ReplySize);
		TCP_Ready = 0;
	}
}

int NetClose(){
	int reply = Try_Send_AT_Err((uint8_t*) "AT+NETCLOSE\r\n","+NETCLOSE: 0\r\n", "+NETCLOSE: 1\r\n",TIMEOUT_1s,3);
	return reply;
}
#endif

void GetCoverage(){
	char coverage[200] = {0};
	int reply = Try_Send_AT1_return((uint8_t*) "AT+CSQ\r\n","OK\r\n",TIMEOUT_2s,coverage,3);

	if(reply){
		char ss[2] = {0};
		ss[0] = coverage[15];
		ss[1] = coverage[16];
		signalStrength = atoi(ss);
	}
}

void GetRTC(){
	char rtc[200] = {0};
#ifdef GSM3G
	int reply = Try_Send_AT1((uint8_t*) "AT+CTZU=1\r\n","OK\r\n",TIMEOUT_2s,3);
#endif
#ifdef GSM2G
	int reply = Try_Send_AT1((uint8_t*) "AT+CLTS=1\r\n","OK\r\n",TIMEOUT_2s,3);
#endif
	if(reply){
		reply = Try_Send_AT1((uint8_t*) "AT&W\r\n","OK\r\n",TIMEOUT_2s,3);
	}
	if(reply){
		reply = Try_Send_AT1_return((uint8_t*) "AT+CCLK?\r\n","OK\r\n",TIMEOUT_2s,rtc,3);
		if(reply){
			char HoursAT[2]="00";
			char MinutesAT[2]="00";
			char SecondsAT[2]="00";
			char DateAT[2]="00";
			char MonthAT[2]="00";
			char YearAT[2]="00";

			RTC_TimeTypeDef sTime = {0};
			RTC_DateTypeDef sDate = {0};

			YearAT[0]=rtc[19];
			YearAT[1]=rtc[20];
			MonthAT[0] =rtc[22];
			MonthAT[1] =rtc[23];
			DateAT[0] =rtc[25];
			DateAT[1] =rtc[26];

			HoursAT[0]=rtc[28];
			HoursAT[1]=rtc[29];
			MinutesAT[0]=rtc[31];
			MinutesAT[1]=rtc[32];
			SecondsAT[0]=rtc[34];
			SecondsAT[1]=rtc[35];

			sprintf(timeZone, strtok(&rtc[36], "\""));

		  	sTime.Hours = atoi(HoursAT);
		    sTime.Minutes = atoi(MinutesAT);
		    sTime.Seconds = atoi(SecondsAT);
		    sDate.Month = atoi(MonthAT);
		    sDate.Date = atoi(DateAT);
		    sDate.Year = atoi(YearAT);

		    if(sDate.Year > 22){
		    	My_RTC_Init(sTime, sDate);
		    }
		    HAL_Delay(1);
		}
	}
}

void SendAT(uint8_t *p_string){
	memset(gsmreply, 0, GSM_ReplySize);

	uint16_t length = 0;
	while (p_string[length] != '\0')
	{
		length++;
	}
	HAL_UART_Transmit_IT(&huart1, p_string, length);
//	HAL_UART_Transmit(&huart1, p_string, length,1000);
	if(WDog){
		HAL_IWDG_Refresh(&hiwdg);
	}
}

int SendAT_Reply1(uint8_t *data, char *reply, int timeout){
	memset(gsmreply, 0, GSM_ReplySize);

	uint32_t start_tick = HAL_GetTick();
	uint16_t data_length = 0;
	uint16_t reply_length = 0;
	uint8_t received_char;
	uint16_t received_len = 0;
	while (data[data_length] != '\0')
	{
		data_length++;
	}
	while (reply[reply_length] != '\0')
	{
		reply_length++;
	}
	HAL_UART_Transmit_IT(&huart1, data, data_length);
//	HAL_UART_Transmit(&huart1, data, data_length,1000);
	if(WDog){
		HAL_IWDG_Refresh(&hiwdg);
	}

	while (HAL_GetTick() - start_tick < timeout){
		if (HAL_UART_Receive(&huart1, &received_char, 1, TIMEOUT_1s) == HAL_OK){
			gsmreply[received_len++] = received_char;
			if(received_len >= reply_length ){
				if(strstr((char*) gsmreply, reply)){
					memset(gsmreply, 0, GSM_ReplySize);
					if(WDog){
						HAL_IWDG_Refresh(&hiwdg);
					}
					return 1;
				}
			}
		}

	}
	memset(gsmreply, 0, GSM_ReplySize);
	return 0;
}

int SendAT_Reply2(uint8_t *data, char *reply1, char *reply2, int timeout){
	memset(gsmreply, 0, GSM_ReplySize);

	uint32_t start_tick = HAL_GetTick();
	uint16_t data_length = 0;
	uint16_t reply1_length = 0;
	uint16_t reply2_length = 0;
	uint16_t check_length = 0;
	uint8_t received_char;
	uint16_t received_len = 0;
	while (data[data_length] != '\0')
	{
		data_length++;
	}
	while (reply1[reply1_length] != '\0')
	{
		reply1_length++;
	}
	while (reply2[reply2_length] != '\0')
	{
		reply2_length++;
	}
	if(reply1_length <= reply2_length){
		check_length = reply1_length;
	}else{
		check_length = reply2_length;
	}
	HAL_UART_Transmit_IT(&huart1, data, data_length);
//	HAL_UART_Transmit(&huart1, data, data_length,1000);
	if(WDog){
		HAL_IWDG_Refresh(&hiwdg);
	}

	while (HAL_GetTick() - start_tick < timeout){
		if (HAL_UART_Receive(&huart1, &received_char, 1, TIMEOUT_1s) == HAL_OK){
			gsmreply[received_len++] = received_char;
			if(received_len >= check_length ){
				if(strstr((char*) gsmreply, reply1)){
					memset(gsmreply, 0, GSM_ReplySize);
					if(WDog){
						HAL_IWDG_Refresh(&hiwdg);
					}
					return 1;
				}else if(strstr((char*) gsmreply, reply2)){
					memset(gsmreply, 0, GSM_ReplySize);
					if(WDog){
						HAL_IWDG_Refresh(&hiwdg);
					}
					return 1;
				}
			}
		}

	}
	memset(gsmreply, 0, GSM_ReplySize);
	return 0;
}

int SendAT_Reply3(uint8_t *data, char *reply1, char *reply2, char *reply3, int timeout){
	memset(gsmreply, 0, GSM_ReplySize);

	uint32_t start_tick = HAL_GetTick();
	uint16_t data_length = 0;
	uint16_t reply1_length = 0;
	uint16_t reply2_length = 0;
	uint16_t reply3_length = 0;
	uint16_t check_length = 0;
	uint8_t received_char;
	uint16_t received_len = 0;
	while (data[data_length] != '\0')
	{
		data_length++;
	}
	while (reply1[reply1_length] != '\0')
	{
		reply1_length++;
	}
	while (reply2[reply2_length] != '\0')
	{
		reply2_length++;
	}
	while (reply3[reply3_length] != '\0')
	{
		reply3_length++;
	}
	if((reply1_length <= reply2_length) && (reply1_length <= reply3_length)){
		check_length = reply1_length;
	}else if((reply2_length <= reply1_length) && (reply2_length <= reply3_length)){
		check_length = reply2_length;
	}else{
		check_length = reply3_length;
	}
	HAL_UART_Transmit_IT(&huart1, data, data_length);
//	HAL_UART_Transmit(&huart1, data, data_length,1000);
	if(WDog){
		HAL_IWDG_Refresh(&hiwdg);
	}

	while (HAL_GetTick() - start_tick < timeout){
		if (HAL_UART_Receive(&huart1, &received_char, 1, TIMEOUT_1s) == HAL_OK){
			gsmreply[received_len++] = received_char;
			if(received_len >= check_length ){
				if(strstr((char*) gsmreply, reply1)){
					memset(gsmreply, 0, GSM_ReplySize);
					if(WDog){
						HAL_IWDG_Refresh(&hiwdg);
					}
					return 1;
				}else if(strstr((char*) gsmreply, reply2)){
					memset(gsmreply, 0, GSM_ReplySize);
					if(WDog){
						HAL_IWDG_Refresh(&hiwdg);
					}
					return 1;
				}else if(strstr((char*) gsmreply, reply3)){
					memset(gsmreply, 0, GSM_ReplySize);
					if(WDog){
						HAL_IWDG_Refresh(&hiwdg);
					}
					return 1;
				}
			}
		}

	}
	memset(gsmreply, 0, GSM_ReplySize);
	return 0;
}

int SendAT_Reply_error(uint8_t *data, char *reply, char *error, int timeout){
	memset(gsmreply, 0, GSM_ReplySize);

	uint32_t start_tick = HAL_GetTick();
	uint16_t data_length = 0;
	uint16_t reply_length = 0;
	uint16_t error_length = 0;
	uint16_t check_length = 0;
	uint8_t received_char;
	uint16_t received_len = 0;
	while (data[data_length] != '\0')
	{
		data_length++;
	}
	while (reply[reply_length] != '\0')
	{
		reply_length++;
	}
	while (error[error_length] != '\0')
	{
		error_length++;
	}
	if(reply_length <= error_length){
		check_length = reply_length;
	}else{
		check_length = error_length;
	}
	HAL_UART_Transmit_IT(&huart1, data, data_length);
//	HAL_UART_Transmit(&huart1, data, data_length,1000);
	if(WDog){
		HAL_IWDG_Refresh(&hiwdg);
	}

	while (HAL_GetTick() - start_tick < timeout){
		if (HAL_UART_Receive(&huart1, &received_char, 1, TIMEOUT_1s) == HAL_OK){
			gsmreply[received_len++] = received_char;
			if(received_len >= check_length ){
				if(strstr((char*) gsmreply, reply)){
					memset(gsmreply, 0, GSM_ReplySize);
					if(WDog){
						HAL_IWDG_Refresh(&hiwdg);
					}
					return 1;
				}else if(strstr((char*) gsmreply, error)){
					memset(gsmreply, 0, GSM_ReplySize);
					if(WDog){
						HAL_IWDG_Refresh(&hiwdg);
					}
					return -1;
				}
			}
		}

	}
	memset(gsmreply, 0, GSM_ReplySize);
	return 0;
}

int SendAT_Reply1_return(uint8_t *data, char *reply, int timeout, char *ret){
	memset(gsmreply, 0, GSM_ReplySize);

	uint32_t start_tick = HAL_GetTick();
	uint16_t data_length = 0;
	uint16_t reply_length = 0;
	uint8_t received_char;
	uint16_t received_len = 0;
	while (data[data_length] != '\0')
	{
		data_length++;
	}
	while (reply[reply_length] != '\0')
	{
		reply_length++;
	}
	HAL_UART_Transmit_IT(&huart1, data, data_length);
//	HAL_UART_Transmit(&huart1, data, data_length,1000);
	if(WDog){
		HAL_IWDG_Refresh(&hiwdg);
	}

	while (HAL_GetTick() - start_tick < timeout){
		if (HAL_UART_Receive(&huart1, &received_char, 1, TIMEOUT_1s) == HAL_OK){
			gsmreply[received_len++] = received_char;
			if(received_len >= reply_length ){
				if(strstr((char*) gsmreply, reply)){
					memcpy(ret,gsmreply,200);
					memset(gsmreply, 0, GSM_ReplySize);
					if(WDog){
						HAL_IWDG_Refresh(&hiwdg);
					}
					return 1;
				}
			}
		}

	}
	memset(gsmreply, 0, GSM_ReplySize);
	return 0;
}

int Try_Send_AT1(uint8_t *data, char *reply, int timeout, int max_tries){
	int tries = 0;
	int err = 100;
	while(tries < max_tries){
		err = SendAT_Reply1(data,reply,timeout);
		if(err == 1){
			return 1;
		}else{
			tries++;
		}
	}
	return err;
}

int Try_Send_AT2(uint8_t *data, char *reply1, char *reply2, int timeout, int max_tries){
	int tries = 0;
	int err = 100;
	while(tries < max_tries){
		err = SendAT_Reply2(data,reply1,reply2,timeout);
		if(err == 1){
			return 1;
		}else{
			tries++;
		}
	}
	return err;
}

int Try_Send_AT3(uint8_t *data, char *reply1, char *reply2, char *reply3, int timeout, int max_tries){
	int tries = 0;
	int err = 100;
	while(tries < max_tries){
		err = SendAT_Reply3(data,reply1,reply2,reply3,timeout);
		if(err == 1){
			return 1;
		}else{
			tries++;
		}
	}
	return err;
}

int Try_Send_AT_Err(uint8_t *data, char *reply1, char *error, int timeout, int max_tries){
	int tries = 0;
	int err = 100;
	while(tries < max_tries){
		err = SendAT_Reply_error(data,reply1,error,timeout);
		if(err == 1){
			return 1;
		}else if(err == -1){
			return -1;
		}else{
			tries++;
		}
	}
	return err;
}

int Try_Send_AT1_return(uint8_t *data, char *reply, int timeout, char *ret, int max_tries){
	int tries = 0;
	int err = 100;
	while(tries < max_tries){
		err = SendAT_Reply1_return(data, reply, timeout, ret);
		if(err == 1){
			return 1;
		}else{
			tries++;
		}
	}
	return err;
}

#ifdef GSM3G
int TCP_Connect(int state){
	TCP_state = state;
	int err;
	TCP_Ready = 0;
	switch(TCP_state){
	case 0:
		err = 100;
		err = Try_Send_AT1((uint8_t*) "AT\r\n", "OK\r\n", TIMEOUT_2s, 3);
		if(err != 1){return -1;}
		TCP_state = 1;
	case 1:
		err = 100;
		err = Try_Send_AT1((uint8_t*) "AT+CICCID\r\n", "OK\r\n", TIMEOUT_2s, 3);
		if(err != 1){return -1;}
		TCP_state = 2;
	case 2:
		err = 100;  //need to add more checking conditions
		err = Try_Send_AT3((uint8_t*) "AT+CGREG?\r\n", "CGREG: 0,1\r\n", "CGREG: 1,1\r\n", "CGREG: 1,5\r\n", TIMEOUT_1s, 10);
		if(err != 1){return -1;}
		TCP_state = 3;
	case 3:
		err = 100;
		memset(AtCom,0,sizeof(AtCom));
		sprintf(AtCom, "AT+CGSOCKCONT=1,\"IP\",\"%s\"\r\n", APN);
		err = Try_Send_AT1((uint8_t*) AtCom, "OK\r\n", TIMEOUT_2s, 3);
		if(err != 1){return -1;}
		TCP_state = 4;
	case 4:
		err = 100;
		err = Try_Send_AT1((uint8_t*) "AT+CSOCKSETPN=1\r\n", "OK\r\n", TIMEOUT_2s, 3);
		if(err != 1){return -1;}
		TCP_state = 5;
	case 5:
		err = 100;
		err = Try_Send_AT1((uint8_t*) "AT+CIPMODE=1\r\n", "OK\r\n", TIMEOUT_2s, 3);
		if(err != 1){
			err = 100;
			err = Try_Send_AT1((uint8_t*) "AT+CIPMODE?\r\n", "+CIPMODE: 1\r\n", TIMEOUT_2s, 3);
			if(err != 1){return -1;}
		}
		GetCoverage();
		TCP_state = 6;
	case 6:
		err = 100;
#ifdef GSMNEW
		err = Try_Send_AT2((uint8_t*) "AT+NETOPEN\r\n", "+NETOPEN: 0\r\n", "opened", TIMEOUT_5s, 3);
#endif
#ifdef GSMOLD
		err = Try_Send_AT1((uint8_t*) "AT+NETOPEN=\"TCP\"\r\n", "Network opened\r\n", TIMEOUT_5s, 3);
#endif
		if(err != 1){return -1;}
		GetRTC();
		TCP_state = 7;
	case 7:
		err = 100;
		memset(AtCom,0,sizeof(AtCom));
#ifdef GSMNEW
		sprintf(AtCom, "AT+CIPOPEN=0,\"TCP\",\"%s\",1883\r\n", MQTTServer);
#endif
#ifdef GSMOLD
		sprintf(AtCom, "AT+TCPCONNECT=\"%s\",1883\r\n", MQTTServer);
#endif
		err = Try_Send_AT1((uint8_t*) AtCom, "CONNECT 9600\r\n", TIMEOUT_2s, 3);
		if(err != 1){return -1;}
		GSM_Module_Mode = 1;
		TCP_state = 9;
	}

	if(TCP_state == 9){
		TCP_Ready = 1;
		if(WDog){
			HAL_IWDG_Refresh(&hiwdg);
		}
		return 1;
	}
	if(WDog){
		HAL_IWDG_Refresh(&hiwdg);
	}
	return 0;
}
#endif

#ifdef GSM2G
int TCP_Connect(int state){
	TCP_state = state;
	int err;
	TCP_Ready = 0;
	switch(TCP_state){
	case 0:
		err = 100;
		err = Try_Send_AT1((uint8_t*) "AT\r\n", "OK\r\n", TIMEOUT_2s, 3);
		if(err != 1){return -1;}
		TCP_state = 1;
	case 1:
		err = 100;
		err = Try_Send_AT1((uint8_t*) "AT+CSCLK=0\r\n", "OK\r\n", TIMEOUT_2s, 3);
		if(err != 1){return -1;}
		TCP_state = 2;
	case 2:
		err = 100;
		err = Try_Send_AT1((uint8_t*) "AT+CIPSHUT\r\n", "SHUT OK\r\n", TIMEOUT_2s, 3);
		if(err != 1){return -1;}
		TCP_state = 3;
	case 3:
		err = 100;  //need to add more checking conditions
		err = Try_Send_AT3((uint8_t*) "AT+CGREG?\r\n", "CGREG: 0,1\r\n\r\nOK\r\n", "CGREG: 1,1\r\n\r\nOK\r\n", "CGREG: 1,5\r\n\r\nOK\r\n", TIMEOUT_1s, 10);
		if(err != 1){return -1;}
		TCP_state = 4;
	case 4:
		err = 100;
		err = Try_Send_AT1((uint8_t*) "AT+CGATT=1\r\n", "OK\r\n", TIMEOUT_2s, 3);
		if(err != 1){return -1;}
		TCP_state = 5;
	case 5:
		err = 100;
		err = Try_Send_AT1((uint8_t*) "AT+CIPMODE=1\r\n", "OK\r\n", TIMEOUT_2s, 3);
		if(err != 1){
			err = 100;
			err = Try_Send_AT1((uint8_t*) "AT+CIPMODE?\r\n", "+CIPMODE: 1\r\n", TIMEOUT_2s, 3);
			if(err != 1){return -1;}
		}
		TCP_state = 6;
	case 6:
		err = 100;
		memset(AtCom,0,sizeof(AtCom));
		sprintf(AtCom, "AT+CSTT=\"%s\",\"\",\"\"\r\n", APN);
		err = Try_Send_AT1((uint8_t*) AtCom, "OK\r\n", TIMEOUT_2s, 3);
		if(err != 1){return -1;}
		GetCoverage();
		TCP_state = 7;
	case 7:
		err = 100;
		err = Try_Send_AT1((uint8_t*) "AT+CIICR\r\n", "OK\r\n", TIMEOUT_2s, 3);
		if(err != 1){return -1;}
		TCP_state = 8;
	case 8:
		err = 100;
		err = Try_Send_AT1((uint8_t*) "AT+CIFSR\r\n", "\r\n", TIMEOUT_2s, 3);
		if(err != 1){return -1;}
		GetRTC();
		TCP_state = 9;
	case 9:
		err = 100;
		memset(AtCom,0,sizeof(AtCom));
		sprintf(AtCom, "AT+CIPSTART=\"TCP\",\"%s\",1883\r\n", MQTTServer);
		err = Try_Send_AT1((uint8_t*) AtCom, "CONNECT\r\n", TIMEOUT_2s, 3);
		if(err != 1){return -1;}
		GSM_Module_Mode = 1;
		TCP_state = 10;
	}

	if(TCP_state == 10){
		TCP_Ready = 1;
		if(WDog){
			HAL_IWDG_Refresh(&hiwdg);
		}
		return 1;
	}
	if(WDog){
		HAL_IWDG_Refresh(&hiwdg);
	}
	return 0;
}
#endif
/*End Of GSM Functions*/

/*Testing Code*/
//int err = 10;
//void gsm_test_function(){
//}
/*End of Testing Code*/
