/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

int init_sector(void);
void set_sector(int x, int y);
int skipx_sector(int x, int y);
void add_char_sector(int cn);
void del_char_sector(int cn);
int getfirst_char_sector(int x, int y);
void init_sound_sector(void);
int sector_hear(int xf, int yf, int xt, int yt);
int sector_hear_shout(int xf, int yf, int xt, int yt);
