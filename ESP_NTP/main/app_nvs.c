/*
 * app_nvs.c
 *
 *  Created on: Dec 22, 2022
 *      Author: kolev
 */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "nvs_flash.h"

#include "app_nvs.h"
#include "wifi_app.h"

// EXTERN variables
extern char alarmJSON;


// Tag for logging to the monitor
static const char TAG[] = "nvs";

// NVS name space used for station mode credentials
const char app_nvs_sta_creds_namespace[] = "stacreds";
const char app_nvs_serial_namespace[] = "serial";
const char app_nvs_alarms_namespace[] = "alarms";
/**
 * Saves station mode Wifi credentials to NVS
 * @return ESP_OK if successful.
 */
esp_err_t app_nvs_save_sta_creds (void)
{
	nvs_handle handle;
	esp_err_t esp_err;
	ESP_LOGI(TAG, "app_nvs_save_sta_creds: Saving station mode credentials to flash");

	wifi_config_t *wifi_sta_config = wifi_app_get_wifi_config();
	if (wifi_sta_config) {
		esp_err = nvs_open(app_nvs_sta_creds_namespace, NVS_READWRITE, &handle);
		if (esp_err != ESP_OK) {
			printf("app_nvs_save_sta_creds: Error (%s) opening NVS handle!\n", esp_err_to_name(esp_err));
			return esp_err;
		}

		// Set SSID
		esp_err = nvs_set_blob(handle, "ssid", wifi_sta_config->sta.ssid,
		MAX_SSID_LENGTH);
		if (esp_err != ESP_OK) {
			printf("app_nvs_save_sta_creds: Error (%s) setting SSID to NVS!\n", esp_err_to_name(esp_err));
			return esp_err;
		}

		// Set Password
		esp_err = nvs_set_blob(handle, "password", wifi_sta_config->sta.password,
		MAX_PASSWORD_LENGTH);
		if (esp_err != ESP_OK) {
			printf("app_nvs_save_sta_creds: Error (%s) setting Password to NVS!\n", esp_err_to_name(esp_err));
			return esp_err;
		}

		// Commit credentials to NVS
		esp_err = nvs_commit(handle);
		if (esp_err != ESP_OK) {
			printf("app_nvs_save_sta_creds: Error (%s) comitting credentials to NVS!\n", esp_err_to_name(esp_err));
			return esp_err;
		}
		nvs_close(handle);
		ESP_LOGI(TAG, "app_nvs_save_sta_creds: wrote wifi_sta_config: Station SSID: %s Password: %s",
				wifi_sta_config->sta.ssid, wifi_sta_config->sta.password);
	}

	printf("app_nvs_save_sta_creds: returned ESP_OK\n");
	return ESP_OK;
}

/**
 * Loads the previously saved credentials from NVS.
 * @return true if previously saved credentials were found.
 */
bool app_nvs_load_sta_creds (void)
{
  nvs_handle handle;
  esp_err_t esp_err;

  ESP_LOGI(TAG, "app_nvs_load_sta_creds: Loading Wifi credentials from flash");

	if (nvs_open(app_nvs_sta_creds_namespace, NVS_READONLY, &handle) == ESP_OK) {

		wifi_config_t *wifi_sta_config = wifi_app_get_wifi_config();

		if (wifi_sta_config == NULL) {
			wifi_sta_config = (wifi_config_t*) malloc(sizeof(wifi_config_t));
		}
		memset(wifi_sta_config, 0x00, sizeof(wifi_config_t));

		// Allocate buffer
		size_t wifi_config_size = sizeof(wifi_config_t);
		uint8_t *wifi_config_buff = (uint8_t*) malloc(sizeof(uint8_t) * wifi_config_size); ///> todo check Ensures our buffer is large enough
		memset(wifi_config_buff, 0x00, sizeof(wifi_config_size));

		// Load SSID
		wifi_config_size = sizeof(wifi_sta_config->sta.ssid);
		esp_err = nvs_get_blob(handle, "ssid", wifi_config_buff, &wifi_config_size);		///> todo check
		if (esp_err != ESP_OK) {
			free(wifi_config_buff);
			printf("app_nvs_load_sta_creds: (%s) no station SSID found in NVS\n", esp_err_to_name(esp_err));
			return false;
		}
		memcpy(wifi_sta_config->sta.ssid, wifi_config_buff, wifi_config_size);

		// Load Password
		wifi_config_size = sizeof(wifi_sta_config->sta.password);
		esp_err = nvs_get_blob(handle, "password", wifi_config_buff, &wifi_config_size);
		if (esp_err != ESP_OK) {
			free(wifi_config_buff);
			printf("app_nvs_load_sta_creds: (%s) retrieving password!\n", esp_err_to_name(esp_err));
			return false;
		}
		memcpy(wifi_sta_config->sta.password, wifi_config_buff, wifi_config_size);

		free(wifi_config_buff);
		nvs_close(handle);

		printf("app_nvs_load_sta_creds: SSID: %s Password: %s\n", wifi_sta_config->sta.ssid, wifi_sta_config->sta.password);
		return wifi_sta_config->sta.ssid[0] != '\0';
	}
	else {
		return false;
	}

}

/**
 * Clears station mode credentials from NVS
 * @return ESP_OK if successful.
 */
