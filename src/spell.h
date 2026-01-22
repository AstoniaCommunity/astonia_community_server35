/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

// spell costs moved here so that NPC drivers can use them

#define BLESSCOST (5 * POWERSCALE)
#define HEALCOST heal cost is variable(half of the hp replaced)
#define FREEZECOST (6 * POWERSCALE)
#define MAGICSHIELDCOST magic shield cost is variable(half of the shield value)
#define FLASHCOST (6 * POWERSCALE)
#define FIREBALLCOST (6 * POWERSCALE)

#define WARCRYCOST (12 * POWERSCALE)

#define WARCRYDURATION (TICKS * 10)
#define BLESSDURATION (TICKS * 60 * 5)
#define FLASHDURATION (TICKS * 5)
#define FREEZEDURATION (TICKS * 6)
#define HEALDURATION (TICKS * 8)
#define HEALTICK (TICKS / 4)

#define MAGICSHIELDMOD 2
