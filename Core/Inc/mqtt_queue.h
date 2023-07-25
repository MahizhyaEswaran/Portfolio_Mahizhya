/*
 * Queue Structure to Store MQTT data
 * By: Anistus H
 * 2023/06/14
 */

#ifndef _NEWMQTTQUEUE_H_
#define _NEWMQTTQUEUE_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

// Define the maximum size of the queue
#define QUEUE_SIZE 30

// Structure to hold MQTT data
typedef struct MqttStr {
    char topic[50];
    char data[100];
    int qos;
    int retain;
} Mqtt_str;

// Structure to hold queue data
typedef struct {
    Mqtt_str data;
} mqtt_queue_data_t;

// Structure to hold queue information
typedef struct {
    mqtt_queue_data_t queue_data[QUEUE_SIZE];
    uint16_t head;
    uint16_t tail;
    uint16_t count;
} mqtt_queue_t;

static const Mqtt_str empty_mqtt_str = {0};

void mqtt_queue_init(mqtt_queue_t *q);
uint8_t mqtt_enqueue(mqtt_queue_t *q, Mqtt_str *data);
uint8_t mqtt_dequeue(mqtt_queue_t *q, Mqtt_str *data);
uint8_t mqtt_peek(mqtt_queue_t *q, Mqtt_str *data);
uint16_t mqtt_queue_count(mqtt_queue_t *q);

#endif /* INC_MQTTQUEUE_H_ */
