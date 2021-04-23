#ifndef SERIALCOMMUNICATION_BPM_H_
#define SERIALCOMMUNICATION_BPM_H_

#include "asf.h"

/** Configuration parameter array, with following configuration settings in order

	1.	Trigger offset wrt fiducial
	2.	Trigger level 
	3.	Send flag for beam parameters
	4.	Send flag for plot data
	5.	Select fiducial or collector data (not implemented as for now)
	
	**/

uint8_t config[5];
uint8_t echo[5];

void pdc_uart_initialization(void);

void configure_UART(void);

void send_cycle_plot(void);

void send_beam_parameters(void);


#endif /* SERIALCOMMUNICATION_BPM_H_ */