/*
 * mqtt_bk.h
 *
 *  Created on: Jan 17, 2023
 *      Author: kolev
 */

#ifndef MAIN_MQTT_BK_H_
#define MAIN_MQTT_BK_H_

//#define CONFIG_BROKER_URL 		"84.43.202.81"
//#define conf_MQTT_SERVER_IP      84, 43, 202, 81
//// #define conf_MQTT_SERVER        "service.grid-one.eu"
//#define conf_MQTT_PORT          16649
//#define conf_MQTT_CLIENT_ID     "P0001H01"
//#define conf_MQTT_SUB_TOPIC     "0001/S/001/#"
//#define conf_MQTT_USER          "vb"
//#define conf_MQTT_PASS          "PAss1020"

#define REMOTE_DO 				GPIO_NUM_26
#define LIGHT_SWITCH			GPIO_NUM_25
#define LIGHT_SWITCH_2			GPIO_NUM_27
/**
 * Starts the MQTT task.
 */
void mqtt_app_start(void);

#endif /* MAIN_MQTT_BK_H_ */
