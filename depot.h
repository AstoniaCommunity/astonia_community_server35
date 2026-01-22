/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

#define MAXDEPOT 120

struct depot_ppd {
    unsigned int changes; // serial number, gets increased with each change to depot contents
    unsigned int lastsave; // serial number of last save
    unsigned int loaded; // depot was loaded correctly and can be used
    int gold; // gold in depot (bank account)
    int payment_tokens; // number of payment tokens the player owns (for use with trader or own account)
    int dummy[11]; // free for future extensions
    struct item itm[MAXDEPOT]; // items in depot
};
void player_depot(int cn, int nr, int flag, int fast, int is_account_depot);
void depot_sort(int cn);
