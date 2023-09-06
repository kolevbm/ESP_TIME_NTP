/*
 * rgb_led.c
 *
 *  Created on: Dec 12, 2022
 *      Author: kolev
 */

#include <stdbool.h>

#include "driver/ledc.h"
#include "rgb_led.h"
#include "board_config.h"
#include "tasks_common.h"

uint8_t led_work_state = 0;

static void configure_led(void)
{
    esp_rom_gpio_pad_select_gpio(GPO_PIN);
  	gpio_set_direction(GPO_PIN, GPIO_MODE_OUTPUT);

//    gpio_reset_pin(GPO_PIN);
//    gpio_set_direction(GPO_PIN, GPIO_MODE_OUTPUT);
}

void set_led_work_state(uint8_t workState){
  led_work_state = workState;
}
static void rgb_led_task(void *arg){
  bool state = true;
  while (1)
  {
	// todo: instead of delays use periodic timer and change only time period
	switch (led_work_state) {
	  case LED_WORK_STATE_DARK:
		gpio_set_level (GPO_PIN, 0);
		break;
	  case LED_WORK_STATE_LIGHT:
		gpio_set_level (GPO_PIN, 1);
		break;
	  case LED_WORK_STATE_FAST_BLINK:
		gpio_set_level (GPO_PIN, state);
		state = !state;
		vTaskDelay (pdMS_TO_TICKS(250));
		break;
	  case LED_WORK_STATE_SLOW_BLINK:
		gpio_set_level (GPO_PIN, state);
		state = !state;
		vTaskDelay (pdMS_TO_TICKS(2000));
		break;
	  case LED_WORK_STATE_OTA_UPDATE:
		gpio_set_level (GPO_PIN, state);
		state = !state;
		vTaskDelay (pdMS_TO_TICKS(100));
		gpio_set_level (GPO_PIN, state);
		state = !state;
		vTaskDelay (pdMS_TO_TICKS(100));
		gpio_set_level (GPO_PIN, state);
		state = !state;
		vTaskDelay (pdMS_TO_TICKS(100));
		gpio_set_level (GPO_PIN, state);
		state = !state;
		vTaskDelay (pdMS_TO_TICKS(100));
		gpio_set_level (GPO_PIN, state);
		state = !state;
		vTaskDelay (pdMS_TO_TICKS(1000));
		break;
	  default:
		gpio_set_level (GPO_PIN, 0);
		break;
	}
	vTaskDelay (10);
  }
}

/**
 * RTOS task run
 */
void rgb_led_app_start(void)
{
	configure_led();
	xTaskCreatePinnedToCore(rgb_led_task, "rgb_led_task",
							RGB_LED_TASK_STACK_SIZE, NULL,
							RGB_LED_TASK_PRIORITY, NULL,
							RGB_LED_TASK_CORE_ID);
}
