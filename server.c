/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

#define __USE_BSD_SIGNAL

#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <zlib.h>
#include <malloc.h>
#include <time.h>

#include "server.h"
#include "player.h"
#include "io.h"
#include "notify.h"
#include "libload.h"
#include "tool.h"
#include "sleep.h"
#include "log.h"
#include "create.h"
#include "los.h"
#include "path.h"
#include "timer.h"
#include "effect.h"
#include "database.h"
#include "map.h"
#include "date.h"
#include "container.h"
#include "store.h"
#include "mem.h"
#include "sector.h"
#include "chat.h"
#include "lookup.h"
#include "clan.h"
#include "motd.h"
#include "respawn.h"
#include "consistency.h"
#include "talk.h"
#include "club.h"
#include "version.h"
#include "config.h"

unsigned long long rdtsc(void);

int
    quit = 0, // SIGINT: leave program
    demon = 0, // daemonize server?
    mem_usage = 0, // current memory usage
    ticker = 0, // time counter
    showprof = 0, // SIGTSTP: show profiler output
    sercn = 0, // unique (per server run) ID for characters
    serin = 0, // unique (per server run) ID for items
    online = 0, // number of players online
    areaID = 0, // ID of the area this server is responsible for
    areaM = 0, // our mirror number
    multi = 1, // start extra thread for database accesses
    cycles, // last tick used X cycles
    server_port, // tcp port of this server
    time_now, // current time()
    server_net = 0, // server is supposed to use this network (if multiply interfaces are present)
    server_idle, // idle percentage of this server
    shutdown_at = 0,
    shutdown_down = 0,
    nologin = 0,
    serverID = 0,
    isxmas = 0;

volatile long long
    sent_bytes_raw = 0, // bytes sent including approximation of tcp/ip overhead
    sent_bytes = 0; // bytes sent

volatile long long
    rec_bytes_raw = 0, // bytes sent including approximation of tcp/ip overhead
    rec_bytes = 0; // bytes sent

struct character *ch;
struct item *it;
struct map *map;
struct effect *ef;

void sig_leave(int dummy) {
    quit = 1;
}

void sig_showprof(int dummy) {
    showprof = 1;
}

void tick_char(void);

void *end_of_data_ptr;

void showmem(void) {
    struct mallinfo mi;
    extern int db_store, used_queries;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    mi = mallinfo();
#pragma GCC diagnostic pop

    xlog("-------- Memory usage info: --------");
    xlog("Used characters: %5d/%5d (%4.1f%%)", used_chars, MAXCHARS, 100.0 / MAXCHARS * used_chars);
    xlog("Used items     : %5d/%5d (%4.1f%%)", used_items, MAXITEM, 100.0 / MAXITEM * used_items);
    xlog("Used effects   : %5d/%5d (%4.1f%%)", used_effects, MAXEFFECT, 100.0 / MAXEFFECT * used_effects);
    xlog("Used containers: %5d/%5d (%4.1f%%)", used_containers, MAXCONTAINER, 100.0 / MAXCONTAINER * used_containers);
    xlog("Used timers    : %5d", used_timers);
    xlog("Used notifies  : %5d", used_msgs);
    xlog("Used queries   : %5d", used_queries);
    xlog("Stored Results : %5d", db_store);
    xlog("server used    : %.3fM", mem_usage / 1024.0 / 1024.0);
    xlog("malloc/mmap    : %.3fM in %d blocks", mi.hblkhd / 1024.0 / 1024.0, mi.hblks);
    xlog("malloc/sbrk    : %.3fM", mi.arena / 1024.0 / 1024.0);
    xlog("malloc/total   : %.3fM", (mi.arena + mi.hblkhd) / 1024.0 / 1024.0);
    xlog("malloc/unused  : %.3fM in %d blocks", (mi.fordblks) / 1024.0 / 1024.0, mi.ordblks);
    xlog("brk has grown  : %.3fM", ((int)(sbrk(0)) - (int)(end_of_data_ptr)) / (1024.0 * 1024.0));
    xlog("------------------------------------");
    list_mem();
}

