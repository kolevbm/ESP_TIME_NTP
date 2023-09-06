/*
 * uart_bk.c
 *
 *  Created on: Jan 24, 2023
 *      Author: kolev
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
#include "driver/gptimer.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp32/rom/uart.h"
// LOCAL INCLUDES
#include "uart_bk.h"
#include "temp_rh_meas.h"
#include "board_config.h"




#define TIMER_DIVIDER         (16)  //  Hardware timer clock divider
//#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)/1000000  // convert counter value to microseconds
#define TIMER_SCALE           (5)  // convert counter value to microseconds

typedef struct {
    int timer_group;
    int timer_idx;
    int alarm_interval;
    bool auto_reload;
} example_timer_info_t;

typedef struct {
    example_timer_info_t info;
    uint64_t timer_counter_value;
} example_timer_event_t;

static void HC12ConfigMode();

// EXTERN VARIABLES
extern float temperature;
extern bool f_newValues;

// GLOBAL VARIABLES
static const char TAG[] = "[uart_app]";
//static const int RX_BUF_SIZE = 256;
#define RX_BUF_SIZE	256

uint8_t uartRxBuf[RX_BUF_SIZE] = "";
uint8_t uartNewFrame = 0;
uint16_t uartRxCount = 0;

SemaphoreHandle_t uartMutex = NULL;

gptimer_handle_t gptimer = NULL;
uint8_t 	ed_type = 0;
uint16_t 	ed_id = 0;
uint8_t  	ed_battery = 0;
uint8_t 	ed_battery_alm = 0;
float 		ed_weight = 0;
float 		ed_temp = 0;
float 		ed_hum = 0;


// word to bytes
volatile static union {
  struct {
      uint8_t ByteL;
      uint8_t ByteH;
  } Bytes;
  uint16_t Word;
} ConvertW2B;



static bool IRAM_ATTR timer_group_isr_callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata);
void timer_restart();
void uart_init(void);


static bool IRAM_ATTR timer_group_isr_callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata)
{
    BaseType_t high_task_awoken = pdFALSE;

    // the timer has ticked => 3.5 character time has passed => a full frame is received
    uartNewFrame = 1;   // raise flag to indicate this; note: the timer is one shot, so it will not start until this flag is cleared and a new byte is received!

    return high_task_awoken == pdTRUE; // return whether we need to yield at the end of ISR
}

void timer_restart()
{
	ESP_ERROR_CHECK(gptimer_stop(gptimer));
	ESP_ERROR_CHECK(gptimer_set_raw_count(gptimer,0));
	ESP_ERROR_CHECK(gptimer_start(gptimer));
}


void uart_init(void)
{
    const uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    // We won't use a buffer for sending data.  !!!!

    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, RX_BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_2, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, TXD2_PIN, RXD2_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
}

static void HC12ConfigMode(){
	esp_rom_gpio_pad_select_gpio(RF_SET);
	gpio_set_direction(RF_SET, GPIO_MODE_OUTPUT);
	gpio_set_level(RF_SET, 0);
}

/* activate the HC12 module by setting the SET pin HIGH */
static void HC12RunMode(){
    esp_rom_gpio_pad_select_gpio(RF_SET);
  	gpio_set_direction(RF_SET, GPIO_MODE_OUTPUT);
  	gpio_set_level(RF_SET, 1);
}

