//
// Created by paulo on 22/09/2022.
//

#ifndef EQMOUNT_CUSTOM_CONTROLLER_ASTRO_CONV_H
#define EQMOUNT_CUSTOM_CONTROLLER_ASTRO_CONV_H

#include "steppers.h"
#include "adc.h"
#include "PA6H.h"

#define ASTRO_TARGET_NAME_MAX_CHARACTERS 27
#define EARTH_ROTATION_HOURS             24
#define EARTH_ROTATION_MINS              1440.0
#define EARTH_ROTATION_SECS              86400.0
#define SECONDS_PER_HOUR                 3600
#define MINUTES_PER_HOUR                 60

#define WORM_GEAR_RATIO 96
#define STEPPER_FULL_RANGE (STEPPER_MAX_STEPS * WORM_GEAR_RATIO)

#define TRACKING_SPEED_PULSE_PERIOD_duS 28125 /**< in deca-microseconds, us = 281250 */
#define MAX_SPEED_PULSE_PERIOD_duS 187 /**< in deca-microseconds, us = 1870 */

typedef enum {
	UNREACHABLE_SET = 0,
	UNREACHABLE_UNRISEN,
	REACHABLE,
	UNKNOWN_REACH = -1,
} reachability_t;

typedef struct {
	double decimal_hours;
	uint8_t hours;
	uint8_t minutes;
} ptime_t;

typedef struct {
	double decimal_degrees;
	uint16_t degrees;
	uint8_t arc_minutes;
} angle_t;

typedef struct astro_pos {
	ptime_t right_ascension;
	angle_t declination;
} astro_pos_t;

typedef struct astro_target {
	astro_pos_t position;
	char* name; /** characters limit + '\0' */
} astro_target_t;

void astro_init(void);

void astro_update_LMST(void);

bool astro_is_at_target(void);

void astro_update_raw_fine_adjusts(void);

void astro_update_target(astro_target_t target);

GNSS_data_t* astro_get_gnss_pointer(void);

void astro_start_tracking(void);

void astro_goto_target(void);

void astro_go_home(void);

void astro_full_stop(void);

/** Must be called from timer isr, just after ARR reload. */
void astro_stepper_position_step(motor_axis_t axis);

/** Must be called from timer isr, just after ARR reload, updating TIM->ARR. */
uint16_t moving_to_target_smoothen_period(void);

#endif //EQMOUNT_CUSTOM_CONTROLLER_ASTRO_CONV_H