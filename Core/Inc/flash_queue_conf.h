/*
 * Configuration File for flash_queue library
 * By: Anistus H
 * 2023/06/20
 */

#ifndef INC_FLASH_QUEUE_CONF_H_
#define INC_FLASH_QUEUE_CONF_H_

#include <sensor_read.h>

/*
 * Set the appropriate starting address of the flash for the application.
 * Note: This address does not correspond to the actual starting point of
 * 		 the STM32 Flash memory; rather, it indicates the desired location
 * 		 for storing the data.
 */
#define FLASH_START 0x0801C000

/*
 * Structure to hold the flash data, Modify the 'flash_str' structure
 * according to your data storage requirements.
 * IMPORTANT: Ensure that the total size of the "flash_str" structure
 * 			  is a multiple of 4. Otherwise, it will trigger a hard
 * 			  fault breakpoint in the application.
 */
typedef struct FlashStr {
    sensor_reading sensor;
} flash_str;

#endif /* INC_FLASH_QUEUE_CONF_H_ */
