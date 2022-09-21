/**
 * @brief stepper motors control module
 */

#include "steppers.h"

#define RA_REVOLUTION_TIME_SECS 900
#define MICRO_STEPPING 16
#define STEPPER_MAX_STEPS (200 * MICRO_STEPPING)

void stepper_disable(stepper_t *s) {
    s->on_status = 0;
    HAL_GPIO_WritePin(s->enable_pin.GPIO, s->enable_pin.port, (GPIO_PinState) !s->on_status);
}

void stepper_enable(stepper_t *s) {
    s->on_status = 1;
    HAL_GPIO_WritePin(s->enable_pin.GPIO, s->enable_pin.port, (GPIO_PinState) !s->on_status);
    HAL_GPIO_WritePin(s->dir_pin.GPIO, s->dir_pin.port, (GPIO_PinState) s->direction);
}

void stepper_init(stepper_t *s) {
    if (s == NULL) {
        return;
    }

    s->position = 0;
    s->direction = 0;
    stepper_enable(s);

    s->auto_step_period = (s->axis == Right_Ascension) ? (1000000 * RA_REVOLUTION_TIME_SECS / STEPPER_MAX_STEPS) : 0;
    s->is_configured = true;
}

uint16_t half_step(stepper_t *s) {
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

void stepper_reverse_direction(stepper_t *s) {
    s->direction = !s->direction;
    HAL_GPIO_WritePin(s->dir_pin.GPIO, s->dir_pin.port, (GPIO_PinState) s->direction);
}

void stepper_set_direction(stepper_t *s, direction_t dir) {
    if (dir == clockwise) {
        s->direction = clockwise;
        HAL_GPIO_WritePin(s->dir_pin.GPIO, s->dir_pin.port, GPIO_PIN_SET);
    } else {
        s->direction = counter_clockwise;
        HAL_GPIO_WritePin(s->dir_pin.GPIO, s->dir_pin.port, GPIO_PIN_RESET);
    }
}
