#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CAN_BAUD_KBPS          500u // CAN bus bitrate in kbps

/* Default CAN IDs */
#define CAN_ID_LOADCELL        0x120u // ID for load cell measurements
#define CAN_ID_ACCELEROMETER   0x121u // ID for accelerometer XYZ bytes
#define CAN_ID_TEMPERATURE     0x122u // ID for temperature readings
#define CAN_ID_TOOLTYPE        0x123u // ID for tool type status
#define CAN_ID_ENCODER1_DIR_VEL 0x130u // ID for encoder 1 direction and velocity
#define CAN_ID_ENCODER2_DIR_VEL 0x131u // ID for encoder 2 direction and velocity
#define CAN_ID_STATUS          0x200u // ID for system status messages
#define CAN_ID_POT_COMMAND     0x220u // ID for potentiometer control/telemetry

bool can_app_init(void); // Initialize CAN controller and RX mailbox
bool can_app_tx(uint32_t id, const uint8_t *data, uint8_t len); // Transmit a CAN frame
void can_rx_task(void *arg); // FreeRTOS task for CAN RX and command handling
bool can_app_get_status(void); // Get CAN controller status
bool can_app_test_loopback(void); // Test CAN communication with loopback mode
void can_status_task(void *arg); // FreeRTOS task for periodic CAN status monitoring
void can_diagnostic_info(void); // Comprehensive CAN diagnostic information
bool can_app_simple_test(void); // Simple CAN controller state test for debugging
bool can_verify_bitrate(uint32_t expected_kbps); // Verify CAN bit rate configuration

#ifdef __cplusplus
}
#endif