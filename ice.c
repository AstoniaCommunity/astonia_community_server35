/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

#include "server.h"
#include "log.h"
#include "talk.h"
#include "database.h"
#include "map.h"
#include "create.h"
#include "drvlib.h"
#include "tool.h"
#include "effect.h"
#include "libload.h"
#include "player_driver.h"

// library helper functions needed for init
int ch_driver(int nr, int cn, int ret, int lastact); // character driver (decides next action)
int it_driver(int nr, int in, int cn); // item driver (special cases for use)
int ch_died_driver(int nr, int cn, int co); // called when a character dies
int ch_respawn_driver(int nr, int cn); // called when an NPC is about to respawn

// EXPORTED - character/item driver
int driver(int type, int nr, int obj, int ret, int lastact) {
    switch (type) {
    case CDT_DRIVER:
        return ch_driver(nr, obj, ret, lastact);
    case CDT_ITEM:
        return it_driver(nr, obj, ret);
    case CDT_DEAD:
        return ch_died_driver(nr, obj, ret);
    case CDT_RESPAWN:
        return ch_respawn_driver(nr, obj);
    default:
        return 0;
    }
}

#include "ice_shared.c"

int ch_driver(int nr, int cn, int ret, int lastact) {
    switch (nr) {

    default:
        return 0;
    }
}

int it_driver(int nr, int in, int cn) {
    switch (nr) {
    case IDR_ITEMSPAWN:
        itemspawn(in, cn);
        return 1;
    case IDR_MELTINGKEY:
        meltingkey(in, cn);
        return 1;
    case IDR_WARMFIRE:
        warmfire(in, cn);
        return 1;
    case IDR_BACKTOFIRE:
        backtofire(in, cn);
        return 1;
    case IDR_FREAKDOOR:
        freakdoor(in, cn);
        return 1;

    default:
        return 0;
    }
}

int ch_died_driver(int nr, int cn, int co) {
    switch (nr) {
    default:
        return 0;
    }
}
int ch_respawn_driver(int nr, int cn) {
    switch (nr) {
    default:
        return 0;
    }
}
