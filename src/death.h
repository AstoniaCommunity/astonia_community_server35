/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

int kill_char(int cn, int co);
int hurt(int cn, int dam, int cc, int armordiv, int armorper, int shieldper);
int die_char(int cn, int co, int ispk);
int allow_body(int cn, char *name);
int kill_score(int cn, int cc);
int death_loss(int total_exp);
int delayed_hurt(int delay, int cn, int dam, int armorper, int shieldper);
int kill_score_level(int cnlev, int cclev);
int check_first_kill(int cn, int nr);
