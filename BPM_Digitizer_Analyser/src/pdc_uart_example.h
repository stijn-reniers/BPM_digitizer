
#ifndef PDC_UART_EXAMPLE_H_INCLUDED
#define PDC_UART_EXAMPLE_H_INCLUDED

/**
 * \page asfdoc_sam_drivers_pdc_example Peripheral DMA Controller Example
 *
 * \section asfdoc_sam_drivers_pdc_example_purpose Purpose
 *
 * The pdc_uart example demonstrates how to use PDC driver to receive/send
 * data from/to the UART.
 *
 * \section asfdoc_sam_drivers_pdc_example_requirements Requirements
 *
 * This example can be used on any SAM3/4 boards.
 *
 * \section asfdoc_sam_drivers_pdc_example_description Description
 *
 * The SAM controller waits for BUFFER_SIZE data to receive from the UART.
 * As soon as the expected amount of data is received, the whole buffer is
 * sent back to the terminal.
 *
 * \section asfdoc_sam_drivers_pdc_example_usage Usage
 *
 * -# Build the program and download it into the evaluation board.
 * -# On the computer, open, and configure a terminal application
 *    (e.g., HyperTerminal on Microsoft&reg; Windows&reg;) with these settings:
 *   - 115200 baud
 *   - 8 bits of data
 *   - No parity
 *   - 1 stop bit
 *   - No flow control
 * -# In the terminal window, the following text should appear (values depend
 *    on the board and chip used):
     \verbatim
      -- PDC Uart Example xxx --
      -- xxxxxx-xx
      -- Compiled: xxx xx xxxx xx:xx:xx --
     \endverbatim
 * -# The sent text should appear.
 */

 #endif /* PDC_UART_EXAMPLE_H_INCLUDED */
 
