#define TREASURY_HISTORY_EXPIRE (60 * 60 * 24 * 30)

typedef struct treasury_history_data TREASURY_HISTORY_DATA;

struct treasury_history_data {
    TREASURY_HISTORY_DATA *next;
    TREASURY_HISTORY_DATA *prev;

    time_t timestamp;
#define TREASURY_ACTION_DEPOSIT_QP    1
#define TREASURY_ACTION_DEPOSIT_GOLD  2
#define TREASURY_ACTION_WITHDRAW_QP   3
#define TREASURY_ACTION_WITHDRAW_GOLD 4
#define TREASURY_ACTION_WITHDRAW_WAR  5
    int action;
    int amount;
    char *who;
};

struct treasury_data {
    int qp;
    int gold;
    TREASURY_HISTORY_DATA *first_history;
    TREASURY_HISTORY_DATA *last_history;
};

extern struct treasury_data treasury[MAX_CLAN];

void load_treasuries(void);
void save_treasuries(void);

DECLARE_DO_FUN(do_treasury);
