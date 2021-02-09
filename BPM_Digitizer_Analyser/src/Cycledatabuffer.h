/*
 * Cycledatabuffer.h
 *
 * Created: 25/01/2021 12:55:28
 *  Author: Decoster B., Reniers S.
 */ 

#include <math.h>

#define buffersize 16667


#ifndef CYCLEDATABUFFER_H_
#define CYCLEDATABUFFER_H_

struct cycleDataBuffer
{
	uint16_t buffer1[buffersize];
	uint16_t buffer2[buffersize];
};

/** Data buffer creation (as a simple array for now) */
uint16_t buffer[buffersize];
uint16_t bufferIndex=0;
uint16_t half_cycle_length = (buffersize-1)>>1;



/*************************************************************************************************
***************************** BEAM PARAMETER COMPUTATIONS ****************************************
**************************************************************************************************/

/*	Compute average/mean from given sample data within a certain window */

uint16_t sample_average(uint16_t* data, uint16_t length) {
		
	uint32_t sample_avg = 0;
	
	for (uint16_t i = 0; i < length; i++)
	{
		sample_avg += (uint32_t)data[i];
	}
	
	sample_avg = sample_avg/length;
	return sample_avg ;
}

/*	Compute standard deviation from given sample data within a certain window */

uint16_t sample_stddev(uint16_t* data, uint16_t length)
{
	uint32_t avg = sample_average(data, length);
	uint32_t variance = 0;
	
	for (uint16_t i = 0; i< length; i++)
	{
		variance += (uint32_t)(data[i] - avg)*(data[i] - avg);
	}
	
	return sqrt(variance/length);
}

/* Find the maximum of the BPM-80 data half cycle and returns its index*/

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
uint16_t* detect_peaks(uint16_t threshold)
{
	uint16_t[6] peaks_localization{0,0,0,0,0,0};
	uint16_t peak1 = find_max(buffer[0], half_cycle_length);
	uint16_t peak2 = half_cycle_length + find_max(buffer[half_cycle_length], half_cycle_length);
	
	// Use peak1[0] and peak2[0] to find the 4 corner points of the beams, possibly combine with dispersion-based algorithm
	
	peaks_localization[0] = peak1;
	peaks_localization[1] = peak2;
	
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
	
	for (uint16_t i = 0; i <= half_cycle_length-peak2; i++)
	{
		if(buffer[peak2 + i] < threshold)
		{
			peaks_localization[5] = peak2+i;
			break;
		}
	}
	
	return peaks_localization;
}

/* Compute beam intensity of a cycle in X and Y cross section */
/* Takes as input the borders of both peaks*/
/* Outputs beam intensity of X and Y crossection, so that higher level data representation can choose whether to multiply or sum them*/

uint16_t* compute_beam_intensity(uint16_t peak1_left, uint16_t peak1_right, uint16_t peak2_left, uint16_t peak2_right)
{
	uint32_t intensity[2] = {0,0};
	
	for (uint16_t i = peak1_left; i < peak1_right ; i++)
	{
		intensity[0] += (uint32_t)buffer[i];
	}
	
	for (uint16_t i = peak2_left; i < peak2_right ; i++)
	{
		intensity[1] += (uint32_t)buffer[i];
	}
	
	return intensity;
}

/* Compute FWHM X and Y */

uint16_t* compute_fwhm(uint16_t max_X, uint16_t max_Y)
{
	/*algorithm to compute FWHM*/ 
}


/* Compute skewness of the beam  X and Y */

float* compute_skewness(void)
{
	uint32_t skewness[2] = {0,0};
	uint16_t* mean[2] = {0,0};
	mean[0] = sample_average(buffer[0], half_cycle_length) /*put here sample avg*/;
	
	for (uint16_t i = 0; i < buffersize ; i++)
	{
		if (i < buffersize/2) skewness[0] += (uint32_t)buffer[i];
		else skewness[1] += (uint32_t)buffer[i];
	}
	
	return skewness;
}


/* Present beam parameters on terminal*/

void show_beam_parameters(void)
{
	
	uint16_t* peak_localization = 0;
	uint16_t* beam_intensity = 0;
	uint16_t* fwhm = 0;
	float* skewness = 0;
	
	peak_localization = detect_peaks(20);
	beam_intensity = compute_beam_intensity(peak_localization[1], peak_localization[2], peak_localization[4], peak_localization[5]);
	fwhm = compute_fwhm(find_max(buffer[0], half_cycle_length), find_max(buffer[half_cycle_length], half_cycle_length));
	skewness = compute_skewness(peak_localization[1], peak_localization[2], peak_localization[4], peak_localization[5]);
	
	printf("BPM-80 beam characteristics \n");
	printf("---------------------------------------\n\n");
	
	printf("Peak localization : \n\n");
	
	printf("Peak X pinnacle : %u \n", peak_localization[0]);
	printf("Peak X start    : %u \n", peak_localization[1]);
	printf("Peak X end      : %u \n\n", peak_localization[2]);
	
	printf("Peak Y pinnacle : %u \n", peak_localization[3]);
	printf("Peak Y start    : %u \n", peak_localization[4]);
	printf("Peak Y end      : %u \n\n", peak_localization[5]);
	
	printf("Beam intensity profile : \n\n");
	
	printf("X intensity : %u \n", beam_intensity[0]);
	printf("Y intensity : %u \n", beam_intensity[1]);
	printf("Total intensity : %u \n\n", beam_intensity[0] + beam_intensity[1]);
	
	printf("Beam FWHM: \n\n");
	
	printf("X FWHM : %u \n", fwhm[0]);
	printf("Y FWHM : %u \n\n", fwhm[1]);
	
	printf("Beam skewness: \n\n");
	
	printf("X skewness : %f \n", skewness[0]);
	printf("Y skewness : %f \n\n", skewness[1]);
	
}


#endif /* CYCLEDATABUFFER_H_ */



/*

uint16_t* detect_peaks_dispersion(uint16_t lag, uint8_t sigma_limits, uint8_t influence)
{
	uint32_t peak_info[4] = {0, 0, 0, 0};
	
	bool peak_signal[buffersize];
	uint16_t filtered_data[buffersize];
	uint16_t avgFilter[buffersize];
	uint16_t stddevFilter[buffersize];
	
	avgFilter[lag - 1] = sample_average(buffer, lag);
	stddevFilter[lag-1] = sample_stddev(buffer, lag);
	
	for (int i = lag ; i < buffersize ; i++)
	{
		if (buffer[i] - avgFilter[i-1] > sigma_limits * stddevFilter[i-1])
		{
			peak_signal[i] = 1;
			filtered_data[i] = ((influence * buffer[i])>>8) + (((255-influence)*filtered_data[i-1])>>8);
		} else peak_signal[i] = 0;
		
		avgFilter[i] = sample_average(filtered_data[i-lag], lag);
		stddevFilter[i] = sample_stddev(filtered_data[i-lag], lag);
	}
}




*/