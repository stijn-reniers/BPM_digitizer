/*****************************************************************
   Authors : Decoster B., Reniers S.
   Master's thesis project Group T, IMEC
   BPM-80 Embedded digitization module 
   
******************************************************************/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "asf.h"
#include "delay.h"
#include "conf_uart_serial.h"
#include "conf_board.h"
#include "conf_clock.h"
#include "stdio_serial.h"

#include "buffer.h"
#include "algorithms.h"
#include "serialCommunication_BPM.h"


/** ------------------------------------------------------------------------------------ */
/**								MACRO DEFINITIONS										 */
/** ------------------------------------------------------------------------------------ */

#define delayPin LED0_GPIO
#define delayOutput IOPORT_CREATE_PIN(PIOA, 24)
#define resetButton IOPORT_CREATE_PIN(PIOA, 25)

/** Reference voltage for AFEC in mv. */
#define VOLT_REF			   (3300)

/** The DAC Channel value */
#define DACC_CHANNEL_0 0

/** Analog control value */
#define DACC_ANALOG_CONTROL (DACC_ACR_IBCTLCH0(0x02) \
							| DACC_ACR_IBCTLCH1(0x02) \
							| DACC_ACR_IBCTLDACCORE(0x01))

						
								
/** ------------------------------------------------------------------------------------ */
/**								GLOBAL VARIABLES										 */
/** ------------------------------------------------------------------------------------ */

/** Variable containing the AFEC (ADC) sample values for the BPM-80 collector signal  */

uint16_t collector_sample_data = 0;

/* Trigger state of the fiducial */

bool triggered = false;

/* Signal buffer full flag : set at every fiducial pulse, reset in main loop */
/* Volatile keyword is important as it avoids compiler optimization of (basically ignoring) if()-statement in main loop */

volatile bool fullBuffer=false;

/** Variable containing configurable trigger delay in ms (always between 0 and 67, so max 1 fiducial cycle)  */

uint8_t triggerOffset = 0;

int delayCounter=0;


/** ------------------------------------------------------------------------------------ */
/**   Function prototypes																 */
/** ------------------------------------------------------------------------------------ */

void setDelayTimer(int delayFreq);
void updateDelayCounter();


/** ------------------------------------------------------------------------------------ */
/**   Function implementations															 */
/** ------------------------------------------------------------------------------------ */

/**
  Interrupt handler for the Analog Comparator Controller (triggered by fiducial pulse).
*/

void ACC_Handler(void)
{
	/* Obtain the trigger offset from configuration array */
	triggerOffset = config[0];

	/* Set trigger offset to 60ms  if larger, clipping the delay to max one cycle */												
	if (triggerOffset > 60) triggerOffset = 60;								
	
	/* Obtain the interrupt status flag */
	uint32_t ul_status = acc_get_interrupt_status(ACC);								
		
	/* Compare Output Interrupt */
	
	if ((ul_status & ACC_ISR_CE) == ACC_ISR_CE) 
	{
		/* check if Vin+ > Vin- */
		
		if (acc_get_comparison_result(ACC))									
		{
			if(!triggered)
			{
				triggered= true;											
				
				/* If there is no delay configured, handle the databuffers directly */		
													
				if(triggerOffset == 0)
				{
					fullBuffer=true;
					tc_start(TC0,0);
					cycleEnded();	
					
					/* outputs the digitized (pulsed) fiducial trigger signal on a digital pin for the operator */
					ioport_set_pin_level(delayOutput, IOPORT_PIN_LEVEL_HIGH);
				}
				
				/* Otherwise, initiate the hardware timer to trigger an interrupt after the delay period */
				
				else														
				{
					setDelayTimer(1000/triggerOffset);						
					tc_start(TC0,1);
					ioport_set_pin_level(delayPin,LED0_ACTIVE_LEVEL);		
				}
			}				
		} 
		
		
		/* When Vin+ < Vin-, make sure the fiducial trigger is switched off */
		
		else 
		{
			if(triggered)
			triggered=false;
		}
	}
	
}

/**
 * Interrupt handler for the Trigger offset timer (only invoked if trigger delay > 0).
   Timer is stopped immediately after first interrupt, as one period equals the configured delay time (trigger offset).
   Data buffers are then handled to complete the cycle.
 */

void TC1_Handler(void){
	
	ioport_set_pin_level(delayPin,LED0_INACTIVE_LEVEL);
	
	/* outputs the digitized (pulsed) fiducial trigger signal on a digital pin for the operator */
	ioport_set_pin_level(delayOutput, IOPORT_PIN_LEVEL_HIGH);
	
	NVIC_DisableIRQ(TC1_IRQn);
	NVIC_ClearPendingIRQ(TC1_IRQn);
	tc_disable_interrupt(TC0, 1, TC_IER_CPCS);
	tc_stop(TC0,1);
	
	fullBuffer=true;
	cycleEnded();
	tc_start(TC0,0);
	
}


/* AFEC0 interrupt callback function, add samples to the databuffer */

static void collector_data_ready(void)
{
	collector_sample_data = afec_get_latest_value(AFEC0);										
	addSampleCollector(collector_sample_data);													
	updateDelayCounter();
}



/* Configure trigger interrupt-driven AFEC by TIOA output of timer at the desired sample rate.*/

