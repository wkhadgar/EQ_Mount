/**
 * @brief stepper motors control module
 */

#ifndef EQMOUNT_CUSTOM_CONTROLLER_STEPPERS_H
#define EQMOUNT_CUSTOM_CONTROLLER_STEPPERS_H

#include "main.h"

#define MICRO_STEPPING 16
#define STEPPER_MAX_STEPS (200 * MICRO_STEPPING)

typedef enum motor_axis {
    Right_Ascension,
    Declination,
    Focus,
} motor_axis_t;

typedef enum dir_t {
    clockwise,
    counter_clockwise,
} direction_t;

typedef struct stepper_motor {
    motor_axis_t axis;
    uint16_t position; /** < in relation to the full 200 * MICRO_STEPPING steps */
    uint16_t target_position;
    uint32_t auto_step_prescaler; /** < in us ; 0 if invalid */

    direction_t direction; /** < whether rotation happens clockwise or not */
    bool on_status; /** < is the motor being driven right now? */
    bool is_configured;

    struct step {
        GPIO_TypeDef *GPIO;
        uint16_t port;
    } step_pin;
    struct dir {
        GPIO_TypeDef *GPIO;
        uint16_t port;
    } dir_pin;
    struct enable {
        GPIO_TypeDef *GPIO;
        uint16_t port;
    } enable_pin;

} stepper_t;

static stepper_t RA_STEPPER = {
        .axis = Right_Ascension,
        .step_pin = {
                .GPIO = M1_STEP_GPIO_Port,
                .port = M1_STEP_Pin,
        },
        .dir_pin = {
                .GPIO = M1_DIR_GPIO_Port,
                .port = M1_DIR_Pin,
        },
        .enable_pin = {
                .GPIO = M1_ENABLE_GPIO_Port,
                .port = M1_ENABLE_Pin,
        }
};

static stepper_t DEC_STEPPER = {
        .axis = Declination,
        .step_pin = {
                .GPIO = M2_STEP_GPIO_Port,
                .port = M2_STEP_Pin,
        },
        .dir_pin = {
                .GPIO = M2_DIR_GPIO_Port,
                .port = M2_DIR_Pin,
        },
        .enable_pin = {
                .GPIO = M2_ENABLE_GPIO_Port,
                .port = M2_ENABLE_Pin,
        }
};

/**
 * @brief configures a given motor
 *
 * @param s [out] Stepper motor to be configured
 */
void stepper_init(stepper_t *s);

/**
 * @brief Do a single motor half_step
 *
 * @param s [out] Given stepper motor
 * @return uint8_t Final position of the motor, in relation to the configured referencial.
 */
uint16_t half_step(stepper_t *s);

void stepper_reverse_direction(stepper_t *s);

void stepper_set_direction(stepper_t *s, direction_t dir);

#endif //EQMOUNT_CUSTOM_CONTROLLER_STEPPERS_H
