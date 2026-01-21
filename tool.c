/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#include "server.h"
#include "log.h"
#include "talk.h"
#include "direction.h"
#include "create.h"
#include "skill.h"
#include "player.h"
#include "drdata.h"
#include "drvlib.h"
#include "item_id.h"
#include "effect.h"
#include "timer.h"
#include "date.h"
#include "lookup.h"
#include "clan.h"
#include "do.h"
#include "chat.h"
#include "path.h"
#include "map.h"
#include "notify.h"
#include "tool.h"
#include "shrine.h"
#include "military.h"
#include "prof.h"
#include "act.h"
#include "poison.h"
#include "statistics.h"
#include "balance.h"
#include "area.h"
#include "depot.h"
#include "lab.h"
#include "database.h"
#include "consistency.h"
#include "sector.h"
#include "lostcon.h"
#include "death.h"
#include "club.h"
#include "btrace.h"
#include "spell.h"

int _get_complexity_cost(struct item *in);

// compare the ending of two strings
int endcmp(char *a, char *b) {
    char *starta = a, *startb = b;
    int lena, lenb;

    while (*a) a++;
    while (*b) b++;

    lena = a - starta;
    lenb = b - startb;

    if (lenb > lena) return 0;

    a -= lenb;
    b -= lenb;

    return !strcmp(a, b);
}

// translate character speed into ticks
int speed(int speedy, int mode, int ticks) {
    double f;

    if (speedy > 0) speedy /= 2;
    else speedy = speedy * 0.75;

    if (mode == SM_FAST) speedy += 40;
    if (mode == SM_STEALTH) speedy -= 40;

    f = 0.75 + speedy / 288.0;

    if (f < 0.2) f = 0.2;
    if (f > 2.0) f = 2.0;

    ticks /= f;

    if (ticks < 2) return 2;
    if (ticks > 255) return 255;

    return ticks;
}

int speed2(int speedy, int mode, int ticks) {
    double f;

    if (speedy > 0) speedy /= 2;
    else speedy = speedy * 0.75;

    if (mode == SM_FAST) speedy += 40;
    if (mode == SM_STEALTH) speedy -= 40;

    f = 0.75 + speedy / 288.0;

    if (f < 0.2) f = 0.2;
    if (f > 2.0) f = 2.0;

    ticks = ceil(ticks * f);

    if (ticks < 2) return 2;
    if (ticks > 255) return 255;

    return ticks;
}

// translate direction into map offset. sets diag if the
// direction is diagonal.
int dx2offset(int dir, int *x, int *y, int *diag) {
    switch (dir) {
    case DX_UP:
        if (x) *x = 0;
        if (y) *y = -1;
        if (diag) *diag = 0;
        break;
    case DX_DOWN:
        if (x) *x = 0;
        if (y) *y = 1;
        if (diag) *diag = 0;
        break;
    case DX_LEFT:
        if (x) *x = -1;
        if (y) *y = 0;
        if (diag) *diag = 0;
        break;
    case DX_RIGHT:
        if (x) *x = 1;
        if (y) *y = 0;
        if (diag) *diag = 0;
        break;
    case DX_LEFTUP:
        if (x) *x = -1;
        if (y) *y = -1;
        if (diag) *diag = 1;
        break;
    case DX_RIGHTUP:
        if (x) *x = 1;
        if (y) *y = -1;
        if (diag) *diag = 1;
        break;
    case DX_LEFTDOWN:
        if (x) *x = -1;
        if (y) *y = 1;
        if (diag) *diag = 1;
        break;
    case DX_RIGHTDOWN:
        if (x) *x = 1;
        if (y) *y = 1;
        if (diag) *diag = 1;
        break;
    default:
        return 0;
    }
    return 1;
}

// returns the direction pointing from frx,fry to tox,toy
int offset2dx(int frx, int fry, int tox, int toy) {
    int dx, dy;

    dx = tox - frx;
    dy = toy - fry;

    if (abs(dx) / 2 > abs(dy)) dy = 0;
    if (abs(dy) / 2 > abs(dx)) dx = 0;

    if (dx > 0) {
        if (dy > 0) return DX_RIGHTDOWN;
        if (dy < 0) return DX_RIGHTUP;
        return DX_RIGHT;
    }

    if (dx < 0) {
        if (dy > 0) return DX_LEFTDOWN;
        if (dy < 0) return DX_LEFTUP;
        return DX_LEFT;
    }
    if (dy > 0) return DX_DOWN;
    if (dy < 0) return DX_UP;

    return 0;
}

static int arena_check_target(int m) {
    if (!(map[m].flags & MF_ARENA)) return 0;

    return 1;
}

int can_help(int cn, int co) {
    int m;
    unsigned long long mf1, mf2;

    if (cn < 0 || cn >= MAXCHARS) {
        elog("can_help(): called with illegal character cn=%d", cn);
        return 0;
    }

    if (co < 1 || co >= MAXCHARS) {
        elog("can_help(): called with illegal character co=%d (cn=%s,%d)", co, ch[cn].name, cn);
        return 0;
    }

    if (ch[co].flags & CF_DEAD) return 0;

    if (cn == co) return 1;
    if (!cn) return 1;

    m = ch[cn].x + ch[cn].y * MAXMAP;
    mf1 = map[m].flags;

    m = ch[co].x + ch[co].y * MAXMAP;
    mf2 = map[m].flags;

    // no help unless neither is in an arena or both are in the same arena
    if ((mf1 & MF_ARENA) && (mf2 & MF_ARENA) && pathfinder(ch[cn].x, ch[cn].y, ch[co].x, ch[co].y, 0, arena_check_target, 0) != -1) return 1; // both in same
    if ((mf1 & MF_ARENA) || (mf2 & MF_ARENA)) return 0; // one in arena or both in different arenas

    // everything else is fair game
    return 1;
}

// check if cn may attack co. cn may be zero, in that case, almost all
// attacks are legal.
int can_attack(int cn, int co) {
    int c1, c2, m;
    unsigned long long mf1, mf2;

    if (cn < 0 || cn >= MAXCHARS) {
        elog("can_attack(): called with illegal character cn=%d", cn);
        return 0;
    }

    if (co < 1 || co >= MAXCHARS) {
        elog("can_attack(): called with illegal character co=%d (cn=%s,%d)", co, ch[cn].name, cn);
        {
            char *ptr = NULL;
            *ptr = 0;
        }
        return 0;
    }

    if (!(ch[co].flags)) return 0;

    if (ch[co].flags & (CF_DEAD | CF_NOATTACK)) return 0;

    if (cn == co) return 0;

    if (!cn) return 1;

    // players may not attack characters with CF_NOPLRATT set
    if ((ch[cn].flags & (CF_PLAYER | CF_PLAYERLIKE)) && (ch[co].flags & CF_NOPLRATT)) return 0;
    if ((ch[co].flags & (CF_PLAYER | CF_PLAYERLIKE)) && (ch[cn].flags & CF_NOPLRATT)) return 0;

    m = ch[cn].x + ch[cn].y * MAXMAP;
    mf1 = map[m].flags;

    m = ch[co].x + ch[co].y * MAXMAP;
    mf2 = map[m].flags;

    // check if one of them is in a peace-zone
    if ((mf1 & MF_PEACE) || (mf2 & MF_PEACE)) return 0;

    // everything's fair in the arena
    if ((mf1 & MF_ARENA) && (mf2 & MF_ARENA) && pathfinder(ch[cn].x, ch[cn].y, ch[co].x, ch[co].y, 0, arena_check_target, 0) != -1) {
        // unless its a clan-area too, then no attacking of same clan or allied clan
        if ((mf1 & MF_CLAN) && (mf2 & MF_CLAN) && (c1 = get_char_clan(cn)) && (c2 = get_char_clan(co)) && (c1 == c2 || clan_alliance(c1, c2))) return 0;
        return 1;
    }

    // clans may attack under special circumstances
    if ((c1 = get_char_clan(cn)) && (c2 = get_char_clan(co)) && areaID != 1) {
        if ((mf1 & MF_CLAN) && (mf2 & MF_CLAN) && clan_can_attack_inside(c1, c2)) return 1;
        if (clan_can_attack_outside(c1, c2) && abs(ch[cn].level - ch[co].level) <= 3) return 1;
    }

    // one of them is 'player-like', dont allow attacks between him and players/player-likes
    if ((ch[cn].flags & (CF_PLAYER | CF_PLAYERLIKE)) && (ch[co].flags & CF_PLAYERLIKE)) return 0;
    if ((ch[co].flags & (CF_PLAYER | CF_PLAYERLIKE)) && (ch[cn].flags & CF_PLAYERLIKE)) return 0;

    // player against player. nope.
    if ((ch[cn].flags & CF_PLAYER) && (ch[co].flags & CF_PLAYER)) return 0;

    // at least one NPC is fighting, unless they're of the same group...
    if (ch[cn].group == ch[co].group) return 0;

    // ... clans members may not attack each other if they're of the same clan or of an alliance...
    if ((c1 = get_char_clan(cn)) && (c2 = get_char_clan(co))) {
        if (c1 == c2) return 0;
        if (clan_alliance(c1, c2)) return 0;
    }

    // ... everything else is fair game
    return 1;
}

