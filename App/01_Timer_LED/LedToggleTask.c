/**
 * @file LedToggleTask.c
 * @brief Implementation of LED toggle functions using MCAL Dio module.
 */

#include "LedToggleTask.h"
#include "Dio.h"

/**
 * @brief Toggle LED1 (P33.4)
 * This function flips the logical state of LED1 channel defined in Tresos.
 */
void Toggle_LED1(void) 
{ 
    (void)Dio_FlipChannel(DioConf_DioChannel_LED1); 
}

/**
 * @brief Toggle LED2 (P33.5)
 * This function flips the logical state of LED2 channel defined in Tresos.
 */
void Toggle_LED2(void) 
{ 
    (void)Dio_FlipChannel(DioConf_DioChannel_LED2); 
}
