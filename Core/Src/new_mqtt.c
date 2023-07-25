/*
 * MQTT Interface For SIMCom Modules on top of new_gsm.h
 * By: Anistus H
 * 2023/04/28
 */

/*Includes*/
#include <new_gsm.h>
#include <new_mqtt.h>
#include "my_string.h"
#include <stdio.h>
#include "gpio.h"
#include "usart.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "iwdg.h"
#include "mqtt_conf.h"
/*End of Includes*/

/*Defines*/
#define MQTT_ReplySize 200
/*End of Defines*/

/*User Variables*/
int MQTT_Ready = 0;
int Reconnect_Tries = 0;
int lastPing = 0;
int pingInterval = 0;
unsigned char mqttreply[MQTT_ReplySize];
unsigned char pingPacket[2] = {0xC0, 0x00};
unsigned char disconnectPacket[2] = { 0xE0, 0x00 };
char connectReply[2] = {32, 2};
char subscribeReply[2] = {144, 3};
char publishReply[2] = {64, 2};
char pingReply[2] = {0xD0, 0x00};
char Client_id[50] = {0};
char Username[50] = {0};
char Password[50] = {0};
int KeepAlive = 0;
int received_len_check = 10;
mqtt_queue_t mqtt_data_queue;

//test variables
int pub_count = 0;
int retain_count = 0;
int ping_count = 0;
int recon_count = 0;
int recon_fail_count = 0;
/*End ofUser Variables*/

/*GSM Functions*/
int PublishConnectPacket(const char* clientId, const char* username, const char* password, int keepAliveTimer, int timeout){
	unsigned char Packet[100] = { 0 };
	int size = createMQTTConnectPacket(Packet,clientId,username,password,keepAliveTimer);

	int err = SendMQTT_Reply1(Packet,size,connectReply,2,timeout);

	if(err){
		MQTT_Ready = 1;
		Connected();
	}

	return err;
}

int PublishSubscribePacket(uint16_t packetId, const char* topic, uint8_t qos){
	unsigned char Packet[100] = { 0 };
	int size = createMQTTSubscribePacket(Packet, packetId, topic, qos);

	int err = Try_Send_MQTT1(Packet,size,subscribeReply,2,TIMEOUT_10s,2);

	if(err){
		err = processRetainMessage(0);
		err+=10;
	}else{
		MQTT_Ready = 0;
	}

	return err;
}

int PublishPing(){
	int err = Try_Send_MQTT1(pingPacket,2,pingReply,2,TIMEOUT_5s,2);
	ping_count++;

	if(!err){
		MQTT_Ready = 0;
	}
	return err;
}

int PublishData(const char* topic, const char* message, uint16_t packetId, uint8_t qos, uint8_t retain){
	unsigned char Packet[150] = { 0 };
	int size = createMQTTPublishPacket(Packet, topic, message, packetId, qos, retain);

	int err = Try_Send_MQTT1(Packet,size,publishReply,2,TIMEOUT_10s,2);

	if(!err){
		MQTT_Ready = 0;
	}
	return err;
}

int MQTT_Subscribe(Mqtt_sub_str *data){
	int err = 100;
	int i = 0;
	while(i < data->no_of_topics){
		err = PublishSubscribePacket(i+1, data->topic[i], data->qos[i]);
		i++;
		if(!err){
			MQTT_Ready = 0;
			break;
		}
	}

	if(err){
		Subscribed();
	}
	return err;
}

int MQTT_Publish(const char* topic, const char* message, uint8_t retain){
	return PublishData(topic, message, 1, 1, retain);
}

void MQTT_Init(const char* apn, const char* mqttserver, const char* Id, const char* user, const char* pass, int keepAliveTimer, int pingInter){
	strcpy(Client_id, Id);
	strcpy(Username, user);
	strcpy(Password, pass);
	KeepAlive = keepAliveTimer;
	pingInterval = pingInter*1000;

	GSM_Init(apn, mqttserver);
	mqtt_queue_init(&mqtt_data_queue);
}

int MQTT_Connect(){
	int tries = 0;
	while((!GSM_Module_Ready) && (tries<3)){
		GSM_ON();
		tries++;
	}
	if(GSM_Module_Ready){
		tries = 0;
		while((!TCP_Ready) && (tries<3)){
			TCP_Connect(0);
			tries++;
		}
		if(TCP_Ready){
			PublishConnectPacket(Client_id, Username, Password, KeepAlive,TIMEOUT_10s);
			if(MQTT_Ready){
				return 1;
			}else{
				return -3;
			}
		}else{
			return -2;
		}
	}else{
		return -1;
	}
}

