/*
 * app_nvs.h
 *
 *  Created on: Dec 22, 2022
 *      Author: kolev
 */

#ifndef MAIN_APP_NVS_H_
#define MAIN_APP_NVS_H_


/**
 * Saves station mode Wifi credentials to NVS
 * @return ESP_OK if successful.
 */
esp_err_t app_nvs_save_sta_creds(void);

/**
 * Loads the previously saved credentials from NVS.
 * @return true if previously saved credentials were found.
 */
bool app_nvs_load_sta_creds(void);

/**
 * Clears station mode credentials from NVS
 * @return ESP_OK if successful.
 */
esp_err_t app_nvs_clear_sta_creds(void);

/***
 * Saves the SN of the Hub to NVS
 * @return ESP_OK if successful
 */
esp_err_t app_nvs_save_serial_number(void);
/**
 * Loads the previously saved serial number.
 * @return true if previously saved credentials were found.
 */
uint16_t app_nvs_load_serial_number(void);

#endif /* MAIN_APP_NVS_H_ */
