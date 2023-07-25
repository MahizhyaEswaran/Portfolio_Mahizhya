/*
 * Queue Structure to Store MQTT data
 * By: Anistus H
 * 2023/06/14
 */

#include "mqtt_queue.h"

// Initialize the queue
void mqtt_queue_init(mqtt_queue_t *q) {
    q->head = 0;
    q->tail = 0;
    q->count = 0;
}

// Add an item to the queue
uint8_t mqtt_enqueue(mqtt_queue_t *q, Mqtt_str *data) {
    if (q->count >= QUEUE_SIZE) {
        return 0;
    }
    memcpy(&q->queue_data[q->tail].data, data, sizeof(Mqtt_str));

    q->tail = (q->tail + 1) % QUEUE_SIZE;
    q->count++;
    return 1;
}

// Remove an item from the queue
uint8_t mqtt_dequeue(mqtt_queue_t *q, Mqtt_str *data) {
    if (q->count == 0) {
        return 0;
    }
    memcpy(data, &empty_mqtt_str, sizeof(Mqtt_str));
    memcpy(data, &q->queue_data[q->head].data, sizeof(Mqtt_str));
    memcpy(&q->queue_data[q->head].data, &empty_mqtt_str, sizeof(Mqtt_str));

    q->head = (q->head + 1) % QUEUE_SIZE;
    q->count--;
    if (q->count == 0) {
        q->head = 0;
        q->tail = 0;
    }
    return 1;
}

// Get the head of the queue without removing it
uint8_t mqtt_peek(mqtt_queue_t *q, Mqtt_str *data) {
    if (q->count == 0) {
        return 0;
    }
    memcpy(data, &empty_mqtt_str, sizeof(Mqtt_str));
    memcpy(data, &q->queue_data[q->head].data, sizeof(Mqtt_str));

    return 1;
}

// Get the number of items in the queue
uint16_t mqtt_queue_count(mqtt_queue_t *q) {
    return q->count;
}
