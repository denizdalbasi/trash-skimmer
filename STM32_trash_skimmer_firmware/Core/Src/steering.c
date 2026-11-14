#include "steering.h"

/**
 * @brief Drives both motors forward equally.
 * @param throttle: Base speed (0 to 500)
 */
void steer_go_straight(TIM_HandleTypeDef *htim, int16_t throttle) {
    // Both motors spin forward at the same speed
    motors_set_speeds(htim, throttle, throttle);
}

/**
 * @brief Drives both motors backward equally.
 * @param throttle: Base reverse speed (0 to 500). The function applies the negative.
 */
void steer_reverse(TIM_HandleTypeDef *htim, int16_t throttle) {
    // Both motors spin backward at the same speed
    motors_set_speeds(htim, -throttle, -throttle);
}

/**
 * @brief Turns the boat right by running the left motor forward 
 *        and the right motor in reverse.
 * @param turn_speed: Intensity of the turn (0 to 500)
 */
void steer_turn_right(TIM_HandleTypeDef *htim, int16_t turn_speed) {
    // Left motor forward, right motor reverse creates a rightward pivot
    motors_set_speeds(htim, turn_speed, -turn_speed);
}

/**
 * @brief Turns the boat left by running the right motor forward 
 *        and the left motor in reverse.
 * @param turn_speed: Intensity of the turn (0 to 500)
 */
void steer_turn_left(TIM_HandleTypeDef *htim, int16_t turn_speed) {
    // Left motor reverse, right motor forward creates a leftward pivot
    motors_set_speeds(htim, -turn_speed, turn_speed);
}

/**
 * @brief Stops the boat.
 */
void steer_stop(TIM_HandleTypeDef *htim) {
    // Wrapper for the low-level stop command
    motors_stop(htim);
}
