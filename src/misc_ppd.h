/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

struct misc_ppd {
    int lfreduct_usage_count;

    int complaint_date;

    int last_lq_death;

    int supermax_state;
    int supermax_gold;

    int swapped;

    unsigned char treedone[8];
};
