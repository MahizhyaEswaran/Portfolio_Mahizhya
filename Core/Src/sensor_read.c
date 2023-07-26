/*
 * sensor_read.c
 *
 *  Created on: Jul 24, 2023
 *      Author: Anistus H
 */

#include <sensor_read.h>

#include "sht2x_for_stm32_hal.h"
#include "bh1750.h"
#include "OneWire.h"


/*Environmental Sensor Setup*/
void PowerUp_Sensors(){
	HAL_GPIO_WritePin(Sensor_PWR_GPIO_Port, Sensor_PWR_Pin, RESET);
}

void PowerDown_Sensors(){
	HAL_GPIO_WritePin(Sensor_PWR_GPIO_Port, Sensor_PWR_Pin, SET);
}
//Initialize SHT2x sensor
void SHT2x_Sensor_Init(){
	HAL_I2C_Init(&hi2c1);
	SHT2x_Init(&hi2c1);
	SHT2x_SetResolution(RES_14_12);
}

//DeInitialize SHT2x sensor
void SHT2x_Sensor_DeInit(){
	HAL_I2C_DeInit(&hi2c1);
}
/*End of Environmental Sensor Setup*/

/*Environmental Sensor Readings*/
//Read environmental temperature using SHT2x sensor
float Read_Temp_SHT2x(){
	return SHT2x_GetTemperature(1);
}

//Read environmental relative humidity using SHT2x sensor
float Read_RH_SHT2x(){
	return SHT2x_GetRelativeHumidity(1);
}

//Real soil moisture using sensor connected to ADC
int Read_Soil_Moisture(){
	HAL_ADC_Init(&hadc);
	ADC1->CFGR1  |=  ADC_CFGR1_CONT;
	ADC1->CHSELR  =  ADC_CHSELR_CHSEL1;
	ADC1->SMPR  |=  ADC_SMPR_SMP;
	uint32_t moisture_adc = 0;
	uint32_t moisture = 0;
	HAL_ADC_Start(&hadc);
	HAL_ADC_PollForConversion(&hadc, HAL_MAX_DELAY);
	moisture_adc = HAL_ADC_GetValue(&hadc);
	moisture = (moisture_adc/4);
	HAL_ADC_Stop(&hadc);
	HAL_ADC_DeInit(&hadc);
	return moisture;
}

//Real soil electrical conductivity using sensor connected to ADC
int Read_Soil_EC(){
	uint32_t ec_adc= 0, ec = 0;
	HAL_ADC_Init(&hadc);
	ADC1->CFGR1  |=  ADC_CFGR1_CONT;
	ADC1->CHSELR  =  ADC_CHSELR_CHSEL7  ;
	ADC1->SMPR  |=  ADC_SMPR_SMP;
	HAL_ADC_Start(&hadc);
	HAL_ADC_PollForConversion(&hadc, HAL_MAX_DELAY);
	ec_adc = HAL_ADC_GetValue(&hadc);
	ec = (ec_adc/4);
	HAL_ADC_Stop(&hadc);
	HAL_ADC_DeInit(&hadc);
	return ec;
}