int check_requirements(int cn, int in) {
    int v1, v2, m;

    // check requirements
    for (m = 0; m < MAXMOD; m++) {
        if (!(v2 = it[in].mod_value[m])) continue; // ignore empty ones
        v1 = it[in].mod_index[m];
        if (v1 <= -V_MAX || v1 >= V_MAX) {
            elog("can_wear(): item %s (%d) has illegal modifier (index %d out of bounds). Removing modifier.", it[in].name, in, v1);
            it[in].mod_value[m] = 0;
            it[in].mod_index[m] = 0;
            continue;
        }
        if (v1 < 0) { // negative indizes are requirements
            if (ch[cn].value[1][-v1] < v2) return 0;
        }
    }

    if (it[in].min_level && ch[cn].level < it[in].min_level) return 0;
    if (it[in].max_level && ch[cn].level > it[in].max_level) return 0;

    if ((it[in].needs_class & 1) && !(ch[cn].flags & CF_WARRIOR)) return 0;
    if ((it[in].needs_class & 2) && (!(ch[cn].flags & CF_MAGE) || (ch[cn].flags & CF_WARRIOR))) return 0;
    if ((it[in].needs_class & 4) && (ch[cn].flags & (CF_MAGE | CF_WARRIOR)) != (CF_MAGE | CF_WARRIOR)) return 0;
    if ((it[in].needs_class & 8) && !(ch[cn].flags & CF_ARCH)) return 0;

    if ((it[in].flags & IF_BONDWEAR) && it[in].ownerID != ch[cn].ID) return 0;

    return 1;
}

// check if the character can wear the item at the given position
int can_wear(int cn, int in, int pos) {
    int inr, inl;

    if (cn < 1 || cn >= MAXCHARS) {
        elog("can_wear(): called with illegal character cn=%d", cn);
        return 0;
    }

    if (in < 1 || in >= MAXITEM) {
        elog("can_wear(): called with illegal item in=%d", in);
        return 0;
    }

    if (pos < 0 || pos > 11) {
        elog("can_wear(): called with illegal pos pos=%d", pos);
        return 0;
    }

    // check position
    inl = ch[cn].item[WN_LHAND];
    if (inl < 0 || inl >= MAXITEM) {
        elog("can_wear(): illegal item %d in WN_LHAND of character %s (%d) found. Removing.", in, ch[cn].name, cn);
        inl = ch[cn].item[WN_LHAND] = 0;
    }

    inr = ch[cn].item[WN_RHAND];
    if (inr < 0 || inr >= MAXITEM) {
        elog("can_wear(): illegal item %d in WN_RHAND of character %s (%d) found. Removing.", in, ch[cn].name, cn);
        inl = ch[cn].item[WN_RHAND] = 0;
    }

    switch (pos) {
    case WN_HEAD:
        if (!(it[in].flags & IF_WNHEAD)) return 0;
        break;
    case WN_NECK:
        if (!(it[in].flags & IF_WNNECK)) return 0;
        break;
    case WN_BODY:
        if (!(it[in].flags & IF_WNBODY)) return 0;
        break;
    case WN_ARMS:
        if (!(it[in].flags & IF_WNARMS)) return 0;
        break;
    case WN_BELT:
        if (!(it[in].flags & IF_WNBELT)) return 0;
        break;
    case WN_LEGS:
        if (!(it[in].flags & IF_WNLEGS)) return 0;
        break;
    case WN_FEET:
        if (!(it[in].flags & IF_WNFEET)) return 0;
        break;
    case WN_CLOAK:
        if (!(it[in].flags & IF_WNCLOAK)) return 0;
        break;
    case WN_LRING:
        if (!(it[in].flags & IF_WNLRING)) return 0;
        break;
    case WN_RRING:
        if (!(it[in].flags & IF_WNRRING)) return 0;
        break;

    case WN_LHAND:
        if (inr && (it[inr].flags & IF_WNTWOHANDED)) return 0; // left hand is blocked by two-handed weapon
        if (!(it[in].flags & IF_WNLHAND)) return 0;
        break;

    case WN_RHAND:
        if (it[in].flags & IF_WNTWOHANDED) { // is it a two-handed weapon?
            if (inl) return 0; // yes, but if left hand is full, its still illegal
        } else if (!(it[in].flags & IF_WNRHAND)) return 0; // no, but can it be worn on right hand?
        break;
    }

    return check_requirements(cn, in);
}

// check if the character can carry the item
int can_carry(int cn, int in, int quiet) {
    int n, in2;

    if (cn < 1 || cn >= MAXCHARS) {
        elog("can_carry(): called with illegal character cn=%d", cn);
        return 0;
    }

    if (in < 1 || in >= MAXITEM) {
        elog("can_carry(): called with illegal item in=%d", in);
        return 0;
    }

    if (IDR_ONECARRY(it[in].driver)) {
        for (n = 0; n < INVENTORYSIZE; n++) {
            if (n >= 12 && n < 30) continue;
            if ((in2 = ch[cn].item[n]) && it[in2].driver == it[in].driver) break;
        }
        if (n < INVENTORYSIZE || ((in2 = ch[cn].citem) && it[in2].driver == it[in].driver)) {
            if (!quiet) log_char(cn, LOG_SYSTEM, 0, "You can only carry one %s at a time!", it[in].name);
            return 0;
        }
    }

    if ((it[in].flags & IF_BONDTAKE) && it[in].ownerID != ch[cn].ID) return 0;

    return 1;
}

int die(int cnt, int sides) {
    int val = 0, n;

    if (sides < 1) return 0;

    for (n = 0; n < cnt; n++) {
        val += RANDOM(sides) + 1;
    }

    return val;
}

// X exp yield level Y
int exp2level(int val) {
    return max(1, (int)(sqrt(sqrt(val))));
}

// to reach level X you need Y exp
int level2exp(int level) {
    return pow(level, 4);
}

// to reach level X you need Y exp
int level_value(int level) {
    return pow(level + 1, 4) - pow(level, 4);
}

char *save_number(int nr) {
    static char buf[80];

    switch (nr) {
    case 0:
        return "no";
    case 1:
        return "one";
    case 2:
        return "two";
    case 3:
        return "three";
    case 4:
        return "four";
    case 5:
        return "five";
    case 6:
        return "six";
    case 7:
        return "seven";
    case 8:
        return "eight";
    case 9:
        return "nine";
    case 10:
        return "ten";
    default:
        sprintf(buf, "%d", nr);
        return buf;
    }
}

int check_levelup(int cn) {
    char buf[256];
    int experience, flag = 0;

    experience = max(ch[cn].exp, ch[cn].exp_used);

    while (exp2level(experience) > ch[cn].level) {
        ch[cn].level++;
        set_sector(ch[cn].x, ch[cn].y);
        log_char(cn, LOG_SYSTEM, 0, "Thou gained a level! Thou art level %d now.", ch[cn].level);

        if (ch[cn].saves < 3) {
            ch[cn].saves++;

            log_char(cn, LOG_SYSTEM, 0, "Thy persistence has pleased Ishtar. He will save thee when thou art in need.");
            log_char(cn, LOG_SYSTEM, 0, "Thou hast %s saves now.", save_number(ch[cn].saves));
        } else log_char(cn, LOG_SYSTEM, 0, "Thou already hast 3 saves. Thou cannot get any more.");

        if (ch[cn].level >= 20 && !ch[cn].value[1][V_PROFESSION]) {
            ch[cn].value[1][V_PROFESSION] = 1;
            log_char(cn, LOG_SYSTEM, 0, "Thou mayest now choose to learn a profession.");
        }

        if (ch[cn].level % 10 == 0) {
            sprintf(buf, "0000000000\260c10Grats: %s is level %d now!", ch[cn].name, ch[cn].level);
            server_chat(6, buf);
        }

        reset_name(cn);

        dlog(cn, 0, "gained a level");
        flag = 1;
    }
    if (flag) {
        sprintf(buf, "accounts_level_%d", (ch[cn].level / 10) * 10);
        summary(buf, ch[cn].sID);
    }
    return flag;
}

// low level routine to give exp to a character
// MUST USE
void give_exp(int cn, int val) {
    if (ch[cn].exp < 200 * 200 * 200 * 200) ch[cn].exp += val;

    check_levelup(cn);
}

void give_exp_bonus(int cn, int val) {
    give_exp(cn, val);
}

void give_money(int cn, int val, char *reason) {
    ch[cn].gold += val;
    stats_update(cn, 0, val);
    ch[cn].flags |= CF_ITEMS;
    if (val % 100) log_char(cn, LOG_SYSTEM, 0, "You received %dG %dS.", val / 100, val % 100);
    else log_char(cn, LOG_SYSTEM, 0, "You received %dG.", val / 100);
    dlog(cn, 0, "received %dG, %dS for %s.", val / 100, val % 100, reason);
}

const char *hisname(int cn) {
    if (ch[cn].flags & CF_MALE) return "his";
    if (ch[cn].flags & CF_FEMALE) return "her";
    return "its";
}

const char *Hisname(int cn) {
    if (ch[cn].flags & CF_MALE) return "His";
    if (ch[cn].flags & CF_FEMALE) return "Her";
    return "Its";
}

const char *hename(int cn) {
    if (ch[cn].flags & CF_MALE) return "he";
    if (ch[cn].flags & CF_FEMALE) return "she";
    return "it";
}

