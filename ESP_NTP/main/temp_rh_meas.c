/*
 * temp_rh_meas.c
 *
 *  Created on: Apr 5, 2023
 *      Author: kolev
 */

#include "board_config.h"
#include "temp_rh_meas.h"
#include "driver/i2c.h"

// EXTERN VARIALBES

// GLOBAL VARIABLES
float temperature = 0;
float humidity = 0;
uint8_t hum_raw;
uint8_t temp_raw;
static const char TAG[] = "[Temp_RH]";


/**
 * i2c master initialization
 */
static esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(i2c_master_port, &conf);
    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}
/**
 * send a wake routine to AM2320 sensor
 */
static esp_err_t am2320_wake(){
	esp_err_t err ;
	i2c_cmd_handle_t cmd  =  i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (AM2320_SENSOR_ADDR << 1) | I2C_MASTER_WRITE  , I2C_NACK);
	i2c_master_stop(cmd);
	err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
	if(err != ESP_OK){
		ESP_LOGI(TAG,"wake err: %s", esp_err_to_name(err));
	}
	i2c_cmd_link_delete(cmd);

	return err;
}

/**
 * initialize power pin
 */
void temp_rh_meas_power_init(){
	esp_rom_gpio_pad_select_gpio(SW_I2C);
	gpio_set_direction(SW_I2C, GPIO_MODE_OUTPUT);
}
/**
 * turn on power supply for the sensor
 */
void temp_rh_meas_power_ON(){
	gpio_set_level(SW_I2C, 1);
	vTaskDelay(10 / portTICK_PERIOD_MS); //wait for 10ms after voltage powering on for stable work
}
/**
 * turn off power supply
 */
void temp_rh_meas_power_OFF(){
	gpio_set_level(SW_I2C, 0);
}

/**
 * Local RTOS task
 */
static void temp_rh_meas_task(void *arg)
{
	int64_t lastTime = 0;
	// initialize i2c
	esp_err_t err = i2c_master_init();
	if(err == ESP_OK){
		ESP_LOGI(TAG, "I2C initialized successfully");
	}
	else{
		ESP_LOGI(TAG, "error -> %s",esp_err_to_name(err));
	}
	// begin procedure
	uint8_t writeBuff[] = {0x03, 0x0, 0x04};	// function 0x03, starting address 0x0, bytes 0x04
	uint8_t readBuff[8];
//	temp_rh_meas_power_ON();
	while(1){
	vTaskDelay (pdMS_TO_TICKS(50));
	if ((xTaskGetTickCount () - lastTime) > 10000)
	{
	  temp_rh_meas_power_ON();
	  lastTime = xTaskGetTickCount ();
	  for (int a = 0; a < 3; a++)
	  {
		memset (readBuff, 0, sizeof(readBuff[0]) * 8);  //clear the array
		err = am2320_wake ();
		err = i2c_master_write_to_device (I2C_MASTER_NUM, 0x5C, writeBuff, sizeof(writeBuff),
										  I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
		esp_rom_delay_us (2000); //delays in busy-wait manner a.k.a. locks the MCU for specified microseconds
		err = i2c_master_read_from_device (I2C_MASTER_NUM, 0x5C, readBuff, sizeof(readBuff),
										   I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
		vTaskDelay (100 / portTICK_PERIOD_MS);
	  }
	  uint16_t crcVal = readBuff[7] << 8;  // note: ! low byte is sent first !
	  crcVal += readBuff[6];

	  uint16_t crc16_calc = CRC16 (readBuff, 6);

	  uint8_t crcStatus = (crc16_calc == crcVal) ? 1 : 0;

	  if (crcStatus)
	  {
		ESP_LOGI(TAG, "CRC OK");
		float hum, temp;
		hum = ((readBuff[2] << 8) + readBuff[3]) / 10.0;
		temp = (((readBuff[4] & 0x7F) << 8) + readBuff[5]) / 10.0;
		temp = (readBuff[4] & 0x80) ? -temp : temp;	//check sign
		if (hum < 110 && temp < 100)
		{
		  ESP_LOGI(TAG, "RH = %.2f  T = %.2f", humidity, temperature);
		  humidity = hum;
		  temperature = temp;
		}

	  }

	  else
		ESP_LOGI(TAG, "CRC NOT OK");

	  if (err != ESP_OK)
	  {
		ESP_LOGI(TAG, "write_read error -> %s", esp_err_to_name (err));
	  }

	  temp_rh_meas_power_OFF();
	}

  }

//	temp_rh_meas_power_OFF();

//	i2c_driver_delete(I2C_MASTER_NUM);
//	ESP_LOGI(TAG,"Deleting own task");
//	vTaskDelete(NULL);
}
/**
 * RTOS task run
 */
void temp_rh_meas_app_start(void)
{

	temp_rh_meas_power_init();

	xTaskCreatePinnedToCore(temp_rh_meas_task, "temp_rh_meas_task",
    										TEMP_RH_MEAS_TASK_STACK_SIZE, NULL,
    										TEMP_RH_MEAS_TASK_PRIORITY, NULL,
											TEMP_RH_MEAS_TASK_CORE_ID);
}

