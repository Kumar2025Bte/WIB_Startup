#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Encoder configuration
#define ENCODER_POLLING_RATE_MS    10    // Polling rate in milliseconds
#define VELOCITY_CALC_WINDOW_MS    100   // Time window for velocity calculation in ms

// Shaft encoder specifications
#define SHAFT_ENCODER_POLES				122				//from datasheet
#define SHAFT_ENCODER_INTERPOLATION		64				//from datasheet
#define SHAFT_ENCODER_COUNTS_PER_REV	((SHAFT_ENCODER_POLES * SHAFT_ENCODER_INTERPOLATION)/4)
#define ENCODER_PULSES_PER_REV     SHAFT_ENCODER_COUNTS_PER_REV

#define VELOCITY_SMOOTHING_FACTOR   0//0.7f  // Smoothing factor for velocity (0.0 = no smoothing, 1.0 = no change)
#define DIRECTION_DEBOUNCE_MS      0    // Minimum time between direction changes in ms

// Encoder pin definitions based on pinconfig_workhead_interface_pinconfig.csv
// Note: PA15 is used for SPI DRDY, so ENC2_A is not available
#define ENC1_A_PIN                 PIO_PA5
#define ENC1_B_PIN                 PIO_PA1
#define ENC1_ENABLE_PIN            PIO_PD17
#define ENC2_A_PIN                 PIO_PA15  // ENC2_A available (SPI not used)
#define ENC2_B_PIN                 PIO_PA16
#define ENC2_ENABLE_PIN            PIO_PD27

// Encoder availability flags
#define ENCODER1_AVAILABLE         1
#define ENCODER2_AVAILABLE         1

// Encoder data structure
typedef struct {
    uint32_t position;        // Current position (pulses)
    int32_t velocity;         // Current velocity (degrees per second)
    int32_t smoothed_velocity; // Smoothed velocity for transmission (degrees per second)
    uint8_t direction;        // Direction: 0=stopped, 1=forward, 2=reverse
    uint8_t state_a;          // Current state of A channel
    uint8_t state_b;          // Current state of B channel
    uint8_t prev_state_a;     // Previous state of A channel
    uint8_t prev_state_b;     // Previous state of B channel
    uint32_t last_update_time; // Last update time in ms
    uint32_t last_direction_change; // Time of last direction change
    uint32_t pulse_count;     // Total pulse count for velocity calculation
    uint32_t velocity_window_start; // Start time of velocity calculation window
} encoder_data_t;

// Function prototypes
bool encoder_init(void);
void encoder_poll(encoder_data_t* enc_data);
void encoder_task(void *arg);
int32_t calculate_velocity(encoder_data_t* enc_data, uint32_t current_time);
void apply_velocity_smoothing(encoder_data_t* enc_data);
bool is_direction_change_allowed(encoder_data_t* enc_data, uint32_t current_time, uint8_t new_direction);

#ifdef __cplusplus
}
#endif