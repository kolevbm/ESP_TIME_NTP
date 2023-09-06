/*
 * tasks_common.c
 *
 *  Created on: Apr 8, 2023
 *      Author: kolev
 */

#include <board_config.h>

uint16_t CRC16(uint8_t *frame, uint16_t length) {
	uint16_t CheckSum;
	uint16_t j;
	uint16_t i;
	CheckSum = 0xFFFF;
	for (j = 0; j < length; j++) {
		CheckSum = CheckSum ^ (uint16_t) frame[j];
		for (i = 8; i > 0; i--)
			if ((CheckSum) & 0x0001)
				CheckSum = (CheckSum >> 1) ^ 0xA001;
			else
				CheckSum >>= 1;
	}
	return CheckSum;
	//return ((CheckSum & 0xFF) << 8) | (CheckSum >> 8);        // swapped bytes in word
}

