#include "env.h"
#include "values.h"

Record *freeRecords[] = {0};
uint16_t freeRecordPointer = 0;

Instance *freeInstances[] = {0};
uint16_t freeInstancePointer = 0;

Environment *freeEnvironments[] = {0};
uint16_t freeEnvironmentPointer = 0;

static inline void free_all(){
    uint16_t i = 0;
    while(i < freeRecordPointer)
        memfree(freeRecords[i++]);
    i = 0;
    while(i < freeInstancePointer)
        memfree(freeInstances[i++]);
    i = 0;
    while(i < freeEnvironmentPointer)
        memfree(freeEnvironments[i++]);
}
