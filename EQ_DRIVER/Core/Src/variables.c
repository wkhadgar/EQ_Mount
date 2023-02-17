#include "variables.h"

static volatile bool boolVariables[IRQ_vars_amount];

void set_flag(bool_var_t flag_var) {
	boolVariables[flag_var] = true;
}

void reset_flag(bool_var_t flag_var) {
	boolVariables[flag_var] = false;
}

bool get_flag(bool_var_t flag_var) {
	return boolVariables[flag_var];
}