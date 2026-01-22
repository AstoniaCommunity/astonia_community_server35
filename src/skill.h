/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

struct skill {
    char name[80];
    int base1, base2, base3;
    int cost; // 0=not raisable, 1=skill, 2=attribute, 3=power
    int start; // start value, pts up to this value are free
};

extern struct skill skill[];

int raise_cost(int v, int n);
int raise_value(int cn, int v);
int calc_exp(int cn);
int lower_value(int cn, int v);
int raise_value_exp(int cn, int v);
