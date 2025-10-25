#include "can_app.h"
#include "asf.h"
#include "can.h"

#include "FreeRTOS.h"
#include "task.h"

// Define TickType_t if not already defined
#ifndef TickType_t
typedef portTickType TickType_t;
#endif

#ifndef portTICK_PERIOD_MS
#define portTICK_PERIOD_MS portTICK_RATE_MS // Backward compatibility macro
#endif
#ifndef pdMS_TO_TICKS
#define pdMS_TO_TICKS(ms) ((TickType_t)((ms) / portTICK_RATE_MS)) // Convert ms to OS ticks
#endif

static void can0_configure_pins_local(void)
{
	pmc_enable_periph_clk(ID_PIOB); // Enable PIOB clock for CAN0 pins
	pio_configure(PIOB, PIO_PERIPH_A, PIO_PB2A_CANTX0, 0); // PB2 as CANTX0
	pio_configure(PIOB, PIO_PERIPH_A, PIO_PB3A_CANRX0, 0); // PB3 as CANRX0
}

/* Force robust 500 kbps timing for PCLK = 96 MHz using 16 TQ and ~81.25% sample point.
 * Segments: Prop=5, Phase1=7, Phase2=3, SJW=2, BRP = 12 (encoded as N-1 in CAN_BR fields).
 * This yields bitrate = 96e6 / (12 * 16) = 500000 bps, SP = (1+5+7)/16 = 81.25%.
 */
static void can_force_500k_16tq_timing(Can *p_can, uint32_t mck)
{
(void)mck; // mck expected 96 MHz per board clocking
/* Disable before modifying CAN_BR (per driver convention). */
can_disable(p_can);
const uint32_t brp_enc= 12 - 1; // prescaler 12
const uint32_t sjw_enc= 2  - 1; // SJW = 2 tq
const uint32_t propag_enc= 5  - 1; // PropSeg = 5 tq
const uint32_t phase1_enc= 7  - 1; // PhaseSeg1 = 7 tq
const uint32_t phase2_enc= 3  - 1; // PhaseSeg2 = 3 tq
p_can->CAN_BR =
CAN_BR_PHASE2(phase2_enc) |
CAN_BR_PHASE1(phase1_enc) |
CAN_BR_PROPAG(propag_enc) |
CAN_BR_SJW(sjw_enc)       |
CAN_BR_BRP(brp_enc)       |
CAN_BR_SMP_ONCE; // single sampling
can_enable(p_can);
}