int Reconnect(){
	int err = 100;

	if(!GSM_Module_Ready){
		GSM_PowerControl(1);
		HAL_Delay(1000);
		err = MQTT_Connect();
	}

	if(!TCP_Ready){
//		ToCMDMode();
//		ModuleReset();
//		int tries = 0;
//		while((!TCP_Ready) && (tries<3)){
//			TCP_Connect(0);
//			tries++;
//		}
//		if(TCP_Ready){
//			GSM_Module_Ready = 1;
//			PublishConnectPacket(Client_id, Username, Password, KeepAlive,TIMEOUT_5s);
//			if(MQTT_Ready){
//				err = 1;
//			}else{
//				err = -3;
//			}
//		}else{
//			err = -2;
//			GSM_Module_Ready = 0;
//		}
		//move to next state
		GSM_Module_Ready = 0;
	}

	if(!MQTT_Ready){
//		PublishConnectPacket(Client_id, Username, Password, KeepAlive,TIMEOUT_3s);
//		if(MQTT_Ready){
//			err = 1;
//		}else{
//			err = -3;
//			TCP_Ready = 0;
//		}
		//move to next state
		TCP_Ready = 0;
	}

	if(err == 1){
		Reconnect_Tries = 0;
		return 1;
	}else{
		Reconnect_Tries++;
		if(Reconnect_Tries > 3){
			Reconnect_Tries = 0;
			return -4;
		}
		return Reconnect();
	}
}

void MQTT_Process(){
	//To Reconnect
	if(!MQTT_Ready){
		recon_count++;
		int err = Reconnect();
		if(err<0){
			recon_fail_count++;
			//reconnection error
			//Error_Handler();
		}
	}

	//To Publish Ping
	if((pingInterval > 0) && (HAL_GetTick() - lastPing > pingInterval)){
		PublishPing();
		lastPing = HAL_GetTick();
	}
}

int SendMQTT_Reply1(uint8_t *data, int data_length, char *reply, int reply_length, int timeout){
	memset(mqttreply, 255, MQTT_ReplySize);

	uint32_t start_tick = HAL_GetTick();
	uint8_t received_char;
	uint8_t remaining_length = 0;
	uint16_t received_len = 0;

	HAL_UART_Transmit_IT(&huart1, data, data_length);
//	HAL_UART_Transmit(&huart1, data, data_length,1000);
	if(WDog){
		HAL_IWDG_Refresh(&hiwdg);
	}

	if(reply_length == 2){
		remaining_length = reply[1];
	}

	while (HAL_GetTick() - start_tick < timeout){
		if (HAL_UART_Receive(&huart1, &received_char, 1, TIMEOUT_1s) == HAL_OK){
			mqttreply[received_len++] = received_char;
			if(received_len >= reply_length ){
				if(my_memmem((char*) mqttreply, received_len, reply, reply_length)){
					HAL_UART_Receive(&huart1, &mqttreply[received_len], remaining_length, TIMEOUT_10s);
					if(received_len > received_len_check){
						ProcessReceiveData();
					}
					memset(mqttreply, 0, MQTT_ReplySize);
					if(WDog){
						HAL_IWDG_Refresh(&hiwdg);
					}
					return 1;
				}
			}
		}

	}
	memset(mqttreply, 0, MQTT_ReplySize);
	return 0;
}

int Try_Send_MQTT1(uint8_t *data, int data_length, char *reply, int reply_length, int timeout, int max_tries){
	int tries = 0;
	int err = 100;
	while(tries < max_tries){
		err = SendMQTT_Reply1(data,data_length,reply,reply_length,timeout);
		if(err == 1){
			return 1;
		}else{
			tries++;
		}
	}
	return err;
}

void ProcessReceiveData(){
	pub_count++;
}

int processRetainMessage(int count){
	memset(mqttreply, 0, MQTT_ReplySize);
	int remaining_length = 0;
	int topic_length = 0;
	HAL_StatusTypeDef ret = HAL_UART_Receive(&huart1, mqttreply, 4, TIMEOUT_1s);
	if(ret == HAL_OK){
		if((mqttreply[0] == 49) && (mqttreply[2] == 0)){
			remaining_length = mqttreply[1] - 2;
			topic_length = mqttreply[3];
			memset(mqttreply, 0, MQTT_ReplySize);
			ret = HAL_UART_Receive(&huart1, mqttreply, remaining_length, TIMEOUT_1s);
			if(ret == HAL_OK){
				retain_count++;
				Mqtt_str new_data = {0};
				memcpy(new_data.topic,mqttreply,topic_length);
				memcpy(new_data.data,&mqttreply[topic_length],remaining_length-topic_length);
				mqtt_enqueue(&mqtt_data_queue, &new_data);
				count = processRetainMessage(++count);
				return count;
			}
		}
	}else{
		return count;
	}
	return count;
}

