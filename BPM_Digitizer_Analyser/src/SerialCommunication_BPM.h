#ifndef SERIALCOMMUNICATION_BPM_H_
#define SERIALCOMMUNICATION_BPM_H_

#include "asf.h"

/**************************************************************************************************
 ***************************** BPM SERIAL COMMUNICATION IMPLEMENTATION ****************************
 **************************************************************************************************/

/*-------------------------------------------------------------------------------------------------
/* GLOBAL VARIABLES
-------------------------------------------------------------------------------------------------*/

/** Configuration parameter array, with following configuration settings in order
	1.	Trigger offset/delay (in ms) with respect to fiducial
	2.	Peak detection threshold
	3.	Send flag for beam parameters
	4.	Send flag for plot data
	5.	Correct DC offset automatically
**/
uint8_t config[5];

/* Echo packet, which is used to validate that the command is correctly received at the embedded side*/

uint8_t echo[5];


/*-------------------------------------------------------------------------------------------------
/* FUNCTIONALITY
-------------------------------------------------------------------------------------------------*/

/* Initialize the Peripheral DMA controller for UART communication*/

void pdc_uart_initialization(void);

/* Configure the UART-module (baud rate, parity, ..) */

void configure_UART(void);

/* Start a PDC transfer of the cycle plot buffer */

void send_cycle_plot(void);

/* Start a PDC transfer of the beam parameter byte buffer */

void send_beam_parameters(void);


#endif /* SERIALCOMMUNICATION_BPM_H_ */