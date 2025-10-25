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
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# "Insert system clock initialization code here" comment
 * -# Minimal main function that starts with a call to board_init()
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
#include <asf.h> // Aggregate include for ASF drivers and services
#include "WIB_Init.h"
#include "can_app.h"
#include "tasks.h"

int main (void)
{
	/* Insert system clock initialization code here (sysclk_init()). */

//	sysclk_init(); // Initialize system clocks based on board configuration
//	board_init(); // Initialize board-specific pins/peripherals (as configured)
	//ioport_init(); // Optional: initialize I/O port service if used

	/* Initialize TIB hardware */
	WIB_Init();
	
	/* Initialize CAN controller */
	if (!can_app_init()) {
		// CAN initialization failed - handle error
		while(1); // Stop execution if CAN fails
	}
	
	/* Note: Encoder initialization is handled in the encoder_task */
	
// 	/* Test CAN communication (optional - can be disabled for production) */
// 	// Simple CAN controller state test first
// 	if (!can_app_simple_test()) {
// 		// CAN controller not in good state - check debug variables
// 		volatile uint32_t debug_simple_test_failed = 1;
// 	}
// 	
// 	// Uncomment the following lines to enable CAN self-test
// 	if (!can_app_test_loopback()) {
// 		// CAN test failed - but continue anyway for now
// 		// In production, you might want to handle this differently
// 		volatile uint32_t debug_loopback_test_failed = 1;
// 	}

	/* Create FreeRTOS tasks */
	create_application_tasks();
	
	/* Start FreeRTOS scheduler */
	vTaskStartScheduler();

	/* Should never reach here */
	while(1);
} // End of main
