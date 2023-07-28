/*
 * sensor_read.h
 *
 *  Created on: Jul 24, 2023
 *      Author: Anistus H
 */

#ifndef INC_SENSOR_READ_H_
#define INC_SENSOR_READ_H_

#define TEMP130_CAL_ADDR ((uint16_t*) ((uint32_t) 0x1FF8007E))
#define TEMP30_CAL_ADDR ((uint16_t*) ((uint32_t) 0x1FF8007A))
#define VDD_CALIB ((uint16_t) (300))
#define VDD_APPLI ((uint16_t) (330))

#include "main.h"
#include "main.h"
#include "adc.h"
#include "i2c.h"
#include "iwdg.h"
#include "rtc.h"
#include "usart.h"
#include "gpio.h"

#include <app_conf.h>

//Irrometer Output Structure
typedef struct {
    uint16_t A1;
    uint16_t A2;
    uint16_t X;
} irro_reading;

//structure to store time stamp
typedef struct DateAndTime {
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t min;
}date_time;

//total sensor data Structure
typedef struct SensorReading {
#ifdef SHT2x_Temp_EN
	float SHT2x_temp;
#endif
#ifdef SHT2x_RH_EN
	float SHT2x_rh;
#endif
#ifdef Moist_EC_EN
	uint16_t soil_moist;
	uint16_t soil_ec;
#endif
#ifdef Irro_EN
	irro_reading irrometer;
#endif
#ifdef Light_EN
	uint16_t light;
#endif
#ifdef DS18B20_Temp_EN
	float DS18B20_temp;
#endif
	uint8_t internal_temp;
	uint16_t battery;
	uint8_t power_status;
	uint8_t signal_strength;
    date_time timestamp;
#ifdef Moist_EC_ONLY
    uint16_t padding;
#endif
} sensor_reading;

/*Environmental Sensor Setup*/
void PowerUp_Sensors();
void PowerDown_Sensors();
void SHT2x_Sensor_Init();
void SHT2x_Sensor_DeInit();
/*End of Environmental Sensor Setup*/

/*Environmental Sensor Readings*/
float Read_Temp_SHT2x();
float Read_RH_SHT2x();
int Read_Soil_Moisture();
int Read_Soil_EC();
irro_reading Read_Irrometer(int num_of_read);
uint16_t Read_Light_BH1750();
float Read_Temp_DS18B20();
/*End of Environmental Sensor Readings*/

/*Device Status Reading*/
uint8_t Read_Internal_Temp();
int Read_Battery_Leval();
int Read_Power_Status();
/*End of Device Status Readings*/

#endif /* INC_SENSOR_READ_H_ */
