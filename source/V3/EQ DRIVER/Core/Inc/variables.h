/*
 * variables.h
 *
 *  Created on: Aug 18, 2022
 *      Author: paulo
 */

#ifndef INC_VARIABLES_H_
#define INC_VARIABLES_H_

#include "main.h"

typedef enum {
	selected,
	rotary_triggered,
	ccw,
	update_display,
	wake,
	sleep,
	low_battery,
	toggle_horse,
	on_menu,
	_IRQ_vars_amount,
} bool_var_t;

typedef enum {
	rot_value,
	_INT_vars_amount,
} int_var_t;

volatile static bool_t boolVariables[_IRQ_vars_amount] = { 0 };
volatile static int intVariables[_INT_vars_amount] = { 0 };

bool_t get_flag(bool_var_t bool_var);

void set_flag(bool_var_t bool_var);

void reset_flag(bool_var_t bool_var);

int incremented_var(int_var_t int_var, int8_t increment);

#endif /* INC_VARIABLES_H_ */
