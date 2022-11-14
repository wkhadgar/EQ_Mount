/**
 * @brief stepper motors control module
 */

#ifndef EQMOUNT_CUSTOM_CONTROLLER_STEPPERS_H
#define EQMOUNT_CUSTOM_CONTROLLER_STEPPERS_H

#include "main.h"
#include "tim.h"

#define MICRO_STEPPING 16
#define STEPPER_MAX_STEPS (200 * MICRO_STEPPING)
#define WORM_RATIO 96

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
	struct {
		uint32_t position; /** < in relation to the full WORM_RATIO * STEPPER_MAX_STEPS steps */
		uint32_t target_position;
		direction_t direction; /** < whether rotation happens clockwise or not */
		bool on_status; /** < is the motor being driven right now? */
		bool is_configured;
	} info;
	
	struct timer_config {
		const uint32_t TIM_CHANNEL;
		const TIM_HandleTypeDef* htim;
		TIM_TypeDef* TIM;
		uint16_t pwm_period; /** < in deca-micro_seconds, 0 if invalid */
	} timer_config;
	
	struct {
		const GPIO_TypeDef* GPIO;
		const uint16_t port;
	} step_pin;
	
	struct {
		const GPIO_TypeDef* GPIO;
		const uint16_t port;
	} dir_pin;
	
	struct {
		const GPIO_TypeDef* GPIO;
		const uint16_t port;
	} enable_pin;
	
} stepper_t;

/**
 * @brief configures a given motor
 *
 * @param stp [out] Address of stepper motor to be configured.
 */
void stepper_init(stepper_t* stp);

/**
 * @brief Reverses the stepper direction.
 *
 * @param stp [out] Address of stepper motor to be configured.
 */
void stepper_reverse_direction(stepper_t* stp);

/**
 * @brief Enables the stepper.
 *
 * @param stp [out] Address of stepper motor to be configured.
 */
void stepper_disable(stepper_t* stp);

/**
 * @brief Disables the stepper.
 *
 * @param stp [out] Address of stepper motor to be configured.
 */
void stepper_enable(stepper_t* stp);

/**
 * @brief Sets the stepper direction.
 *
 * @param stp [out] Address of stepper motor to be configured.
 * @param dir Direction of the movement.
 */
void stepper_set_direction(stepper_t* stp, direction_t dir);

/**
 * @brief Updates the pwm_period of the given stepper, to make a smoothened movement to it's target position.
 *
 * @param stp [out] Address of stepper motor to be configured.
 */
uint16_t stepper_to_target_smoothen_period_update(int32_t target_relative_dist);

#endif //EQMOUNT_CUSTOM_CONTROLLER_STEPPERS_H