const char *Hename(int cn) {
    if (ch[cn].flags & CF_MALE) return "He";
    if (ch[cn].flags & CF_FEMALE) return "She";
    return "It";
}

const char *himname(int cn) {
    if (ch[cn].flags & CF_MALE) return "him";
    if (ch[cn].flags & CF_FEMALE) return "her";
    return "it";
}

const char *Sirname(int cn) {
    if (ch[cn].flags & CF_MALE) return "Sir";
    if (ch[cn].flags & CF_FEMALE) return "Lady";
    return "Neuter";
}

void remove_spell(int cn, int in, int pos, int cserial, int iserial) {
    int old_speed;

    // char alive and the right one?
    if (!(ch[cn].flags) || ch[cn].serial != cserial) {
        return;
    }

    // item existant and the right one?
    if (!(it[in].flags) || it[in].serial != iserial) {
        return;
    }

    // item where we expect it to be?
    if (ch[cn].item[pos] != in) {
        return;
    }

    if (ticker < *(int *)(it[in].drdata)) {
        return;
    }

    old_speed = ch[cn].value[0][V_SPEED];

    ch[cn].item[pos] = 0;
    free_item(in);

    update_char(cn);

    if (it[in].driver == IDR_FREEZE && ch[cn].duration) { // adjust duration and step to remove effect of freeze
        int old_dur, new_dur, real_dur;

        old_dur = ch[cn].duration;
        real_dur = speed2(old_speed, ch[cn].speed_mode, ch[cn].duration);
        new_dur = speed(ch[cn].value[0][V_SPEED], ch[cn].speed_mode, real_dur);

        ch[cn].duration = new_dur;
        ch[cn].step = ch[cn].step * new_dur / old_dur;
    }
}

void heal_tick(int cn, int in, int pos, int cserial, int iserial) {
    int val;

    // char alive and the right one?
    if (!(ch[cn].flags) || ch[cn].serial != cserial) {
        return;
    }

    // item existant and the right one?
    if (!(it[in].flags) || it[in].serial != iserial) {
        return;
    }

    // item where we expect it to be?
    if (ch[cn].item[pos] != in) {
        return;
    }

    val = *(int *)(it[in].drdata + 8) * HEALTICK / HEALDURATION;
    ch[cn].hp = min(ch[cn].hp + val, ch[cn].value[0][V_HP] * POWERSCALE);

    val = *(int *)(it[in].drdata + 12) * HEALTICK / HEALDURATION;
    ch[cn].endurance = min(ch[cn].endurance + val, ch[cn].value[0][V_ENDURANCE] * POWERSCALE);

    val = *(int *)(it[in].drdata + 16) * HEALTICK / HEALDURATION;
    ch[cn].mana = min(ch[cn].mana + val, ch[cn].value[0][V_MANA] * POWERSCALE);

    if (ticker + HEALTICK < *(int *)(it[in].drdata))
        set_timer(ticker + HEALTICK, heal_tick, cn, in, pos, cserial, iserial);

    set_sector(ch[cn].x, ch[cn].y); // display new health bar right away
}

// checks if a spell with the same driver is already working
// returns 0 if that is the case, or if the character does not
// have a free spell slot
// returns the number of the spell slot otherwise
int may_add_spell(int cn, int driver) {
    int n, fre = 0, in;

    for (n = 12; n < 30; n++) {
        if ((in = ch[cn].item[n])) {
            if (it[in].driver == driver) { // another spell with the same driver is already active
                if (driver == IDR_BLESS && *(unsigned *)(it[in].drdata) - ticker < TICKS * 30) return n;
                return 0;
            }
        } else fre = n;
    }
    if (fre == 0) return 0;

    return fre;
}

int has_spell(int cn, int driver) {
    int n, in;

    for (n = 12; n < 30; n++)
        if ((in = ch[cn].item[n]) && it[in].driver == driver) return n;

    return 0;
}

int add_same_spell(int cn, int driver) {
    int n, fre = 0, in;

    for (n = 12; n < 30; n++) {
        if ((in = ch[cn].item[n])) {
            if (it[in].driver == driver) return n; // such a spell is already active, return its number
        } else fre = n;
    }
    if (fre == 0) return 0;

    return fre;
}

int add_spell(int cn, int driver, int duration, char *name) {
    int in, fre = -1;

    if (!(fre = may_add_spell(cn, driver))) return 0;

    in = create_item(name);
    if (!in) return 0;

    it[in].driver = driver;

    *(int *)(it[in].drdata) = ticker + duration;
    *(int *)(it[in].drdata + 4) = ticker;

    it[in].carried = cn;

    ch[cn].item[fre] = in;

    create_spell_timer(cn, in, fre);

    update_char(cn);

    return in;
}

char *bignumber(int val) {
    static char num[10][40];
    static int cur = 0;

    cur = (cur + 1) % 10;

    if (val >= 1000000000) sprintf(num[cur], "%d,%03d,%03d,%03d", val / 1000000000, (val % 1000000000) / 1000000, (val % 1000000) / 1000, (val % 1000));
    else if (val >= 1000000) sprintf(num[cur], "%d,%03d,%03d", val / 1000000, (val % 1000000) / 1000, (val % 1000));
    else if (val >= 1000) sprintf(num[cur], "%d,%03d", val / 1000, (val % 1000));
    else sprintf(num[cur], "%d", val);

    return num[cur];
}

int look_item(int cn, struct item *in, int slot) {
    int n, v, m = 0, r = 0, s, ve, cnt = 0;
    double effectivity;

    if (!in->name[0]) return 1; // no name means we dont display anything

    // send slot number for client version > 0
    if (get_char_version(cn) > 0) log_char(cn, LOG_SYSTEM, 0, "\260\260\260ITEMDESC%04d\260\260\260", slot);

    log_char(cn, LOG_SYSTEM, 0, "\260c5%s:", in->name);
    if (in->description[0]) log_char(cn, LOG_SYSTEM, 0, "%s", in->description);
    if (in->ID == IID_HARDKILL) log_char(cn, LOG_SYSTEM, 0, "This is a level %d holy weapon.", in->drdata[37]);

    if (in->complexity > 0) effectivity = (double)in->quality / in->complexity;
    else effectivity = 1.0;

    for (n = 0; n < MAXMOD; n++) {
        if ((v = in->mod_value[n]) && (s = in->mod_index[n]) > -1) {
            if (s != V_ARMOR && s != V_WEAPON && s != V_LIGHT && s != V_SPEED && s != V_DEMON && s != V_COLD) ve = (int)round(v * effectivity);
            else ve = v;

            if (!m) {
                log_char(cn, LOG_SYSTEM, 0, "\260c5Modifiers:");
                m = 1;
            }
            if (in->driver == IDR_DECAYITEM) {
                log_char(cn, LOG_SYSTEM, 0, "%s +%d (active: %+d)", skill[s].name, v, in->drdata[2]);
            } else {
                if (ch[cn].flags & CF_GOD) {
                    if (s == V_ARMOR || s == V_WEAPON) log_char(cn, LOG_SYSTEM, 0, "%s %+.2f \260c1(%d: %d+%d)", skill[s].name, v / 20.0, n, s, v);
                    else if (ve != v && s != V_DEFENSE && s != V_OFFENSE && s != V_SPEED) {
                        cnt++;
                        log_char(cn, LOG_SYSTEM, 0, "%s %+d (base: %+d) \260c1(%d: %d+%d)", skill[s].name, ve, v, n, s, v);
                    } else log_char(cn, LOG_SYSTEM, 0, "%s %+d \260c1(%d: %d+%d)", skill[s].name, v, n, s, v);
                } else {
                    if (s == V_ARMOR || s == V_WEAPON) log_char(cn, LOG_SYSTEM, 0, "%s %+.2f", skill[s].name, v / 20.0);
                    else if (ve != v && s != V_DEFENSE && s != V_OFFENSE && s != V_SPEED) {
                        cnt++;
                        log_char(cn, LOG_SYSTEM, 0, "%s %+d (base: %+d)", skill[s].name, ve, v);
                    } else log_char(cn, LOG_SYSTEM, 0, "%s %+d", skill[s].name, v);
                }
            }
        }
    }

    for (n = 0; n < MAXMOD; n++) {
        if ((v = in->mod_value[n]) && (s = in->mod_index[n]) < 0) {
            if (!r) {
                log_char(cn, LOG_SYSTEM, 0, "\260c5Requirements:");
                r = 1;
            }
            log_char(cn, LOG_SYSTEM, 0, "%s %d (you have %d)", skill[-s].name, v, ch[cn].value[1][-s]);
        }
    }
    if (!r && (in->min_level || in->max_level || in->needs_class)) {
        log_char(cn, LOG_SYSTEM, 0, "\260c5Requirements:");
        r = 1;
    }
    if (in->min_level) log_char(cn, LOG_SYSTEM, 0, "Minimum Level: %d", in->min_level);
    if (in->max_level) log_char(cn, LOG_SYSTEM, 0, "Maximum Level: %d", in->max_level);

    if (in->needs_class & 1) log_char(cn, LOG_SYSTEM, 0, "Only usable by a Warrior.");
    if (in->needs_class & 2) log_char(cn, LOG_SYSTEM, 0, "Only usable by a Mage.");
    if (in->needs_class & 4) log_char(cn, LOG_SYSTEM, 0, "Only usable by a Seyan'Du.");
    if (in->needs_class & 8) log_char(cn, LOG_SYSTEM, 0, "Only usable by an Arch.");

    if (in->flags & IF_BONDTAKE) log_char(cn, LOG_SYSTEM, 0, "This item is bonded to %s. Only %s can take it.", in->ownerID == ch[cn].ID ? "you" : "somebody else", in->ownerID == ch[cn].ID ? "you" : "he");
    if (in->flags & IF_BONDWEAR) log_char(cn, LOG_SYSTEM, 0, "This item is bonded to %s. Only %s can wear it.", in->ownerID == ch[cn].ID ? "you" : "somebody else", in->ownerID == ch[cn].ID ? "you" : "he");

    if (in->flags & IF_QUEST) log_char(cn, LOG_SYSTEM, 0, "This is a quest item. You cannot drop it or give it away.");
    if (in->flags & IF_NOENHANCE) log_char(cn, LOG_SYSTEM, 0, "This item resists magic, so you cannot enhance it using orbs or shrines.");

    if (in->driver == IDR_FLASK && in->drdata[2]) {
        log_char(cn, LOG_SYSTEM, 0, "Duration: %d minutes.", in->drdata[3]);
    }
    if (in->driver == IDR_DECAYITEM) {
        log_char(cn, LOG_SYSTEM, 0, "Duration: %d:%02d:%02d of %d:%02d:%02d active time used up.",
                 ((*(unsigned short *)(in->drdata + 3)) * 2) / 60 / 60, (((*(unsigned short *)(in->drdata + 3)) * 2) / 60) % 60, ((*(unsigned short *)(in->drdata + 3)) * 2) % 60,
                 ((*(unsigned short *)(in->drdata + 5)) * 2) / 60 / 60, (((*(unsigned short *)(in->drdata + 5)) * 2) / 60) % 60, ((*(unsigned short *)(in->drdata + 5)) * 2) % 60);
    }

    if (in->complexity) {
        if (in->driver == IDR_ENCHANTITEM || in->ID == IID_ALCHEMY_INGREDIENT) {
            log_char(cn, LOG_SYSTEM, 0, "\260c5Magical Structure:");
            log_char(cn, LOG_SYSTEM, 0, "Strength: %s, Complexity: %s, (Quality: %.0f%%)", bignumber(in->quality), bignumber(in->complexity), in->quality * 100.0 / in->complexity);
        } else {
            log_char(cn, LOG_SYSTEM, 0, "\260c5Magical Structure:");
            log_char(cn, LOG_SYSTEM, 0, "Strength: %s, Complexity: %s, (Quality %.0f%%, Effectivity: %.0f%%)", bignumber(in->quality), bignumber(in->complexity), 200.0 - 100.0 / _get_complexity_cost(in) * in->complexity, effectivity * 100.0);
        }
    }

    if (in->value) {
        if (in->value > 1000 * 100) log_char(cn, LOG_SYSTEM, 0, "Base Value: %sG", bignumber(in->value / 100));
        else if (in->value % 100) log_char(cn, LOG_SYSTEM, 0, "Base Value: %.2fG", in->value / 100.0);
        else log_char(cn, LOG_SYSTEM, 0, "Base Value: %dG", in->value / 100);
    }

    if (get_char_version(cn) > 0 && slot != -1) log_char(cn, LOG_SYSTEM, 0, "\260c5.");

    return 1;
}

