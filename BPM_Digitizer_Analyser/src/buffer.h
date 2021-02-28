/*
 * buffer.h
 *
 * Created: 10/02/2021 11:11:09
 *  Author: decos
 */ 


#ifndef BUFFER_H_
#define BUFFER_H_
#include "asf.h"

#include <stdio.h>
#include <string.h>
#define buffersize 8334

/* Pointer to the buffer containing the data samples being acquired from the ADC */
uint16_t *afec_buffer;

/* Pointer to the buffer containing the data samples ready for processing */
uint16_t *algorithm_buffer;

/* Pointer to the buffer containing the data samples ready for processing */
uint16_t *transmit_buffer;

bool send_buffer;

void sendBuffer(void);
void addSample(uint16_t sample);
volatile void switchBuffer(void);
void cycleEnded(void);
uint16_t getbuffersFilled(void);
void testPrint(void);
uint16_t* getFilledBuffer(void);
#endif /* BUFFER_H_ */