void Connected(){
	on_connect();
}

__weak void on_connect(){

}

void Subscribed(){
	on_subscribe();
}

__weak void on_subscribe(){

}

size_t createMQTTConnectPacket(unsigned char* packet, const char* clientId, const char* username, const char* password, int keepAliveTimer) {
    size_t clientIdLength = strlen(clientId);
    size_t usernameLength = strlen(username);
    size_t passwordLength = strlen(password);

	// Set fixed packet header
    packet[0] = 0x10;
    // Set variable packet header
    packet[1] = 10+ 6+ clientIdLength+ usernameLength+ passwordLength;  // Remaining length
    packet[2] = 0x00;  // Protocol name length MSB
    packet[3] = 0x04;  // Protocol name length LSB
    packet[4] = 'M';   // Protocol name: MQTT
    packet[5] = 'Q';
    packet[6] = 'T';
    packet[7] = 'T';
    packet[8] = 0x04;  // MQTT protocol version 3.1.1
    packet[9] = 0xC2;  // Connect flags: Clean session,
    packet[10] = keepAliveTimer >> 8;  // Keep-alive MSB
    packet[11] = keepAliveTimer & 0xFF;  // Keep-alive LSB

    // Client ID length
    packet[12] = clientIdLength >> 8;  // Client ID length MSB
    packet[13] = clientIdLength & 0xFF;  // Client ID length LSB

    // Client ID
    memcpy(packet + 14, clientId, clientIdLength);

    // Username length
    packet[14 + clientIdLength] = usernameLength >> 8;  // Username length MSB
    packet[15 + clientIdLength] = usernameLength & 0xFF;  // Username length LSB

    // Username
    memcpy(packet + 16 + clientIdLength, username, usernameLength);

    // Password length
    packet[16 + clientIdLength + usernameLength] = passwordLength >> 8;  // Password length MSB
    packet[17 + clientIdLength + usernameLength] = passwordLength & 0xFF;  // Password length LSB

    // Password
    memcpy(packet + 18 + clientIdLength + usernameLength, password, passwordLength);

    // Calculate and return the total packet length
    return 18 + clientIdLength + usernameLength + passwordLength;
}

size_t createMQTTSubscribePacket(unsigned char* packet, uint16_t packetId, const char* topic, uint8_t qos) {
	size_t topicLength = strlen(topic);
	// Set fixed packet header
    packet[0] = 0x82;  // Subscribe packet type, with reserved bits set to 0
    packet[1] = 5 + topicLength;  // Remaining length

    // Set variable packet header and payload
    // Packet ID
    packet[2] = packetId >> 8;  // Packet ID MSB
    packet[3] = packetId & 0xFF;  // Packet ID LSB

    // Topic length
    packet[4] = topicLength >> 8;  // Topic length MSB
    packet[5] = topicLength & 0xFF;  // Topic length LSB

    // Topic
    memcpy(packet + 6, topic, topicLength);

    // Quality of Service (QoS)
    packet[6 + topicLength] = qos;

    // Calculate and return the total packet length
    return 7 + topicLength;
}

size_t createMQTTPublishPacket(unsigned char* packet, const char* topic, const char* message, uint16_t packetId, uint8_t qos, uint8_t retain) {
    size_t topicLength = strlen(topic);
    size_t messageLength = strlen(message);
	// Set fixed packet header
    packet[0] = 0x30 | (qos << 1) | retain;  // Publish packet type, with QoS and retain flags

    // Set variable packet header and payload
    // Calculate remaining length
    size_t remainingLength = 2 + topicLength + (qos > 0 ? 2 : 0) + messageLength;

    // Set remaining length
	size_t index = 1;
	do {
		uint8_t encodedByte = remainingLength % 128;
		remainingLength /= 128;
		if (remainingLength > 0) {
			encodedByte |= 0x80;
		}
		packet[index++] = encodedByte;
	} while (remainingLength > 0);

	// Topic length
	packet[index++] = topicLength >> 8;  // Topic length MSB
	packet[index++] = topicLength & 0xFF;  // Topic length LSB

	// Topic
	memcpy(packet + index, topic, topicLength);
	index += topicLength;

	// Packet ID for QoS 1 or 2
	if (qos > 0) {
		packet[index++] = packetId >> 8;  // Packet ID MSB
		packet[index++] = packetId & 0xFF;  // Packet ID LSB
	}

	// Message
	memcpy(packet + index, message, messageLength);
	index += messageLength;

	// Calculate and return the total packet length
	return index;
}
/*End Of GSM Functions*/

/*Testing Code*/
//int err2 = 10;
//void mqtt_test_function(){
//}
/*End of Testing Code*/
