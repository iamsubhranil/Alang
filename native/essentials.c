#include "../foreign_interface.h"
#include <time.h>

Data Clock(Environment *env){
    return (new_int(clock()));
}
