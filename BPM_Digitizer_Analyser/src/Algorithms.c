#include "algorithms.h"


uint16_t half_cycle_length = (buffersize-1)>>1;
double beam_parameters[14] = {0};

uint16_t peak_location[7][16] = {0};
uint32_t beam_intensity[2][16] ={0};
uint16_t fwhm[2][16] = {0};
float skewness[2][16] = {0};
	
uint16_t cycle = 0;



/**************************************************************************************************
 ***************************** BEAM PARAMETER ALGORITHM IMPLEMENTATIONS ***************************
 **************************************************************************************************/


/*	Compute population average/mean of the distribution (collector peak), which is a sample index*/

double sample_average(uint16_t start, uint16_t end) 
{
	uint32_t sample_avg = 0;
	uint32_t total_frequency = 0;
	
	for (uint16_t i = start; i < end; i++)
	{
		sample_avg += (uint32_t)algorithm_buffer[i]*i;
		total_frequency += (uint32_t)algorithm_buffer[i];
	}
			
	return (double)sample_avg/total_frequency;
}


/* Find the maximum of the BPM-80 data half cycle and return its index */

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

/* Find the sum of a window of samples*/

uint16_t sum(uint16_t start, uint16_t end)
{
	uint16_t result=0;
	for (uint16_t i=start; i< end; i++ )
	{
		result+= algorithm_buffer[i];
	}
	return result;
}

/* Find beam peak locations and peak widths for X and Y using either the threshold or dispersion-based algorithm */
/* Returns an array of 6 elements : 1. peak position X  2. peak edge left X  3. peak edge right X 
									4. peak position Y  5. peak edge left Y  6. peak edge right Y */

void detect_peaks(uint16_t threshold)
{
	uint16_t peak1 = find_max(algorithm_buffer, half_cycle_length);
	uint16_t peak2 = half_cycle_length + find_max(algorithm_buffer + half_cycle_length, half_cycle_length);
	
	// Use peak1 and peak2 to find the 4 corner points of the beams, possibly combine with dispersion-based algorithm
	
	//		beam_parameters[1] = peak1;
	//		beam_parameters[4] = peak2;
	
	peak_location[0][cycle] = peak1;
	peak_location[3][cycle] = peak2;

	
	// find left corner of X peak
	
	for (uint16_t i = 0; i <= peak1; i++)
	{
		if(algorithm_buffer[peak1 - i] < threshold)
		{
			// beam_parameters[2] = peak1-i;
			peak_location[1][cycle] = peak1-i;
			break;
		}
	}

	
	// find right corner of X peak
	
	for (uint16_t i = 0; i <= half_cycle_length-peak1; i++)
	{
		if(algorithm_buffer [peak1 + i] < threshold)
		{
			//beam_parameters[3] = peak1+i;
			peak_location[2][cycle] = peak1 + i;
			break;
		}
	}
	
	
	
	// find left corner of Y peak
	
	for (uint16_t i = 0; i <= peak2; i++)
	{
		if(algorithm_buffer[peak2 - i] < threshold)
		{
			//beam_parameters[5] = peak2-i;
			peak_location[4][cycle] = peak2-i;
			break;
		}
	}
	
	
	
	// find right corner of Y peak
	
	for (uint16_t i = 0; i <= buffersize-peak2; i++)
	{
		if(algorithm_buffer[peak2 + i] < threshold)
		{
			//beam_parameters[6] = peak2+i;
			peak_location[5][cycle] = peak2 + i;
			break;
		}
	}
	

}

/* Compute beam intensity of a cycle in X and Y cross section */
/* Takes as input the borders of both peaks*/
/* Outputs beam intensity of X and Y cross-section, so that higher level data representation can choose how to combine the values*/

void compute_beam_intensity(uint16_t peak1_left, uint16_t peak1_right, uint16_t peak2_left, uint16_t peak2_right)
{
	//beam_parameters[8] = 0;
	//beam_parameters[9] = 0;
	
	beam_intensity[0][cycle] = 0;
	beam_intensity[1][cycle] = 0;
	
	for (uint16_t i = peak1_left; i < peak1_right ; i++)
	{
		//beam_parameters[8] += (uint32_t)algorithm_buffer[i];
		beam_intensity[0][cycle] += (uint32_t)algorithm_buffer[i];
	}
	
	for (uint16_t i = peak2_left; i < peak2_right ; i++)
	{
		//beam_parameters[9] += (uint32_t)algorithm_buffer[i];
		beam_intensity[1][cycle] += (uint32_t)algorithm_buffer[i];
	}
	
	
}


/* Compute FWHM X and Y (based on variance in this case, assumes more or less gaussian profile */

