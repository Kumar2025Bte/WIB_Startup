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
     * ENCODERS
     ***********************/
    // Encoder 1 inputs (PA5, PA1)
    pio_set_input(PIOA, PIO_PA5, PIO_PULLUP);
    pio_set_input(PIOA, PIO_PA1, PIO_PULLUP);

    // Encoder 1 enable (PD17), active-low — default high (disabled)
    pio_set_output(PIOD, PIO_PD17, 1, 0, 0);
    // If encoder 2 enable (PD27) exists, set default high (disabled)
    pio_set_output(PIOD, PIO_PD27, 1, 0, 0);

   

    /***********************
     * Additional setup
     ***********************/
    // You can add CAN0, TWI0, LED config here once we confirm pins.
}
