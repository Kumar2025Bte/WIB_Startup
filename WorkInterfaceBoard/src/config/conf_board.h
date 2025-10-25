/**
 * \file
 *
 * \brief User board configuration template
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#ifndef CONF_BOARD_H
#define CONF_BOARD_H
#define PIN_SPI_MISO     PIO_PA12_IDX
#define PIN_SPI_MOSI     PIO_PA13_IDX
#define PIN_SPI_SCK      PIO_PA14_IDX
#define PIN_SPI_CS_ADC   PIO_PA11_IDX
#define PIN_SPI_DRDY     PIO_PA15_IDX

#define PIN_I2C_SCL      PIO_PA4_IDX
#define PIN_I2C_SDA      PIO_PA3_IDX
#define PIN_INT1_ACC     PIO_PD28_IDX
#define PIN_INT2_ACC     PIO_PD17_IDX

#define PIN_CAN_TX       PIO_PB2_IDX
#define PIN_CAN_RX       PIO_PB3_IDX

#define PIN_TOOL_SENSOR  PIO_PD21_IDX

#endif // CONF_BOARD_H
