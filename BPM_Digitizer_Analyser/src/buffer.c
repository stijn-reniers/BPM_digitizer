/*
 * buffer.c
 *
 * Created: 10/02/2021 11:12:47
 *  Author: Decoster
 */ 

#include "buffer.h"
#include "SerialCommunication_BPM.h"
#include <stdio.h>
#include <stdbool.h>
	
uint16_t bufferIndexCollector=0;
uint16_t bufferIndexFiducial = 0;
uint16_t buffersFilled=0;

bool currentbuffer=false;
bool printed=false;

uint16_t buffer0[buffersize]={ 0 };
uint16_t buffer1[buffersize]={ 0 };
uint16_t buffer2[buffersize]={ 0 };

uint16_t* afec_buffer_collector = buffer0;
uint16_t* algorithm_buffer = buffer1;
uint16_t* transmit_buffer = buffer2;




void addSampleCollector(uint16_t sample){
	
	if (bufferIndexCollector<buffersize)
	{
		afec_buffer_collector[bufferIndexCollector]= sample;
		bufferIndexCollector++;
	}
	
}


void swap(uint16_t** x, uint16_t** y){
	uint16_t* temp = *x;
	*x=*y;
	*y=temp;
}

volatile void switchBuffer(void){
	
	buffersFilled++;
	if (buffersFilled>16)
	{
		buffersFilled=0; 
		swap(&algorithm_buffer, &transmit_buffer);
	} 
	
	bufferIndexCollector=0;
	bufferIndexFiducial=0;
	
	swap(&afec_buffer_collector, &algorithm_buffer);
	
}


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

uint16_t getbuffersFilled(){
	return buffersFilled;
}


uint16_t* getFilledBuffer(void){
	return algorithm_buffer;
}


uint16_t* getTransmitBuffer(void){
	return transmit_buffer;
}


void testPrint(void){
	puts("first buffer:\n\r");
	for(int i=0; i<buffersize;i++){
		printf("%u\n\r", buffer0[i]);
	}
	puts("second buffer:\n\r");
	for(int i=0; i<buffersize;i++){
		printf("%u\n\r", buffer1[i]);
	}
	puts("third buffer:\n\r");
	for(int i=0; i<buffersize;i++){
		printf("%u\n\r", buffer2[i]);
	}
}

void sendBuffer(void){
	if (!printed)
	{
		printed=true;
		for(int i=0; i<buffersize;i){
			uint16_t sample = buffer0[i];
			uint16_t sample_swapped_bytes = ((sample<<8)&0xff00)|((sample>>8)&0x00ff);
			usart_serial_write_packet(CONF_UART, &sample_swapped_bytes ,2);
		}
	}
}