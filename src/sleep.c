/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

#include <sys/time.h>
#include <unistd.h>

#include "server.h"
#include "log.h"
#include "sleep.h"

int cidle_avg = 0;

long long timel(void) {
    struct timeval tv;

    gettimeofday(&tv, NULL);

    return (long long)tv.tv_sec * (long long)1000000 + (long long)tv.tv_usec;
}

void tick_sleep(int show) {
    struct timeval tv;
    long long now, tosleep, diff;
    static long long next = 0;
    static int nextshow = 0, cidle = 0;

    if (!next) next = timel() + TICK;

    now = timel();

    if (now > next + 1000000) {
        xlog("SERVER TOO SLOW: skipping %.3fs", (now - next) / 1000000.0);
        next = now;
    }

    tosleep = next - now;

    if (tosleep > 80000) {
        xlog("TIME WARP: we're %.3fs ahead? fixing.", tosleep / 1000000.0);
        tosleep = 40000;
        next = now - tosleep;
    }

    // sleep while idle
    if (tosleep > 0) {
        tv.tv_usec = tosleep % 1000000;
        tv.tv_sec = tosleep / 1000000;
        select(0, NULL, NULL, NULL, &tv);

        // how long did we REALLY sleep? (tends to be too long)
        now = timel();
        diff = next - now;
    } else {
        tosleep = 0;
        diff = 0;
    }

    // update statistics
    cidle = 100.0 / TICK * (tosleep - diff);
    cidle_avg = (cidle_avg * 0.0099 + cidle * 0.01) * 100;

    // calculate time for next tick
    next += TICK;

    if (show) {
        if (nextshow) nextshow--;
        else {
            xlog("idle=%6.2f%%.", cidle_avg / 100.0);
            nextshow = TICKS * 3;
        }
    }
}
