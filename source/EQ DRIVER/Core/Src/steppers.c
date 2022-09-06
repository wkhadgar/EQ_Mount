/**
 * @brief stepper motors control module
 */

#include "/steppers.h"

#define RA_REVOLUTION_TIME_SECS 900
#define STEPPER_MAX_STEPS 200

stepper_t RA_STEPPER = {
		.axis = Right_Ascension,
		.step_pin = {
				.GPIOx = M1_STEP_GPIO_Port,
				.port = M1_STEP_Pin,
		},
		.dir_pin = {
				.GPIOx = M1_DIR_GPIO_Port,
				.port = M1_STEP_Pin,
		}
};

stepper_t DEC_STEPPER = {
		.axis = Declination,
		.step_pin = {
				.GPIOx = M2_STEP_GPIO_Port,
				.port = M2_STEP_Pin,
		},
		.dir_pin = {
				.GPIOx = M2_DIR_GPIO_Port,
				.port = M2_STEP_Pin,
		}
};

void stepper_init(stepper_t* s) {
	if (s == NULL) {
		return;
	}
	
	s->position = 0;
	s->direction = 0;
	
	if (s->axis == Right_Ascension) {
		s->auto_rot_period = RA_REVOLUTION_TIME_SECS;
	} else {
		s->auto_rot_period = 0;
	}
	
	s->is_configured == true;
}

uint8_t single_step(stepper_t* s) {
	uint32_t time_last = HAL_GetTick();
	
	HAL_GPIO_WritePin(s->step_pin->GPIOx, s->step_pin->port, 1);
	while ((HAL_GetTick() - time_last) < 2);
	HAL_GPIO_WritePin(s->step_pin->GPIOx, s->step_pin->port, 0);
	
	if (s->direction) {
		s->position = ++s->position % STEPPER_MAX_STEPS;
	} else {
		s->position = (s->position > 0) ? s->position-- : s->position = STEPPER_MAX_STEPS - 1;
	}
	
	return s->position;
}

void stepper_reverse(stepper_t* s) {
	s->direction = !s->direction;
	HAL_GPIO_WritePin(s->dir_pin->GPIOx, s->dir_pin->port, s->direction);
}