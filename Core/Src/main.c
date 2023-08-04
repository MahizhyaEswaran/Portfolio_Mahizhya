/* USER CODE BEGIN Header */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "i2c.h"
#include "iwdg.h"
#include "rtc.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32l0xx_hal_def.h"
#include "stm32l0xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <ctype.h>
#include <flash_queue.h>
#include <app_conf.h>
#include <sensor_read.h>
#include <rm_config_block.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
//
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void MainLoop();
sensor_reading get_sensor_readings();
void prepare_mqtt_msg(sensor_reading *sensor, char *mqtt);
void get_time(sensor_reading *sensor);
void store_to_flash(sensor_reading *sensor);
void publish_from_flash(int tries);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t wakeup = 0;
uint8_t remote_conf = 0;
flash_queue_t flashqueue;
int error = 100;
RM_ConfigBlock rm_config = {0};

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	//
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
//  MX_IWDG_Init();
  MX_RTC_Init();
  MX_ADC_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_UART_DeInit(&huart1);
  HAL_UART_DeInit(&huart2);
  HAL_ADC_DeInit(&hadc);
  HAL_I2C_DeInit(&hi2c1);
  HAL_I2C_DeInit(&hi2c2);

  error = config_rm_block_init(&rm_config);
  My_RTC_Set_Wakeup();

  flash_queue_init(&flashqueue);
  wakeup = 1;
  remote_conf = 1;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

	while (1)
	{
		MainLoop();
	}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE
                              |RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2
                              |RCC_PERIPHCLK_I2C1|RCC_PERIPHCLK_RTC;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void MainLoop(){
	if(wakeup){
		wakeup = 0;
		HAL_Delay(1000);
		int battery = Read_Battery_Leval();
		sensor_reading new_reading = {0};

		//for testing
		battery = 275;
		if(battery >= 270){
			//full process (read sensor and publish to server)
			new_reading = get_sensor_readings();

			HAL_UART_Init(&huart1);
			MQTT_Init(rm_config.apn, rm_config.mqtt_host, rm_config.mqtt_port, rm_config.mqtt_id,
					rm_config.mqtt_username, rm_config.mqtt_password, KEEP_ALIVE, PING_TIME);
			error = MQTT_Connect();
#ifdef EXTRA_MQTT_CON_TRY
			if(error <= 0){
				//connection failed
				MQTT_Process();
			}
#endif

			if(MQTT_Ready){
				get_time(&new_reading);
				new_reading.signal_strength = signalStrength;
				//publish from flash
				publish_from_flash(2);

				if(MQTT_Ready){
					//publish current data
					char mqtt_msg[150] = {0};
					prepare_mqtt_msg(&new_reading, mqtt_msg);
					MQTT_Publish(rm_config.mqtt_pub_topic, mqtt_msg, 0);
				}

				if(!MQTT_Ready){
					//stored to flash
					store_to_flash(&new_reading);
				}
			}else{
				//stored to flash
				store_to_flash(&new_reading);
			}

			if(!remote_conf){
				HAL_UART_DeInit(&huart1);
				GSM_OFF();
			}
		}else if(battery >= 261){
			//only sensor reading
			new_reading = get_sensor_readings();
			//store on the flash
			store_to_flash(&new_reading);
		}
	}

	if(remote_conf){
		Mqtt_sub_str mqtt = {0};
		memcpy(mqtt.topic[0],rm_config.mqtt_rm_conf_topic,strlen(rm_config.mqtt_rm_conf_topic));
		mqtt.qos[0] = 0;
		mqtt.no_of_topics = 1;
		error = MQTT_Subscribe(&mqtt);

		if(error){
			remote_conf = 0;
			if(mqtt_queue_count(&mqtt_data_queue)){
				set_rm_config_via_remote(&rm_config, &mqtt_data_queue);
			}
		}
		HAL_UART_DeInit(&huart1);
		GSM_OFF();
	}

	HAL_SuspendTick();
	HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

	HAL_ResumeTick();
	SystemClock_Config();
}

sensor_reading get_sensor_readings(){
	PowerUp_Sensors();
#ifdef SHT2x_EN
	SHT2x_Sensor_Init();
#endif
	HAL_Delay(5000);

	sensor_reading sensor = {0};
#ifdef SHT2x_Temp_EN
	sensor.SHT2x_temp = Read_Temp_SHT2x();
#endif
#ifdef SHT2x_RH_EN
	sensor.SHT2x_rh = Read_RH_SHT2x();
#endif
#ifdef Moist_EC_EN
	sensor.soil_moist = Read_Soil_Moisture();
	sensor.soil_ec = Read_Soil_EC();
#endif
#ifdef Irro_EN
	sensor.irrometer = Read_Irrometer(3);
#endif
#ifdef Light_EN
	sensor.light = Read_Light_BH1750();
#endif
#ifdef DS18B20_Temp_EN
	sensor.DS18B20_temp = Read_Temp_DS18B20();
#endif
	sensor.internal_temp = Read_Internal_Temp();
	sensor.battery = Read_Battery_Leval();
#ifdef Power_status_EN
	sensor.power_status = Read_Power_Status();
#endif
	sensor.signal_strength = signalStrength;

	get_time(&sensor);

#ifdef SHT2x_EN
	SHT2x_Sensor_DeInit();
#endif
	PowerDown_Sensors();
	return sensor;
}

