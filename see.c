/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

#include <stdlib.h>
#include "server.h"
#include "los.h"
#include "log.h"
#include "date.h"
#include "btrace.h"
#include "balance.h"
#include "see.h"

static inline int check_lightm(int m) {

    return min(255, max(map[m].light, (dlight * map[m].dlight) / 256));
}

int char_see_char_nolos(int cn, int co) {
    int m, dist, light, stealth, vcn, vco;

    if (cn < 1 || cn >= MAXCHARS) {
        elog("char_see_char_nolos: illegal cn %d", cn);
        return 0;
    }
    if (co == 0) return 0;
    if (co < 1 || co >= MAXCHARS) {
        elog("char_see_char_nolos: illegal co %d", co);
        return 0;
    }

    if (!(ch[co].flags)) return 0;

    if (cn == co) return 1;

    if (ch[co].flags & CF_INVISIBLE) return 0;

    if ((ch[cn].flags & (CF_GOD | CF_INFRARED)) == (CF_GOD | CF_INFRARED)) return 1;

    dist = max(abs(ch[cn].x - ch[co].x), abs(ch[cn].y - ch[co].y)) + 1;

    m = ch[co].x + ch[co].y * MAXMAP;

    light = check_lightm(m);
    if (ch[cn].flags & (CF_INFRARED | CF_INFRAVISION)) light = max(32, light);

    if (!light && (abs(ch[cn].x - ch[co].x) > 1 || abs(ch[cn].y - ch[co].y) > 1)) return 0;

    if (dist < 3) return 1;
    dist = dist * dist;

    light = max(32 - light, 0) * 2;

    if (ch[co].speed_mode == SM_STEALTH) stealth = ch[co].value[0][V_STEALTH];
    else stealth = 0;

    // 0...64        4..400
    if (stealth) vco = light + stealth + dist;
    else vco = 0;

    vcn = ch[cn].value[0][V_PERCEPT] + 16 + 49;

    if (vcn >= vco) return 1;

    return 0;
}

int char_see_char(int cn, int co) {
    if (cn < 1 || cn >= MAXCHARS) {
        elog("char_see_char: illegal cn %d", cn);
        btrace("char_see_char");
        return 0;
    }
    if (co == 0) return 0;
    if (co < 1 || co >= MAXCHARS) {
        elog("char_see_char: illegal co %d, cn=%s (%d)", co, ch[cn].name, cn);
        btrace("char_see_char");
        return 0;
    }

    if (!(ch[co].flags)) return 0;

    if (cn == co) return 1;

    if (ch[co].flags & CF_INVISIBLE) return 0;

    if (!los_can_see(cn, ch[cn].x, ch[cn].y, ch[co].x, ch[co].y, DIST)) return 0;

    return char_see_char_nolos(cn, co);
}

int char_see_item(int cn, int in) {
    int m, light;

    if (!(it[in].flags)) return 0;

    if (it[in].carried) return 0;

    if (!los_can_see(cn, ch[cn].x, ch[cn].y, it[in].x, it[in].y, DIST)) return 0;

    if ((it[in].flags & IF_FRONTWALL) &&
        ((map[it[in].x + 1 + it[in].y * MAXMAP].flags & (MF_SIGHTBLOCK | MF_TSIGHTBLOCK)) || !los_can_see(cn, ch[cn].x, ch[cn].y, it[in].x + 1, it[in].y, DIST)) &&
        ((map[it[in].x + it[in].y * MAXMAP + MAXMAP].flags & (MF_SIGHTBLOCK | MF_TSIGHTBLOCK)) || !los_can_see(cn, ch[cn].x, ch[cn].y, it[in].x, it[in].y + 1, DIST)))
        return 0;

    if (!(it[in].flags & IF_TAKE)) return 1;

    m = it[in].x + it[in].y * MAXMAP;

    light = check_lightm(m);
    if (ch[cn].flags & (CF_INFRARED | CF_INFRAVISION)) light = max(32, light);
    if (light < 1) return 0;

    return 1;
}
