#include <asf.h>
#include <stdio_serial.h>
#include <conf_board.h>
#include <conf_pdc_uart.h>

#include "serialCommunication_BPM.h"
#include "buffer.h"
#include "algorithms.h"



/* Size of the Pdc transmit buffer (echoing the Matlab-host commands, omitted in case of C++ ImGUI host) in BYTES*/
#define BUFFER_SIZE_HOST_COMMAND  3	
#define BUFFER_SIZE_PARAMETERS	  38				// change to 112 for double-based transmission
#define BUFFER_SIZE_PLOTDATA	  16668					
#define BUFFER_SIZE_ECHO		  5
uint16_t size_indicator = BUFFER_SIZE_PLOTDATA + 2;
/* Pdc transfer buffer */
uint8_t host_command[BUFFER_SIZE_HOST_COMMAND] = {0};
uint8_t config[5] = {0,20,0,0,0};

/* PDC data packet for host computer commands */
pdc_packet_t g_pdc_uart_packet;

/* PDC data packet for transmitting beam_parameters */
pdc_packet_t beam_parameters_packet;											

/* PDC data packet for transmitting beam_parameters */
pdc_packet_t cycle_plot_packet;			
									
/* PDC data packet for echoing settings changes */
pdc_packet_t echo_packet;

/* Pointer to UART PDC register base (collection of peripheral hardware registers)*/
Pdc *g_p_uart_pdc;



/* Configure UART module with desired settings*/

void configure_UART(void)
{
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
		.paritytype = CONF_UART_PARITY											
	};
		
	sysclk_enable_peripheral_clock(CONSOLE_UART_ID);							
	stdio_serial_init(CONF_UART, &uart_serial_options);							
}


/* Send the plotting data (8334 12-bit sample values) of one BPM80-cycle */

void send_cycle_plot()
{	
	usart_serial_write_packet(UART0, &size_indicator, 2);
	pdc_tx_init(g_p_uart_pdc, &cycle_plot_packet, NULL);
	config[3] = 0;														// reset the plotting data flag in configuration array
																	
}


/* Send beam parameter values (12 double-precision values) */

void send_beam_parameters()
{
	compute_avgd_parameters();
	//uint16_t size_indicator = BUFFER_SIZE_PARAMETERS;
	beam_parameters_bytes[0] = BUFFER_SIZE_PARAMETERS & 0xff;
	beam_parameters_bytes[1] = (BUFFER_SIZE_PARAMETERS << 8) & 0xff00;
	pdc_tx_init(g_p_uart_pdc, &beam_parameters_packet, NULL);
	config[2] = 0;																		// reset parameter data flag in configuration array
}


/* Interrupt handler for UART interrupt. */
 
void console_uart_irq_handler(void)
{
	//Get UART status and check if PDC receive buffer is full 
	if ((uart_get_status(CONSOLE_UART) & UART_SR_RXBUFF) == UART_SR_RXBUFF) {
		
		// Configure PDC for data transfer (RX and TX) 
		
		pdc_rx_init(g_p_uart_pdc, &g_pdc_uart_packet, NULL);				// pass the PDC register base and the address of the transfer buffer, will start the transfer and wait until expected amount of data is received (which will trigger interrupt)
		
		uint8_t command_index = 0;
		if(host_command[0] == 255)											// check front delimiter of the host packet
		{
			command_index= host_command[1];											// second element of host command contains index in configuration array (indicates which setting to change)
			config[command_index] = host_command[2];								// third element is the new value of the specified setting
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
		
		//if (command_index == 0) pdc_tx_init(g_p_uart_pdc, &g_pdc_uart_packet, NULL);						// This transfer echoes the received packet that caused this interrupt, so computer application can check if command is correctly received (only for debugging)
	}
	
	
}


void pdc_uart_initialization(void)
{
	/* Initialize the UART console */
	configure_UART();

	/* Get pointer to UART PDC register base */
	g_p_uart_pdc = uart_get_pdc_base(CONSOLE_UART);

	/* Initialize PDC data packet for transfer (receive/transmit) by specifying base pointer and size of the packet */
	g_pdc_uart_packet.ul_addr = (uint32_t) host_command;					// receive buffer which we also echo back to the computer
	g_pdc_uart_packet.ul_size = BUFFER_SIZE_HOST_COMMAND;								
	
	beam_parameters_packet.ul_addr = (uint32_t) beam_parameters_bytes;			// transmit packet/buffer for beam parameters
	beam_parameters_packet.ul_size = BUFFER_SIZE_PARAMETERS;
	
	cycle_plot_packet.ul_addr = (uint32_t) transmit_buffer;					// start address of transfer packet data is the buffer we defined ourselves
	cycle_plot_packet.ul_size = BUFFER_SIZE_PLOTDATA;
	
	echo_packet.ul_addr = (uint32_t) echo;
	echo_packet.ul_size = BUFFER_SIZE_ECHO;
	
	/* Enable PDC transfers, here we set both transmitter and receiver high (full duplex). Receiver and transmitter hardware operate independently. 
	   We start the receive transfer, transmits are always started in response to a received command*/
	pdc_enable_transfer(g_p_uart_pdc, PERIPH_PTCR_RXTEN | PERIPH_PTCR_TXTEN);
	pdc_rx_init(g_p_uart_pdc, &g_pdc_uart_packet, NULL);
	
	/* Enable UART IRQ for receive buffer full (host command received)*/
	uart_enable_interrupt(CONSOLE_UART, UART_IER_RXBUFF);
	
	/* Enable UART interrupt */
	NVIC_EnableIRQ(CONSOLE_UART_IRQn);
	
}
