/*
 * component_twi.h
 *
 * Created: 13-08-2025 22:48:18
 *  Author: conta
 */ 
/**
 * \file
 *
 * \brief Component description for TWI
 *
 * Copyright (c) 2012-2018 Atmel Corporation.
 *
 */

#ifndef _SAM4E_TWI_COMPONENT_
#define _SAM4E_TWI_COMPONENT_

/* -------- TWI_CR : (TWI Offset: 0x00) Control Register -------- */
typedef struct {
  __O  uint32_t TWI_CR;        /**< \brief (Twi Offset: 0x00) Control Register */
  __IO uint32_t TWI_MMR;       /**< \brief (Twi Offset: 0x04) Master Mode Register */
  __IO uint32_t TWI_SMR;       /**< \brief (Twi Offset: 0x08) Slave Mode Register */
  __IO uint32_t TWI_IADR;      /**< \brief (Twi Offset: 0x0C) Internal Address Register */
  __IO uint32_t TWI_CWGR;      /**< \brief (Twi Offset: 0x10) Clock Waveform Generator Register */
       uint32_t Reserved1[3];
  __I  uint32_t TWI_SR;        /**< \brief (Twi Offset: 0x20) Status Register */
  __O  uint32_t TWI_IER;       /**< \brief (Twi Offset: 0x24) Interrupt Enable Register */
  __O  uint32_t TWI_IDR;       /**< \brief (Twi Offset: 0x28) Interrupt Disable Register */
  __I  uint32_t TWI_IMR;       /**< \brief (Twi Offset: 0x2C) Interrupt Mask Register */
  __I  uint32_t TWI_RHR;       /**< \brief (Twi Offset: 0x30) Receive Holding Register */
  __O  uint32_t TWI_THR;       /**< \brief (Twi Offset: 0x34) Transmit Holding Register */
       uint32_t Reserved2[50];
  __IO uint32_t TWI_RPR;       /**< \brief (Twi Offset: 0x100) Receive Pointer Register */
  __IO uint32_t TWI_RCR;       /**< \brief (Twi Offset: 0x104) Receive Counter Register */
  __IO uint32_t TWI_TPR;       /**< \brief (Twi Offset: 0x108) Transmit Pointer Register */
  __IO uint32_t TWI_TCR;       /**< \brief (Twi Offset: 0x10C) Transmit Counter Register */
  __IO uint32_t TWI_RNPR;      /**< \brief (Twi Offset: 0x110) Receive Next Pointer Register */
  __IO uint32_t TWI_RNCR;      /**< \brief (Twi Offset: 0x114) Receive Next Counter Register */
  __IO uint32_t TWI_TNPR;      /**< \brief (Twi Offset: 0x118) Transmit Next Pointer Register */
  __IO uint32_t TWI_TNCR;      /**< \brief (Twi Offset: 0x11C) Transmit Next Counter Register */
  __O  uint32_t TWI_PTCR;      /**< \brief (Twi Offset: 0x120) Transfer Control Register */
  __I  uint32_t TWI_PTSR;      /**< \brief (Twi Offset: 0x124) Transfer Status Register */
       uint32_t Reserved3[39];
  __IO uint32_t TWI_WPMR;      /**< \brief (Twi Offset: 0xE4) Write Protection Mode Register */
  __I  uint32_t TWI_WPSR;      /**< \brief (Twi Offset: 0xE8) Write Protection Status Register */
} Twi;

#endif /* _SAM4E_TWI_COMPONENT_ */