bool can_app_init(void)
{
	uint32_t mck = sysclk_get_peripheral_hz(); // Peripheral clock frequency
	if (mck == 0) return false; // Invalid clock frequency
	
	// DIAGNOSTIC: Store actual clock frequencies for debugging
	volatile uint32_t debug_peripheral_hz = mck;
	volatile uint32_t debug_system_core_hz = SystemCoreClock;
	// Expected: mck = 96,000,000 Hz for proper 500kbps CAN timing
	// If these values are different, CAN bit rate will be incorrect
	
	pmc_enable_periph_clk(ID_CAN0); // Enable CAN0 peripheral clock
	can0_configure_pins_local(); // Route pins to CAN peripheral
	
	// Add delay after pin configuration to ensure stability
	delay_ms(10);
	
	// Initialize CAN controller with proper baudrate constant
	// Try 500kbps first (desired rate), then fall back to lower rates if needed
	volatile uint32_t debug_bitrate_used = 0;
	if (can_init(CAN0, mck, CAN_BPS_500K));
//	mck  = mck/6;
// 	if (can_init(CAN0, mck, CAN_BPS_500K)) {
// 		debug_bitrate_used = 500; // 500k worked - preferred rate
// 		/* Override ASF default timing (8..14 TQ) with 16 TQ @ ~81% SP for better margin. */
// 		can_force_500k_16tq_timing(CAN0, mck);
// 	} else if (can_init(CAN0, mck, CAN_BPS_250K)) {
// 		debug_bitrate_used = 250; // 250k worked - fallback
// 	} else if (can_init(CAN0, mck, CAN_BPS_125K)) {
// 		debug_bitrate_used = 125; // 125k worked - last resort
// 	} else {
// 		// CAN initialization failed - likely due to clock/baudrate mismatch
// 		volatile uint32_t debug_can_init_fail = 1;
// 		volatile uint32_t debug_can_sr_after_fail = CAN0->CAN_SR;
// 		return false; // CAN baudrate configuration failed
// 	}
	
	// DIAGNOSTIC: Check CAN status immediately after init
	volatile uint32_t debug_can_sr_after_init = CAN0->CAN_SR;
	volatile uint32_t debug_can_mr_after_init = CAN0->CAN_MR;
	
	// DIAGNOSTIC: Read back CAN baudrate register to verify configuration
	volatile uint32_t debug_can_br = CAN0->CAN_BR;
	// Decode the CAN_BR register fields for analysis
	volatile uint32_t debug_phase2 = (debug_can_br & CAN_BR_PHASE2_Msk) >> CAN_BR_PHASE2_Pos;
	volatile uint32_t debug_phase1 = (debug_can_br & CAN_BR_PHASE1_Msk) >> CAN_BR_PHASE1_Pos;
	volatile uint32_t debug_propag = (debug_can_br & CAN_BR_PROPAG_Msk) >> CAN_BR_PROPAG_Pos;
	volatile uint32_t debug_sjw = (debug_can_br & CAN_BR_SJW_Msk) >> CAN_BR_SJW_Pos;
	volatile uint32_t debug_brp = (debug_can_br & CAN_BR_BRP_Msk) >> CAN_BR_BRP_Pos;
	// Calculate actual bit rate: mck / ((brp + 1) * (1 + (propag + 1) + (phase1 + 1) + (phase2 + 1)))
	volatile uint32_t debug_total_tq = 1 + (debug_propag + 1) + (debug_phase1 + 1) + (debug_phase2 + 1);
	volatile uint32_t debug_actual_bitrate = mck / ((debug_brp + 1) * debug_total_tq);
	// Expected: debug_actual_bitrate should be 500000 for 500kbps
	// Additional debugging: Store intermediate calculation values
	volatile uint32_t debug_brp_plus_1 = debug_brp + 1;
	volatile uint32_t debug_divisor = debug_brp_plus_1 * debug_total_tq;
	volatile uint32_t debug_expected_bitrate = debug_bitrate_used * 1000; // Convert kbps to bps
	
	// DEBUG: Calculate what 500kbps should look like with current MCK
	volatile uint32_t debug_target_500k = 500000; // 500kbps in bps
	volatile uint32_t debug_optimal_divisor = mck / debug_target_500k;
	volatile uint32_t debug_optimal_brp = debug_optimal_divisor / 16 - 1; // Assuming 16 TQ
	volatile uint32_t debug_optimal_tq = debug_optimal_divisor / (debug_optimal_brp + 1);
	
	can_reset_all_mailbox(CAN0); // Reset all mailboxes to known state
	
	/* Configure a generic RX mailbox 0 - ACCEPT ALL MESSAGES for testing */
	can_mb_conf_t mb;
	mb.ul_mb_idx = 0; // Mailbox index 0 for RX
	mb.uc_obj_type = CAN_MB_RX_MODE; // Receive mode
	mb.uc_id_ver = 0; // Standard ID (11-bit)
	mb.ul_id_msk = 0; // NO MASK - Accept all messages (for loopback test compatibility)
	mb.ul_id = 0; // Don't filter on specific ID
	mb.uc_length = 8; // Set MDLC before arming to avoid undefined length
	
	// DIAGNOSTIC: Store mailbox configuration for debugging
	volatile uint32_t debug_mb_idx = mb.ul_mb_idx;
	volatile uint32_t debug_mb_type = mb.uc_obj_type;
	volatile uint32_t debug_mb_id_mask = mb.ul_id_msk;
	volatile uint32_t debug_mb_id = mb.ul_id;
	
	can_mailbox_init(CAN0, &mb); // Apply configuration
	// Arm RX mailbox 0 to start receiving
	can_mailbox_send_transfer_cmd(CAN0, &mb);
	
	// DIAGNOSTIC: Verify mailbox was configured
	volatile uint32_t debug_mb_status_after_init = can_mailbox_get_status(CAN0, 0);
	
	// Initialize TX mailbox 1 for future use
	can_mb_conf_t tx_init;
	tx_init.ul_mb_idx = 1; // Use mailbox 1 for TX
	tx_init.uc_obj_type = CAN_MB_TX_MODE; // Transmit mode
	tx_init.uc_tx_prio = 15; // High priority
	tx_init.uc_id_ver = 0; // Standard ID
	tx_init.ul_id_msk = 0; // Not used for TX
	can_mailbox_init(CAN0, &tx_init); // Configure TX mailbox
	
	// DIAGNOSTIC: Verify TX mailbox was configured
	volatile uint32_t debug_tx_mb_status_after_init = can_mailbox_get_status(CAN0, 1);
	
	// Verify the bit rate is correct
	if (!can_verify_bitrate(debug_bitrate_used)) {
		volatile uint32_t debug_bitrate_verification_failed = 1;
		// Continue anyway, but flag the issue
	}
	
	return true;
}

