/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

void exit_player(int nr);
int init_io(void);
void io_loop(void);
void psend(int nr, char *buf, int len);
void pflush(void);
void exit_io(void);
