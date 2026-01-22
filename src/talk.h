/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

#define LOG_SYSTEM 0
#define LOG_TALK 1 // dat1=co, everyone in area gets it, blocked by MF_SOUNDBLOCK|MF_SHOUTBLOCK
#define LOG_SHOUT 2 // dat1=co, everyone in area gets it, blocked by MF_SHOUTBLOCK
#define LOG_INFO 3 // dat1=co, visibility check determines whether the char gets the message or not

int log_char(int cn, int type, int dat1, char *format, ...) __attribute__((format(printf, 4, 5)));
int log_area(int xc, int yc, int type, int dat1, int maxdist, char *format, ...) __attribute__((format(printf, 6, 7)));
int holler(int cn, char *format, ...) __attribute__((format(printf, 2, 3)));
int shout(int cn, char *format, ...) __attribute__((format(printf, 2, 3)));
int say(int cn, char *format, ...) __attribute__((format(printf, 2, 3)));
int emote(int cn, char *format, ...) __attribute__((format(printf, 2, 3)));
int quiet_say(int cn, char *format, ...) __attribute__((format(printf, 2, 3)));
int whisper(int cn, char *format, ...) __attribute__((format(printf, 2, 3)));
int murmur(int cn, char *format, ...) __attribute__((format(printf, 2, 3)));
int sound_area(int xc, int yc, int type);