void prof_reset(void);

int maxchars = 0, maxitem = 0, maxeffect = 0;

void show(char *ptr, int size) {
    int n;

    for (n = 0; n < size; n++) {
        if (*ptr < 32) printf(".");
        else printf("%c", *ptr);
        ptr++;
    }
}

int main(int argc, char *args[]) {
    int n, c;
    unsigned long long prof, start, end;

    end_of_data_ptr = sbrk(0);

    time_now = time(NULL);

    printf("\n");
    printf("   ********************************************\n");
    printf("   *     Astonia 3 - The Conflict Server      *\n");
    printf("   *         Version %d.%d Build %-5d          *\n", VERSION >> 24, (VERSION >> 16) & 255, VERSION & 65535);
    printf("   ********************************************\n");
    printf("   * Copyright (c) 2001-2008 Intent Software  *\n");
    printf("   * Copyright (c) 1997-2001 Daniel Brockhaus *\n");
    printf("   ********************************************\n");
    printf("\n");

    if (argc > 1) {
        while (1) {
            c = getopt(argc, args, "a:m:i:w:dhcs:f:e");
            if (c == -1) break;

            switch (c) {
            case 'a':
                if (optarg) areaID = atoi(optarg);
                break;
            case 'm':
                if (optarg) areaM = atoi(optarg);
                break;
            case 'd':
                demon = 1;
                break;
            default:
            case 'h':
                fprintf(stderr, "Usage: %s [-a <areaID>] [-m <mirror>] [-d] [-c] [-s name=value] [-f filename] [-e]\n\n-d Demonize\n-c Disable concurrent database access\n-s Set config name to value (e.g. dbhost=localhost).\n-f Read config file <filename>.\n-e Read configuration from environment variables.\n", args[0]);
                exit(0);
            case 'c':
                multi = 0;
                break;
            case 'i':
                if (optarg) serverID = atoi(optarg);
                break;
            case 's':
                config_string(optarg);
                break;
            case 'f':
                config_file(optarg);
                break;
            case 'e':
                config_getenv();
                break;
            }
        }
    }

    if (!areaID) areaID = 1;
    if (!areaM) areaM = 1;
    if (!serverID) serverID = 1;

    // set character number limit depending on area
    switch (areaID) {
    case 1:
        maxchars = 512;
        break; // cameron
    case 2:
        maxchars = 896;
        break; // below aston
    case 3:
        maxchars = 384;
        break; // aston
    case 4:
        maxchars = 1024;
        break; // epents
    case 5:
        maxchars = 768;
        break; // sewers
    case 6:
        maxchars = 384;
        break; // earth UG
    case 7:
        maxchars = 1024;
        break; // fire pents
    case 8:
        maxchars = 384;
        break; // fire UG
    case 9:
        maxchars = 1024;
        break; // ice pents
    case 10:
        maxchars = 512;
        break; // ice UG
    case 11:
        maxchars = 384;
        break; // ice palace
    case 12:
        maxitem = 1024 * 48;
        maxchars = 384;
        break; // mines
    case 13:
        maxchars = 384;
        break; // clan halls
    case 14:
        maxchars = 1024;
        break; // random dungeons
    case 15:
        maxchars = 384;
        break; // swamp
    case 16:
        maxchars = 384;
        break; // forest
    case 17:
        maxchars = 512;
        break; // exkordon
    case 18:
        maxchars = 768;
        break; // bones and towers
    case 19:
        maxchars = 384;
        break; // nomads
    case 22:
        maxchars = 512;
        break; // labyrinth
    case 23:
        maxchars = 512;
        break; // ice army caves
    case 24:
        maxchars = 384;
        break; // ice army caves
    case 25:
        maxchars = 768;
        break; // RWW
    case 26:
        maxchars = 384;
        break; // below aston 2
    case 28:
        maxchars = 384;
        break; // brannington forest
    case 29:
        maxchars = 512;
        break; // brannington town
    case 30:
        maxchars = 384;
        break; // clan spawners
    case 31:
        maxitem = 1024 * 40;
        maxchars = 512;
        break; // grimroot mines
    case 33:
        maxchars = 1600;
        break; // long tunnel
    case 34:
        maxchars = 1280;
        break;
    case 35:
        maxchars = 768;
        break;
    case 36:
        maxchars = 768;
        break;
    case 37:
        maxchars = 1024;
        break;

    default:
        maxchars = 768;
        break;
    }

    // set item and effect limit
    if (!maxitem) maxitem = max(maxchars * 12 + 10240, 20480);
    if (!maxeffect) maxeffect = max(maxchars * 2, 1024);

    printf("serverID=%d, areaID=%d, areaM=%d, maxchars=%d, maxitem=%d, maxeffect=%d\n\n", serverID, areaID, areaM, maxchars, maxitem, maxeffect);

    if (demon) {
        char buf[80];

        gethostname(buf, 79);
        buf[79] = 0;
        printf("Demonizing (%s)...\n\n", buf);

        if (fork()) exit(0);
        for (n = 0; n < 256; n++) close(n);
        setsid();
        nologin = 1;
    }

    // ignore the silly pipe errors:
    signal(SIGPIPE, SIG_IGN);

    // ignore sighup - just to be safe
    signal(SIGHUP, SIG_IGN);

    // shutdown gracefully if possible:
    signal(SIGQUIT, sig_leave);
    signal(SIGINT, sig_leave);
    signal(SIGTERM, sig_leave);

    // show profile info on CTRL-Z
    signal(SIGTSTP, sig_showprof);

    // init random number generator
    srand(time_now);

    if (!init_smalloc()) exit(1);
    if (!init_mem()) exit(1);
    if (!init_prof()) exit(1);
    if (!init_log()) exit(1);
    if (!init_database()) exit(1);
    if (!init_lookup()) exit(1);
    if (!init_sector()) exit(1);
    if (!init_los()) exit(1);
    if (!init_timer()) exit(1);
    if (!init_notify()) exit(1);
    if (!init_create()) exit(1);
    if (!init_lib()) exit(1);
    if (!init_io()) exit(1);
    if (!init_path()) exit(1);
    if (!init_effect()) exit(1);
    if (!init_container()) exit(1);
    if (!init_store()) exit(1);
    if (!init_chat()) exit(1);

    init_sound_sector();

    xlog("AreaID=%d, AreaM=%d: Init done, entering game loop...", areaID, areaM);

    dlog(0, 0, "Server started");
    prof_reset();

    while (!quit) {
        sprintf(args[0], "./server35 -a %2d -m %2d -i %d # %3d on %5.2f%% load %5.2fM memory (busy)", areaID, areaM, serverID, online, (10000 - server_idle) / 100.0, mem_usage / 1024.0 / 1024.0);
        start = rdtsc();
        lock_server();

        time_now = time(NULL);
        prof = prof_start(26);
        tick_date();
        prof_stop(26, prof);
        prof = prof_start(22);
        tick_timer();
        prof_stop(22, prof);
        prof = prof_start(4);
        tick_char();
        prof_stop(4, prof);
        prof = prof_start(24);
        tick_effect();
        prof_stop(24, prof);
        prof = prof_start(36);
        tick_clan();
        prof_stop(36, prof);
        prof = prof_start(39);
        tick_club();
        prof_stop(39, prof);
        prof = prof_start(5);
        tick_player();
        prof_stop(5, prof);
        prof = prof_start(34);
        tick_login();
        prof_stop(34, prof);
        prof = prof_start(6);
        pflush();
        prof_stop(6, prof);
        prof = prof_start(7);
        io_loop();
        prof_stop(7, prof);
        prof = prof_start(3);
        tick_chat();
        prof_stop(3, prof);

        if (showprof) {
            show_prof();
            showprof = 0;
        }

        prof = prof_start(8);
        prof_update();
        prof_stop(8, prof);

        end = rdtsc();
        cycles = end - start;

        if ((ticker & 2047) == 0) {
            prof = prof_start(27);
            area_alive(0);
            prof_stop(27, prof);
            prof = prof_start(28);
            backup_players();
            prof_stop(28, prof);
            call_stat_update();
            read_motd();
            reinit_log();
        }

        if ((ticker & 255) == 0) {
            call_check_task();
            call_area_load();
            shutdown_warn();
        }

        if ((ticker & 255) == 168) {
            prof = prof_start(38);
            consistency_check_items();
            consistency_check_map();
            consistency_check_chars();
            consistency_check_containers();
            prof_stop(38, prof);
        }

        unlock_server();

        sprintf(args[0], "./server35 -a %2d -m %2d -i %d # %3d on %5.2f%% load %5.2fM memory (idle)", areaID, areaM, serverID, online, (10000 - server_idle) / 100.0, mem_usage / 1024.0 / 1024.0);

        prof = prof_start(1);
        tick_sleep(0);
        prof_stop(1, prof);

        ticker++;
    }

    xlog("Left game loop");
    respawn_check();

    for (n = 1; n < MAXCHARS; n++) {
        if (ch[n].flags & CF_PLAYER) {
            exit_char(n);
        }
    }
    area_alive(1);

    show_prof();

    dlog(0, 0, "Server stopped");

    xlog("map check");
    check_map();

    exit_lib();
    exit_database();

    xlog("Clean shutdown");
    showmem();
    exit_log();
    exit_io();

    return 0;
}

