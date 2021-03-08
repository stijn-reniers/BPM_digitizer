/*
 * SerialCommunication_BPM.c
 *
 * Created: 24/02/2021 9:59:48
 *  Author: Gebruiker
 */ 

#include <asf.h>
#include <stdio_serial.h>
#include <conf_board.h>
#include <conf_pdc_uart.h>
#include "SerialCommunication_BPM.h"
#include "buffer.h"
#include "Algorithms.h"


/* Size of the Pdc transfer buffer */
#define BUFFER_SIZE  3						

/* Pdc transfer buffer */
uint8_t host_command[BUFFER_SIZE] = {0};
uint8_t config[5] = {0};

	
/* PDC data packet for host computer commands */
pdc_packet_t g_pdc_uart_packet;

/* PDC data packet for transmitting beam_parameters */
pdc_packet_t beam_parameters_packet;											// Pass here the address and size of the beam parameter array

/* PDC data packet for transmitting beam_parameters */
pdc_packet_t cycle_plot_packet;												// Pass here the address and size of the cycle plot array

/* Pointer to UART PDC register base (collection of hardware registers)*/
Pdc *g_p_uart_pdc;


/* Configure UART module with desired settings*/

static void configure_UART(void)
{
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
		.paritytype = CONF_UART_PARITY									// no parity bit
	};
		
	sysclk_enable_peripheral_clock(CONSOLE_UART_ID);					// Enable the peripheral clock of the UART0 peripheral
	stdio_serial_init(CONF_UART, &uart_serial_options);					// Setting UART as the stdio device, passing the options by reference
}


void send_cycle_plot()
{	
	
	pdc_tx_init(g_p_uart_pdc, &cycle_plot_packet, NULL);
	config[3] = 0;

}


void send_beam_parameters()
{
	compute_beam_parameters();
	pdc_tx_init(g_p_uart_pdc, &beam_parameters_packet, NULL);
	config[2] = 0;
}


/**
 * \brief Interrupt handler for UART interrupt. */
 
void console_uart_irq_handler(void)
{
	 //Get UART status and check if PDC receive buffer is full 
	if ((uart_get_status(CONSOLE_UART) & UART_SR_RXBUFF) == UART_SR_RXBUFF) {
		
		// Configure PDC for data transfer (RX and TX) 
		pdc_rx_init(g_p_uart_pdc, &g_pdc_uart_packet, NULL);		// pass thee PDC register base and the address of the transfer buffer, will do the transfer until expected amount of data is received (which will trigger interrupt)
		uint8_t index = host_command[1];
		config[index] = host_command[2];
		if (index == 1) dacc_write_conversion_data(DACC, config[1]*16);		
		//pdc_tx_init(g_p_uart_pdc, &g_pdc_uart_packet, NULL);		// For now, this transfer echoes the received packet that caused this interrupt, but the packet pointer can be set to the databuffer containing plot- or parameter data
	}
	
	
}



void pdc_uart_initialization(void)
{
	/* Initialize the UART console */
	configure_UART();

	/* Get pointer to UART PDC register base */
	g_p_uart_pdc = uart_get_pdc_base(CONSOLE_UART);

	/* Initialize PDC data packet for transfer (receive/transmit) */
	g_pdc_uart_packet.ul_addr = (uint32_t) host_command;		// start address of transfer packet data is the buffer we defined ourselves
	g_pdc_uart_packet.ul_size = BUFFER_SIZE;					// size of the buffer/packet
	
	beam_parameters_packet.ul_addr = (uint32_t) beam_parameters;		// start address of transfer packet data is the buffer we defined ourselves
	beam_parameters_packet.ul_size = 104;
	
	cycle_plot_packet.ul_addr = (uint32_t) transmit_buffer;		// start address of transfer packet data is the buffer we defined ourselves
	cycle_plot_packet.ul_size = 16668;
	
	/* Enable PDC transfers, here we set both transmitter and receiver high (full duplex) */
	pdc_enable_transfer(g_p_uart_pdc, PERIPH_PTCR_RXTEN | PERIPH_PTCR_TXTEN);
	pdc_rx_init(g_p_uart_pdc, &g_pdc_uart_packet, NULL);
	
	/* Enable UART IRQ for receive buffer full*/
	uart_enable_interrupt(CONSOLE_UART, UART_IER_RXBUFF);
	
	/* Enable UART interrupt */
	NVIC_EnableIRQ(CONSOLE_UART_IRQn);
	
}
