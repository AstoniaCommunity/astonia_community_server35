/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

#define ATTACK_DIV 10 // turns an average of 50 weapon value into an average of 5 damage (before armor)

#define FIREBALL_DAMAGE (85 / ATTACK_DIV) // makes it the same as weapon damage
#define STRIKE_DAMAGE (56 / ATTACK_DIV) // ...

#define RAGEMOD 4 // rage skill is divided by this before being used

int skilldiff2percent(int diff);
int tactics2skill(int val);
