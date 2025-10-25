#pragma once
#include <stdint.h>

#define SPI0_CS_PIO   PIOD
#define SPI0_CS_ID    ID_PIOD
#define SPI0_CS_PIN   20
#define SPI0_CS_MASK  (1u << SPI0_CS_PIN)

void spi0_init(uint32_t bitrate_hz, uint8_t mode);
static inline void spi0_cs_low(void)  { SPI0_CS_PIO->PIO_CODR = SPI0_CS_MASK; }
static inline void spi0_cs_high(void) { SPI0_CS_PIO->PIO_SODR = SPI0_CS_MASK; }
uint8_t spi0_transfer(uint8_t tx);
void spi0_transfer_buffer(const uint8_t *tx, uint8_t *rx, uint32_t len);
void spi0_test_pulse(void);
