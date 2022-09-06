/**
 * @brief stepper motors control module
 */

#ifndef EQMOUNT_CUSTOM_CONTROLLER_STEPPERS_H
#define EQMOUNT_CUSTOM_CONTROLLER_STEPPERS_H

enum motor_axis_t {
	Right_Ascension,
	Declination,
	Focus,
};

typedef struct stepper_motor {
	motor_axis_t axis;
	uint8_t position; /** < in relation to the full 200 steps */
	uint8_t auto_rot_period; /** < in seconds */
	
	bool direction; /** < whether rotation happens clockwise or not */
	bool on_status; /** < is the motor been driven right now? */
	bool is_configured;
	
	struct {
		GPIO_TypeDef* GPIOx;
		uint16_t port;
	} step_pin;
	struct {
		GPIO_TypeDef GPIOx;
		uint16_t port;
	} dir_pin;
	
} stepper_t;

/**
 * @brief configures a given motor
 *
 * @param s [out] Stepper motor to be configured
 */
void stepper_init(stepper_t* s);

/**
 * @brief Do a single motor step
 *
 * @param s [out] Given stepper motor
 * @return uint8_t Final position of the motor, in relation to the configured referencial.
 */
uint8_t single_step(stepper_t* s);

void stepper_reverse(stepper_t* s);

#endif //EQMOUNT_CUSTOM_CONTROLLER_STEPPERS_H