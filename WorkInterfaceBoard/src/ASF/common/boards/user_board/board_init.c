#include "asf.h"

void board_init(void)
{
    // Enable peripheral clocks for PIO controllers
    pmc_enable_periph_clk(ID_PIOA);
    pmc_enable_periph_clk(ID_PIOD);

    /***********************
     * SPI0 - ADS1120
     ***********************/
  
    /***********************
     * ENCODER PINS
     ***********************/
    // Encoder 1 pins (PA5, PA1) - configured as inputs with pull-up
    pio_set_input(PIOA, PIO_PA5, PIO_PULLUP);  // ENC1_A
    pio_set_input(PIOA, PIO_PA1, PIO_PULLUP);  // ENC1_B
    
    // Encoder 2 pins (PA15 conflicts with DRDY, using PA16 for ENC2_B)
    // Note: PA15 is used for SPI DRDY, so ENC2_A functionality is disabled
    pio_set_input(PIOA, PIO_PA16, PIO_PULLUP); // ENC2_B
    pio_set_input(PIOA, PIO_PA15, PIO_PULLUP);  // ENC1_A
    // Encoder enable pins (PD17, PD27) - configured as outputs, high = enabled
    pio_set_output(PIOD, PIO_PD17, 1, 0, 0);  // ENC1_ENABLE
    pio_set_output(PIOD, PIO_PD27, 1, 0, 0);  // ENC2_ENABLE


}
