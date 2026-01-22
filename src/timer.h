/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

struct timer {
    int due;

    void (*func)(int, int, int, int, int);
    int dat1, dat2, dat3, dat4, dat5;

    struct timer *next;
};

extern int used_timers;

int init_timer(void);
void tick_timer(void);
int set_timer(int due, void (*func)(int, int, int, int, int), int dat1, int dat2, int dat3, int dat4, int dat5);
