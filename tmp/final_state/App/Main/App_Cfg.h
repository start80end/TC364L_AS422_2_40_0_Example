/**
 * @file App_Cfg.h
 * @brief Global Application Feature and System Configuration.
 */

#ifndef APP_CFG_H
#define APP_CFG_H

/* Project Feature Switchboard */
#define USE_EXAMPLE_TIMER_LED (1)  /* GTM-based LED Toggle project */
#define USE_EXAMPLE_SPI_SBC   (1)  /* SPI-based SBC (TLE9263) control project */
#define USE_EXAMPLE_ADC       (0)  
#define USE_EXAMPLE_PWM       (0)  

/* Global Timer System Setup */
#define TASK_TICK_1MS (1)

/* SBC Operation Mode Definitions */
#define SBC_MODE_PRODUCTION 0  /* Standard mode with watchdog active */
#define SBC_MODE_DEVELOPER  1  /* Debug mode for board bring-up/development */

/* CURRENT_SBC_MODE selects the behavior of the Task Scheduler and SBC logic */
#define CURRENT_SBC_MODE SBC_MODE_PRODUCTION

#endif /* APP_CFG_H */
