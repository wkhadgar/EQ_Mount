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
    NRF_SENT,
    NRF_RECEIVE,
	NRF_MAX_RT,
	IRQ_vars_amount,
} bool_var_t;

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
void clear_flag(bool_var_t flag_var);

#endif /* INC_VARIABLES_H_ */