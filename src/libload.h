/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

int init_lib(void);
void exit_lib(void);

#define CDT_DRIVER 0
#define CDT_ITEM 1
#define CDT_DEAD 2
#define CDT_RESPAWN 3
#define CDT_SPECIAL 4

int char_driver(int nr, int type, int cn, int ret, int last_action);
int item_driver(int nr, int in, int cn);
