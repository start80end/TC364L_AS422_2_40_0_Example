/**
 * @file TaskScheduler.c
 * @brief GTM-based periodic task scheduler implementation.
 */

#include "TaskScheduler.h"
#include "Gpt.h"
#include "App_Cfg.h"

#if (USE_EXAMPLE_TIMER_LED == 1)
#include "../01_Timer_LED/LedToggleTask.h"
#endif

#if (USE_EXAMPLE_SPI_SBC == 1)
#include "../02_SPI_SBC/Sbc_SPI_Control.h"
#endif

/* Global runtime variables */
TASK_CONTROL TaskControl;
TASK_CONFIG  TaskConfig;

/**
 * @brief 1ms Tick Notification Callback (GTM TOM0_CH0)
 */
void IoHwAb_GptNotification0(void) {
    TaskControl.B.TickCount++;
    TaskControl.B.Enable = 1; /* Request task evaluation */
}

/* Dummy callback for unused GPT channels */
void IoHwAb_GptNotification1(void) {}

/**
 * @brief Initialize the periodic task scheduler
 */
void TaskScheduler_Initialization(unsigned int maxTask) {
    TaskControl.U = 0;
    TaskConfig.TaskMax = maxTask;
    TaskConfig.SystemTick = SYSTEM_TICK;

    /* Enable GTM TOM notifications */
    Gpt_EnableNotification(GptConf_GptChannelConfiguration_GptChannelConfiguration_0);
    Gpt_StartTimer(GptConf_GptChannelConfiguration_GptChannelConfiguration_0,
                   SYSTEM_TICK_TIMER_PERIOD);
}

/**
 * @brief Logic to determine which task is due (Modulo based)
 */
static void TaskScheduler_TaskCalculation(void) {
    uint32 count = TaskControl.B.TickCount;

    if ((count % TASK_1000ms) == 0) TaskControl.B.TaskRun = TASK_1000ms;
    else if ((count % TASK_500ms) == 0)  TaskControl.B.TaskRun = TASK_500ms;
    else if ((count % TASK_100ms) == 0)  TaskControl.B.TaskRun = TASK_100ms;
    else if ((count % TASK_50ms) == 0)   TaskControl.B.TaskRun = TASK_50ms;
    else if ((count % TASK_20ms) == 0)   TaskControl.B.TaskRun = TASK_20ms;
    else if ((count % TASK_10ms) == 0)   TaskControl.B.TaskRun = TASK_10ms;
    else if ((count % TASK_5ms) == 0)    TaskControl.B.TaskRun = TASK_5ms;
    else if ((count % TASK_1ms) == 0)    TaskControl.B.TaskRun = TASK_1ms;
}

/**
 * @brief Execute pending tasks based on prioritized timing
 */
void TaskScheduler_ActivateTask(void) {
    /* Reset counter after completing one full system cycle */
    if (TaskControl.B.TickCount >= TaskConfig.TaskMax) {
        TaskControl.B.TickCount = 0;
    }

    /* Process tasks only on 1ms tick trigger */
    if (TaskControl.B.Enable == 1) {
        TaskControl.B.Enable = 0; 

        TaskScheduler_TaskCalculation();

        switch (TaskControl.B.TaskRun) {
            case TASK_1000ms: 
                #if (USE_EXAMPLE_TIMER_LED == 1)
                Toggle_LED2(); 
                #endif

                #if (USE_EXAMPLE_SPI_SBC == 1)
                /* Production Mode: Background watchdog servicing */
                #if (CURRENT_SBC_MODE == SBC_MODE_PRODUCTION)
                if (Sbc_IsInitialized() == TRUE) {
                    Sbc_ServiceWatchdog();
                }
                #endif
                #endif
                break;

            case TASK_100ms:  
                #if (USE_EXAMPLE_TIMER_LED == 1)
                Toggle_LED1(); 
                #endif
                
                #if (USE_EXAMPLE_SPI_SBC == 1)
                /* Handle SBC connectivity and initial start-up */
                /* [REFACTORED] SPI_Command 경합 방지를 위해 TaskScheduler에서는 
                   SBC 모드와 초기화 확인 프로세스만 최소화함. 
                   초기화 제어는 Sbc_SPI_Test(while loop)에서 전담. */
                #endif
                break;

            default: break;
        }
        
        TaskControl.B.TaskRun = 0;
    }
}
