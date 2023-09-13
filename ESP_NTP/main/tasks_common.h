/*
 * tasks_common.h
 *
 *  Created on: Dec 13, 2022
 *      Author: kolev
 */

#ifndef MAIN_TASKS_COMMON_H_
#define MAIN_TASKS_COMMON_H_

// WiFi application task
#define WIFI_APP_TASK_STACK_SIZE			4096
#define WIFI_APP_TASK_PRIORITY				5
#define WIFI_APP_TASK_CORE_ID				0

// HTTP server task
#define HTTP_SERVER_TASK_STACK_SIZE			8192
#define HTTP_SERVER_TASK_PRIORITY			4
#define HTTP_SERVER_TASK_CORE_ID			0

// HTTP server monitor task
#define HTTP_SERVER_MONITOR_STACK_SIZE		4096
#define HTTP_SERVER_MONITOR_PRIORITY		3
#define HTTP_SERVER_MONITOR_CORE_ID			0

// AM2320 Measurement
//#define TEMP_RH_MEAS_TASK_STACK_SIZE			2048
//#define TEMP_RH_MEAS_TASK_PRIORITY				6
//#define TEMP_RH_MEAS_TASK_CORE_ID				0

// MQTT task

/**
 * Parameters for the Mqtt stack are in the SDKConfig
 * defaults are
 * STACK_SIZE			6144
 * MONITOR_PRIORITY		5
 * CORE_ID				0
 */

#define MQTT_TASK_STACK_SIZE			6144
#define MQTT_TASK_PRIORITY				9
#define MQTT_TASK_CORE_ID				0

// Wifi_Reset_button task
//#define WIFI_RESET_BUTTON_TASK_STACK_SIZE	2048
//#define WIFI_RESET_BUTTON_TASK_PRIORITY		6
//#define WIFI_RESET_BUTTON_TASK_CORE_ID		0


// SNTP_time sync  task
#define SNTP_TIME_SYNC_TASK_STACK_SIZE		2048
#define SNTP_TIME_SYNC_TASK_PRIORITY		7
#define SNTP_TIME_SYNC_TASK_CORE_ID			1

// RGB_LED   task
#define RGB_LED_TASK_STACK_SIZE		2048
#define RGB_LED_TASK_PRIORITY		8
#define RGB_LED_TASK_CORE_ID		1

// ALARMS   task
#define ALARMS_TASK_STACK_SIZE		2048
#define ALARMS_TASK_PRIORITY		10
#define ALARMS_TASK_CORE_ID			1

/**
 * Functions
 */

uint16_t CRC16(uint8_t * frame, uint16_t length);

#endif /* MAIN_TASKS_COMMON_H_ */
