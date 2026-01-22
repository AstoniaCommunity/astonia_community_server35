/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

int init_chat(void);
void tick_chat(void);
int cmd_chat(int cn, char *text);
void list_chat(int cn);
void leave_chat(int cn, int nr);
void join_chat(int cn, int nr);
int tell_chat(int cnID, int coID, int staffmode, char *format, ...) __attribute__((format(printf, 4, 5)));
int server_chat(int channel, char *text);
void npc_chat(int cn, int channel, char *format, ...) __attribute__((format(printf, 3, 4)));
