/*
 * alarms_app.c
 *
 *  Created on: Sep 13, 2023
 *      Author: kolev
 */

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/apps/sntp.h"				// lwip light weight IP libraries
#include "driver/gpio.h"

#include "board_config.h"

#include "tasks_common.h"
#include "http_server.h"
#include "sntp_time_sync.h"
#include "wifi_app.h"
#include "rgb_led.h"
#include "app_nvs.h"
#include "alarms_app.h"
#include "cJSON.h"

// EXTERN VARIABLEs
extern SemaphoreHandle_t xSemaphoreAlarms;
extern bool timeIsSynchronized;
extern bool f_newAlarm;

// GLOBAL VARIABLEs
static const char TAG[] = "alarms_app";

char alarm1[] = "hh:mm";
char alarm2[] = "hh:mm";
char alarm3[] = "hh:mm";
char alarm4[] = "hh:mm";
char alarm5[] = "hh:mm";
char alarm6[] = "hh:mm";
char alarm7[] = "hh:mm";
int alarm_delay_ms = 2000;
int alarm_repetition = 3;

// FUNCTION DECLARATIONs

/**
 * Reads the alarms from the NVS json string
 * and convert them to data structures
 * @return void
 */
void alarms_processing(void);

/**
 * Reads the alarms from the NVS json string
 * and convert them to data structures
 * @return void
 */
void alarms_processing(void);

/**
 * Reads the alarms from the NVS json string
 * and convert them to data structures
 * @return void
 */
void alarms_processing(void)
{
	esp_err_t err = ESP_OK;
	if (xSemaphoreAlarms != NULL) {
		/* See if we can obtain the semaphore.  If the semaphore is not
		 available wait 10 ticks to see if it becomes free. */
		if ( xSemaphoreTake( xSemaphoreAlarms, ( TickType_t ) 10 ) == pdTRUE) {
			/* We were able to obtain the semaphore and can now access the
			 shared resource. */

			//todo: 1. read the alarms from NVS
			char *jsonAlarms = malloc(300);
			size_t json_size = sizeof(jsonAlarms);

			if (jsonAlarms != NULL) {
				err = app_nvs_load_alarms_p(jsonAlarms, json_size);
				if (err == ESP_OK) {
					//todo: 2. extract the json string to a struture
					/*
					 * {"alarm1" : "10:00",  "alarm2" : "23:25", "alarm3" : "12:00", "alarm4" : "12:30", "alarm5" : "15:00", "alarm6" : "15:15", "alarm7" : "17:00"}
					 */
					const cJSON *json_alarm1 = NULL;
					const cJSON *json_alarm2 = NULL;
					const cJSON *json_alarm3 = NULL;
					const cJSON *json_alarm4 = NULL;
					const cJSON *json_alarm5 = NULL;
					const cJSON *json_alarm6 = NULL;
					const cJSON *json_alarm7 = NULL;
					const cJSON *json_delay = NULL;
					const cJSON *json_repetition = NULL;


					cJSON *json_body = cJSON_Parse(jsonAlarms);

					if (json_body == NULL) {
						const char *error_ptr = cJSON_GetErrorPtr();
						if (error_ptr != NULL) {
							ESP_LOGI(TAG, "Error before: %s\n", error_ptr);
						}
					}

					// alarm 1
					json_alarm1 = cJSON_GetObjectItemCaseSensitive(json_body, "alarm1");
					if (cJSON_IsString(json_alarm1) && (json_alarm1->valuestring != NULL)) {
						strcpy(alarm1, json_alarm1->valuestring);
						ESP_LOGI(TAG, "Checking monitor \"%s\"\n", json_alarm1->valuestring);
					}
					// alarm 2
					json_alarm2 = cJSON_GetObjectItemCaseSensitive(json_body, "alarm2");
					if (cJSON_IsString(json_alarm2) && (json_alarm2->valuestring != NULL)) {
						strcpy(alarm2, json_alarm2->valuestring);
						ESP_LOGI(TAG, "Checking monitor \"%s\"\n", json_alarm2->valuestring);
					}
					// alarm 3
					json_alarm3 = cJSON_GetObjectItemCaseSensitive(json_body, "alarm3");
					if (cJSON_IsString(json_alarm3) && (json_alarm3->valuestring != NULL)) {
						strcpy(alarm3, json_alarm3->valuestring);
						ESP_LOGI(TAG, "Checking monitor \"%s\"\n", json_alarm3->valuestring);
					}
					// alarm 4
					json_alarm4 = cJSON_GetObjectItemCaseSensitive(json_body, "alarm4");
					if (cJSON_IsString(json_alarm4) && (json_alarm4->valuestring != NULL)) {
						strcpy(alarm4, json_alarm4->valuestring);
						ESP_LOGI(TAG, "Checking monitor \"%s\"\n", json_alarm4->valuestring);
					}
					// alarm 5
					json_alarm5 = cJSON_GetObjectItemCaseSensitive(json_body, "alarm5");
					if (cJSON_IsString(json_alarm5) && (json_alarm5->valuestring != NULL)) {
						strcpy(alarm5, json_alarm5->valuestring);
						ESP_LOGI(TAG, "Checking monitor \"%s\"\n", json_alarm5->valuestring);
					}
					// alarm 6
					json_alarm6 = cJSON_GetObjectItemCaseSensitive(json_body, "alarm6");
					if (cJSON_IsString(json_alarm6) && (json_alarm6->valuestring != NULL)) {
						strcpy(alarm6, json_alarm6->valuestring);
						ESP_LOGI(TAG, "Checking monitor \"%s\"\n", json_alarm6->valuestring);
					}
					// alarm 7
					json_alarm7 = cJSON_GetObjectItemCaseSensitive(json_body, "alarm7");
					if (cJSON_IsString(json_alarm7) && (json_alarm7->valuestring != NULL)) {
						strcpy(alarm7, json_alarm7->valuestring);
						ESP_LOGI(TAG, "Checking monitor \"%s\"\n", json_alarm7->valuestring);
					}
					// alarm delay
					json_delay = cJSON_GetObjectItemCaseSensitive(json_body, "delay");
					if (cJSON_IsString(json_delay) && (json_delay->valueint != NULL)) {
						alarm_delay_ms = json_delay->valueint;
						alarm_delay_ms *=1000;
						ESP_LOGI(TAG, "Checking monitor \"%i\"\n", json_delay->valueint);
					}
					// alarm repetition
					json_repetition = cJSON_GetObjectItemCaseSensitive(json_body, "repetition");
					if (cJSON_IsString(json_repetition) && (json_repetition->valueint != NULL)) {
						alarm_repetition = json_repetition->valueint;
						ESP_LOGI(TAG, "Checking monitor \"%i\"\n", json_repetition->valueint);
					}

					cJSON_Delete(json_body); 	// free space from malloc

				}
				else {
					ESP_LOGI(TAG, "error reading alarms, do not proccess any further");
				}
			}

			free(jsonAlarms);				// free space from malloc

			/* We have finished accessing the shared resource.  Release the
			 semaphore. */
			xSemaphoreGive(xSemaphoreAlarms);
		}
		else {
			/* We could not obtain the semaphore and can therefore not access
			 the shared resource safely. */
			ESP_LOGI(TAG, "Could not obtain xSemaphoreAlarms");
			err = ESP_FAIL;
		}
	}
}
/*
 * configure alarm output
 */
