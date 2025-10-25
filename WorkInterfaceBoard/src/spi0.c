#include "spi0.h"
#include "sam4e.h"
#include "pmc.h"
#include "pio.h"
#include "spi.h"
#include "sysclk.h"

#include "sam4e.h"

/* Assumed SPI0 on Peripheral A:
 *  PA12 = SPI0_MISO (A)
 *  PA13 = SPI0_MOSI (A)
 *  PA14 = SPI0_SPCK (A)
 *  PA11 = SPI0_NPCS0 (A)
 */


#define SPI0_CS_PIN   (1u << 11)   // PA11 for NPCS0

// Configure PA11 as output for manual CS control
void spi0_cs_init(void)
{
	PMC->PMC_PCER0 |= (1u << ID_PIOA);        // Enable PIOA clock
	PIOA->PIO_PER   = SPI0_CS_PIN;            // Enable PIO control
	PIOA->PIO_OER   = SPI0_CS_PIN;            // Set as output
	PIOA->PIO_SODR  = SPI0_CS_PIN;            // Drive high (inactive)
}

void spi0_cs_low(void)
{
	PIOA->PIO_CODR = SPI0_CS_PIN;             // Drive low (active)
}

void spi0_cs_high(void)
{
	PIOA->PIO_SODR = SPI0_CS_PIN;             // Drive high (inactive)
}

static void spi0_config_pins(void)
{
    const uint32_t spi_periph_mask = (1u << 12) | (1u << 13) | (1u << 14);

    /* Disable PIO control on these pins (hand over to peripheral) */
    PIOA->PIO_PDR = spi_periph_mask;

    /* Select Peripheral A: ABCDSR[1:0] = 00 for these pins */
    PIOA->PIO_ABCDSR[0] &= ~spi_periph_mask;  // bit = 0
    PIOA->PIO_ABCDSR[1] &= ~spi_periph_mask;  // bit = 0

    /* Optional: disable internal pull-ups/pull-downs if not needed */
    PIOA->PIO_PUDR = spi_periph_mask;

    /* Optional: disable interrupts on those pins */
    PIOA->PIO_IDR = spi_periph_mask;
}


static inline void _mode_to_polarity_phase(uint8_t mode, uint32_t *cpol, uint32_t *ncpha)
{
    switch (mode & 0x3) {
        case 0: *cpol = 0; *ncpha = 1; break; // SPI mode 0: CPOL=0, NCPHA=1
        case 1: *cpol = 0; *ncpha = 0; break; // SPI mode 1: CPOL=0, NCPHA=0
        case 2: *cpol = 1; *ncpha = 1; break; // SPI mode 2: CPOL=1, NCPHA=1
        default:*cpol = 1; *ncpha = 0; break; // SPI mode 3: CPOL=1, NCPHA=0
    }
}

void spi0_init(uint32_t bitrate_hz, bool lsbfirst)
{
	spi0_config_pins(); // Configure SPI0 pins for peripheral control

	// Enable peripheral clock for SPI
	PMC->PMC_PCER0 = (1u << ID_SPI);

	// Reset and configure SPI
	SPI->SPI_CR = SPI_CR_SWRST; // Software reset
	SPI->SPI_CR = SPI_CR_SPIEN; // Enable SPI

	// Master mode, fixed NPCS = 0
	SPI->SPI_MR = SPI_MR_MSTR | SPI_MR_MODFDIS | SPI_MR_PCS(0);

	// Configure Chip Select 0 for ADS1120: Mode 1 (CPOL=0, NCPHA=0)
	uint32_t scbr = SystemCoreClock / bitrate_hz; // Baud rate divider
	if (scbr < 1) scbr = 1;
	if (scbr > 255) scbr = 255;

	SPI->SPI_CSR[0] = SPI_CSR_BITS_8_BIT   // 8-bit transfer
	| SPI_CSR_SCBR(scbr);  // Clock divider

/*	if (lsbfirst)
	SPI->SPI_CSR[0] |= SPI_CSR_LSBFRST;
	*/
}

uint8_t spi0_transfer(uint8_t tx)
{
    uint16_t r;
    while ((spi_read_status(SPI) & SPI_SR_TDRE) == 0) { } // Wait for TX ready
    spi_write(SPI, tx, 0, 0); // Write byte to transmit
    while ((spi_read_status(SPI) & SPI_SR_RDRF) == 0) { } // Wait for RX ready
    spi_read(SPI, &r, 0); // Read received data
    return (uint8_t)r; // Return low byte
}

void spi0_transfer_buffer(const uint8_t *tx, uint8_t *rx, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        uint8_t t = tx ? tx[i] : 0xFF; // Default filler if no TX buffer
        uint8_t r = spi0_transfer(t); // Transfer one byte
        if (rx) rx[i] = r; // Store if RX buffer provided
    }
}

void spi0_test_pulse(void)
{
    spi0_cs_low(); // Assert CS
    (void)spi0_transfer(0xAA); // Send test byte
    spi0_cs_high(); // Deassert CS
}
