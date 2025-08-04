#include "leak.h"
#include "date_time.h"

#include <math.h>
#include "delta.h"
#include "console.h"
#include "formatter.h"

date_time_t date_time_elapsed_since_start = {
    .hour = 12
};

void update_date_time(void) {
    //1분 = 1일
    date_time_elapsed_since_start.second += delta_time * (24.0f * 60.0f);

    if (date_time_elapsed_since_start.second >= 60.0f) {
        const int minutes = (int)(date_time_elapsed_since_start.second / 60.0f);

        date_time_elapsed_since_start.minute += minutes;
        date_time_elapsed_since_start.second -= minutes * 60.0f;
    }

    if (date_time_elapsed_since_start.minute >= 60) {
        date_time_elapsed_since_start.hour += date_time_elapsed_since_start.minute / 60;
        date_time_elapsed_since_start.minute %= 60;
    }

    if (date_time_elapsed_since_start.hour >= 24) {
        date_time_elapsed_since_start.day += date_time_elapsed_since_start.hour / 24;
        date_time_elapsed_since_start.hour %= 24;
    }
}

void render_time(void) {
    static float blink = 0.0f;
    static char *pBlink = " ";
    blink += delta_time;
    if (blink >= 2.0f) {
        blink = 0.0f;
        pBlink = " ";
    } else if (blink >= 1.0f)
        pBlink = ":";
    
    char * const pDay = format_string("Day %d", date_time_elapsed_since_start.day),
         * const pTime = format_string("%02d%s%02d", date_time_elapsed_since_start.hour, pBlink, date_time_elapsed_since_start.minute);

    COORD position = {
        .X = (SHORT)(console.size.X - strlen(pDay))
    };
    fprint_string(pDay, position, BACKGROUND_T_BLACK, FOREGROUND_T_WHITE);

    position.X = (SHORT)(console.size.X - strlen(pTime));
    ++position.Y;
    fprint_string(pTime, position, BACKGROUND_T_BLACK, FOREGROUND_T_WHITE);

    free(pDay);
    free(pTime);
}