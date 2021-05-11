#ifndef ALGORITHMS_H_
#define ALGORITHMS_H_

#include <math.h>
#define buffersize 8334
#include "buffer.h"
#include "serialCommunication_BPM.h"


/**************************************************************************************************
 ***************************** BEAM PARAMETER ALGORITHMS ******************************************
 **************************************************************************************************/

/*-------------------------------------------------------------------------------------------------
/* GLOBAL VARIABLES 
-------------------------------------------------------------------------------------------------*/

/* Byte buffer that will be sent over serially */

uint8_t beam_parameters_bytes[38];

/* 2D-arrays that accumulate the parameters values over the last 16 cycles */

uint16_t peak_location[7][16];
uint32_t beam_intensity[2][16];
uint16_t fwhm[2][16];
float skewness[2][16];

/* cycle counter (circular index between 0 and 15)*/
uint16_t cycle;

/* Parameters for adaptive DC-compensation algorithms */

uint16_t current_offset;
uint avg_dc_offset;

/* Variables that contain the 16 cycle-averaged beam parameters, and the standard deviation on the beam position within those 16 cycles*/

uint32_t average_peak_info;
uint32_t average_intensity;
uint32_t average_fwhm;
float average_skewness;
float peak_variance;
	

/*-------------------------------------------------------------------------------------------------
/* FUNCTIONALITY 
-------------------------------------------------------------------------------------------------*/

/*	Compute average/mean from given sample data within a certain window  */

float sample_average(uint16_t start, uint16_t end);

/* Find the maximum of the collector signal half cycle and return its index*/

uint16_t find_max(uint16_t* halfcycle, uint16_t length);

/* Find the sum of a window of samples*/

uint16_t sum(uint16_t start, uint16_t end);

/* Find the beam peak locations and peak widths X and Y by detecting the maximum within each halfcycle, 
   and working down until a lower threshold*/
/* Returns an array of 6 elements : 1. peak position X  2. peak edge left X  3. peak edge right X 
									4. peak position Y  5. peak edge left Y  6. peak edge right Y */

void detect_peaks(uint16_t threshold);

/* Compute the beam intensity in the X and Y cross section */

void compute_beam_intensity(uint16_t peak1_left, uint16_t peak1_right, uint16_t peak2_left, uint16_t peak2_right);

/* Compute the beam FWHM in the X and Y cross section, based on the half of the maximum */

void compute_fwhm_by_inspection(uint16_t peak1_left, uint16_t peak1_right, uint16_t peak2_left, uint16_t peak2_right, uint16_t peak1_max, uint16_t peak2_max);

/* Compute the beam FWHM in the X and Y cross section, based on the standard deviation, only for (quasi-)Gaussian beams*/

void compute_fwhm_by_stdev(uint16_t peak1_left, uint16_t peak1_right, uint16_t peak2_left, uint16_t peak2_right);

/* Compute skewness of the beam X and Y, using Pearson's moment coefficient of skewness (default version)*/

void compute_skewness_pearson_moment(uint16_t peak1_left, uint16_t peak1_right, uint16_t peak2_left, uint16_t peak2_right);

/* Compute skewness of the beam X and Y, using Pearson's second (non-parametric) coefficient of skewness (alternative version, less precise than moment version)*/

void compute_skewness_pearson_non_parametric(uint16_t peak1_left, uint16_t peak1_right, uint16_t peak2_left, uint16_t peak2_right, uint16_t peak1_max, uint16_t peak2_max);

/* Call the currently used algorithm functions (defined above) to compute all parameters*/
/* This function is called at the end of every cycle (15 Hz), in the main processing loop (see main.c)*/

void compute_beam_parameters(void);

/* Compute 16-cycle average of each parameter*/
/* This function is called when a command is received to send the parameters to the host computer (see serialCommunication_BPM.c)*/

void compute_avgd_parameters(void);

/* Detect and adjust the DC-offset of the collector signal adaptively */
/* This function is called when the parameters are computed, but can be switched on or off by a command from the host computer.*/

void dc_offset_compensation(void);

/* Compensates the sample values of each peak for the DC-offset, so that the algorithms are not affected */

void set_peaks(void);

/* De-compensates the DC-offset for plotting later on*/

void reset_peaks(void);


#endif /* ALGORITHMS_H_ */