static void configure_tc_trigger(void)
{
    uint32_t ul_div = 0;
	uint32_t ul_tc_clks = 0;
	uint32_t ul_sysclk = sysclk_get_cpu_hz();											
	
	int sampleFreq= SAMPLING_FREQUENCY;
	
	/* AFEC conversion timer */
	pmc_enable_periph_clk(ID_TC0);	
	
	pmc_enable_periph_clk(ID_TC1);
	
	tc_find_mck_divisor(sampleFreq, ul_sysclk, &ul_div, &ul_tc_clks, ul_sysclk);
	tc_init(TC0, 0, ul_tc_clks | TC_CMR_CPCTRG | TC_CMR_WAVE |
			TC_CMR_ACPA_CLEAR | TC_CMR_ACPC_SET);

	TC0->TC_CHANNEL[0].TC_RA = (ul_sysclk / ul_div) / (sampleFreq*2);
	TC0->TC_CHANNEL[0].TC_RC = (ul_sysclk / ul_div) / sampleFreq;
	

    afec_set_trigger(AFEC0, AFEC_TRIG_TIO_CH_0);										
	
}



/* Configure a delay timer to create the desired phase offset configured by the operator */

void setDelayTimer(int delayFreq){
	
	uint32_t ul_sysclk = sysclk_get_cpu_hz();											
	uint32_t ul_div=0;
	uint32_t ul_tc_clks=0;
	uint32_t counts=0;
	
	tc_find_mck_divisor(delayFreq, ul_sysclk, &ul_div, &ul_tc_clks, ul_sysclk);
	tc_init(TC0,1,ul_tc_clks | TC_CMR_CPCTRG);
	counts = (ul_sysclk/ul_div)/delayFreq;
	tc_write_rc(TC0,1,counts);
	
	NVIC_DisableIRQ(TC1_IRQn);
	NVIC_ClearPendingIRQ(TC1_IRQn);
	NVIC_EnableIRQ((IRQn_Type) ID_TC1);
	tc_enable_interrupt(TC0, 1, TC_IER_CPCS);
	
}

/* Configure the AFEC module (AFEC0, channel 6) for ADC conversion of the collector signal */

static void configure_afec(void)
{
	struct afec_config afec_cfg;
	struct afec_ch_config afec_ch_cfg;

	afec_enable(AFEC0);
	
	afec_get_config_defaults(&afec_cfg);
	afec_ch_get_config_defaults(&afec_ch_cfg);

	afec_init(AFEC0, &afec_cfg);
	afec_ch_set_config(AFEC0, AFEC_CHANNEL_6, &afec_ch_cfg);
	afec_channel_set_analog_offset(AFEC0,AFEC_CHANNEL_6, current_offset);
	
	configure_tc_trigger();
	
	afec_channel_enable(AFEC0, AFEC_CHANNEL_6);
	afec_set_callback(AFEC0, AFEC_INTERRUPT_DATA_READY, collector_data_ready, 1);
	
	afec_start_calibration(AFEC0);
	while((afec_get_interrupt_status(AFEC0) & AFEC_ISR_EOCAL) != AFEC_ISR_EOCAL);

}


/* Configure the DAC controller module for comparator trigger level */

static void configureDACC(void){
	
	pmc_enable_periph_clk(ID_DACC);
	dacc_reset(DACC);
	dacc_disable_trigger(DACC);
	dacc_set_transfer_mode(DACC, 0);
	dacc_set_timing(DACC, 0, 0xf);
	dacc_set_channel_selection(DACC, DACC_CHANNEL_0);
	dacc_enable_channel(DACC, DACC_CHANNEL_0);
	dacc_set_analog_control(DACC, DACC_ANALOG_CONTROL);
	dacc_write_conversion_data(DACC, 3100);
	
}


/* A routine to create a short pulse to indicate the position of the fiducial signal using a digital output pin */

void updateDelayCounter(){
	
	if (ioport_get_pin_level(delayOutput) == IOPORT_PIN_LEVEL_HIGH)
	{
		if (delayCounter>=100)
		{
		    delayCounter=0;
			ioport_set_pin_level(delayOutput,IOPORT_PIN_LEVEL_LOW);
		}
		else
		{
			delayCounter++;
		}
	}
	
	else
	{
		delayCounter=0;
	}
}

/* Main entry point of the application */

int main (void)
{
	/* Initializations of peripherals */
	
	sysclk_init();
	board_init();
	pdc_uart_initialization();
	configure_afec();
	configureDACC();
	
	ioport_set_pin_dir(delayPin,IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(delayOutput, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(resetButton, IOPORT_DIR_INPUT);
	
	ioport_set_pin_level(delayOutput,IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_mode(resetButton,IOPORT_MODE_PULLUP|IOPORT_MODE_DEBOUNCE);
	ioport_set_pin_sense_mode(resetButton, IOPORT_SENSE_RISING);
	
	ioport_set_pin_dir(delayPin,IOPORT_DIR_OUTPUT);
	
	pmc_enable_periph_clk(ID_ACC);
	acc_init(ACC, ACC_MR_SELPLUS_AD7, ACC_MR_SELMINUS_DAC0,			
	ACC_MR_EDGETYP_ANY, ACC_MR_INV_DIS);
	NVIC_EnableIRQ(ACC_IRQn);
	acc_enable_interrupt(ACC);	
	
	/* Main event loop, polling for asynchronous data requests and configuration commands from host application*/
	
	while (1) 
	{
		if(fullBuffer)												// flag that indicates a cycle has ended
		{
			fullBuffer=false;
			
			/* Compute the parameters corresponding to this cycle*/
			compute_beam_parameters();
			
			/* Check if data has to be send */
			if (config[2]!= 0) send_beam_parameters();
			else if (config[3]!= 0) send_cycle_plot();
		}
		
		
		/* Check for external signal from reset switch to reset the hardware */
		
		if(ioport_get_pin_level(resetButton)!=IOPORT_PIN_LEVEL_HIGH){
			rstc_start_software_reset(RSTC);
		}
	}
	
	afec_disable_interrupt(AFEC0, AFEC_INTERRUPT_ALL);
	tc_stop(TC0, 0);
	NVIC_DisableIRQ(ACC_IRQn);
	
}