#define DUEL_START_ROOM 10597
#define DUEL_MIN_ROOM 10598
#define DUEL_MAX_ROOM 10600

#define DUEL_TIMEOUT_READY     60 /* everyone has 60 seconds to accept */
#define DUEL_TIMEOUT_SET       20 /* 20 seconds to prepare before combat */
#define DUEL_TIMEOUT_GO        1200 /* 20 minute maximum duels */
#define DUEL_TIMEOUT_LINKDEAD  20 /* person has 20 seconds to reconnect while
                                     linkdead, otherwise they forfeit */
#define DUEL_TIMEOUT_IDLE       6 /* minutes person can be idle for */

#define DUEL_END_WIN      0
#define DUEL_END_TIMEOUT  1
#define DUEL_END_SUICIDE  2
#define DUEL_END_LINKDEAD 3
#define DUEL_END_MOVEROOM 4
#define DUEL_END_IDLE     5
#define DUEL_END_OBJ      6

#define DUEL_RAWKILL_NORMAL 0
#define DUEL_RAWKILL_OBJ    1

/* double-linked list macros */
#define DLINK(link, first, last, next, prev) \
do { \
  if ( (link)->prev || (link)->next ) hang("DLINK: link already in list?"); \
  if ( (last) && (last)->next ) hang("DLINK: last->next NON-NULL!"); \
  if ( !(first) ) \
    (first) = (link); \
  else \
    (last)->next = (link); \
  (link)->next = NULL; \
  (link)->prev = (last); \
  (last) = (link); \
} while(0)

#define DUNLINK(link, first, last, next, prev) \
do { \
  if ( (link)->prev && (((link)->prev)->next != (link)) ) \
    hang("DUNLINK: link->prev->next corrupted!"); \
  if ( (link)->next && (((link)->next)->prev != (link)) ) \
    hang("DUNLINK: link->next->prev corrupted!"); \
  if ( !(link)->next ) \
    (last) = (link)->prev; \
  else \
    (link)->next->prev = (link)->prev; \
  if ( !(link)->prev ) \
    (first) = (link)->next; \
  else \
    (link)->prev->next = (link)->next; \
  (link)->prev = NULL; \
  (link)->next = NULL; \
} while(0)
/* END: double linked list macros */

#define CREATE_MEMBER(type, parm)                           \
do {                                                        \
    parm = (type *)malloc(sizeof(type));                    \
                                                            \
    if (!parm)                                              \
        bug("CREATE_MEMBER: unable to allocate memory");    \
    else                                                    \
        memset(parm, 0, sizeof(type));                      \
} while(0)

#define DESTROY_MEMBER(parm)    \
do {                            \
    if (parm)                   \
        free(parm);             \
} while (0)

#define DUEL_ALLOWED_CHARMIE(ch)                                               \
do {                                                                           \
    DUEL_DATA *duel;                                                           \
                                                                               \
    if (!IS_NPC(ch) && (duel = find_duel(ch)))                                 \
        if (duel->stage == DUEL_STAGE_GO && !IS_SET(duel->flags, DUEL_CHARM))  \
            return FALSE;                                                      \
                                                                               \
} while (0)

/* duel structures */
typedef struct duel_data DUEL_DATA;
typedef struct duel_player_data DUEL_PLAYER_DATA;
typedef struct duel_watcher_data DUEL_WATCHER_DATA;
typedef struct duel_obj_data DUEL_OBJ_DATA;

DUEL_DATA *first_duel;
DUEL_DATA *last_duel;

struct duel_obj_data {
    DUEL_OBJ_DATA *next;
    DUEL_OBJ_DATA *prev;

    OBJ_DATA *obj;
};

struct duel_player_data {
    DUEL_PLAYER_DATA *next;
    DUEL_PLAYER_DATA *prev;

    DUEL_OBJ_DATA    *first_obj;
    DUEL_OBJ_DATA    *last_obj;

    AFFECT_DATA *first_affect;
    AFFECT_DATA *last_affect;

    CHAR_DATA   *ch;
    bool        accepted;
    time_t      linkdead;
    int         hp;
    int         mana;
    int         move;
    int         energy;
    int         str;
    int         intel;
    int         dex;
    int         race;
};

struct duel_watcher_data {
    DUEL_WATCHER_DATA *next;
    DUEL_WATCHER_DATA *prev;

    CHAR_DATA *ch;
};

struct duel_data {
    DUEL_DATA   *next;
    DUEL_DATA   *prev;

    DUEL_PLAYER_DATA   *first_player;
    DUEL_PLAYER_DATA   *last_player;

    DUEL_WATCHER_DATA   *first_watcher;
    DUEL_WATCHER_DATA   *last_watcher;

#define DUEL_CHARM      BIT_1
#define DUEL_NOMAGIC    BIT_2
#define DUEL_NOOBJ      BIT_3
#define DUEL_DBLDAM     BIT_4
#define DUEL_DBLHEAL    BIT_5
#define DUEL_SUPERCAST  BIT_6
#define DUEL_SPAR       BIT_7
#define DUEL_RANDSTAT   BIT_8
#define DUEL_RANDRACE   BIT_9
    int         flags;

    int         vnum;
    time_t      challenged;
    time_t      started;
    int         countdown;

#define DUEL_STAGE_READY 1
#define DUEL_STAGE_SET   2
#define DUEL_STAGE_GO    3
    int         stage;
};

bool             is_in_duel          (CHAR_DATA *ch, bool minstage);
bool             is_watching_duel    (CHAR_DATA *ch);
DUEL_DATA        *find_watching_duel (CHAR_DATA *ch);
DUEL_WATCHER_DATA *find_duel_watcher (CHAR_DATA *ch);
DUEL_DATA        *find_duel          (CHAR_DATA *ch);
DUEL_PLAYER_DATA *find_duel_player   (CHAR_DATA *ch);
DUEL_DATA        *create_duel        (void);
DUEL_PLAYER_DATA *create_duel_player (void);
int              find_duel_room      (void);
DUEL_DATA        *new_duel           (CHAR_DATA *ch, CHAR_DATA *victim);
void             start_duel          (DUEL_DATA *duel);
void             cancel_duel         (DUEL_DATA *duel, CHAR_DATA *ch, int type);
void             duel_update         (void);
void             duel_rawkill        (CHAR_DATA *ch, CHAR_DATA *victim, int type);

