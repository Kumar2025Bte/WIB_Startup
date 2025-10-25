#include "encoder.h"
#include "asf.h"
#include "FreeRTOS.h"
#include "task.h"
#include "can_app.h"
#include "pio_handler.h"
#include "FreeRTOSConfig.h"

// Global encoder data structures
static encoder_data_t encoder1_data = {0};
static encoder_data_t encoder2_data = {0};

// Forward declarations
static void encoder_pioa_isr(uint32_t id, uint32_t mask);
static inline void encoder_handle_rising_on_a(encoder_data_t *enc_data, uint32_t other_level);
static inline void encoder_handle_rising_on_b(encoder_data_t *enc_data, uint32_t other_level);
static inline uint32_t get_time_ms_from_isr(void);

// CAN message IDs for encoder data
#define CAN_ID_ENCODER1_DIR_VEL    0x130u  // Encoder 1 direction and velocity
#define CAN_ID_ENCODER2_DIR_VEL    0x131u  // Encoder 2 direction and velocity
#ifndef TickType_t
typedef portTickType TickType_t; // Backward-compatible alias if TickType_t isn't defined
#endif
#ifndef pdMS_TO_TICKS
#define pdMS_TO_TICKS(ms) ((TickType_t)((ms) / portTICK_PERIOD_MS)) // Convert milliseconds to OS ticks
#endif

#ifndef portTICK_PERIOD_MS
#define portTICK_PERIOD_MS portTICK_RATE_MS // Legacy macro mapping
#endif
bool encoder_init(void)
{
    // Enable PIO clocks for encoder pins
    pmc_enable_periph_clk(ID_PIOA);
    pmc_enable_periph_clk(ID_PIOD);
    
    // Configure encoder pins as inputs with pull-up and fast deglitch filter
    // Note: Use PIO_DEGLITCH (synchronous) instead of slow-clock PIO_DEBOUNCE to avoid missing fast pulses
    pio_configure(PIOA, PIO_INPUT, ENC1_A_PIN, PIO_PULLUP | PIO_DEGLITCH);
    pio_configure(PIOA, PIO_INPUT, ENC1_B_PIN, PIO_PULLUP | PIO_DEGLITCH);
    if (ENCODER2_AVAILABLE) {
        pio_configure(PIOA, PIO_INPUT, ENC2_A_PIN, PIO_PULLUP | PIO_DEGLITCH);
        pio_configure(PIOA, PIO_INPUT, ENC2_B_PIN, PIO_PULLUP | PIO_DEGLITCH);
    }
    
    // Configure enable pins as outputs default high (active-low enable)
    pio_configure(PIOD, PIO_OUTPUT_1, ENC1_ENABLE_PIN, PIO_DEFAULT);
    pio_clear(PIOD, ENC1_ENABLE_PIN);  // Enable encoder 1 (active-low)
    
    if (ENCODER2_AVAILABLE) {
        pio_configure(PIOD, PIO_OUTPUT_1, ENC2_ENABLE_PIN, PIO_DEFAULT);
        pio_clear(PIOD, ENC2_ENABLE_PIN);  // Enable encoder 2 (active-low)
    }
    // Set up external interrupts on rising edges (X2 decoding on A and B rising)
    // Register ONE interrupt source per pin so the callback 'mask' uniquely identifies the pin.
    pio_handler_set_pin(ENC1_A_PIN, PIO_IT_RISE_EDGE, encoder_pioa_isr);
    pio_enable_pin_interrupt(ENC1_A_PIN);
    pio_handler_set_pin(ENC1_B_PIN, PIO_IT_RISE_EDGE, encoder_pioa_isr);
    pio_enable_pin_interrupt(ENC1_B_PIN);
    if (ENCODER2_AVAILABLE) {
        pio_handler_set_pin(ENC2_A_PIN, PIO_IT_RISE_EDGE, encoder_pioa_isr);
        pio_enable_pin_interrupt(ENC2_A_PIN);
        pio_handler_set_pin(ENC2_B_PIN, PIO_IT_RISE_EDGE, encoder_pioa_isr);
        pio_enable_pin_interrupt(ENC2_B_PIN);
    }
    // Keep GPIO IRQ at lowest urgency to avoid starving SysTick/CAN tasks
    pio_handler_set_priority(PIOA, PIOA_IRQn, configLIBRARY_LOWEST_INTERRUPT_PRIORITY);

    // Initialize encoder data structures
    encoder1_data.position = 0;
    encoder1_data.velocity = 0;
    encoder1_data.smoothed_velocity = 0;
    encoder1_data.direction = 0;
    encoder1_data.state_a = 0;
    encoder1_data.state_b = 0;
    encoder1_data.prev_state_a = 0;
    encoder1_data.prev_state_b = 0;
    encoder1_data.last_update_time = 0;
    encoder1_data.last_direction_change = 0;
    encoder1_data.pulse_count = 0;
    encoder1_data.velocity_window_start = 0;
    
    encoder2_data.position = 0;
    encoder2_data.velocity = 0;
    encoder2_data.smoothed_velocity = 0;
    encoder2_data.direction = 0;
    encoder2_data.state_a = 0;
    encoder2_data.state_b = 0;
    encoder2_data.prev_state_a = 0;
    encoder2_data.prev_state_b = 0;
    encoder2_data.last_update_time = 0;
    encoder2_data.last_direction_change = 0;
    encoder2_data.pulse_count = 0;
    encoder2_data.velocity_window_start = 0;
    // Initialize initial states from pins
    encoder1_data.state_a = pio_get(PIOA, PIO_TYPE_PIO_INPUT, ENC1_A_PIN) ? 1u : 0u;
    encoder1_data.state_b = pio_get(PIOA, PIO_TYPE_PIO_INPUT, ENC1_B_PIN) ? 1u : 0u;
    if (ENCODER2_AVAILABLE) {
        encoder2_data.state_a = pio_get(PIOA, PIO_TYPE_PIO_INPUT, ENC2_A_PIN) ? 1u : 0u;
        encoder2_data.state_b = pio_get(PIOA, PIO_TYPE_PIO_INPUT, ENC2_B_PIN) ? 1u : 0u;
    }

    return true;
}

