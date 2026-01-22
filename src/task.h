/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

int process_task(unsigned char *taskdata);
int task_set_clan_rank(int target_ID, int master_ID, int clan, int rank, char *master_name);
int task_fire_from_clan(int target_ID, int master_ID, int clan, char *master_name);
int task_punish_player(int target_ID, int master_ID, char *reason);
int task_unpunish_player(int target_ID, int master_ID, int ID);
int task_set_flags(int target_ID, int master_ID, unsigned long long flags);
