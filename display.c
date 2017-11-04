#include <stdio.h>
#include <stdlib.h>

#include "display.h"
#include "allocator.h"

static int hasError = 0;

int hadError(){
    return hasError;
}