// Function to verify CAN bit rate configuration
bool can_verify_bitrate(uint32_t expected_kbps)
{
	uint32_t mck = sysclk_get_peripheral_hz();
	uint32_t can_br = CAN0->CAN_BR;
	
	// Decode CAN_BR register
	uint32_t phase2 = (can_br & CAN_BR_PHASE2_Msk) >> CAN_BR_PHASE2_Pos;
	uint32_t phase1 = (can_br & CAN_BR_PHASE1_Msk) >> CAN_BR_PHASE1_Pos;
	uint32_t propag = (can_br & CAN_BR_PROPAG_Msk) >> CAN_BR_PROPAG_Pos;
	uint32_t brp = (can_br & CAN_BR_BRP_Msk) >> CAN_BR_BRP_Pos;
	
	// Calculate actual bit rate
	uint32_t total_tq = 1 + (propag + 1) + (phase1 + 1) + (phase2 + 1);
	uint32_t actual_bitrate = mck / ((brp + 1) * total_tq);
	uint32_t expected_bitrate = expected_kbps * 1000;
	
	// Allow 1% tolerance
	uint32_t tolerance = expected_bitrate / 100;
	return (actual_bitrate >= (expected_bitrate - tolerance)) && 
	       (actual_bitrate <= (expected_bitrate + tolerance));
}

bool can_app_tx(uint32_t id, const uint8_t *data, uint8_t len)
{
	// First, reset the TX mailbox to ensure it's in a clean state
	can_mb_conf_t reset_mb;
	reset_mb.ul_mb_idx = 1;
	reset_mb.uc_obj_type = CAN_MB_DISABLE_MODE; // Disable first
	can_mailbox_init(CAN0, &reset_mb);
	
	// Small delay to ensure reset takes effect
	delay_ms(1);
	
	// Now configure the TX mailbox properly. IMPORTANT: set ul_id before mailbox init
	can_mb_conf_t tx;
	tx.ul_mb_idx = 1; // Use mailbox 1 for TX
	tx.uc_obj_type = CAN_MB_TX_MODE; // Transmit mode
	tx.uc_tx_prio = 15; // High priority
	tx.uc_id_ver = 0; // Standard ID
	tx.ul_id_msk = 0; // Not used for TX
	tx.ul_id = CAN_MID_MIDvA(id); // Set CAN identifier BEFORE init (driver may snapshot)
	can_mailbox_init(CAN0, &tx); // Configure mailbox
	uint32_t dl = 0, dh = 0; // Data low/high 32-bit words
	for (uint8_t i=0;i<4 && i<len;i++) dl |= ((uint32_t)data[i]) << (i*8); // Pack first 4 bytes
	for (uint8_t i=4;i<8 && i<len;i++) dh |= ((uint32_t)data[i]) << ((i-4)*8); // Pack next 4 bytes
	tx.ul_datal = dl;
	tx.ul_datah = dh;
	tx.uc_length = len; // DLC
	
	if (can_mailbox_write(CAN0, &tx) != CAN_MAILBOX_TRANSFER_OK) {
		// DIAGNOSTIC: TX write failed
		volatile uint32_t debug_tx_write_failed = 1;
		volatile uint32_t debug_mb_status_after_write = can_mailbox_get_status(CAN0, 1);
		return false; // Load MB failed
	}
	
	can_global_send_transfer_cmd(CAN0, CAN_TCR_MB1); // Trigger transmission
	
	// Wait for transmission to complete
	delay_ms(10);
	
	return true;
}


