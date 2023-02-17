//
// Created by paulo on 11/11/2022.
//

#ifndef DEBUG_ASTRO_TARGETS_H
#define DEBUG_ASTRO_TARGETS_H

/**
 * @brief Time position struct.
 */
typedef struct {
	double decimal_hours;
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;
} time__t;

/**
 * @brief Angle storing struct.
 */
typedef struct {
	double decimal_degrees;
	int16_t degrees;
	uint8_t arc_minutes;
	uint8_t arc_seconds;
} angle_t;

/**
 * @brief Standardized position struct.
 */
typedef struct astro_pos {
	time__t right_ascension;
	angle_t declination;
} astro_pos_t;

/**
 * @brief Target struct.
 */
typedef struct astro_target {
	astro_pos_t position;
} astro_target_t;

typedef enum {
	VENUS = 0,
	MARS,
	JUPITER,
	SATURN,
	_TARGET_AMOUNT,
} target_t;

static const astro_target_t Saturn = {
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

static const astro_target_t Jupiter = {
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

static const astro_target_t Mars = {
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

static const astro_target_t Venus = {
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
static const astro_target_t astro_target[_TARGET_AMOUNT] = {
		[VENUS] = Venus,
		[MARS] = Mars,
		[JUPITER] = Jupiter,
		[SATURN] = Saturn,
};

#endif //DEBUG_ASTRO_TARGETS_H
