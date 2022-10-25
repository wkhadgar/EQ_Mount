/**
 * @brief stepper motors control module
 */

#include "../Inc/steppers.h"

#define WORM_GEAR_ROTATION_PERIOD_uS 900000000
#define STEPPER_ROTATION_PULSE_PERIOD_uS ((WORM_GEAR_ROTATION_PERIOD_uS) / STEPPER_MAX_STEPS)
#define MAX_SPEED_PULSE_PERIOD_uS 1875


void stepper_disable(stepper_t* s) {
    s->on_status = 0;
    HAL_GPIO_WritePin(s->enable_pin.GPIO, s->enable_pin.port, (GPIO_PinState) !s->on_status);
}

void stepper_enable(stepper_t* s) {
    s->on_status = 1;
    HAL_GPIO_WritePin(s->enable_pin.GPIO, s->enable_pin.port, (GPIO_PinState) !s->on_status);
    HAL_GPIO_WritePin(s->dir_pin.GPIO, s->dir_pin.port, (GPIO_PinState) s->direction);
}

void stepper_init(stepper_t* s) {
    if (s == NULL) {
        return;
    }

    if (!s->is_configured) {
        s->position = 0;
        s->direction = 0;
        stepper_enable(s);

        s->auto_step_ticks = (s->axis == Right_Ascension) ? (
                (STEPPER_ROTATION_PULSE_PERIOD_uS / MAX_SPEED_PULSE_PERIOD_uS)) : 2;
        s->is_configured = true;
    }
}

uint16_t half_step(stepper_t* s) {
    static bool rise = 0;
    rise = !rise;

    int8_t inc;
    if (rise) {
        HAL_GPIO_WritePin(s->step_pin.GPIO, s->step_pin.port, GPIO_PIN_SET); //pulse rise

        if (s->direction) {
            inc = 1;
        } else {
            inc = -1;
        }
        s->position += inc;
    } else {
        HAL_GPIO_WritePin(s->step_pin.GPIO, s->step_pin.port, GPIO_PIN_RESET); //pulse fall
    }

    return s->position;
}

void stepper_reverse_direction(stepper_t* s) {
    s->direction = !s->direction;
    HAL_GPIO_WritePin(s->dir_pin.GPIO, s->dir_pin.port, (GPIO_PinState) s->direction);
}

void stepper_set_direction(stepper_t* s, direction_t dir) {
    if (dir == clockwise) {
        s->direction = clockwise;
        HAL_GPIO_WritePin(s->dir_pin.GPIO, s->dir_pin.port, GPIO_PIN_SET);
    } else {
        s->direction = counter_clockwise;
        HAL_GPIO_WritePin(s->dir_pin.GPIO, s->dir_pin.port, GPIO_PIN_RESET);
    }
}

uint16_t stepper_to_target_smoothen_period(stepper_t* s, uint16_t target_pos) {
    static const uint16_t DECELERATION_DELTA_RANGE = 5 * MICRO_STEPPING;
    static const uint16_t MAX_PERIOD = BASE_PERIOD * 8;
    static const uint8_t PERIOD_DELTA = 20;
    static int8_t delta_magnitude = -1;
    static uint16_t this_step_period = MAX_PERIOD;

    int target_relative_dist = target_pos - s->position;
    uint16_t absolute_dist = target_relative_dist >= 0 ? target_relative_dist : -target_relative_dist;

    if (!absolute_dist) return MAX_PERIOD;

    /** direction of move decision */
    {
        if ((target_relative_dist > 0) && ((target_relative_dist / 2) >= (STEPPER_MAX_STEPS / 2))) {
            stepper_set_direction(s, clockwise);
        } else if (target_relative_dist > 0) {
            stepper_set_direction(s, counter_clockwise);
        } else if ((absolute_dist / 2) >= (STEPPER_MAX_STEPS / 2)) {
            stepper_set_direction(s, counter_clockwise);
        } else {
            stepper_set_direction(s, clockwise);
        }
    }

    if (absolute_dist < DECELERATION_DELTA_RANGE) {
        delta_magnitude = 1; //increase period
    } else {
        delta_magnitude = -1; //decrease period
    }

    this_step_period += delta_magnitude * PERIOD_DELTA; /** new period */

    if (this_step_period > MAX_PERIOD) { //clamping deceleration;
        this_step_period = MAX_PERIOD;
    } else if (this_step_period < BASE_PERIOD) { //clamping acceleration
        this_step_period = BASE_PERIOD;
    }

    return this_step_period;
}