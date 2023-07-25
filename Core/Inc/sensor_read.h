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

//Irrometer Output Structure
typedef struct {
    double A1;
    double A2;
    int X;
} irro_reading;

//total sensor data Structure
typedef struct SensorReading {
	float SHT2x_temp;
	float SHT2x_rh;
	int soil_moist;
	int soil_ec;
	irro_reading irrometer;
	float light;
	float DS18B20_temp;
	float internal_temp;
	uint16_t battery;
	uint8_t power_status;
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
float Read_Light_BH1750();
float Read_Temp_DS18B20();
/*End of Environmental Sensor Readings*/

/*Device Status Reading*/
float Read_Internal_Temp();
int Read_Battery_Leval();
int Read_Power_Status();
/*End of Device Status Readings*/

#endif /* INC_SENSOR_READ_H_ */
