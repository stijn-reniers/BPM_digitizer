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

void sendBuffer(void);
void addSample(uint16_t sample);
volatile void switchBuffer(void);
void cycleEnded(void);
uint16_t getbuffersFilled(void);
void testPrint(void);
uint16_t* getFilledBuffer(void);
uint16_t* getTransmitBuffer(void);
void swap(uint16_t** x, uint16_t** y);
#endif /* BUFFER_H_ */