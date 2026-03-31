/**
 * @file Sbc_SPI_Control.c
 * @brief TLE9263-3BQX SBC Mode Control Implementation (SPI based).
 *
 * Target: AURIX TC364 + TLE9263-3BQX
 * 
 * [SPI Hardware Specification]
 * - Interface: QSPI3 (Master)
 * - Frequency: 1.0 MHz
 * - Pin Mapping:
 *   - SCLK: P22.0 (ALT3)
 *   - MRST: P22.1 (Input)
 *   - MTSR: P22.2 (ALT3)
 *   - CS:   P22.3 (SLS3, ALT3)
 * - SPI Mode: Mode 1 (CPOL=0, CPHA=1)
 * - Bit Order: LSB First
 * 
 * [SPI Protocol Specification]
 * - Frame Length: 16-bit
 * - TX Frame [0:6] Address, [7] R/W, [8:15] Data
 * - RX Frame [0:7] Status Word, [8:15] Data Content
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
 * @brief Calculate Even Parity bit for Watchdog Register (bit 7).
 * @param wd_val 7-bit watchdog configuration value.
 * @return 8-bit value with parity bit at MSB.
 */
static uint8 Sbc_CalcWDParity(uint8 wd_val) {
    uint8 parity = wd_val;
    parity ^= (parity >> 4U);
    parity ^= (parity >> 2U);
    parity ^= (parity >> 1U);
    
    if ((parity & 0x01U) != 0U) {
        /* Set bit 7 (MSB) to achieve even parity */
        return (uint8)(wd_val | 0x80U);
    }
    return (uint8)(wd_val & 0x7FU);
}

/**
 * @brief Initialize SBC Control (Safe Start-up Sequence)
 * @return TRUE if initialization succeeded, FALSE otherwise.
 */
boolean Sbc_Init(void) {
    uint16 status;
    uint8 wd_val;

    /* [Step 0] Safe Start-up: Perform WD Trigger as the FIRST SPI Command 
     * Recommended by datasheet to transition from Init -> Normal mode immediately.
     */
    wd_val = (uint8)WD_1000MS | (uint8)((uint8)TIME_OUT_WD << SBC_BITPOSITION(WD_WIN_MASK));
    wd_val = Sbc_CalcWDParity(wd_val);
    
    uint16 wd_frame = (uint16)(((uint16)wd_val << 8U) | (uint16)((uint8)SBC_WD_CTRL & 0x7FU) | 0x80U);
    status = Sbc_Transfer16(wd_frame);

    if (status == 0x0000U) {
        /* SBC not responsive or power-off */
        return FALSE; 
    }

    /* SBC detected! Normal mode transition confirmed by Step 0. */

    /* [Step 1] Clear all pending status flags (Read & Clear) 
     * Necessary to clear POR (Power-On Reset) to track subsequent errors.
     */
    (void)Sbc_ReadAndClearReg(SBC_DEV_STAT);
    (void)Sbc_ReadAndClearReg(SBC_SUP_STAT_2);
    (void)Sbc_ReadAndClearReg(SBC_SUP_STAT_1);
    (void)Sbc_ReadAndClearReg(SBC_THERM_STAT);

    /* [Step 2] Configure Hardware Control Register (SBC_HW_CTRL) */
    (void)Sbc_WriteRegField(SBC_HW_CTRL, CFG_MASK, (uint8)SBC_CFG1_RESTART_FAILSAFE_1WDFAIL);

    /* [Step 3] Configure Bus Control 1 (SBC_BUS_CTRL_1) */
    (void)Sbc_WriteRegField(SBC_BUS_CTRL_1, CAN_MASK, (uint8)CAN_WAKECAPABLE_SWK);
    (void)Sbc_WriteRegField(SBC_BUS_CTRL_1, LIN1_MASK, (uint8)LIN_OFF);

    /* [Step 4] Final Transition Confirmation (SBC_M_S_CTRL) */
    (void)Sbc_WriteRegField(SBC_M_S_CTRL, MODE_MASK, (uint8)SBC_NORMAL_MODE);
    (void)Sbc_WriteRegField(SBC_M_S_CTRL, VCC2_ON_MASK, (uint8)VCC2_OFF);
    (void)Sbc_WriteRegField(SBC_M_S_CTRL, VCC3_ON_MASK, (uint8)VCC3_OFF);

    g_sbcInitialized = TRUE;
    return TRUE;
}

/**
 * @brief Reports if SBC has been successfully initialized.
 */