// ****** profiler ********

#ifdef BIGPROF
#define MAXDEPTH 10
#define MAXPROF 1000

struct profile {
    unsigned long long cycles;
    unsigned long long calls;
    unsigned char task[MAXDEPTH];
};

static struct profile prof[MAXPROF];
static unsigned int maxprof = 0;
#endif

struct proftab {
    unsigned long long cycles;
};

static struct proftab proftab[100];

static char *profname[100] = {
    "*ALL*", //0
    "IDLE", //1
    "sector_hear", //2
    "tick_chat", //3
    "tick_char", //4
    "tick_player", //5
    "pflush", //6
    "io_loop", //7
    "prof_update", //8
    "rec_player", //9
    "send_player", //10
    "send()", //11
    "deflate()", //12
    "los_can_see", //13
    "reset_los", //14
    "build_los", //15
    "add_light", //16
    "pathfinder", //17
    "notify_area", //18
    "char_driver", //19
    "notify_char", //20
    "set_sector", //21
    "tick_timer", //22
    "act", //23
    "tick_effect", //24
    "dlog", //25
    "tick_date", //26
    "area_alive", //27
    "backup_players", //28
    "load_player", //29
    "compute_dlight", //30
    "reset_dlight", //31
    "remove_lights", //32
    "add_lights", //33
    "tick_login", //34
    "set_data", //35
    "tick_clan", //36
    "background database", //37
    "consistency", //38
    "tick_club", //39
};

