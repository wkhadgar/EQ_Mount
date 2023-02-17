//
// Created by paulo on 22/09/2022.
//

#ifndef EQMOUNT_CUSTOM_CONTROLLER_ASTRO_CONV_H
#define EQMOUNT_CUSTOM_CONTROLLER_ASTRO_CONV_H

#include "steppers.h"
#include "astro_targets.h"
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

typedef enum {
	STOPPED = 0,
	TRACKING,
	GOING_TO,
} movement_t;

/**
 * @brief Initializes the astro modules.
 */
void astro_init(void);

/**
 * @brief Calculates LST by estimating GMST and shifting it to the current longitude.
 *
 * @note Pseudo code for GMST calculation:
 *  midnight = floor(2458484.833333) + 0.5            // J0
 *	days_since_midnight = 2458484.833333 - 2458484.5  //
 *	hours_since_midnight = days_since_midnight * 24   // H
 *	days_since_epoch = 2458484.833333 - 2451545.0     // D
 *	centuries_since_epoch = days_since_epoch / 36525  // T
 *	whole_days_since_epoch = 2458484.5 - 2451545.0    // D0
 *
 *	GMST = 6.697374558 + 0.06570982441908 * whole_days_since_epoch + 1.00273790935 * hours_since_midnight
 *		 + 0.000026 * centuries_since_epoch^2
 */
void astro_update_LMST(void);

/**
 * @brief Checks whether the mount has reached the target position.
 *
 * @return bool false if the mount is not at the target position, true otherwise.
 */
bool astro_is_at_target(void);

/**
 * @brief Updates the value from the adjust knob.
 */
void astro_update_raw_fine_adjusts(void);

/**
 * @brief Sets a new target to the mount, overwriting the previous one.
 *
 * @param target The target to be tracked.
 */
void astro_update_target(astro_target_t target);

/**
 * @brief Gives the pointer to the GNSS structure.
 *
 * @return GNSS_data_t [out] Pointer to the GNSS data struct.
 */
GNSS_data_t* astro_get_gnss_pointer(void);

/**
 * @brief Starts the movement of the mount, tracking the target.
 */
void astro_start_tracking(void);

/**
 * @brief Starts the movement of the mount, going to the target.
 */
void astro_goto_target(void);

/**
 * @brief Starts the movement of the mount, going to home, being the LST RA.
 */
void astro_go_home(void);

/**
 * @brief Completely stops the movement of the mount.
 */
void astro_full_stop(void);

/**
 * @brief Free both motors to be moved barehanded.
 */
void astro_release(void);

/**
 * @brief Locks both motors back.
 */
void astro_engage(void);

/**
 * @brief ISR to update the RA stepper related changes.
 *
 * @note Must be called from timer ARR IRQ, as soon as the ARR reloads, to update the future PWM period.
 */
void astro_RA_position_step_isr(void);

/**
 * @brief ISR to update the DEC stepper related changes.
 *
 * @note Must be called from timer ARR IRQ, as soon as the ARR reloads, to update the future PWM period.
 */
void astro_DEC_position_step_isr(void);

#endif //EQMOUNT_CUSTOM_CONTROLLER_ASTRO_CONV_H
