#ifndef MOTORS_H
#define MOTORS_H

#include "stm32f4xx_hal.h" // Includes the STM32 HAL library for hardware definitions

/* 
 * ESC PWM MACROS:
 * These define the standard microsecond values for 50Hz RC servo signals.
 * - 1000us = Full Reverse
 * - 1500us = Neutral / Stop (Required for arming)
 * - 2000us = Full Forward
 */
#define PWM_FULL_REV  1000
#define PWM_NEUTRAL   1500
#define PWM_FULL_FWD  2000

/* Function Prototypes */

// Initializes the timers and executes the required 3-second ESC arming sequence.
void motors_init(TIM_HandleTypeDef *htim);

// Sets the speed of the left and right motors. 
// Inputs should be between -500 (full reverse) and +500 (full forward).
void motors_set_speeds(TIM_HandleTypeDef *htim, int16_t left_throttle, int16_t right_throttle);

// A safety function to immediately halt both motors.
void motors_stop(TIM_HandleTypeDef *htim);

#endif /* MOTORS_H */
