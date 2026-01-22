/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

#define MAXSHRINE 256

#define DEATH_SHRINE (51)
#define DEATH_SHRINE_INDEX (DEATH_SHRINE / 32)
#define DEATH_SHRINE_BIT (1u << (DEATH_SHRINE & 31))

struct shrine_ppd {
    unsigned int used[MAXSHRINE / 32];
    unsigned char continuity;
};
