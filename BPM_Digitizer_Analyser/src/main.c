/* 
   Authors : Decoster Bram, Reniers Stijn
   Master's thesis project Group T, IMEC
   BPM-80 Digitization module 
*/

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
#include "Algorithms.h"
#include "SerialCommunication_BPM.h"

#define delayPin LED0_GPIO

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

/** Variables containing the AFEC samples for the Collector (0) and Fiducial(1) BPM-80 signal  */

uint16_t collector_sample_data = 0;

/** Trigger state of the fiducial */

bool triggered= false;

/** Signal buffer full flag */

volatile bool fullBuffer=false;

/** Indicate start of sampling */

bool startedSampling= false;

uint8_t triggerOffset;


/** ------------------------------------------------------------------------------------ */
/**   Function prototypes																 */
/** ------------------------------------------------------------------------------------ */

static void setDelayTimer(int delayFreq);


/**
 * Interrupt handler for the Analog Comparator Controller (triggered by fiducial).
 */
void ACC_Handler(void)
{
	triggerOffset = config[0];										// Obtain the trigger offset from configuration array
	if (triggerOffset > 67) triggerOffset = 67;
	uint32_t ul_status;
	ul_status = acc_get_interrupt_status(ACC);
	
	/* Compare Output Interrupt */
	
	if ((ul_status & ACC_ISR_CE) == ACC_ISR_CE) {
		if (acc_get_comparison_result(ACC)) {
			
			
						
			if(!triggered){
				triggered= true;
				fullBuffer=true;
				if(triggerOffset==0){
				tc_start(TC0,0);
				cycleEnded();
						
				}else{
					setDelayTimer(1000/triggerOffset);						// set the timer frequency base on delay time
					tc_start(TC0,1);
					ioport_set_pin_level(delayPin,LED0_ACTIVE_LEVEL);
				}
		
				
			}				
		} else {
				if(triggered)
				triggered=false;
		}
	}
	
}

/**
 * Interrupt handler for the Trigger offset timer.
 */

void TC1_Handler(void){
	
	ioport_set_pin_level(delayPin,LED0_INACTIVE_LEVEL);
	NVIC_DisableIRQ(TC1_IRQn);
	NVIC_ClearPendingIRQ(TC1_IRQn);
	tc_disable_interrupt(TC0, 1, TC_IER_CPCS);
	tc_stop(TC0,1);
	cycleEnded();
	tc_start(TC0,0);
	
}


/* AFEC0 interrupt callback function. */

static void collector_data_ready(void)
{
	collector_sample_data = afec_get_latest_value(AFEC0);										// Obtain latest sample from COLLECTOR signal (EXT3 - pin4 (ch6))
	addSampleCollector(collector_sample_data);													// Add the sample to the collector signal buffer
}



/* Configure trigger interrupt-driven AFEC by TIOA output of timer at the desired sample rate.*/

static void configure_tc_trigger(void)
{
    uint32_t ul_div = 0;
	uint32_t ul_tc_clks = 0;
	uint32_t ul_sysclk = sysclk_get_cpu_hz();											// returns (possibly prescaled) clock frequency
	
	uint32_t ul_div2=0;
	uint32_t ul_tc_clks2=0;
	int sampleFreq= 250000;
	
	pmc_enable_periph_clk(ID_TC0);														// Enable peripheral clock of timer counter 0
	pmc_enable_periph_clk(ID_TC1);
	tc_find_mck_divisor(sampleFreq, ul_sysclk, &ul_div, &ul_tc_clks, ul_sysclk);
	tc_init(TC0, 0, ul_tc_clks | TC_CMR_CPCTRG | TC_CMR_WAVE |
			TC_CMR_ACPA_CLEAR | TC_CMR_ACPC_SET);

	TC0->TC_CHANNEL[0].TC_RA = (ul_sysclk / ul_div) / (sampleFreq*2);
	TC0->TC_CHANNEL[0].TC_RC = (ul_sysclk / ul_div) / sampleFreq;
	

    afec_set_trigger(AFEC0, AFEC_TRIG_TIO_CH_0);										// Set TC0 as the trigger for AFEC module
	
}


/* Configure a delay timer to create the desired phase offset configured by the operator */

static void setDelayTimer(int delayFreq){
	
	uint32_t ul_sysclk = sysclk_get_cpu_hz();											// returns (possibly prescaled) clock frequency
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

/* Configure the AFEC module (AFEC0, channel 6) for ADC conversion */

static void configure_afec(void)
{
	struct afec_config afec_cfg;
	struct afec_ch_config afec_ch_cfg;

	afec_enable(AFEC0);
	
	afec_get_config_defaults(&afec_cfg);
	afec_ch_get_config_defaults(&afec_ch_cfg);

	afec_init(AFEC0, &afec_cfg);
	afec_ch_set_config(AFEC0, AFEC_CHANNEL_6, &afec_ch_cfg);
	afec_channel_set_analog_offset(AFEC0,AFEC_CHANNEL_6, 0x800);
	
	configure_tc_trigger();
	
	afec_channel_enable(AFEC0, AFEC_CHANNEL_6);
	afec_set_callback(AFEC0, AFEC_INTERRUPT_DATA_READY, collector_data_ready, 1);
	
	afec_start_calibration(AFEC0);
	while((afec_get_interrupt_status(AFEC0) & AFEC_ISR_EOCAL) != AFEC_ISR_EOCAL);
	
	
		
}


/* Configure the DAC module for comparator trigger level */

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
	pmc_enable_periph_clk(ID_ACC);
	acc_init(ACC, ACC_MR_SELPLUS_AD7, ACC_MR_SELMINUS_DAC0,			// set pin AFEC1 AD1 (EXT1 pin 4) as + comparator and DAC0 as -
	ACC_MR_EDGETYP_ANY, ACC_MR_INV_DIS);
	NVIC_EnableIRQ(ACC_IRQn);
	acc_enable_interrupt(ACC);	
	
	/* Main event loop */
	
	while (1) {
		
		if(fullBuffer){
			
			fullBuffer=false;
			
			if (config[2]!= 0) send_beam_parameters();
			if (config[3]!= 0) send_cycle_plot();
		}
		
	}
	
	afec_disable_interrupt(AFEC0, AFEC_INTERRUPT_ALL);
	tc_stop(TC0, 0);
	NVIC_DisableIRQ(ACC_IRQn);
	
}