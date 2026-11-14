#ifndef STEERING_H
#define STEERING_H

#include "stm32f4xx_hal.h"
#include "motors.h" // Requires the low-level motor driver

/* Function Prototypes */

// Drives the skimmer straight forward at the specified throttle
void steer_go_straight(TIM_HandleTypeDef *htim, int16_t throttle);

// Reverses the skimmer straight backward at the specified throttle
void steer_reverse(TIM_HandleTypeDef *htim, int16_t throttle);

// Executes a zero-radius turn to the right (spins in place)
void steer_turn_right(TIM_HandleTypeDef *htim, int16_t turn_speed);

// Executes a zero-radius turn to the left (spins in place)
void steer_turn_left(TIM_HandleTypeDef *htim, int16_t turn_speed);

// Immediately halts all movement
void steer_stop(TIM_HandleTypeDef *htim);

#endif /* STEERING_H */
