/**
 * @file Sbc_SPI_Control.c
 * @brief TLE9263-3BQX SBC Mode Control Implementation (SPI based).
 *
 * Target: AURIX TC364 + TLE9263-3BQX
 * 
 * [SPI Hardware Specification]
 * - Interface: QSPI3 (Master), Asynchronous Mode (Non-blocking)
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
#include "IfxStm_reg.h"  /* STM0 timer register access for non-blocking delay */

/* Internal SPI Buffers */
static uint16 Sbc_SpiTxBuffer;
static uint16 Sbc_SpiRxBuffer;

/* Error Status Tracker */
volatile SBC_ErrorCode g_SbcError = {SBC_M_S_CTRL, 0, 0};

/* Connection State */
static boolean g_sbcInitialized = FALSE;

/**
 * [Shadow Register] Stores the last successfully written value for M_S_CTRL.
 * Used to prevent register corruption in Sbc_WriteRegField if Sbc_ReadReg 
 * fails due to SPI timeout (returning 0x0000).
 */
static uint8 s_shadowMsCtrl = 0x02U; /* Default: Normal Mode (VRT3) */

/**
 * [STM Timer Constants] Assuming 100MHz SPB clock.
 * Formula: ms * (Clock Frequency / 1000) = STM ticks
 */
#define SBC_STM_HZ           (100000000UL)
#define SBC_MS_TO_TICKS(ms)  ((uint32)((ms) * (SBC_STM_HZ / 1000UL)))

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
 * @brief SPI 16-bit Transfer with Async Polling and Timeout.
 * Includes recovery logic to reset the SPI driver in case of lock-ups.
 */
static uint16 Sbc_Transfer16(uint16 frame) {
    uint32 timeout = 1000000U;
    
    Sbc_SpiTxBuffer = frame;
    (void)Spi_WriteIB(SpiConf_SpiChannel_Ch_Sbc_Data, (Spi_DataBufferType*)&Sbc_SpiTxBuffer);
    (void)Spi_AsyncTransmit(SpiConf_SpiSequence_Seq_Sbc_Init);
    
    /* Poll until transfer is complete or timeout occurs */
    while ((Spi_GetSequenceResult(SpiConf_SpiSequence_Seq_Sbc_Init) == SPI_SEQ_PENDING) && (timeout > 0U)) {
        timeout--;
        __nop();
    }

    if (Spi_GetSequenceResult(SpiConf_SpiSequence_Seq_Sbc_Init) == SPI_SEQ_OK) {
        (void)Spi_ReadIB(SpiConf_SpiChannel_Ch_Sbc_Data, (Spi_DataBufferType*)&Sbc_SpiRxBuffer);
    } else {
        /**
         * Timeout Recovery: Reset the SPI driver to ensure subsequent calls can proceed.
         * The hardware usually finishes the 16-microsecond frame even if the driver hang,
         * but software needs a clean state.
         */
        Sbc_SpiRxBuffer = 0U;
        (void)Spi_DeInit();
        Spi_Init(&Spi_Config);
        
        volatile uint32 recovery_delay = 500000U;
        while(recovery_delay--) { __nop(); }
    }
    
    return Sbc_SpiRxBuffer;
}

/** @brief Read & Clear (Status registers only) */
uint16 Sbc_ReadAndClearReg(sbc_register_t addr) {
    uint16 frame = (uint16)((uint8)addr | 0x80U);
    return Sbc_Transfer16(frame);
}

/** @brief Read register (Returns 16-bit Status + Data) */
uint16 Sbc_ReadReg(sbc_register_t addr) {
    uint16 frame = (uint16)((uint8)addr & 0x7FU);
    return Sbc_Transfer16(frame);
}

/** @brief Read specific bit field from a register */
uint8 Sbc_ReadRegField(sbc_register_t addr, sbc_bit_mask_t mask) {
    uint8 data = (uint8)((Sbc_ReadReg(addr) >> 8U) & 0xFFU);
    return (uint8)((data & (uint8)mask) >> SBC_BITPOSITION(mask));
}

/** 
 * @brief Write data to SBC register and verify the result.
 */