#ifdef BIGPROF
static unsigned char task_stack[MAXDEPTH];
#endif
static int depth = 0;

int init_prof(void) {
    bzero(proftab, sizeof(proftab));
#ifdef BIGPROF
    bzero(prof, sizeof(prof));
#endif

    return 1;
}

unsigned long long prof_start(int task) {
#ifdef BIGPROF
    task_stack[depth] = task;
#endif

    if (depth < 9) depth++;
    else elog("depth overflow (%d)!", task);

    return rdtsc();
}

void prof_stop(int task, unsigned long long cycle) {
    long long td;
#ifdef BIGPROF
    int n;

    task_stack[depth] = 0;
#endif

    depth--;

    td = rdtsc() - cycle;
    if (td < 0 || td > 1000000000) {
        xlog("prof overflow %lld (%llu,%llu) %s %d", td, rdtsc(), cycle, profname[task], task);
        return;
    }

    if (task >= 0 && task < 100) {
        proftab[task].cycles += td;
    }

    if (!depth) {
        proftab[0].cycles += td;
    }

#ifdef BIGPROF
    for (n = 0; n < maxprof; n++)
        if (!memcmp(prof[n].task, task_stack, MAXDEPTH)) break;
    if (n == maxprof) {
        memcpy(prof[n].task, task_stack, MAXDEPTH);
        if (maxprof < MAXPROF - 1) maxprof++;
    }
    prof[n].calls++;
    prof[n].cycles += td;
#endif
}

