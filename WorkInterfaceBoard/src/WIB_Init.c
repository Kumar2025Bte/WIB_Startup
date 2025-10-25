/*
 * CFile1.c
 *
 * Created: 8/21/2025 10:12:19 AM
 *  Author: MKumar
 */ 
#include "WIB_Init.h"
int tool_type = 9;
unsigned char who_lis2 = 0;
int WIB_Init()
{
	SystemInit();
	board_init();
	/* Replace with your application code */
//	spi0_init(1000000, false);   // 1 MHz, MSB-first (lsbfirst=false)
								//initialise SPI0 interface for Load Cell ADS1120
	//toolsense_init();			//Initialise GPIOs for sensing tool type
	
	// Debug: Print SystemCoreClock value
	// You can check this value in debugger or via UART
	volatile uint32_t debug_clock = SystemCoreClock;
	
	// Initialize I2C0 at 100kHz

	

	return 0;  // Success
}