bool can_app_reset(void)
{
	
	can_disable(CAN0);
	delay_ms(10);
	can_enable(CAN0);
	delay_ms(10);
		can_reset_all_mailbox(CAN0);
	can_mb_conf_t mb;
	mb.ul_mb_idx = 0; // Mailbox index 0 for RX
	mb.uc_obj_type = CAN_MB_RX_MODE; // Receive mode
	mb.uc_id_ver = 0; // Standard ID (11-bit)
	mb.ul_id_msk = CAN_MAM_MIDvA_Msk; // Mask for standard ID only
	mb.ul_id = CAN_MID_MIDvA(CAN_ID_POT_COMMAND); // Filter on command base ID
	can_mailbox_init(CAN0, &mb); // Apply configuration
	// Arm RX mailbox 0 to start receiving after reset
	can_mailbox_send_transfer_cmd(CAN0, &mb);
	return true;	
}
void can_rx_task(void *arg)
{
	(void)arg; // Unused
	uint32_t error_count = 0;
	
	for (;;) {
		// Check CAN controller status first
		if (!can_app_get_status()) {
			error_count++;
			if (error_count > 100) { // After 100 consecutive errors, try to reinitialize
				can_app_reset();// Note: In a real system, you might want to reset the CAN controller here
				error_count = 0;
			}
			vTaskDelay(pdMS_TO_TICKS(100)); // Wait longer on errors
			continue;
		}
		
		error_count = 0; // Reset error count on successful status check
		
		// Check if RX mailbox has data
		uint32_t mb_status = can_mailbox_get_status(CAN0, 0);
		if (mb_status & CAN_MSR_MRDY) { // If RX mailbox has data
			can_mb_conf_t rx;
			rx.ul_mb_idx = 0;
			
			// Read mailbox with error checking
			if (can_mailbox_read(CAN0, &rx) == CAN_MAILBOX_TRANSFER_OK) {
				uint8_t len = (uint8_t)(rx.ul_status >> CAN_MSR_MDLC_Pos) & 0xF; // Extract DLC
				if (len > 8) len = 8; // Sanity check on length
				
				uint8_t data[8] = {0}; // Initialize to zero
				// Unpack data words
				for (uint8_t i = 0; i < 4 && i < len; i++) {
					data[i] = (rx.ul_datal >> (i * 8)) & 0xFF;
				}
				for (uint8_t i = 0; i < 4 && (i + 4) < len; i++) {
					data[4 + i] = (rx.ul_datah >> (i * 8)) & 0xFF;
				}
				
				// Extract CAN ID and process based on message type
				uint32_t can_id = rx.ul_fid;//(rx.ul_id >> CAN_MID_MIDvA_Pos) & 0x7FFu;
// 				if (can_id == CAN_ID_POT_COMMAND) {
// 					handle_pot_command(data, len); // Execute command
// 				}
				// Note: Other message IDs are received but not processed in this task
				// This allows loopback test (ID 0x123) to be received successfully
			}
		}
		
		vTaskDelay(pdMS_TO_TICKS(5)); // Small delay to yield CPU time
	}
}
bool can_app_get_status(void){
	// Check if CAN controller is properly initialized and running
	if (CAN0->CAN_SR & CAN_SR_BOFF) {
		return false; // CAN controller is in bus-off state (bad)
	}
	
	// Check if there are any error conditions
	uint32_t status = CAN0->CAN_SR;
	if (status & (CAN_SR_ERRA | CAN_SR_WARN | CAN_SR_BOFF)) {
		// DIAGNOSTIC: Store error status for debugging
		volatile uint32_t debug_can_status = status;
		volatile uint32_t debug_tx_errors = can_get_tx_error_cnt(CAN0);
		volatile uint32_t debug_rx_errors = can_get_rx_error_cnt(CAN0);
		// High error counts often indicate bit rate mismatch
		return false; // Error conditions present
	}
	
	// Check if CAN controller is enabled
	if (!(CAN0->CAN_MR & CAN_MR_CANEN)) {
		return false; // CAN controller not enabled
	}
	
	return true; // CAN is working properly
}