void prepare_mqtt_msg(sensor_reading *sensor, char *mqtt){
	int sensor_count = 0;
	char data[30] = {0};

	//date and time
	if(((sensor->timestamp.month == 0) && (sensor->timestamp.day == 0)
			&& (sensor->timestamp.hour == 0) && (sensor->timestamp.min == 0))){
		sprintf(data,"DT:0|");
		strncat(mqtt, data, strlen(data));
		memset(data, 0, sizeof(data));
	}else{
		sprintf(data,"ZZ:%02d%02d%02d%02d/%s|", sensor->timestamp.month, sensor->timestamp.day,
				sensor->timestamp.hour, sensor->timestamp.min, timeZone);
		strncat(mqtt, data, strlen(data));
		memset(data, 0, sizeof(data));
	}

#ifdef SHT2x_Temp_EN
	//SHT2x temperature
	sprintf(data,"%d-T:%.2f;",sensor_count++, sensor->SHT2x_temp);
	strncat(mqtt, data, strlen(data));
	memset(data, 0, sizeof(data));
#endif

#ifdef SHT2x_RH_EN
	//SHT2x relative humidity
	sprintf(data,"%d-H:%.2f;",sensor_count++, sensor->SHT2x_rh);
	strncat(mqtt, data, strlen(data));
	memset(data, 0, sizeof(data));
#endif

#ifdef Moist_EC_EN
	//soil moisture and EC
	sprintf(data,"%d-MEA4:%03d/%03d;",sensor_count++, sensor->soil_moist, sensor->soil_ec);
	strncat(mqtt, data, strlen(data));
	memset(data, 0, sizeof(data));
#endif

#ifdef Irro_EN
	//irrometer reading
	sprintf(data,"%d-IRO:%04d/%04d/%.2f;",sensor_count++, sensor->irrometer.A1, sensor->irrometer.A2, sensor->SHT2x_temp);
	strncat(mqtt, data, strlen(data));
	memset(data, 0, sizeof(data));
#endif

#ifdef Light_EN
	//light intensity
	sprintf(data,"%d-LIA1:%06d;",sensor_count++, sensor->light);
	strncat(mqtt, data, strlen(data));
	memset(data, 0, sizeof(data));
#endif

#ifdef DS18B20_Temp_EN
	//soil temperature (DS18b20)
	sprintf(data,"%d-ST:%.2f;",sensor_count++, sensor->DS18B20_temp);
	strncat(mqtt, data, strlen(data));
	memset(data, 0, sizeof(data));
#endif

	//device status
	sprintf(data,"%d-B:%03d;%d-IT:%02d;",sensor_count, sensor->battery,sensor_count+1, sensor->internal_temp);
	strncat(mqtt, data, strlen(data));
	memset(data, 0, sizeof(data));
	sensor_count+=2;

	//signal strength
	sprintf(data,"%d-SS:%02d;",sensor_count++, sensor->signal_strength);
	strncat(mqtt, data, strlen(data));
	memset(data, 0, sizeof(data));

#ifdef Power_status_EN
	//power status
	sprintf(data,"%d-PS:%d;",sensor_count++, sensor->power_status);
	strncat(mqtt, data, strlen(data));
	memset(data, 0, sizeof(data));
#endif
}

void get_time(sensor_reading *sensor){
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

	if( (!RTC_OK) && (sDate.Year < 22) ){
		sensor->timestamp.month = 0;
		sensor->timestamp.day = 0;
		sensor->timestamp.hour = 0;
		sensor->timestamp.min = 0;
	}else{
		sensor->timestamp.month = sDate.Month;
		sensor->timestamp.day = sDate.Date;
		sensor->timestamp.hour = sTime.Hours;
		sensor->timestamp.min = sTime.Minutes;
	}
}

void store_to_flash(sensor_reading *sensor){
	flash_str new = {0};
	new.sensor = *sensor;
	flash_enqueue(&flashqueue, &new);
	HAL_Delay(1);
}

void publish_from_flash(int tries){
	while(flash_queue_count(&flashqueue) && tries){
		flash_str data = {0};
		flash_peek(&flashqueue, &data);

		char mqtt_msg[150] = {0};
		prepare_mqtt_msg(&data.sensor, mqtt_msg);

		if(MQTT_Publish(rm_config.mqtt_pub_topic, mqtt_msg, 0)){
			flash_dequeue(&flashqueue, &data);
		}else{
			//publish failed
			tries--;
			if(!tries){
				break;
			}
			MQTT_Process();
		}
	}
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	//  /* User can add his own implementation to report the HAL error return state */
	//  __disable_irq();
	//  while (1)
	//  {
	//  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
