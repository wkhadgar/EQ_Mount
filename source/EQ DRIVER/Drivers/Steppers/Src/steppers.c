/**
 * @brief stepper motors control module
 */

#include "../Inc/steppers.h"


void stepper_disable(stepper_t* stp) {
    stp->info.on_status = 0;
    HAL_GPIO_WritePin(stp->enable_pin.GPIO, stp->enable_pin.port, (GPIO_PinState) !stp->info.on_status);
}

void stepper_enable(stepper_t* stp) {
    stp->info.on_status = 1;
    HAL_GPIO_WritePin(stp->enable_pin.GPIO, stp->enable_pin.port, (GPIO_PinState) !stp->info.on_status);
    HAL_GPIO_WritePin(stp->dir_pin.GPIO, stp->dir_pin.port, (GPIO_PinState) stp->info.direction);
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
    HAL_GPIO_WritePin(stp->dir_pin.GPIO, stp->dir_pin.port, (GPIO_PinState) stp->info.direction);
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

static int32_t get_relative_dist(stepper_t* stp) {
	return (stp->info.target_position - stp->info.position);
}

uint16_t stepper_to_target_smoothen_period_update(int32_t target_relative_dist) {
	static const uint16_t DECELERATION_DELTA_RANGE = 5 * MICRO_STEPPING;
	static const uint16_t MAX_PERIOD = 1850 * 8;
	static const uint8_t PERIOD_DELTA = 20;
	static int8_t delta_magnitude = -1;
	static uint16_t this_step_period = MAX_PERIOD;
	
	uint32_t absolute_dist = target_relative_dist >= 0 ? target_relative_dist : -target_relative_dist;
	
	if (!absolute_dist) {
		return MAX_PERIOD;
    }

    if (absolute_dist < DECELERATION_DELTA_RANGE) {
        delta_magnitude = 1; //increase period
    } else {
        delta_magnitude = -1; //decrease period
    }

    this_step_period += delta_magnitude * PERIOD_DELTA; /** new period */

    if (this_step_period > MAX_PERIOD) { //clamping deceleration;
        this_step_period = MAX_PERIOD;
    } else if (this_step_period < 1850) { //clamping acceleration
        this_step_period = 1850;
    }

    return this_step_period;
}