// Polling no longer used; retained for compatibility (no-op)
void encoder_poll(encoder_data_t* enc_data)
{
    (void)enc_data;
}

int32_t calculate_velocity(encoder_data_t* enc_data, uint32_t current_time)
{
    if (enc_data->velocity_window_start == 0) {
        enc_data->velocity_window_start = current_time;
        return 0;
    }
    
    uint32_t time_diff = current_time - enc_data->velocity_window_start;
    if (time_diff == 0) return 0;
    
    // Snapshot and reset pulse accumulator atomically
    taskENTER_CRITICAL();
    uint32_t pulses = enc_data->pulse_count;
    enc_data->pulse_count = 0;
    taskEXIT_CRITICAL();

    // Calculate velocity in pulses per second
    int32_t velocity_pulses_per_sec = (pulses * 1000) / time_diff;
    
    // Convert from pulses per second to degrees per second
    // Formula: (pulses/sec) * (360 degrees/rev) / (pulses/rev) = degrees/sec
    // Using integer arithmetic: (velocity_pulses_per_sec * 360) / ENCODER_PULSES_PER_REV
    int32_t velocity_degrees_per_sec = (velocity_pulses_per_sec * 360) / ENCODER_PULSES_PER_REV;
    
    // Apply direction sign
    if (enc_data->direction == 2) { // Reverse
        velocity_degrees_per_sec = -velocity_degrees_per_sec;
    }
    
    return velocity_degrees_per_sec;
}

void apply_velocity_smoothing(encoder_data_t* enc_data)
{
    // Apply exponential smoothing to reduce jerky velocity changes
    float smoothing_factor = VELOCITY_SMOOTHING_FACTOR;
    enc_data->smoothed_velocity = (int32_t)(smoothing_factor * enc_data->smoothed_velocity + 
                                           (1.0f - smoothing_factor) * enc_data->velocity);
}

