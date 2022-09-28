//
// Created by paulo on 22/09/2022.
//

#ifndef EQMOUNT_CUSTOM_CONTROLLER_ASTRO_CONV_H
#define EQMOUNT_CUSTOM_CONTROLLER_ASTRO_CONV_H

#include "steppers.h"
#include <math.h>
#include "PA6H.h"

#define ASTRO_TARGET_NAME_MAX_CHARACTERS 27
#define EARTH_ROTATION_HOURS             24.0
#define EARTH_ROTATION_SECS              86400.0
#define EARTH_ROTATION_MINS              1440.0
#define UTC_ZONE_OFFSET                  -3
#define RA_WORM_RATIO                    96

typedef struct astro_pos {
    uint16_t right_ascension; /** minutes (unit is hours) */
    uint16_t declination; /** minutes (unit is degrees)  */
} astro_pos_t;

typedef struct astro_target {
    astro_pos_t position;
    char name[ASTRO_TARGET_NAME_MAX_CHARACTERS + 1]; /** characters limit + '\0' */
} astro_target_t;

typedef struct mount_data {
    double LMST;
    GNSS_data_t GNSS_data;
    astro_pos_t orientation;
    struct axis {
        stepper_t DEC;
        stepper_t RA;
    } axis_stepper;
} mount_data_t;

void astro_go_to(astro_target_t *target, mount_data_t *EQ_mount);

void astro_start_tracking(mount_data_t *EQ_mount);

void astro_stop_tracking(mount_data_t *EQ_mount);

void update_LST(mount_data_t *EQ_mount);

#endif //EQMOUNT_CUSTOM_CONTROLLER_ASTRO_CONV_H