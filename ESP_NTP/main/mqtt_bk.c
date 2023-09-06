/*
 * mqtt_bk.c
 *	Based on MQTT (over TCP) Example
 *  Created on: Jan 17, 2023
 *      Author: kolev
 */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_event_base.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "driver/gpio.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "tasks_common.h"
#include "mqtt_bk.h"
#include "wifi_app.h"
#include "board_config.h"
#include "rgb_led.h"

// EXTERN VARIABLES
extern uint8_t ed_type;
extern uint16_t ed_id;
extern uint8_t ed_battery;
extern uint8_t ed_battery_alm;
extern float ed_weight;
extern float ed_temp;
extern float ed_hum;
extern SemaphoreHandle_t uartMutex;
extern float temperature;
extern float humidity;
extern uint16_t serialNumber;
extern bool g_sta_connected;

// GLOBAL VARIBLES

bool f_newValues;
bool mqttConnected = false;
bool client_initialized = false;
bool closeClientConnection = false;

esp_mqtt_client_config_t mqtt_cfg;
esp_mqtt_client_handle_t client;

static const char *TAG = "MQTT";

// STRUCTS AND UNIONS
struct {
	int64_t elapsedTime;
	uint8_t seconds;
	uint64_t inSeconds;
	uint8_t minutes;
	uint8_t hours;
	uint8_t days;
} uptime;
// LOCAL FUNCTIONS

static void log_error_if_nonzero(const char *message, int error_code)
{
	if (error_code != 0) {
		ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
	}
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
	ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%ld", base, event_id);
	esp_mqtt_event_handle_t event = event_data;
	esp_mqtt_client_handle_t client = event->client;
//  int msg_id;
	switch ((esp_mqtt_event_id_t) event_id) {
	case MQTT_EVENT_CONNECTED:
		ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
		mqttConnected = true;
		set_led_work_state(LED_WORK_STATE_SLOW_BLINK);
		char *baseTopic = (char*) malloc(40);
		sprintf(baseTopic, "beem/H%u/SYS/Status", serialNumber);
		esp_mqtt_client_publish(client, baseTopic, "1", 0, 0, 1);
		free(baseTopic);

		// connected to broker inform the user with LED indication
		gpio_set_level(GPO_PIN, 1);
		break;

	case MQTT_EVENT_DISCONNECTED:
		ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
		gpio_set_level(GPO_PIN, 1);
		mqttConnected = false;
		break;

	case MQTT_EVENT_SUBSCRIBED:
		ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
//	  msg_id = esp_mqtt_client_publish (client, "/topic/rc", "data", 0, 0, 0);
//	  ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
		break;

	case MQTT_EVENT_UNSUBSCRIBED:
		ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
		break;

	case MQTT_EVENT_PUBLISHED:
		ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
		break;

	case MQTT_EVENT_DATA:
		ESP_LOGI(TAG, "MQTT_EVENT_DATA");
//	  printf ("TOPIC=%.*s\r\n", event->topic_len, event->topic);
//	  printf ("DATA=%.*s\r\n", event->data_len, event->data);
//	  printf ("size %d\r\n", event->data_len);

		char topic_str[50];
		memset(topic_str, 0, sizeof(topic_str));
		strncpy(topic_str, (const char*) (event->topic), event->topic_len);

		char payload_str[20];
		memset(payload_str, 0, sizeof(payload_str));
		strncpy(payload_str, (const char*) (event->data), event->data_len);

//	  if(strcmp(topic_str, "/topic/rc")){
//		if (strcmp(payload_str, "On") == 0){
//		  gpio_set_level(REMOTE_DO, 1);
//		}
//		if (strcmp(payload_str, "Off") == 0){
//		  gpio_set_level(REMOTE_DO, 0);
//		}
//		if(strcmp(payload_str, "Toggle") == 0){
//		  gpio_set_level(REMOTE_DO, 1);
//		  vTaskDelay(3000/portTICK_PERIOD_MS); //2000 ms
//		  gpio_set_level(REMOTE_DO, 0);
//		}
//	  }

		break;
	case MQTT_EVENT_ERROR:
		ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
		if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
			log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
			log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
			log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
			ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

		}
		break;
	default:
		ESP_LOGI(TAG, "Other event id:%d", event->event_id);
		break;
	}
}