boolean Sbc_IsInitialized(void) {
    return g_sbcInitialized;
}

/**
 * @brief Service (Trigger) SBC Watchdog.
 */
void Sbc_ServiceWatchdog(void) {
    uint8 wd_val = (uint8)WD_1000MS | (uint8)((uint8)TIME_OUT_WD << SBC_BITPOSITION(WD_WIN_MASK));
    wd_val = Sbc_CalcWDParity(wd_val);
    (void)Sbc_WriteReg(SBC_WD_CTRL, wd_val);
}

/**
 * @brief Perform 16-bit SPI sequence (ASIL-compliant).
 */
static uint16 Sbc_Transfer16(uint16 frame) {
    Sbc_SpiTxBuffer = frame;
    (void)Spi_WriteIB(SpiConf_SpiChannel_Ch_Sbc_Data, (Spi_DataBufferType*)&Sbc_SpiTxBuffer);
    (void)Spi_SyncTransmit(SpiConf_SpiSequence_Seq_Sbc_Init);
    (void)Spi_ReadIB(SpiConf_SpiChannel_Ch_Sbc_Data, (Spi_DataBufferType*)&Sbc_SpiRxBuffer);
    return Sbc_SpiRxBuffer;
}

/** @brief Read & Clear (Status registers only) */
uint16 Sbc_ReadAndClearReg(sbc_register_t addr) {
    uint16 frame = (uint16)((uint8)addr | 0x80U);
    return Sbc_Transfer16(frame);
}

/** @brief Read (Returns 16-bit Status + Data) */
uint16 Sbc_ReadReg(sbc_register_t addr) {
    uint16 frame = (uint16)((uint8)addr & 0x7FU);
    return Sbc_Transfer16(frame);
}

/** @brief Read specific bit field */
uint8 Sbc_ReadRegField(sbc_register_t addr, sbc_bit_mask_t mask) {
    uint8 data = (uint8)((Sbc_ReadReg(addr) >> 8U) & 0xFFU);
    return (uint8)((data & (uint8)mask) >> SBC_BITPOSITION(mask));
}

/** @brief Write byte and verify (Write-Read-Compare) */
SBC_ErrorCode Sbc_WriteReg(sbc_register_t addr, uint8 data) {
    SBC_ErrorCode errCode;
    uint16 frame = (uint16)(((uint16)data << 8U) | (uint16)((uint8)addr & 0x7FU) | 0x80U);
    
    (void)Sbc_Transfer16(frame);
    
    uint8 actual = (uint8)((Sbc_ReadReg(addr) >> 8U) & 0xFFU);
    
    errCode.SBC_Register = addr;
    errCode.expectedValue = data;
    errCode.flippedBitsMask = actual ^ data;
    
    g_SbcError = errCode;
    return errCode;
}

/**
 * @brief Write to a specific bit field in a register.
 */
SBC_ErrorCode Sbc_WriteRegField(sbc_register_t addr, sbc_bit_mask_t mask, uint8 val) {
    uint8 currentData = (uint8)((Sbc_ReadReg(addr) >> 8U) & 0xFFU);
    uint8 newData = currentData & (uint8)(~(uint8)mask);
    newData |= (uint8)((val << SBC_BITPOSITION(mask)) & (uint8)mask);
    return Sbc_WriteReg(addr, newData);
}

/** @brief Manual SPI Command Interface for Debugging. */
void Sbc_SPI_Test(void) {
    boolean commandExecuted = FALSE;

    if (SPI_Command == 1) {
        /* [1] VCC2 ON, VCC3 ON */
        (void)Sbc_WriteRegField(SBC_M_S_CTRL, VCC2_ON_MASK, (uint8)VCC2_ON_NORMAL);
        (void)Sbc_WriteRegField(SBC_M_S_CTRL, VCC3_ON_MASK, (uint8)VCC3_ENABLED);
        commandExecuted = TRUE;
        SPI_Command = 0;
    }
    else if (SPI_Command == 2) {
        /* [2] VCC2 OFF, VCC3 OFF */
        (void)Sbc_WriteRegField(SBC_M_S_CTRL, VCC2_ON_MASK, (uint8)VCC2_OFF);
        (void)Sbc_WriteRegField(SBC_M_S_CTRL, VCC3_ON_MASK, (uint8)VCC3_OFF);
        commandExecuted = TRUE;
        SPI_Command = 0;
    }

    if (commandExecuted) {
        /* Notification hook for debuggers */
    }
}