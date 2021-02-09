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

/** Reference voltage for AFEC in mv. */
#define VOLT_REF			   (3300)

/** The maximal digital value (unsigned long)*/
#define MAX_DIGITAL_12_BIT     (4095UL)

/** Title to appear on Putty terminal*/
#define STRING_HEADER "-- Vertical prototype 1 : AFEC interface and UART link --\r\n" \


#define fiducialInput IOPORT_CREATE_PIN(PORTA, 6)
#define buffersize 16667

/** ------------------------------------------------------------------------------------ */
/** Global variable definitions   														 */
/** ------------------------------------------------------------------------------------ */

/** Variables containing the AFEC samples for the 
	Collector (0) and Fiducial(1) BPM-80 signal  */
float g_afec0_sample_data, g_afec1_sample_data;

/** Data buffer creation (as a simple array for now) */
uint16_t buffer[buffersize];
uint16_t bufferIndex=0;

/** The maximal digital value */
static uint32_t g_max_digital;

/** The delay counter value */
static uint32_t g_delay_cnt;


/** ------------------------------------------------------------------------------------ */
/** Function definitions																 */
/** ------------------------------------------------------------------------------------ */


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

	printf("%d.%d mv \n\r", (int32_t)j, (int32_t)i);
}


/* Send sample value over UART stdio */

static void print_sample(uint16_t sample)
{
	/*uint32_t zero = 0;
	if(usart_is_tx_ready(CONF_UART)){
		usart_serial_putchar(CONF_UART,sample);
	}
	//usart_write(CONF_UART,zero);
	
	*/
	//float voltage  = ((float)sample/4096)*3.3;
	printf("%u\n\r", sample);
	
	
}


/* brief AFEC0 DRDY interrupt callback function. */

static void afec0_data_ready(void)
{
	g_afec0_sample_data = afec_get_latest_value(AFEC0);					// Obtain latest sample from COLLECTOR signal (EXT3 - pin4 (ch6))
	buffer[bufferIndex]= g_afec0_sample_data;							// Transfer the sample to the buffer @ sample_index within the scanning wire cycle
	bufferIndex++;														// adjust buffer index
}

/* brief AFEC1 DRDY interrupt callback function. */

static void afec1_data_ready(void)
{
	g_afec1_sample_data = afec_get_latest_value(AFEC1);					// Obtain latest sample from FIDUCIAL signal (EXT3 - pin3 (ch0))
	
}


/* Configure to trigger interrupt-driven AFEC by TIOA output of timer at the desired sample rate.*/

static void configure_tc_trigger(void)
{
    uint32_t ul_div = 0;
	uint32_t ul_tc_clks = 0;
	uint32_t ul_sysclk = sysclk_get_cpu_hz();											// returns (possibly prescaled) clock frequency
	
	int sampleFreq= 250000;
	
	pmc_enable_periph_clk(ID_TC0);														// Enable peripheral clock of timer counter 0

	tc_find_mck_divisor(sampleFreq, ul_sysclk, &ul_div, &ul_tc_clks, ul_sysclk);
	tc_init(TC0, 0, ul_tc_clks | TC_CMR_CPCTRG | TC_CMR_WAVE |
			TC_CMR_ACPA_CLEAR | TC_CMR_ACPC_SET);

	TC0->TC_CHANNEL[0].TC_RA = (ul_sysclk / ul_div) / (sampleFreq*2);
	TC0->TC_CHANNEL[0].TC_RC = (ul_sysclk / ul_div) / sampleFreq;

	 
	tc_start(TC0, 0);																	// Start the TC0 timer
    afec_set_trigger(AFEC0, AFEC_TRIG_TIO_CH_0);										// Set TC0 as the trigger for AFEC module
}

static void set_afec_test(void)
{
	struct afec_config afec_cfg;
	struct afec_ch_config afec_ch_cfg;

	afec_enable(AFEC0);
	afec_get_config_defaults(&afec_cfg);
	afec_ch_get_config_defaults(&afec_ch_cfg);

		g_delay_cnt = 1000;
		afec_enable(AFEC1);
		afec_init(AFEC0, &afec_cfg);
		afec_init(AFEC1, &afec_cfg);
		afec_ch_set_config(AFEC0, AFEC_CHANNEL_6, &afec_ch_cfg);
		afec_ch_set_config(AFEC1, AFEC_CHANNEL_0, &afec_ch_cfg);
		afec_channel_set_analog_offset(AFEC1, AFEC_CHANNEL_0, 0x800);
		afec_channel_set_analog_offset(AFEC0,AFEC_CHANNEL_6, 0x800);
		afec_set_trigger(AFEC1, AFEC_TRIG_SW);
		configure_tc_trigger();
		afec_channel_enable(AFEC1, AFEC_CHANNEL_0);
		afec_channel_enable(AFEC0, AFEC_CHANNEL_6);
		afec_set_callback(AFEC0, AFEC_INTERRUPT_DATA_READY, afec0_data_ready, 1);
		afec_set_callback(AFEC1, AFEC_INTERRUPT_DATA_READY, afec1_data_ready, 1);
		afec_start_calibration(AFEC0);
		while((afec_get_interrupt_status(AFEC0) & AFEC_ISR_EOCAL) != AFEC_ISR_EOCAL);
		afec_start_calibration(AFEC1);
		while((afec_get_interrupt_status(AFEC1) & AFEC_ISR_EOCAL) != AFEC_ISR_EOCAL);
	
}
int main (void)
{
	/* Initialize the SAM system. */
	sysclk_init();
	board_init();

	configure_console();

	/* Output example information. */
	//puts(STRING_HEADER);

	g_afec0_sample_data = 0;
	g_afec1_sample_data = 0;
	g_max_digital = MAX_DIGITAL_12_BIT;
	bool test;
	set_afec_test();
			while (bufferIndex<buffersize) {
			printf(".");
			//afec_start_software_conversion(AFEC1);
			//delay_ms(g_delay_cnt);
			/* Check if the user enters a key. */
			//if (!uart_read(CONF_UART, &uc_key)) {
			/* Disable all afec interrupt. */
			//afec_disable_interrupt(AFEC0, AFEC_INTERRUPT_ALL);
			//afec_disable_interrupt(AFEC1, AFEC_INTERRUPT_ALL);
			//tc_stop(TC0, 0);
			//set_afec_test();
			//}
		}
		
		afec_disable_interrupt(AFEC0, AFEC_INTERRUPT_ALL);
		afec_disable_interrupt(AFEC1, AFEC_INTERRUPT_ALL);
		tc_stop(TC0, 0);
		
		uint16_t i=0;
		while (i< buffersize)
		{
			print_sample(buffer[i]);
			
			i++;
		}
		
		
}