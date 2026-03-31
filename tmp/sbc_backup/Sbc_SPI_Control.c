/**
 * @file Sbc_SPI_Control.c
 * @brief SBC (TLE9263) Mode and SPI Communication Implementation.
 */

#include "Spi.h"
#include "Spi_PBcfg.h"
#include "Sbc_SPI_Control.h" 

/* Internal SPI Buffers */
static uint16 Sbc_SpiTxBuffer;
static uint16 Sbc_SpiRxBuffer;

/* Command Trigger (Manual control in Developer Mode) */
volatile uint8 SPI_Command = 0;

/* Error Status Tracker */
volatile SBC_ErrorCode g_SbcError = {SBC_M_S_CTRL, 0, 0};

/* Connection State */
static boolean g_sbcInitialized = FALSE;

/* Internal Helpers */
static uint16 Sbc_Transfer16(uint16 frame);
static uint8  Sbc_CalcWDParity(uint8 wd_val);

/**
 * @brief Calculate Handshake Parity for Watchdog register bit 7.
 */
static uint8 Sbc_CalcWDParity(uint8 wd_val) {
    uint8 parity = wd_val;
    parity ^= (parity >> 4U);
    parity ^= (parity >> 2U);
    parity ^= (parity >> 1U);
    
    if ((parity & 0x01U) != 0U) {
        return (uint8)(wd_val | 0x80U); /* Set even parity bit */
    }
    return (uint8)(wd_val & 0x7FU);
}

/**
 * @brief Initial SBC handshake and start-up sequence.
 */
boolean Sbc_Init(void) {
    uint16 status;
    uint8 wd_val;

    /* Transition from Init to Normal Mode by writing WD_CTRL */
    wd_val = (uint8)WD_1000MS | (uint8)((uint8)TIME_OUT_WD << SBC_BITPOSITION(WD_WIN_MASK));
    wd_val = Sbc_CalcWDParity(wd_val);
    
    uint16 wd_frame = (uint16)(((uint16)wd_val << 8U) | (uint16)((uint8)SBC_WD_CTRL & 0x7FU) | 0x80U);
    status = Sbc_Transfer16(wd_frame);

    if (status == 0x0000U) {
        return FALSE; /* SBC Unresponsive or Power-off */
    }

    g_sbcInitialized = TRUE;
    return TRUE;
}

/**
 * @brief Perform 16-bit SPI sequence (ASIL-compliant status check).
 */
static uint16 Sbc_Transfer16(uint16 frame) {
    Sbc_SpiTxBuffer = frame;
    Sbc_SpiRxBuffer = 0;

    /* Transmit and Wait for synchronous completion */
    Std_ReturnType ret = Spi_SyncTransmit(SpiConf_SpiSequence_SpiSequence_SBC);

    if (ret == E_OK) {
        /* Read status and data content */
        (void)Spi_ReadIB(SpiConf_SpiChannel_SpiChannel_SBC_RX, &Sbc_SpiRxBuffer);
        return Sbc_SpiRxBuffer;
    }
    return 0x0000;
}

/**
 * @brief Service the TLE9263 standard watchdog.
 */
void Sbc_ServiceWatchdog(void) {
    if (g_sbcInitialized == TRUE) {
        uint8 wd_val = (uint8)WD_1000MS | (uint8)((uint8)TIME_OUT_WD << SBC_BITPOSITION(WD_WIN_MASK));
        wd_val = Sbc_CalcWDParity(wd_val);
        (void)Sbc_WriteReg(SBC_WD_CTRL, wd_val);
    }
}

boolean Sbc_IsInitialized(void) {
    return g_sbcInitialized;
}

/**
 * @brief Manual SPI Command Interface for Debugging.
 */
void Sbc_SPI_Test(void) {
    /* SPI_Command Handlers
     * 1: Sbc_Init() (Already handled in TaskScheduler)
     * 2: Read Master-Slave Control
     * 10: Toggle LED via GPIO (If mapped)
     */
    if (SPI_Command == 2) {
        (void)Sbc_ReadReg(SBC_M_S_CTRL);
        SPI_Command = 0;
    }
}

/* Base SPI Accessors */
uint16 Sbc_ReadReg(sbc_register_t addr) {
    uint16 frame = (uint16)((uint8)addr & 0x7FU);
    return Sbc_Transfer16(frame);
}

SBC_ErrorCode Sbc_WriteReg(sbc_register_t addr, uint8 data) {
    uint16 frame = (uint16)(((uint16)data << 8U) | (uint16)((uint8)addr & 0x7FU) | 0x80U);
    uint16 resp = Sbc_Transfer16(frame);
    
    SBC_ErrorCode err = {addr, 0, data};
    uint8 actualRead = (uint8)(resp >> 8U);
    
    /* Optional: Implement write-verify logic here */
    return err;
}