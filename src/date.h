/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

extern int dlight;

extern int newmoon;
extern int fullmoon;
extern int moonsize;

extern int solstice;
extern int equinox;
extern int summer_solstice;
extern int winter_solstice;
extern int spring_equinox;
extern int fall_equinox;

extern int sunrise;
extern int sunset;
extern int moonrise;
extern int moonset;
extern int moonlight;
extern int sunlight;

extern int minute; // 0...59
extern int hour; // 0...23
extern int mday; // 0...29
extern int wday; // 0...6
extern int yday; // 0...359
extern int week; // warning: weeks since beginning of time !
extern int month; // 0...12
extern int year; // 0...

extern int moon; // warning: moons since beginning of time !
extern int moonday; // 0...27
extern int realtime; // seconds realtime since beginning of our epoch

void tick_date(void);
void showtime(int cn);
