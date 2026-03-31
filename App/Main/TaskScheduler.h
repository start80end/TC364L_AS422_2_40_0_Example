/** @brief GTM Based Task Scheduler Header */

#ifndef TASK_SCHEDULER_H
#define TASK_SCHEDULER_H

#include "Mcu.h"

/* Select System Tick Resolution (Default 1ms) */
#define SYSTEM_TICK 1000

/* GTM CMU_FXCLK1 = 6.25 MHz (0.16us per tick) */
/* 6250 ticks = 1ms (1000us) */
#define SYSTEM_TICK_TIMER_PERIOD (6250)

/**
 * @brief Task ID definitions based on cycle time (unit: ms)
 */
typedef enum {
    TASK_1ms   = 1,
    TASK_5ms   = 5,
    TASK_10ms  = 10,
    TASK_20ms  = 20,
    TASK_50ms  = 50,
    TASK_100ms = 100,
    TASK_500ms = 500,
    TASK_1000ms = 1000
} TASK_ID;

/** @brief Task Control Union */
typedef union {
    unsigned int U;
    struct {
        unsigned int TickCount : 16; /* Current tick index */
        unsigned int TaskRun   : 10; /* Task ID to run */
        unsigned int Enable    : 1;  /* Scheduler trigger flag */
        unsigned int Reserved  : 5;
    } B;
} TASK_CONTROL;

/**
 * @brief Global parameters for task limits
 */
typedef struct {
    unsigned int TaskMax;
    unsigned int SystemTick;
} TASK_CONFIG;

/* Global Task Control for Debugging */
extern TASK_CONTROL TaskControl;
extern TASK_CONFIG  TaskConfig;

/** @brief Init scheduler structure and timer */
void TaskScheduler_Initialization(unsigned int maxTask);

/** @brief Background loop activator */
void TaskScheduler_ActivateTask(void);

#endif /* TASK_SCHEDULER_H */
