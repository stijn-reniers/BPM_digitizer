/*
 * Cycledatabuffer.h
 *
 * Created: 25/01/2021 12:55:28
 *  Author: Decoster B., Reniers S.
 */ 

#include <math.h>

#define buffersize 8334


#ifndef CYCLEDATABUFFER_H_
#define CYCLEDATABUFFER_H_


//uint16_t buffer[buffersize];

//uint16_t bufferIndex=0;
uint16_t half_cycle_length = (buffersize-1)>>1;

uint16_t peaks_localization[6] = {0,0,0,0,0,0};
uint16_t beam_intensity[2] = {0,0};
double skewness[2] = {0,0};	
uint16_t fwhm[2] = {0,0};
uint8_t count=1;
uint16_t currentIndex=0;
uint16_t sendQuota=0;
	
/* Allows to convert integer sample value to float */

static void print_float(float voltage)
{
	uint8_t i;
	int32_t j;
	float f;

	f = 100.00 * voltage;

	j = (int32_t)f;

	if (voltage > 0) {
		i = j - (int32_t)voltage * 100;
		} else {
		i = (int32_t)voltage * 100 - j;
	}

	j = j / 100;

	printf("%d.%d\n\r", (int32_t)j, (int32_t)i);
}

/**************************************************************************************************
 ***************************** BEAM PARAMETER ALGORITHMS ****************************************
 **************************************************************************************************/

/*	Compute average/mean from given sample data within a certain window  */

uint16_t sample_average(uint16_t start, uint16_t end, uint16_t* buffer) {
		
	uint32_t sample_avg = 0;
	uint32_t total_frequency = 0;
	
	for (uint16_t i = start; i < end; i++)
	{
		sample_avg += (uint32_t)buffer[i]*i;
	}
	
	for (uint16_t i = start; i < end; i++)
	{
		total_frequency += (uint32_t)buffer[i];
	}
	
	return sample_avg/total_frequency;
}


/* Find the maximum of the BPM-80 data half cycle and return its index*/

uint16_t find_max(uint16_t* halfcycle, uint16_t length)
{
	uint16_t maximum = 0;
	uint16_t max_index = 0;
									
	for (uint16_t i = 0; i < length; i++)
	{
		if (halfcycle[i] > maximum) 
		{
			max_index = i;
			maximum = halfcycle[i];
		}
	}
	
	
	return max_index;
}

/* Find beam peak locations and peak widths X and Y using the dispersion-based algorithm */
/* Returns an array of 6 elements : 1. peak position X  2. peak edge left X  3. peak edge right X 
									4. peak position Y  5. peak edge left Y  6. peak edge right Y */

void detect_peaks(uint16_t threshold, uint16_t* buffer)
{
	uint16_t peak1 = find_max(buffer, half_cycle_length);
	uint16_t peak2 = half_cycle_length + find_max(buffer+half_cycle_length, half_cycle_length);
	
	// Use peak1 and peak2 to find the 4 corner points of the beams, possibly combine with dispersion-based algorithm
	
	peaks_localization[0] = peak1;
	peaks_localization[3] = peak2;
	
			
	// find left corner of X peak
		
	for (uint16_t i = 0; i <= peak1; i++)
	{
		if(buffer[peak1 - i] < threshold) 
		{
			peaks_localization[1] = peak1-i;
			break;  
		}
	}
	
	// find right corner of X peak
	
	for (uint16_t i = 0; i <= half_cycle_length-peak1; i++)
	{
		if(buffer[peak1 + i] < threshold) 
		{
			peaks_localization[2] = peak1+i;
			break;
		}
	}
	
	// find left corner of Y peak
	
	for (uint16_t i = 0; i <= peak2; i++)
	{
		if(buffer[peak2 - i] < threshold)
		{
			peaks_localization[4] = peak2-i;
			break;
		}
	}
	
	// find right corner of Y peak
	
	for (uint16_t i = 0; i <= buffersize-peak2; i++)
	{
		if(buffer[peak2 + i] < threshold)
		{
			peaks_localization[5] = peak2+i;
			break;
		}
	}
		
}

/* Compute beam intensity of a cycle in X and Y cross section */
/* Takes as input the borders of both peaks*/
/* Outputs beam intensity of X and Y crossection, so that higher level data representation can choose whether to multiply or sum them*/

