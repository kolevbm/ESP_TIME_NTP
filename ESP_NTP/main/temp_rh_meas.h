/*
 * temp_rh_meas.h
 *
 *  Created on: Apr 5, 2023
 *      Author: kolev
 */

#ifndef MAIN_TEMP_RH_MEAS_H_
#define MAIN_TEMP_RH_MEAS_H_

#define I2C_MASTER_SCL_IO           I2C_SCL_PIN      /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           I2C_SDA_PIN      /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              0                          /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          100000                     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       500
#define I2C_ACK	1
#define I2C_NACK 0

#define AM2320_SENSOR_ADDR                 0x5C        /*!< Slave address of the MPU9250 sensor */
#define AM2320_WHO_AM_I_REG_ADDR           0x00        /*!< Register addresses of the "who am I" register */


// initialize power pin
void temp_rh_meas_power_init();
// turn on power supply
void temp_rh_meas_power_ON();
// turn off power supply
void temp_rh_meas_power_OFF();

void temp_rh_meas_app_start(void);

#endif /* MAIN_TEMP_RH_MEAS_H_ */
