/**
 * \file
 *
 * \brief User board initialization template
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#include <asf.h>
#include <board.h>
#include <conf_board.h>


void board_init(void)
{
    // Enable peripheral clocks for relevant PIO controllers
    pmc_enable_periph_clk(ID_PIOA);
    pmc_enable_periph_clk(ID_PIOB);
    pmc_enable_periph_clk(ID_PIOC);
    pmc_enable_periph_clk(ID_PIOD);

    /***********************
     * SPI0 - ADS1120
     ***********************/
    // Assign peripheral A functions for SPI0 pins
    pio_configure(PIOA, PIO_PERIPH_A, PIO_PA14A_SPCK,  PIO_DEFAULT); // SPCK
    pio_configure(PIOA, PIO_PERIPH_A, PIO_PA12A_MISO,  PIO_DEFAULT); // MISO
    pio_configure(PIOA, PIO_PERIPH_A, PIO_PA13A_MOSI,  PIO_DEFAULT); // MOSI

    // CS for ADS1120 (manual GPIO control)
    pio_set_output(PIOA, PIO_PA11, 1, 0, 0); // Set high (inactive)

    // DRDY pin as input
    pio_set_input(PIOA, PIO_PA15, PIO_PULLUP);

    /***********************
     * TOOL SENSE (PD21)
     ***********************/
    pio_set_input(PIOD, PIO_PD21, PIO_PULLUP);

    /***********************
     * Additional setup
     ***********************/
    // You can add CAN0, TWI0, LED config here once we confirm pins.
}

