/*
 * board_config.h
 *
 *	- Defines board pinouts and
 *	- Includes the common header files
 *
 *  Created on: Apr 5, 2023
 *      Author: kolev
 */

#ifndef MAIN_BOARD_CONFIG_H_
#define MAIN_BOARD_CONFIG_H_
/**
 * INCLUDES
 */
// GENARAL INCLUDES
#include <stdio.h>
#include <string.h>
// FREERTOS
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// ESP IDF SPECIFIC
#include "esp_err.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"

#include "tasks_common.h"

/**
 * GPIOs DEFINES
 */

//#define TXD2_PIN			GPIO_NUM_17		// UART							ar[oe=0, ie=1]
//#define RXD2_PIN 			GPIO_NUM_16		// UART							ar[oe=0, ie=1]
//#define RF_SET				GPIO_NUM_25		// HC-12 set pin 				ar[oe=0, ie=1]  br[oe=0, ie=0]
//#define SW_I2C				GPIO_NUM_4		// I2C bus devices power supply	ar[oe=0, ie=0]
//#define I2C_SCL_PIN			GPIO_NUM_22		// I2C clock					ar[oe=0, ie=1]
//#define I2C_SDA_PIN			GPIO_NUM_21		// I2C data						ar[oe=0, ie=1]
//#define GPI_PIN				GPIO_NUM_34		// general purpose input		ar[oe=0, ie=1, wpu]
//#define GPO_PIN				GPIO_NUM_4		// general purpose output		LED status
#define GPO_PIN				GPIO_NUM_2		// blue Led on DevKit
#define ALARM_GPO				GPIO_NUM_4		//

#endif /* MAIN_BOARD_CONFIG_H_ */
