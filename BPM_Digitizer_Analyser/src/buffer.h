#ifndef BUFFER_H_
#define BUFFER_H_

#include "asf.h"
#include <stdio.h>
#include <string.h>

#define buffersize 8334

/* Pointer to the buffer containing the data samples being acquired from the ADC */
uint16_t* afec_buffer_collector;

/* Pointer to the buffer containing the data samples ready for processing */
uint16_t *algorithm_buffer;

/* Pointer to the buffer containing the data samples ready for processing */
uint16_t *transmit_buffer;


void swap(uint16_t** x, uint16_t** y);

void copyCollectorBuffer(uint16_t* to);

void sendBuffer(void);

void addSampleCollector(uint16_t sample);

volatile void switchBuffer(void);

void cycleEnded(void);

uint16_t getbuffersFilled(void);

#endif /* BUFFER_H_ */