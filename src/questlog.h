/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

#define MAXQUEST 100
#define QF_OPEN 1
#define QF_DONE 2

struct quest {
    unsigned char done : 6;
    unsigned char flags : 2;
};

void questlog_open(int cn, int qnr);
int questlog_done(int cn, int qnr);
void questlog_reopen(int cn, int qnr);
int questlog_isdone(int cn, int qnr);
void destroy_item_byID(int cn, int ID);
int questlog_count(int cn, int qnr);
int questlog_scale(int cnt, int ex);
void questlog_close(int cn, int qnr);
