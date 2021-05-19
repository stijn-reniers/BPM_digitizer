#include "algorithms.h"

/**************************************************************************************************
 ***************************** BEAM PARAMETER ALGORITHMS IMPLEMENTATION ***************************
 **************************************************************************************************/

uint16_t half_cycle_length = (buffersize-1)>>1;

/* Initialize byte buffer to zero */

uint8_t beam_parameters_bytes[38] = {0};
	
/* These pointers allow the parameters of different type to be stored in the same byte buffer */
	
uint16_t* peakLocationPtr = (uint16_t*) (beam_parameters_bytes+2);
uint32_t* intensityPtr = (uint32_t*)(beam_parameters_bytes+14);
uint16_t* fwhmPtr = (uint16_t*)(beam_parameters_bytes+18);
int32_t* skewnessPtr = (int32_t*)(beam_parameters_bytes+22);
int32_t* stdDevPtr = (int32_t*) (beam_parameters_bytes+30);

/* 2D-arrays that accumulate the parameters values over the last 16 cycles, initialized to zero*/

uint16_t peak_location[7][16] = {0};
uint32_t beam_intensity[2][16] ={0};
uint16_t fwhm[2][16] = {0};
float skewness[2][16] = {0};

/* Circular index initialized to 0 */
	
uint16_t cycle = 0;

/* Parameters for adaptive DC-compensation algorithm
Initialize the current conversion window from 0 to 3.3 V */

uint16_t current_offset= 2048;
uint avg_dc_offset = 0;
uint offset_target = 20;


/*	Compute population average/mean of the distribution (collector peak), which is a sample index*/

float sample_average(uint16_t start, uint16_t end) 
{
	uint32_t sample_avg = 0;
	uint32_t total_frequency = 0;
	
	for (uint16_t i = start; i < end; i++)
	{
		sample_avg += (uint32_t)algorithm_buffer[i]*i;
		total_frequency += (uint32_t)algorithm_buffer[i];
	}
			
	return (float) sample_avg/total_frequency;
}


/* Find the maximum of the given collector signal halfcycle and return its index */

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


/* Find the peak positions within each halfcycle (descr. see algorithms.h) */

