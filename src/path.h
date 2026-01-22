/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

int init_path(void);
int pathfinder(int fx, int fy, int tx, int ty, int mindist, int (*check_target)(int), int maxstephint);
int pathcost(void);

int pathbestdir(void);
int pathbestx(void);
int pathbesty(void);
int pathbestcost(void);
int pathnodes(void);
int pathbestdist(void);

extern int path_rect_fx;
extern int path_rect_tx;
extern int path_rect_fy;
extern int path_rect_ty;

int rect_check_target(int m);
