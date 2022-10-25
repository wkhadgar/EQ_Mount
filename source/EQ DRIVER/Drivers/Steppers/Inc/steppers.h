/**
 * @brief stepper motors control module
 */

#ifndef EQMOUNT_CUSTOM_CONTROLLER_STEPPERS_H
#define EQMOUNT_CUSTOM_CONTROLLER_STEPPERS_H

#include "main.h"
#include "tim.h"

#define MICRO_STEPPING 16
#define STEPPER_MAX_STEPS (200 * MICRO_STEPPING)

typedef enum motor_axis {
    Right_Ascension,
    Declination,
    Focus,
} motor_axis_t;

typedef enum dir_t {
    counter_clockwise = -1,
    clockwise = 1,
} direction_t;

typedef struct stepper_motor {
    motor_axis_t axis;
    uint16_t position; /** < in relation to the full 200 * MICRO_STEPPING steps */
    uint16_t target_position;
    uint16_t auto_step_ticks; /** < in CNT ticks ; 0 if invalid */

    direction_t direction; /** < whether rotation happens clockwise or not */
    bool on_status; /** < is the motor being driven right now? */
    bool is_configured;

    const struct timer {
        TIM_TypeDef* TIMx;
        TIM_HandleTypeDef* htimx;
    } timer_config;
    const struct step {
        GPIO_TypeDef *GPIO;
        uint16_t port;
    } step_pin;
    const struct dir {
        GPIO_TypeDef *GPIO;
        uint16_t port;
    } dir_pin;
    const struct enable {
        const GPIO_TypeDef *GPIO;
        const uint16_t port;
    } enable_pin;

} stepper_t;

/**
 * @brief configures a given motor
 *
 * @param s [out] Stepper motor to be configured
 */
void stepper_init(stepper_t *s);

/**
 * @brief Do a single motor half_step if step pin is set as GPIO output.
 *
 * @param s [out] Given stepper motor
 * @return uint8_t Final position of the motor, in relation to the configured referencial.
 */
uint16_t half_step(stepper_t *s);

void stepper_reverse_direction(stepper_t *s);

void stepper_set_direction(stepper_t *s, direction_t dir);

uint16_t stepper_to_target_smoothen_period(stepper_t* s, uint16_t target_pos);

#endif //EQMOUNT_CUSTOM_CONTROLLER_STEPPERS_H
