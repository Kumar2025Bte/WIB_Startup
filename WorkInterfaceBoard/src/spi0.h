// spi0.h
#ifndef SPI0_H
#define SPI0_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
	#endif

	void    spi0_init(uint32_t bitrate_hz, bool lsbfirst); // Initialize SPI0 with target bitrate
	uint8_t spi0_transfer(uint8_t byte); // Transfer one byte and return received byte
	void    spi0_select(void); // (Optional) Select device (may be unimplemented)
	void    spi0_deselect(void); // (Optional) Deselect device (may be unimplemented)
	void    spi0_cs_init(void); // Initialize manual chip-select GPIO
	void    spi0_cs_low(void); // Drive CS low (active)
	void    spi0_cs_high(void); // Drive CS high (inactive)

	#ifdef __cplusplus
}
#endif

#endif // SPI0_H