static char *rankname[] = {
    "nobody", //0
    "Private", //1
    "Private First Class", //2
    "Lance Corporal", //3
    "Corporal", //4
    "Sergeant", //5	lvl 30
    "Staff Sergeant", //6
    "Master Sergeant", //7
    "First Sergeant", //8	lvl 45
    "Sergeant Major", //9
    "Second Lieutenant", //10	lvl 55
    "First Lieutenant", //11
    "Captain", //12
    "Major", //13
    "Lieutenant Colonel", //14
    "Colonel", //15
    "Brigadier General", //16
    "Major General", //17
    "Lieutenant General", //18
    "General", //19
    "Field Marshal", //20 	lvl 105
    "Knight of Astonia", //21
    "Baron of Astonia", //22
    "Earl of Astonia", //23
    "Warlord of Astonia" //24	lvl 125
};

struct rank_ppd {
    int army_rank;
};

char *get_title(int co) {
    if (!(ch[co].flags & CF_WON)) return "";

    if (ch[co].flags & CF_FEMALE) return "Lady ";
    else return "Sir ";
}

int look_char(int cn, int co) {
    struct rank_ppd *ppd;
    struct shrine_ppd *shrine;
    char buf[1024];
    int len, cnt;

    if (ch[cn].flags & CF_PLAYER) {
        shrine = set_data(co, DRD_RANDOMSHRINE_PPD, sizeof(struct shrine_ppd));
        if (shrine && shrine->used[DEATH_SHRINE_INDEX] & DEATH_SHRINE_BIT) {
            log_char(cn, LOG_SYSTEM, 0, "#1%s%s the Brave (%d):", get_title(co), ch[co].name, ch[co].level);
        } else {
            log_char(cn, LOG_SYSTEM, 0, "#1%s%s (%d):", get_title(co), ch[co].name, ch[co].level);
        }

        len = 0;
        len += sprintf(buf + len, "#2%s ", ch[co].description);
        if (ch[co].flags & CF_PLAYER) {
            len += sprintf(buf + len, "%s has %d saves, was saved %d times and died %d times. ", Hename(co), ch[co].saves, ch[co].got_saved, ch[co].deaths);
            if ((cnt = count_solved_labs(co))) len += sprintf(buf + len, "%s solved %d parts of the labyrinth. ", ch[co].name, cnt);

            if (check_first_kill(co, 403)) len += sprintf(buf + len, "%s has mastered Hell. ", Hename(co));
            else if (check_first_kill(co, 395) || check_first_kill(co, 396) ||
                     check_first_kill(co, 397) || check_first_kill(co, 398) ||
                     check_first_kill(co, 399) || check_first_kill(co, 400) ||
                     check_first_kill(co, 401) || check_first_kill(co, 402)) len += sprintf(buf + len, "%s has ventured into Hell. ", Hename(co));
            else if (check_first_kill(co, 388) || check_first_kill(co, 389) ||
                     check_first_kill(co, 390) || check_first_kill(co, 391) ||
                     check_first_kill(co, 392) || check_first_kill(co, 393) ||
                     check_first_kill(co, 394)) len += sprintf(buf + len, "%s got %s nose burned in Hell. ", Hename(co), hisname(co));
        }

        plr_send_inv(cn, co);

        len += show_prof_info(cn, co, buf + len);

        if (ch[co].flags & (CF_PLAYER | CF_PLAYERLIKE)) {
            if (!(ppd = set_data(co, DRD_RANK_PPD, sizeof(struct rank_ppd)))) return 0; // OOPS

            if (ppd->army_rank) {
                len += sprintf(buf + len, "%s is a %s in the Imperial Army. ", ch[co].name, rankname[ppd->army_rank]);
            }
            len += show_clan_info(cn, co, buf + len);
            len += show_club_info(cn, co, buf + len);
        }
        log_char(cn, LOG_SYSTEM, 0, "%s", buf);
    }
    if (ch[cn].flags & CF_GOD) {
        int n, in, fn;

        for (n = 12; n < 30; n++) {
            if ((in = ch[co].item[n])) look_item(cn, it + in, -1);
        }

        for (n = 0; n < 4; n++) {
            if (!(fn = ch[co].ef[n])) continue;
            log_char(cn, LOG_SYSTEM, 0, "slot %d, effect type %d", n, ef[fn].type);
        }
    }

    return 1;
}

int set_army_rank(int cn, int rank) {
    struct rank_ppd *ppd;

    if (!(ppd = set_data(cn, DRD_RANK_PPD, sizeof(struct rank_ppd)))) return 0; // OOPS

    ppd->army_rank = min(24, rank);

    return 1;
}

int get_army_rank_int(int cn) {
    struct rank_ppd *ppd;

    if (!(ppd = set_data(cn, DRD_RANK_PPD, sizeof(struct rank_ppd)))) return 0; // OOPS

    if (ppd->army_rank > 24) ppd->army_rank = 24;

    return ppd->army_rank;
}

char *get_army_rank_string(int cn) {
    struct rank_ppd *ppd;

    if (!(ppd = set_data(cn, DRD_RANK_PPD, sizeof(struct rank_ppd)))) return ""; // OOPS

    return rankname[min(24, ppd->army_rank)];
}

int bigdir(int dir) {
    switch (dir) {
    case DX_RIGHTUP:
        return DX_RIGHT;
    case DX_RIGHTDOWN:
        return DX_RIGHT;
    case DX_LEFTUP:
        return DX_LEFT;
    case DX_LEFTDOWN:
        return DX_LEFT;

    default:
        return dir;
    }
}