void compute_beam_intensity(uint16_t peak1_left, uint16_t peak1_right, uint16_t peak2_left, uint16_t peak2_right, uint16_t* buffer)
{
	beam_intensity[0] = 0;
	beam_intensity[1] = 0;
	
	for (uint16_t i = peak1_left; i < peak1_right ; i++)
	{
		beam_intensity[0] += (uint32_t)buffer[i];
	}
	
	for (uint16_t i = peak2_left; i < peak2_right ; i++)
	{
		beam_intensity[1] += (uint32_t)buffer[i];
	}
	
}

 uint16_t sum(uint16_t start, uint16_t end, uint16_t* buffer)
 {
	 uint16_t result=0;
	 for (uint16_t i=start; i< end; i++ )
	 {
		 result+=buffer[i];
	 }
	 return result;
 }
 
 /* Compute FWHM X and Y */

 void compute_fwhm(uint16_t peak1_left, uint16_t peak1_right, uint16_t peak2_left, uint16_t peak2_right,uint16_t* buffer)
 {
	 
	 uint16_t mean[2] = {0,0};
	 long long summed=0;
	 int variance=0;
	 
	 mean[0] = sample_average(peak1_left, peak1_right,buffer);
	 mean[1] = sample_average(peak2_left, peak2_right,buffer);
	 
	 for (uint16_t i=peak1_left;i<peak1_right;i++ )
	 {
		 summed+= (pow((i-mean[0]),2)*buffer[i]);
	 }
	 variance=summed/sum(peak1_left,peak1_right,buffer);
	
	 fwhm[0]= (uint16_t)(sqrt(variance)*2.355);
	 summed=0;
	 variance=0;
	 for (uint16_t i=peak2_left;i<peak2_right;i++ )
	 {
		 summed+= (pow((i-mean[1]),2)*buffer[i]);
	 }
	 variance=summed/sum(peak2_left,peak2_right,buffer);
	 fwhm[1]=(uint16_t) (sqrt(variance)*2.355);
 }




/* Compute skewness of the beam  X and Y */

void compute_skewness(uint16_t peak1_left, uint16_t peak1_right, uint16_t peak2_left, uint16_t peak2_right,uint16_t* buffer)
{
	int first_peak_mean  =  sample_average(peak1_left, peak1_right,buffer);
	int second_peak_mean =  sample_average(peak2_left, peak2_right,buffer);
	
	double third_central = 0;
	uint32_t second_central = 0;
	
	 for (int i=peak1_left;i<peak1_right;i++ )
	 {
		 third_central = (double)i-1054;//(pow((i-first_peak_mean),3)*buffer[i]);
		 second_central += (pow((i-first_peak_mean),2)*buffer[i]);
	 }
	 
	 second_central = second_central / sum(peak1_left,peak1_right,buffer);
	 //third_central = third_central / sum(peak1_left,peak1_right,buffer);
	
	skewness[0] = peak1_left;
	
	third_central = 0;
	second_central = 0;
	
	
	 for (int i=peak2_left;i<peak2_right;i++ )
	 {
		 second_central += (pow((i-second_peak_mean),2)*buffer[i]);
	 }
	 
	 second_central = second_central / sum(peak2_left,peak2_right,buffer);
	
	skewness[1] = second_central;
	
}



/* Present beam parameters on terminal*/

