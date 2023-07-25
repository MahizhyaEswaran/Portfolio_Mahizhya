/*
 * Queue Structure to Store MQTT data in STM32 internal FLASH
 * By: Anistus H
 * 2023/06/19
 */

#ifndef _FLASHQUEUE_H_
#define _FLASHQUEUE_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <flash.h>
#include <flash_queue_conf.h>

// Define the queue structure storing address
#define QUEUE_ADDRESS (FLASH_START-128)
#define PAGE_COUNT (FLASH_END - FLASH_START)/PAGESIZE

// Structure to hold queue data
typedef struct {
	flash_str data;
} flash_queue_data_t;

// Structure to hold queue information
typedef struct {
    uint16_t head;
    uint16_t tail;
    uint16_t count;
} flash_queue_t;

// Creation of an empty flash structure
static const flash_str empty_flash_str = {0};

/*Prototyping for the flash functions*/
void flash_queue_init(flash_queue_t *q);
uint8_t flash_enqueue(flash_queue_t *q, flash_str *data);
uint8_t flash_dequeue(flash_queue_t *q, flash_str *data);
uint8_t flash_peek(flash_queue_t *q, flash_str *data);
uint16_t flash_queue_count(flash_queue_t *q);
uint8_t save_flash_queue(flash_queue_t *q);
uint8_t read_flash_queue(flash_queue_t *q);

#endif /* INC_FLASHQUEUE_H_ */
