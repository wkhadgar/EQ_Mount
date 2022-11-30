//
// Created by paulo on 11/11/2022.
//

#ifndef DEBUG_ASTRO_TARGETS_H
#define DEBUG_ASTRO_TARGETS_H

#include "astro_conv.h"

typedef enum {
	VENUS = 0,
	MARS,
	JUPITER,
	SATURN,
	_TARGET_AMOUNT,
} target_t;

const astro_target_t Saturn = {
		.position = {
				.right_ascension = {
						.decimal_hours = 21.4144,
						.hours = 21,
						.minutes = 24,
						.seconds = 52,
				},
				.declination = {
						.decimal_degrees = -16.5075,
						.degrees = -16,
						.arc_minutes = 30,
						.arc_seconds = 27,
				},
		},
};

const astro_target_t Jupiter = {
		.position = {
				.right_ascension = {
						.decimal_hours = 23.94444,
						.hours = 23,
						.minutes = 56,
						.seconds = 40,
				},
				.declination = {
						.decimal_degrees = -1.9,
						.degrees = -1,
						.arc_minutes = 58,
						.arc_seconds = 28,
				},
		},
};

const astro_target_t Mars = {
		.position = {
				.right_ascension = {
						.decimal_hours = 5.3327,
						.hours = 5,
						.minutes = 19,
						.seconds = 58,
				},
				.declination = {
						.decimal_degrees = 24.4841,
						.degrees = 24,
						.arc_minutes = 29,
						.arc_seconds = 3,
				},
		},
};

const astro_target_t Venus = {
		.position = {
				.right_ascension = {
						.decimal_hours = 16.0311,
						.hours = 16,
						.minutes = 1,
						.seconds = 52,
				},
				.declination = {
						.decimal_degrees = -20.5152,
						.degrees = -20,
						.arc_minutes = 30,
						.arc_seconds = 55,
				},
		},
};

/**
 * @brief Control array for easier selection of the ROM targets.
 */
const astro_target_t astro_target[_TARGET_AMOUNT] = {
		[VENUS] = Venus,
		[MARS] = Mars,
		[JUPITER] = Jupiter,
		[SATURN] = Saturn,
};

#endif //DEBUG_ASTRO_TARGETS_H