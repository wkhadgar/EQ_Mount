#include "variables.h"

void set_var(irq_var_t gVar)	{
	gVariables[gVar] = 1;
	return;
}

void reset_var(irq_var_t gVar)	{
	gVariables[gVar] = 0;
	return;
}

bool_t get_var(irq_var_t gVar)	{
	return gVariables[gVar];
}