SBC_ErrorCode Sbc_WriteReg(sbc_register_t addr, uint8 data) {
    SBC_ErrorCode errCode;
    uint16 frame = (uint16)(((uint16)data << 8U) | (uint16)((uint8)addr & 0x7FU) | 0x80U);
    
    (void)Sbc_Transfer16(frame);
    
    /* Verify write result by reading back the register content */
    uint8 actual = (uint8)((Sbc_ReadReg(addr) >> 8U) & 0xFFU);
    
    errCode.SBC_Register = addr;
    errCode.expectedValue = data;
    errCode.flippedBitsMask = actual ^ data;
    
    g_SbcError = errCode;
    return errCode;
}

/**
 * @brief Write to a specific bit field in a register.
 * Uses Shadow Register protection for M_S_CTRL to prevent full register 
 * corruption if Read-Modify-Write fails due to SPI timeout.
 */
SBC_ErrorCode Sbc_WriteRegField(sbc_register_t addr, sbc_bit_mask_t mask, uint8 val) {
    uint16 readResult = Sbc_ReadReg(addr);
    uint8 currentData = (uint8)((readResult >> 8U) & 0xFFU);

    /* If Read fails (0x0000 due to timeout), use the last known successful value for protection */
    if ((readResult == 0x0000U) && (addr == SBC_M_S_CTRL)) {
        currentData = s_shadowMsCtrl;
    }

    uint8 newData = (currentData & (uint8)(~(uint8)mask));
    newData |= (uint8)((val << SBC_BITPOSITION(mask)) & (uint8)mask);

    SBC_ErrorCode ret = Sbc_WriteReg(addr, newData);

    /* Update Shadow Register after a successful write */
    if (addr == SBC_M_S_CTRL) {
        s_shadowMsCtrl = newData;
    }
    return ret;
}

/**
 * @brief SBC Power Control State Machine.
 * Executes mode transitions using STM timer based non-blocking delays.
 * This allows other tasks (e.g. LED toggling) to run concurrently.
 */
typedef enum {
    SBC_STATE_BOOT_WAIT = 0U,   /* Wait for SBC power-up stabilization */
    SBC_STATE_INIT      = 1U,   /* Clear POR flags and basic setup */
    SBC_STATE_CMD1      = 2U,   /* Apply VCC3 ON, VCC2 OFF */
    SBC_STATE_CMD1_WAIT = 3U,   /* Wait for observation */
    SBC_STATE_CMD2      = 4U,   /* Apply VCC2 ON, VCC3 OFF */
    SBC_STATE_DONE      = 5U    /* Sequence finished */
} SbcTestState_t;

static SbcTestState_t s_testState  = SBC_STATE_BOOT_WAIT;
static uint32         s_testTimer  = 0U;

void Sbc_SPI_Test(void) {
    uint32 elapsed;

    switch (s_testState) {

        case SBC_STATE_BOOT_WAIT:
            if (s_testTimer == 0U) {
                s_testTimer = MODULE_STM0.TIM0.U;
            }
            elapsed = MODULE_STM0.TIM0.U - s_testTimer;
            if (elapsed >= SBC_MS_TO_TICKS(1500U)) {
                s_testTimer = 0U;
                s_testState = SBC_STATE_INIT;
            }
            break;

        case SBC_STATE_INIT:
            (void)Sbc_Init();
            s_testState = SBC_STATE_CMD1;
            break;

        case SBC_STATE_CMD1:
            /* Command: VCC3 ON, VCC2 OFF (Normal Mode) */
            (void)Sbc_WriteReg(SBC_M_S_CTRL, 0x20U);
            s_testTimer = MODULE_STM0.TIM0.U;
            s_testState = SBC_STATE_CMD1_WAIT;
            break;

        case SBC_STATE_CMD1_WAIT:
            elapsed = MODULE_STM0.TIM0.U - s_testTimer;
            if (elapsed >= SBC_MS_TO_TICKS(2000U)) {
                s_testState = SBC_STATE_CMD2;
            }
            break;

        case SBC_STATE_CMD2:
            /* Command: VCC2 ON, VCC3 OFF (Normal Mode) */
            (void)Sbc_WriteReg(SBC_M_S_CTRL, 0x08U);
            s_testState = SBC_STATE_DONE;
            break;

        case SBC_STATE_DONE:
        default:
            /* Finished; no further commands sent */
            break;
    }
}

/**
 * @brief SPI Sequence End Notification Callback
 * Registered in Tresos (SpiSeqEndNotification).
 * Currently unused as polling is preferred for stability.
 */
void Sbc_SpiNotification(void) {
    /* Reserved for future interrupt-driven mode transitions */
}