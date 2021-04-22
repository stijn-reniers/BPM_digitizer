#include "buffer.h"
#include "serialCommunication_BPM.h"
#include <stdio.h>
#include <stdbool.h>
	
uint16_t bufferIndexCollector=0;
uint16_t buffersFilled = 0;

/* Three buffer instances */ 

uint16_t buffer0[buffersize]={ 0 };
uint16_t buffer1[buffersize]={ 0 };
uint16_t buffer2[buffersize]={ 0 };

/* Three buffer pointers, that can each be assigned to one of the three instances */

uint16_t* afec_buffer_collector = buffer0;
uint16_t* algorithm_buffer = buffer1;
uint16_t* transmit_buffer = buffer2;


/* Add sample to the signal buffer */

void addSampleCollector(uint16_t sample){
	
	if (bufferIndexCollector<buffersize )
	{
		afec_buffer_collector[bufferIndexCollector]= sample;
		bufferIndexCollector++;
	}
}

/* General pointer swap */

void swap(uint16_t** x, uint16_t** y){
	uint16_t* temp = *x;
	*x=*y;
	*y=temp;
}


/* Buffer switch operation, signal buffer and algorithm are switched. 
   If 16 buffers have been filled, the contents of the algorithm buffer are switched to the transmit buffer. */

volatile void switchBuffer(void){
	
	buffersFilled++;
	if (buffersFilled>16)
	{
		buffersFilled=0; 
		swap(&algorithm_buffer, &transmit_buffer);
		
		
	} 
	
	swap(&afec_buffer_collector, &algorithm_buffer);
		
	bufferIndexCollector=0;
		
}

/* Fills possible gap at the end of the signal buffer with zeros, and performs the buffer pointer switch */

void cycleEnded(void){
	
	if (bufferIndexCollector < buffersize)
	{
		for (int i=bufferIndexCollector;i<buffersize;i++)
		{
			addSampleCollector(0);
		}
	} 
	
	
	switchBuffer();
}