void detect_peaks(uint16_t threshold)
{
	uint16_t peak1 = find_max(algorithm_buffer, half_cycle_length);
	uint16_t peak2 = half_cycle_length + find_max(algorithm_buffer + half_cycle_length, half_cycle_length);
	
	// Use the two maxima to find the 4 corner points of the beams
		
	peak_location[0][cycle] = peak1;
	peak_location[3][cycle] = peak2;

	
	// find left corner of X-peak
	
	for (uint16_t i = 0; i <= peak1; i++)
	{
		if(algorithm_buffer[peak1 - i] < threshold)
		{
			peak_location[1][cycle] = peak1-i;
			break;
		}
	}

	// find right corner of X-peak
	
	for (uint16_t i = 0; i <= half_cycle_length-peak1; i++)
	{
		if(algorithm_buffer [peak1 + i] < threshold)
		{
			peak_location[2][cycle] = peak1 + i;
			break;
		}
	}
		
	// find left corner of Y-peak
	
	for (uint16_t i = 0; i <= peak2; i++)
	{
		if(algorithm_buffer[peak2 - i] < threshold)
		{
			peak_location[4][cycle] = peak2-i;
			break;
		}
	}
		
	// find right corner of Y-peak
	
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


/* Compute the beam FWHM in the X and Y cross section, based on the standard deviation (only for (quasi-)Gaussian beams*/

void compute_fwhm_by_inspection(uint16_t peak1_left, uint16_t peak1_right, uint16_t peak2_left, uint16_t peak2_right,  uint16_t peak1_max, uint16_t peak2_max)
{
	    
		/* FWHM of X-peak */
		
		uint16_t half_max = algorithm_buffer[peak1_max]/2;
		
		uint16_t left_hm = 5000;
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
		
		uint16_t right_hm = 5000;
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
		
		
		/* FWHM of Y-peak */
		
		half_max = algorithm_buffer[peak2_max]/2;
		
		left_hm = 5000;
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
		
		right_hm = 5000;
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


/* Compute the beam FWHM in the X and Y cross section, based on the standard deviation, only for (quasi-)Gaussian beams*/

void compute_fwhm_by_stdev(uint16_t peak1_left, uint16_t peak1_right, uint16_t peak2_left, uint16_t peak2_right)
{
	
		/* FWHM of X-peak */
		
		uint16_t mean[2] = {0,0};
		long long summed= 0;
		int variance= 0;
		
		mean[0] = sample_average(peak1_left, peak1_right);
		mean[1] = sample_average(peak2_left, peak2_right);
		
		for (uint16_t i=peak1_left; i<peak1_right; i++)
		{
			summed+= (i-mean[0])*(i-mean[0])*algorithm_buffer[i];
		}
		
		variance= summed/sum(peak1_left,peak1_right);
		
		fwhm[0][cycle] = (uint16_t)(sqrt(variance)*2.355);
		
		/* FWHM of Y-peak */
		
		summed=0;
		variance=0;
		
		for (uint16_t i=peak2_left;i<peak2_right;i++ )
		{
			summed+= (i-mean[1])*(i-mean[1])*algorithm_buffer[i];
		}
		
		variance=summed/sum(peak2_left,peak2_right);
		
		fwhm[1][cycle] = (uint16_t)(sqrt(variance)*2.355);
}

	

/* Compute skewness of the beam  X and Y */

void compute_skewness_pearson_moment(uint16_t peak1_left, uint16_t peak1_right, uint16_t peak2_left, uint16_t peak2_right)
{
	
	float first_peak_mean  =  sample_average(peak1_left, peak1_right);
	float second_peak_mean =  sample_average(peak2_left, peak2_right);
	
	/* Compute skewness of X-peak */
	
	float third_central = 0;
	float second_central = 0;
	int sum1 = sum(peak1_left,peak1_right);
	int sum2 = sum(peak2_left,peak2_right);
	

	for (int i=peak1_left;i<peak1_right;i++ )
	{
		
		float sample = (float) algorithm_buffer[i];
		float spread = (float)i-first_peak_mean;
		third_central += spread*spread*spread*sample;
		second_central += spread*spread*sample;
		
	}
	
	third_central = third_central/sum1;
	second_central = second_central/sum1;
	
	float denominator = sqrt(second_central*second_central*second_central);
	third_central = third_central/denominator;
	
	skewness[0][cycle] = third_central;
	
	/* Compute skewness of Y-peak */
	
	third_central = 0;
	second_central = 0;
	
	for (int i=peak2_left;i<peak2_right;i++ )
	{
		float sample = (float) algorithm_buffer[i];
		float spread = (float)i-second_peak_mean;
		third_central += spread*spread*spread*sample;
		second_central += spread*spread*sample;
	}
	
	third_central = third_central/sum2;
	second_central = second_central/sum2;
	
	denominator = sqrt(second_central*second_central*second_central);
	third_central = third_central/denominator;
	
	skewness[1][cycle] = third_central;
}


/* Compute skewness of the beam X and Y, using Pearson's second (non-parametric) coefficient of skewness (alternative version)*/

void compute_skewness_pearson_non_parametric(uint16_t peak1_left, uint16_t peak1_right, uint16_t peak2_left, uint16_t peak2_right, uint16_t peak1_max, uint16_t peak2_max)
{
	   double first_peak_mean  =  sample_average(peak1_left, peak1_right);
	   double second_peak_mean =  sample_average(peak2_left, peak2_right);
	   
	   /* Compute median for X and Y peaks */
	   
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
	   
	   /* Compute variance for X and Y peaks */
	   
	   float first_peak_variance = 0;
	   float second_peak_variance = 0;
	   
	   for (uint16_t i=peak1_left; i<peak1_right; i++ )
	   {
		   first_peak_variance += ((i-first_peak_mean)*(i-first_peak_mean)*algorithm_buffer[i]);
	   }
	   
	   first_peak_variance = first_peak_variance/sum(peak1_left, peak1_right);
	    
	   for (uint16_t i=peak2_left; i<peak2_right; i++ )
	   {
		   second_peak_variance += ((i-second_peak_mean)*(i-second_peak_mean)*algorithm_buffer[i]);
	   }
	   
	   second_peak_variance = second_peak_variance/sum(peak2_left,peak2_right);
	   
	   /* compute Pearson's second coefficient of skew */
	   
	   skewness[0][cycle] = 3*(first_peak_mean - first_peak_median)/sqrt(first_peak_variance);
	   skewness[1][cycle] = 3*(second_peak_mean - second_peak_median)/sqrt(second_peak_variance);
}


/* Call the currently used algorithm functions (defined above) to compute all parameters*/

void compute_beam_parameters()
{
	/* If requested, run iteration of adaptive DC-offset algorithm */
	
	if (config[4] == 255) dc_offset_compensation();
	
	/* Detect peak positions */
	
	detect_peaks(config[1] + 2048 - current_offset);
		
	/* Compensate peaks for parameter computations */
	
	if (config[4] == 255) set_peaks();
	
	/* Compute beam parameters using selected methods */
	
	compute_beam_intensity(peak_location[1][cycle], peak_location[2][cycle], peak_location[4][cycle], peak_location[5][cycle]);
	compute_fwhm_by_inspection(peak_location[1][cycle], peak_location[2][cycle], peak_location[4][cycle], peak_location[5][cycle], peak_location[0][cycle], peak_location[3][cycle]);
	compute_skewness_pearson_moment(peak_location[1][cycle], peak_location[2][cycle], peak_location[4][cycle], peak_location[5][cycle]);
	
	/* De-compensate peaks after parameter computations */
	
	if (config[4] == 255) reset_peaks();
	
	/* If no DC-offset compensation is required, set conversion range from 0 to 3.3V */
	
	else 
	{
		afec_channel_set_analog_offset(AFEC0,AFEC_CHANNEL_6, 2048);
		current_offset = 2048;
	}
	
	/* Update the circular index in the 2D-buffers maintaining parameters over last 16 cycles */
	
	cycle++;
	if (cycle > 15) cycle = 0;
}


/* Compute 16-cycle average of each parameter*/

void compute_avgd_parameters()
{
	/* Compute average of the peak position parameter values */
	
	for (uint8_t i=0; i<6;i++)
	{		
		average_peak_info = 0;
		for (uint8_t j = 0; j < 16; j++) average_peak_info += peak_location[i][j];
		peakLocationPtr[i]= (uint16_t)(average_peak_info/16);
	}
	
	/* Compute the variance in peak position over the last 16 cycles */
	
	for(uint8_t i=0; i<4; i+=3){
		peak_variance=0;
		for(uint8_t j=0; j<16;j++) peak_variance+= (peak_location[i][j]-peakLocationPtr[i])*(peak_location[i][j]-peakLocationPtr[i]);
		if(i==0){
			stdDevPtr[0]= (int32_t)(sqrt(peak_variance/15)*10000);
			}else{
			stdDevPtr[1]= (int32_t)(sqrt(peak_variance/15)*10000);
			}
	}
		
	/* Compute average of the peak intensity, as an arithmetic average of X and Y cross section values, so only one value at the end */
	
	average_intensity = 0;
	for (uint8_t i=0; i<2;i++)
	{
		for (uint8_t j = 0; j < 16; j++) average_intensity += beam_intensity[i][j];
	}	
	*intensityPtr = (uint32_t)(average_intensity/32);
		
	/* Compute average of the FWHM */ 
	
	for (uint8_t i=0; i<2;i++)
	{
		average_fwhm = 0;
		for (uint8_t j = 0; j < 16; j++) average_fwhm += fwhm[i][j];
		fwhmPtr[i]= (uint16_t)(average_fwhm/16);
	}
	
	/* Compute average of the skewness */ 
	
	for (uint8_t i=0; i<2;i++)
	{
		average_skewness = 0;
		for (uint8_t j = 0; j < 16; j++) average_skewness += skewness[i][j];
		skewnessPtr[i]=  (int32_t) ((average_skewness/16)*10000);
	}
}


/* Detect and adjust the DC-offset of the collector signal adaptively */

void dc_offset_compensation() {
	
	uint dc_integral = 0;
	
	/* Compute the integral between the lower ADC reference (starting at 0V = 2048) and the part between the two peaks (= lower reference of the signal) */
	
	for (uint16_t i = peak_location[2][cycle]; i < peak_location[4][cycle]; i++)
	{
		dc_integral += algorithm_buffer[i];
	}
	
	/* Divide the integral by the integration interval to obtain the average DC-offset of the signal */
	
	avg_dc_offset = dc_integral/(peak_location[4][cycle]-peak_location[2][cycle]);			
	
	/* Adjust the current offset to evolve to the target offset value, based on the calculated average DC-offset (step size 1 here is enough for fast convergence) */
	
	if(avg_dc_offset>offset_target){
		current_offset+= 1;					
	}else if(avg_dc_offset<offset_target){
		current_offset-= 1; 
	}
	
	/* Set the current offset to the new value, which should converge to the target value */
	
	afec_channel_set_analog_offset(AFEC0,AFEC_CHANNEL_6,current_offset);
}


/* Compensates the sample values of each peak for the DC-offset, so that the algorithms are not affected */

void set_peaks(void)
{
	for(uint16_t i = peak_location[1][cycle]; i < peak_location[2][cycle];i++)
	{
		algorithm_buffer[i] -= 2048-current_offset;
	}
	
	for(uint16_t i = peak_location[4][cycle]; i < peak_location[5][cycle];i++)
	{
		algorithm_buffer[i] -= 2048-current_offset;
	}
}

/* De-compensates the DC-offset for plotting later on*/

void reset_peaks(void)
{
	for(uint16_t i = peak_location[1][cycle]; i < peak_location[2][cycle];i++)
	{
		algorithm_buffer[i] += 2048-current_offset;
	}
	
	for(uint16_t i = peak_location[4][cycle]; i < peak_location[5][cycle];i++)
	{
		algorithm_buffer[i] += 2048-current_offset;
	}
}