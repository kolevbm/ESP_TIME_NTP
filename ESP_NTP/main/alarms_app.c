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

#include "tasks_common.h"
#include "http_server.h"
#include "sntp_time_sync.h"
#include "wifi_app.h"
#include "rgb_led.h"
#include "app_nvs.h"
#include "alarms_app.h"


// EXTERN VARIABLEs
extern SemaphoreHandle_t xSemaphoreAlarms;


// GLOBAL VARIABLEs
static const char TAG[] = "alarms_app";

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

			// 1. read the alarms from NVS
			char *jsonAlarms = malloc(250);
			size_t json_size = sizeof(jsonAlarms);

			if (jsonAlarms != NULL) {
				err = app_nvs_load_alarms_p(jsonAlarms, json_size);
				ESP_LOGI(TAG, "alarms read: %s", jsonAlarms);
			}

			//todo: 2. extract the json string to a struture

			free(jsonAlarms);

			/* We have finished accessing the shared resource.  Release the
			 semaphore. */
			xSemaphoreGive(xSemaphoreAlarms);
		}
		else {
			/* We could not obtain the semaphore and can therefore not access
			 the shared resource safely. */
			err = ESP_FAIL;
		}
	}

}

static void alarm_task(void *pvParam)
{
	ESP_LOGI(TAG, "ALARMS Task started");
	time_t time_now = 0;

	// todo: get the alarms from NVS


	while (1) {


		// todo: 1. if a new alarm is stored in NVS read it, use flags

		// todo 2. get current time
//		sntp_time_sync_get_time_t();

		// todo 3. check if current time is equal to an alarm set and ring the alarm


		time_now = sntp_time_sync_get_time_t();
		struct tm time_info = { 0 };
		localtime_r(&time_now, &time_info);

		vTaskDelay(10000 / portTICK_PERIOD_MS);
	}

	vTaskDelete(NULL);
}

void alarms_task_start(void)
{
	xTaskCreatePinnedToCore(&alarm_task, "sntp_time_sync", ALARMS_TASK_STACK_SIZE,
	NULL,
						ALARMS_TASK_PRIORITY, NULL, ALARMS_TASK_CORE_ID);
}
