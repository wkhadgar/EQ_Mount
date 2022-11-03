//
// Created by paulo on 22/09/2022.
//

#ifndef EQMOUNT_CUSTOM_CONTROLLER_ASTRO_CONV_H
#define EQMOUNT_CUSTOM_CONTROLLER_ASTRO_CONV_H

#include <math.h>
#include "steppers.h"
#include "adc.h"
#include "tim.h"
#include "PA6H.h"

#define ASTRO_TARGET_NAME_MAX_CHARACTERS 27
#define EARTH_ROTATION_HOURS             24
#define EARTH_ROTATION_MINS              1440.0
#define EARTH_ROTATION_SECS              86400.0
#define SECONDS_PER_HOUR                 3600
#define MINUTES_PER_HOUR                 60

#define TRACKING_SPEED_PULSE_PERIOD_duS 28125 /**< in deca-microseconds, us = 281250 */
#define MAX_SPEED_PULSE_PERIOD_duS 187 /**< in deca-microseconds, us = 1870 */

typedef enum {
		UNREACHABLE_SET = 0,
		UNREACHABLE_UNRISEN,
		REACHABLE,
		UNKNOWN_REACH = -1,
} reachability_t;

typedef struct {
	uint8_t hours;
  	uint8_t minutes;
} ptime_t;

typedef struct {
	uint16_t degrees;
	uint8_t arc_minutes;
} angle_t;

typedef struct astro_pos {
    ptime_t right_ascension;
    angle_t declination;
} astro_pos_t;

typedef struct astro_target {
    astro_pos_t position;
    char name[ASTRO_TARGET_NAME_MAX_CHARACTERS + 1]; /** characters limit + '\0' */
} astro_target_t;

void astro_update_LMST(void);

void astro_update_fine_adjusts(void);

reachability_t astro_set_target(astro_target_t target);

GNSS_data_t* astro_get_gnss_pointer(void);

void astro_stepper_position_step(motor_axis_t axis);

void astro_init(void);

void astro_go_to(astro_target_t* target);

void astro_start_tracking(void);

void astro_full_stop(void);


#endif //EQMOUNT_CUSTOM_CONTROLLER_ASTRO_CONV_H