void compute_fwhm(uint16_t peak1_left, uint16_t peak1_right, uint16_t peak2_left, uint16_t peak2_right)
{
	
	uint16_t mean[2] = {0,0};
	long long summed=0;
	int variance=0;
	
	mean[0] = sample_average(peak1_left, peak1_right);
	mean[1] = sample_average(peak2_left, peak2_right);
	
	for (uint16_t i=peak1_left;i<peak1_right;i++ )
	{
		summed+= (pow((i-mean[0]),2)*algorithm_buffer[i]);
	}
	variance=summed/sum(peak1_left,peak1_right);
	
	//beam_parameters[10]= (uint16_t)(sqrt(variance)*2.355);
	fwhm[0][cycle] = (uint16_t)(sqrt(variance)*2.355);
	
	summed=0;
	variance=0;
	for (uint16_t i=peak2_left;i<peak2_right;i++ )
	{
		summed+= (pow((i-mean[1]),2)*algorithm_buffer[i]);
	}
	variance=summed/sum(peak2_left,peak2_right);
	//beam_parameters[11]=(uint16_t) (sqrt(variance)*2.355);
	fwhm[1][cycle] = (uint16_t)(sqrt(variance)*2.355);
}


/* Compute skewness of the beam  X and Y */

void compute_skewness(uint16_t peak1_left, uint16_t peak1_right, uint16_t peak2_left, uint16_t peak2_right)
{
	double first_peak_mean  =  sample_average(peak1_left, peak1_right);
	double second_peak_mean =  sample_average(peak2_left, peak2_right);
	
	double third_central = 0;
	double second_central = 0;
	int sum1 = sum(peak1_left,peak1_right);
	int sum2 = sum(peak2_left,peak2_right);
	
	for (int i=peak1_left;i<peak1_right;i++ )
	{
		
		double sample = (double) algorithm_buffer[i];
		double spread = (double)i-first_peak_mean;
		third_central += spread*spread*spread*sample;
		second_central += spread*spread*sample;
		
	}
	
	third_central = third_central/sum1;
	second_central = second_central/sum1;
	
	double denominator = sqrt(second_central*second_central*second_central);
	third_central = third_central/denominator;
	
	//beam_parameters[12] = third_central;
	skewness[0][cycle] = third_central;
	
	third_central = 0;
	second_central = 0;
	
	for (int i=peak2_left;i<peak2_right;i++ )
	{
		double sample = (double) algorithm_buffer[i];
		double spread = (double)i-second_peak_mean;
		third_central += spread*spread*spread*sample;
		second_central += spread*spread*sample;
	}
	
	third_central = third_central/sum2;
	second_central = second_central/sum2;
	
	denominator = sqrt(second_central*second_central*second_central);
	third_central = third_central/denominator;
	
	//beam_parameters[13] = third_central;
	skewness[1][cycle] = third_central;
}




/* Compute the parameters (to be called at the end of each cycle) and put a delimiter in front that is certain to be different than parameter values*/

void compute_beam_parameters()
{
	detect_peaks(20);	// threshold of 20 (16 mv), might be made user-configurable later
	compute_beam_intensity(peak_location[1][cycle], peak_location[2][cycle], peak_location[4][cycle], peak_location[5][cycle]);
	compute_fwhm(peak_location[1][cycle], peak_location[2][cycle], peak_location[4][cycle], peak_location[5][cycle]);
	compute_skewness(peak_location[1][cycle], peak_location[2][cycle], peak_location[4][cycle], peak_location[5][cycle]);
	
	cycle++;
	if (cycle > 15) cycle = 0;
	
	
}

void compute_avgd_parameters()
{
	
	for (uint8_t i = 0; i< 14 ; i++) beam_parameters[i] = 0;
	
	for(uint8_t i = 0 ; i < 6; i++)
	{
		for (uint8_t j = 0; j < 16; j++) beam_parameters[i+1] += peak_location[i][j];
		beam_parameters[i + 1] = (uint16_t) (beam_parameters[i+1] / 16);
	}
	
	for(uint8_t i = 0 ; i < 2; i++)
	{
		for (uint8_t j = 0; j < 16; j++) beam_parameters[i+8] += beam_intensity[i][j];
		beam_parameters[i + 8] = (uint32_t) (beam_parameters[i+8] / 16);
	}
	
	
	for(uint8_t i = 0 ; i < 2; i++)
	{
		for (uint8_t j = 0; j < 16; j++) beam_parameters[i+10] += fwhm[i][j];
		beam_parameters[i + 10] = (uint16_t) (beam_parameters[i+10] / 16);
	}
	
	for (uint8_t j = 0; j < 16; j++) beam_parameters[12] += skewness[0][j];
	beam_parameters[12] = (float) (beam_parameters[12] / 16);
	
	for (uint8_t j = 0; j < 16; j++) beam_parameters[13] += skewness[1][j];
	beam_parameters[13] = (float) (beam_parameters[13] / 16);
	
	
	
}