int is_facing(int cn, int co) {
    int ok = 0;

    switch (ch[cn].dir) {
    case DX_RIGHT:
        if (ch[cn].x + 1 == ch[co].x && ch[cn].y == ch[co].y) ok = 1;
        break;
    case DX_LEFT:
        if (ch[cn].x - 1 == ch[co].x && ch[cn].y == ch[co].y) ok = 1;
        break;
    case DX_UP:
        if (ch[cn].x == ch[co].x && ch[cn].y - 1 == ch[co].y) ok = 1;
        break;
    case DX_DOWN:
        if (ch[cn].x == ch[co].x && ch[cn].y + 1 == ch[co].y) ok = 1;
        break;
    default:
        break;
    }

    return ok;
}

int is_back(int cn, int co) {
    int ok = 0;

    switch (ch[cn].dir) {
    case DX_LEFT:
        if (ch[cn].x + 1 == ch[co].x && ch[cn].y == ch[co].y) ok = 1;
        break;
    case DX_RIGHT:
        if (ch[cn].x - 1 == ch[co].x && ch[cn].y == ch[co].y) ok = 1;
        break;
    case DX_DOWN:
        if (ch[cn].x == ch[co].x && ch[cn].y - 1 == ch[co].y) ok = 1;
        break;
    case DX_UP:
        if (ch[cn].x == ch[co].x && ch[cn].y + 1 == ch[co].y) ok = 1;
        break;
    default:
        break;
    }

    return ok;
}

// create an item representing amount silver coins
int create_money_item(int amount) {
    int in;

    in = create_item("money");
    if (!in) return 0;

    if (amount > 9999999) {
        it[in].sprite = 109;
    } else if (amount > 999999) {
        it[in].sprite = 108;
    } else if (amount > 99999) {
        it[in].sprite = 107;
    } else if (amount > 9999) {
        it[in].sprite = 106;
    } else if (amount > 999) {
        it[in].sprite = 105;
    } else if (amount > 99) {
        it[in].sprite = 104;
    } else if (amount > 9) {
        it[in].sprite = 103;
    } else if (amount > 2) {
        it[in].sprite = 102;
    } else if (amount == 2) {
        it[in].sprite = 101;
    } else if (amount == 1) {
        it[in].sprite = 100;
    }
    sprintf(it[in].description, "%.2fG.", amount / 100.0);

    it[in].value = amount;

    return in;
}

// destroy money item and return the amount of silver coins it represented
int destroy_money_item(int in) {
    int value;

    value = it[in].value;

    free_item(in);

    return value;
}

char *strcasestr(const char *haystack, const char *needle) {
    const char *ptr;

    for (ptr = needle; *haystack; haystack++) {
        if (toupper(*ptr) == toupper(*haystack)) {
            ptr++;
            if (!*ptr) return (char *)(haystack + (needle - ptr + 1));
        } else ptr = needle;
    }
    return NULL;
}

#include "special_item.c"

void set_item_requirements_sub(int in) {
    int lvl, sum, high, n;

    for (n = sum = high = 0; n < MAXMOD; n++) {
        switch (it[in].mod_index[n]) {
        case V_WEAPON:
            break;
        case V_ARMOR:
            break;
        case V_SPEED:
            break;
        case V_DEMON:
            break;
        case V_LIGHT:
            break;
        case V_COLD:
            break;
        default:
            if (it[in].mod_index[n] >= 0 && it[in].mod_value[n] > 0) {
                high = max(high, it[in].mod_value[n]);
                sum += it[in].mod_value[n];
            }
            break;
        }
    }
    switch (high) {
    case 0:
        lvl = 0;
        break;
    case 1:
        lvl = 2;
        break;
    case 2:
        lvl = 3;
        break;
    case 3:
        lvl = 5;
        break;
    case 4:
        lvl = 10;
        break;
    case 5:
        lvl = 15;
        break;
    case 6:
        lvl = 17;
        break;
    case 7:
        lvl = 20;
        break;
    case 8:
        lvl = 23;
        break;
    case 9:
        lvl = 26;
        break;
    case 10:
        lvl = 30;
        it[in].needs_class |= 8;
        break;
    case 11:
        lvl = 33;
        it[in].needs_class |= 8;
        break;
    case 12:
        lvl = 36;
        it[in].needs_class |= 8;
        break;
    case 13:
        lvl = 40;
        it[in].needs_class |= 8;
        break;
    case 14:
        lvl = 43;
        it[in].needs_class |= 8;
        break;
    case 15:
        lvl = 46;
        it[in].needs_class |= 8;
        break;
    case 16:
        lvl = 50;
        it[in].needs_class |= 8;
        break;
    case 17:
        lvl = 53;
        it[in].needs_class |= 8;
        break;
    case 18:
        lvl = 56;
        it[in].needs_class |= 8;
        break;
    case 19:
        lvl = 60;
        it[in].needs_class |= 8;
        break;
    case 20:
        lvl = 63;
        it[in].needs_class |= 8;
        break;
    case 21:
        lvl = 66;
        it[in].needs_class |= 8;
        break;
    case 22:
        lvl = 70;
        it[in].needs_class |= 8;
        break;
    case 23:
        lvl = 73;
        it[in].needs_class |= 8;
        break;
    default:
        lvl = 76;
        it[in].needs_class |= 8;
        break;
    }

    lvl += sum - high;
    it[in].min_level = lvl;
}

int level2maxitem(int level) {

    if (level < 2) return 0;
    if (level < 3) return 1;
    if (level < 5) return 2;
    if (level < 10) return 3;
    if (level < 15) return 4;
    if (level < 17) return 5;
    if (level < 20) return 6;
    if (level < 23) return 7;
    if (level < 26) return 8;
    if (level < 30) return 9;
    if (level < 33) return 10;
    if (level < 36) return 11;
    if (level < 40) return 12;
    if (level < 43) return 13;
    if (level < 46) return 14;
    if (level < 50) return 15;
    if (level < 53) return 16;
    if (level < 56) return 17;
    if (level < 60) return 18;
    if (level < 63) return 19;

    return 20;
}

void set_item_requirements(int in) {
    set_item_requirements_sub(in);
}

void set_legacy_item_lvl(int in, int cn) {
    if (it[in].flags & (IF_WNHEAD | IF_WNNECK | IF_WNBODY | IF_WNARMS | IF_WNBELT | IF_WNLEGS | IF_WNFEET | IF_WNLHAND | IF_WNRHAND | IF_WNCLOAK | IF_WNLRING | IF_WNRRING | IF_WNTWOHANDED))
        set_item_requirements_sub(in);
}

int find_item_type(int in) {
    int n, m, s, dead, cnt, need;

    for (n = 0; n < ARRAYSIZE(special_item); n++) {
        for (m = dead = cnt = 0; m < MAXMOD; m++) {
            if ((s = it[in].mod_index[m]) < 0) continue;
            if (it[in].mod_value[m] < 1) continue;
            if (s == V_ARMOR || s == V_WEAPON || s == V_LIGHT || s == V_SPEED || s == V_DEMON || s == V_COLD) continue;
            if (special_item[n].mod_index[0] != s &&
                special_item[n].mod_index[1] != s &&
                special_item[n].mod_index[2] != s) {
                dead = 1;
                break;
            }
            cnt++;
        }
        if (special_item[n].mod_index[0] != -1) need = 1;
        else need = 0;
        if (special_item[n].mod_index[1] != -1) need++;
        if (special_item[n].mod_index[2] != -1) need++;

        if (!dead && cnt == need) {
            return n;
        }
    }
    return -1;
}

char *find_item_strength_text(int in) {
    int m, str, v, s;

    for (m = str = 0; m < MAXMOD; m++) {
        if ((s = it[in].mod_index[m]) < 0) continue;
        if ((v = it[in].mod_value[m]) < 1) continue;
        if (s == V_ARMOR || s == V_WEAPON || s == V_LIGHT || s == V_SPEED || s == V_DEMON || s == V_COLD) continue;
        str = max(str, v);
    }
    return item_strength_text(str);
}

int set_item_name(int in) {
    char *str_desc;
    int idx;

    if (it[in].min_level == 0) {
        snprintf(it[in].description, sizeof(it[0].description) - 1, "A plain %s.", it[in].name);
        it[in].description[sizeof(it[0].description) - 1] = 0;
        return 1;
    }

    idx = find_item_type(in);
    if (idx == -1) return 0;

    str_desc = find_item_strength_text(in);

    snprintf(it[in].description, sizeof(it[0].description) - 1, "%s%s of %s%s.",
             str_desc,
             it[in].name,
             special_item[idx].the ? "the " : "",
             special_item[idx].name);

    it[in].description[sizeof(it[0].description) - 1] = 0;
    return 1;
}

