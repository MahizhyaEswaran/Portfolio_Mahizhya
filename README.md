# SenzAgro DigiPlant V3


## Introduction

The DigiPlant device is an environmental monitoring device equipped with multiple sensors, including temperature, humidity, soil moisture, electrical conductivity, light intensity, and soil temperature. Additionally, the device features an irrometer sensor. This device is designed to gather various environmental parameters and transmit them to an online server using MQTT through a GSM module. 

This firmware represents the third iteration of the firmware developed for this device, incorporating a multitude of features. With this firmware version, the device's sensor configuration can be tailored to specific requirements. Notably, this firmware includes flash storage functionality, which effectively manages connectivity issues in low coverage areas. Moreover, this firmware enables remote configuration adjustments, eliminating the need for physical access to the device. Lastly, the firmware offers Firmware Over-The-Air (FOTA) capability, facilitating seamless firmware updates.

### Firmware Flow

1. Upon device startup, the firmware initiates by checking the availability of the configuration block in the flash memory. If the configuration block exists, it's copied to RAM for usage. If not, the device configures itself using default settings, storing this newly configured block in the flash memory.

2. Subsequently, the firmware examines the bootloader configuration. This information aids in recognizing the prior operation status of the bootloader and its ongoing state. Using this data, the firmware communicates an acknowledgment to the server.

3. The firmware then enters the main program loop, where it remains unless reset or a Firmware Over-The-Air (FOTA) request is received.

4. Inside this loop, the firmware first checks the wakeup flag and remote configuration flag. If set, it proceeds with the relevant operation. Otherwise, it enters a sleep mode, periodically waking to check for the flag settings, managed by the Real-Time Clock (RTC) wakeup handler.

5. Should the wakeup flag be active, the firmware enters the data transmission part of its operation. Here, it evaluates the battery reading. If the battery can adequately power the GSM module, the firmware reads sensor data and transmits it to the server through MQTT. Prior to sending current data, any previously stored data in the flash is dispatched.

6. In cases where battery power is sufficient solely for sensor operation, the firmware gathers sensor data, storing it in the flash memory, and returns to sleep mode.

7. If battery power is critically low, the firmware immediately enters sleep mode.

8. In the event the remote configuration flag is set, the firmware first subscribes to the remote configuration topic.

9. If any configurations are available within the topic, the firmware implements these configurations before returning to sleep mode. It remains in this state, awaiting the next operational cycle.

10. Furthermore, if the received configuration contains a Firmware Over-The-Air (FOTA) request, the firmware updates the URL for the new firmware within the bootloader configuration. Subsequently, the device is reset to enable the bootloader to handle the FOTA process seamlessly.


## Configuration

Before configuring anything within the firmware, a crucial prerequisite is the presence of the FOTA bootloader firmware in the flash memory.

Begin by installing the bootloader firmware onto the device, following the instructions provided in the FOTA bootloader repository.

As outlined in the FOTA documentation, the bootloader must reside in the bootloader slot, while this firmware should be installed in app slot 1. However, before deploying this firmware to app slot 1, specific configurations need to be addressed, as detailed in the `app_conf.h` file.

Commence by setting the MQTT parameters essential for MQTT configuration. The following snippet demonstrates this configuration:

```c
/*-----------------------MQTT Configuration-----------------------*/
#define APN "apn"
#define MQTT_HOST "server_ip_or_domain (e.g., 52.24.122.165)"
#define MQTT_ID "client_id"
#define MQTT_PORT 1883
#define MQTT_USER "username"
#define MQTT_PASS "password"
#define KEEP_ALIVE 255
#define PING_TIME 0         // 0 indicates no pinging

//#define IMEI_Topic
#define PublishTopic "D2S/SA/V1/sample_topic/S"
//#define SubscribeTopic "sample_topic/#"
#define RmConfTopic "S2D/SA/V1/sample_topic/C/#"
/*--------------------END of MQTT Configuration-------------------*/
```

Note that if you prefer to use the IMEI number as topics instead of custom topics, you can uncomment the `IMEI_Topic` definition, which will overwrite the custom topics.

Following the MQTT configurations, proceed to the sensor configuration in the same file. Here's the relevant part:

