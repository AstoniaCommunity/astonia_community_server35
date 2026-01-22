/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

#define AF1_STORAGE_HINT (1u << 1)
#define AF1_BUY_HINT (1u << 2)

struct area1_ppd {
    int yoakin_state;
    int yoakin_seen_timer;

    int gwendy_state;
    int gwendy_seen_timer;

    int terion_state;

    int james_state;
    int flags;

    int darkin_state;

    int gerewin_state;
    int gerewin_seen_timer;

    int nook_state;

    int lydia_state;
    int lydia_seen_timer;

    int asturin_state;
    int asturin_seen_timer;

    int guiwynn_state;
    int guiwynn_seen_timer;

    int logain_state;
    int logain_seen_timer;

    int reskin_state;
    int reskin_seen_timer;
    int reskin_got_bits;

    int shrike_state;
    int shrike_fails;
};
