#include <asf.h>
#include <stdio_serial.h>
#include <conf_board.h>
#include <conf_pdc_uart.h>

#include "serialCommunication_BPM.h"
#include "buffer.h"
#include "algorithms.h"

/* Size of the Pdc transmit- and receive buffers in BYTES*/

#define BUFFER_SIZE_HOST_COMMAND  3
#define BUFFER_SIZE_PARAMETERS	  38				
#define BUFFER_SIZE_PLOTDATA	  16668
#define BUFFER_SIZE_ECHO		  5

uint16_t size_indicator = BUFFER_SIZE_PLOTDATA + 2;

/**************************************************************************************************
 ***************************** BPM SERIAL COMMUNICATION IMPLEMENTATION ****************************
 **************************************************************************************************/


/* Pdc transfer buffer */

uint8_t host_command[BUFFER_SIZE_HOST_COMMAND] = {0};
uint8_t config[5] = {0,20,0,0,0};

/* PDC data packet for host computer commands */

pdc_packet_t host_command_packet;

/* PDC data packet for transmitting beam_parameters */

pdc_packet_t beam_parameters_packet;											

/* PDC data packet for transmitting plot data */

pdc_packet_t cycle_plot_packet;			
									
/* PDC data packet for echoing settings changes */

pdc_packet_t echo_packet;

/* Pointer to UART PDC register base (collection of peripheral hardware registers)*/

Pdc *g_p_uart_pdc;



/* Configure UART module with desired settings*/
/* Selected UART-module is defined in conf_uart_serial.h */

void configure_UART(void)
{
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
		.paritytype = CONF_UART_PARITY											
	};
		
	sysclk_enable_peripheral_clock(CONSOLE_UART_ID);							
	stdio_serial_init(CONF_UART, &uart_serial_options);							
}


/* Send the plotting data (8334 uint_16 sample values) of one BPM80-cycle */

void send_cycle_plot()
{	
	/* Add the size indicator delimiter to the front of the packet */
	usart_serial_write_packet(UART0, &size_indicator, 2);
	
	/* Start the PDC transfer of the payload */
	pdc_tx_init(g_p_uart_pdc, &cycle_plot_packet, NULL);
	
	/* reset the plotting data flag in the configuration array */
	config[3] = 0;														
																	
}


/* Send beam parameter values (12 double-precision values) */

void send_beam_parameters()
{
	
	compute_avgd_parameters();
	
	/* Add the size indicator delimiter to the front of the packet */
	beam_parameters_bytes[0] = BUFFER_SIZE_PARAMETERS & 0xff;
	beam_parameters_bytes[1] = (BUFFER_SIZE_PARAMETERS << 8) & 0xff00;
	
	/* Start the PDC transfer of the packet*/
	pdc_tx_init(g_p_uart_pdc, &beam_parameters_packet, NULL);
	
	/* reset the parameter data flag in the configuration array */
	config[2] = 0;																		
	
}


/* Interrupt handler for console UART interrupt. */
 
void console_uart_irq_handler(void)
{
	/* Get UART status and check if PDC receive buffer is full (host command received) */
	
	if ((uart_get_status(CONSOLE_UART) & UART_SR_RXBUFF) == UART_SR_RXBUFF) {
		
		/* Configure PDC for data transfer (RX and TX) */
		
		pdc_rx_init(g_p_uart_pdc, &host_command_packet, NULL);				
		
		uint8_t command_index = 0;
		
		/* Check is a host command is correctly received and extract data*/
		if(host_command[0] == 255)											
		{
			/* Update the configuration setting affected by the host command */
			command_index= host_command[1];											
			config[command_index] = host_command[2];	
			
			/* For all configuration setting changes, echo back the request*/							
			if(command_index!= 2 && command_index!= 3){
				echo[0] = BUFFER_SIZE_ECHO & 0xff;
				echo[1] = (BUFFER_SIZE_ECHO << 8) & 0xff00;
				for (int i=0;i<3;i++)
				{
					echo[i+2]=host_command[i];
				}
				pdc_tx_init(g_p_uart_pdc, &echo_packet, NULL);
			}
		}
	}
}


/* Initialize the Peripheral DMA controller for UART communication*/

void pdc_uart_initialization(void)
{
	/* Initialize the UART console */
	
	configure_UART();

	/* Get pointer to UART PDC register base */
	
	g_p_uart_pdc = uart_get_pdc_base(CONSOLE_UART);

	/* Initialize PDC data packet for transfer (receive/transmit) by specifying base pointer and size of the packet */
	
	host_command_packet.ul_addr = (uint32_t) host_command;					
	host_command_packet.ul_size = BUFFER_SIZE_HOST_COMMAND;								
	
	beam_parameters_packet.ul_addr = (uint32_t) beam_parameters_bytes;			
	beam_parameters_packet.ul_size = BUFFER_SIZE_PARAMETERS;
	
	cycle_plot_packet.ul_addr = (uint32_t) transmit_buffer;					
	cycle_plot_packet.ul_size = BUFFER_SIZE_PLOTDATA;
	
	echo_packet.ul_addr = (uint32_t) echo;
	echo_packet.ul_size = BUFFER_SIZE_ECHO;
	
	/* Enable PDC transfers, here we set both transmitter and receiver high (full duplex). Receiver and transmitter hardware operate independently. 
	   We start the receive transfer, transmits are always started in response to a received command*/
	
	pdc_enable_transfer(g_p_uart_pdc, PERIPH_PTCR_RXTEN | PERIPH_PTCR_TXTEN);
	pdc_rx_init(g_p_uart_pdc, &host_command_packet, NULL);
	
	/* Enable UART IRQ for receive buffer full (host command received)*/
	
	uart_enable_interrupt(CONSOLE_UART, UART_IER_RXBUFF);
	
	/* Enable UART interrupt */
	
	NVIC_EnableIRQ(CONSOLE_UART_IRQn);
	
}