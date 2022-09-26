//
// Created by paulo on 22/09/2022.
//

#ifndef EQMOUNT_CUSTOM_CONTROLLER_ASTRO_CONV_H
#define EQMOUNT_CUSTOM_CONTROLLER_ASTRO_CONV_H

#include "steppers.h"

#define EARTH_ROTATION_MIN 1440
#define EARTH_ROTATION_SECS 86400
#define RA_WORM_RATIO 96
#define LOCAL_LATITUDE_SECS -34396 /**< 09°33'16" S */
#define LOCAL_LONGITUDE_SECS -128789 /**< -035°46'29" E */
#define ASTRO_TARGET_NAME_MAX_CHARACTERS 27

typedef struct astro_pos {
    uint16_t right_ascension; /** minutes (unit is hours) */
    uint16_t declination; /** minutes (unit is degrees)  */
} astro_pos_t;

typedef struct astro_target {
    astro_pos_t position;
    char name[ASTRO_TARGET_NAME_MAX_CHARACTERS + 1]; /** characters limit + '\0' */
} astro_target_t;

typedef struct mount_data {
    uint32_t local_time;
    uint32_t local_date;
    astro_pos_t orientation;
} mount_data_t;

static mount_data_t EQ_MOUNT = {0};

void astro_go_to(astro_target_t *target, stepper_t *right_ascension_stp, stepper_t *declination_stp);

void astro_start_tracking(stepper_t *right_ascension_stp, stepper_t *declination_stp);

void astro_stop_tracking(stepper_t *right_ascension_stp, stepper_t *declination_stp);

#endif //EQMOUNT_CUSTOM_CONTROLLER_ASTRO_CONV_H