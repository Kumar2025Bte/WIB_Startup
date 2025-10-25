/**
 * \file
 *
 * \brief Component description for Power Management Controller (PMC) - SAM4E series
 *
 * Copyright (c) 2011-2018 Microchip Technology Inc.
 */

#ifndef _SAM4E_PMC_COMPONENT_
#define _SAM4E_PMC_COMPONENT_

/* -------- PMC_SCER : (PMC Offset: 0x00) System Clock Enable Register -------- */
#define PMC_SCER_UHP           (0x1u << 6) /**< USB Host Port Clock Enable */
#define PMC_SCER_UDP           (0x1u << 7) /**< USB Device Port Clock Enable */
#define PMC_SCER_PCK0          (0x1u << 8) /**< Programmable Clock 0 Enable */
#define PMC_SCER_PCK1          (0x1u << 9) /**< Programmable Clock 1 Enable */
#define PMC_SCER_PCK2          (0x1u << 10) /**< Programmable Clock 2 Enable */

/* -------- PMC_SCDR : (PMC Offset: 0x04) System Clock Disable Register -------- */
#define PMC_SCDR_UHP           (0x1u << 6) /**< USB Host Port Clock Disable */
#define PMC_SCDR_UDP           (0x1u << 7) /**< USB Device Port Clock Disable */
#define PMC_SCDR_PCK0          (0x1u << 8) /**< Programmable Clock 0 Disable */
#define PMC_SCDR_PCK1          (0x1u << 9) /**< Programmable Clock 1 Disable */
#define PMC_SCDR_PCK2          (0x1u << 10) /**< Programmable Clock 2 Disable */

/* -------- PMC_SCSR : (PMC Offset: 0x08) System Clock Status Register -------- */
#define PMC_SCSR_UHP           (0x1u << 6) /**< USB Host Port Clock Status */
#define PMC_SCSR_UDP           (0x1u << 7) /**< USB Device Port Clock Status */
#define PMC_SCSR_PCK0          (0x1u << 8) /**< Programmable Clock 0 Status */
#define PMC_SCSR_PCK1          (0x1u << 9) /**< Programmable Clock 1 Status */
#define PMC_SCSR_PCK2          (0x1u << 10) /**< Programmable Clock 2 Status */

/* ... Other registers here ... */

/* -------- PMC_FSMR : (PMC Offset: 0x30) Fast Start-up Mode Register -------- */
#define PMC_FSMR_FLPM_Pos                   16
#define PMC_FSMR_FLPM_Msk                   (0x3u << PMC_FSMR_FLPM_Pos) /**< Flash Low-power Mode */
#define PMC_FSMR_FLPM_FLASH_IDLE            (0x0u << 16) /**< Flash idle in Wait mode */
#define PMC_FSMR_FLPM_FLASH_STANDBY         (0x1u << 16) /**< Flash in standby mode in Wait mode */
#define PMC_FSMR_FLPM_FLASH_DEEP_POWERDOWN  (0x2u << 16) /**< Flash in deep powerdown in Wait mode */

/* Compatibility aliases expected by ASF drivers */
#define PMC_WAIT_MODE_FLASH_IDLE            PMC_FSMR_FLPM_FLASH_IDLE
#define PMC_WAIT_MODE_FLASH_STANDBY         PMC_FSMR_FLPM_FLASH_STANDBY
#define PMC_WAIT_MODE_FLASH_DEEP_POWERDOWN  PMC_FSMR_FLPM_FLASH_DEEP_POWERDOWN

/* -------- PMC_FSPR : (PMC Offset: 0x34) Fast Start-up Polarity Register -------- */
#define PMC_FSPR_FSTP_Pos    0
#define PMC_FSPR_FSTP_Msk    (0xffffffffu << PMC_FSPR_FSTP_Pos)

/* -------- PMC_WPMR : (PMC Offset: 0xE4) Write Protection Mode Register -------- */
#define PMC_WPMR_WPEN        (0x1u << 0) /**< Write Protection Enable */
#define PMC_WPMR_WPKEY_Pos   8
#define PMC_WPMR_WPKEY_Msk   (0xFFFFFFu << PMC_WPMR_WPKEY_Pos)
#define PMC_WPMR_WPKEY_PASSWD (0x504D43u << PMC_WPMR_WPKEY_Pos) /**< Password 'PMC' */

/* -------- PMC_WPSR : (PMC Offset: 0xE8) Write Protection Status Register -------- */
#define PMC_WPSR_WPVS        (0x1u << 0) /**< Write Protection Violation Status */
#define PMC_WPSR_WPVSRC_Pos  8
#define PMC_WPSR_WPVSRC_Msk  (0xFFFFu << PMC_WPSR_WPVSRC_Pos)

/* ========== Instance definition for PMC peripheral ========== */
#define ID_PMC 5 /**< PMC instance ID */

#endif /* _SAM4E_PMC_COMPONENT_ */
