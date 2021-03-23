#ifndef ALGORITHMS_H_
#define ALGORITHMS_H_

#include <math.h>
#define buffersize 8334
#include "buffer.h"

double beam_parameters[13];

	
/**************************************************************************************************
 ***************************** BEAM PARAMETER ALGORITHMS ******************************************
 **************************************************************************************************/

/*	Compute average/mean from given sample data within a certain window  */

double sample_average(uint16_t start, uint16_t end);

/* Find the maximum of the BPM-80 data half cycle and return its index*/

uint16_t find_max(uint16_t* halfcycle, uint16_t length);

/* Find the sum of a window of samples*/

uint16_t sum(uint16_t start, uint16_t end);

/* Find beam peak locations and peak widths X and Y using either the threshold or dispersion-based algorithm */
/* Returns an array of 6 elements : 1. peak position X  2. peak edge left X  3. peak edge right X 
									4. peak position Y  5. peak edge left Y  6. peak edge right Y */

void detect_peaks(uint16_t threshold);


/* Compute beam intensity of a cycle in X and Y cross section */
/* Takes as input the borders of both peaks*/
/* Outputs beam intensity of X and Y cross-section, so that higher level data representation can choose whether to multiply or sum them*/

void compute_beam_intensity(uint16_t peak1_left, uint16_t peak1_right, uint16_t peak2_left, uint16_t peak2_right);

/* Compute FWHM X and Y */

 void compute_fwhm(uint16_t peak1_left, uint16_t peak1_right, uint16_t peak2_left, uint16_t peak2_right);

/* Compute skewness of the beam  X and Y */

void compute_skewness(uint16_t peak1_left, uint16_t peak1_right, uint16_t peak2_left, uint16_t peak2_right);

/* Present beam parameters on terminal*/

void compute_beam_parameters(void);


#endif /* ALGORITHMS_H_ */


