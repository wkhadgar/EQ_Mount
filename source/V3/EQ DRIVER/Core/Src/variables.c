#include "variables.h"

void set_flag(bool_var_t bool_var) {
	boolVariables[bool_var] = 1;
	return;
}

void reset_flag(bool_var_t bool_var) {
	boolVariables[bool_var] = 0;
	return;
}

bool_t get_flag(bool_var_t bool_var) {
	return boolVariables[bool_var];
}

int increment_var(int_var_t int_var, int8_t increment) {
	return (intVariables[int_var] += increment);
}