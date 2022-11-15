//
// Created by paulo on 11/11/2022.
//

#ifndef DEBUG_ASTRO_TARGETS_H
#define DEBUG_ASTRO_TARGETS_H

#include "astro_conv.h"

const astro_target_t teste_target = {
		.name = "Alvo de teste",
		.position = {
				.right_ascension = {
						.decimal_hours = 20.50,
						.hours = 20,
						.minutes = 30,
						.seconds = 0,
				},
				.declination = {
						.decimal_degrees = 90.0,
						.degrees = 90,
						.arc_minutes = 0,
						.arc_seconds = 0,
				},
		},
};

#endif //DEBUG_ASTRO_TARGETS_H