//Read Irrometer Reading
irro_reading Read_Irrometer(int num_of_read){
	HAL_ADC_Init(&hadc);
	ADC1->CFGR1  |=  ADC_CFGR1_CONT;
	ADC1->CHSELR  =  ADC_CHSELR_CHSEL9;
	ADC1->SMPR  |=  ADC_SMPR_SMP;

	double ARead_A1 = 0;
    double ARead_A2 = 0;
    int X_val=0;
    irro_reading data = {0};

    HAL_GPIO_WritePin(MUX_IN_GPIO_Port, MUX_IN_Pin, GPIO_PIN_SET);  //SET- irrometer, RESET-10K
    HAL_Delay(100);
	for (int i=0; i<num_of_read; i++){
		HAL_GPIO_WritePin(MUX2_3_GPIO_Port, MUX2_3_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(MUX1_3_GPIO_Port, MUX1_3_Pin, GPIO_PIN_SET);
//		delay(90);
		HAL_Delay(90);
		HAL_ADC_Start(&hadc);
		HAL_ADC_PollForConversion(&hadc, HAL_MAX_DELAY);
		ARead_A1 += HAL_ADC_GetValue(&hadc);
		HAL_Delay(10);
		HAL_ADC_Stop(&hadc);
		HAL_GPIO_WritePin(MUX1_3_GPIO_Port, MUX1_3_Pin, GPIO_PIN_RESET);
		HAL_Delay(40);
		HAL_GPIO_WritePin(MUX2_3_GPIO_Port, MUX2_3_Pin, GPIO_PIN_SET);
//		delay(90);
		HAL_Delay(90);
		HAL_ADC_Start(&hadc);
		HAL_ADC_PollForConversion(&hadc, HAL_MAX_DELAY);
		ARead_A2 += HAL_ADC_GetValue(&hadc);
		HAL_Delay(10);
		HAL_ADC_Stop(&hadc);
		HAL_Delay(40);
	}
	ARead_A1=ARead_A1/num_of_read;
	ARead_A2=ARead_A2/num_of_read;

	HAL_GPIO_WritePin(MUX2_3_GPIO_Port, MUX2_3_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(MUX1_3_GPIO_Port, MUX1_3_Pin, GPIO_PIN_RESET);
	HAL_Delay(1000);
	HAL_GPIO_WritePin(MUX_IN_GPIO_Port, MUX_IN_Pin, GPIO_PIN_RESET);  //SET- irrometer, RESET-10K
	for(int i=0; i<num_of_read; i++){
		HAL_GPIO_WritePin(MUX2_3_GPIO_Port, MUX2_3_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(MUX1_3_GPIO_Port, MUX1_3_Pin, GPIO_PIN_SET);
//		delay(90);
		HAL_Delay(90);
		HAL_ADC_Start(&hadc);
		HAL_ADC_PollForConversion(&hadc, HAL_MAX_DELAY);
		X_val += HAL_ADC_GetValue(&hadc);
		HAL_Delay(10);
		HAL_ADC_Stop(&hadc);
		HAL_Delay(40);
	}
	X_val=X_val/num_of_read;
	HAL_GPIO_WritePin(MUX2_3_GPIO_Port, MUX2_3_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(MUX1_3_GPIO_Port, MUX1_3_Pin, GPIO_PIN_RESET);
	HAL_Delay(1000);

	data.A1 = (uint16_t)ARead_A1;
	data.A2 = (uint16_t)ARead_A2;
	data.X = (uint16_t)X_val;
	HAL_ADC_DeInit(&hadc);

	return data;
}

//Read light intensity using BH1750 sensor
uint16_t Read_Light_BH1750(){
	HAL_I2C_Init(&hi2c2);
	float light_f = 0;
	BH1750_Init(&hi2c2);
	BH1750_SetMode(CONTINUOUS_HIGH_RES_MODE_2);
	HAL_Delay(1000);
	BH1750_ReadLight(&light_f);
	HAL_Delay(1000);
	HAL_I2C_DeInit(&hi2c2);
	return (uint16_t)light_f;
}

//Read environmental temperature using DS18B20 sensor
float Read_Temp_DS18B20(){
	float temp = 0;
	HAL_UART_Init(&huart2);
	HAL_Delay(500);
	get_temp_ds18b20(&temp);
	HAL_UART_DeInit(&huart2);
	return temp;
}
/*End of Environmental Sensor Readings*/

/*Device Status Readings*/
uint8_t Read_Internal_Temp(){
	HAL_ADC_Init(&hadc);	// initializing ADC peripheral
	ADC1->CFGR1  |=  ADC_CFGR1_CONT;	// set ADC peripheral to continuous mode
	ADC1->CHSELR  =  ADC_CHSELR_CHSEL18;	// select channel for conversion
	ADC1->SMPR  |=  ADC_SMPR_SMP;	// sets the sampling time
	ADC->CCR  |=  ADC_CCR_TSEN;
	HAL_ADC_Start(&hadc);
	HAL_ADC_PollForConversion(&hadc, 100);
	uint16_t ADC_VAL = 0;
	// wait for ADC conversion to complete
	ADC_VAL = HAL_ADC_GetValue(&hadc);		// get reading
	HAL_ADC_Stop(&hadc);
	HAL_ADC_DeInit(&hadc);
	float temperature = 0;
	temperature  =  ((ADC_VAL *  VDD_APPLI  /  VDD_CALIB) - (int32_t)  *TEMP30_CAL_ADDR  );
	temperature  =  temperature  *  (int32_t)(130  -  30);
	temperature  =  temperature  /  (int32_t)(*TEMP130_CAL_ADDR - *TEMP30_CAL_ADDR);
	temperature  =  temperature  +  30;
	return (uint8_t)temperature;
}

int Read_Battery_Leval(){
	int batteryADCReading = 0, batteryLevel = 0;
	HAL_ADC_Init(&hadc);  // initializing ADC peripheral
	ADC1->CFGR1  |=  ADC_CFGR1_CONT;	// set ADC peripheral to continuous mode
	ADC1->CHSELR  =  ADC_CHSELR_CHSEL0;  	// select channel for conversion
	ADC1->SMPR  |=  ADC_SMPR_SMP;	// sets the sampling time
	HAL_ADC_Start(&hadc);
	HAL_ADC_PollForConversion(&hadc, HAL_MAX_DELAY); 	// wait for ADC conversion to complete
	batteryADCReading = HAL_ADC_GetValue(&hadc);	// get reading
	batteryLevel = (batteryADCReading/4);
	HAL_ADC_Stop(&hadc);
	HAL_ADC_DeInit(&hadc);
	return batteryLevel;
}

int Read_Power_Status(){
	//power failure detection
	if (HAL_GPIO_ReadPin(PWR_MEASURE_GPIO_Port, PWR_MEASURE_Pin) == GPIO_PIN_SET){
		return 1;
	}else{
		return 0;
	}
}
/*End of Device Status Readings*/
