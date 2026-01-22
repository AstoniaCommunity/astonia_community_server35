/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

void ilog(char *format, ...) __attribute__((format(printf, 1, 2)));
void elog(char *format, ...) __attribute__((format(printf, 1, 2)));
void xlog(char *format, ...) __attribute__((format(printf, 1, 2)));
void charlog(int cn, char *format, ...) __attribute__((format(printf, 2, 3)));
int init_log(void);
void exit_log(void);
void elog_item(int in);
void reinit_log(void);
void log_items(int cn);