bool can_app_test_loopback(void)
{
	// DIAGNOSTIC: Store initial state
	volatile uint32_t debug_initial_sr = CAN0->CAN_SR;
	volatile uint32_t debug_initial_mr = CAN0->CAN_MR;
	
	// For SAM4E, implement loopback by configuring two mailboxes:
	// - MB1 for TX
	// - MB2 for RX (configured to receive the test message)
	
	// First, configure a dedicated RX mailbox for loopback test
	can_mb_conf_t rx_mb;
	rx_mb.ul_mb_idx = 2; // Use mailbox 2 for loopback RX
	rx_mb.uc_obj_type = CAN_MB_RX_MODE; // Receive mode
	rx_mb.uc_id_ver = 0; // Standard ID (11-bit)
	rx_mb.ul_id_msk = CAN_MAM_MIDvA_Msk; // Match exact ID
	rx_mb.ul_id = CAN_MID_MIDvA(0x100); // Accept only test ID 0x100
	can_mailbox_init(CAN0, &rx_mb); // Configure mailbox
	// Arm RX mailbox 2 to start receiving
	can_mailbox_send_transfer_cmd(CAN0, &rx_mb);
	
	// Reset and configure TX mailbox
	can_mb_conf_t reset_tx;
	reset_tx.ul_mb_idx = 1;
	reset_tx.uc_obj_type = CAN_MB_DISABLE_MODE; // Disable first
	can_mailbox_init(CAN0, &reset_tx);
	delay_ms(1);
	
	// Configure TX mailbox
	can_mb_conf_t tx_mb;
	tx_mb.ul_mb_idx = 1; // Use mailbox 1 for TX
	tx_mb.uc_obj_type = CAN_MB_TX_MODE; // Transmit mode
	tx_mb.uc_tx_prio = 15; // High priority
	tx_mb.uc_id_ver = 0; // Standard ID
	tx_mb.ul_id_msk = 0; // Not used for TX
	// IMPORTANT: set ID before initializing mailbox so driver snapshots it correctly
	tx_mb.ul_id = CAN_MID_MIDvA(0x100);
	can_mailbox_init(CAN0, &tx_mb); // Configure mailbox
	
	// Wait for configuration to take effect
	delay_ms(10);
	
	// Check if TX mailbox is ready
	uint32_t tx_mb_status = can_mailbox_get_status(CAN0, 1);
	volatile uint32_t debug_tx_mb_ready = (tx_mb_status & CAN_MSR_MRDY) != 0;
	if (!(tx_mb_status & CAN_MSR_MRDY)) {
		// DIAGNOSTIC: TX mailbox not ready
		volatile uint32_t debug_tx_mb_not_ready = 1;
		return false;
	}
	
	// Test transmission
	uint8_t test_data[4] = {0xAA, 0x55, 0x12, 0x34};
	uint32_t test_id = 0x100; // Test ID that matches RX filter
	
	// Send test message using the TX mailbox
	// Ensure mailbox's ID is set (already set above) and data prepared
	uint32_t dl = 0, dh = 0; // Data low/high 32-bit words
	for (uint8_t i=0;i<4;i++) dl |= ((uint32_t)test_data[i]) << (i*8); // Pack data
	tx_mb.ul_datal = dl;
	tx_mb.ul_datah = dh;
	tx_mb.uc_length = 4; // DLC
	
	if (can_mailbox_write(CAN0, &tx_mb) != CAN_MAILBOX_TRANSFER_OK) {
		// DIAGNOSTIC: TX failed
		volatile uint32_t debug_tx_failed = 1;
		volatile uint32_t debug_tx_sr = CAN0->CAN_SR;
		return false; // Transmission failed
	}
	
	// Trigger transmission
	can_global_send_transfer_cmd(CAN0, CAN_TCR_MB1); // Send from MB1
	
	// DIAGNOSTIC: TX succeeded, check status
	volatile uint32_t debug_after_tx_sr = CAN0->CAN_SR;
	
	// Wait for message to be transmitted and looped back
	// In a real CAN network, the TX message should be seen by all nodes including self
	delay_ms(50); // Increased delay for message propagation
	
	// Check if message was received in mailbox 2
	uint32_t mb_status = can_mailbox_get_status(CAN0, 2); // Check MB2
	volatile uint32_t debug_mb_status = mb_status;
	volatile bool debug_msg_ready = (mb_status & CAN_MSR_MRDY) != 0;
	
	if (mb_status & CAN_MSR_MRDY) {
		can_mb_conf_t rx;
		rx.ul_mb_idx = 2; // Read from MB2
		if (can_mailbox_read(CAN0, &rx) == CAN_MAILBOX_TRANSFER_OK) {
			// Verify the received data matches what we sent
			uint32_t received_id = (rx.ul_id >> CAN_MID_MIDvA_Pos) & 0x7FFu;
			volatile uint32_t debug_received_id = received_id;
			volatile uint32_t debug_expected_id = test_id;
			
			// DIAGNOSTIC: Check data integrity
			volatile uint32_t debug_rx_datal = rx.ul_datal;
			volatile uint32_t debug_rx_datah = rx.ul_datah;
			
			// Verify ID and data match
			if (received_id == test_id && rx.ul_datal == dl) {
				// Restore original mailbox configuration
				// Reconfigure MB2 back to unused state
				rx_mb.ul_mb_idx = 2;
				rx_mb.uc_obj_type = CAN_MB_RX_MODE;
				rx_mb.uc_id_ver = 0;
				rx_mb.ul_id_msk = 0; // Accept all
				rx_mb.ul_id = 0;
				can_mailbox_init(CAN0, &rx_mb);
				// Re-arm RX mailbox 2
				can_mailbox_send_transfer_cmd(CAN0, &rx_mb);
				
				return true; // Test passed
			} else {
				// DIAGNOSTIC: ID or data mismatch
				volatile uint32_t debug_mismatch = 1;
			}
		} else {
			// DIAGNOSTIC: Mailbox read failed
			volatile uint32_t debug_mb_read_failed = 1;
		}
	} else {
		// DIAGNOSTIC: No message received 
		volatile uint32_t debug_no_loopback_rx = 1;
		volatile uint32_t debug_final_sr = CAN0->CAN_SR;
		
		// Check if TX was successful
		uint32_t tx_status = can_mailbox_get_status(CAN0, 1);
		volatile uint32_t debug_tx_mb_status = tx_status;
		
		// Check error counters
		volatile uint32_t debug_tx_errors = can_get_tx_error_cnt(CAN0);
		volatile uint32_t debug_rx_errors = can_get_rx_error_cnt(CAN0);
	}
	
	// Restore mailbox configuration
	rx_mb.ul_mb_idx = 2;
	rx_mb.uc_obj_type = CAN_MB_RX_MODE;
	rx_mb.uc_id_ver = 0;
	rx_mb.ul_id_msk = 0; // Accept all
	rx_mb.ul_id = 0;
	can_mailbox_init(CAN0, &rx_mb);
	// Re-arm RX mailbox 2 after restore
	can_mailbox_send_transfer_cmd(CAN0, &rx_mb);
	
	return false; // Test failed
}

