/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

#define MAXCONTAINER 1024
#define CONTAINERSIZE (INVENTORYSIZE)

struct container {
    int cn, in; // character or item which is this container (reference object)

    int owner; // for graves: the victim may access the grave
    int owner_not_seyan; // for graves: new seyans may not access their old grave
    int killer; // for graves: the killer may access the grave

    int item[CONTAINERSIZE]; // up to CONTAINERSIZE item numbers (contents of container)

    struct container *next;
};

extern struct container *con;

extern int used_containers;

int alloc_container(int cn, int in);
void free_container(int ct);
int init_container(void);
int create_item_container(int in);
int destroy_item_container(int in);
int add_item_container(int ct, int in, int pos);
int remove_item_container(int in);
int container_itemcnt(int in);