static void rx_task(void *arg)
{
	uartMutex = xSemaphoreCreateMutex();

//    gptimer_handle_t gptimer = NULL; // moved to global variable
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, // 1MHz, 1 tick=1us
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));
    gptimer_event_callbacks_t cbs = {
        .on_alarm = timer_group_isr_callback,
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));

    ESP_LOGI(TAG, "Enable timer");
    ESP_ERROR_CHECK(gptimer_enable(gptimer));

    ESP_LOGI(TAG, "Start timer, stop it at alarm event");
	gptimer_alarm_config_t alarm_config1 = {
		.reload_count = 0,
		.alarm_count = 3646, // period in microseconds
		.flags.auto_reload_on_alarm = false
	};
	ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config1));
	ESP_ERROR_CHECK(gptimer_start(gptimer));


    while (1) {

    	if(uartNewFrame == 0)   // the last received frame is processed => collect data for the next frame
    	{
        	uint8_t tmpByte = 0;
    		if(uart_read_bytes(UART_NUM_2, &tmpByte, 1, 2 / portTICK_PERIOD_MS ) > 0)	// 2ms is just enough to read 1 byte at 9600 bauds
    		{
				uartRxBuf[uartRxCount] = tmpByte;           // store the received byte in the input buffer array
				uartRxCount++;                              // increment byte index for the next read

				if(uartRxCount > (RX_BUF_SIZE-1)) uartRxCount = RX_BUF_SIZE-1;    // do not let the counter to overflow beyond the array size
				timer_restart();               // (re)start timer1 with the 3.5 character time
    		}
    	}

    	if(uartNewFrame == 1)					// there is new frame received
    	{

        	if(xSemaphoreTake(uartMutex, pdMS_TO_TICKS(20)) == pdTRUE){
			for (int a = 0; a < uartRxCount; a++){
			  ESP_LOGI(TAG, "[%i] = %x", a, uartRxBuf[a]);
			}
			if(uartRxBuf[0] == 1){				// check frame type (ToDo: add all supported types with OR operator)
			  // check frame CRC
			  ConvertW2B.Word = CRC16(uartRxBuf, uartRxCount-2);
			  if( (uartRxBuf[uartRxCount-2] == ConvertW2B.Bytes.ByteH) &&
				  (uartRxBuf[uartRxCount-1] == ConvertW2B.Bytes.ByteL) )
			  {
				ed_type = uartRxBuf[0];
				switch(uartRxBuf[0])			// Process request according the frame type
				{
				  case 1:					// type 1 (battery, weight, temperature, humidity)
					ed_id = 			(uint16_t)	((uartRxBuf[1] << 8) + uartRxBuf[2]);
					ed_battery =		(uint8_t) 	(uartRxBuf[3] & 0x7F);
					ed_battery_alm	=	(uint8_t)	(uartRxBuf[3] & 0x80);
					ed_weight =			(float)		((uartRxBuf[4] << 8) + uartRxBuf[5])/100;
					ed_temp = 			(float) 	((uartRxBuf[6] << 8) + uartRxBuf[7])/10;
					ed_hum =			(float)		((uartRxBuf[8] << 8) + uartRxBuf[9])/10;
					ESP_LOGI(TAG, "ID = %u, batt = %u, batt_alm = %u, weight = %.2f, temp = %.2f, hum = %.2f ",
							 ed_id, ed_battery, ed_battery_alm, ed_weight, ed_temp, ed_hum);
					f_newValues = true;
				  break;
				//ToDo: add cases for all supported types
				}
			  }
			  else{
				ESP_LOGI(TAG, "BAD CRC");
			  }
	        	//memset(uartRxBuf, 0, RX_BUF_SIZE);
		    }
			xSemaphoreGive(uartMutex);
          }

        	// clear data buffer, counter and flag
        	memset(uartRxBuf, 0, RX_BUF_SIZE);
        	uartRxCount = 0;
        	uartNewFrame = 0;
        }

    	// IMPORTANT! NO DELAY HERE (THERE IS ALREADY ENOUGH IN uart_read_bytes()!!!)
//        vTaskDelay(pdMS_TO_TICKS(10));
//		vTaskDelay(1 / portTICK_PERIOD_MS);
    }


}

void uart_app_start(void)
{
	uart_init();
	/* activate the HC12 module by setting the SET pin HIGH*/
	HC12RunMode();

    xTaskCreate(rx_task, "uart_rx_task", 1024*2, NULL, configMAX_PRIORITIES-1, NULL);
}

