#ifndef CONF_BOARD_H
#define CONF_BOARD_H

/* ---------- Enable the blocks you’re using ---------- */
#define CONF_BOARD_SPI0
#define CONF_BOARD_TWI0
#define CONF_BOARD_CAN0
#define CONF_BOARD_MAG_SWITCH

/* ---------- Pin mapping (EDIT THESE to match your schematic) ---------- */
/* SPI0 (ADS1120) */
#define PIN_SPI0_MISO        PIO_PA12       /* e.g. PA12, Peripheral A */
#define PIN_SPI0_MOSI        PIO_PA13       /* e.g. PA13, Peripheral A */
#define PIN_SPI0_SPCK        PIO_PA14       /* e.g. PA14, Peripheral A */
#define PIN_SPI0_NPCS0       PIO_PA11       /* e.g. PA11, Peripheral A */
#define PIN_SPI0_DRDY		PIO_PA15

#define PIN_SPI0_PERIPH      PIO_PERIPH_A   /* change if your pins use B */

/* ADS1120 extra control pins (EDIT as per schematic) */
#define PIN_ADS1120_DRDY     PIO_PA15        /* input from ADC DRDY */
//#define PIN_ADS1120_START    PIO_PB1        /* output to ADC START/SYNC */
//#define PIN_ADS1120_RST      PIO_PB2        /* output to ADC RESET (if used) */

/* TWI0 (LIS2DE12) */
#define PIN_TWI0_TWD         PIO_PA3        /* TWD0 on PA3 (often Peripheral A) */
#define PIN_TWI0_TWCK        PIO_PA4        /* TWCK0 on PA4 */
#define PIN_TWI0_PERIPH      PIO_PERIPH_A

/* CAN0 */
#define PIN_CAN0_TX          PIO_PB13       /* example */
#define PIN_CAN0_RX          PIO_PB12
#define PIN_CAN0_PERIPH      PIO_PERIPH_A

/* Magnetic switch input (EDIT pin) */
#define PIN_MAG_SW           PIO_PC5


#endif /* CONF_BOARD_H */