bool is_direction_change_allowed(encoder_data_t* enc_data, uint32_t current_time, uint8_t new_direction)
{
    // Don't allow direction changes if we're already in that direction
    if (enc_data->direction == new_direction) {
        return false;
    }
    
    // Don't allow direction changes too frequently (debouncing)
    if (current_time - enc_data->last_direction_change < DIRECTION_DEBOUNCE_MS) {
        return false;
    }
    
    // Only allow direction change if we have some velocity (not just noise)
    if (abs(enc_data->velocity) < 2) { // Minimum velocity threshold
        return false;
    }
    
    return true;
}

void encoder_task(void *arg)
{
    (void)arg; // Unused parameter
    
    // Initialize encoders
    if (!encoder_init()) {
        // Encoder initialization failed
        while(1) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
    
    // Wait a bit for encoders to stabilize
    vTaskDelay(pdMS_TO_TICKS(100));
    
    for (;;) {
        // Periodically compute velocity windows (interrupts update pulse counts and position)
        uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
        if (current_time - encoder1_data.velocity_window_start >= VELOCITY_CALC_WINDOW_MS) {
            encoder1_data.velocity = calculate_velocity(&encoder1_data, current_time);
            apply_velocity_smoothing(&encoder1_data);
            encoder1_data.velocity_window_start = current_time;
        }
        if (ENCODER2_AVAILABLE && (current_time - encoder2_data.velocity_window_start >= VELOCITY_CALC_WINDOW_MS)) {
            encoder2_data.velocity = calculate_velocity(&encoder2_data, current_time);
            apply_velocity_smoothing(&encoder2_data);
            encoder2_data.velocity_window_start = current_time;
        }

        // Send encoder 1 data over CAN
        uint8_t enc1_data[6];
        enc1_data[0] = (uint8_t)(encoder1_data.direction & 0xFF);
		if (encoder1_data.smoothed_velocity <0)
		{
			
			encoder1_data.smoothed_velocity = encoder1_data.smoothed_velocity  *(-1);
		}
		
//         enc1_data[1] = (uint8_t)(encoder1_data.smoothed_velocity & 0xFF);
//         enc1_data[2] = (uint8_t)((encoder1_data.smoothed_velocity >> 8) & 0xFF);
//         enc1_data[3] = (uint8_t)((encoder1_data.smoothed_velocity >> 16) & 0xFF);
//         enc1_data[4] = (uint8_t)((encoder1_data.smoothed_velocity >> 24) & 0xFF);
  enc1_data[4] = (uint8_t)(encoder1_data.smoothed_velocity & 0xFF);
  enc1_data[3] = (uint8_t)((encoder1_data.smoothed_velocity >> 8) & 0xFF);
  enc1_data[2] = (uint8_t)((encoder1_data.smoothed_velocity >> 16) & 0xFF);
  enc1_data[1] = (uint8_t)((encoder1_data.smoothed_velocity >> 24) & 0xFF);

        enc1_data[5] = (uint8_t)(encoder1_data.position & 0xFF);
        
        can_app_tx(CAN_ID_ENCODER1_DIR_VEL, enc1_data, 6);
        
        // Debug: Store encoder data for debugging
        volatile uint32_t debug_enc1_direction = encoder1_data.direction;
        volatile uint32_t debug_enc1_velocity = encoder1_data.velocity;
        volatile uint32_t debug_enc1_smoothed_velocity = encoder1_data.smoothed_velocity;
        volatile uint32_t debug_enc1_position = encoder1_data.position;
        
        // Send encoder 2 data over CAN (only if available)
        if (ENCODER2_AVAILABLE) {
            uint8_t enc2_data[6];
            enc2_data[0] = (uint8_t)(encoder2_data.direction & 0xFF);
			if (encoder2_data.smoothed_velocity <0)
		{
			
			encoder2_data.smoothed_velocity = encoder2_data.smoothed_velocity  *(-1);
		}
//             enc2_data[1] = (uint8_t)(encoder2_data.smoothed_velocity & 0xFF);
//             enc2_data[2] = (uint8_t)((encoder2_data.smoothed_velocity >> 8) & 0xFF);
//             enc2_data[3] = (uint8_t)((encoder2_data.smoothed_velocity >> 16) & 0xFF);
//             enc2_data[4] = (uint8_t)((encoder2_data.smoothed_velocity >> 24) & 0xFF);

  enc2_data[4] = (uint8_t)(encoder2_data.smoothed_velocity & 0xFF);
  enc2_data[3] = (uint8_t)((encoder2_data.smoothed_velocity >> 8) & 0xFF);
  enc2_data[2] = (uint8_t)((encoder2_data.smoothed_velocity >> 16) & 0xFF);
  enc2_data[1] = (uint8_t)((encoder2_data.smoothed_velocity >> 24) & 0xFF);
  
            enc2_data[5] = (uint8_t)(encoder2_data.position & 0xFF);
            
            can_app_tx(CAN_ID_ENCODER2_DIR_VEL, enc2_data, 6);
        }
        
        // Task period
        vTaskDelay(pdMS_TO_TICKS(ENCODER_POLLING_RATE_MS));
    }
}

// ===== Interrupt-driven quadrature decoding (X2 on rising edges) =====
static inline uint32_t get_time_ms_from_isr(void)
{
    // Avoid calling FreeRTOS API from GPIO ISR; leave as no-op for now
    return 0u;
}

static inline void encoder_handle_rising_on_a(encoder_data_t *enc_data, uint32_t other_level)
{
    // A rose; if B == 0 -> forward, else reverse (matches prior software decoding conventions)
    if (other_level == 0) {
        enc_data->position++;
        enc_data->direction = 1u;
    } else {
        enc_data->position--;
        enc_data->direction = 2u;
    }
    enc_data->pulse_count++;
    // Timestamping omitted to keep ISR minimal
}

static inline void encoder_handle_rising_on_b(encoder_data_t *enc_data, uint32_t other_level)
{
    // B rose; if A == 0 -> reverse, else forward (matches prior software decoding conventions)
    if (other_level == 0) {
        enc_data->position--;
        enc_data->direction = 2u;
    } else {
        enc_data->position++;
        enc_data->direction = 1u;
    }
    enc_data->pulse_count++;
    // Timestamping omitted to keep ISR minimal
}

static void encoder_pioa_isr(uint32_t id, uint32_t mask)
{
    if (id != ID_PIOA) {
        return;
    }

    // Each registration is per-pin, so 'mask' equals the specific pin that triggered.
    switch (mask) {
        case ENC1_A_PIN: {
            uint32_t b = pio_get(PIOA, PIO_TYPE_PIO_INPUT, ENC1_B_PIN) ? 1u : 0u;
            encoder_handle_rising_on_a(&encoder1_data, b);
            break;
        }
        case ENC1_B_PIN: {
            uint32_t a = pio_get(PIOA, PIO_TYPE_PIO_INPUT, ENC1_A_PIN) ? 1u : 0u;
            encoder_handle_rising_on_b(&encoder1_data, a);
            break;
        }
        case ENC2_A_PIN: {
            if (ENCODER2_AVAILABLE) {
                uint32_t b2 = pio_get(PIOA, PIO_TYPE_PIO_INPUT, ENC2_B_PIN) ? 1u : 0u;
                encoder_handle_rising_on_a(&encoder2_data, b2);
            }
            break;
        }
        case ENC2_B_PIN: {
            if (ENCODER2_AVAILABLE) {
                uint32_t a2 = pio_get(PIOA, PIO_TYPE_PIO_INPUT, ENC2_A_PIN) ? 1u : 0u;
                encoder_handle_rising_on_b(&encoder2_data, a2);
            }
            break;
        }
        default:
            break;
    }
}