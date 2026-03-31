/**
 * @file Cpu0_Main.c
 * @brief Main entry point for Core 0.
 */

#include "Ifx_Os.h"
#include "Ifx_Ssw_Infra.h"
#include "Ifx_Ssw.h"

#include "App_Cfg.h"
#include "Dio.h"
#include "Gpt.h"
#include "IfxSrc_reg.h"
#include "Irq.h"
#include "Mcu.h"
#include "Port.h"
#include "McalLib.h"
#include "TaskScheduler.h"

/* Project Feature Includes */
#if (USE_EXAMPLE_TIMER_LED == 1)
#include "../01_Timer_LED/LedToggleTask.h"
#endif

#if (USE_EXAMPLE_SPI_SBC == 1)
#include "Spi.h"
#include "Spi_PBcfg.h"
#include "../02_SPI_SBC/Sbc_SPI_Control.h"
#endif

#include "Mcu_PBcfg.h"
#include "Port_PBcfg.h"

/**
 * @brief Main function for Core 0
 */
void core0_main(void) {
    unsigned short cpuWdtPassword;
    unsigned short safetyWdtPassword;

    /* Enable interrupts early to match project configuration */
    ENABLE();

    /* Disable Watchdogs for debugging/demo purposes */
    cpuWdtPassword = Ifx_Ssw_getCpuWatchdogPassword(&MODULE_SCU.WDTCPU[0]);
    safetyWdtPassword = Ifx_Ssw_getSafetyWatchdogPassword();
    Ifx_Ssw_disableCpuWatchdog(&MODULE_SCU.WDTCPU[0], cpuWdtPassword);
    Ifx_Ssw_disableSafetyWatchdog(safetyWdtPassword);

    /* Phase 1: MCU Initialization (Clock & PLL) */
    Mcu_Init(&Mcu_Config);
    if (Mcu_InitClock(0) == E_OK) {
        while (Mcu_GetPllStatus() != MCU_PLL_LOCKED) {
            /* Wait for PLL Stability */
        }
        Mcu_DistributePllClock();
    }

    /* Phase 2: Fundamental Peripheral Initialization */
    Port_Init(&Port_Config);

    #if (USE_EXAMPLE_SPI_SBC == 1)
    /* Phase 3: SPI & SBC Communication Setup */
    Spi_Init(&Spi_Config);
    #endif

    /* Phase 4: GTM-based Task Scheduler Setup (Replacing legacy STM) */
    Gpt_Init(&Gpt_Config);
    IrqGtm_Init(); /* Initialize GTM interrupts */
    
    /* Enable GTM TOM0_CH0 Interrupt for Scheduler Tick */
    Mcal_SetBitAtomic(&(SRC_GTMTOM00.U), 10, 1, 1); /* SRE = 1 */

    /* Initialize task scheduler with 1000ms max cycle */
    TaskScheduler_Initialization(TASK_1000ms);

    while (1) {
        /* Execute Periodic Tasks */
        TaskScheduler_ActivateTask();
        
        #if (USE_EXAMPLE_SPI_SBC == 1)
        /* Handle manual SBC debug/test commands */
        Sbc_SPI_Test();
        #endif
        
        __nop();
    }
}