esp_err_t app_nvs_clear_sta_creds (void)
{
  nvs_handle handle;
  esp_err_t esp_err;
  ESP_LOGI(TAG, "app_nvs_clear_sta_creds: Clearing Wifi station mode credentials from flash");

  esp_err = nvs_open (app_nvs_sta_creds_namespace, NVS_READWRITE, &handle);
  if (esp_err != ESP_OK)
  {
	printf ("app_nvs_clear_sta_creds: Error (%s) opening NVS handle!\n", esp_err_to_name (esp_err));
	return esp_err;
  }

  // Erase credentials
  esp_err = nvs_erase_all (handle);
  if (esp_err != ESP_OK)
  {
	printf ("app_nvs_clear_sta_creds: Error (%s) erasing station mode credentials!\n", esp_err_to_name (esp_err));
	return esp_err;
  }

  // Commit clearing credentials from NVS
  esp_err = nvs_commit (handle);
  if (esp_err != ESP_OK)
  {
	printf ("app_nvs_clear_sta_creds: Error (%s) NVS commit!\n", esp_err_to_name (esp_err));
	return esp_err;
  }
  nvs_close (handle);

  printf ("app_nvs_clear_sta_creds: returned ESP_OK\n");
  return ESP_OK;
}

/***
 * Saves the SN of the Hub to NVS
 * @return ESP_OK if successful
 */
esp_err_t app_nvs_save_serial_number (void)
{
  nvs_handle handle;
  esp_err_t err;
  ESP_LOGI(TAG, "Saving serial number to flash");

  err = nvs_open (app_nvs_serial_namespace, NVS_READWRITE, &handle);
  uint16_t serialNumber = 0;

  // Erase credentials
  err = nvs_erase_all (handle);
  if (err != ESP_OK)
  {
	ESP_LOGI(TAG, "app_nvs_save_serial_number: Error (%s) erasing Serial number!", esp_err_to_name (err));
  }
  // prepare the Serial Number from the Kconfig file
  serialNumber = CONFIG_HUB_SN;
  err = nvs_set_u16 (handle, "serial", serialNumber);

  // Commit data to NVS
  err = nvs_commit (handle);
  if (err != ESP_OK)
  {
	ESP_LOGI(TAG, "app_nvs_save_serial_number: Error (%s) committing Serial Number to NVS!", esp_err_to_name (err));
	return err;
  }
  else{
	ESP_LOGI(TAG, "Serial number saved: %u", serialNumber);
  }
  // Close the NVS handle
  nvs_close (handle);

  return ESP_OK;
}
/**
 * Loads the previously saved serial number.
 * @return 0 if not initialized else Serial Number
 */
uint16_t app_nvs_load_serial_number (void)
{
  nvs_handle handle;
  esp_err_t err;
  // Open NVS
  err = nvs_open (app_nvs_serial_namespace, NVS_READWRITE, &handle);
  uint16_t serialNumber = 0;

  err = nvs_get_u16 (handle, "serial", &serialNumber);
  switch (err) {
	case ESP_OK:
	  ESP_LOGI(TAG, "Serial number of device: H%u", serialNumber);
	  break;
	case ESP_ERR_NVS_NOT_FOUND:
	  ESP_LOGI(TAG, "Serial number is not initialized yet");
	  break;
	default:
	  ESP_LOGI(TAG, "Error (%s) reading!\n", esp_err_to_name (err));
  }
  // Close the NVS handle
  nvs_close (handle);
  return serialNumber;
}


/*
 * Save the Alarms to the NVS
 * @return ESP_OK
 */
esp_err_t app_nvs_save_alarms(void){

  nvs_handle handle;
  esp_err_t err;
  ESP_LOGI(TAG, "Saving Alarms to flash");

  err = nvs_open (app_nvs_alarms_namespace, NVS_READWRITE, &handle);

  // Erase previous stored values in the namespace
  err = nvs_erase_all (handle);
  if (err != ESP_OK)
  {
	ESP_LOGI(TAG, "app_nvs_save_alarms: Error (%s) erasing Alarms namespace!", esp_err_to_name (err));
  }

  // todo: prepare the Alarms json to string or something like this

  err = nvs_set_str (handle, "Alarms", &alarmJSON);

  // Commit data to NVS
  err = nvs_commit (handle);
  if (err != ESP_OK)
  {
	ESP_LOGI(TAG, "app_nvs_save_serial_number: Error (%s) committing Serial Number to NVS!", esp_err_to_name (err));
	return err;
  }
  else{
	ESP_LOGI(TAG, "Writing of Alarms OK");
  }
  // Close the NVS handle
  nvs_close (handle);

  return ESP_OK;

}

/**
 * Loads the previously saved Alarms.
 * @return esp error type
 */
esp_err_t app_nvs_load_alarms (void)
{
  nvs_handle handle;
  esp_err_t err;
  // Open NVS
  err = nvs_open (app_nvs_alarms_namespace, NVS_READWRITE, &handle);
  ESP_LOGI(TAG, "nvs opened, now reading json");

  // first get the size
  size_t required_size;
  err = nvs_get_str (handle, "Alarms", NULL, &required_size);
  // second allocate memory for the string
  char* str_value = malloc (required_size);
  err = nvs_get_str(handle, "Alarms", str_value, &required_size);

  switch (err) {
	case ESP_OK:
	  ESP_LOGI(TAG, "Alarms: %s", str_value);
	  // after successful reading of NVS copy the json string in the extern variable
	  strcpy(&alarmJSON, str_value);
	  break;
	case ESP_ERR_NVS_NOT_FOUND:
	  ESP_LOGI(TAG, "Alarms not initialized yet");
	  break;
	default:
	  ESP_LOGI(TAG, "Error (%s) reading!\n", esp_err_to_name (err));
  }
  // free the allocated memory
  free(str_value);

  // Close the NVS handle
  nvs_close (handle);
  return err;
}

