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
	b_selected,
	b_rotary_trigged,
	_IRQ_vars_amount,
} irq_var_t;

volatile static bool_t gVariables[_IRQ_vars_amount] = {0};

void set_var(irq_var_t gVar);

bool_t get_var(irq_var_t gVar);

void reset_var(irq_var_t gVar);


#endif /* INC_VARIABLES_H_ */
