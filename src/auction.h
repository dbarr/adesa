#define AUCTION_REVISION 1
#define AUCTION_FILE "../data/auction.dat"
#define MAX_AUCTION_DURATION (60*60*24*7)

#define BIDTYPE(auc) ((IS_SET(auc->flags, AUCTION_TYPE_GOLD) ? "@@yGP" : "@@aQP"))
#define BIDTYPE2(type) ((IS_SET(type, AUCTION_TYPE_GOLD) ? "@@yGP" : "@@aQP"))
#define AUCTYPE(type) ((type == AUCTION_TYPE_GOLD ? "gold" : "quest point"))
#define AUCTYPE2(auc) ((IS_SET(auc->flags, AUCTION_TYPE_GOLD) ? "gold" : "quest point"))
#define AUCCMD(type) ((type == AUCTION_TYPE_GOLD ? "gauction" : "qauction"))

#if defined(KEY)
#undef KEY
#endif
#define KEY(literal, field, value) if (!str_cmp(word, literal)) { field = value; fMatch = TRUE; break; }
#define SKEY(literal, field, value) if (!str_cmp(word, literal)) { if (field != NULL) free_string(field); field = value; fMatch = TRUE; break; }
#define TEMP_VNUM 3090

typedef struct auction_data AUCTION_DATA;

extern AUCTION_DATA *first_auction;
extern AUCTION_DATA *last_auction;
extern int top_obj_index;

extern void auction_do (CHAR_DATA *ch, char *argument, int type);
extern void auction_do_list (CHAR_DATA *ch, int type);
extern void auction_del (AUCTION_DATA *auc);
extern void auction_end (AUCTION_DATA *auc);
extern void auction_give (AUCTION_DATA *auc, char *to, bool spend);
extern bool valid_auction_obj(CHAR_DATA *ch, OBJ_DATA *obj, int type, int reserve, int duration);
extern OBJ_DATA *fread_auction_obj (FILE *fp);
extern void fwrite_auction_obj (OBJ_DATA *obj, FILE *fp);
extern void load_auctions(void);
extern void save_auctions(void);
extern void auc_update(void);
extern int available_qps(CHAR_DATA *ch);
extern int available_gold(CHAR_DATA *ch);
extern char *get_unique_keyword(void);

extern bool can_save args((CHAR_DATA *ch, OBJ_DATA *obj));

struct auction_data {
    AUCTION_DATA *next;
    AUCTION_DATA *prev;

    OBJ_DATA     *pObj;
    int          reserve;
    time_t       expire_time;

#define AUCTION_TYPE_GOLD   1
#define AUCTION_TYPE_QPS    2
    int          flags;
    char         *keyword;

    char         *owner;
    char         *owner_name;
    char         *bidder;
    char         *bidder_name;
    int          amount;
};
