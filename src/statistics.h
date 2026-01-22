/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

#define MAXSTAT 365
#define RESOLUTION (60 * 60 * 24)

struct stats {
    int exp;
    int gold;
    int online;
};

struct stats_ppd {
    int last_update;
    struct stats stats[MAXSTAT];
};

void stats_update(int cn, int onl, int gold);
int stats_online_time(int cn);