static void configure_GPIO(void)
{
	esp_rom_gpio_pad_select_gpio(ALARM_GPO);
	gpio_set_direction(ALARM_GPO, GPIO_MODE_OUTPUT);
}

/*
 * Trigger alarm
 */
static void triggerAlarm(int delay_ms, int repetitions)
{
	ESP_LOGI(TAG,"Alarm triggerring");
	for (int i = 0; i < repetitions; ++i) {
		gpio_set_level(ALARM_GPO, 1);
		vTaskDelay(pdMS_TO_TICKS(delay_ms));
		gpio_set_level(ALARM_GPO, 0);
		vTaskDelay(pdMS_TO_TICKS(delay_ms));
	}
}

static void alarm_task(void *pvParam)
{
	ESP_LOGI(TAG, "ALARMS Task started");

	time_t time_now = 0;

	// todo: 0. get the alarms from NVS
	// give time for initialization of semaphore in http task
	vTaskDelay(5000 / portTICK_PERIOD_MS);
	// make the initial reading
	alarms_processing();
	configure_GPIO();


	while (1) {

		// todo: 1. if a new alarm is stored in NVS read it, use flags
		if (f_newAlarm) {
			ESP_LOGI(TAG, "New alarms recorded, performing alarm reading");
			alarms_processing();
			f_newAlarm = false;
		}

		// todo 2. get current time

		//create an empty struct to store the time in
		struct tm time_info = { 0 };
		// get current time
		time(&time_now);
		// Convert given time since epoch (a time_t value pointed to by timer) into calendar time
		localtime_r(&time_now, &time_info);

		/* todo 3. check if current time is Synchronized
		 * and equal to an alarm set and ring the alarm
		 */
		if (timeIsSynchronized) {
			ESP_LOGI(TAG, "Time Synchronized, alarms will be triggered");
			char currentTime[] = "hh:mm";
			sprintf(currentTime, "%02i:%02i", time_info.tm_hour, time_info.tm_min);
			ESP_LOGI(TAG, "Time is: %s", currentTime);

			if (strcmp(alarm1, currentTime) == 0) {
				triggerAlarm(alarm_delay_ms, alarm_repetition);
			}
			else if (strcmp(alarm2, currentTime) == 0) {
				triggerAlarm(alarm_delay_ms, alarm_repetition);
			}
			else if (strcmp(alarm3, currentTime) == 0) {
				triggerAlarm(alarm_delay_ms, alarm_repetition);
			}
			else if (strcmp(alarm4, currentTime) == 0) {
				triggerAlarm(alarm_delay_ms, alarm_repetition);
			}
			else if (strcmp(alarm5, currentTime) == 0) {
				triggerAlarm(alarm_delay_ms, alarm_repetition);
			}
			else if (strcmp(alarm6, currentTime) == 0) {
				triggerAlarm(alarm_delay_ms, alarm_repetition);
			}
			else if (strcmp(alarm7, currentTime) == 0) {
				triggerAlarm(alarm_delay_ms, alarm_repetition);
			}
		}
		else {
			ESP_LOGI(TAG, "Time is not Synchronized, alarms will not be triggered");
		}
		vTaskDelay(60000 / portTICK_PERIOD_MS);
	}

	vTaskDelete(NULL);
}

void alarms_task_start(void)
{
	xTaskCreatePinnedToCore(&alarm_task, "alarm_task", ALARMS_TASK_STACK_SIZE,
	NULL,
									ALARMS_TASK_PRIORITY, NULL, ALARMS_TASK_CORE_ID);
}