void can_diagnostic_info(void)
{
	// Comprehensive CAN diagnostic information
	volatile uint32_t can_sr = CAN0->CAN_SR;
	volatile uint32_t can_mr = CAN0->CAN_MR;
	volatile uint32_t can_br = CAN0->CAN_BR;
	volatile uint32_t can_ecr = CAN0->CAN_ECR;
	
	// Mailbox statuses
	volatile uint32_t mb0_status = can_mailbox_get_status(CAN0, 0);
	volatile uint32_t mb1_status = can_mailbox_get_status(CAN0, 1);
	volatile uint32_t mb2_status = can_mailbox_get_status(CAN0, 2);
	
	// Error counters
	volatile uint32_t tx_errors = can_get_tx_error_cnt(CAN0);
	volatile uint32_t rx_errors = can_get_rx_error_cnt(CAN0);
	
	// Mailbox ready flags
	volatile bool mb0_ready = (mb0_status & CAN_MSR_MRDY) != 0;
	volatile bool mb1_ready = (mb1_status & CAN_MSR_MRDY) != 0;
	volatile bool mb2_ready = (mb2_status & CAN_MSR_MRDY) != 0;
	
	// Error conditions
	volatile bool bus_off = (can_sr & CAN_SR_BOFF) != 0;
	volatile bool error_active = (can_sr & CAN_SR_ERRA) != 0;
	volatile bool warning = (can_sr & CAN_SR_WARN) != 0;
	
	// Store all diagnostic info in volatile variables for debugging
	(void)can_sr; (void)can_mr; (void)can_br; (void)can_ecr;
	(void)mb0_status; (void)mb1_status; (void)mb2_status;
	(void)tx_errors; (void)rx_errors;
	(void)mb0_ready; (void)mb1_ready; (void)mb2_ready;
	(void)bus_off; (void)error_active; (void)warning;
}

