/*
 * rgb_led.h
 *
 *  Created on: Dec 12, 2022
 *      Author: kolev
 */

#ifndef MAIN_RGB_LED_H_
#define MAIN_RGB_LED_H_


typedef enum led_woking_mode
{
  LED_WORK_STATE_FAST_BLINK = 0,
  LED_WORK_STATE_SLOW_BLINK,
  LED_WORK_STATE_DARK,
  LED_WORK_STATE_LIGHT,
  LED_WORK_STATE_OTA_UPDATE
} led_woking_mode_e;

 void set_led_work_state(uint8_t workState);
 /*
  * RGB LED task start
  */
 void rgb_led_app_start(void);
#endif /* MAIN_RGB_LED_H_ */
