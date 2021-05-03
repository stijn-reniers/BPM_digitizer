#include "algorithms.h"

uint16_t half_cycle_length = (buffersize-1)>>1;

//------- Byte-based fashion of sending data -------//

// Array consists of 1 start byte - 28 data bytes - 1 stop byte

uint8_t beam_parameters_bytes[38] = {0};
uint16_t* peakLocationPtr = (uint16_t*) (beam_parameters_bytes+2);
uint32_t* intensityPtr = (uint32_t*)(beam_parameters_bytes+14);
uint16_t* fwhmPtr = (uint16_t*)(beam_parameters_bytes+18);
int32_t* skewnessPtr = (int32_t*)(beam_parameters_bytes+22);

int32_t* stdDevPtr = (int32_t*) (beam_parameters_bytes+30);

// ------ 2D-arrays containing circularly buffered parameter values of the last 16 cycles ----//

uint16_t peak_location[7][16] = {0};
uint32_t beam_intensity[2][16] ={0};
uint16_t fwhm[2][16] = {0};
float skewness[2][16] = {0};
	
uint16_t cycle = 0;
uint16_t current_offset= 2048;
uint avg_dc_offset = 0;
uint offset_target = 20;


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
		
	peak_location[0][cycle] = peak1;
	peak_location[3][cycle] = peak2;

	
	// find left corner of X peak
	
	for (uint16_t i = 0; i <= peak1; i++)
	{
		if(algorithm_buffer[peak1 - i] < threshold)
		{
			peak_location[1][cycle] = peak1-i;
			break;
		}
	}

	
	// find right corner of X peak
	
	for (uint16_t i = 0; i <= half_cycle_length-peak1; i++)
	{
		if(algorithm_buffer [peak1 + i] < threshold)
		{
			peak_location[2][cycle] = peak1 + i;
			break;
		}
	}
	
	
	
	// find left corner of Y peak
	
	for (uint16_t i = 0; i <= peak2; i++)
	{
		if(algorithm_buffer[peak2 - i] < threshold)
		{
			peak_location[4][cycle] = peak2-i;
			break;
		}
	}
	
	
	
	// find right corner of Y peak
	
	for (uint16_t i = 0; i <= buffersize-peak2; i++)
	{
		if(algorithm_buffer[peak2 + i] < threshold)
		{
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
	
	beam_intensity[0][cycle] = 0;
	beam_intensity[1][cycle] = 0;
	
	for (uint16_t i = peak1_left; i < peak1_right ; i++)
	{
		beam_intensity[0][cycle] += (uint32_t)algorithm_buffer[i];
	}
	
	for (uint16_t i = peak2_left; i < peak2_right ; i++)
	{
		beam_intensity[1][cycle] += (uint32_t)algorithm_buffer[i];
	}
	
}


/* Compute FWHM X and Y (based on variance in this case, assumes more or less gaussian profile */

void compute_fwhm(uint16_t peak1_left, uint16_t peak1_right, uint16_t peak2_left, uint16_t peak2_right,  uint16_t peak1_max, uint16_t peak2_max)
{
	
	/* For a gaussian beam
	
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
	
	fwhm[0][cycle] = (uint16_t)(sqrt(variance)*2.355);
	
	summed=0;
	variance=0;
	for (uint16_t i=peak2_left;i<peak2_right;i++ )
	{
		summed+= (pow((i-mean[1]),2)*algorithm_buffer[i]);
	}
	variance=summed/sum(peak2_left,peak2_right);
		fwhm[1][cycle] = (uint16_t)(sqrt(variance)*2.355); 
		*/
		
	
	/* In general, for all beam types, requires 2 extra arguments */
		
		
		uint16_t half_max = algorithm_buffer[peak1_max]/2;
		
		uint16_t left_hm = 1000;
		uint16_t left_hm_index = 0;
		
		for (uint16_t i = peak1_max; i > peak1_left; i--)
		{
			uint16_t gap = abs(algorithm_buffer[i] - half_max);
			if (gap <= left_hm)
			{
				left_hm = gap;
				left_hm_index = i;
			}
		}
		
		uint16_t right_hm = 1000;
		uint16_t right_hm_index = 0;
		
		for (uint16_t i = peak1_max; i < peak1_right; i++)
		{
			uint16_t gap = abs(algorithm_buffer[i] - half_max);
			if (gap <= right_hm)
			{
				right_hm = gap;
				right_hm_index = i;
			}
		}
		
		fwhm[0][cycle] = right_hm_index - left_hm_index;
		
		half_max = algorithm_buffer[peak2_max]/2;
		
		left_hm = 1000;
		left_hm_index = 0;
		
		for (uint16_t i = peak2_max; i > peak2_left; i--)
		{
			uint16_t gap = abs(algorithm_buffer[i] - half_max);
			if (gap <= left_hm)
			{
				left_hm = gap;
				left_hm_index = i;
			}
		}
		
		right_hm = 1000;
		right_hm_index = 0;
		
		for (uint16_t i = peak2_max; i < peak2_right; i++)
		{
			uint16_t gap = abs(algorithm_buffer[i] - half_max);
			if (gap <= right_hm)
			{
				right_hm = gap;
				right_hm_index = i;
				
			}
		}
		
		fwhm[1][cycle] = right_hm_index - left_hm_index;
	
	
	
	
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
	
	skewness[1][cycle] = third_central;
	
	
} 

/* Alternative method for skewness, employing Pearson's coefficient of skewness 
   void compute_skewness(uint16_t peak1_left, uint16_t peak1_right, uint16_t peak2_left, uint16_t peak2_right, uint16_t peak1_max, uint16_t peak2_max)
   {
	   double first_peak_mean  =  sample_average(peak1_left, peak1_right);
	   double second_peak_mean =  sample_average(peak2_left, peak2_right);
	   
	   uint16_t first_peak_median = 0;
	   uint16_t half_intensity = beam_intensity[0][cycle]/2;
	   uint32_t left_med = 0;
	   for (uint16_t i=peak1_left;i<peak1_right;i++ )
	   {
		   left_med += algorithm_buffer[i];
		   if(left_med >= half_intensity) 
		   {
			   first_peak_median = i;
			   break;
		   }
	   }
	   half_intensity = beam_intensity[1][cycle]/2;
	   uint16_t second_peak_median = 0;
	   uint32_t right_med = 0;
	   
	   for (uint16_t i=peak2_left;i<peak2_right;i++ )
	   {
		   right_med += algorithm_buffer[i];
		   if(right_med >= half_intensity)
		   {
			   second_peak_median = i;
			   break;
		   }
	   }
	   
	   // compute variance
	   
	   float first_peak_variance = 0;
	   float second_peak_variance = 0;
	   
	   for (uint16_t i=peak1_left;i<peak1_right;i++ )
	   {
		   first_peak_variance += ((i-first_peak_mean)*(i-first_peak_mean)*algorithm_buffer[i]);
	   }
	   
	   first_peak_variance = first_peak_variance/sum(peak1_left, peak1_right);
	    
	   for (uint16_t i=peak2_left;i<peak2_right;i++ )
	   {
		   second_peak_variance += ((i-second_peak_mean)*(i-second_peak_mean)*algorithm_buffer[i]);
	   }
	   
	   second_peak_variance = second_peak_variance/sum(peak2_left,peak2_right);
	   
	   // compute Pearson's coefficient of skew
	   
	   skewness[0][cycle] = 3*(first_peak_mean - first_peak_median)/sqrt(first_peak_variance);
	   skewness[1][cycle] = 3*(second_peak_mean - second_peak_median)/sqrt(second_peak_variance);
	   
	    
   }
*/



/* Compute the parameters (to be called at the end of each cycle) and put a delimiter in front that is certain to be different than parameter values*/

void compute_beam_parameters()
{
	if (config[4] == 255) dc_offset_compensation();
	detect_peaks(config[1] + 2048 - current_offset);	// threshold of 20 (16 mv), might be made user-configurable later
	if (config[4] == 255) set_peaks();
	compute_beam_intensity(peak_location[1][cycle], peak_location[2][cycle], peak_location[4][cycle], peak_location[5][cycle]);
	compute_fwhm(peak_location[1][cycle], peak_location[2][cycle], peak_location[4][cycle], peak_location[5][cycle], peak_location[0][cycle], peak_location[3][cycle]);
	compute_skewness(peak_location[1][cycle], peak_location[2][cycle], peak_location[4][cycle], peak_location[5][cycle]);
	if (config[4] == 255) reset_peaks();
	else 
	{
		afec_channel_set_analog_offset(AFEC0,AFEC_CHANNEL_6, 2048);
		current_offset = 2048;
	}
	//compute_skewness(peak_location[1][cycle], peak_location[2][cycle], peak_location[4][cycle], peak_location[5][cycle], peak_location[0][cycle], peak_location[3][cycle]);
	
	cycle++;
	if (cycle > 15) cycle = 0;
	
	
}

void compute_avgd_parameters()
{
		
	// Byte-based parameter array computations
	
	
	
	for (uint8_t i=0; i<6;i++)
	{		
		average_peak_info = 0;
		for (uint8_t j = 0; j < 16; j++) average_peak_info += peak_location[i][j];
		peakLocationPtr[i]= (uint16_t)(average_peak_info/16);
	}
	
	
	average_intensity = 0;
	for (uint8_t i=0; i<2;i++)
	{
		for (uint8_t j = 0; j < 16; j++) average_intensity += beam_intensity[i][j];
	}
	
	*intensityPtr = (uint32_t)(average_intensity/32);
	
	
	//peak variance
	for(uint8_t i=0; i<4; i+=3){
		peak_variance=0;
		for(uint8_t j=0; j<16;j++) peak_variance+= (peak_location[i][j]-peakLocationPtr[i])*(peak_location[i][j]-peakLocationPtr[i]);
		if(i==0){
			stdDevPtr[0]= (int32_t)(sqrt(peak_variance/15)*10000);
			}else{
			stdDevPtr[1]= (int32_t)(sqrt(peak_variance/15)*10000);
		}
	}
	
	
	
	for (uint8_t i=0; i<2;i++)
	{
		average_fwhm = 0;
		for (uint8_t j = 0; j < 16; j++) average_fwhm += fwhm[i][j];
		fwhmPtr[i]= (uint16_t)(average_fwhm/16);
	}
	
	
	for (uint8_t i=0; i<2;i++)
	{
		average_skewness = 0;
		for (uint8_t j = 0; j < 16; j++) average_skewness += skewness[i][j];
		skewnessPtr[i]=  (int32_t) ((average_skewness/16)*10000);
	}
	
}

void dc_offset_compensation(){
	
	uint dc_integral = 0;
	
	for (uint16_t i = peak_location[2][cycle]; i < peak_location[4][cycle]; i++)
	{
		dc_integral += algorithm_buffer[i];
	}
	
	avg_dc_offset = dc_integral/(peak_location[4][cycle]-peak_location[2][cycle]);			// always a positive number !
	
	if(avg_dc_offset>offset_target){
		current_offset+= 1; 
	}else if(avg_dc_offset<offset_target){
		current_offset-= 1; 
	}
	
	afec_channel_set_analog_offset(AFEC0,AFEC_CHANNEL_6,current_offset);

}

void set_peaks(void)
{
	for(uint16_t i = peak_location[1][cycle]; i < peak_location[2][cycle];i++)
	{
		algorithm_buffer[i] -= avg_dc_offset;
	}
	
	for(uint16_t i = peak_location[4][cycle]; i < peak_location[5][cycle];i++)
	{
		algorithm_buffer[i] -= avg_dc_offset;
	}
}

void reset_peaks(void)
{
	for(uint16_t i = peak_location[1][cycle]; i < peak_location[2][cycle];i++)
	{
		algorithm_buffer[i] += avg_dc_offset;
	}
	
	for(uint16_t i = peak_location[4][cycle]; i < peak_location[5][cycle];i++)
	{
		algorithm_buffer[i] += avg_dc_offset;
	}
}