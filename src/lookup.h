/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

int lookup_name(char *name, char *realname);
int lookup_ID(char *name, int ID);
void lookup_add_cache(unsigned int ID, char *name);
int init_lookup(void);
