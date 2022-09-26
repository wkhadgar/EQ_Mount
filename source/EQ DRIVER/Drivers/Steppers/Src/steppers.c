/**
 * @brief stepper motors control module
 */

#include "../Inc/steppers.h"

#define WORM_GEAR_ROTATION_PERIOD_uS 900000000
#define STEPPER_ROTATION_PULSE_PERIOD_uS ((WORM_GEAR_ROTATION_PERIOD_uS) / STEPPER_MAX_STEPS)
#define MAX_SPEED_PULSE_PERIOD_uS 1875


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

    if (!s->is_configured) {
        s->position = 0;
        s->direction = 0;
        stepper_enable(s);

        s->auto_step_prescaler = (s->axis == Right_Ascension) ? (
                (STEPPER_ROTATION_PULSE_PERIOD_uS / MAX_SPEED_PULSE_PERIOD_uS) / 2) : 2;
        s->is_configured = true;
    }
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