```c
/*----------------------Sensor Configuration----------------------*/
#define SHT2x_Temp_EN
#define SHT2x_RH_EN
#define Moist_EN
#define EC_EN
#define Irro_EN
#define Light_EN
#define DS18B20_Temp_EN
#define Power_status_EN
/*-------------------END of Sensor Configuration------------------*/
```

This permits you to select only the required sensors. Simply comment out those not needed.

Then, define the wakeup period and the remote configuration check period in the main flow configuration:

```c
/*---------------------Main Flow Configuration--------------------*/
#define WAKEUP_TIME 5	//time in minutes

//Uncomment if you need an extra connection attempt after the initial
//connection attempt fails.
//#define EXTRA_MQTT_CON_TRY

#define CONFIG_PERIOD 10
/*-----------------END of Main Flow Configuration-----------------*/
```

Once these configurations are set, you can proceed to install the code in app slot 1. This can be achieved by debugging the code or using STM32Programmer. Instructions for these procedures can be found in the "Build and Debug" section.

## Remote Configuration Guide

This firmware incorporates a remote configuration feature, enabling the adjustment of basic settings such as wakeup period, configuration period, and MQTT parameters. Additionally, remote configuration can initiate FOTA updates.

Remote configuration commands should be transmitted to the designated remote configuration topic from the server. These commands should adhere to the format outlined below:

```
RM|index-value;index-value;...|END
```

Here, the `index` refers to the parameter to be configured, while 'value' represents the new value for that parameter. The list of available indices and corresponding examples are detailed below:

- **1** - MQTT Server Address (e.g., RM|1-52.24.122.165;|END)
- **2** - MQTT Client ID (e.g., RM|2-client_id_01;|END)
- **3** - APN (e.g., RM|3-dialogbb;|END)
- **4** - MQTT Port Number (e.g., RM|4-1883;|END)
- **5** - MQTT Username (e.g., RM|5-username;|END)
- **6** - MQTT Password (e.g., RM|6-password;|END)
- **7** - Wakeup Period (e.g., RM|7-10;|END (min))
- **8** - Remote Config Period (e.g., RM|8-120;|END (min))
- **9** - MQTT Publish Topic (e.g., RM|9-D2S/SA/V1/sample_topic/S;|END)
- **10** - MQTT Subscribe Topic (e.g., RM|10-S2D/SA/V1/sample_topic/#;|END)
- **11** - MQTT Remote Config Topic (e.g., RM|11-S2D/SA/V1/sample_topic/C/#;|END)
- **12** - IMEI_Topic Selection (e.g., RM|12-1;|END (select imei topic))
- **97** - FOTA Request (e.g., RM|97-url;|END)
- **98** - Bootloader Config Reset (e.g., RM|98-1;|END)
- **99** - Device Reset (e.g., RM|99-1;|END)

It's possible to combine multiple configurations into a single MQTT message. However, the length of the MQTT message must be less than 100 characters.

Since the device subscribes to a wildcard topic, you can send multiple configurations in different topics simultaneously, as demonstrated below:

```plaintext
S2D/SA/V1/sample_topic/C/1 -> RM|7-10;8-60;|END 
S2D/SA/V1/sample_topic/C/2 -> RM|1-mqtt.senzmate.com;2-mqtt_id_1;|END 
S2D/SA/V1/sample_topic/C/3 -> RM|9-2S/SA/V1/new_topic/S;|END
```

By leveraging this remote configuration guide, you can efficiently adjust various parameters of the device's behavior and initiate FOTA updates through remote communication.

## Build and Debug
To build and debug the project for the STM32 microcontroller, you can follow the standard STM32 build and debug process. Here are the general steps:

1. Set up your development environment, such as STM32CubeIDE, and ensure you have the necessary tools and software installed.

2. Open the master project file in your chosen development environment.

3. Configure the project settings and build options according to your requirements. Make sure to select the correct target device and compiler options.

4. Build the project to compile the source code and generate the executable file.

5. Connect your STM32 development board to your computer using a suitable programming/debugging interface, such as ST-Link or J-Link.

6. Configure the debugger settings in your development environment to connect to the STM32 board. Ensure that the necessary debug probe drivers are installed.

7. Start the debugging session and load the compiled code onto the STM32 board.

8. During the debugging process, monitor the console or debugging output to verify the behavior of the code and ensure it interacts correctly with the slave devices.

It is crucial to ensure that the slave code is also running on a separate device while running the master code. This allows proper communication and synchronization between the master and slave devices.