static void mqtt_init()
{

	char tempclient[20];
	sprintf(tempclient, "Hub%u", serialNumber);
	const char *clientID = &tempclient;
	esp_err_t err = ESP_OK;
#ifndef CONFIG_BROKER_AS_IP
	ESP_LOGI(TAG, "Configuring client");
	mqtt_cfg.broker.address.uri 	 				= CONFIG_MQTT_BROKER_URL;
	mqtt_cfg.broker.address.port	 				= CONFIG_MQTT_PORT;

	mqtt_cfg.credentials.username  					= CONFIG_MQTT_USER;
	mqtt_cfg.credentials.authentication.password  	= CONFIG_MQTT_PASS;
	mqtt_cfg.credentials.client_id 					= clientID;
	mqtt_cfg.credentials.set_null_client_id = true;


#else
	ESP_LOGI(TAG, "Configuring client");

	mqtt_cfg.broker.address.hostname = CONFIG_MQTT_BROKER_IP;
	mqtt_cfg.broker.address.port = CONFIG_MQTT_PORT;
	mqtt_cfg.broker.address.transport = MQTT_TRANSPORT_OVER_TCP;
	mqtt_cfg.credentials.username = CONFIG_MQTT_USER;
	mqtt_cfg.credentials.authentication.password = CONFIG_MQTT_PASS;
	mqtt_cfg.credentials.client_id = clientID;//CONFIG_MQTT_CLIENT_ID;
	mqtt_cfg.credentials.set_null_client_id = true;


#endif
	char *baseTopic = (char*) malloc(40);
	char lastWillMsg[] = "0";

	sprintf(baseTopic, "beem/H%u/SYS/Status", serialNumber);

	mqtt_cfg.session.last_will.topic = baseTopic;
	mqtt_cfg.session.last_will.msg = lastWillMsg;
//	mqtt_cfg.session.last_will.msg_len 	= sizeof(lastWillMsg);
	mqtt_cfg.session.last_will.qos = 0;
	mqtt_cfg.session.last_will.retain = 1;
	mqtt_cfg.session.keepalive = 120;

	client = esp_mqtt_client_init(&mqtt_cfg);
	/* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
	esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler,
	NULL);
	ESP_LOGI(TAG, "Starting client");
	err = esp_mqtt_client_start(client);

	free(baseTopic);

	if (err == ESP_OK) {
		client_initialized = true;
	}

	ESP_LOGI(TAG, "Client created => host %s, port : %ld, ID: %s", mqtt_cfg.broker.address.hostname,
			mqtt_cfg.broker.address.port, mqtt_cfg.credentials.client_id);
}

static void mqtt_task(void *arg)
{

	mqtt_init();
	f_newValues = false;
	uint16_t lastID = 0;
	int64_t lastTime = 0;
	int64_t lastTime2 = 0;
	memset(&uptime, 0, sizeof(uptime));

	while (1) {

		if (xTaskGetTickCount() - uptime.elapsedTime > 1000) {

			uptime.elapsedTime = xTaskGetTickCount();
			uptime.seconds++;
			uptime.inSeconds++;
			if (uptime.seconds == 60) {
				uptime.minutes++;
				uptime.seconds = 0;
				if (uptime.minutes == 60) {
					uptime.hours++;
					uptime.minutes = 0;
					if (uptime.hours == 24) {
						uptime.days++;
						uptime.hours = 0;
					}
				}
			}
		}

		if (g_sta_connected && !mqttConnected) {
			vTaskDelay(pdMS_TO_TICKS(5000));
			if (!mqttConnected && client_initialized) {
				// loss of internet connection try to reconnect
				if (esp_mqtt_client_reconnect(client) == ESP_OK) {
					ESP_LOGI(TAG, "Client reconnected to broker");
				}
				ESP_LOGI(TAG, "trying to reconnect to broker");
			}
		}
		if (!g_sta_connected) {
			closeClientConnection = true;
			esp_mqtt_client_disconnect(client);
			ESP_LOGI(TAG, "deleting own task");
			vTaskDelay(pdMS_TO_TICKS(1000));
			ESP_LOGI(TAG, "Restarting device");
			esp_restart();
			vTaskDelete(NULL);
		}

		if ((xTaskGetTickCount() - lastTime) > 300000) {
			lastTime = xTaskGetTickCount();
			lastID = 0;
		}

		if (mqttConnected) {
			// send Hub data and Hub sensors
			// todo: make these countings with timers in place of Gettickcount
			if ((xTaskGetTickCount() - lastTime2) > 300000) {
				lastTime2 = xTaskGetTickCount();

				char payload[255];
				memset(&payload, 0, sizeof(255));
				char baseTopic[20];
				char tempTopic[80];
				memset(&baseTopic, 0, sizeof(20));
				memset(&tempTopic, 0, sizeof(80));

				sprintf(baseTopic, "beem/H%u/SYS", serialNumber);

				sprintf(payload, "{\"RH\" : %.2f, \"temp\" : %.2f,\"uptimeSec\" : %llu, \"RSSI\" : %i }",
						humidity, temperature, uptime.inSeconds, wifi_app_get_rssi()
						);

				esp_mqtt_client_publish(client, baseTopic, payload, 0, 0, 0);
				ESP_LOGI(TAG, "%s/%s", tempTopic, payload);


				char str_end[20];
				char str_data[20];
				memset (str_end, 0, sizeof(str_end));
				memset (str_data, 0, sizeof(str_data));

				sprintf(str_end, "/uptime");
				strcpy(tempTopic, baseTopic);
				strcat(tempTopic, str_end);
				sprintf(str_data, "%ud:%uh:%um:%us", uptime.days, uptime.hours, uptime.minutes, uptime.seconds);
				esp_mqtt_client_publish(client, tempTopic, str_data, 0, 0, 0);
				ESP_LOGI(TAG, "%s/%s", tempTopic, str_data);
				memset(str_end, 0, sizeof(str_end));
				memset(str_data, 0, sizeof(str_data));
				memset(tempTopic, 0, sizeof(80));


				sprintf(str_end, "/SSID");
				strcpy(tempTopic, baseTopic);
				strcat(tempTopic, str_end);
				sprintf(str_data, "%s", wifi_app_get_ssid());
				esp_mqtt_client_publish(client, tempTopic, str_data, 0, 0, 0);
				ESP_LOGI(TAG, "%s/%s", tempTopic, str_data);
				memset(str_end, 0, sizeof(str_end));
				memset(str_data, 0, sizeof(str_data));
				memset(tempTopic, 0, sizeof(80));


				esp_reset_reason_t lastReason = esp_reset_reason();
				char reason[20];
				switch (lastReason) {
				  case ESP_RST_UNKNOWN:
					sprintf(reason,"ESP_RST_UNKNOWN");
					break;
				  case ESP_RST_POWERON:
					sprintf(reason,"ESP_RST_POWERON");
					break;
				  case ESP_RST_EXT:
					sprintf(reason,"ESP_RST_EXT");
					break;
				  case ESP_RST_SW:
					sprintf(reason,"ESP_RST_SW");
					break;
				  case ESP_RST_PANIC:
					sprintf(reason,"ESP_RST_PANIC");
					break;
				  case ESP_RST_INT_WDT:
					sprintf(reason,"ESP_RST_INT_WDT");
					break;
				  case ESP_RST_TASK_WDT:
					sprintf(reason,"ESP_RST_TASK_WDT");
					break;
				  case ESP_RST_WDT:
					sprintf(reason,"ESP_RST_WDT");
					break;
				  case ESP_RST_DEEPSLEEP:
					sprintf(reason,"ESP_RST_DEEPSLEEP");
					break;
				  case ESP_RST_BROWNOUT:
					sprintf(reason,"ESP_RST_BROWNOUT");
					break;
				  case ESP_RST_SDIO:
					sprintf(reason,"ESP_RST_SDIO");
					break;
				  default:

					break;
				}
				sprintf(str_end, "/LastResetReason");
				strcpy(tempTopic, baseTopic);
				strcat(tempTopic, str_end);
				sprintf(str_data, "%s", reason);
				esp_mqtt_client_publish(client, tempTopic, str_data, 0, 0, 0);
				ESP_LOGI(TAG, "%s/%s", tempTopic, str_data);
				memset(str_end, 0, sizeof(str_end));
				memset(str_data, 0, sizeof(str_data));
				memset(tempTopic, 0, sizeof(80));

			}

			if (f_newValues && (lastID != ed_id)) {
				// todo: use Queue instead of mutex
				if (uartMutex != NULL) {
					if (xSemaphoreTake(uartMutex, pdMS_TO_TICKS(20)) == pdTRUE) {
						lastID = ed_id;
						f_newValues = false;
						ESP_LOGI(TAG, "newValues to publish");

						char payload[255];
						memset (&payload, 0, sizeof(255));
						char baseTopic[25];
						char tempTopic[80];
						memset(&baseTopic, 0, sizeof(20));
						memset(&tempTopic, 0, sizeof(80));

//						sprintf(baseTopic, "beem/%u/", ed_id);
//						char str_end[20];
//						char str_data[20];
//						memset(str_end, 0, sizeof(str_end));
//						memset(str_data, 0, sizeof(str_data));

						switch (ed_type) {
						case 1:
						  sprintf (baseTopic, "beem/H%u/edge/E%u", serialNumber, ed_id);
						  sprintf (payload, "{\"batt\" : %u, \"battAlm\" : %u,\"RH\" : %.2f, \"temp\" : %.2f , \"weight\" : %.2f, \"type\" : %u}",
								   ed_battery, ed_battery_alm, ed_hum, ed_temp, ed_weight, ed_type);
						  esp_mqtt_client_publish (client, baseTopic, payload, 0, 0, 0);
						  ESP_LOGI(TAG, "%s/%s", tempTopic, payload);


//							sprintf(str_end, "batt");
//							strcpy(tempTopic, baseTopic);
//							strcat(tempTopic, str_end);
//							sprintf(str_data, "%u", ed_battery);
//							esp_mqtt_client_publish(client, tempTopic, str_data, 0, 0, 0);
//							ESP_LOGI(TAG, "%s/%s", tempTopic, str_data);
//							memset(str_end, 0, sizeof(str_end));
//							memset(str_data, 0, sizeof(str_data));
//							memset(tempTopic, 0, sizeof(80));
//
//							sprintf(str_end, "battAlm");
//							strcpy(tempTopic, baseTopic);
//							strcat(tempTopic, str_end);
//							sprintf(str_data, "%u", ed_battery_alm);
//							esp_mqtt_client_publish(client, tempTopic, str_data, 0, 0, 0);
//							ESP_LOGI(TAG, "%s/%s", tempTopic, str_data);
//							memset(str_end, 0, sizeof(str_end));
//							memset(str_data, 0, sizeof(str_data));
//							memset(tempTopic, 0, sizeof(80));

//							sprintf(str_end, "RH");
//							strcpy(tempTopic, baseTopic);
//							strcat(tempTopic, str_end);
//							sprintf(str_data, "%.2f", ed_hum);
//							esp_mqtt_client_publish(client, tempTopic, str_data, 0, 0, 0);
//							ESP_LOGI(TAG, "%s/%s", tempTopic, str_data);
//							memset(str_end, 0, sizeof(str_end));
//							memset(str_data, 0, sizeof(str_data));
//							memset(tempTopic, 0, sizeof(80));
//
//							sprintf(str_end, "temp");
//							strcpy(tempTopic, baseTopic);
//							strcat(tempTopic, str_end);
//							sprintf(str_data, "%.2f", ed_temp);
//							esp_mqtt_client_publish(client, tempTopic, str_data, 0, 0, 0);
//							ESP_LOGI(TAG, "%s/%s", tempTopic, str_data);
//							memset(str_end, 0, sizeof(str_end));
//							memset(str_data, 0, sizeof(str_data));
//							memset(tempTopic, 0, sizeof(80));

//							sprintf(str_end, "weight");
//							strcpy(tempTopic, baseTopic);
//							strcat(tempTopic, str_end);
//							sprintf(str_data, "%.2f", ed_weight);
//							esp_mqtt_client_publish(client, tempTopic, str_data, 0, 0, 0);
//							ESP_LOGI(TAG, "%s/%s", tempTopic, str_data);
//							memset(str_end, 0, sizeof(str_end));
//							memset(str_data, 0, sizeof(str_data));
//							memset(tempTopic, 0, sizeof(80));
//
//							sprintf(str_end, "type");
//							strcpy(tempTopic, baseTopic);
//							strcat(tempTopic, str_end);
//							sprintf(str_data, "%u", ed_type);
//							esp_mqtt_client_publish(client, tempTopic, str_data, 0, 0, 0);
//							ESP_LOGI(TAG, "%s/%s", tempTopic, str_data);
//							memset(str_end, 0, sizeof(str_end));
//							memset(str_data, 0, sizeof(str_data));
//							memset(tempTopic, 0, sizeof(80));

							break;
						default:
						  break;
						}

						xSemaphoreGive(uartMutex);
					}
				}
			}

		}

		vTaskDelay(pdMS_TO_TICKS(10));
	}
}

void mqtt_app_start(void)
{

	xTaskCreatePinnedToCore(mqtt_task, "mqtt_task", MQTT_TASK_STACK_SIZE, NULL,
	MQTT_TASK_PRIORITY, NULL,
	MQTT_TASK_CORE_ID);
}