void show_beam_parameters(uint16_t* buffer,uint16_t* transmitBuffer)
{
	
 	detect_peaks(20, buffer);
 	compute_beam_intensity(peaks_localization[1], peaks_localization[2], peaks_localization[4], peaks_localization[5], buffer);
 	compute_fwhm(peaks_localization[1], peaks_localization[2], peaks_localization[4], peaks_localization[5],buffer);
 	compute_skewness(peaks_localization[1], peaks_localization[2], peaks_localization[4], peaks_localization[5],buffer);

	
	uint16_t peak_info[7] = {0,0,0,0,0,0,9999};
	
	
	
	for (uint16_t i = 0; i < 6; i++)
	{
		 peak_info[i] = peaks_localization[i];
	}
	
	uint16_t peak_width1 = peak_info[2] - peak_info[1] + 1;
	uint16_t peak_one_plot_data[peak_width1];
	uint16_t peak_width2 = peak_info[5] - peak_info[4] + 1;
	uint16_t peak_two_plot_data[peak_width2];
	
	
	for (uint16_t i = 0; i < peak_width1-1; i++)
	{
		peak_one_plot_data[i] =  buffer[peak_info[1] + i];
	}
	
	peak_one_plot_data[peak_width1-1] = 8888; 
	
	for (uint16_t i = 0; i < peak_width2-1; i++)
	{
		peak_two_plot_data[i] =  buffer[peak_info[4] + i];
	}
	
	peak_two_plot_data[peak_width2-1] = 7777; 
	
	
	
	uint16_t beam_parameters[4] = {0,0,0,0};
	
	beam_parameters[0] = beam_intensity[0];
	beam_parameters[1] = beam_intensity[1];
	beam_parameters[2] = fwhm[0];
	beam_parameters[3] = fwhm[1];
			
	
	uint16_t delimiter = 6666;
	usart_serial_write_packet(CONF_UART, &delimiter,2);
	
	for (uint16_t i = 0; i < 7; i++)
    {
		usart_serial_write_packet(CONF_UART, peak_info+i,2);
				
	}


	for (uint16_t i = 0; i < 4; i++)
	{
		usart_serial_write_packet(CONF_UART, beam_parameters + i,2);
	}
	
	for (uint16_t i = 0; i < 2; i++)
	{
		usart_serial_write_packet(CONF_UART, skewness + i,8);
	}
	
	sendQuota+=556;
	if(sendQuota>8334){
		sendQuota= 8334;
	}
	uint16_t transmissionLength= sendQuota- currentIndex;
	usart_serial_write_packet(CONF_UART,8888,2);
	usart_serial_write_packet(CONF_UART,sendQuota,2);
	usart_serial_write_packet(CONF_UART,currentIndex,2);
	for(uint16_t i=currentIndex; i<sendQuota;i+=2 ){
		usart_serial_write_packet(CONF_UART,transmitBuffer[i],2);
	}
	if (sendQuota== 8334)
	{
		sendQuota=0;
		currentIndex=0;
	} 
	else
	{
		currentIndex= sendQuota;
	}
	/*for (uint16_t i = 0; i < peak_width1; i++)
	{
		usart_serial_write_packet(CONF_UART, peak_one_plot_data + i,2);
	}
	
	for (uint16_t i = 0; i < peak_width2; i++)
	{
		usart_serial_write_packet(CONF_UART, peak_two_plot_data + i,2);
	}*/
	
	
	
	
		
		
	/*	
				
		
		

	
	uint16_t separate= 9999;
	usart_serial_write_packet(CONF_UART, &separate,2); */
	
	/*
	printf("%BPM-80 beam characteristics \n\r",count++);
	printf("************************************************\n\r\r");
	
	printf("Peak localization : \n\n\r");
	printf("---------------------------------------\n\n\r");
	
	printf("Peak X pinnacle : %u \n\r", *peaks_localization);
	printf("Peak X start    : %u \n\r", *(peaks_localization+1));
	printf("Peak X end      : %u \n\n\r\r", *(peaks_localization+2));
	
	printf("Peak Y pinnacle : %u \n\r", peaks_localization[3]);
	printf("Peak Y start    : %u \n\r", peaks_localization[4]);
	printf("Peak Y end      : %u \n\n\r\r", peaks_localization[5]);
	
	printf("Beam intensity profile : \n\n\r");
	printf("---------------------------------------\n\n\r");
	
	printf("X intensity : %u \n\r", beam_intensity[0]);
	printf("Y intensity : %u \n\r", beam_intensity[1]);
	printf("Total intensity : %u \n\n\r\r", beam_intensity[0] + beam_intensity[1]);
	
	printf("Beam FWHM: \n\n");
	
	printf("X FWHM : %u \n", fwhm[0]);
	printf("Y FWHM : %u \n\n", fwhm[1]);
	
	printf("Beam skewness: \n\n\r");
	printf("---------------------------------------\n\n\r");
	
	printf("X skewness : ");
	print_float(skewness[0]);
	
	printf("Y skewness : ");
	print_float(skewness[1]);
	
	*/
}




#endif /* CYCLEDATABUFFER_H_ */


