/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

#define MAXSTORE 64
#define STORESIZE 40

struct ware {
    int cnt;
    int always;
    struct item item;
};

struct store {
    struct ware ware[STORESIZE];
};

extern struct store **store;

int init_store(void);
int salesprice(int cn, int co, int nr);
int buyprice(int cn, int in);
int sell(int cn, int co, int nr);
int buy(int cn, int co);
int create_store(int cn, int ignore);
void player_store(int cn, int nr, int flag, int fast);
int add_special_store(int cn);
