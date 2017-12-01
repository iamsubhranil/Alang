#include <math.h>

#include "../foreign_interface.h"

Data Sin(Environment *env){
    return new_float(sin(get_double("x",  env)));
}

Data Cos(Environment *env){
    return new_float(cos(get_double("x",  env)));
}

Data Tan(Environment *env){
    return new_float(tan(get_double("x",  env)));
}

Data ASin(Environment *env){
    return new_float(asin(get_double("x",  env)));
}
Data ACos(Environment *env){
    return new_float(acos(get_double("x",  env)));
}

Data ATan(Environment *env){
    return new_float(atan(get_double("x",  env)));
}

Data Sinh(Environment *env){
    return new_float(sinh(get_double("x",  env)));
}

Data Cosh(Environment *env){
    return new_float(cosh(get_double("x",  env)));
}

Data Tanh(Environment *env){
    return new_float(tanh(get_double("x",  env)));
}

Data Log(Environment *env){
    return new_float(log(get_double("x",  env)));
}

Data Log10(Environment *env){
    return new_float(log10(get_double("x",  env)));
}

Data Exp(Environment *env){
    return new_float(exp(get_double("x",  env)));
}
