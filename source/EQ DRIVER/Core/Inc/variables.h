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
	IRQ_vars_amount,
} bool_var_t;

typedef enum {
	rot_value,
	INT_vars_amount,
} int_var_t;

volatile static bool boolVariables[IRQ_vars_amount] = {0};
volatile static int intVariables[INT_vars_amount] = {0};

/**
 * @brief: gets the boolean value of a flag.
 * @param flag_var: enum declared value of the flag.
 * @return True if flag_var is True, False otherwise.
 */
bool get_flag(bool_var_t flag_var);

/**
 * @brief: sets a given boolean to True.
 * @param flag_var: enum declared value of the flag.
 */
void set_flag(bool_var_t flag_var);

/**
 * @brief: sets a given boolean to False.
 * @param flag_var: enum declared value of the flag.
 */
void reset_flag(bool_var_t flag_var);

/**
 * @brief: increment a int enum declared var and returns its new value.
 * @param int_var: enum declared int.
 * @param increment value to increment the int_var.
 * @return new int_var value.
 */
int incremented_var(int_var_t int_var, int8_t increment);

#endif /* INC_VARIABLES_H_ */