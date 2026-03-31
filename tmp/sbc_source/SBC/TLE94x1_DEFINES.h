/*********************************************************************************************************************
 * Copyright (c) 2021, Infineon Technologies AG
 *
 * Distributed under the Boost Software License, Version 1.0.
 *********************************************************************************************************************/

#ifndef TLE94x1_DEFINES_H
#define TLE94x1_DEFINES_H

/* ================================================================================ */
/* ================           General Control Registers            ================ */
/* ================================================================================ */

typedef enum {
    /* General Control Registers */
    SBC_M_S_CTRL                = 0x01U,
    SBC_HW_CTRL_0               = 0x02U,
    SBC_WD_CTRL                 = 0x03U,
    SBC_BUS_CTRL_0              = 0x04U,
    SBC_WK_CTRL_0               = 0x06U,
    SBC_WK_CTRL_1               = 0x07U,
    SBC_WK_PUPD_CTRL            = 0x08U,
    SBC_BUS_CTRL_3              = 0x0BU,
    SBC_TIMER_CTRL              = 0x0CU,
    SBC_HW_CTRL_1               = 0x0EU,
    SBC_HW_CTRL_2               = 0x0FU,
    SBC_GPIO_CTRL               = 0x17U,
    SBC_PWM_CTRL                = 0x18U,
    SBC_PWM_FREQ_CTRL           = 0x1CU,
    SBC_HW_CTRL_3               = 0x1DU,
    SBC_SYS_STATUS_CTRL_0       = 0x1EU,
    SBC_SYS_STATUS_CTRL_1       = 0x1FU,

    /* Selective Wake Control Registers */
    SBC_SWK_CTRL                = 0x20U,
    SBC_SWK_BTL0_CTRL           = 0x21U,
    SBC_SWK_BTL1_CTRL           = 0x22U,
    SBC_SWK_ID3_CTRL            = 0x23U,
    SBC_SWK_ID2_CTRL            = 0x24U,
    SBC_SWK_ID1_CTRL            = 0x25U,
    SBC_SWK_ID0_CTRL            = 0x26U,
    SBC_SWK_MASK_ID3_CTRL       = 0x27U,
    SBC_SWK_MASK_ID2_CTRL       = 0x28U,
    SBC_SWK_MASK_ID1_CTRL       = 0x29U,
    SBC_SWK_MASK_ID0_CTRL       = 0x2AU,
    SBC_SWK_DLC_CTRL            = 0x2BU,
    SBC_SWK_DATA7_CTRL          = 0x2CU,
    SBC_SWK_DATA6_CTRL          = 0x2DU,
    SBC_SWK_DATA5_CTRL          = 0x2EU,
    SBC_SWK_DATA4_CTRL          = 0x2FU,
    SBC_SWK_DATA3_CTRL          = 0x30U,
    SBC_SWK_DATA2_CTRL          = 0x31U,
    SBC_SWK_DATA1_CTRL          = 0x32U,
    SBC_SWK_DATA0_CTRL          = 0x33U,
    SBC_SWK_CAN_FD_CTRL         = 0x34U,
    SBC_SWK_OSC_TRIM_CTRL       = 0x38U,
    SBC_SWK_OPT_CTRL            = 0x39U,
    SBC_SWK_CDR_CTRL1           = 0x3CU,
    SBC_SWK_CDR_CTRL2           = 0x3DU,
    SBC_SWK_CDR_LIMIT_HIGH      = 0x3EU,
    SBC_SWK_CDR_LIMIT_LOW       = 0x3FU,

    /* General Status Registers */
    SBC_SUP_STAT_1              = 0x40U,
    SBC_SUP_STAT_0              = 0x41U,
    SBC_THERM_STAT              = 0x42U,
    SBC_DEV_STAT                = 0x43U,
    SBC_BUS_STAT                = 0x44U,
    SBC_WK_STAT_0               = 0x46U,
    SBC_WK_STAT_1               = 0x47U,
    SBC_WK_LVL_STAT             = 0x48U,
    SBC_GPIO_OC_STAT            = 0x54U,
    SBC_GPIO_OL_STAT            = 0x55U,

    /* Selective Wake Status Registers */
    SBC_SWK_STAT                = 0x70U,
    SBC_SWK_ECNT_STAT           = 0x71U,
    SBC_SWK_CDR_STAT1           = 0x72U,
    SBC_SWK_CDR_STAT2           = 0x73U,
    SBC_FAM_PROD_STAT           = 0x7EU
} sbc_register_t;

