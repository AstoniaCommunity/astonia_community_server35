/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

void poison_someone(int cn, int pwr, int type);
void poison_callback(int cn, int in, int pos, int cserial, int iserial);
int remove_all_poison(int cn);
int remove_poison(int cn, int type);
