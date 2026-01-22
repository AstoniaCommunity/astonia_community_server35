/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

int ignoring(int cn, int ID);
void ignore(int cn, int ID);
void list_ignore(int cn);
int ignore_cmd(int cn, char *name);
void clearignore_cmd(int cn);
