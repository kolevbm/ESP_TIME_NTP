/*
 * sntp_time_sync.c
 *
 *  Created on: Dec 27, 2022
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
#include "json.h"

static const char TAG[] = "sntp_time_sync";

// EXTERN VARIABLES
extern SemaphoreHandle_t xSemaphoreAlarms;
// SNTP operating mode set status
static bool sntp_op_mode_set = false;

/**
 * Initialize SNTP service using SNTP_OPMODE_POLL mode.
 */
static void sntp_time_sync_init_sntp(void)
{
	ESP_LOGI(TAG, "Initializing the SNTP service");

	if (!sntp_op_mode_set)
	{
		// Set the operating mode
		sntp_setoperatingmode(SNTP_OPMODE_POLL);
		sntp_op_mode_set = true;
	}

	sntp_setservername(0, "pool.ntp.org");

	// Initialize the servers
	sntp_init();

	// Let the http_server know service is initialized
	http_server_monitor_send_message(HTTP_MSG_TIME_SERVICE_INITIALIZED);
}

/**
 * Gets the current time and if the current time is not up to date,
 * the sntp_time_synch_init_sntp function is called.
 */
static void sntp_time_sync_obtain_time(void)
{
	time_t now = 0;
	struct tm time_info = {0};

	time(&now);
	localtime_r(&now, &time_info);

	// Check the time, in case we need to initialize/reinitialize
	if (time_info.tm_year < (2016 - 1900))
	{
		sntp_time_sync_init_sntp();
		// Set the local time zone
		setenv("TZ", "EET-2EEST-3,M3.5.0/03:00:00,M10.5.0/04:00:00", 1);
		tzset();
		set_led_work_state(LED_WORK_STATE_SLOW_BLINK);
	}
}

/**
 * The SNTP time synchronization task.
 * @param arg pvParam.
 */
static void sntp_time_sync(void *pvParam)
{
  ESP_LOGI(TAG, "SNTP Task started");
	while (1)
	{
		sntp_time_sync_obtain_time();
		vTaskDelay(10000 / portTICK_PERIOD_MS);
	}

	vTaskDelete(NULL);
}

char* sntp_time_sync_get_time(void)
{
	static char time_buffer[100] = {0};

	time_t now = 0;
	struct tm time_info = {0};

	time(&now);
	localtime_r(&now, &time_info);

	if (time_info.tm_year < (2016 - 1900))
	{
		ESP_LOGI(TAG, "Time is not set yet");
	}
	else
	{
		strftime(time_buffer, sizeof(time_buffer), "%d.%m.%Y %H:%M:%S", &time_info);
		ESP_LOGI(TAG, "Current time info: %s", time_buffer);
	}

	return time_buffer;
}

time_t sntp_time_sync_get_time_t(void)
{
	static char time_buffer[100] = {0};
	time_t now = 0;
	struct tm time_info = {0};

	time(&now);
	localtime_r(&now, &time_info);

	if (time_info.tm_year < (2016 - 1900))
	{
		ESP_LOGI(TAG, "Time is not set yet");
	}
	else
	{
		strftime(time_buffer, sizeof(time_buffer), "%d.%m.%Y %H:%M:%S", &time_info);
		ESP_LOGI(TAG, "Current time info: %s", time_buffer);
	}

	return now;
}

void alarms_processing(void)
{
	esp_err_t err = ESP_OK;
	if (xSemaphoreAlarms != NULL) {
		/* See if we can obtain the semaphore.  If the semaphore is not
		 available wait 10 ticks to see if it becomes free. */
		if ( xSemaphoreTake( xSemaphoreAlarms, ( TickType_t ) 10 ) == pdTRUE) {
			/* We were able to obtain the semaphore and can now access the
			 shared resource. */
			char *jsonAlarms = malloc(250);
			size_t json_size = sizeof(jsonAlarms);

			if (jsonAlarms != NULL) {
				err = app_nvs_load_alarms_p(jsonAlarms, json_size);
				ESP_LOGI(TAG, "alarms read: %s", jsonAlarms);


			}

			free(jsonAlarms);
//			app_nvs_load_alarms_p

			/* ... */
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

void sntp_time_sync_task_start(void)
{
	xTaskCreatePinnedToCore(&sntp_time_sync, "sntp_time_sync", SNTP_TIME_SYNC_TASK_STACK_SIZE, NULL, SNTP_TIME_SYNC_TASK_PRIORITY, NULL, SNTP_TIME_SYNC_TASK_CORE_ID);
}