// potionprob: 1 (0%), 2(50%), 10(90%), ...
// maxchance: 10000 (normal), 1000 (no junk), 250 (better), 50 (good), 10 (real good), 1 (absolutely cool)
int create_special_item(int strength, int potionprob, int maxchance) {
    int in, n, tot, cnt, m, nr;
    char buf[256];
    int type, len;
    char *str_desc;

    if (RANDOM(potionprob)) {
        strength += RANDOM(4);
        if (strength < 6) n = 1;
        else if (strength < 9) n = 2;
        else n = 3;

        switch (RANDOM(3)) {
        case 0:
            sprintf(buf, "healing_potion%d", n);
            break;
        case 1:
            sprintf(buf, "mana_potion%d", n);
            break;
        case 2:
            sprintf(buf, "combo_potion%d", n);
            break;
        }
        in = create_item(buf);
        return in;
    }

    type = RANDOM(6);

    switch (type) {
    case 0:
        sprintf(buf, "plain_gold_ring");
        break;
    case 1:
        sprintf(buf, "plain_gold_ring");
        break;
    case 2:
        sprintf(buf, "blue_cape");
        break;
    case 3:
        sprintf(buf, "belt");
        break;
    case 4:
        sprintf(buf, "amulet");
        break;
    case 5:
        sprintf(buf, "boots");
        break;
    default:
        return 0;
    }

    in = create_item(buf);
    if (!in) return 0;

    // strength
    strength = 1; // !! fixme someday !!
    str_desc = item_strength_text(strength);

    // find all fitting items
    for (nr = tot = 0; nr < ARRAYSIZE(special_item); nr++) {
        if (special_item[nr].needflag && (it[in].flags & IF_WEAPON) && !(it[in].flags & special_item[nr].needflag)) continue;
        if (maxchance < special_item[nr].chance) continue;
        tot += special_item[nr].chance;
    }
    if (!tot) return 0;

    // roll dice
    cnt = RANDOM(tot);

    // find correct item
    for (nr = tot = 0; nr < ARRAYSIZE(special_item); nr++) {
        if (special_item[nr].needflag && (it[in].flags & IF_WEAPON) && !(it[in].flags & special_item[nr].needflag)) continue;
        if (maxchance < special_item[nr].chance) continue;
        tot += special_item[nr].chance;
        if (tot > cnt) break;
    }
    if (nr == ARRAYSIZE(special_item)) {
        elog("create_special_item(): dice wrong");
        return 0;
    }

    len = snprintf(buf, sizeof(it[0].description) - 1, "%s%s of %s%s.",
                   str_desc,
                   it[in].name,
                   special_item[nr].the ? "the " : "",
                   special_item[nr].name);

    if (len < 3 || len == sizeof(it[0].description) - 1) return 0;

    for (n = m = 0; n < MAXMOD && m < 3; n++) {
        if (special_item[nr].mod_index[m] == -1) break;

        if (!it[in].mod_index[n]) { // add modifier
            it[in].mod_index[n] = special_item[nr].mod_index[m];
            it[in].mod_value[n] = strength;
            m++;
        }
    }

    // change description
    strcpy(it[in].description, buf);

    // add min_level and make arch-only if str>10
    set_item_requirements(in);
    calculate_complexity(in);
    it[in].value = it[in].quality * 10;

    // note change
    it[in].ID = IID_GENERIC_SPECIAL;

    return in;
}

// gives non-gauss random values from 1 to val (inclusive)
// low numbers are a lot more probable. max value is 215
int lowhi_random(int val) {
    int p;

    p = sqrt(sqrt(RANDOM((val + 1) * (val + 1) * (val + 1) * (val + 1) - 1) + 1));

    return val - p + 1;
}

int create_spell_timer(int cn, int in, int pos) {
    switch (it[in].driver) {
    case IDR_BLESS:
        create_show_effect(EF_BLESS, cn, *(int *)(it[in].drdata + 4), *(int *)(it[in].drdata), 0, it[in].mod_value[0]);
        set_timer(*(unsigned int *)(it[in].drdata), remove_spell, cn, in, pos, ch[cn].serial, it[in].serial);
        break;
    case IDR_WARCRY:
        create_show_effect(EF_WARCRY, cn, *(int *)(it[in].drdata + 4), *(int *)(it[in].drdata), 0, 0);
        set_timer(*(unsigned int *)(it[in].drdata), remove_spell, cn, in, pos, ch[cn].serial, it[in].serial);
        break;
    case IDR_FREEZE:
        create_show_effect(EF_FREEZE, cn, *(int *)(it[in].drdata + 4), *(int *)(it[in].drdata), 0, 0);
        set_timer(*(unsigned int *)(it[in].drdata), remove_spell, cn, in, pos, ch[cn].serial, it[in].serial);
        break;
    case IDR_FLASH:
        set_timer(*(unsigned int *)(it[in].drdata), remove_spell, cn, in, pos, ch[cn].serial, it[in].serial);
        break;
    case IDR_ARMOR:
    case IDR_WEAPON:
    case IDR_HP:
    case IDR_MANA:
        set_timer(*(unsigned int *)(it[in].drdata), remove_spell, cn, in, pos, ch[cn].serial, it[in].serial);
        break;
    case IDR_POTION_SP:
        create_show_effect(EF_POTION, cn, *(int *)(it[in].drdata + 4), *(int *)(it[in].drdata), 0, it[in].mod_value[0]);
        set_timer(*(unsigned int *)(it[in].drdata), remove_spell, cn, in, pos, ch[cn].serial, it[in].serial);
        break;

    case IDR_CURSE:
        create_show_effect(EF_CURSE, cn, *(int *)(it[in].drdata + 4), *(int *)(it[in].drdata), 0, -it[in].mod_value[0]);
        set_timer(*(unsigned int *)(it[in].drdata), remove_spell, cn, in, pos, ch[cn].serial, it[in].serial);
        break;

    case IDR_POISON0:
    case IDR_POISON1:
    case IDR_POISON2:
    case IDR_POISON3:
        set_timer(*(unsigned int *)(it[in].drdata), remove_spell, cn, in, pos, ch[cn].serial, it[in].serial);
        set_timer(ticker + TICKS, poison_callback, cn, in, pos, ch[cn].serial, it[in].serial);
        break;
    case IDR_NONOMAGIC:
    case IDR_FIRE:
    case IDR_FLASHY:
    case IDR_INFRARED:
    case IDR_OXYGEN:
    case IDR_UWTALK:
        set_timer(*(unsigned int *)(it[in].drdata), remove_spell, cn, in, pos, ch[cn].serial, it[in].serial);
        break;
    case IDR_HEAL:
        set_timer(*(unsigned int *)(it[in].drdata), remove_spell, cn, in, pos, ch[cn].serial, it[in].serial);
        set_timer(ticker + HEALTICK, heal_tick, cn, in, pos, ch[cn].serial, it[in].serial);
        create_show_effect(EF_HEAL, cn, *(unsigned int *)(it[in].drdata) - HEALDURATION, *(unsigned int *)(it[in].drdata), 0, 0);
        break;
    }
    return 1;
}

int store_citem(int cn) {
    int n;

    for (n = 30; n < INVENTORYSIZE; n++) {
        if (ch[cn].item[n]) continue;
        return swap(cn, n);
    }
    return 0;
}

int store_item(int cn, int in) {
    int n;

    for (n = 30; n < INVENTORYSIZE; n++) {
        if (ch[cn].item[n]) continue;
        ch[cn].item[n] = in;
        it[in].carried = cn;
        return 1;
    }
    return 0;
}

void look_values_bg(int cnID, int coID) {
    int co, n;
    char buf[512];
    struct depot_ppd *ppd;

    for (co = getfirst_char(); co; co = getnext_char(co)) {
        if (ch[co].ID == coID) break;
    }
    if (!co) return;

    sprintf(buf, "%s, level %d, class %s%s%s",
            ch[co].name,
            ch[co].level,
            (ch[co].flags & CF_ARCH) ? "A" : "",
            (ch[co].flags & CF_WARRIOR) ? "W" : "",
            (ch[co].flags & CF_MAGE) ? "M" : "");
    tell_chat(0, cnID, 1, "%s", buf);

    sprintf(buf, "Desc: %s", ch[co].description);
    tell_chat(0, cnID, 1, "%s", buf);

    sprintf(buf, "Playing for %d hours.", stats_online_time(co) / 60);
    tell_chat(0, cnID, 1, "%s", buf);

    ppd = set_data(co, DRD_DEPOT_PPD, sizeof(struct depot_ppd));
    if (ppd && !ppd->loaded) ppd = NULL;
    if (ppd) sprintf(buf, "Gold in hand: %.2fG, gold in bank: %.2fG", ch[co].gold / 100.0, ppd->gold / 100.0);
    else sprintf(buf, "Gold in hand: %.2fG", ch[co].gold / 100.0);
    tell_chat(0, cnID, 1, "%s", buf);

    sprintf(buf, "Mirror: %d, actual mirror: %d. Area %d, %s", ch[co].mirror, areaM, areaID, get_section_name(ch[co].x, ch[co].y));
    tell_chat(0, cnID, 1, "%s", buf);

    for (n = 0; n <= V_IMMUNITY; n += 3) {
        sprintf(buf, "%s: %d/%d \010%s: %d/%d \020%s: %d/%d",
                skill[n].name, ch[co].value[1][n], ch[co].value[0][n],
                n + 1 <= V_IMMUNITY ? skill[n + 1].name : "none",
                n + 1 <= V_IMMUNITY ? ch[co].value[1][n + 1] : 0,
                n + 1 <= V_IMMUNITY ? ch[co].value[0][n + 1] : 0,
                n + 2 <= V_IMMUNITY ? skill[n + 2].name : "none",
                n + 2 <= V_IMMUNITY ? ch[co].value[1][n + 2] : 0,
                n + 2 <= V_IMMUNITY ? ch[co].value[0][n + 2] : 0);
        tell_chat(0, cnID, 1, "%s", buf);
    }
    sprintf(buf, "%s: %d/%d \010%s: %d/%d \020%s: %d/%d",
            skill[V_DURATION].name, ch[co].value[1][V_DURATION], ch[co].value[0][V_DURATION],
            skill[V_RAGE].name, ch[co].value[1][V_RAGE], ch[co].value[0][V_RAGE],
            skill[V_PROFESSION].name, ch[co].value[1][V_PROFESSION], ch[co].value[0][V_PROFESSION]);
    tell_chat(0, cnID, 1, "%s", buf);

    tell_chat(0, cnID, 1, "WV: %.2f, AV: %.2f", ch[co].value[0][V_WEAPON] / 20.0, ch[co].value[0][V_ARMOR] / 20.0);
}

