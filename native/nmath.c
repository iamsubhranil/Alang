#include <math.h>

#include "../foreign_interface.h"

Object Sin(int i, Environment *env){
    return fromDouble(sin(get_double("x", i, env)));
}

Object Cos(int i, Environment *env){
    return fromDouble(cos(get_double("x", i, env)));
}

Object Tan(int i, Environment *env){
    return fromDouble(tan(get_double("x", i, env)));
}

Object ASin(int i, Environment *env){
    return fromDouble(asin(get_double("x", i, env)));
}
Object ACos(int i, Environment *env){
    return fromDouble(acos(get_double("x", i, env)));
}

Object ATan(int i, Environment *env){
    return fromDouble(atan(get_double("x", i, env)));
}

Object Sinh(int i, Environment *env){
    return fromDouble(sinh(get_double("x", i, env)));
}

Object Cosh(int i, Environment *env){
    return fromDouble(cosh(get_double("x", i, env)));
}

Object Tanh(int i, Environment *env){
    return fromDouble(tanh(get_double("x", i, env)));
}

Object Log(int i, Environment *env){
    return fromDouble(log(get_double("x", i, env)));
}

Object Log10(int i, Environment *env){
    return fromDouble(log10(get_double("x", i, env)));
}

Object Exp(int i, Environment *env){
    return fromDouble(exp(get_double("x", i, env)));
}
