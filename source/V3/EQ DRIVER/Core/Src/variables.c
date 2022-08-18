#include "variables.h"

void set_var(irq_var_t gVar)	{
	if (!gVariables[gVar]) gVariables[gVar] = 1;
	return;
}

bool_t get_var(irq_var_t gVar)	{
	return gVariables[gVar];
}
void reset_var(irq_var_t gVar)	{
	if (gVariables[gVar]) gVariables[gVar] = 0;
	return;
}
