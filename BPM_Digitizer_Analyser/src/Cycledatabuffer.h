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


uint16_t buffer[buffersize];

uint16_t bufferIndex=0;
uint16_t half_cycle_length = (buffersize-1)>>1;

uint16_t peaks_localization[6] = {0,0,0,0,0,0};
uint16_t beam_intensity[2] = {0,0};
float skewness[2] = {0,0};	
	
	
	
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

uint16_t sample_average(uint16_t start, uint16_t end) {
		
	uint32_t sample_avg = 0;
	
	for (uint16_t i = start; i < end; i++)
	{
		sample_avg += (uint32_t)buffer[i];
	}
	
	sample_avg = sample_avg/(end-start);
	return sample_avg ;
}

/*	Compute standard deviation from given sample data within a certain window */

uint16_t sample_variance(uint16_t start, uint16_t end)
{
	uint32_t avg = sample_average(start, end);
	uint32_t variance = 0;
	
	for (uint16_t i = start; i< end; i++)
	{
		variance += (uint32_t)(buffer[i] - avg)*(buffer[i] - avg);
	}
	
	return variance/(end-start);
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

void detect_peaks(uint16_t threshold)
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

void compute_beam_intensity(uint16_t peak1_left, uint16_t peak1_right, uint16_t peak2_left, uint16_t peak2_right)
{
	
	for (uint16_t i = peak1_left; i < peak1_right ; i++)
	{
		beam_intensity[0] += (uint32_t)buffer[i];
	}
	
	for (uint16_t i = peak2_left; i < peak2_right ; i++)
	{
		beam_intensity[1] += (uint32_t)buffer[i];
	}
	
}

/* Compute FWHM X and Y */

void compute_fwhm(uint16_t max_X, uint16_t max_Y)
{
	/*algorithm to compute FWHM*/ 
}


/* Compute skewness of the beam  X and Y */

void compute_skewness(uint16_t peak1_left, uint16_t peak1_right, uint16_t peak2_left, uint16_t peak2_right)
{
	uint16_t* mean[2] = {0,0};
		
	mean[0] = sample_average(peak1_left, peak1_right);
	mean[1] = sample_average(peak2_left, peak2_right);
	
	uint16_t first_peak_mean  = mean[0];
	uint16_t second_peak_mean = mean[1];
	
	uint32_t numerator = 0;
	uint16_t denominator = 0;
	
	for (uint16_t i = peak1_left; i < peak1_right; i++)
	{
		numerator += pow((buffer[i]-first_peak_mean),3); 
	}
	
	skewness[0] =  sample_variance(peak1_left, peak1_right) ;
	
	
	numerator = 0;
	denominator = 0;
	
	for (uint16_t i = peak2_left; i < peak2_right; i++)
	{
		numerator += pow((buffer[i]-second_peak_mean),3);
	}
	
	skewness[1] = ((peak2_right - peak2_left - 2)*pow(sample_variance(peak2_left, peak2_right),3)) ;
	
}



/* Present beam parameters on terminal*/

void show_beam_parameters()
{
	
	
	detect_peaks(20);
	compute_beam_intensity(peaks_localization[1], peaks_localization[2], peaks_localization[4], peaks_localization[5]);
	//fwhm = compute_fwhm(buffer, find_max(buffer, half_cycle_length), find_max(buffer + half_cycle_length, half_cycle_length));
	compute_skewness(peaks_localization[1], peaks_localization[2], peaks_localization[4], peaks_localization[5]);
	
	printf(" BPM-80 beam characteristics \n\r");
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
	
	//printf("Beam FWHM: \n\n");
	
	//printf("X FWHM : %u \n", fwhm[0]);
	//printf("Y FWHM : %u \n\n", fwhm[1]);
	
	printf("Beam skewness: \n\n\r");
	printf("---------------------------------------\n\n\r");
	
	printf("X skewness : ");
	print_float(skewness[0]);
	
	printf("Y skewness : ");
	print_float(skewness[1]);
	
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