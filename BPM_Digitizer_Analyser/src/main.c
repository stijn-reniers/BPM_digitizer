/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * This is a bare minimum user application template.
 *
 * For documentation of the board, go \ref group_common_boards "here" for a link
 * to the board-specific documentation.
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# Minimal main function that starts with a call to board_init()
 * -# Basic usage of on-board LED and button
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "asf.h"
#include "delay.h"
#include "conf_uart_serial.h"
/** Reference voltage for AFEC in mv. */
#define VOLT_REF        (3300)

/** The maximal digital value */
#define MAX_DIGITAL_12_BIT     (4095UL)

#define STRING_HEADER "-- AFEC Feature Test Example --\r\n" \
"-- "BOARD_NAME" --\r\n" \
"-- Compiled: "__DATE__" "__TIME__" --""\r"

/** ------------------------------------------------------------------------------------ */
/** global variables       																*/
/** ------------------------------------------------------------------------------------ */
/** AFEC sample data */
float g_afec0_sample_data, g_afec1_sample_data;

/** The maximal digital value */
static uint32_t g_max_digital;

/** The delay counter value */
static uint32_t g_delay_cnt;
/** ------------------------------------------------------------------------------------ */
/** function defenitions																*/
/** ------------------------------------------------------------------------------------ */


/**
 * \brief Configure UART console.
 */
static void configure_console(void)
{
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
		.paritytype = CONF_UART_PARITY
	};

	/* Configure console UART. */
	sysclk_enable_peripheral_clock(CONSOLE_UART_ID);
	stdio_serial_init(CONF_UART, &uart_serial_options);
}
static void print_sample(uint32_t sample)
{
	/*uint32_t zero = 0;
	if(usart_is_tx_ready(CONF_UART)){
		usart_putchar(CONF_UART,sample);
	}*/
	//usart_write(CONF_UART,zero);
	printf("%d ", (uint16_t)sample);
}
/**
 * \brief AFEC0 DRDY interrupt callback function.
 */
static void afec0_data_ready(void)
{
	g_afec0_sample_data = afec_get_latest_value(AFEC0);
	//puts("BPM channel Voltage:");
	print_sample(g_afec0_sample_data);
}

/**
 * \brief AFEC1 DRDY interrupt callback function.
 */
static void afec1_data_ready(void)
{
	g_afec1_sample_data = afec_get_latest_value(AFEC1);
	puts("Fiducial channel Voltage:");
	print_sample(g_afec1_sample_data);
}
/**
 * \brief Simple function to print sample data.
 */


/**
 * \brief Configure to trigger AFEC by TIOA output of timer.
 */
static void configure_tc_trigger(void)
{
    uint32_t ul_div = 0;
	uint32_t ul_tc_clks = 0;
	uint32_t ul_sysclk = sysclk_get_cpu_hz();
	int sampleFreq= 100000;
	// Enable peripheral clock. 
	pmc_enable_periph_clk(ID_TC0);

	tc_find_mck_divisor(sampleFreq, ul_sysclk, &ul_div, &ul_tc_clks, ul_sysclk);
	tc_init(TC0, 0, ul_tc_clks | TC_CMR_CPCTRG | TC_CMR_WAVE |
			TC_CMR_ACPA_CLEAR | TC_CMR_ACPC_SET);

	TC0->TC_CHANNEL[0].TC_RA = (ul_sysclk / ul_div) / (sampleFreq*2);
	TC0->TC_CHANNEL[0].TC_RC = (ul_sysclk / ul_div) / sampleFreq;

	// Start the Timer. 
	tc_start(TC0, 0); 
	

	
	
    afec_set_trigger(AFEC0, AFEC_TRIG_TIO_CH_0);
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
	puts(STRING_HEADER);

	g_afec0_sample_data = 0;
	g_afec1_sample_data = 0;
	g_max_digital = MAX_DIGITAL_12_BIT;

	set_afec_test();

	while (1) {
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


	//ioport_get_pin_level(BUTTON_0_PIN) == BUTTON_0_ACTIVE) {

}