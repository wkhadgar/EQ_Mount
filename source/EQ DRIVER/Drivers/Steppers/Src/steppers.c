/**
 * @brief stepper motors control module
 */

#include "../Inc/steppers.h"
#include <math.h>

void stepper_disable(stepper_t* stp) {
	stp->info.on_status = 0;
	HAL_GPIO_WritePin(stp->enable_pin.GPIO, stp->enable_pin.port, (!stp->info.on_status));
}

void stepper_enable(stepper_t* stp) {
	stp->info.on_status = 1;
	HAL_GPIO_WritePin(stp->enable_pin.GPIO, stp->enable_pin.port, !stp->info.on_status);
	HAL_GPIO_WritePin(stp->dir_pin.GPIO, stp->dir_pin.port, stp->info.direction);
}

void stepper_init(stepper_t* stp) {
    if (stp == NULL) {
        return;
    }

    if (!stp->info.is_configured) {
		stp->info.direction = 0;
		stp->info.position = 48 * STEPPER_MAX_STEPS;
		stp->info.target_position = 48 * STEPPER_MAX_STEPS;
		stp->info.is_configured = true;
	
		stepper_enable(stp);
	}
}

void stepper_reverse_direction(stepper_t* stp) {
	stp->info.direction = !stp->info.direction;
	HAL_GPIO_WritePin(stp->dir_pin.GPIO, stp->dir_pin.port, stp->info.direction);
}

void stepper_set_direction(stepper_t* stp, direction_t dir) {
	if (dir == clockwise) {
		stp->info.direction = clockwise;
		HAL_GPIO_WritePin(stp->dir_pin.GPIO, stp->dir_pin.port, GPIO_PIN_SET);
	} else {
		stp->info.direction = counter_clockwise;
		HAL_GPIO_WritePin(stp->dir_pin.GPIO, stp->dir_pin.port, GPIO_PIN_RESET);
	}
}

#define MIN_PERIOD 120
#define MAX_PERIOD 1000
#define DELTA_PERIOD 20
#define DECELERATION_DELTA_RANGE 1000

static int8_t delta_magnitude = -1;
static uint16_t this_step_period = MAX_PERIOD;

inline uint16_t
stepper_to_target_smoothen_period_update(int32_t
target_relative_dist) {

uint32_t absolute_dist = target_relative_dist >= 0 ? target_relative_dist : -target_relative_dist;

if (!absolute_dist) {
this_step_period = MAX_PERIOD;
return MAX_PERIOD;
}

if (absolute_dist < DECELERATION_DELTA_RANGE) {
delta_magnitude = 1; //increase period
} else {
delta_magnitude = -1; //decrease period
}

this_step_period +=

delta_magnitude* sqrt(sqrt(this_step_period

/MIN_PERIOD)); /** new period */

if (this_step_period > MAX_PERIOD) { //clamping deceleration;
this_step_period = MAX_PERIOD;
} else if (this_step_period < MIN_PERIOD) { //clamping acceleration
this_step_period = MIN_PERIOD;
}

return
this_step_period;
}