void can_status_task(void *arg)
{
	(void)arg; // Unused
	uint32_t status_report_interval = 0;
	
	for (;;) {
		// Check CAN status periodically
		bool can_ok = can_app_get_status();
		
		// Run diagnostics every 5 seconds
		if (status_report_interval % 5 == 0) {
			can_diagnostic_info();
		}
		
		// Report status every 10 seconds (10000ms / 1000ms = 10 iterations)
		status_report_interval++;
		if (status_report_interval >= 10) {
			status_report_interval = 0;
			
			// Send status message
			uint8_t status_data[2] = {0};
			status_data[0] = can_ok ? 0x01 : 0x00; // Status byte
			status_data[1] = 0x00; // Reserved
			
			// Use the dedicated status ID
			can_app_tx(CAN_ID_STATUS, status_data, 2);
		}
		
		vTaskDelay(pdMS_TO_TICKS(1000)); // Check every second
	}
}

// Simple CAN test function to help diagnose Protocol Violation issues
bool can_app_simple_test(void)
{
	// Check if CAN controller is in a good state
	uint32_t can_sr = CAN0->CAN_SR;
	volatile uint32_t debug_simple_test_sr = can_sr;
	
	// Check for bus-off condition
	if (can_sr & CAN_SR_BOFF) {
		volatile uint32_t debug_bus_off = 1;
		return false;
	}
	
	// Check for error active state
	if (can_sr & CAN_SR_ERRA) {
		volatile uint32_t debug_error_active = 1;
		volatile uint32_t debug_tx_errors = can_get_tx_error_cnt(CAN0);
		volatile uint32_t debug_rx_errors = can_get_rx_error_cnt(CAN0);
		return false;
	}
	
	// Check if CAN controller is enabled
	if (!(CAN0->CAN_MR & CAN_MR_CANEN)) {
		volatile uint32_t debug_can_not_enabled = 1;
		return false;
	}
	
	// If we get here, CAN controller appears to be in good state
	volatile uint32_t debug_can_ok = 1;
	return true;
}