/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

int read_motd(void);
void show_motd(int nr);
#ifdef STAFF
int check_staff_start(void);
void check_staff_stop(void);
#endif
