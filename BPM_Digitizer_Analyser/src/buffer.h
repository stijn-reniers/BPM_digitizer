#ifndef BUFFER_H_
#define BUFFER_H_

#include "asf.h"
#include <stdio.h>
#include <string.h>

#define SAMPLING_FREQUENCY 125000
#define buffersize SAMPLING_FREQUENCY/15


/**************************************************************************************************
 ***************************** DATA BUFFERS *******************************************************
 **************************************************************************************************/

/*-------------------------------------------------------------------------------------------------
/* GLOBAL VARIABLES
-------------------------------------------------------------------------------------------------*/

/* Pointer to the buffer containing the data samples being acquired from the ADC */
uint16_t* afec_buffer_collector;

/* Pointer to the buffer containing the data samples ready for processing */
uint16_t *algorithm_buffer;

/* Pointer to the buffer containing the data samples for plotting */
uint16_t *transmit_buffer;


/*-------------------------------------------------------------------------------------------------
/* FUNCTIONALITY
-------------------------------------------------------------------------------------------------*/

/* Perform a pointer swap */

void swap(uint16_t** x, uint16_t** y);

/* Add a converted sample to the collector signal buffer */

void addSampleCollector(uint16_t sample);

/* Performs a swap of the buffer pointers */

volatile void switchBuffer(void);

/* This function is called at the end of a 15 Hz cycle, to perform the buffer pointer swap.
   The sample buffer is appended with zeros to the end and the buffer switch is executed */

void cycleEnded(void);


#endif /* BUFFER_H_ */