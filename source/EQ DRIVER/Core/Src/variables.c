#include "variables.h"

void set_flag(bool_var_t flag_var) {
    boolVariables[flag_var] = 1;
}

void reset_flag(bool_var_t flag_var) {
    boolVariables[flag_var] = 0;
}

bool get_flag(bool_var_t flag_var) {
    return boolVariables[flag_var];
}
