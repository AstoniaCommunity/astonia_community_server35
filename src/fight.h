/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

struct person {
    unsigned int cn;
    unsigned int ID;

    unsigned short lastx, lasty;
    unsigned char visible;
    unsigned char hurtme;
};

struct fight_driver_data {
    struct person enemy[10];

    int start_dist; // distance from respawn point at which to start attacking
    int stop_dist; // distance from respawn point at which to stop attacking
    int char_dist; // distance from character we start attacking

    int home_x, home_y; // position to compare start_dist and start_dist with

    int lasthit;
};
