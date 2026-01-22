/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

struct prof {
    char *name;

    char base; // start value
    char max; // maximum value
    char step; // raised in steps
};
extern struct prof prof[P_MAX];
int free_prof_points(int co);
int show_prof_info(int cn, int co, char *buf);
void cmd_steal(int cn);
int get_support_prof(int cn, int p);