void lollipop_bg(int cnID, int coID) {
    int co, in;

    for (co = getfirst_char(); co; co = getnext_char(co)) {
        if (ch[co].ID == coID) break;
    }
    if (!co) return;

    in = create_item("lollipop");
    if (give_char_item(co, in)) {
        log_char(co, LOG_SYSTEM, 0, "A lollipop dropped out of thin air into your hand.");
        tell_chat(0, cnID, 1, "Gave lollipop to %s.", ch[co].name);
    } else tell_chat(0, cnID, 1, "Could not give lollipop to %s.", ch[co].name);
}

void kick_char_by_sID(int sID) {
    int co;

    for (co = getfirst_char(); co; co = getnext_char(co)) {
        if (ch[co].sID == sID) break;
    }
    if (!co) return;

    exit_char(co);
    if (ch[co].player) player_client_exit(ch[co].player, "You have been kicked by game administration.");
}

static int shutdown_last = 0;

void shutdown_warn(void) {
    char buf[256];
    int min, n;

    if (!shutdown_at) return;

    min = (shutdown_at - realtime + 50) / 60;
    if (min != shutdown_last) {
        if (min > 0) sprintf(buf, "#03The server will go down in %d minute%s. Expected downtime: %d minutes.", min, min > 1 ? "s" : "", shutdown_down);
        else sprintf(buf, "#03The server will go down NOW. Expected downtime: %d minutes.", shutdown_down);
        for (n = 1; n < MAXCHARS; n++) {
            if (!(ch[n].flags & CF_PLAYER)) continue;
            log_char(n, LOG_SYSTEM, 0, "%s", buf);
        }
        shutdown_last = min;
        if (min < 3) nologin = 1;
    }
    if (realtime >= shutdown_at) {
        quit = 1;
    }
}

void shutdown_bg(int t, int down) {
    if (t) {
        shutdown_at = t;
        shutdown_down = down;
        shutdown_last = 999;
        shutdown_warn();
    } else if (shutdown_at || nologin) {
        int n;
        shutdown_at = nologin = 0;

        for (n = 1; n < MAXCHARS; n++) {
            if (!(ch[n].flags & CF_PLAYER)) continue;
            log_char(n, LOG_SYSTEM, 0, "\260c3Shutdown has been cancelled.");
        }
    }
}

int edemon_reduction(int cn, int str) {
    return max(0, str - ch[cn].value[0][V_DEMON]);
}

// can cn see co based on CF_INVIS etc.?
int is_visible(int cn, int co) {
    if (ch[co].flags & CF_INVISIBLE) return 0;

    return 1;
}

void destroy_char(int cn) {
    del_all_data(cn);
    purge_messages(cn);
    destroy_chareffects(cn);
    destroy_items(cn);
    free_char(cn);
}

void remove_destroy_char(int cn) {
    remove_char(cn);
    destroy_char(cn);
}

void give_military_pts(int cn, int co, int pts, int exps) {
    struct military_ppd *ppd;
    int rank;
    char buf[256];

    if (!(ppd = set_data(co, DRD_MILITARY_PPD, sizeof(struct military_ppd)))) return;

    give_exp(co, exps);
    ppd->normal_exp += exps;

    ppd->military_pts += pts;
    rank = cbrt(ppd->military_pts);
    while (rank < 25 && rank > get_army_rank_int(co)) {
        set_army_rank(co, rank);
        say(cn, "You've been promoted to %s. Congratulations, %s!", get_army_rank_string(co), ch[co].name);
        if (get_army_rank_int(co) > 9) {
            sprintf(buf, "0000000000\260c10Grats: %s is a %s now!", ch[co].name, get_army_rank_string(co));
            server_chat(6, buf);
        }
    }
    dlog(co, 0, "got %d military pts, %d exp", pts, exps);
}

void give_military_pts_no_npc(int co, int pts, int exps) {
    struct military_ppd *ppd;
    int rank;
    char buf[256];

    if (!(ppd = set_data(co, DRD_MILITARY_PPD, sizeof(struct military_ppd)))) return;

    give_exp(co, exps);
    ppd->normal_exp += exps;

    ppd->military_pts += pts;
    rank = cbrt(ppd->military_pts);
    if (rank < 25 && rank > get_army_rank_int(co)) {
        set_army_rank(co, rank);
        log_char(co, LOG_SYSTEM, 0, "You've been promoted to %s!", get_army_rank_string(co));
        if (get_army_rank_int(co) > 9) {
            sprintf(buf, "0000000000\260c10Grats: %s is a %s now!", ch[co].name, get_army_rank_string(co));
            server_chat(6, buf);
        }
    }
    dlog(co, 0, "got %d military pts, %d exp", pts, exps);
}

int create_drop_char(char *name, int x, int y) {
    int cn;

    cn = create_char(name, 0);
    if (!cn) return 0;

    if (drop_char(cn, x, y, 0)) {

        ch[cn].tmpx = ch[cn].x;
        ch[cn].tmpy = ch[cn].y;

        update_char(cn);

        ch[cn].hp = ch[cn].value[0][V_HP] * POWERSCALE;
        ch[cn].endurance = ch[cn].value[0][V_ENDURANCE] * POWERSCALE;
        ch[cn].mana = ch[cn].value[0][V_MANA] * POWERSCALE;

        ch[cn].dir = DX_RIGHTDOWN;

        return cn;
    }

    destroy_char(cn);

    return 0;
}

int check_dlight(int x, int y) {
    int m;
    if (x < 0 || y < 0 || x >= MAXMAP || y >= MAXMAP) return 0;

    m = x + y * MAXMAP;
    return (dlight * map[m].dlight) / 256;
}

int check_light(int x, int y) {
    int m;
    if (x < 0 || y < 0 || x >= MAXMAP || y >= MAXMAP) return 0;

    m = x + y * MAXMAP;
    return map[m].light;
}

char *lower_case(char *src) {
    static char buf[256], *dst;
    int n;

    for (n = 0, dst = buf; n < 255 && *src; n++) *dst++ = tolower(*src++);
    *dst = 0;

    return buf;
}

int give_char_item(int cn, int in) {
    int n;

    if (!ch[cn].citem) ch[cn].citem = in;
    else {
        for (n = 30; n < INVENTORYSIZE; n++) {
            if (!ch[cn].item[n]) {
                ch[cn].item[n] = in;
                break;
            }
        }
        if (n == INVENTORYSIZE) return 0;
    }
    it[in].carried = cn;
    ch[cn].flags |= CF_ITEMS;
    return 1;
}

int create_orb(void) {
    btrace("no more orbs!");

    return 0;
}

int create_orb2(int what) {
    btrace("no more orbs!");

    return 0;
}

int take_money(int cn, int val) {
    if (ch[cn].gold < val) return 0;
    ch[cn].gold -= val;
    ch[cn].flags |= CF_ITEMS;
    return 1;
}