void prof_update(void) {
    int n;

    for (n = 0; n < 100; n++) {
        proftab[n].cycles = proftab[n].cycles * 0.999;
    }
    server_idle = 10000.0 / proftab[0].cycles * proftab[1].cycles;
}

void prof_reset(void) {
    int n;

    for (n = 0; n < 100; n++) {
        proftab[n].cycles = 0;
    }
}

int profsort(const void *a, const void *b) {
    int n, m;

    n = *(int *)(a);
    m = *(int *)(b);

    if (proftab[m].cycles > proftab[n].cycles) return 1;
    if (proftab[m].cycles < proftab[n].cycles) return -1;

    return 0;
}

#ifdef BIGPROF
int profcomp(const void *va, const void *vb) {
    const struct profile *a = va, *b = vb;
    int n;

    for (n = 0; n < MAXDEPTH; n++) {
        if (a->task[n] < b->task[n]) return -1;
        if (a->task[n] > b->task[n]) return 1;
    }
    return 0;
}
#endif

void show_prof(void) {
    int n, m;
    double proz;
    int tab[100];
#ifdef BIGPROF
    unsigned long long total = 0;
#endif

    for (n = 0; n < 100; n++) {
        tab[n] = n;
    }

    qsort(tab, 100, sizeof(int), profsort);

    xlog("----- Profile: -----");

    for (n = 1; n < 100; n++) {
        m = tab[n];
        if (!proftab[m].cycles) break;
        proz = 100.0 / proftab[0].cycles * proftab[m].cycles;
        if (proz < 0.1) break;
        xlog("%-13.13s %5.2f%%", profname[m], proz);
    }
    xlog("--------------------");
    showmem();
#ifdef BIGPROF

    qsort(prof, maxprof, sizeof(struct profile), profcomp);

    for (n = 0; n < maxprof; n++)
        if (!prof[n].task[1]) total += prof[n].cycles;

    for (n = 0; n < maxprof; n++) {
        for (m = 0; m < 10 && prof[n].task[m]; m++) {
            printf("%s ", profname[prof[n].task[m]]);
        }
        printf("%llu calls, %llu cycles, %.2f%%\n", prof[n].calls, prof[n].cycles, 100.0 / total * prof[n].cycles);
    }

    bzero(prof, sizeof(prof));
    maxprof = 0;
#endif

    xlog("serials: char=%d, item=%d", sercn, serin);
}

void cmd_show_prof(int cn) {
    int n, m;
    double proz;
    int tab[100];

    for (n = 0; n < 100; n++) {
        tab[n] = n;
    }

    qsort(tab, 100, sizeof(int), profsort);

    log_char(cn, LOG_SYSTEM, 0, "--- Profile ---");
    for (n = 1; n < 100; n++) {
        m = tab[n];
        if (!proftab[m].cycles) break;
        proz = 100.0 / proftab[0].cycles * proftab[m].cycles;
        if (proz < 0.5) break;
        log_char(cn, LOG_SYSTEM, 0, "%-13.13s \020%.2f%%", profname[m], proz);
    }
    log_char(cn, LOG_SYSTEM, 0, "---------------");
}
