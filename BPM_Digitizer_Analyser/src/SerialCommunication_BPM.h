/*
 * SerialCommunication_BPM.h
 *
 * Created: 23/02/2021 21:41:31
 *  Author: S
 */ 
#include "asf.h"

#ifndef SERIALCOMMUNICATION_BPM_H_
#define SERIALCOMMUNICATION_BPM_H_


/** Configuration parameters **/

uint8_t config[4];


void pdc_uart_initialization(void);

static void configure_UART(void);

void send_cycle_plot();

void send_beam_parameters();

uint32_t send_peaks_only();

uint32_t check_host_commands();


#endif /* SERIALCOMMUNICATION_BPM_H_ */