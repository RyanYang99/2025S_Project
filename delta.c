#include "leak.h"
#include "delta.h"

#include <time.h>

float delta_time = 0.0f;

void delta_time_update(void) {
    static clock_t last_time = 0;
    clock_t current_time = clock();

    if (last_time == 0)
        last_time = current_time;
    else {
        delta_time = (float)(current_time - last_time) / CLOCKS_PER_SEC;
        last_time = current_time;
    }
}