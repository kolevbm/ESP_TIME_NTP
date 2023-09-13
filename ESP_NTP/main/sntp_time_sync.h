/*
 * sntp_time_sync.h
 *
 *  Created on: Dec 27, 2022
 *      Author: kolev
 */

#ifndef MAIN_SNTP_TIME_SYNC_H_
#define MAIN_SNTP_TIME_SYNC_H_

/**
 * Starts the NTP server synchronization task.
 */
void sntp_time_sync_task_start(void);

/**
 * Returns local time if set.
 * @return local time buffer.
 */
char* sntp_time_sync_get_time(void);

/**
 * Returns local time if set.
 * @return local time buffer.
 */
time_t sntp_time_sync_get_time_t(void);

/**
 *  Process alarms
 *  Get alarms from NVS and compare with current time
 */

void alarms_processing(void);
#endif /* MAIN_SNTP_TIME_SYNC_H_ */
