/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

#include "server.h"
#include "notify.h"
#include "do.h"
#include "drdata.h"
#include "effect.h"
#include "tool.h"
#include "drvlib.h"
#include "player.h"
#include "lostcon.h"
#include "spell.h"

// library helper functions needed for init
int ch_driver(int nr, int cn, int ret, int lastact); // character driver (decides next action)
int it_driver(int nr, int in, int cn); // item driver (special cases for use)
int ch_died_driver(int nr, int cn); // called when a character dies

// EXPORTED - character/item driver
int driver(int type, int nr, int obj, int ret, int lastact) {
    switch (type) {
    case 0:
        return ch_driver(nr, obj, ret, lastact);
    case 1:
        return it_driver(nr, obj, ret);
    case 2:
        return ch_died_driver(nr, obj);
    default:
        return 0;
    }
}

//-----------------------

struct lostcon_driver_data {
    int timeout;
};

void lostcon_driver(int cn, int ret, int lastact) {
    struct lostcon_driver_data *dat;
    struct lostcon_ppd *ppd;
    int co, n, in, fn, m, arena = 0;
    struct msg *msg, *next;

    dat = set_data(cn, DRD_LOSTCONDRIVER, sizeof(struct lostcon_driver_data));
    if (!dat) return; // oops...

    ppd = set_data(cn, DRD_LOSTCON_PPD, sizeof(struct lostcon_ppd));
    if (!ppd) return; // oops...

    if (!dat->timeout) {
        dat->timeout = ticker + LAGOUT_TIME;
        fight_driver_reset(cn);
    }

    if (ch[cn].x > 1 && ch[cn].x < MAXMAP && ch[cn].y > 1 && ch[cn].y < MAXMAP) {
        m = ch[cn].x + ch[cn].y * MAXMAP;
        if (!ch[cn].player && (map[m].flags & MF_RESTAREA)) { // leave at once if at rest area
            exit_char(cn);
            return;
        }
        if (map[m].flags & MF_ARENA) arena = 1;
    }

    // leave if timeout is reached
    if (!ch[cn].player && ticker > dat->timeout) {
        exit_char(cn);
        return;
    }

    // loop through our messages
    for (msg = ch[cn].msg; msg; msg = next) {
        next = msg->next;

        // did we see someone?
        if (msg->type == NT_CHAR) {

            co = msg->dat1;
        }

        // did we get hit?
        if (msg->type == NT_GOTHIT) {

            fight_driver_note_hit(cn);

            co = msg->dat1;
            if (co) fight_driver_add_enemy(cn, co, 1, 1);
        }

        // did we hear something?
        if (msg->type == NT_TEXT) {
            if (msg->dat1 == 1 && msg->dat3 != cn) { // someone (not us) is talking
            }
        }

        remove_message(cn, msg);
    }

    // do something. whenever possible, call do_idle with as high a tick count
    // as reasonable when doing nothing.

    for (n = 0; n < 4; n++) {
        if (!(fn = ch[cn].ef[n])) continue;
        if (ef[fn].type == EF_LAG) break;
    }
    if (n == 4) create_show_effect(EF_LAG, cn, ticker, ticker + TICKS, 0, 1);
    else ef[fn].stop = ticker + TICKS;

    // low on hp?
    if (!arena && ch[cn].hp < ch[cn].value[0][V_HP] * (int)(POWERSCALE * 0.66) && !has_spell(cn, IDR_HEAL)) {
        // try heal
        if (ch[cn].mana > ch[cn].value[0][V_MANA] * POWERSCALE / 2 && do_heal(cn, cn)) return;
    }

    // low on mana?
    if (!arena && ch[cn].mana < ch[cn].value[0][V_MANA] * POWERSCALE / 4 && !has_spell(cn, IDR_HEAL)) {
        // use potion
        if (!ppd->nomana || !ppd->nocombo) {
            for (n = 30; n < INVENTORYSIZE; n++) {
                if ((in = ch[cn].item[n]) && it[in].driver == IDR_POTION && it[in].drdata[2]) {
                    if (it[in].drdata[1] && !ppd->nocombo) {
                        use_item(cn, in);
                        break;
                    }
                    if (!it[in].drdata[1] && !ppd->nomana) {
                        use_item(cn, in);
                        break;
                    }
                }
            }
        }
    }

    // low on magic shield?

    fight_driver_update(cn);
    if (fight_driver_attack_visible(cn, 0)) return;
    if (!arena && fight_driver_follow_invisible(cn)) return;

    if (ch[cn].value[0][V_BLESS] && ch[cn].mana >= BLESSCOST && may_add_spell(cn, IDR_BLESS) && do_bless(cn, cn)) return;

    if (!arena) {
        if (!ppd->noshield && ch[cn].value[0][V_MAGICSHIELD] * POWERSCALE / MAGICSHIELDMOD > ch[cn].lifeshield && ch[cn].mana >= POWERSCALE * 3 && do_magicshield(cn)) return;
        if (ch[cn].value[0][V_HEAL] && ch[cn].hp < ch[cn].value[0][V_HP] * POWERSCALE / 2 && ch[cn].mana >= POWERSCALE * 3 && do_heal(cn, cn)) return;
    }

    do_idle(cn, TICKS);
}

void lostcon_dead(int cn) {
    struct lostcon_driver_data *dat;

    dat = set_data(cn, DRD_LOSTCONDRIVER, sizeof(struct lostcon_driver_data));
    if (!dat) return; // oops...

    dat->timeout = 0;
}

int ch_driver(int nr, int cn, int ret, int lastact) {
    switch (nr) {
    case CDR_LOSTCON:
        lostcon_driver(cn, ret, lastact);
        return 1;
    default:
        return 0;
    }
}

int it_driver(int nr, int in, int cn) {
    switch (nr) {
    default:
        return 0;
    }
}

int ch_died_driver(int nr, int cn) {
    switch (nr) {
    case CDR_LOSTCON:
        lostcon_dead(cn);
        return 1;
    default:
        return 0;
    }
}
