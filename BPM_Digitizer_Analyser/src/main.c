/* 
   Authors : Decoster Bram, Reniers Stijn
   Master's thesis project Group T, IMEC

   BPM-80 Digitization module vertical prototype 1
   ------------------------------------------------
   Features :
	-	AFEC ADC interface
	-	Data buffer interface
	-	Initial (rough ) implementation of the four algorithms
	-	U(S)ART serial data link
	-	Putty/MATLAB host application for data representation
	
   -------------------------------------------------

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
#include "Cycledatabuffer.h"

#define delayPin LED0_GPIO
/** Reference voltage for AFEC in mv. */
#define VOLT_REF			   (3300)

/** The maximal digital value (unsigned long)*/
#define MAX_DIGITAL_12_BIT     (4095UL)

/** The maximal digital value */
#define MAX_DIGITAL (4095)
/** Title to appear on Putty terminal*/
#define STRING_HEADER "-- Vertical prototype 1 : AFEC interface and UART link --\r\n" \


#define fiducialInput IOPORT_CREATE_PIN(PORTA, 6)

/** The DAC Channel value */
#define DACC_CHANNEL_0 0

/** Analog control value */
#define DACC_ANALOG_CONTROL (DACC_ACR_IBCTLCH0(0x02) \
							| DACC_ACR_IBCTLCH1(0x02) \
							| DACC_ACR_IBCTLDACCORE(0x01))
/** ------------------------------------------------------------------------------------ */
/** Global variable definitions   														 */
/** ------------------------------------------------------------------------------------ */

/** Variables containing the AFEC samples for the 
	Collector (0) and Fiducial(1) BPM-80 signal  */
uint16_t g_afec0_sample_data;



/** The maximal digital value */
static uint32_t g_max_digital;

/** The delay counter value */
static uint32_t g_delay_cnt;
int delay=20;
bool triggered= false;
bool fullBuffer=false;
bool startedSampling= false;
int test=0;

/** ------------------------------------------------------------------------------------ */
/** Function definitions																 */
/** ------------------------------------------------------------------------------------ */
static void setDelayTimer(int delayFreq);
/**
 * Interrupt handler for the ACC.
 */
void ACC_Handler(void)
{
	uint32_t ul_status;
	ul_status = acc_get_interrupt_status(ACC);
	/* Compare Output Interrupt */
	if ((ul_status & ACC_ISR_CE) == ACC_ISR_CE) {
		if (acc_get_comparison_result(ACC)) {
			if(!triggered){
				//puts("-ISR- Voltage Comparison Result: AD5 > DAC0\r");
				fullBuffer=true;
				startedSampling=false;
				test=0;
				triggered= true;
				if(delay==0){
					tc_start(TC0,0);
					cycleEnded();
				}else{
					setDelayTimer(delay);
					tc_start(TC0,1);
					//tc_start(TC0,0);
					ioport_set_pin_level(delayPin,LED0_ACTIVE_LEVEL);
				}
				
			}				
		} else {
			//puts("-ISR- Voltage Comparison Result: AD5 < DAC0\r");
			if(triggered)
				triggered=false;
		}
	}
}

void TC1_Handler(void){
	startedSampling=true;
	//printf("%d\n\r",test);
	ioport_set_pin_level(delayPin,LED0_INACTIVE_LEVEL);
	
	
	NVIC_DisableIRQ(TC1_IRQn);
	NVIC_ClearPendingIRQ(TC1_IRQn);
	tc_disable_interrupt(TC0, 1, TC_IER_CPCS);
	tc_stop(TC0,1);
	cycleEnded();
	tc_start(TC0,0);
	
}
/* brief AFEC0 DRDY interrupt callback function. */

static void afec0_data_ready(void)
{
	if (!startedSampling)
	{
		test++;
	}
	g_afec0_sample_data = afec_get_latest_value(AFEC0);					// Obtain latest sample from COLLECTOR signal (EXT3 - pin4 (ch6))
	addSample(g_afec0_sample_data);
}
/* Configure UART console */

static void configure_console(void)
{
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
		.paritytype = CONF_UART_PARITY
	};

	sysclk_enable_peripheral_clock(CONSOLE_UART_ID);					// Enable the peripheral clock of the UART0 peripheral
	stdio_serial_init(CONF_UART, &uart_serial_options);					// Setting UART as the stdio device, passing the options by reference
}


/* Configure to trigger interrupt-driven AFEC by TIOA output of timer at the desired sample rate.*/

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

static void set_afec_test(void)
{
	struct afec_config afec_cfg;
	struct afec_ch_config afec_ch_cfg;

	afec_enable(AFEC0);
	afec_get_config_defaults(&afec_cfg);
	afec_ch_get_config_defaults(&afec_ch_cfg);

		g_delay_cnt = 1000;
		afec_init(AFEC0, &afec_cfg);
		afec_init(AFEC1, &afec_cfg);
		afec_ch_set_config(AFEC0, AFEC_CHANNEL_6, &afec_ch_cfg);
		afec_ch_set_config(AFEC1, AFEC_CHANNEL_0, &afec_ch_cfg);
		afec_channel_set_analog_offset(AFEC1, AFEC_CHANNEL_0, 0x800);
		afec_channel_set_analog_offset(AFEC0,AFEC_CHANNEL_6, 0x800);
		afec_set_trigger(AFEC1, AFEC_TRIG_SW);
		configure_tc_trigger();
		if(delay!=0){
			setDelayTimer(delay);
		}
		afec_channel_enable(AFEC1, AFEC_CHANNEL_0);
		afec_channel_enable(AFEC0, AFEC_CHANNEL_6);
		afec_set_callback(AFEC0, AFEC_INTERRUPT_DATA_READY, afec0_data_ready, 1);
		afec_start_calibration(AFEC0);
		while((afec_get_interrupt_status(AFEC0) & AFEC_ISR_EOCAL) != AFEC_ISR_EOCAL);
		
}

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

int main (void)
{
	sysclk_init();
	board_init();
	configure_console();
	configureDACC();
	
	ioport_set_pin_dir(delayPin,IOPORT_DIR_OUTPUT);
	g_afec0_sample_data = 0;
	g_max_digital = MAX_DIGITAL_12_BIT;
	set_afec_test();
	pmc_enable_periph_clk(ID_ACC);
	acc_init(ACC, ACC_MR_SELPLUS_AD7, ACC_MR_SELMINUS_DAC1,
	ACC_MR_EDGETYP_ANY, ACC_MR_INV_DIS);
	NVIC_EnableIRQ(ACC_IRQn);
	acc_enable_interrupt(ACC);	
	while (1) {
		if(getbuffersFilled()==100){
			//break;
		}
					
		if(fullBuffer){
			fullBuffer=false;
			show_beam_parameters(getFilledBuffer());
		}
		
		
	}
	
	afec_disable_interrupt(AFEC0, AFEC_INTERRUPT_ALL);
	tc_stop(TC0, 0);
	NVIC_DisableIRQ(ACC_IRQn);
	

	
		
}