void tabunga(int cn, int co, char *ptr) {
    if ((ch[co].flags & CF_GOD) && strcasestr(ptr, "tabunga") && char_dist(cn, co) < 3) {
        say(cn, "%s (%d):", ch[cn].name, ch[cn].level);
        say(cn, "HP:        %3d/%3d (%d)", ch[cn].value[1][V_HP], ch[cn].value[0][V_HP], ch[cn].hp / POWERSCALE);
        say(cn, "Endurance: %3d/%3d (%d)", ch[cn].value[1][V_ENDURANCE], ch[cn].value[0][V_ENDURANCE], ch[cn].endurance / POWERSCALE);
        say(cn, "Mana:      %3d/%3d (%d)", ch[cn].value[1][V_MANA], ch[cn].value[0][V_MANA], ch[cn].mana / POWERSCALE);
        say(cn, "Wisdom:    %3d/%3d", ch[cn].value[1][V_WIS], ch[cn].value[0][V_WIS]);
        say(cn, "Intuition: %3d/%3d", ch[cn].value[1][V_INT], ch[cn].value[0][V_INT]);
        say(cn, "Agility:   %3d/%3d", ch[cn].value[1][V_AGI], ch[cn].value[0][V_AGI]);
        say(cn, "Strength:  %3d/%3d", ch[cn].value[1][V_STR], ch[cn].value[0][V_STR]);
        say(cn, "Hand2Hand: %3d/%3d", ch[cn].value[1][V_HAND], ch[cn].value[0][V_HAND]);
        say(cn, "Sword:     %3d/%3d", ch[cn].value[1][V_SWORD], ch[cn].value[0][V_SWORD]);
        say(cn, "Twohanded: %3d/%3d", ch[cn].value[1][V_TWOHAND], ch[cn].value[0][V_TWOHAND]);
        say(cn, "Attack:    %3d/%3d", ch[cn].value[1][V_ATTACK], ch[cn].value[0][V_ATTACK]);
        say(cn, "Parry:     %3d/%3d", ch[cn].value[1][V_PARRY], ch[cn].value[0][V_PARRY]);
        say(cn, "Tactics:   %3d/%3d", ch[cn].value[1][V_TACTICS], ch[cn].value[0][V_TACTICS]);
        say(cn, "Immunity:  %3d/%3d", ch[cn].value[1][V_IMMUNITY], ch[cn].value[0][V_IMMUNITY]);
        say(cn, "Bless:     %3d/%3d", ch[cn].value[1][V_BLESS], ch[cn].value[0][V_BLESS]);
        say(cn, "M-Shield:  %3d/%3d  (%d)", ch[cn].value[1][V_MAGICSHIELD], ch[cn].value[0][V_MAGICSHIELD], ch[cn].lifeshield / POWERSCALE);
        say(cn, "Flash:     %3d/%3d", ch[cn].value[1][V_FLASH], ch[cn].value[0][V_FLASH]);
        say(cn, "Freeze:    %3d/%3d", ch[cn].value[1][V_FREEZE], ch[cn].value[0][V_FREEZE]);
        say(cn, "F-Ball:    %3d/%3d", ch[cn].value[1][V_FIRE], ch[cn].value[0][V_FIRE]);
        say(cn, "Percept:   %3d/%3d", ch[cn].value[1][V_PERCEPT], ch[cn].value[0][V_PERCEPT]);
        say(cn, "Stealth:   %3d/%3d", ch[cn].value[1][V_STEALTH], ch[cn].value[0][V_STEALTH]);
        say(cn, "Warcry:    %3d/%3d", ch[cn].value[1][V_WARCRY], ch[cn].value[0][V_WARCRY]);
        say(cn, "Offensive Value: %d, WV: %.2f", ch[cn].value[0][V_OFFENSE], ch[cn].value[0][V_WEAPON] / 20.0);
        say(cn, "Defensive Value: %d, AV: %.2f", ch[cn].value[0][V_DEFENSE], ch[cn].value[0][V_ARMOR] / 20.0);
        say(cn, "Speed          : %d", ch[cn].value[0][V_SPEED]);
    }
}

int get_lifeshield_max(int cn) {
    if (ch[cn].value[0][V_MAGICSHIELD]) return ch[cn].value[0][V_MAGICSHIELD] / MAGICSHIELDMOD;
    return 0;
}

void bondtake_item(int in, int cn) {
    it[in].flags |= IF_BONDTAKE;
    it[in].ownerID = ch[cn].ID;
}

void bondwear_item(int in, int cn) {
    it[in].flags |= IF_BONDWEAR;
    it[in].ownerID = ch[cn].ID;
}

int cnt_free_inv(int cn) {
    int n, cnt;

    for (n = 30, cnt = 0; n < INVENTORYSIZE; n++) {
        if (!ch[cn].item[n]) cnt++;
    }
    return cnt;
}

void do_whostaff(int cnID) {
    int co;

    for (co = getfirst_char(); co; co = getnext_char(co)) {
        if (ch[co].flags & CF_INVISIBLE) continue;
        if (!(ch[co].flags & CF_PLAYER)) continue;
        if (!(ch[co].flags & (CF_STAFF | CF_GOD))) continue;

        tell_chat(0, cnID, 1, "%s [%s]%s",
                  ch[co].name,
                  ch[co].staff_code,
                  ch[co].driver == 0 ? "" : " (lagging)");
    }
}

void do_whotutor(int cnID) {
    int co;

    for (co = getfirst_char(); co; co = getnext_char(co)) {
        if (ch[co].flags & CF_INVISIBLE) continue;
        if (!(ch[co].flags & CF_PLAYER)) continue;
        if (!(ch[co].flags & (CF_TUTOR | CF_STAFF | CF_GOD))) continue;

        tell_chat(0, cnID, 1, "%s%s%s",
                  !(ch[co].flags & CF_TUTOR) ? "\260c11" : "",
                  ch[co].name,
                  ch[co].driver == 0 ? "" : " (lagging)");
    }
}

void update_item(int in) {
    int cn;

    if (in < 1 || in >= MAXITEM) {
        elog("update item called with illegal in=%d", in);
        return;
    }
    if (!it[in].flags) {
        elog("update item called with freed in=%d (%s)", in, it[in].name);
        return;
    }

    if ((cn = it[in].carried)) {
        if (cn < 1 || cn >= MAXCHARS) {
            elog("update item wants to use illegal cn=%d", cn);
            return;
        }
        if (!ch[cn].flags) {
            elog("update item wants to use freed cn=%d (%s)", cn, ch[cn].name);
            return;
        }
        ch[it[in].carried].flags |= CF_ITEMS;
        return;
    }
    if (it[in].contained) return;

    if (it[in].x < 1 || it[in].x >= MAXMAP || it[in].y < 1 || it[in].y >= MAXMAP) {
        elog("update item wants to use illegal map pos %d,%d", it[in].x, it[in].y);
        return;
    }
    set_sector(it[in].x, it[in].y);
}

void player_use_potion(int cn) {
    int n, in;
    struct lostcon_ppd *ppd;

    if (has_spell(cn, IDR_HEAL)) return;

    ppd = set_data(cn, DRD_LOSTCON_PPD, sizeof(struct lostcon_ppd));
    if (!ppd) return; // oops...

    if (ch[cn].hp < ch[cn].value[1][V_HP] * (int)(POWERSCALE * 0.60)) {
        if (!ppd->nolife || !ppd->nocombo) {
            for (n = 30; n < INVENTORYSIZE; n++) {
                if ((in = ch[cn].item[n]) && it[in].driver == IDR_POTION && it[in].drdata[1]) {
                    if (it[in].drdata[2] && !ppd->nocombo) {
                        use_item(cn, in);
                        break;
                    }
                    if (!it[in].drdata[2] && !ppd->nolife) {
                        use_item(cn, in);
                        break;
                    }
                }
            }
        }
    }
}

void player_use_recall(int cn) {
    int n, in;
    struct lostcon_ppd *ppd;

    // dont use recall *after* you died
    if (ch[cn].hp < (POWERSCALE / 2)) return;

    ppd = set_data(cn, DRD_LOSTCON_PPD, sizeof(struct lostcon_ppd));
    if (!ppd) return; // oops...

    if (ch[cn].hp < ch[cn].value[1][V_HP] * (int)(POWERSCALE * 0.40) && !ppd->norecall) {
        for (n = 30; n < INVENTORYSIZE; n++) {
            if ((in = ch[cn].item[n]) && it[in].driver == IDR_RECALL && it[in].drdata[0] >= ch[cn].level) {
                use_item(cn, in);
                return;
            }
        }
    }
}

int check_can_wear_item(int cn) {
    int in, n;

    for (n = 0; n < 12; n++) {
        if ((in = ch[cn].item[n]) && !can_wear(cn, in, n) && give_char_item(cn, in)) {
            dlog(cn, in, "non-wearable item removed");
            ch[cn].item[n] = 0;
        }
    }
    return 0;
}

int count_enhancements(int in) {
    int m, cnt = 0;

    // check requirements
    for (m = 0; m < MAXMOD; m++) {
        if (!(it[in].mod_value[m])) continue; // ignore empty ones
        if (it[in].mod_index[m] < 0) continue; // ignore requirements

        switch (it[in].mod_index[m]) {
        case V_WEAPON:
            break;
        case V_ARMOR:
            break;
        case V_SPEED:
            break;
        case V_DEMON:
            break;
        case V_COLD:
            break;
        case V_LIGHT:
            break;
        default:
            cnt++;
            break;
        }
    }
    return cnt;
}

int _get_complexity_cost(struct item *in) {
    int m, v, cost, cnt = 0;

    for (m = cost = 0; m < MAXMOD; m++) {
        switch (in->mod_index[m]) {
        case V_WEAPON:
            break;
        case V_ARMOR:
            break;
        case V_SPEED:
            break;
        case V_DEMON:
            break;
        case V_COLD:
            break;
        case V_LIGHT:
            break;
        default:
            if (in->mod_index[m] >= 0 && (v = in->mod_value[m]) > 0) {
                cost += (v * (v + 1)) * 125;
                cnt++;
            }
            break;
        }
    }
    if (cnt >= 2) cost *= 2;
    if (cnt >= 3) cost *= 2;

    return cost;
}

int get_complexity_cost(int in) {
    return _get_complexity_cost(it + in);
}

int calculate_complexity(int in) {
    int cost;

    cost = get_complexity_cost(in);
    if (cost > 0) {
        it[in].quality = (int)(cost * 1.1001);
        it[in].complexity = cost;
        return 1;
    }
    return 0;
}

void update_fix_item(int in, int cn) {
    if (it[in].ID != IID_CLANJEWEL) return;
    if (it[in].driver == IDR_CLANJEWEL) return;

    it[in].driver = IDR_CLANJEWEL;
    *(unsigned int *)(it[in].drdata + 0) = realtime - 60 * 59;
}
