/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

int get_section(int x, int y);
void show_section(int x, int y, int cn);
void register_npc_to_section(int cn);
int register_kill_in_section(int cn, int co);
void walk_section_msg(int cn);
void area_sound(int cn);
char *get_section_name(int x, int y);