typedef enum {
    /* SBC_M_S_CTRL */
    MODE_MASK                   = 0xC0U,
    VCC2_ON_MASK                = 0x18U,
    VCC1_OV_RST_MASK            = 0x04U,
    VCC1_RT_MASK                = 0x03U,

    /* SBC_HW_CTRL_0 */
    SOFT_RESET_RO_MASK          = 0x40U,
    FO_ON_MASK                  = 0x20U,
    CP_EN_MASK                  = 0x04U,
    CFG_MASK                    = 0x01U,

    /* SBC_WD_CTRL */
    CHECKSUM_MASK               = 0x80U,
    WD_STM_EN_0_MASK            = 0x40U,
    WD_WIN_MASK                 = 0x20U,
    WD_EN_WK_BUS_MASK           = 0x10U,
    WD_TIMER_MASK               = 0x07U,

    /* SBC_BUS_CTRL_0 */
    CAN_MASK                    = 0x07U,

    /* SBC_WK_CTRL_0 */
    TIMER_WK_EN_MASK            = 0x40U,
    WD_STM_EN_1_MASK            = 0x04U,

    /* SBC_WK_CTRL_1 */
    INT_GLOBAL_MASK             = 0x80U,
    WK_MEAS_MASK                = 0x20U,
    WK_EN_MASK                  = 0x01U,

    /* SBC_WK_PUPD_CTRL */
    GPIO_WK_PUPD_MASK           = 0x30U,
    WK1_PUPD_MASK               = 0x03U,

    /* SBC_BUS_CTRL_3 */
    CAN_FLASH_MASK              = 0x01U,

    /* SBC_TIMER_CTRL */
    TIMER_ON_MASK               = 0x70U,
    TIMER_PER_MASK              = 0x07U,

    /* SBC_PWM_CTRL */
    PWM_DC_MASK                 = 0xFFU,

    /* SBC_PWM_FREQ_CTRL */
    PWM_FREQ_MASK               = 0x03U,

    /* SBC_SWK_CTRL */
    OSC_CAL_MASK                = 0x80U,
    TRIM_EN_MASK                = 0x60U,
    CANTO_MASK_MASK             = 0x10U,
    CFG_VAL_MASK                = 0x01U,

    /* SBC_DEV_STAT */
    DEV_STAT_MASK               = 0xC0U,
    WD_FAIL_MASK                = 0x0CU,
    SPI_FAIL_MASK               = 0x02U,
    FAILURE_MASK                = 0x01U
} sbc_bit_mask_t;

/* --- Named Enumerations for Settings --- */

typedef enum {
    SBC_MODE_SLEEP              = 0x00U,
    SBC_MODE_STOP               = 0x01U,
    SBC_MODE_NORMAL              = 0x02U,
    SBC_MODE_RESET              = 0x03U
} sbc_mode_t;

typedef enum {
    SBC_VCC2_OFF                = 0x00U,
    SBC_VCC2_ON_NORMAL,
    SBC_VCC2_ON_NORMAL_STOP,
    SBC_VCC2_ON_ALWAYS
} sbc_vcc2_on_t;

typedef enum {
    SBC_VCC1_RT_VRT1            = 0x00U,
    SBC_VCC1_RT_VRT2,
    SBC_VCC1_RT_VRT3,
    SBC_VCC1_RT_VRT4
} sbc_vcc1_tr_t;

typedef enum {
    WD_10MS                     = 0x00U,
    WD_20MS,
    WD_50MS,
    WD_100MS,
    WD_200MS,
    WD_500MS,
    WD_1000MS,
    WD_10000MS
} sbc_wd_timer_t;

typedef enum {
    WINDOW_WD                   = 0x01U,
    TIMEOUT_WD                  = 0x00U
} sbc_wd_win_t;

#endif /* TLE94x1_DEFINES_H */
