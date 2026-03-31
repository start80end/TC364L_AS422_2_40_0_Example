/** @brief TLE9263 SBC Control Header (SPI) */

#ifndef SBC_SPI_CONTROL_H
#define SBC_SPI_CONTROL_H

#include "Std_Types.h"
#include "SBC/TLE926x_DEFINES.h"

/**
 * @name Watchdog Configuration (WD_CTRL)
 * @{
 */
/* Using definitions from TLE926x_DEFINES.h (e.g., WD_1000MS, TIME_OUT_WD) */
/** @} */

/**
 * @brief SBC Error Code Structure
 */
typedef struct {
    sbc_register_t SBC_Register;  /* Register address where error/write occurred */
    uint8 flippedBitsMask;       /* Mask of bits that differ from expected (0 if OK) */
    uint8 expectedValue;         /* The value that was intended to be written/read */
} SBC_ErrorCode;

/** @brief Mask to bit position helper */
#define SBC_BITPOSITION(mask) (uint8)((mask & 0x01U) ? 0 : \
                                      (mask & 0x02U) ? 1 : \
                                      (mask & 0x04U) ? 2 : \
                                      (mask & 0x08U) ? 3 : \
                                      (mask & 0x10U) ? 4 : \
                                      (mask & 0x20U) ? 5 : \
                                      (mask & 0x40U) ? 6 : \
                                      (mask & 0x80U) ? 7 : 0)

/* SBC Logic functions */
boolean Sbc_Init(void);
void Sbc_ServiceWatchdog(void);
boolean Sbc_IsInitialized(void);

/* Debug & Control Variables */
extern volatile uint8 SPI_Command;

/**
 * @brief Read from SBC Register (Returns 16-bit Status + Data)
 */
uint16 Sbc_ReadReg(sbc_register_t addr);

/** @brief Read and Clear SBC Status register */
uint16 Sbc_ReadAndClearReg(sbc_register_t addr);

/** @brief Read bit field from register */
uint8 Sbc_ReadRegField(sbc_register_t addr, sbc_bit_mask_t mask);

/** @brief Write byte to register and verify */
SBC_ErrorCode Sbc_WriteReg(sbc_register_t addr, uint8 data);

/** @brief Write bit field to register */
SBC_ErrorCode Sbc_WriteRegField(sbc_register_t addr, sbc_bit_mask_t mask, uint8 val);

/** @brief Periodic test task for SBC commands */
void Sbc_SPI_Test(void);

#endif /* SBC_SPI_CONTROL_H */
