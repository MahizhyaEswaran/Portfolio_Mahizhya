/*
 * Queue Structure to Store MQTT data in STM32 internal FLASH
 * By: Anistus H
 * 2023/06/19
 */

#include "flash_queue.h"

int queue_size  = 0;

// Initialize the queue
void flash_queue_init(flash_queue_t *q) {
	int element_size = sizeof(flash_str);
	if(element_size%4 >0){
		//throw error
		//The size of 'flash_str' should be multiple of 4
		__BKPT(0);
	}
	queue_size = (FLASH_END - FLASH_START)/element_size;

	read_flash_queue(q);
	if((q->head >= q->tail) || (q->count>queue_size)){
	    q->head = 0;
	    q->tail = 0;
	    q->count = 0;
	    save_flash_queue(q);
	    Flash_Erase_Page(FLASH_START,PAGE_COUNT);
	}
}

// Add an item to the queue in flash
uint8_t flash_enqueue(flash_queue_t *q, flash_str *data) {
    if (q->count >= queue_size) {
        return 0;
    }
    Flash_Write_Data(FLASH_START + q->tail*sizeof(flash_str), (uint32_t *) data, sizeof(flash_str));

    q->tail = (q->tail + 1) % queue_size;
    q->count++;
    save_flash_queue(q);
    return 1;
}

// Remove an item from the queue in flash
uint8_t flash_dequeue(flash_queue_t *q, flash_str *data) {
    if (q->count == 0) {
    	memcpy(data, &empty_flash_str, sizeof(flash_str));
        return 0;
    }
    memcpy(data, &empty_flash_str, sizeof(flash_str));
    memcpy(data, (void *) FLASH_START + q->head*sizeof(flash_str), sizeof(flash_str));

    q->head = (q->head + 1) % queue_size;
    q->count--;
    if (q->count == 0) {
        q->head = 0;
        q->tail = 0;
        Flash_Erase_Page(FLASH_START,PAGE_COUNT);
    }
    save_flash_queue(q);
    return 1;
}

// Get the head of the queue without removing it
uint8_t flash_peek(flash_queue_t *q, flash_str *data) {
    if (q->count == 0) {
    	memcpy(data, &empty_flash_str, sizeof(flash_str));
        return 0;
    }
    memcpy(data, &empty_flash_str, sizeof(flash_str));
	memcpy(data, (void *) FLASH_START + q->head*sizeof(flash_str), sizeof(flash_str));

    return 1;
}

// Get the number of items in the queue
uint16_t flash_queue_count(flash_queue_t *q) {
    return q->count;
}

// Store flash structure to flash
uint8_t save_flash_queue(flash_queue_t *q) {
	Flash_Erase_Page(QUEUE_ADDRESS, 1);
    Flash_Write_Data(QUEUE_ADDRESS , (uint32_t *) q, sizeof(flash_queue_t));
    return 1;
}

// Read flash structure from flash
uint8_t read_flash_queue(flash_queue_t *q) {
	memcpy(q, (void *) QUEUE_ADDRESS , sizeof(flash_queue_t));
	return 1;
}
