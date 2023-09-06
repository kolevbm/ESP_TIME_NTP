//#include <stdio.h>
//#include <stdbool.h>
//#include <unistd.h>
//
//#include "freertos/FreeRTOS.h"
//#include "esp_wifi.h"
//#include "esp_system.h"
//#include "esp_event.h"
////#include "esp_event_loop.h"
//#include "nvs_flash.h"
//#include "driver/gpio.h"

/**
 * Application entry point.
 */

#include "esp_log.h"
#include "nvs_flash.h"

#include "wifi_app.h"
#include "temp_rh_meas.h"
#include "sntp_time_sync.h"
#include "wifi_reset_button.h"
#include "mqtt_bk.h"
#include "mqtt_client.h"
#include "uart/uart_bk.h"
#include "app_nvs.h"
#include "board_config.h"
#include "rgb_led.h"

static const char TAG[] = "main";
uint16_t serialNumber;
extern bool closeClientConnection;
extern esp_mqtt_client_handle_t client;



void wifi_application_connected_events(void) {
	ESP_LOGI(TAG, "WiFi Application Connected!");
	sntp_time_sync_task_start();
	set_led_work_state(LED_WORK_STATE_FAST_BLINK);
	mqtt_app_start();
}

void app_main(void) {
	ESP_LOGI(TAG, "[APP] Startup..");
	ESP_LOGI(TAG, "[APP] Free memory: %ld bytes", esp_get_free_heap_size());
	ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());
	// Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}

	ESP_ERROR_CHECK(ret); // todo: to be removed in production code

#ifdef CONFIG_COMMIT_NEW_SERIAL
  app_nvs_save_serial_number();
#endif
	serialNumber = app_nvs_load_serial_number();

	// Start Wifi
	wifi_app_start();
	rgb_led_app_start();
	set_led_work_state(LED_WORK_STATE_FAST_BLINK);
	// Config WiFi_Reset_button
//	wifi_reset_button_config();

	uart_app_start();

//	temp_rh_meas_app_start();

	// Set connected event callback
	wifi_app_set_callback(wifi_application_connected_events);

	while (1) {
		if (closeClientConnection) {
			closeClientConnection = false;
			esp_mqtt_client_stop(client);
		}
		vTaskDelay(pdMS_TO_TICKS(10));
	}

}
