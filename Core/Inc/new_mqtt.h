/*
 * MQTT Interface For SIMCom Modules on top of new_gsm.h
 * By: Anistus H
 * 2023/04/28
 */

#ifndef _NEWMQTT_H_
#define _NEWMQTT_H_

#define TIMEOUT_1s 1000
#define TIMEOUT_2s 2000
#define TIMEOUT_3s 3000
#define TIMEOUT_5s 5000
#define TIMEOUT_10s 10000

#include <stdint.h>
#include "mqtt_queue.h"

typedef struct MqttSubStr {
   char topic[5][50];
   int no_of_topics;
   int qos[5];
} Mqtt_sub_str;

extern int MQTT_Ready;
extern mqtt_queue_t mqtt_data_queue;

int PublishConnectPacket(const char* clientId, const char* username, const char* password, int keepAliveTimer, int timeout);
int PublishSubscribePacket(uint16_t packetId, const char* topic, uint8_t qos);
int PublishPing();
int PublishData(const char* topic, const char* message, uint16_t packetId, uint8_t qos, uint8_t retain);
int MQTT_Subscribe(Mqtt_sub_str *data);
int MQTT_Publish(const char* topic, const char* message, uint8_t retain);
void MQTT_Init(const char* apn, const char* mqttserver, uint16_t port, const char* Id, const char* user, const char* pass, int keepAliveTimer, int pingInter);
int MQTT_Connect();
int Reconnect();
void MQTT_Process();
int SendMQTT_Reply1(uint8_t *data, int data_length, char *reply, int reply_length, int timeout);
int Try_Send_MQTT1(uint8_t *data, int data_length, char *reply, int reply_length, int timeout, int max_tries);
void ProcessReceiveData();
int processRetainMessage(int count);
void Connected();
void on_connect();
void Subscribed();
void on_subscribe();
size_t createMQTTConnectPacket(unsigned char* packet, const char* clientId, const char* username, const char* password, int keepAliveTimer);
size_t createMQTTSubscribePacket(unsigned char* packet, uint16_t packetId, const char* topic, uint8_t qos);
size_t createMQTTPublishPacket(unsigned char* packet, const char* topic, const char* message, uint16_t packetId, uint8_t qos, uint8_t retain);

void mqtt_test_function();

#endif /* INC_MQTT_H_ */
