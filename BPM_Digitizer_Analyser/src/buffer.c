/*
 * buffer.c
 *
 * Created: 10/02/2021 11:12:47
 *  Author: Decoster
 */ 
#include "buffer.h"
#include <stdio.h>
#include <stdbool.h>

uint16_t buffer0[buffersize]={ 0 };
uint16_t buffer1[buffersize]={ 0 };
uint16_t bufferIndex=0;
uint16_t buffersFilled=0;
bool currentbuffer=false;
bool printed=false;


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

void addSample(uint16_t sample){
	if (bufferIndex<buffersize)
	{
		if (currentbuffer)
		{
			//puts("filling buffer 1\n");
			buffer1[bufferIndex]= sample;
		}else{
			//puts("filling buffer 0\n");
			buffer0[bufferIndex]= sample;
		}
		bufferIndex++;
	}
}

volatile void switchBuffer(void){
	buffersFilled++;
	bufferIndex=0;
	//puts("buffer index reset\n");
	currentbuffer= !currentbuffer;
}

void cycleEnded(void){
	//printf("%u\n\r", bufferIndex);
	if (bufferIndex<buffersize)
	{
		for (int i=bufferIndex;i<buffersize;i++)
		{
			addSample(0);
		}
	} 
	
	switchBuffer();
	
}

uint16_t getbuffersFilled(){
	return buffersFilled;
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
}

uint16_t* getFilledBuffer(void){
	if (currentbuffer)
	{
		return buffer0;
	} 
	else
	{
		return buffer1;
	}
}