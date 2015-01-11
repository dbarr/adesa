/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *       _/          _/_/_/     _/    _/     _/    ACK! MUD is modified    *
 *      _/_/        _/          _/  _/       _/    Merc2.0/2.1/2.2 code    *
 *     _/  _/      _/           _/_/         _/    (c)Stephen Dooley 1994  *
 *    _/_/_/_/      _/          _/  _/             "This mud has not been  *
 *   _/      _/      _/_/_/     _/    _/     _/      tested on animals."   *
 *                                                 Version #: 2.01         *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

#define args(list)             list
#define DECLARE_DO_FUN(fun)    DO_FUN    fun
#define DECLARE_SPEC_FUN(fun)  SPEC_FUN  fun
#define DECLARE_SPELL_FUN(fun) SPELL_FUN fun
#define DECLARE_OBJ_FUN(fun)   OBJ_FUN   fun

/*
 * Short scalar types.
 */
#if     !defined(NOWHERE)
# define NOWHERE -1
#endif

#if     !defined(FALSE)
# define FALSE    0
#endif

#if     !defined(TRUE)
# define TRUE     1
#endif

#if !defined(sh_int)
typedef short int sh_int;
#endif

#if !defined(bool)
typedef unsigned char bool;
#endif

typedef int long_int;

char *crypt args((const char *key, const char *salt));

/*
 *  Your mud info here
 */

#ifndef BPORT
# define mudnamecolor  "@@aA@@cd@@de@@gs@@Wa@@N"
# define mudnamenocolor  "Adesa"
# define sacgodname  "Muffin"
# define goodgodname  "Q"
# define neutralgodname  "Ogma"
# define evilgodname  "Erigol"
#else
# define mudnamecolor  "@@aA@@cd@@de@@gs@@Wa @@g(@@dBUILDER@@g)@@N"
# define mudnamenocolor  "Adesa (BUILDER)"
# define sacgodname  "Muffin"
# define goodgodname  "Q"
# define neutralgodname  "Ogma"
# define evilgodname  "Erigol"
#endif

/* thanks wd! */
#if defined(__GNUC__)
# define __UNUSED __attribute__ ((__unused__))
# define __INLINE inline
#else
# define __UNUSED
# define __INLINE
#endif
#define IDSTRING(var, string) static const char var[] __UNUSED = string

/*
 * String and memory management parameters.
 */
#define MAX_KEY_HASH             2048
#define MAX_STRING_LENGTH        8192
#define MSL         MAX_STRING_LENGTH
#define MAX_INPUT_LENGTH          640
#define MIL          MAX_INPUT_LENGTH
#define MAX_AREAS                 200
#define MAX_VNUM 32767

/* I believe this is the maximum number of CPU seconds the mud is allowed to consume while
 * in the booting process. Used to avoid infinite loops.
 */
#define BOOT_DB_ABORT_THRESHOLD 25

/* I believe this is the maximum number of CPU seconds the mud is allowed to consume in
 * ALARM_FREQUENCY seconds. Used to avoid infinite loops. */
#define RUNNING_ABORT_THRESHOLD 10

/* See above */
#define ALARM_FREQUENCY         20


/* Put various revision numbers here */
#define AREA_REVISION           3
/****/


/*
 * Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself.
 */

#define MAX_CLAN_EQ          6
#define MAX_COLOUR          30
#define MAX_ANSI            29
#define MAX_ALIASES         12
#define MAX_IGNORES          3
#define MAX_RACE            14
#define MAX_CLAN             9
#define MAX_SKILL          400
#define MAX_CLASS            5
#define MAX_LEVEL           90
#define MAX_QINFO           15
#define MAX_TRADEITEMS      10
#define MAX_MESSAGES        20
#define MOUNT_COST          10 /* moves lost while mounted */
#define MAX_STANCE          12
#define MAX_NUM_IMMS        10
#define LEVEL_HERO     (MAX_LEVEL - 9)
#define LEVEL_IMMORTAL (MAX_LEVEL - 8)

#define MAX_ITEM          32

/* Used as flags to the advance_level() command */
#define ADVANCE_ADEPT       32

/*
 * Well known mob virtual numbers.
 * Defined in #MOBILES.
 */

#define MOB_VNUM_DOGGY              99
#define MOB_VNUM_WATERELEM        1003
#define MOB_VNUM_SKELETON         1005
#define MOB_VNUM_FIREELEM         1004
#define MOB_VNUM_EARTHELEM        1028
#define MOB_VNUM_IRON_GOLEM       1029
#define MOB_VNUM_SOUL_THIEF       1030
#define MOB_VNUM_HOLY_AVENGER     1031
#define MOB_VNUM_DIAMOND_GOLEM    1032
#define MOB_VNUM_COMBAT_ELEMENTAL 1036 /* Conflict of Fire and Ice */
#define MOB_VNUM_PEGASUS          1037
#define MOB_VNUM_NIGHTMARE        1038
#define MOB_VNUM_ELEM_BEAST       1039
#define MOB_VNUM_INT_DEVOURER     1040
#define MOB_VNUM_SHADOW_HOUND     1041
#define MOB_VNUM_SHADOWDRAGON     1043
#define MOB_VNUM_ZOMBIE           1044

/* Random Quest Obj Vnum Range */
#define OBJ_VNUM_QUEST_MIN      66
#define OBJ_VNUM_QUEST_MAX      80

/* Pulses. A pulse is the basis of the mud system. PULSE_PER_SECOND of these
 * happen every second. If you're spamming in commands, only one per pulse is
 * executed. Increasing the pulse increases gameplay among other things. Some
 * of our code assumes that 8 pulses happen per second, which is probably bad! */
#define PULSE_PER_SECOND 8
#define PULSE_VIOLENCE   (2  * PULSE_PER_SECOND)
#define PULSE_MOBILE     (4  * PULSE_PER_SECOND)
#define PULSE_OBJFUN     (4  * PULSE_PER_SECOND)
#define PULSE_TICK       (60 * PULSE_PER_SECOND)
#define PULSE_MESSAGE    (12 * PULSE_PER_SECOND)
#define PULSE_ROOMS      (30 * PULSE_PER_SECOND)
#define PULSE_AREA       (80 * PULSE_PER_SECOND)
#define PULSE_AUCTION    (30 * PULSE_PER_SECOND)
#define PULSE_RAUCTION   (30 * PULSE_PER_SECOND)
#define PULSE_MAUCTION   (30 * PULSE_PER_SECOND)
#define PULSE_TELEPORT   (1  * PULSE_PER_SECOND)

#define LIQUID_BLOOD         13

/* Bitvector definitions */
#define  BIT_0 0
#define  BIT_1 1
#define  BIT_2 2
#define  BIT_3 4
#define  BIT_4 8
#define  BIT_5 16
#define  BIT_6 32
#define  BIT_7 64
#define  BIT_8 128
#define  BIT_9 256
#define BIT_10 512
#define BIT_11 1024
#define BIT_12 2048
#define BIT_13 4096
#define BIT_14 8192
#define BIT_15 16384
#define BIT_16 32768
#define BIT_17 65536
#define BIT_18 131072
#define BIT_19 262144
#define BIT_20 524288
#define BIT_21 1048576
#define BIT_22 2097152
#define BIT_23 4194304
#define BIT_24 8388608
#define BIT_25 16777216
#define BIT_26 33554432
#define BIT_27 67108864
#define BIT_28 134217728
#define BIT_29 268435456
#define BIT_30 536870912
#define BIT_31 1073741824
#define BIT_32 2147483648

/* Teleport flag defines */
#define TEL_NOMOB      BIT_1 /* mobs dont teleport */
#define TEL_NOPC       BIT_2 /* PCs dont teleport */
#define TEL_NOCHARM    BIT_3 /* charmies dont teleport */
#define TEL_FIGHTING   BIT_4 /* teleported even if fighting */
#define TEL_CHARMWAIT  BIT_5 /* charmies dont go if master fighting */
#define TEL_SOLITARY   BIT_6 /* one pc/npc at a time only */

/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
#define OBJ_VNUM_MONEY_ONE         2
#define OBJ_VNUM_MONEY_SOME        3
#define OBJ_VNUM_CORPSE_NPC       10
#define OBJ_VNUM_CORPSE_PC        11
#define OBJ_VNUM_LIGHT_BALL       21 /* for 'continual light' spell */
#define OBJ_VNUM_SPRING           19 /* for 'create spring' spell */
#define OBJ_VNUM_FOOD             30 /* for 'produce food' spell */
#define OBJ_VNUM_FIREBLADE        31 /* for 'fireblade' spell */
#define OBJ_VNUM_TOKEN            32 /* for token command */
#define OBJ_VNUM_WINDOW           33 /* for 'window' spell */
#define OBJ_VNUM_PORTAL           34 /* for 'portal' spell */
#define OBJ_VNUM_BEACON           35 /* for 'beacon' spell */
#define OBJ_VNUM_SOUL_POTION      37 /* for 'condense soul' spell */
#define OBJ_VNUM_CAPTURED_SOUL    38 /* for 'soul net' spell */
#define OBJ_VNUM_CONFLAGRATION  1036 /* for iceshield+cloak:flaming mob */

/*
 * Well known room virtual numbers.
 * Defined in #ROOMS.
 */
#define ROOM_VNUM_JAIL               1
#define ROOM_VNUM_LIMBO              2
#define ROOM_VNUM_TEMPLE          3001
#define ROOM_VNUM_SCHOOL          3700
#define ROOM_VNUM_MORGUE          3300
#define ROOM_VNUM_MORIBUND        3301
#define ROOM_VNUM_BUILDER         1019
#define ROOM_VNUM_BODIES            30 /* for switched imms, apparently */
#define ROOM_VNUM_CLAN            1022 /* for clan bosses */
#define ROOM_VNUM_DM_RECALL       3001
#define ROOM_VNUM_ETHEREAL_PLANE  3850
#define ROOM_VNUM_INT_HEAL        3871
#define ROOM_VNUM_MID_BOTTOM      3001 /* i think the code this is used in is incomplete */
#define ROOM_VNUM_MID_TOP         3200 /* ditto */

/* Stances */
#define STANCE_WARRIOR        0 /* warrior */
#define STANCE_CASTER         1 /* mage */
#define STANCE_AMBUSH         2 /* ninja */
#define STANCE_AC_BEST        3 /* shadows */
#define STANCE_HR_BEST        4 /* essence */
#define STANCE_DR_BEST        5 /* beast */
#define STANCE_AC_WORST       6 /* flame */
#define STANCE_HR_WORST       7 /* spirit */
#define STANCE_DR_WORST       8 /* void */
#define STANCE_SUPER_FIGHTER  9 /* dragon */
#define STANCE_SUPER_SPEED   10 /* snake */

/* MCCP stuff */
#define COMPRESS2         86 /* telnet option */
#define COMPRESS_BUF_SIZE 16384

/* Avatar "levels" */
#define AV_NOVICE         9
#define AV_INTERMEDIATE  31
#define AV_ADVANCED      54
#define AV_EXPERT        77
#define AV_MASTER       100

/*
 *  SSM stuff
 */
#define STR(x) #x
#define SX(x) STR(x)
#define _caller __FILE__ ":" SX(__LINE__)
#define fread_string(x) _fread_string((x), _caller)
char *_fread_string args((FILE * fp, const char *caller));
#define str_dup(x) _str_dup((x), _caller)
char *_str_dup args((const char *str, const char *caller));
#define fread_string_eol(x) _fread_string_eol((x), _caller)
char *_fread_string_eol args((FILE * fp, const char *caller));
#define free_string(x) _free_string((x), _caller)
void _free_string args((char *pstr, const char *caller));

/* YUCK! */
extern char *target_name;

/*
 * Structure types. This basically allows us to not use "struct". While this
 * could be confusing, it is MIGHTY convenient. You can just assume that a
 * capitalised data type is a struct
 */
typedef struct portal_data          PORTAL_DATA;
typedef struct affect_data          AFFECT_DATA;
typedef struct room_affect_data     ROOM_AFFECT_DATA;
typedef struct area_data            AREA_DATA;
typedef struct ban_data             BAN_DATA;
typedef struct char_data            CHAR_DATA;
typedef struct descriptor_data      DESCRIPTOR_DATA;
typedef struct exit_data            EXIT_DATA;
typedef struct extra_descr_data     EXTRA_DESCR_DATA;
typedef struct shelp_data           SHELP_DATA;
typedef struct kill_data            KILL_DATA;
typedef struct mob_index_data       MOB_INDEX_DATA;
typedef struct note_data            NOTE_DATA;
typedef struct obj_data             OBJ_DATA;
typedef struct obj_index_data       OBJ_INDEX_DATA;
typedef struct pc_data              PC_DATA;
typedef struct reset_data           RESET_DATA;
typedef struct room_index_data      ROOM_INDEX_DATA;
typedef struct shop_data            SHOP_DATA;
typedef struct time_info_data       TIME_INFO_DATA;
typedef struct weather_data         WEATHER_DATA;
typedef struct mob_prog_data        MPROG_DATA;
typedef struct mob_prog_act_list    MPROG_ACT_LIST;
typedef struct build_data_list      BUILD_DATA_LIST;
typedef struct mobprog_item         MOBPROG_ITEM;
typedef struct shield_data          SHIELD_DATA;
typedef struct politics_data_type   POL_DATA;
typedef struct member_data          MEMBER_DATA;
typedef struct trigger_data         TRIGGER_DATA;
typedef struct corpse_data          CORPSE_DATA;
typedef struct mark_data            MARK_DATA;
typedef struct mark_list_member     MARK_LIST_MEMBER;
typedef struct message_data         MESSAGE_DATA;
typedef struct quest_data           QUEST_DATA;
typedef struct answering_data       ANSWERING_DATA;
typedef struct ignore_data          IGNORE_DATA;
typedef struct cinfo_data           CINFO_DATA;
typedef struct board_data           BOARD_DATA;
typedef struct control_data         CONTROL_DATA;
typedef struct ruler_data           RULER_DATA;
typedef struct influence_data       INFLUENCE_DATA;
typedef struct interact_data        INTERACT_DATA;
typedef struct influence_list       INFLUENCE_LIST;
typedef struct control_list         CONTROL_LIST;
typedef struct queued_interact_list QUEUED_INTERACT_LIST;
typedef struct dl_list              DL_LIST;
typedef struct brand_data           BRAND_DATA;
typedef struct teleport_data        TELEPORT_DATA;
typedef struct dns_data             DNS_DATA;
typedef struct locker_data          LOCKER_DATA;
typedef struct rename_data          RENAME_DATA;
typedef struct buf_data_struct      BUF_DATA_STRUCT;

/*
 * Function types.
 */
typedef void DO_FUN    args((CHAR_DATA *ch, char *argument));
typedef bool SPEC_FUN  args((CHAR_DATA *ch));
typedef bool SPELL_FUN args((int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj));
typedef void OBJ_FUN   args((OBJ_DATA *obj, CHAR_DATA *keeper));
typedef void RET_FUN   args((void *, char **, CHAR_DATA *, bool));

/* Used by the enchant command */
#define ENCHANT_EXTRA_FLAGS -1
#define ENCHANT_APPLY_FLAGS -2
#define ENCHANT_OBJFUNS     -3

/* These are for skill_table lookup funcs... to save writing 2 functions */
#define RETURN_BEST_LEVEL   1
#define RETURN_BEST_CLASS   2

/* Used by interp() */
#define CLAN_ONLY -1
#define BOSS_ONLY -2

/*
 * Updated pointer referencing, curtesy of Spectrum, from Beyond the Veil
 *
 */
#define OBJ_NEXT        1
#define OBJ_NEXTCONTENT 2
#define OBJ_NULL        3

struct obj_ref_type
{
    bool                inuse;
    struct obj_ref_type *next;
    OBJ_DATA            **var;
    int type;
};

#define CHAR_NEXT     1
#define CHAR_NEXTROOM 2
#define CHAR_NULL     3

struct char_ref_type
{
    bool                 inuse;
    struct char_ref_type *next;
    CHAR_DATA            **var;
    int                  type;
};

#define OREF(v, type) do { \
    static struct obj_ref_type s = {FALSE, NULL, NULL, type}; s.var = &v; \
    obj_reference(&s); \
} while(0)

#define OUREF(var) obj_unreference(&var);

#define CREF(v, type) do { \
    static struct char_ref_type s = {FALSE, NULL, NULL, type}; s.var = &v; \
    char_reference(&s); \
} while(0)

#define CUREF(var) char_unreference(&var);

struct board_data
{
    bool         is_free;
    BOARD_DATA   *next;
    BOARD_DATA   *prev;

    int          vnum;
    MESSAGE_DATA *first_message;
    MESSAGE_DATA *last_message;
    int          min_read_lev;
    int          min_write_lev;
    int          expiry_time;
    int          clan;
};

struct message_data
{
    bool         is_free;
    MESSAGE_DATA *next;
    MESSAGE_DATA *prev;

    BOARD_DATA   *board;
    time_t       datetime;
    char         *author;
    char         *title;
    char         *message;

};

struct teleport_data
{
    bool            is_free;
    TELEPORT_DATA   *next;
    TELEPORT_DATA   *prev;

    int             flags; /* flags for room teleports */
    int             vnum;  /* vnum to teleport to */
    int             wait;  /* teleport every wait seconds */
    int             timer; /* used to count down, whee! */
    char            *in;   /* message to show ch when teleporting em */
    char            *out;  /* message to show room when a ch arrives */
    ROOM_INDEX_DATA *room; /* what room is it in? */
};

struct quest_data
{
    bool         is_free;
    QUEST_DATA   *next;
    QUEST_DATA   *prev;

    char         *mob;   /* who was the questor? */
    char         *thief; /* who stole the item? */
    char         *ch;    /* which player returned the item? */
    int          qp;     /* how much was it worth? */

#define QUEST_NORMAL BIT_1
#define QUEST_ENDED  BIT_2
#define QUEST_KILLED BIT_3
#define QUEST_MASK   BIT_4
    int          flags;  /* various flags... */
};

struct locker_data
{
    bool        is_free;
    LOCKER_DATA *next;
    LOCKER_DATA *prev;

    ROOM_INDEX_DATA *room;
#define LOCKER_SAVEALL BIT_1
    int         flags;
    int         maxitem;
    int         maxweight;
    bool        types[MAX_ITEM];
    bool        valid;
};

struct answering_data
{
    bool           is_free;
    ANSWERING_DATA *next;
    ANSWERING_DATA *prev;

    char           *name;
    char           *message;
    time_t         time;
};

struct ignore_data
{
    bool        is_free;
    IGNORE_DATA *next;
    IGNORE_DATA *prev;

    char        *char_ignored;
};

struct cinfo_data
{
    bool       is_free;
    CINFO_DATA *next;
    CINFO_DATA *prev;

    char       *name;
    int        clan;
    int        position;
    int        flags;
    time_t     lastlogin;
    int        level;
    int        remort;
    int        adept;
};

struct rename_data
{
    bool       is_free;
    RENAME_DATA *next;
    RENAME_DATA *prev;

    char        *playername;
    char        *oldshort;
    char        *oldlong;
    char        *oldkeyword;
    char        *newshort;
    char        *newlong;
    char        *newkeyword;
    unsigned long int id;
};

/*
 * Colour look-up table structure thingy.
 */

struct colour_type
{
    char *name;
    int  index;
};

struct ansi_type
{
    char *name;
    char *value;
    int  index;
    char letter;
    int  stlen;
};

struct avatarlimit_type
{
    char *name;
    int limit;
};

/* Structure for material/strengths. Not even used, but I'll leave it in. */
struct material_type
{
   char *name;    /* Name of the material */
   char *descrip; /* Descr. of how strong it is */
   int  quality;  /* 0 = crap, 100 = non-breakable */
};

#define NO_MATERIAL 10  /* Number of materials */

/* For brands */
struct dl_list
{
    bool    is_free;
    DL_LIST *next;
    DL_LIST *prev;

    void    *this_one;
};

/*
 * Site ban structure.
 */
struct ban_data
{
    bool     is_free;
    BAN_DATA *next;
    BAN_DATA *prev;

    char     *name;
    char     *banned_by;
    bool     newbie;
};

struct brand_data
{
    bool       is_free;
    BRAND_DATA *next;
    BRAND_DATA *prev;
    char       *branded;
    char       *branded_by;
    char       *dt_stamp;
    char       *message;
    char       *priority;
};


/*
 * Time and weather stuff. Weather stuff is SO useless!
 */
#define SUN_DARK      0
#define SUN_RISE      1
#define SUN_LIGHT     2
#define SUN_SET       3

#define SKY_CLOUDLESS 0
#define SKY_CLOUDY    1
#define SKY_RAINING   2
#define SKY_LIGHTNING 3

#define MOON_DOWN     0
#define MOON_RISE     1
#define MOON_LOW      2
#define MOON_PEAK     3
#define MOON_FALL     4
#define MOON_SET      5

#define MOON_NEW      0
#define MOON_WAX_CRE  1
#define MOON_WAX_HALF 2
#define MOON_WAX_GIB  3
#define MOON_FULL     4
#define MOON_WAN_GIB  5
#define MOON_WAN_HALF 6
#define MOON_WAN_CRE  7

struct time_info_data
{
    int minute;
    int hour;
    int day;
    int month;
    int year;
    int moon;
};

struct weather_data
{
    int  mmhg;
    int  change;
    int  sky;
    int  sunlight;
    int  moon_phase;
    int  moon_loc;
    bool phase_changed;
};

struct corpse_data
{
    bool        is_free;
    CORPSE_DATA *next;
    CORPSE_DATA *prev;

    OBJ_DATA    *this_corpse;
};

/*
 * Connected state for a channel.
 */

/* These values referenced by users command and other places. Negative means not playing */
#define CON_SETTING_STATS          1
#define CON_PLAYING                0
#define CON_GET_NAME              -1
#define CON_GET_OLD_PASSWORD      -2
#define CON_CONFIRM_NEW_NAME      -3
#define CON_GET_NEW_PASSWORD      -4
#define CON_CONFIRM_NEW_PASSWORD  -5
#define CON_GET_NEW_SEX           -6
#define CON_GET_NEW_CLASS         -7
#define CON_GET_RACE              -9
#define CON_READ_MOTD            -10
#define CON_GET_STATS            -11
#define CON_FINISHED             -12 /* Unused */
#define CON_MENU                 -13
#define CON_COPYOVER_RECOVER     -14
#define CON_QUITTING             -15
#define CON_RECONNECTING         -16

/* values used to check a new player has selected all options */
#define CHECK_RACE   BIT_1
#define CHECK_CLASS  BIT_2
#define CHECK_SEX    BIT_3
#define CHECK_STATS  BIT_4

/* need this include for z_stream */
#include <zlib.h>

/*
 * Descriptor (channel) structure.
 */
struct  descriptor_data
{
    bool            is_free;
    DESCRIPTOR_DATA *next;
    DESCRIPTOR_DATA *prev;
    DESCRIPTOR_DATA *snoop_by;
    CHAR_DATA       *character;
    CHAR_DATA       *original;
    char            *host;
    char            *ip;
    sh_int          descriptor;
    sh_int          connected;
    bool            fcommand;
    char            inbuf[4 * MIL];
    char            incomm[MIL];
    char            inlast[MIL];
    int             repeat;
    char            *showstr_head;
    char            *showstr_point;
    char            *outbuf;
    int             outsize;
    int             outtop;
    z_stream        *out_compress;
    unsigned char   *out_compress_buf;
    unsigned int    remote_port;
    int             check;
    int             flags;
    int             childpid;
    time_t          timeout;
    char            challenge[33];
};

#define DESC_FLAG_PASSTHROUGH 1     /* Used when data is being passed to another prog */

struct family_name_type
{
    char  *name;
    sh_int clan_enemy;
};

struct politics_data_type
{
    sh_int diplomacy[MAX_CLAN][MAX_CLAN];
    bool   daily_negotiate_table[MAX_CLAN][MAX_CLAN];
    int    treasury[MAX_CLAN];
    bool   end_current_state[MAX_CLAN][MAX_CLAN];
};

struct mudset_type
{
    char    *name;
    void    *var;

#define MUDSET_TYPE_BOOL   1
#define MUDSET_TYPE_INT    2
#define MUDSET_TYPE_STRING 3
    int     type;
};

struct stance_app_type
{
    char   *name;
    int ac_mod;
    int dr_mod;
    int hr_mod;
    sh_int speed_mod;
};

/*
 * Attribute bonus structures.
 */
struct str_app_type
{
    sh_int tohit;
    sh_int todam;
    sh_int carry;
    sh_int wield;
};

struct int_app_type
{
    sh_int learn;
    sh_int spell_mod;
    sh_int mana_regen;
};

struct wis_app_type
{
    sh_int practice;
    sh_int spell_save;
};

struct dex_app_type
{
    int defensive;
};

struct con_app_type
{
    int hitp;
    sh_int shock;
};

/*
 * TO types for act().
 */
#define TO_ROOM    0
#define TO_NOTVICT 1
#define TO_VICT    2
#define TO_CHAR    3

/*
 * shelp addition
 */
struct shelp_data
{
    bool       is_free;
    SHELP_DATA *next;
    SHELP_DATA *prev;

    char       *name;
    char       *duration;
    char       *modify;
    char       *type;
    char       *target;
    char       *desc;
};

/*
 * Shop types.
 */
#define MAX_TRADE 5

struct shop_data
{
    bool      is_free;
    SHOP_DATA *next;
    SHOP_DATA *prev;
    sh_int    keeper;              /* Vnum of shop keeper mob      */
    sh_int    buy_type[MAX_TRADE]; /* Item types shop will buy     */
    sh_int    profit_buy;          /* Cost multiplier for buying   */
    sh_int    profit_sell;         /* Cost multiplier for selling  */
    sh_int    open_hour;           /* First opening hour           */
    sh_int    close_hour;          /* First closing hour           */
};

/*
 * Per-class stuff. A lot of the variables are outdated or unused.
 */
struct class_type
{
    char   who_name[4]; /* Three-letter name for 'who' */
    char   *class_name; /* Full name                   */
    sh_int attr_prime;  /* Prime attribute             */
    char   *attr;       /* Prime                       */
    sh_int weapon;      /* First weapon                */
    sh_int guild;       /* Vnum of guild room          */
    sh_int skill_adept; /* Maximum skill level         */
    sh_int thac0_00;    /* Thac0 for level  0          */
    sh_int thac0_32;    /* Thac0 for level 32          */
    int hp_min;         /* Min hp gained on leveling   */
    int hp_max;         /* Max hp gained on leveling   */
    bool   fMana;       /* Class gains mana on level   */
    char   *skill1;     /* Auto-learnt skill if any    */
};

/* Racial flags. Some unused */
#define RACE_MOD_NONE          BIT_0
#define RACE_MOD_FAST_HEAL     BIT_1
#define RACE_MOD_SLOW_HEAL     BIT_2
#define RACE_MOD_STRONG_MAGIC  BIT_3
#define RACE_MOD_WEAK_MAGIC    BIT_4
#define RACE_MOD_NO_MAGIC      BIT_5
#define RACE_MOD_IMMUNE_POISON BIT_6
#define RACE_MOD_RESIST_SPELL  BIT_7
#define RACE_MOD_WOODLAND      BIT_8
#define RACE_MOD_DARKNESS      BIT_9
#define RACE_MOD_HUGE          BIT_10
#define RACE_MOD_LARGE         BIT_11
#define RACE_MOD_TINY          BIT_12
#define RACE_MOD_SMALL         BIT_13
#define RACE_MOD_TAIL          BIT_14

#define RACE_HMN  0
#define RACE_HOB  1
#define RACE_DWF  2
#define RACE_ELF  3
#define RACE_GNO  4
#define RACE_OGR  5
#define RACE_DRW  6
#define RACE_LAM  7
#define RACE_DRG  8
#define RACE_CEN  9
#define RACE_TTN 10
#define RACE_PIX 11
#define RACE_MIN 12
#define RACE_TRL 13

/* Racial table. Some variables unused */
struct race_type
{
    char   race_name[4];     /* Three letter name for race */
    char   *race_title;      /* Full race name */
    sh_int recall;           /* Race's recall location */
    sh_int race_room;        /* vnum of race-only room */
    sh_int race_str;         /* max_str to use for race */
    sh_int race_int;
    sh_int race_wis;
    sh_int race_dex;
    sh_int race_con;
    int    race_flags;       /* flags for the various racial stuff */
    int    classes;          /* Number of classes for race */
    sh_int limit[MAX_CLASS]; /* Max for each class */
    char   *comment;         /* comments shown for new players */
    char   *skill1;

};

struct clan_type
{
    char   *clan_name;      /* The name of the Clan */
    char   *clan_abbr;      /* Abbrev. name - FIVE CHARS */
    sh_int donat_room;      /* clan donation */
    sh_int clan_room;       /* Clan-only room */
    char   *leader;         /* Clan leader */
    char   *enemies;        /* Enemies (if any) */
    int    eq[MAX_CLAN_EQ]; /* vnums of objects to load */
};

struct exp_type
{
    long_int mob_base;            /* Base exp for mob of level x  */
    long_int exp_base[MAX_CLASS]; /* Cost for each class of level */
};

/*
 * Data structure for notes.
 */
struct note_data
{
    bool      is_free;
    NOTE_DATA *next;
    NOTE_DATA *prev;

    char      *from;
    char      *to;
    char      *subject;
    char      *text;
    time_t    date_stamp;
    bool      unread;
};

/*
 * An affect.
 */
struct affect_data
{
    bool        is_free;
    AFFECT_DATA *next;
    AFFECT_DATA *prev;

    sh_int      type;
    sh_int      duration;
    sh_int      location;
    int         modifier;
    int         bitvector;
    CHAR_DATA   *caster;
    int         level;
    int         save;
};

struct room_affect_data
{
    bool             is_free;
    ROOM_AFFECT_DATA *next;
    ROOM_AFFECT_DATA *prev;
    sh_int           duration;
    sh_int           level;
    int              type;
    int              bitvector;
    int              applies_spell; /* what spell is cast on a ch by the room.. a sn */
    int              modifier;
    sh_int           location;
    CHAR_DATA        *caster;
    char             *name; /* name of rune/affect */
};

/*
 * A kill structure (indexed by level). Set, but only used in weird imm command(?)
 */
struct kill_data
{
    sh_int number;
    sh_int killed;
};

#define SHIELD_NONE    0
#define SHIELD_FIRE    1
#define SHIELD_ICE     2
#define SHIELD_SHOCK   3
#define SHIELD_DEMON   4

#define CLOAK_NONE     0
#define CLOAK_FLAMING  1
#define CLOAK_ADEPT    2
#define CLOAK_MANA     3
#define CLOAK_ABSORB   4
#define CLOAK_REFLECT  5
#define CLOAK_REGEN    6

struct shield_type
{
    char        *name;          /* name of the shield (including colours) */
    bool        harmful;        /* does the shield damage the victim? */
    int         damage;         /* damage amount to victim */
    int         protection;     /* absorb percentage of regular damage */
    int         mprotection;    /* absorb percentage of regular damage, if mob */
    char        *absorb_self;
    char        *absorb_victim;
    char        *absorb_room;
    char        *add_self;
    char        *add_room;
    char        *remove_self;
    char        *remove_room;
};

struct shield_data
{
    bool         is_free;
    SHIELD_DATA  *next;
    SHIELD_DATA  *prev;

    sh_int       index;    /* The index to the shield table */
    int          hit;      /* Shield hitpoints */
};

struct buf_data_struct
{
    bool                is_free;
    BUF_DATA_STRUCT    *next;
    BUF_DATA_STRUCT    *prev;
    CHAR_DATA          *ch;
    char               **dest;
    char               *buf;
    int                 pos;
    RET_FUN            *returnfunc;
    void               *returnparm;
    int                 old_char_pos;
};

/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (Start of section ... start here)                     *
 *                                                                         *
 ***************************************************************************/

/********************* Define Flags for hunting *************************/
#define ACT_HUNT_CHAR   BIT_1 /* Hunting a character            */
#define ACT_HUNT_OBJ    BIT_2 /* Hunting an object              */
#define ACT_HUNT_INFORM BIT_3 /* Mob will gossip when hunting   */
#define ACT_HUNT_CR     BIT_4 /* Mob is doing a corpse retrival */
#define ACT_HUNT_MOVE   BIT_5 /* Just walking somewhere         */

#define NO_VNUM            -1  /* For ch->move_to thingy        */
/************************************************************************/

/** Define what is calling the trigger handler for objects ******/
#define TRIGGER_OPEN        1 /* Opening a container     UNUSED */
#define TRIGGER_CLOSE       2 /* Closing a container     UNUSED */
#define TRIGGER_EAT         3 /* Eating some food        UNUSED */
#define TRIGGER_C_DRINK     4 /* Drinking from container UNUSED */
#define TRIGGER_F_DRINK     5 /* Drinking from fountain  UNUSED */
#define TRIGGER_WEAR        6 /* Wearing an object       UNUSED */
#define TRIGGER_GET         7 /* Picking up an object    */
#define TRIGGER_DROP        8 /* Dropping an object      UNUSED */
#define TRIGGER_EXAMINE     9 /* Examining an object     */

/** Define what actions the triggered object can do *************/
#define ACTION_TRANSPORT    1 /* Transports victim to room */
#define ACTION_RESTORE      2 /* Restores victim           */
#define ACTION_SLAY         3 /* Kills victim              */
#define ACTION_TRANSFORM    4 /* Loads mob(s) in its place */
/****************************************************************/

/* BitVector flags for room-affect spells.  These are used in the
   ROOM_AFFECT_DATA structure, and passes to the major handling
   functions.  They MUST be used whenever a room-affect spell is
   being cast (from within code) */

#define ROOM_BV_NONE           BIT_0
#define ROOM_BV_SILENCE        BIT_1  /* Like silence room flag                      */
#define ROOM_BV_SAFE           BIT_2  /* Like safe room flag                         */
#define ROOM_BV_ENCAPS         BIT_3  /* magically blocks exits                      */
/* BIT_4 unused (was shade) */
#define ROOM_BV_HEAL_REGEN     BIT_5  /* room heals hits quicker                     */
#define ROOM_BV_HEAL_STEAL     BIT_6  /* room takes hits instead of giving           */
#define ROOM_BV_MANA_REGEN     BIT_7  /* room heals mana quicker                     */
#define ROOM_BV_MANA_STEAL     BIT_8  /* room saps mana                              */
#define ROOM_BV_FIRE_RUNE      BIT_9  /* room does fire damage                       */
#define ROOM_BV_FIRE_TRAP      BIT_10 /* room is fire trapped                        */
#define ROOM_BV_DAMAGE_TRAP    BIT_11 /* room is physical damage trapped             */
#define ROOM_BV_SHOCK_RUNE     BIT_12 /* room is shock runed                         */
#define ROOM_BV_SHOCK_TRAP     BIT_13 /* room is shock trapped                       */
#define ROOM_BV_SPELL_ON_ENTER BIT_14 /* room spell cast on entrance                 */
#define ROOM_BV_SPELL_ALWAYS   BIT_15 /* room casts spell continuously               */
#define ROOM_BV_HOLD           BIT_16 /* room lets you in, but not out..recall works */
#define ROOM_BV_POISON_RUNE    BIT_17 /* room gives poison to entering ch            */
#define ROOM_BV_SOUL_NET       BIT_18 /* makes soul instead of corpse                */
#define ROOM_BV_WARNING_RUNE   BIT_19 /* warning runes! fun                          */
#define ROOM_BV_SMOKESCREEN    BIT_20 /* smokescreen                                 */
#define ROOM_BV_SENTRY         BIT_21 /* sentry                                      */
#define ROOM_BV_SMOKESCREEN_AREA BIT_22 /* area-wide smokescreen                     */

/*
 * ACT bits for mobs.
 * Used in #MOBILES.
 */
#define ACT_IS_NPC      BIT_1  /* Auto set for mobs    */
#define ACT_SENTINEL    BIT_2  /* Stays in one room    */
#define ACT_SCAVENGER   BIT_3  /* Picks up objects     */
#define ACT_REMEMBER    BIT_4  /* Remembers target     */
#define ACT_NO_FLEE     BIT_5  /* Can't flee from mob  */
#define ACT_AGGRESSIVE  BIT_6  /* Attacks PC's         */
#define ACT_STAY_AREA   BIT_7  /* Won't leave area     */
#define ACT_WIMPY       BIT_8  /* Flees when hurt      */
#define ACT_PET         BIT_9  /* Auto set for pets    */
#define ACT_TRAIN       BIT_10 /* Can train PC's       */
#define ACT_PRACTICE    BIT_11 /* Can practice PC's    */
#define ACT_NPCPROT     BIT_12 /* Protection against NPC normal attacks */
#define ACT_HEAL        BIT_13 /* Sells spells         */
#define ACT_ADAPT       BIT_14 /* Adapts weapons       */
#define ACT_UNDEAD      BIT_15 /* Mob is undead        */
#define ACT_BANKER      BIT_16 /* Is a banker          */
#define ACT_NO_BODY     BIT_17 /* No body for damage   */
#define ACT_HUNTER      BIT_18 /* Hunts attackers      */
#define ACT_NOMIND      BIT_19 /* Psi attack no-no     */
#define ACT_NODISPEL    BIT_20 /* Mob is undispellable */ /* (was originally Postmaster bit) */
#define ACT_REWIELD     BIT_21 /* Uses better weapons  */
#define ACT_RE_EQUIP    BIT_22 /* Uses better equipment*/
#define ACT_NOASSIST    BIT_23 /* Won't assist other mobs */
#define ACT_RAND_TARGET BIT_24 /* Mob attacks a random target */
#define ACT_SAFE        BIT_25 /* Mob can not be attacked */
#define ACT_SOLO        BIT_26 /* Mob does solo combat */
#define ACT_NOLIFESTEAL BIT_27 /* Mob can't be lifestolen from */
#define ACT_MOUNT       BIT_28 /* Mountable MOB        */
#define ACT_NOMINDSTEAL BIT_29 /* Mob can't be mindstolen from */
#define ACT_ALWAYS_AGGR BIT_30 /* Always aggressive    */
#define ACT_NORESCUE    BIT_31 /* Can't rescued from   */

/* build bits for OLC -S- */
#define ACT_BUILD_NOWT   0 /* not doing anything */
#define ACT_BUILD_REDIT  1 /* editing rooms      */
#define ACT_BUILD_OEDIT  2 /* editing objects    */
#define ACT_BUILD_MEDIT  3 /* editing mobiles    */
#define ACT_BUILD_MPEDIT 4 /* editing mobprogs   */

/* for buildtab.c. table entry can't be used except by creator, apparently */
#define NO_USE -999

/*
 * New bits to determine what skills a mob can do in combat -S-
 */

#define MOB_NONE      BIT_1
#define MOB_SECOND    BIT_2
#define MOB_THIRD     BIT_3
#define MOB_FOURTH    BIT_4
#define MOB_PUNCH     BIT_5
#define MOB_HEADBUTT  BIT_6
#define MOB_KNEE      BIT_7
#define MOB_DISARM    BIT_8
#define MOB_TRIP      BIT_9
#define MOB_NODISARM  BIT_10
#define MOB_NOTRIP    BIT_11
#define MOB_DODGE     BIT_12
#define MOB_PARRY     BIT_13
#define MOB_MARTIAL   BIT_14
#define MOB_ENHANCED  BIT_15
#define MOB_DUALWIELD BIT_16
#define MOB_DIRT      BIT_17
#define MOB_FIFTH     BIT_18
#define MOB_SIXTH     BIT_19
#define MOB_CHARGE    BIT_20

/*
 * New bits to determine what spells a mob will cast in combat -S-
 * These are for offensive spells.
 * Remember spec_funs may still cast as well! - but spells cast
 *  this way WILL reduce the mob's mana <g>
 */

#define CAST_NONE            BIT_1
#define CAST_MAGIC_MISSILE   BIT_2
#define CAST_SHOCKING_GRASP  BIT_3
#define CAST_BURNING_HANDS   BIT_4
#define CAST_COLOUR_SPRAY    BIT_5
#define CAST_FIREBALL        BIT_6
#define CAST_HELLSPAWN       BIT_7
#define CAST_ACID_BLAST      BIT_8
#define CAST_CHAIN_LIGHTNING BIT_9
/* BIT_10 unused */
#define CAST_FLARE           BIT_11
#define CAST_FLAMESTRIKE     BIT_12
#define CAST_EARTHQUAKE      BIT_13
#define CAST_MIND_FLAIL      BIT_14
#define CAST_PLANERGY        BIT_15
#define CAST_PHOBIA          BIT_16
#define CAST_MIND_BOLT       BIT_17
#define CAST_STATIC          BIT_18
#define CAST_EGO_WHIP        BIT_19
#define CAST_BLOODY_TEARS    BIT_20
#define CAST_MINDFLAME       BIT_21
#define CAST_SUFFOCATE       BIT_22
#define CAST_NERVE_FIRE      BIT_23
#define CAST_LIGHTNING_BOLT  BIT_24
#define CAST_HEAT_ARMOR      BIT_25
#define CAST_LAVA_BURST      BIT_26

/*
 * New bits to determine the defensive spells available to
 * mobs. May be used in fights (cure light, heal, etc)
 * will deduct mana from the mob when cast.
 */

#define DEF_NONE          BIT_1
#define DEF_CURE_LIGHT    BIT_2
#define DEF_CURE_SERIOUS  BIT_3
#define DEF_CURE_CRITIC   BIT_4
#define DEF_CURE_HEAL     BIT_5
#define DEF_SHIELD_FIRE   BIT_6
#define DEF_SHIELD_ICE    BIT_7
#define DEF_SHIELD_SHOCK  BIT_8
#define DEF_SHIELD_DEMON  BIT_9
#define DEF_CURE_DIVINE   BIT_10
#define DEF_CLOAK_ABSORB  BIT_11
#define DEF_CLOAK_REFLECT BIT_12
#define DEF_CLOAK_FLAMING BIT_13
#define DEF_SHIELD_RECAST BIT_14

/*
 * Bits for 'affected_by'.
 * Used in #MOBILES.
 * Some are unused.
 */

#define AFF_BLIND            BIT_1
#define AFF_INVISIBLE        BIT_2
#define AFF_DETECT_EVIL      BIT_3
#define AFF_DETECT_INVIS     BIT_4
#define AFF_DETECT_MAGIC     BIT_5
#define AFF_DETECT_HIDDEN    BIT_6
#define AFF_CLOAK_REFLECTION BIT_7
#define AFF_SANCTUARY        BIT_8
#define AFF_FAERIE_FIRE      BIT_9
#define AFF_INFRARED         BIT_10
#define AFF_CURSE            BIT_11
#define AFF_CLOAK_FLAMING    BIT_12
#define AFF_POISON           BIT_13
#define AFF_PROTECT          BIT_14
#define AFF_CLOAK_ABSORPTION BIT_15
#define AFF_SNEAK            BIT_16
#define AFF_HIDE             BIT_17
#define AFF_SLEEP            BIT_18
#define AFF_CHARM            BIT_19
#define AFF_FLYING           BIT_20
#define AFF_PASS_DOOR        BIT_21
#define AFF_ANTI_MAGIC       BIT_22
#define AFF_DETECT_UNDEAD    BIT_23
#define AFF_BERSERK          BIT_24
/* BIT_25 unused */
/* BIT_26 unused */
#define AFF_HOLD             BIT_27
#define AFF_PARALYSIS        BIT_28
#define AFF_CLOAK_ADEPT      BIT_29
#define AFF_CLOAK_REGEN      BIT_30
#define AFF_CLOAK_MANA       BIT_31

/*
 * Sex.
 * Used in #MOBILES, and various other places.
 */
#define SEX_NEUTRAL 0
#define SEX_MALE    1
#define SEX_FEMALE  2

/*
 * Item types.
 * Used in #OBJECTS.
 */
#define ITEM_LIGHT        1
#define ITEM_SCROLL       2
#define ITEM_WAND         3
#define ITEM_STAFF        4
#define ITEM_WEAPON       5
#define ITEM_BEACON       6
#define ITEM_PORTAL       7
#define ITEM_TREASURE     8
#define ITEM_ARMOR        9
#define ITEM_POTION       10
#define ITEM_CLUTCH       11
#define ITEM_FURNITURE    12
#define ITEM_TRASH        13
#define ITEM_TRIGGER      14
#define ITEM_CONTAINER    15
#define ITEM_QUEST        16
#define ITEM_DRINK_CON    17
#define ITEM_KEY          18
#define ITEM_FOOD         19
#define ITEM_MONEY        20
#define ITEM_STAKE        21
#define ITEM_BOAT         22
#define ITEM_CORPSE_NPC   23
#define ITEM_CORPSE_PC    24
#define ITEM_FOUNTAIN     25
#define ITEM_PILL         26
#define ITEM_BOARD        27
#define ITEM_SOUL         28
#define ITEM_PIECE        29
#define ITEM_SPELL_MATRIX 30
#define ITEM_ENCHANTMENT  31

/*
 * Extra flags.
 * Used in #OBJECTS.
 */
#define ITEM_GLOW         BIT_1
#define ITEM_HUM          BIT_2
#define ITEM_NODISARM     BIT_3
#define ITEM_LOCK         BIT_4
#define ITEM_EVIL         BIT_5
#define ITEM_INVIS        BIT_6
#define ITEM_MAGIC        BIT_7
#define ITEM_NODROP       BIT_8
#define ITEM_BLESS        BIT_9
#define ITEM_ANTI_GOOD    BIT_10
#define ITEM_ANTI_EVIL    BIT_11
#define ITEM_ANTI_NEUTRAL BIT_12
#define ITEM_NOREMOVE     BIT_13
#define ITEM_INVENTORY    BIT_14
#define ITEM_NOSAVE       BIT_15
#define ITEM_CLAN_EQ      BIT_16
#define ITEM_TRIG_DESTROY BIT_17
#define ITEM_NO_AUCTION   BIT_18
#define ITEM_REMORT       BIT_19
#define ITEM_ADEPT        BIT_20
#define ITEM_RARE         BIT_21
#define ITEM_NODISPEL     BIT_22
#define ITEM_NOLOOT       BIT_23
#define ITEM_NOSAC        BIT_24
#define ITEM_UNIQUE       BIT_25
#define ITEM_LIFESTEALER  BIT_26
/* BIT_27 is not in use */
#define ITEM_MINDSTEALER  BIT_28
#define ITEM_NOSELL       BIT_29
#define ITEM_NODESTROY    BIT_30
#define ITEM_NOSTEAL      BIT_31

/* Magical applies for items */
#define ITEM_APPLY_NONE        BIT_1
#define ITEM_APPLY_INFRA       BIT_2
#define ITEM_APPLY_INV         BIT_3
#define ITEM_APPLY_DET_INV     BIT_4
#define ITEM_APPLY_SANC        BIT_5
#define ITEM_APPLY_SNEAK       BIT_6
#define ITEM_APPLY_HIDE        BIT_7
#define ITEM_APPLY_PROT        BIT_8
#define ITEM_APPLY_ENHANCED    BIT_9
#define ITEM_APPLY_DET_MAG     BIT_10
#define ITEM_APPLY_DET_HID     BIT_11
#define ITEM_APPLY_DET_EVIL    BIT_12
#define ITEM_APPLY_PASS_DOOR   BIT_13
#define ITEM_APPLY_DET_POISON  BIT_14
#define ITEM_APPLY_FLY         BIT_15
#define ITEM_APPLY_KNOW_ALIGN  BIT_16
#define ITEM_APPLY_DET_UNDEAD  BIT_17
#define ITEM_APPLY_HEATED      BIT_18
#define ITEM_APPLY_ARENAHEATED BIT_19

/*
 * Wear flags.
 * Used in #OBJECTS.
 */
#define ITEM_TAKE        BIT_1
#define ITEM_WEAR_FINGER BIT_2
#define ITEM_WEAR_NECK   BIT_3
#define ITEM_WEAR_BODY   BIT_4
#define ITEM_WEAR_HEAD   BIT_5
#define ITEM_WEAR_LEGS   BIT_6
#define ITEM_WEAR_FEET   BIT_7
#define ITEM_WEAR_HANDS  BIT_8
#define ITEM_WEAR_ARMS   BIT_9
#define ITEM_WEAR_SHIELD BIT_10
#define ITEM_WEAR_ABOUT  BIT_11
#define ITEM_WEAR_WAIST  BIT_12
#define ITEM_WEAR_WRIST  BIT_13
#define ITEM_WIELD       BIT_14
#define ITEM_HOLD        BIT_15
#define ITEM_WEAR_FACE   BIT_16
#define ITEM_WEAR_EAR    BIT_17
#define ITEM_HOLD_MAGIC  BIT_18
/* BIT_19 unused */
#define ITEM_WEAR_CLAN   BIT_20

/*
 * Apply types (for affects).
 * Used in #OBJECTS.
 */
#define APPLY_NONE          0
#define APPLY_STR           1
#define APPLY_DEX           2
#define APPLY_INT           3
#define APPLY_WIS           4
#define APPLY_CON           5
#define APPLY_SEX           6
#define APPLY_CLASS         7
#define APPLY_LEVEL         8
#define APPLY_AGE           9
#define APPLY_HEIGHT        10
#define APPLY_WEIGHT        11
#define APPLY_MANA          12
#define APPLY_HIT           13
#define APPLY_MOVE          14
#define APPLY_GOLD          15
#define APPLY_EXP           16
#define APPLY_AC            17
#define APPLY_HITROLL       18
#define APPLY_DAMROLL       19
#define APPLY_SAVING_PARA   20
#define APPLY_SAVING_ROD    21
#define APPLY_SAVING_PETRI  22
#define APPLY_SAVING_BREATH 23
#define APPLY_SAVING_SPELL  24

/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE BIT_1
#define CONT_PICKPROOF BIT_2
#define CONT_CLOSED    BIT_3
#define CONT_LOCKED    BIT_4

/*
 * Room flags.
 * Used in #ROOMS.
 */
#define ROOM_DARK        BIT_1
#define ROOM_REGEN       BIT_2
#define ROOM_NO_MOB      BIT_3
#define ROOM_INDOORS     BIT_4
#define ROOM_NO_MAGIC    BIT_5
#define ROOM_HOT         BIT_6
#define ROOM_COLD        BIT_7
#define ROOM_PK          BIT_8
#define ROOM_QUIET       BIT_9
#define ROOM_PRIVATE     BIT_10
#define ROOM_SAFE        BIT_11
#define ROOM_SOLITARY    BIT_12
#define ROOM_PET_SHOP    BIT_13
#define ROOM_NO_RECALL   BIT_14
#define ROOM_NO_TELEPORT BIT_15
#define ROOM_HUNT_MARK   BIT_16
#define ROOM_NO_CHARM    BIT_17
#define ROOM_NO_PORTAL   BIT_18
#define ROOM_NO_REPOP    BIT_19
#define ROOM_NO_QUIT     BIT_20
#define ROOM_ANTI_PORTAL BIT_21
#define ROOM_ARENA       BIT_22
#define ROOM_ARENA2      BIT_23

/*
 * Directions.
 * Used in #ROOMS.
 */
#define DIR_NORTH 0
#define DIR_EAST  1
#define DIR_SOUTH 2
#define DIR_WEST  3
#define DIR_UP    4
#define DIR_DOWN  5

/*
 * Exit flags.
 * Used in #ROOMS.
 */
#define EX_ISDOOR     BIT_1
#define EX_CLOSED     BIT_2
#define EX_LOCKED     BIT_3
#define EX_CLIMB      BIT_4
#define EX_IMMORTAL   BIT_5
#define EX_PICKPROOF  BIT_6
#define EX_SMASHPROOF BIT_7
#define EX_PASSPROOF  BIT_8
#define EX_NODETECT   BIT_9
#define EX_REALMLORD  BIT_10

/*
 * Sector types.
 * Used in #ROOMS.
 */
#define SECT_INSIDE       0
#define SECT_CITY         1
#define SECT_FIELD        2
#define SECT_FOREST       3
#define SECT_HILLS        4
#define SECT_MOUNTAIN     5
#define SECT_WATER_SWIM   6
#define SECT_WATER_NOSWIM 7
#define SECT_RECALL_OK    8
#define SECT_AIR          9
#define SECT_DESERT       10
#define SECT_MAX          11

/*
 * Equpiment wear locations.
 * Used in #RESETS.
 */
#define WEAR_NONE     -1
#define WEAR_LIGHT    0
#define WEAR_FINGER_L 1
#define WEAR_FINGER_R 2
#define WEAR_NECK_1   3
#define WEAR_NECK_2   4
#define WEAR_BODY     5
#define WEAR_HEAD     6
#define WEAR_LEGS     7
#define WEAR_FEET     8
#define WEAR_HANDS    9
#define WEAR_ARMS     10
#define WEAR_SHIELD   11
#define WEAR_ABOUT    12
#define WEAR_WAIST    13
#define WEAR_WRIST_L  14
#define WEAR_WRIST_R  15
#define WEAR_WIELD    16
#define WEAR_HOLD     17
#define WEAR_FACE     18
#define WEAR_EAR      19
#define WEAR_MAGIC    20
#define WEAR_WIELD_2  21
#define WEAR_CLAN     22
#define MAX_WEAR      23

/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (End of this section ... stop here)                   *
 *                                                                         *
 ***************************************************************************/

/*
 * Conditions.
 */
#define COND_DRUNK  0
#define COND_FULL   1
#define COND_THIRST 2

/*
 * Positions.
 */
#define POS_DEAD     0
#define POS_MORTAL   1
#define POS_INCAP    2
#define POS_STUNNED  3
#define POS_SLEEPING 4
#define POS_RESTING  5
#define POS_FIGHTING 6
#define POS_STANDING 7
#define POS_WRITING  8
#define POS_BUILDING 9
#define POS_RIDING   10

/*
 * ACT bits for players.
 */
#define PLR_IS_NPC      BIT_1
#define PLR_BOUGHT_PET  BIT_2
#define PLR_CLAN_LEADER BIT_3
#define PLR_AUTOEXIT    BIT_4
#define PLR_AUTOLOOT    BIT_5
#define PLR_AUTOSAC     BIT_6
#define PLR_BLANK       BIT_7
#define PLR_BRIEF       BIT_8
#define PLR_NO_PRAY     BIT_9
#define PLR_COMBINE     BIT_10
#define PLR_PROMPT      BIT_11
#define PLR_TELNET_GA   BIT_12
#define PLR_HOLYLIGHT   BIT_13
#define PLR_WIZINVIS    BIT_14
#define PLR_BUILDER     BIT_15
#define PLR_SILENCE     BIT_16
#define PLR_NO_EMOTE    BIT_17
#define PLR_COLOUR      BIT_18
#define PLR_NO_TELL     BIT_19
#define PLR_LOG         BIT_20
#define PLR_DENY        BIT_21
#define PLR_FREEZE      BIT_22
#define PLR_THIEF       BIT_23
#define PLR_KILLER      BIT_24
#define PLR_NOSUMMON    BIT_25
#define PLR_NOVISIT     BIT_26
#define PLR_QUESTING    BIT_27
#define PLR_BRIEF2      BIT_28
#define PLR_AUTOASSIST  BIT_29
/* BIT_30 unused */
/* BIT_31 unused */

/* additional config bitvector */
#define PLR_AUTOGOLD    BIT_1
#define PLR_AUTOSPLIT   BIT_2
#define PLR_MASKQP      BIT_3
#define PLR_SHOWBLACK   BIT_4
#define PLR_ANSWERING   BIT_5
#define PLR_SHOWDAMAGE  BIT_6
#define PLR_NOGIVE      BIT_7
#define PLR_SAVECHECK   BIT_8
#define PLR_NORESCUE    BIT_9
#define PLR_PACIFIST    BIT_10
#define PLR_NOBOND      BIT_11
#define PLR_NOFOLLOW    BIT_12
#define PLR_NOOBJFUN    BIT_13


/*
 * Player flags
 */
#define PFLAG_PKOK           BIT_1
#define PFLAG_AFK            BIT_2
/* #define PFLAG_AMBAS          BIT_3 THIS FLAG IS NOT IN USE! CAN SAFELY REUSE */
/* #define PFLAG_VAMP           BIT_4 THIS FLAG IS NOT IN USE! CAN SAFELY REUSE */
#define PFLAG_CLAN_DIPLOMAT  BIT_5
#define PFLAG_CLAN_BOSS      BIT_6
#define PFLAG_CLAN_TREASURER BIT_7
#define PFLAG_CLAN_ARMOURER  BIT_8
#define PFLAG_CLAN_LEADER    BIT_9
/* #define PFLAG_SUPER_COUNCIL  BIT_10 THIS FLAG IS NOT IN USE! CAN SAFELY REUSE */
/* #define PFLAG_WEREWOLF       BIT_11 THIS FLAG IS NOT IN USE! CAN SAFELY REUSE */
/* #define PFLAG_RAGED          BIT_12 THIS FLAG IS NOT IN USE! CAN SAFELY REUSE */
/* #define PFLAG_SHIFTED        BIT_13 THIS FLAG IS NOT IN USE! CAN SAFELY REUSE */
/* #define PFLAG_RULER          BIT_14 THIS FLAG IS NOT IN USE! CAN SAFELY REUSE */
/* #define PFLAG_BLIND_PLAYER   BIT_15 THIS FLAG IS NOT IN USE! CAN SAFELY REUSE */
#define PFLAG_NO_BODY        BIT_16
#define PFLAG_DEBUG          BIT_17
#define PFLAG_SPECIALNAME    BIT_18
#define PFLAG_XAFK           BIT_19  /*Added by Josh with some help from Evan */
#define PFLAG_CLAN_2LEADER   BIT_20  /*Added by Josh */
#define PFLAG_CLAN_DESERTER  BIT_21  /* Josh again, for leaving a clan during war */
#define PFLAG_CLAN_LEAVER    BIT_22  /* just leaving a clan */
#define PFLAG_CLAN_TRUSTED   BIT_23  /* trusted in the clan to see cwhere */
#define PFLAG_SAFE           BIT_24  /* can't be attacked/stolen from */

/*
 * Channel bits.
 */
#define CHANNEL_AUCTION     BIT_1
#define CHANNEL_GOSSIP      BIT_2
#define CHANNEL_MUSIC       BIT_3
#define CHANNEL_IMMTALK     BIT_4
#define CHANNEL_NEWBIE      BIT_5
#define CHANNEL_QUESTION    BIT_6
#define CHANNEL_SHOUT       BIT_7
#define CHANNEL_YELL        BIT_8
#define CHANNEL_FLAME       BIT_9
#define CHANNEL_ZZZ         BIT_10
#define CHANNEL_RACE        BIT_11
#define CHANNEL_CLAN        BIT_12
#define CHANNEL_NOTIFY      BIT_13
#define CHANNEL_INFO        BIT_14
#define CHANNEL_LOG         BIT_15
#define CHANNEL_CREATOR     BIT_16
#define CHANNEL_ALLCLAN     BIT_17
#define CHANNEL_ALLRACE     BIT_18
#define CHANNEL_HERMIT      BIT_19 /* Turns off ALL channels */
#define CHANNEL_BEEP        BIT_20
#define CHANNEL_FAMILY      BIT_21
#define CHANNEL_DIPLOMAT    BIT_22
#define CHANNEL_CRUSADE     BIT_23
#define CHANNEL_REMORTTALK  BIT_24
#define CHANNEL_AUTOCRUSADE BIT_25
#define CHANNEL_ADEPT       BIT_26
#define CHANNEL_OOC         BIT_27
#define CHANNEL_QUEST       BIT_28
#define CHANNEL_AVATAR      BIT_29
#define CHANNEL_TRIVIA      BIT_30
#define CHANNEL_PKOK        BIT_31

/* Additional channel bitvector */
#define CHANNEL2_CHALLENGE  BIT_1
#define CHANNEL2_GAIN       BIT_2
#define CHANNEL2_ALLY       BIT_3
#define CHANNEL2_ARENA      BIT_4

/* Monitor channels - for imms to select what mud-based info they receive */
#define MONITOR_CONNECT     BIT_1
#define MONITOR_AREA_UPDATE BIT_2
#define MONITOR_AREA_BUGS   BIT_3
#define MONITOR_AREA_SAVING BIT_4
#define MONITOR_GEN_IMM     BIT_5
#define MONITOR_GEN_MORT    BIT_6
#define MONITOR_COMBAT      BIT_7
#define MONITOR_HUNTING     BIT_8
#define MONITOR_BUILD       BIT_9
#define MONITOR_CLAN        BIT_10
#define MONITOR_OBJ         BIT_11
#define MONITOR_MOB         BIT_12
#define MONITOR_ROOM        BIT_13
#define MONITOR_MAGIC       BIT_14
#define MONITOR_BAD         BIT_15
#define MONITOR_RAWCOL      BIT_16

/*
 * Hunt flags for mobs
 */
#define HUNT_WORLD      0x00000001 /* Search the whole world       */
#define HUNT_OPENDOOR   0x00000002 /* Can open obstructung doors   */
#define HUNT_UNLOCKDOOR 0x00000004 /* Can unlock obstructing doors */
#define HUNT_PICKDOOR   0x00000008 /* Can pick obstructing doors   */
#define HUNT_INFORM     0x00000010 /* Yells while hunting          */
#define HUNT_CR         0x00000020 /* Is preforming a CR           */
#define HUNT_MERC       0x00000040 /* Is gonna assassinate someone */
#define HUNT_ALL        0x0000001E /* can hunt through anything    */

/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 * Some of these variables are probably unused or not needed
 */
struct mob_index_data
{
    bool           is_free;
    MOB_INDEX_DATA *next;
    SPEC_FUN       *spec_fun;
    SHOP_DATA      *pShop;
    AREA_DATA      *area;
    char           *player_name;
    char           *short_descr;
    char           *long_descr;
    char           *description;
    sh_int         vnum;
    sh_int         count;
    sh_int         killed;
    sh_int         sex;
    sh_int         level;
    int            act;
    int            config;
    int            affected_by;
    int            aggro_list;
    sh_int         alignment;
    int            ac_mod;        /* ac modifier */
    int            hr_mod;        /* hitroll modifier */
    int            dr_mod;        /* damroll modifier */
    int            hp_mod;        /* hp modifier */
    int            mana_mod;      /* mana modifier */
    int            custom_xp;
    sh_int         custom_minlev;
    sh_int         custom_maxlev;
    char           *target;       /* last ch to attack */
    MPROG_DATA     *first_mprog;  /* used by MOBprogram */
    MPROG_DATA     *last_mprog;
    int            progtypes;     /* Used by MOBprogram */
    int            skills;        /* skill flags */
    int            cast;          /* casting flags */
    int            def;           /* casting flags */
    sh_int         class;
    sh_int         clan;
    sh_int         race;
    sh_int         position;
    int            hunt_flags;
    char           *path;
};

/*
 * One character (PC or NPC).
 */
struct char_data
{
    bool            is_free;
    CHAR_DATA       *next;
    CHAR_DATA       *prev;
    CHAR_DATA       *next_player;
    CHAR_DATA       *prev_player;
    CHAR_DATA       *next_in_room;
    CHAR_DATA       *prev_in_room;

    CHAR_DATA       *master;
    CHAR_DATA       *leader;
    CHAR_DATA       *fighting;
    CHAR_DATA       *reply;
    CHAR_DATA       *ireply;
    CHAR_DATA       *hunting;
    OBJ_DATA        *hunt_obj;
    CHAR_DATA       *hunt_for;
    ROOM_INDEX_DATA *hunt_home;
    char            *searching;
    int             hunt_flags;
    bool            is_quitting;
    SPEC_FUN        *spec_fun;
    MOB_INDEX_DATA  *pIndexData;
    DESCRIPTOR_DATA *desc;
    AFFECT_DATA     *first_affect;
    AFFECT_DATA     *last_affect;
    AFFECT_DATA     *first_saved_aff;
    AFFECT_DATA     *last_saved_aff;
    NOTE_DATA       *pnote;
    OBJ_DATA        *first_carry;
    OBJ_DATA        *last_carry;
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *was_in_room;
    ROOM_INDEX_DATA *last_room;
    PC_DATA         *pcdata;
    char            *name;
    OBJ_DATA        *sitting;
    char            *short_descr;
    char            *short_descr_orig;
    char            *long_descr;
    char            *long_descr_orig;
    char            *description;
    char            *afk_msg;
    ANSWERING_DATA  *first_message;
    ANSWERING_DATA  *last_message;
    int             num_messages;
    sh_int          sex;
    sh_int          login_sex;
    sh_int          class;
    sh_int          clan;
    sh_int          race;
    sh_int          level;
    int             lvl[MAX_CLASS];
    int             lvl2[MAX_CLASS];
    int             adept_level;
    int             sentence;
    sh_int          invis;
    sh_int          trust;
    bool            wizbit;
    int             played;
    time_t          logon;
    time_t          save_time;
    sh_int          timer;
    sh_int          wait;
    int             hit;
    int             max_hit;
    int             mana;
    int             max_mana;
    int             move;
    int             max_move;
    int             energy;
    int             max_energy;
    int             gold;
    int             balance;
    int             exp;
    int             act;
    int             config; /* more config bits, since act is full. */
    int             act_build;
    int             build_vnum;
    int             affected_by;
    sh_int          position;
    sh_int          practice;
    sh_int          carry_weight;
    sh_int          carry_number;
    sh_int          saving_throw;
    sh_int          alignment;
    int             hitroll;
    int             damroll;
    int             armor;
    int             ac_mod;
    int             stance_ac_mod;
    int             hr_mod;
    int             stance_hr_mod;
    int             dr_mod;
    int             stance_dr_mod;
    char            *target;
    int             wimpy;
    int             deaf;
    int             deaf2;
    MPROG_ACT_LIST  *first_mpact;
    MPROG_ACT_LIST  *last_mpact;
    int             mpactnum;
    int             skills;
    int             cast;
    int             def;
    CHAR_DATA       *riding;
    CHAR_DATA       *rider;
    int             quest_points;
    SHIELD_DATA     *first_shield;
    SHIELD_DATA     *last_shield;
    int             stunTimer;
    sh_int          num_followers;
    sh_int          extract_timer;
    BRAND_DATA      *current_brand;
    int             stance;
    bool            using_named_door;
    int             tele_timer;
    int             path_steps;

    int             energy_wait;
    int             energy_wait_count;
};

/*
 * Data which only PC's have.
 */

struct pc_data
{
    bool            is_free;
    PC_DATA         *next;
    PC_DATA         *prev;

    int             colour[MAX_COLOUR];
    char            *pwd;
    char            *bamfin;
    char            *room_enter;
    char            *room_exit;
    char            *bamfout;
    char            *title;
    char            *immskll;
    char            *host;
    char            *ip;
    sh_int          failures;
    sh_int          clan;
    time_t          desert_time;
    time_t          accept_time;
    int             accept_clan;
    char            *accept_name;
    int             generation;
    sh_int          perm_str;
    sh_int          perm_int;
    sh_int          perm_wis;
    sh_int          perm_dex;
    sh_int          perm_con;
    sh_int          max_str;
    sh_int          max_int;
    sh_int          max_wis;
    sh_int          max_dex;
    sh_int          max_con;
    sh_int          mod_str;
    sh_int          mod_int;
    sh_int          mod_wis;
    sh_int          mod_dex;
    sh_int          mod_con;
    sh_int          condition[3];
    sh_int          pagelen;
    sh_int          learned[MAX_SKILL];
    char            *header;
    char            *message;
    char            *alias_name[MAX_ALIASES];
    char            *alias[MAX_ALIASES];
    char            *who_name;
    int             pkills;
    int             unpkills;
    int             pkilled;
    int             mkills;
    int             mkilled;
    int             circles_attempted; /* Added by -Ogma- */
    int             circles_landed;
    int             pflags;
    time_t          lastlogint;
    sh_int          order[MAX_CLASS];
    sh_int          index[MAX_CLASS];
    int             monitor;
    sh_int          quest_points;
    IGNORE_DATA     *first_ignore;
    IGNORE_DATA     *last_ignore;
    int             num_ignores;
    sh_int          recall_vnum;
    sh_int          keep_vnum; /* keep vnum for players */
    sh_int          kdon_vnum; /* keep donation vnum for players */
    int             mana_from_gain;
    int             hp_from_gain;
    int             move_from_gain;
    char            *load_msg;
    char            hicol;
    char            dimcol;
    char            *pedit_string[5];
    char            *origname; /* specialname addition */
    sh_int          runes; /* number of active rune:warnings (possibly other runes later on) */
    char            *prompt;
    char            *battleprompt;
    char            *noteprompt;
    CHAR_DATA       *trading_with;
    bool            trading_accepts;
    OBJ_DATA        *trading_objs[MAX_TRADEITEMS];
    ROOM_INDEX_DATA *trading_room;
    int             trading_gold;
    time_t          safetimer;
    int             fighttimer;

#define MAX_COOKIES 2
#define MAX_COOKIE_LENGTH (5 + 1)
#define COOKIE_AUCTION 0
#define COOKIE_TRAIN   1
    char            cookies[MAX_COOKIES][MAX_COOKIE_LENGTH];
    time_t          cookiesexpire[MAX_COOKIES];

    RENAME_DATA     *rename;
    bool            in_arena;
    int             arena_save_hp;
    int             arena_save_mana;
    int             arena_save_move;
    int             arena_save_energy;
    AFFECT_DATA     *arena_save_first_affect;
    AFFECT_DATA     *arena_save_last_affect;
    ROOM_INDEX_DATA *arena_save_room;

    bool            deimm; /* for immortals to pretend to be mortal for one command */
    char            *autostance;
    time_t          news_last_read;
    bool            avatar;
    int             stealth;
    int             energy_level;
    int             energy_used;
    time_t          idlecheck;
};

/*
 * MOBprogram block
*/
struct mob_prog_act_list
{
    bool           is_free;
    MPROG_ACT_LIST *next;
    MPROG_ACT_LIST *prev;
    char           *buf;
    CHAR_DATA      *mob;
    CHAR_DATA      *ch;
    OBJ_DATA       *obj;
    void           *vo;
};

struct mob_prog_data
{
    bool       is_free;
    MPROG_DATA *next;
    MPROG_DATA *prev;
    int        type;
    char       *arglist;
    char       *comlist;
    char       *filename;
};

extern bool MOBtrigger;
extern bool nosave;
extern int writeerrno;

/* Mob program bitvector */
#define ERROR_PROG     -1
#define IN_FILE_PROG   BIT_0
#define ACT_PROG       BIT_1
#define SPEECH_PROG    BIT_2
#define RAND_PROG      BIT_3
#define FIGHT_PROG     BIT_4
#define DEATH_PROG     BIT_5
#define HITPRCNT_PROG  BIT_6
#define ENTRY_PROG     BIT_7
#define GREET_PROG     BIT_8
#define ALL_GREET_PROG BIT_9
#define GIVE_PROG      BIT_10
#define BRIBE_PROG     BIT_11

#define LIQ_WATER       0
#define LIQ_MAX         18

struct  liq_type
{
    char   *liq_name;
    char   *liq_color;
    sh_int liq_affect[3];
};

/*
 * Extra description data for a room or object.
 */
struct extra_descr_data
{
    bool             is_free;
    EXTRA_DESCR_DATA *next;
    EXTRA_DESCR_DATA *prev;

    char             *keyword;
    char             *description;
};

struct trigger_data
{
    bool         is_free;
    TRIGGER_DATA *next;
    TRIGGER_DATA *prev;

    char         *message;      /* properly formatted act format string to use in a TO_ROOM */
    int          trigger;       /* command used on object */
    int          event;         /* trigger function index to be called */
    int          data;          /* data used in the event call..vnum, spell index, etc. */
    int          register_data; /* for storing generic info */
    int          on_value;      /* for conditional triggers..happens when register is higher than on_value */
    int         at_vnum;        /* for at another room triggers. event will happen there */
    bool        force_message;  /* Always does an act message, in addition to trigger */
    char        *spell_name;
};

struct obj_index_data
{
    bool             is_free;
    OBJ_INDEX_DATA   *next;

    EXTRA_DESCR_DATA *first_exdesc;
    EXTRA_DESCR_DATA *last_exdesc;
    OBJ_FUN          *obj_fun;
    AFFECT_DATA      *first_apply;
    AFFECT_DATA      *last_apply;
    AREA_DATA        *area;
    char             *owner;
    char             *name;
    sh_int           level;
    char             *short_descr;
    char             *description;
    sh_int           vnum;
    int              item_type;
    int              extra_flags;
    int              wear_flags;
    int              item_apply;
    sh_int           count;
    sh_int           weight;
    int              cost;
    bool             newcost;
    sh_int           rarity;
    int              value[4];
    TRIGGER_DATA     *first_trigger;
    TRIGGER_DATA     *last_trigger;
};

/*
 * One object.
 */
struct obj_data
{
    bool             is_free;
    OBJ_DATA         *next;
    OBJ_DATA         *prev;

    OBJ_DATA         *next_in_carry_list; /* carry list is the list on a char, or in a container */
    OBJ_DATA         *prev_in_carry_list;
    OBJ_DATA         *first_in_carry_list;
    OBJ_DATA         *last_in_carry_list;
    OBJ_DATA         *next_in_room;
    OBJ_DATA         *prev_in_room;
    OBJ_DATA         *first_in_room;
    OBJ_DATA         *last_in_room;
    OBJ_DATA         *first_content;
    OBJ_DATA         *last_content;
    OBJ_DATA         *next_content;
    OBJ_DATA         *prev_content;
    OBJ_DATA         *in_obj;
    OBJ_FUN          *obj_fun;
    CHAR_DATA        *carried_by;
    EXTRA_DESCR_DATA *first_exdesc;
    EXTRA_DESCR_DATA *last_exdesc;
    AFFECT_DATA      *first_apply;
    AFFECT_DATA      *last_apply;
    OBJ_INDEX_DATA   *pIndexData;
    ROOM_INDEX_DATA  *in_room;
    char             *owner;
    char             *name;
    char             *short_descr;
    char             *description;
    int              item_type;
    int              extra_flags;
    int              wear_flags;
    int              item_apply;
    int              wear_loc;
    sh_int           weight;
    int              cost;
    bool             newcost; /* alternative cost for objects if needed */
    sh_int           level;
    sh_int           timer;
    int              value[4];
/*    sh_int           condition;   % value for condition, unused */
    unsigned int     id;
};

/*
 * Exit data.
 */
struct exit_data
{
    bool            is_free;
    EXIT_DATA       *next;

    ROOM_INDEX_DATA *to_room;
    sh_int          vnum;
    sh_int          exit_info;
    sh_int          key;
    char            *keyword;
    char            *description;
};

/*
 * Reset commands:
 *   '*': comment
 *   'M': read a mobile
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'D': set state of door
 *   'R': randomize room exits
 *   'C': put object in corpse when mob dies
 *   'S': stop (end of list)
 */

/*
 * Area-reset definition.
 */
struct reset_data
{
    bool       is_free;
    RESET_DATA *next;
    RESET_DATA *prev;
    char       command;
    sh_int     ifflag;
    sh_int     arg1;
    sh_int     arg2;
    sh_int     arg3;
    char       *notes;
    char       *auto_message;
};

#define AREA_PAYAREA     BIT_1
#define AREA_TELEPORT    BIT_2
#define AREA_BUILDING    BIT_3
#define AREA_NOSHOW      BIT_4
#define AREA_NO_ROOM_AFF BIT_5
#define AREA_BUILDVISIT  BIT_6
#define AREA_NOCHARM     BIT_7
#define AREA_NOENERGY    BIT_8

/*
 *   Npc Interaction stuff  Zen
 *   ... what? undefining all this!
 */

#if 0
struct queued_interact_list
{
    bool                 is_free;
    QUEUED_INTERACT_LIST *next;
    QUEUED_INTERACT_LIST *prev;

    CHAR_DATA            *mob;
};

struct influence_list
{
    bool           is_free;
    INFLUENCE_LIST *next;
    INFLUENCE_LIST *prev;

    INFLUENCE_DATA *this_one;
};

struct control_list
{
    bool         is_free;
    CONTROL_LIST *next;
    CONTROL_LIST *prev;

    CONTROL_DATA *this_one;
};

struct control_data
{
    bool           is_free;
    INFLUENCE_LIST *first_influence; /* what current influences are for the area */
    INFLUENCE_LIST *last_influence;

    INTERACT_DATA  *first_interact;  /* tells the mobs what to do */
    INTERACT_DATA  *last_interact;
    char           *keyword;
    AREA_DATA      *area;
};

struct influence_data
{
    bool       is_free;
    int        influence;
};

struct interact_data
{
    bool          is_free;
    INTERACT_DATA *next;
    INTERACT_DATA *prev;

    int           type;
    int           min_value;
    int           max_value;
    int           action;
    char          *say;
    CHAR_DATA     *target;
};
#endif /* what the hell is this crap? */

struct ruler_data
{
    bool        is_free;
    RULER_DATA *next;
    RULER_DATA *prev;

    char       *name;
    char       *whoname;
    char       *rank;
    int         clan;
    time_t      realmtime;
    bool        hide;
};

struct area_data
{
    bool            is_free;
    AREA_DATA       *next;
    AREA_DATA       *prev;
    RESET_DATA      *first_reset;
    RESET_DATA      *last_reset;
    char            *name;
    int             avnum;
    sh_int          age;
    sh_int          nplayer;
    int             offset;
    int             modified;
    int             min_vnum;
    int             max_vnum;
    int             area_num;
    char            *owner;
    char            *can_read;
    char            *can_write;
    int             gold;
    char            *filename;
    int             flags;
    int             aggro_list;
    BUILD_DATA_LIST *first_area_room;
    BUILD_DATA_LIST *last_area_room;
    BUILD_DATA_LIST *first_area_object;
    BUILD_DATA_LIST *last_area_object;
    BUILD_DATA_LIST *first_area_mobile;
    BUILD_DATA_LIST *last_area_mobile;
    BUILD_DATA_LIST *first_area_mobprog;
    BUILD_DATA_LIST *last_area_mobprog;
    BUILD_DATA_LIST *first_area_shop;
    BUILD_DATA_LIST *last_area_shop;
    CONTROL_DATA    *control;
    char            *keyword;
    sh_int          min_level;
    sh_int          max_level;
    char            *level_label;
    sh_int          reset_rate;
    char            *reset_msg;
    char            *nocmd;
    char            *nospell;
};

/*
 * Room type.
 */
struct room_index_data
{
    bool             is_free;
    ROOM_INDEX_DATA  *next;

    CHAR_DATA        *first_person;
    CHAR_DATA        *last_person;
    OBJ_DATA         *first_content;
    OBJ_DATA         *last_content;
    EXTRA_DESCR_DATA *first_exdesc;
    EXTRA_DESCR_DATA *last_exdesc;
    AREA_DATA        *area;
    EXIT_DATA        *exit[6];
    char             *name;
    char             *description;
    char             *auto_message;
    sh_int           block_timer;
    sh_int           vnum;
    int              room_flags;
    sh_int           light;
    sh_int           sector_type;
    char             *nocmd;
    char             *nospell;
    BUILD_DATA_LIST  *first_room_reset;
    BUILD_DATA_LIST  *last_room_reset;
    ROOM_AFFECT_DATA *first_room_affect;
    ROOM_AFFECT_DATA *last_room_affect;
    int              affected_by;
    MARK_LIST_MEMBER *first_mark_list;
    MARK_LIST_MEMBER *last_mark_list;
    TELEPORT_DATA    *tele;
    LOCKER_DATA      *locker;
};

struct build_data_list /* used for storing area file data */
{
    bool            is_free;
    BUILD_DATA_LIST *next;
    BUILD_DATA_LIST *prev;

    void            *data;
};

struct mobprog_item /* for re-creating #MOBPROGS section */
{
    MOB_INDEX_DATA *mob;
    char           *filename;
};

struct lookup_type
{
    char              *text;
    unsigned long int value;
    int               level;  /* immortal level that can use this value. */
};

/*
 * Types of attacks.
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that.
 */

#define TYPE_CRACK     -4 /* cracks are ignored by the damage limiter */
#define TYPE_MARTIAL   -3 /* for martial arts */
#define TYPE_UNDEFINED -2
#define TYPE_IGNORE    -1
#define TYPE_HIT       1000

/*
 *  Target types.
 */
#define TAR_IGNORE         0
#define TAR_CHAR_OFFENSIVE 1
#define TAR_CHAR_DEFENSIVE 2
#define TAR_CHAR_SELF      3
#define TAR_OBJ_INV        4

/* Used as flags in skill_table */
#define MORTAL 1
#define REMORT 2
#define ADEPT  3
#define AVATAR 4

#define NORM   1
#define SUB    2

/*
 * Skills include spells as a particular case.
 */
struct skill_type
{
    sh_int    flag1;                  /* mort or remort?              */
    sh_int    flag2;                  /* TODO: this is no longer necessary, remove it */
    char      *name;                  /* Name of skill                */
    sh_int    skill_level[MAX_CLASS]; /* Level needed by class        */
    SPELL_FUN *spell_fun;             /* Spell pointer (for spells)   */
    sh_int    target;                 /* Legal targets                */
    sh_int    minimum_position;       /* Position for caster / user   */
    sh_int    *pgsn;                  /* Pointer to associated gsn    */
    sh_int    slot;                   /* Slot for #OBJECT loading     */
    int       min_mana;               /* Minimum mana used            */
    sh_int    beats;                  /* Waiting time after use       */
    char      *noun_damage;           /* Damage message               */
    char      *msg_off;               /* Wear off message             */
    char      *room_off;              /* Wear off msg TO_ROOM         */
};

/*
 * These are skill_lookup return values for common skills and spells. They're
 * calculated on startup which makes them useful to reference directly instead
 * of using skill_lookup which loops through every skill, which could be costly
 * in a critical section.
 */

extern sh_int gsn_adrenaline;
extern sh_int gsn_appraise;
extern sh_int gsn_backstab;
extern sh_int gsn_bash;
extern sh_int gsn_berserk;
extern sh_int gsn_bite;
extern sh_int gsn_black_hand;
extern sh_int gsn_blindness;
extern sh_int gsn_change_sex;
extern sh_int gsn_charge;
extern sh_int gsn_charm_person;
extern sh_int gsn_circle;
extern sh_int gsn_climb;
extern sh_int gsn_cloak_absorb;
extern sh_int gsn_cloak_adept;
extern sh_int gsn_cloak_flaming;
extern sh_int gsn_cloak_mana;
extern sh_int gsn_cloak_reflect;
extern sh_int gsn_cloak_regen;
extern sh_int gsn_cure_critical;
extern sh_int gsn_cure_light;
extern sh_int gsn_cure_serious;
extern sh_int gsn_curse;
extern sh_int gsn_decapitate;
extern sh_int gsn_detect_magic;
extern sh_int gsn_dirt;
extern sh_int gsn_disarm;
extern sh_int gsn_disguise;
extern sh_int gsn_divine_intervention;
extern sh_int gsn_dodge;
extern sh_int gsn_dualwield;
extern sh_int gsn_emotion_control;
extern sh_int gsn_emount;
extern sh_int gsn_enhanced_damage;
extern sh_int gsn_feed;
extern sh_int gsn_fifth_attack;
extern sh_int gsn_find_doors;
extern sh_int gsn_fly;
extern sh_int gsn_fourth_attack;
extern sh_int gsn_frenzy;
extern sh_int gsn_grab;
extern sh_int gsn_harm;
extern sh_int gsn_headbutt;
extern sh_int gsn_heal;
extern sh_int gsn_hide;
extern sh_int gsn_hunt;
extern sh_int gsn_hypnosis;
extern sh_int gsn_imprint;
extern sh_int gsn_instruct;
extern sh_int gsn_invis;
extern sh_int gsn_kick;
extern sh_int gsn_knee;
extern sh_int gsn_lsd;
extern sh_int gsn_martial_arts;
extern sh_int gsn_mass_invis;
extern sh_int gsn_mount;
extern sh_int gsn_nodisarm;
extern sh_int gsn_notrip;
extern sh_int gsn_parry;
extern sh_int gsn_peek;
extern sh_int gsn_pick_lock;
extern sh_int gsn_poison;
extern sh_int gsn_punch;
extern sh_int gsn_rescue;
extern sh_int gsn_scent;
extern sh_int gsn_scout;
extern sh_int gsn_second_attack;
extern sh_int gsn_shadow;
extern sh_int gsn_shield_block;
extern sh_int gsn_shield_demon;
extern sh_int gsn_shield_fire;
extern sh_int gsn_shield_ice;
extern sh_int gsn_shield_shock;
extern sh_int gsn_sixth_attack;
extern sh_int gsn_sleep;
extern sh_int gsn_smash;
extern sh_int gsn_sneak;
extern sh_int gsn_stake;
extern sh_int gsn_steal;
extern sh_int gsn_stun;
extern sh_int gsn_target;
extern sh_int gsn_teach;
extern sh_int gsn_third_attack;
extern sh_int gsn_trip;
extern sh_int gsn_unit_tactics;
extern sh_int gsn_smokescreen_novice;
extern sh_int gsn_smokescreen_intermediate;
extern sh_int gsn_smokescreen_advanced;
extern sh_int gsn_smokescreen_expert;
extern sh_int gsn_smokescreen_master;
extern sh_int gsn_sentry_novice;
extern sh_int gsn_sentry_intermediate;
extern sh_int gsn_sentry_advanced;
extern sh_int gsn_sentry_expert;
extern sh_int gsn_sentry_master;
extern sh_int gsn_stealth;
extern sh_int gsn_stealth_novice;
extern sh_int gsn_stealth_intermediate;
extern sh_int gsn_stealth_advanced;
extern sh_int gsn_stealth_expert;
extern sh_int gsn_stealth_master;
extern sh_int gsn_nutrition;
extern sh_int gsn_nutrition_novice;
extern sh_int gsn_compassion;
extern sh_int gsn_compassion_novice;
extern sh_int gsn_compassion_intermediate;
extern sh_int gsn_compassion_advanced;
extern sh_int gsn_compassion_expert;
extern sh_int gsn_compassion_master;
extern sh_int gsn_innerflame;
extern sh_int gsn_innerflame_novice;
extern sh_int gsn_innerflame_intermediate;
extern sh_int gsn_innerflame_advanced;
extern sh_int gsn_innerflame_expert;
extern sh_int gsn_innerflame_master;

/*
 * Utility macros.
 */
#define UMIN(a, b)           ((a) < (b) ? (a) : (b))
#define UMAX(a, b)           ((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)      ((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define LOWER(c)             ((c) >= 'A' && (c) <= 'Z' ? (c) + 'a'-'A' : (c))
#define UPPER(c)             ((c) >= 'a' && (c) <= 'z' ? (c) + 'A'-'a' : (c))
#define IS_SET(flag, bit)    ((flag) & (bit))
#define SET_BIT(var, bit)    ((var) |= (bit))
#define REMOVE_BIT(var, bit) ((var) &= ~(bit))

/*
 * Character macros.
 */
#define IS_UNDEAD(ch)        (IS_NPC(ch) ? IS_SET(ch->act, ACT_UNDEAD ) : FALSE)
#define IS_NPC(ch)           (IS_SET((ch)->act, ACT_IS_NPC))
#define IS_IMMORTAL(ch)      (!IS_NPC(ch) && get_trust(ch) >= LEVEL_IMMORTAL && (ch)->pcdata && (ch)->pcdata->deimm == FALSE)
#define IS_HERO(ch)          (get_trust(ch) >= LEVEL_HERO)
#define IS_AFFECTED(ch, sn)  (IS_SET((ch)->affected_by, (sn)))
#define IS_GOOD(ch)          (ch->alignment >= 350)
#define IS_EVIL(ch)          (ch->alignment <= -350)
#define IS_NEUTRAL(ch)       (!IS_GOOD(ch) && !IS_EVIL(ch))
#define IS_AWAKE(ch)         (ch->position > POS_SLEEPING)
#define GET_AC(ch)           (IS_NPC(ch) ? (REAL_AC(ch) + ch->ac_mod) : REAL_AC(ch) + ch->stance_ac_mod)

#define REAL_AC(ch)          ((ch)->armor  + (IS_AWAKE(ch)                                                           \
                                              ? (IS_NPC(ch)                                                          \
                                                ? (dex_app[get_curr_dex(ch)].defensive * get_pseudo_level(ch) / 20)  \
                                                : (dex_app[get_curr_dex(ch)].defensive * get_pseudo_level(ch) / 10)) \
                                              : 0))

#define GET_HITROLL(ch)      (IS_NPC(ch) ? (REAL_HITROLL(ch) + ch->hr_mod + (get_pseudo_level(ch) / 4)) : REAL_HITROLL(ch) + (ch->level / 8) + ch->stance_hr_mod)
#define REAL_HITROLL(ch)     ((ch)->hitroll + (str_app[get_curr_str(ch)].tohit * get_pseudo_level(ch) / 10))
#define GET_DAMROLL(ch)      (IS_NPC(ch) ? (REAL_DAMROLL(ch) + ch->dr_mod + (ch->level / 3)) : REAL_DAMROLL(ch) + (ch->level / 10) + ch->stance_dr_mod)
#define REAL_DAMROLL(ch)     ((ch)->damroll + (str_app[get_curr_str(ch)].todam * get_pseudo_level(ch) / 10))
#define IS_OUTSIDE(ch)       (!IS_SET((ch)->in_room->room_flags, ROOM_INDOORS))
#define WAIT_STATE(ch, np)   ((ch)->wait = UMAX((ch)->wait, (np)))
#define MANA_COST(ch, sn)    (IS_NPC(ch) ? 0 : UMAX(skill_table[sn].min_mana, 100 / (2 + ch->level - skill_table[sn].skill_level[ch->class])))
#define IS_RIDING(ch)        (!IS_NPC(ch) && ((ch)->riding))
#define IS_DEBUGGER(ch)      (IS_NPC(ch) ? FALSE : IS_SET(ch->pcdata->pflags, PFLAG_DEBUG))
#define IS_NGR_CHARMIE(p, c) (    !IS_NPC(p)                       \
                               && (p)->pcdata                      \
                               && IS_NPC(c)                        \
                               && IS_AFFECTED((c), AFF_CHARM)      \
                               && (c)->master                      \
                               && !can_group((p), (c)->master)     \
                               && !p->pcdata->in_arena             \
                             )

/* Find out if a sn is for a cloak */
#define IS_CLOAK_SN(sn) (   sn == gsn_cloak_absorb  \
                         || sn == gsn_cloak_reflect \
                         || sn == gsn_cloak_flaming \
                         || sn == gsn_cloak_mana    \
                         || sn == gsn_cloak_adept   \
                         || sn == gsn_cloak_regen   \
                        )

/* Get an index for the cloak table from a cloak sn */
#define GET_INDEX_CLOAK(sn) (                   \
(sn ==        gsn_cloak_absorb  ? CLOAK_ABSORB  \
 : (sn ==     gsn_cloak_reflect ? CLOAK_REFLECT \
  : (sn ==    gsn_cloak_flaming ? CLOAK_FLAMING \
   : (sn ==   gsn_cloak_mana    ? CLOAK_MANA    \
    : (sn ==  gsn_cloak_adept   ? CLOAK_ADEPT   \
     : (sn == gsn_cloak_regen   ? CLOAK_REGEN   \
      :                           CLOAK_NONE    \
       )                                        \
      )                                         \
     )                                          \
    )                                           \
   )                                            \
))

/*
 * Object macros.
 */
#define CAN_WEAR(obj, part)    (IS_SET((obj)->wear_flags,  (part)))
#define IS_OBJ_STAT(obj, stat) (IS_SET((obj)->extra_flags, (stat)))

/*
 * Description macros.
 */
#define PERS(ch, looker) (ch->is_free == FALSE && can_see(looker, (ch)) ? (ch)->short_descr : (ch->is_free == FALSE && IS_IMMORTAL(ch) ? "A Mystical Being" : "Someone"))
#define NAME(ch)         ((ch)->short_descr)

/* Added stuff - Flar */
#define CH(descriptor)  ((descriptor)->original ? (descriptor)->original : (descriptor)->character)

/*
 * Linked list macros, -- Altrag
 */

/* Regular linking of double-linked list */
#define LINK(link, first, last, next, prev)                                \
do {                                                                       \
    if ((link)->is_free == TRUE)      hang("LINK: link is FREE!");         \
    if ((link)->is_free != FALSE)     hang("LINK: link is corrupted!");    \
    if ((link)->prev || (link)->next) hang("LINK: link already in list?"); \
    if ((last) && (last)->next)       hang("LINK: last->next NON-NULL!");  \
                                                                           \
    if (!(first))                                                          \
        (first) = (link);                                                  \
    else                                                                   \
        (last)->next = (link);                                             \
                                                                           \
    (link)->next = NULL;                                                   \
    (link)->prev = (last);                                                 \
    (last) = (link);                                                       \
} while(0)

/* Link at the head of the list rather than the tail.  Double linked */
#define TOPLINK(link, first, last, next, prev)                                \
do {                                                                          \
    if ((link)->is_free == TRUE)      hang("TOPLINK: link is FREE!");         \
    if ((link)->is_free != FALSE)     hang("TOPLINK: link is corrupted!");    \
    if ((link)->prev || (link)->next) hang("TOPLINK: link already in list?"); \
    if ((first) && (first)->prev)     hang("TOPLINK: first->prev NON-NULL!"); \
                                                                              \
    if (!(last))                                                              \
        (last) = (link);                                                      \
    else                                                                      \
        (first)->prev = (link);                                               \
                                                                              \
    (link)->prev = NULL;                                                      \
    (link)->next = (first);                                                   \
    (first) = (link);                                                         \
} while(0)

/* Insert link before ref link */
#define LINK_BEFORE(link, ref, first, last, next, prev)                           \
do {                                                                              \
    if ((link)->is_free == TRUE)      hang("LINK_BEFORE: link is FREE!");         \
    if ((link)->is_free != FALSE)     hang("LINK_BEFORE: link is corrupted!");    \
    if ((link)->prev || (link)->next) hang("LINK_BEFORE: link already in list?"); \
    if (!(ref))                       hang("LINK_BEFORE: ref is NULL!");          \
    if ((ref)->is_free == TRUE)       hang("LINK_BEFORE: ref is FREE!");          \
    if ((ref)->is_free != FALSE)      hang("LINK_BEFORE: ref is corrupted!");     \
                                                                                  \
    (link)->next = (ref);                                                         \
    (link)->prev = (ref)->prev;                                                   \
                                                                                  \
    if (!(ref)->prev)                                                             \
        (first) = (link);                                                         \
    else                                                                          \
        ((ref)->prev)->next = (link);                                             \
                                                                                  \
    (ref)->prev = (link);                                                         \
} while (0)

/* Insert link after ref link */
#define LINK_AFTER(link, ref, first, last, next, prev)                           \
do {                                                                             \
    if ((link)->is_free == TRUE)      hang("LINK_AFTER: link is FREE!");         \
    if ((link)->is_free != FALSE)     hang("LINK_AFTER: link is corrupted!");    \
    if ((link)->prev || (link)->next) hang("LINK_AFTER: link already in list?"); \
    if (!(ref))                       hang("LINK_AFTER: ref is NULL!");          \
    if ((ref)->is_free == TRUE)       hang("LINK_AFTER: ref is FREE!");          \
    if ((ref)->is_free != FALSE)      hang("LINK_AFTER: ref is corrupted!");     \
                                                                                 \
    (link)->prev = (ref);                                                        \
    (link)->next = (ref)->next;                                                  \
                                                                                 \
    if (!(ref)->next)                                                            \
        (last) = (link);                                                         \
    else                                                                         \
        ((ref)->next)->prev = (link);                                            \
                                                                                 \
    (ref)->next = (link);                                                        \
} while (0)

/* Unlink a double linked list */
#define UNLINK(link, first, last, next, prev)                                                          \
do {                                                                                                   \
    if ((link)->is_free == TRUE)                          hang("UNLINK: link is FREE!");               \
    if ((link)->is_free != FALSE)                         hang("UNLINK: link is corrupted!");          \
    if ((link)->prev && (((link)->prev)->next != (link))) hang("UNLINK: link->prev->next corrupted!"); \
    if ((link)->next && (((link)->next)->prev != (link))) hang("UNLINK: link->next->prev corrupted!"); \
                                                                                                       \
    if (!(link)->next)                                                                                 \
        (last) = (link)->prev;                                                                         \
    else                                                                                               \
        (link)->next->prev = (link)->prev;                                                             \
                                                                                                       \
    if (!(link)->prev)                                                                                 \
        (first) = (link)->next;                                                                        \
    else                                                                                               \
        (link)->prev->next = (link)->next;                                                             \
                                                                                                       \
    (link)->prev = NULL;                                                                               \
    (link)->next = NULL;                                                                               \
} while(0)

/* Link to the end of a single-linked list */
#define SING_LINK(link, first, next, DATA_TYPE)             \
do {                                                        \
    if (!(first))                                           \
        (first) = (link);                                   \
    else {                                                  \
        DATA_TYPE *last;                                    \
        for (last = (first); last->next; last = last->next) \
            ;                                               \
        last->next = (link);                                \
    }                                                       \
                                                            \
    (link)->next = NULL;                                    \
} while(0)

/* Link to head of a single-linked list (normal linking) */
#define SING_TOPLINK(link, first, next) \
do {                                    \
    (link)->next = (first);             \
    (first) = (link);                   \
} while(0)

/* Unlink a single linked list */
#define SING_UNLINK(link, first, next, DATA_TYPE)     \
do {                                                  \
    if ((link) == (first))                            \
        (first) = (link)->next;                       \
    else {                                            \
        DATA_TYPE *prev;                              \
                                                      \
        for (prev = (first); prev; prev = prev->next) \
            if (prev->next == (link))                 \
                break;                                \
                                                      \
        if (prev != NULL)                             \
            prev->next = (link)->next;                \
        else                                          \
            hang("SING_UNLINK: link not in list!");   \
    }                                                 \
} while(0)

/* Half linked lists have a LAST pointer, but not a PREV pointer, making
   them approximately halfway between a single linked list and a double
   linked list. -- Altrag */

/* Link to end of a half-linked list */
#define HALF_LINK(link, first, last, next) \
do {                                       \
    if (!(last))                           \
        (first) = (link);                  \
    else                                   \
        (last)->next = (link);             \
                                           \
    (link)->next = NULL;                   \
    (last) = (link);                       \
} while(0)

/* Link to head of a half-linked list. */
#define HALF_TOPLINK(link, first, last, next) \
do {                                          \
    if ((last) == (first))                    \
        (last) = (link);                      \
                                              \
    (link)->next = (first);                   \
    (first) = (link);                         \
} while(0)

/* Unlink a half-linked list. */
#define HALF_UNLINK(link, first, last, next, DATA_TYPE) \
do {                                                    \
    if ((link) == (first)) {                            \
      (first) = (link)->next;                           \
                                                        \
      if ((link) == (last))                             \
          (last) = NULL;                                \
    }                                                   \
    else {                                              \
        DATA_TYPE *prev;                                \
                                                        \
        for (prev = (first); prev; prev = prev->next)   \
           if (prev->next == (link))                    \
               break;                                   \
                                                        \
        if (prev != NULL) {                             \
            prev->next = (link)->next;                  \
                                                        \
            if ((link) == (last))                       \
                (last) = prev;                          \
        }                                               \
        else                                            \
            hang("HALF_UNLINK: link not in list!");     \
    }                                                   \
} while(0)

/*
 * Miscellaneous macros.
 */

/* spec: macro-ised getmem as a wrapper around _getmem for mem_log handling */
#define getmem(size)  _getmem(size, _caller, 1)
#define qgetmem(size) _getmem(size, _caller, 0)

#define _dispose(mem, log)                                                  \
do {                                                                        \
    if (!(mem))                                                             \
        bug("Disposing NULL memory");                                       \
                                                                            \
    if (log && mem_log)                                                     \
        xlogf("dispose(%p) from %s:%d", (void *)(mem), __FILE__, __LINE__); \
                                                                            \
    free((mem));                                                            \
    (mem) = NULL;                                                           \
} while(0)

#define dispose(mem)  _dispose(mem, 1)
#define qdispose(mem) _dispose(mem, 0)

/*
 * Structure for a command in the command lookup table.
 */
struct cmd_type
{
    char * const name;
    DO_FUN       *do_fun;
    sh_int       position;
    sh_int       level;
    sh_int       log;
    bool         can_order; /* can this command be used by the 'order' command? */
};

/*
 * Structure for a social in the socials table.
 */
struct social_type
{
    char *name;
    char *char_no_arg;
    char *others_no_arg;
    char *char_found;
    char *others_found;
    char *vict_found;
    char *char_auto;
    char *others_auto;
};

/*
 * Global constants.
 */
extern const struct str_app_type    str_app[26];
extern const struct int_app_type    int_app[26];
extern const struct wis_app_type    wis_app[26];
extern const struct dex_app_type    dex_app[26];
extern const struct con_app_type    con_app[26];
extern const struct colour_type     colour_table[MAX_COLOUR];
extern const struct ansi_type       ansi_table[MAX_ANSI];
extern const struct class_type      class_table[MAX_CLASS];
extern const struct class_type      remort_table[MAX_CLASS];
extern const struct race_type       race_table[MAX_RACE];
extern const struct exp_type        exp_table[141];
extern const struct clan_type       clan_table[MAX_CLAN];
extern const struct cmd_type        cmd_table[];
extern const struct liq_type        liq_table[LIQ_MAX];
extern const struct skill_type      skill_table[MAX_SKILL];
extern const struct stance_app_type stance_app[MAX_STANCE];
extern char * const                 title_table[MAX_CLASS][36][2];
extern       struct social_type     *social_table;
extern       struct mudset_type     mudset_table[];
extern const struct shield_type     shield_table[];
extern const struct shield_type     cloak_table[];
extern const struct avatarlimit_type avatarlimit_table[];

/* spec: log all calls to getmem/dispose when set */
extern bool mem_log;

/*
 * Global variables.
 */
extern time_t          current_time;
extern time_t          today_time;
extern bool            fLogAll;
extern FILE *          fpReserve;
extern KILL_DATA       kill_table[];
extern char            log_buf[];
extern TIME_INFO_DATA  time_info;
extern WEATHER_DATA    weather_info;
extern DESCRIPTOR_DATA *descriptor_list;
extern CHAR_DATA       *char_list;
extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];
extern OBJ_INDEX_DATA  *obj_index_hash[MAX_KEY_HASH];

/* board junk */
extern BOARD_DATA   *first_board;
extern BOARD_DATA   *last_board;
extern BOARD_DATA   *board_free;
extern MESSAGE_DATA *message_free;

/* Various linked lists head/tail pointer declarations. -- Altrag */
#include "lists.h"

/*
 * Command functions.
 * Defined in act_*.c (mostly).
 */

/**************************
 * This bunch of commands *
 * added by Stephen ;)    *
 **************************/

DECLARE_DO_FUN(do_test);
DECLARE_DO_FUN(do_exlist);
DECLARE_DO_FUN(do_vlist);
DECLARE_DO_FUN(do_rename);
DECLARE_DO_FUN(do_sdelete);
DECLARE_DO_FUN(do_accept);
DECLARE_DO_FUN(do_adapt);
DECLARE_DO_FUN(do_affected);
DECLARE_DO_FUN(do_afk);
DECLARE_DO_FUN(do_afk_msg);
DECLARE_DO_FUN(do_immortal);
DECLARE_DO_FUN(do_xlook);
DECLARE_DO_FUN(do_xlookaff);
DECLARE_DO_FUN(do_alias);
DECLARE_DO_FUN(do_ally);
DECLARE_DO_FUN(do_appraise);
DECLARE_DO_FUN(build_arealist);
DECLARE_DO_FUN(do_areasave);
DECLARE_DO_FUN(do_ask);
DECLARE_DO_FUN(do_assist);
DECLARE_DO_FUN(do_atalk);
DECLARE_DO_FUN(do_bamfin);
DECLARE_DO_FUN(do_bamfout);
DECLARE_DO_FUN(do_bank);
DECLARE_DO_FUN(do_banish);
DECLARE_DO_FUN(do_bash);
DECLARE_DO_FUN(do_beep);
DECLARE_DO_FUN(do_berserk);
DECLARE_DO_FUN(do_cleft);
DECLARE_DO_FUN(do_board);
DECLARE_DO_FUN(do_bid);
DECLARE_DO_FUN(do_rbid);
DECLARE_DO_FUN(do_mbid);
DECLARE_DO_FUN(do_cdonate);
DECLARE_DO_FUN(do_circle);
DECLARE_DO_FUN(do_clan);
DECLARE_DO_FUN(do_clan_list);
DECLARE_DO_FUN(do_clan_recall);
DECLARE_DO_FUN(do_clutch);
DECLARE_DO_FUN(do_clutchinfo);
DECLARE_DO_FUN(do_colist);
DECLARE_DO_FUN(do_colour);
DECLARE_DO_FUN(do_creator);
DECLARE_DO_FUN(do_cset);
DECLARE_DO_FUN(do_ctalk);
DECLARE_DO_FUN(do_cwhere);
DECLARE_DO_FUN(do_deathmatc);
DECLARE_DO_FUN(do_diagnose);
DECLARE_DO_FUN(do_dirt);
DECLARE_DO_FUN(display_messages);
DECLARE_DO_FUN(do_dog);
DECLARE_DO_FUN(do_donate);
DECLARE_DO_FUN(do_duel);
DECLARE_DO_FUN(do_edit);
DECLARE_DO_FUN(do_enter);
DECLARE_DO_FUN(do_feed);
DECLARE_DO_FUN(do_fights);
DECLARE_DO_FUN(do_finger);
DECLARE_DO_FUN(do_flame);
DECLARE_DO_FUN(do_gain);
DECLARE_DO_FUN(do_gold);
DECLARE_DO_FUN(do_gossip);
DECLARE_DO_FUN(do_ooc);
DECLARE_DO_FUN(do_espanol);
DECLARE_DO_FUN(do_francais);
DECLARE_DO_FUN(do_quest2);
DECLARE_DO_FUN(do_qinfo);
DECLARE_DO_FUN(do_guild);
DECLARE_DO_FUN(do_halls);
DECLARE_DO_FUN(do_headbutt);
DECLARE_DO_FUN(do_knee);
DECLARE_DO_FUN(do_heal);
DECLARE_DO_FUN(do_hunt);
DECLARE_DO_FUN(do_resetpassword);
DECLARE_DO_FUN(do_irename);
DECLARE_DO_FUN(do_isnoop);
DECLARE_DO_FUN(do_iwhere);
DECLARE_DO_FUN(do_leav);
DECLARE_DO_FUN(do_leave);
DECLARE_DO_FUN(do_lhunt);
DECLARE_DO_FUN(do_maffect);
DECLARE_DO_FUN(do_make);
DECLARE_DO_FUN(do_makeimm);
DECLARE_DO_FUN(do_monitor);
DECLARE_DO_FUN(do_mpcr);
DECLARE_DO_FUN(do_music);
DECLARE_DO_FUN(do_newbie);
DECLARE_DO_FUN(do_news);
DECLARE_DO_FUN(do_nocmd);
DECLARE_DO_FUN(do_nodispel);
DECLARE_DO_FUN(do_nospell);
DECLARE_DO_FUN(do_nopray);
DECLARE_DO_FUN(do_pemote);
DECLARE_DO_FUN(do_pkok);
DECLARE_DO_FUN(do_players);
DECLARE_DO_FUN(do_pray);
DECLARE_DO_FUN(do_punch);
DECLARE_DO_FUN(do_quest);
DECLARE_DO_FUN(do_race);
DECLARE_DO_FUN(do_race_list);
DECLARE_DO_FUN(do_retrieve);
DECLARE_DO_FUN(do_avatar);
DECLARE_DO_FUN(do_rules);
DECLARE_DO_FUN(do_scan);
DECLARE_DO_FUN(do_setclass);
DECLARE_DO_FUN(do_smash);
DECLARE_DO_FUN(do_shadowform);
DECLARE_DO_FUN(do_shedit);
DECLARE_DO_FUN(do_shelp);
DECLARE_DO_FUN(do_sstat);
DECLARE_DO_FUN(do_stake);
DECLARE_DO_FUN(do_status);
DECLARE_DO_FUN(do_stun);
DECLARE_DO_FUN(do_togbuild);
DECLARE_DO_FUN(do_togleader);
DECLARE_DO_FUN(do_tongue);
DECLARE_DO_FUN(do_trip);
DECLARE_DO_FUN(do_whisper);
DECLARE_DO_FUN(do_whois);
DECLARE_DO_FUN(do_whoname);
DECLARE_DO_FUN(do_worth);
DECLARE_DO_FUN(do_zzz);
DECLARE_DO_FUN(do_listspells);
DECLARE_DO_FUN(do_reward);
DECLARE_DO_FUN(do_xpreward);
DECLARE_DO_FUN(do_objrename);
DECLARE_DO_FUN(do_sdelete);
DECLARE_DO_FUN(do_grab);
DECLARE_DO_FUN(build_interpret);
DECLARE_DO_FUN(do_build);
DECLARE_DO_FUN(do_delete);
DECLARE_DO_FUN(do_read);
DECLARE_DO_FUN(do_savearea);
DECLARE_DO_FUN(do_write);
DECLARE_DO_FUN(do_mfindlev);
DECLARE_DO_FUN(do_ofindlev);
DECLARE_DO_FUN(do_advance);
DECLARE_DO_FUN(do_allow);
DECLARE_DO_FUN(do_answer);
DECLARE_DO_FUN(do_areas);
DECLARE_DO_FUN(do_at);
DECLARE_DO_FUN(do_auction);
DECLARE_DO_FUN(do_rauction);
DECLARE_DO_FUN(do_mauction);
DECLARE_DO_FUN(do_answering);
DECLARE_DO_FUN(do_auto);
DECLARE_DO_FUN(do_autoexit);
DECLARE_DO_FUN(do_autoloot);
DECLARE_DO_FUN(do_autosac);
DECLARE_DO_FUN(do_autosplit);
DECLARE_DO_FUN(do_autostance);
DECLARE_DO_FUN(do_autogold);
DECLARE_DO_FUN(do_autoassist);
DECLARE_DO_FUN(do_backstab);
DECLARE_DO_FUN(do_ban);
DECLARE_DO_FUN(do_blank);
DECLARE_DO_FUN(do_bprompt);
DECLARE_DO_FUN(do_brandish);
DECLARE_DO_FUN(do_brief);
DECLARE_DO_FUN(do_brief2);
DECLARE_DO_FUN(do_bug);
DECLARE_DO_FUN(do_buy);
DECLARE_DO_FUN(do_cast);
DECLARE_DO_FUN(do_channels);
DECLARE_DO_FUN(do_charmpurge);
DECLARE_DO_FUN(do_close);
DECLARE_DO_FUN(do_cinfo);
DECLARE_DO_FUN(do_combine);
DECLARE_DO_FUN(do_commands);
DECLARE_DO_FUN(do_compare);
DECLARE_DO_FUN(do_compress);
DECLARE_DO_FUN(do_config);
DECLARE_DO_FUN(do_consider);
DECLARE_DO_FUN(do_credits);
DECLARE_DO_FUN(do_dblxp);
DECLARE_DO_FUN(do_deimm);
DECLARE_DO_FUN(do_deny);
DECLARE_DO_FUN(do_description);
DECLARE_DO_FUN(do_disarm);
DECLARE_DO_FUN(do_disconnect);
DECLARE_DO_FUN(do_dleft);
DECLARE_DO_FUN(do_down);
DECLARE_DO_FUN(do_drink);
DECLARE_DO_FUN(do_drop);
DECLARE_DO_FUN(do_east);
DECLARE_DO_FUN(do_eat);
DECLARE_DO_FUN(do_echo);
DECLARE_DO_FUN(do_emote);
DECLARE_DO_FUN(do_equipment);
DECLARE_DO_FUN(do_eqaffects);
DECLARE_DO_FUN(do_examine);
DECLARE_DO_FUN(do_exits);
DECLARE_DO_FUN(do_fill);
DECLARE_DO_FUN(do_flee);
DECLARE_DO_FUN(do_follow);
DECLARE_DO_FUN(do_force);
DECLARE_DO_FUN(do_freeze);
DECLARE_DO_FUN(do_trivia);
DECLARE_DO_FUN(do_gauction);
DECLARE_DO_FUN(do_get);
DECLARE_DO_FUN(do_give);
DECLARE_DO_FUN(do_goto);
DECLARE_DO_FUN(do_group);
DECLARE_DO_FUN(do_gtell);
DECLARE_DO_FUN(do_help);
DECLARE_DO_FUN(do_helps);
DECLARE_DO_FUN(do_hide);
DECLARE_DO_FUN(do_holylight);
DECLARE_DO_FUN(do_idea);
DECLARE_DO_FUN(do_ihelps);
DECLARE_DO_FUN(do_immtalk);
DECLARE_DO_FUN(do_inventory);
DECLARE_DO_FUN(do_invis);
DECLARE_DO_FUN(do_ireply);
DECLARE_DO_FUN(do_kdon);
DECLARE_DO_FUN(do_keep);
DECLARE_DO_FUN(do_kick);
DECLARE_DO_FUN(do_kill);
DECLARE_DO_FUN(do_list);
DECLARE_DO_FUN(do_loadlink);
DECLARE_DO_FUN(do_lock);
DECLARE_DO_FUN(do_log);
DECLARE_DO_FUN(do_look);
DECLARE_DO_FUN(do_loot);
DECLARE_DO_FUN(do_map);
DECLARE_DO_FUN(do_maps);
DECLARE_DO_FUN(do_maskqp);
DECLARE_DO_FUN(do_max_players);
DECLARE_DO_FUN(do_mecho);
DECLARE_DO_FUN(do_memory);
DECLARE_DO_FUN(do_mflag);
DECLARE_DO_FUN(do_mfind);
DECLARE_DO_FUN(do_mlist);
DECLARE_DO_FUN(do_mload);
DECLARE_DO_FUN(do_mpasound);
DECLARE_DO_FUN(do_mpat);
DECLARE_DO_FUN(do_mpecho);
DECLARE_DO_FUN(do_mpechoaround);
DECLARE_DO_FUN(do_mpechoat);
DECLARE_DO_FUN(do_mpforce);
DECLARE_DO_FUN(do_mpgoto);
DECLARE_DO_FUN(do_mpjunk);
DECLARE_DO_FUN(do_mpkill);
DECLARE_DO_FUN(do_mpmload);
DECLARE_DO_FUN(do_mpoload);
DECLARE_DO_FUN(do_mppurge);
DECLARE_DO_FUN(do_mpstat);
DECLARE_DO_FUN(do_mptransfer);
DECLARE_DO_FUN(do_mptongue);
DECLARE_DO_FUN(do_mset);
DECLARE_DO_FUN(do_mstat);
DECLARE_DO_FUN(do_mwhere);
DECLARE_DO_FUN(do_mudset);
DECLARE_DO_FUN(do_murde);
DECLARE_DO_FUN(do_murder);
DECLARE_DO_FUN(do_noemote);
DECLARE_DO_FUN(do_north);
DECLARE_DO_FUN(do_nopk);
DECLARE_DO_FUN(do_note);
DECLARE_DO_FUN(do_notell);
DECLARE_DO_FUN(do_nprompt);
DECLARE_DO_FUN(do_nosummon);
DECLARE_DO_FUN(do_novisit);
DECLARE_DO_FUN(do_nogive);
DECLARE_DO_FUN(do_norescue);
DECLARE_DO_FUN(do_nobond);
DECLARE_DO_FUN(do_nofollow);
DECLARE_DO_FUN(do_noobjspam);
DECLARE_DO_FUN(do_ocount);
DECLARE_DO_FUN(do_oflag);
DECLARE_DO_FUN(do_ofind);
DECLARE_DO_FUN(do_olist);
DECLARE_DO_FUN(do_oload);
DECLARE_DO_FUN(do_open);
DECLARE_DO_FUN(do_opotion);
DECLARE_DO_FUN(do_orare);
DECLARE_DO_FUN(do_order);
DECLARE_DO_FUN(do_oreset);
DECLARE_DO_FUN(do_oset);
DECLARE_DO_FUN(do_ostat);
DECLARE_DO_FUN(do_owhere);
DECLARE_DO_FUN(do_owear);
DECLARE_DO_FUN(do_pacifist);
DECLARE_DO_FUN(do_pagelen);
DECLARE_DO_FUN(do_pardon);
DECLARE_DO_FUN(do_password);
DECLARE_DO_FUN(do_peace);
DECLARE_DO_FUN(do_pgroup);
DECLARE_DO_FUN(do_pick);
DECLARE_DO_FUN(do_practice);
DECLARE_DO_FUN(do_preport);
DECLARE_DO_FUN(do_prompt);
DECLARE_DO_FUN(do_purge);
DECLARE_DO_FUN(do_put);
DECLARE_DO_FUN(do_qauction);
DECLARE_DO_FUN(do_quaff);
DECLARE_DO_FUN(do_question);
DECLARE_DO_FUN(do_quest2);
DECLARE_DO_FUN(do_qui);
DECLARE_DO_FUN(do_quit);
DECLARE_DO_FUN(do_reboo);
DECLARE_DO_FUN(do_reboot);
DECLARE_DO_FUN(do_recall);
DECLARE_DO_FUN(do_recho);
DECLARE_DO_FUN(do_recite);
DECLARE_DO_FUN(do_remove);
DECLARE_DO_FUN(do_rename);
DECLARE_DO_FUN(do_rent);
DECLARE_DO_FUN(do_reply);
DECLARE_DO_FUN(do_report);
DECLARE_DO_FUN(do_rescue);
DECLARE_DO_FUN(do_rest);
DECLARE_DO_FUN(do_restore);
DECLARE_DO_FUN(do_return);
DECLARE_DO_FUN(do_roomlist);
DECLARE_DO_FUN(do_rset);
DECLARE_DO_FUN(do_rstat);
DECLARE_DO_FUN(do_sacrifice);
DECLARE_DO_FUN(do_save);
DECLARE_DO_FUN(do_savecheck);
DECLARE_DO_FUN(do_say);
DECLARE_DO_FUN(do_score);
DECLARE_DO_FUN(do_sell);
DECLARE_DO_FUN(do_shout);
DECLARE_DO_FUN(do_showblack);
DECLARE_DO_FUN(do_showdamage);
DECLARE_DO_FUN(do_shutdow);
DECLARE_DO_FUN(do_shutdown);
DECLARE_DO_FUN(do_silence);
DECLARE_DO_FUN(do_sla);
DECLARE_DO_FUN(do_slay);
DECLARE_DO_FUN(do_sleep);
DECLARE_DO_FUN(do_slist);
DECLARE_DO_FUN(do_slookup);
DECLARE_DO_FUN(do_sneak);
DECLARE_DO_FUN(do_snoop);
DECLARE_DO_FUN(do_socials);
DECLARE_DO_FUN(do_south);
DECLARE_DO_FUN(do_spells);
DECLARE_DO_FUN(do_split);
DECLARE_DO_FUN(do_sreport);
DECLARE_DO_FUN(do_sset);
DECLARE_DO_FUN(do_stand);
DECLARE_DO_FUN(do_steal);
DECLARE_DO_FUN(do_switch);
DECLARE_DO_FUN(do_tell);
DECLARE_DO_FUN(do_time);
DECLARE_DO_FUN(do_title);
DECLARE_DO_FUN(do_token);
DECLARE_DO_FUN(do_trade);
DECLARE_DO_FUN(do_train);
DECLARE_DO_FUN(do_transfer);
DECLARE_DO_FUN(do_trust);
DECLARE_DO_FUN(do_typo);
DECLARE_DO_FUN(do_unlock);
DECLARE_DO_FUN(do_up);
DECLARE_DO_FUN(do_users);
DECLARE_DO_FUN(do_value);
DECLARE_DO_FUN(do_visible);
DECLARE_DO_FUN(do_wake);
DECLARE_DO_FUN(do_wear);
DECLARE_DO_FUN(do_weather);
DECLARE_DO_FUN(do_west);
DECLARE_DO_FUN(do_where);
DECLARE_DO_FUN(do_who);
DECLARE_DO_FUN(do_wimpy);
DECLARE_DO_FUN(do_wizhelp);
DECLARE_DO_FUN(do_wizify);
DECLARE_DO_FUN(do_wizlist);
DECLARE_DO_FUN(do_wizlock);
DECLARE_DO_FUN(do_yell);
DECLARE_DO_FUN(do_zap);
DECLARE_DO_FUN(do_otype);
DECLARE_DO_FUN(do_owhere);
DECLARE_DO_FUN(do_ignore);
DECLARE_DO_FUN(do_family);
DECLARE_DO_FUN(do_mount);
DECLARE_DO_FUN(do_dismount);
DECLARE_DO_FUN(do_qpspend);
DECLARE_DO_FUN(do_disguise);
DECLARE_DO_FUN(do_instruct);
DECLARE_DO_FUN(do_frenzy);
DECLARE_DO_FUN(do_adrenaline);
DECLARE_DO_FUN(do_target);
DECLARE_DO_FUN(do_charge);
DECLARE_DO_FUN(do_connect);
DECLARE_DO_FUN(do_stance);
DECLARE_DO_FUN(do_enchant);
DECLARE_DO_FUN(do_version);
DECLARE_DO_FUN(do_ctoggle);
DECLARE_DO_FUN(do_politics);
DECLARE_DO_FUN(do_negotiate);
DECLARE_DO_FUN(do_diptalk);
DECLARE_DO_FUN(do_familytalk);
DECLARE_DO_FUN(do_remorttalk);
DECLARE_DO_FUN(do_crusade);
DECLARE_DO_FUN(do_adepttalk);
DECLARE_DO_FUN(do_rulers);
DECLARE_DO_FUN(do_scout);
DECLARE_DO_FUN(do_alink);
DECLARE_DO_FUN(do_for);
DECLARE_DO_FUN(do_hotreboo);
DECLARE_DO_FUN(do_hotreboot);
DECLARE_DO_FUN(do_imtlset);
DECLARE_DO_FUN(do_gain_stat_reset);
DECLARE_DO_FUN(do_sedit);
DECLARE_DO_FUN(do_olmsg);
DECLARE_DO_FUN(do_scheck);
DECLARE_DO_FUN(do_immbrand);
DECLARE_DO_FUN(do_xpcalc);
DECLARE_DO_FUN(do_unpractice);

DECLARE_DO_FUN(do_stealth);
DECLARE_DO_FUN(do_idlecheck);

/*
 * Spell functions.
 * Defined in magic.c.
 */
DECLARE_SPELL_FUN(spell_null);
DECLARE_SPELL_FUN(spell_ego_whip);
DECLARE_SPELL_FUN(spell_physic_thrust);
DECLARE_SPELL_FUN(spell_physic_crush);
DECLARE_SPELL_FUN(spell_mind_flail);
DECLARE_SPELL_FUN(spell_acid_blast);
DECLARE_SPELL_FUN(spell_animate);
DECLARE_SPELL_FUN(spell_armor);
DECLARE_SPELL_FUN(spell_badbreath);
DECLARE_SPELL_FUN(spell_bark_skin);
DECLARE_SPELL_FUN(spell_bless);
DECLARE_SPELL_FUN(spell_blindness);
DECLARE_SPELL_FUN(spell_bloody_tears);
DECLARE_SPELL_FUN(spell_burning_hands);
DECLARE_SPELL_FUN(spell_call_lightning);
DECLARE_SPELL_FUN(spell_calm);
DECLARE_SPELL_FUN(spell_cause_critical);
DECLARE_SPELL_FUN(spell_cause_light);
DECLARE_SPELL_FUN(spell_cause_serious);
DECLARE_SPELL_FUN(spell_change_sex);
DECLARE_SPELL_FUN(spell_charm_person);
DECLARE_SPELL_FUN(spell_chill_touch);
DECLARE_SPELL_FUN(spell_colour_spray);
DECLARE_SPELL_FUN(spell_continual_light);
DECLARE_SPELL_FUN(spell_control_weather);
DECLARE_SPELL_FUN(spell_create_food);
DECLARE_SPELL_FUN(spell_create_spring);
DECLARE_SPELL_FUN(spell_create_water);
DECLARE_SPELL_FUN(spell_cure_blindness);
DECLARE_SPELL_FUN(spell_cure_critical);
DECLARE_SPELL_FUN(spell_cure_light);
DECLARE_SPELL_FUN(spell_cure_poison);
DECLARE_SPELL_FUN(spell_cure_serious);
DECLARE_SPELL_FUN(spell_curse);
DECLARE_SPELL_FUN(spell_detect_evil);
DECLARE_SPELL_FUN(spell_detect_hidden);
DECLARE_SPELL_FUN(spell_detect_invis);
DECLARE_SPELL_FUN(spell_detect_magic);
DECLARE_SPELL_FUN(spell_detect_poison);
DECLARE_SPELL_FUN(spell_detect_undead);
DECLARE_SPELL_FUN(spell_detection);
DECLARE_SPELL_FUN(spell_dimension_blade);
DECLARE_SPELL_FUN(spell_dispel_evil);
DECLARE_SPELL_FUN(spell_dispel_magic);
DECLARE_SPELL_FUN(spell_earthquake);
DECLARE_SPELL_FUN(spell_enchant_weapon);
DECLARE_SPELL_FUN(spell_encumber);
DECLARE_SPELL_FUN(spell_enhance_weapon);
DECLARE_SPELL_FUN(spell_energy_drain);
DECLARE_SPELL_FUN(spell_faerie_fire);
DECLARE_SPELL_FUN(spell_faerie_fog);
DECLARE_SPELL_FUN(spell_fighting_trance);
DECLARE_SPELL_FUN(spell_fireball);
DECLARE_SPELL_FUN(spell_fire_blade);
DECLARE_SPELL_FUN(spell_flamestrike);
DECLARE_SPELL_FUN(spell_fly);
DECLARE_SPELL_FUN(spell_gate);
DECLARE_SPELL_FUN(spell_general_purpose);
DECLARE_SPELL_FUN(spell_giant_strength);
DECLARE_SPELL_FUN(spell_harm);
DECLARE_SPELL_FUN(spell_heal);
DECLARE_SPELL_FUN(spell_high_explosive);
DECLARE_SPELL_FUN(spell_hypnosis);
DECLARE_SPELL_FUN(spell_identify);
DECLARE_SPELL_FUN(spell_influx);
DECLARE_SPELL_FUN(spell_infravision);
DECLARE_SPELL_FUN(spell_invis);
DECLARE_SPELL_FUN(spell_know_alignment);
DECLARE_SPELL_FUN(spell_know_weakness);
DECLARE_SPELL_FUN(spell_know_critical);
DECLARE_SPELL_FUN(spell_know_item);
DECLARE_SPELL_FUN(spell_lightning_bolt);
DECLARE_SPELL_FUN(spell_locate_object);
DECLARE_SPELL_FUN(spell_lsd);
DECLARE_SPELL_FUN(spell_magic_missile);
DECLARE_SPELL_FUN(spell_mass_invis);
DECLARE_SPELL_FUN(spell_mind_bolt);
DECLARE_SPELL_FUN(spell_nerve_fire);
DECLARE_SPELL_FUN(spell_night_vision);
DECLARE_SPELL_FUN(spell_pass_door);
DECLARE_SPELL_FUN(spell_phase);
DECLARE_SPELL_FUN(spell_poison);
DECLARE_SPELL_FUN(spell_produce_food);
DECLARE_SPELL_FUN(spell_protection);
DECLARE_SPELL_FUN(spell_refresh);
DECLARE_SPELL_FUN(spell_remove_curse);
DECLARE_SPELL_FUN(spell_sanctuary);
DECLARE_SPELL_FUN(spell_see_magic);
DECLARE_SPELL_FUN(spell_sense_evil);
DECLARE_SPELL_FUN(spell_shocking_grasp);
DECLARE_SPELL_FUN(spell_shield);
DECLARE_SPELL_FUN(spell_sleep);
DECLARE_SPELL_FUN(spell_stalker);
DECLARE_SPELL_FUN(spell_stone_skin);
DECLARE_SPELL_FUN(spell_suffocate);
DECLARE_SPELL_FUN(spell_summon);
DECLARE_SPELL_FUN(spell_teleport);
DECLARE_SPELL_FUN(spell_ventriloquate);
DECLARE_SPELL_FUN(spell_warcry);
DECLARE_SPELL_FUN(spell_weaken);
DECLARE_SPELL_FUN(spell_window);
DECLARE_SPELL_FUN(spell_portal);
DECLARE_SPELL_FUN(spell_beacon);
DECLARE_SPELL_FUN(spell_word_of_recall);
DECLARE_SPELL_FUN(spell_acid_breath);
DECLARE_SPELL_FUN(spell_fire_breath);
DECLARE_SPELL_FUN(spell_frost_breath);
DECLARE_SPELL_FUN(spell_gas_breath);
DECLARE_SPELL_FUN(spell_lightning_breath);
DECLARE_SPELL_FUN(spell_planergy);
DECLARE_SPELL_FUN(spell_static);
DECLARE_SPELL_FUN(spell_visit);
DECLARE_SPELL_FUN(spell_chain_lightning);
DECLARE_SPELL_FUN(spell_phobia);
DECLARE_SPELL_FUN(spell_barrier);
DECLARE_SPELL_FUN(spell_mindflame);
DECLARE_SPELL_FUN(spell_laserbolt);
DECLARE_SPELL_FUN(spell_hellspawn);
DECLARE_SPELL_FUN(spell_travel);
DECLARE_SPELL_FUN(spell_flare);
DECLARE_SPELL_FUN(spell_mystic_armor);
DECLARE_SPELL_FUN(spell_seal_room);
DECLARE_SPELL_FUN(spell_deflect_weapon);
DECLARE_SPELL_FUN(spell_black_hand);
DECLARE_SPELL_FUN(spell_throw_needle);
DECLARE_SPELL_FUN(spell_morale);
DECLARE_SPELL_FUN(spell_leadership);
DECLARE_SPELL_FUN(spell_ice_bolt);
DECLARE_SPELL_FUN(spell_waterelem);
DECLARE_SPELL_FUN(spell_skeleton);
DECLARE_SPELL_FUN(spell_poison_weapon);
DECLARE_SPELL_FUN(spell_embrace);
DECLARE_SPELL_FUN(spell_mesmerise);
DECLARE_SPELL_FUN(spell_ethereal);
DECLARE_SPELL_FUN(spell_fireelem);
DECLARE_SPELL_FUN(spell_rune_fire);
DECLARE_SPELL_FUN(spell_rune_shock);
DECLARE_SPELL_FUN(spell_rune_poison);
DECLARE_SPELL_FUN(spell_healing_light);
DECLARE_SPELL_FUN(spell_wither_shadow);
DECLARE_SPELL_FUN(spell_mana_flare);
DECLARE_SPELL_FUN(spell_mana_drain);
DECLARE_SPELL_FUN(spell_cage);
DECLARE_SPELL_FUN(spell_cloak_absorb);
DECLARE_SPELL_FUN(spell_cloak_reflect);
DECLARE_SPELL_FUN(spell_cloak_flaming);
DECLARE_SPELL_FUN(spell_cloak_darkness);
DECLARE_SPELL_FUN(spell_room_dispel);
DECLARE_SPELL_FUN(spell_cloak_adept);
DECLARE_SPELL_FUN(spell_cloak_regen);
DECLARE_SPELL_FUN(spell_throw_star);
DECLARE_SPELL_FUN(spell_soul_net);
DECLARE_SPELL_FUN(spell_condense_soul);
DECLARE_SPELL_FUN(spell_restoration);
DECLARE_SPELL_FUN(spell_infuse);
DECLARE_SPELL_FUN(spell_holy_light);
DECLARE_SPELL_FUN(spell_divine_intervention);
DECLARE_SPELL_FUN(spell_holy_armor);
DECLARE_SPELL_FUN(spell_earthelem);
DECLARE_SPELL_FUN(spell_iron_golem);
DECLARE_SPELL_FUN(spell_diamond_golem);
DECLARE_SPELL_FUN(spell_soul_thief);
DECLARE_SPELL_FUN(spell_holy_avenger);
DECLARE_SPELL_FUN(spell_heat_armor);
DECLARE_SPELL_FUN(spell_retri_strike);
DECLARE_SPELL_FUN(spell_lava_burst);
DECLARE_SPELL_FUN(spell_fireshield);
DECLARE_SPELL_FUN(spell_iceshield);
DECLARE_SPELL_FUN(spell_shockshield);
DECLARE_SPELL_FUN(spell_shadowshield);
DECLARE_SPELL_FUN(spell_thoughtshield);
DECLARE_SPELL_FUN(spell_summon_pegasus);
DECLARE_SPELL_FUN(spell_summon_nightmare);
DECLARE_SPELL_FUN(spell_summon_beast);
DECLARE_SPELL_FUN(spell_summon_devourer);
DECLARE_SPELL_FUN(spell_summon_shadow);
DECLARE_SPELL_FUN(spell_creature_bond);
DECLARE_SPELL_FUN(spell_corrupt_bond);
DECLARE_SPELL_FUN(spell_group_heal); /* Ariakan */
DECLARE_SPELL_FUN(spell_carapace); /* M & P */
DECLARE_SPELL_FUN(spell_summon_shadowdragon); /* Ariakan */
DECLARE_SPELL_FUN(spell_hellfire);
DECLARE_SPELL_FUN(spell_demonshield);
DECLARE_SPELL_FUN(spell_cloak_mana);
DECLARE_SPELL_FUN(spell_warning_rune);
DECLARE_SPELL_FUN(spell_emount);
DECLARE_SPELL_FUN(spell_gaze_mirror);

DECLARE_SPELL_FUN(spell_avatar_default);

DECLARE_SPELL_FUN(spell_tranquility_novice);
DECLARE_SPELL_FUN(spell_tranquility_intermediate);
DECLARE_SPELL_FUN(spell_tranquility_advanced);
DECLARE_SPELL_FUN(spell_tranquility_expert);
DECLARE_SPELL_FUN(spell_tranquility_master);

DECLARE_SPELL_FUN(spell_smokescreen_novice);
DECLARE_SPELL_FUN(spell_smokescreen_intermediate);
DECLARE_SPELL_FUN(spell_smokescreen_advanced);
DECLARE_SPELL_FUN(spell_smokescreen_expert);
DECLARE_SPELL_FUN(spell_smokescreen_master);

DECLARE_SPELL_FUN(spell_sentry_novice);
DECLARE_SPELL_FUN(spell_sentry_intermediate);
DECLARE_SPELL_FUN(spell_sentry_advanced);
DECLARE_SPELL_FUN(spell_sentry_expert);
DECLARE_SPELL_FUN(spell_sentry_master);

DECLARE_SPELL_FUN(spell_coldwave_novice);
DECLARE_SPELL_FUN(spell_coldwave_intermediate);
DECLARE_SPELL_FUN(spell_coldwave_advanced);
DECLARE_SPELL_FUN(spell_coldwave_expert);
DECLARE_SPELL_FUN(spell_coldwave_master);

DECLARE_SPELL_FUN(spell_innerflame_novice);
DECLARE_SPELL_FUN(spell_innerflame_intermediate);
DECLARE_SPELL_FUN(spell_innerflame_advanced);
DECLARE_SPELL_FUN(spell_innerflame_expert);
DECLARE_SPELL_FUN(spell_innerflame_master);

/*
 * OS-dependent declarations.
 * These are all very standard library functions,
 *   but some systems have incomplete or non-ansi header files.
 */
char *crypt args((const char *key, const char *salt));

/*
 * The crypt(3) function is not available on some operating systems.
 * In particular, the U.S. Government prohibits its export from the
 *   United States to foreign countries.
 * Turn on NOCRYPT to keep passwords in plain text.
 */
#if     defined(NOCRYPT)
# define crypt(s1, s2)   (s1)
#endif

#ifndef MSDOS
# ifndef unix
#  define unix
# endif
#endif


/*
 * Data files used by the server.
 *
 * AREA_LIST contains a list of areas to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 *
 * The NULL_FILE is held open so that we have a stream handle in reserve,
 *   so players can go ahead and telnet to all the other descriptors.
 * Then we close it whenever we need to open a file (e.g. a save file).
 */

#define PLAYER_DIR "../player/"
#define NPC_DIR    "../npcs/"
#define NULL_FILE  "/dev/null"
#define MOB_DIR    "MOBProgs/"

#define AREA_LIST       "../data/area.lst"      /* List of areas                */
#define AREA_LIST_NEW   "../data/area.lst.new"
#define BUG_FILE        "../data/bugs.txt"      /* For 'bug' and bug( )         */
#define IDEA_FILE       "../data/ideas.txt"     /* For 'idea'                   */
#define TYPO_FILE       "../data/typos.txt"     /* For 'typo'                   */
#define NOTE_FILE       "../data/notes.lst"     /* For 'notes'                  */
#define SHUTDOWN_FILE   "../data/shutdown.txt"  /* For 'shutdown'               */
#define CLAN_FILE       "../data/clandata.dat" /* stores clan diplomacy data   */
#define CINFO_FILE      "../data/claninfo.dat" /* stores cinfo data   */
#define CORPSE_FILE     "../data/corpses.lst"
#define CORPSELOG_FILE  "../data/corpses.txt"
#define BANS_FILE       "../data/bans.lst"
#define RULERS_FILE     "../data/rulers.lst"
#define BRANDS_FILE     "../data/brands.lst"
#define BRANDSLOG_FILE  "../data/brands.txt"
#define SHELP_FILE      "../data/shelp.lst"
#define MUDSET_FILE     "../data/mudset.lst"
#define RENAME_FILE     "../data/rename.lst"
#define SOCIAL_FILE     "../data/socials.txt"
#define TREASURY_FILE   "../data/treasury.lst"

#define HELP_DIR       "helps/"
#define HELP_FILE      "helps.lst"

#define LOCKER_DIR "locker/"

#define HELP_NORMAL 0
#define HELP_IMM    1
#define HELP_BUILD  2
#define HELP_RULES  3
#define HELP_MAP    4

#if !defined(WHO_HTML_FILE)
# define WHO_HTML_FILE  "/dev/null"  /* for ftping who list to html web page :) */
# define WHO_COUNT_FILE "/dev/null"
#endif

#define COPYOVER_FILE   "COPYOVER.TXT"  /* Temp data file used for copyover */
#define EXE_FILE        "../src/merc"   /* The one that runs the ACK! */

void FPRINTF(FILE *fp, char *fmt, ...) __attribute__((format(printf, 2, 3)));

/*
 * Our function prototypes.
 * One big lump ... this is every function in Merc.
 */
#define CD  CHAR_DATA
#define MID MOB_INDEX_DATA
#define OD  OBJ_DATA
#define OID OBJ_INDEX_DATA
#define RID ROOM_INDEX_DATA
#define SF  SPEC_FUN

/* act_comm.c */
bool is_note_to         args((CHAR_DATA *ch, NOTE_DATA *pnote));
void add_follower       args((CHAR_DATA *ch, CHAR_DATA *master));
void stop_follower      args((CHAR_DATA *ch));
void die_follower       args((CHAR_DATA *ch));
bool is_same_group      args((CHAR_DATA *ach, CHAR_DATA *bch));
bool is_group_leader    args((CHAR_DATA *ch));
char *slur_text         args((char *argument));
void send_to_room       args((char *message, ROOM_INDEX_DATA *room));
void list_who_to_output args((void));
bool can_group          args((CHAR_DATA *ch, CHAR_DATA *victim));
bool can_group_level    args((CHAR_DATA *ch, int level));

/* act_info.c */
void set_title        args((CHAR_DATA *ch, char *title));
char *colour_string   args((CHAR_DATA *CH, char *argument));
char *get_family_name args((CHAR_DATA *ch));
char *get_tribe_name  args((CHAR_DATA *ch));
int  dur_to_secs      args((char *argument));
bool send_help        args((void *to, char *argument, int type, bool desc));
void show_helps       args((CHAR_DATA *ch, int type));

int  commands_cmp     args((const void *x, const void *y));
void commands_list    args((CHAR_DATA *ch, int imm));

/* act_move.c */
void move_char args((CHAR_DATA *ch, int door));

/* act_obj.c */
void get_obj        args((CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container));
bool can_wear_at    args((CHAR_DATA * ch, OBJ_DATA * obj, int location));
bool has_quest_item args((CHAR_DATA *ch));

/* act_wiz.c */
ROOM_INDEX_DATA *find_location args((CHAR_DATA *ch, char *arg));

/* board.c */
BOARD_DATA *load_board        args((OBJ_INDEX_DATA *pObj));
void       do_show_contents   args((CHAR_DATA *ch, OBJ_DATA *obj));
void       do_show_message    args((CHAR_DATA *ch, int mess_num, OBJ_DATA *obj));
void       do_edit_message    args((CHAR_DATA *ch, int mess_num, OBJ_DATA *obj));
void       do_add_to_message  args((CHAR_DATA *ch, char *argument));
void       do_start_a_message args((CHAR_DATA *ch, char *argument));
void       save_message_data  args((void));
void       load_messages      args((void));

/* comm.c */
void   close_socket      args((DESCRIPTOR_DATA *dclose));
void   show_menu_to      args((DESCRIPTOR_DATA *d)); /* Main */
void   show_amenu_to     args((DESCRIPTOR_DATA *d)); /* Attributes */
void   show_rmenu_to     args((DESCRIPTOR_DATA *d)); /* Race */
void   show_smenu_to     args((DESCRIPTOR_DATA *d)); /* Sex */
void   show_cmenu_to     args((DESCRIPTOR_DATA *d)); /* Class */
void   write_to_buffer   args((DESCRIPTOR_DATA *d, const char *txt, int length));
void   send_to_char      args((const char *txt, CHAR_DATA *ch));
void   show_string       args((DESCRIPTOR_DATA *d, char *input));
void   act               args((const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, int type));
void   hang              args((const char *str));
time_t get_today_ctime   args((void));
char   *get_today_string args((time_t today));
char   *percbar          args((int curhp, int maxhp, int width));

/* db.c */
void perm_update         args((void));
void boot_db             args((bool fCopyOver));
void area_update         args((void));
void message_update      args((void));
CD   *create_mobile      args((MOB_INDEX_DATA *pMobIndex));
OD   *create_object      args((OBJ_INDEX_DATA *pObjIndex, int level));
void clear_char          args((CHAR_DATA *ch));
void free_char           args((CHAR_DATA *ch));
char *get_extra_descr    args((const char *name, EXTRA_DESCR_DATA *ed));
MID  *get_mob_index      args((int vnum));
OID  *get_obj_index      args((int vnum));
RID  *get_room_index     args((int vnum));
char fread_letter        args((FILE *fp));
int  fread_number        args((FILE *fp));
unsigned int fread_unumber args((FILE *fp));
int  *fread_number_array args((FILE *fp));
char *print_number_array args((int *arr));
char *fread_string       args((FILE *fp));

#define fread_to_eol(x) _fread_to_eol((x), _caller)
void _fread_to_eol       args((FILE *fp, const char *caller));

char *fsave_to_eol       args((FILE *fp));
char *fread_word         args((FILE *fp));
char *wordwrap           args((char *source, char *line1, char *line2, int width)); /* shelp addition */
void *_getmem            args((int size, const char *caller, int log));
void dispose             args((void *mem, int size));
char *str_dup            args((const char *str));
void free_string         args((char *pstr));
int  number_fuzzy        args((int number));
int  number_range        args((int from, int to));
int  number_percent      args((void));
int  number_door         args((void));
int  number_bits         args((int width));
int  number_mm           args((void));
int  dice                args((int number, int size));
int  interpolate         args((int level, int value_00, int value_32));
void smash_tilde         args((char *str));
bool str_cmp             args((const char *astr, const char *bstr));
bool str_prefix          args((const char *astr, const char *bstr));
bool str_iprefix         args((const char *astr, const char *bstr));
bool str_infix           args((const char *astr, const char *bstr));
int  strpos              args((const char *haystack, const char *needle));
bool str_suffix          args((const char *astr, const char *bstr));
char *capitalize         args((const char *str));
char *number_comma       args((int x));
char *number_comma_r     args((int x, char *dst));
void append_file         args((CHAR_DATA *ch, char *file, char *str));
void tail_chain          args((void));
void safe_strcat         args((int max_len, char *dest, char *source));
void send_to_descrips    args((const char *message));
void bug                 args((const char *str));
void log_string          args((const char *str));
void bugf                (char * fmt, ...) __attribute__((format(printf, 1, 2)));
void xlogf               (char * fmt, ...) __attribute__((format(printf, 1, 2)));
void sendf               (CHAR_DATA *, char * fmt, ...) __attribute__((format(printf, 2, 3)));
void challenge           (char *message);
void challengef          (char * fmt, ...) __attribute__((format(printf, 1, 2)));
void gain                (char *message);
void gainf               (char * fmt, ...) __attribute__((format(printf, 1, 2)));
void auction             args((char *message));
void auctionf            (char * fmt, ...) __attribute__((format(printf, 1, 2)));
void arena               (char *message);
void arenaf              (CHAR_DATA *, char * fmt, ...) __attribute__((format(printf, 2, 3)));

/* fight.c */
void violence_update args((void));
void multi_hit       args((CHAR_DATA *ch, CHAR_DATA *victim, int dt));
void damage          args((CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt));
void update_pos      args((CHAR_DATA *victim));
void stop_fighting   args((CHAR_DATA *ch, bool fBoth));
void death_cry       args((CHAR_DATA *ch));
void raw_kill        args((CHAR_DATA *victim, char *argument));
void check_killer    args((CHAR_DATA *ch, CHAR_DATA *victim));
bool is_safe         args((CHAR_DATA *ch, CHAR_DATA *victim, bool showmessage));
bool is_in_pk        args((CHAR_DATA *ch));
char *showdamage     args((CHAR_DATA *to, CHAR_DATA *ch, CHAR_DATA *victim, int dam, bool crack));

/* handler.c */
bool remove_obj        args((CHAR_DATA *ch, int iWear, bool fReplace));
int  get_trust         args((CHAR_DATA *ch));
void my_get_age        args((CHAR_DATA *ch, char *buf));
int  my_get_hours      args((CHAR_DATA *ch));
int  get_age           args((CHAR_DATA *ch));
int  get_curr_str      args((CHAR_DATA *ch));
int  get_curr_int      args((CHAR_DATA *ch));
int  get_curr_wis      args((CHAR_DATA *ch));
int  get_curr_dex      args((CHAR_DATA *ch));
int  get_curr_con      args((CHAR_DATA *ch));
int  can_carry_n       args((CHAR_DATA *ch));
int  can_carry_w       args((CHAR_DATA *ch));
bool is_name           args((const char *str, char *namelist));
void affect_to_room    args((ROOM_INDEX_DATA *room, ROOM_AFFECT_DATA *raf));
void r_affect_remove   args((ROOM_INDEX_DATA *room, ROOM_AFFECT_DATA *raf));
void affect_to_char    args((CHAR_DATA *ch, AFFECT_DATA *paf));
void affect_remove     args((CHAR_DATA *ch, AFFECT_DATA *paf));
void affect_strip      args((CHAR_DATA *ch, int sn));
bool is_affected       args((CHAR_DATA *ch, int sn));
void affect_join       args((CHAR_DATA *ch, AFFECT_DATA *paf));
AFFECT_DATA *find_apply_location args((void *obj, int location, bool pindex));
void char_from_room    args((CHAR_DATA *ch));
void char_to_room      args((CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex));
void obj_to_char       args((OBJ_DATA *obj, CHAR_DATA *ch));
void obj_from_char     args((OBJ_DATA *obj));
int  apply_ac          args((OBJ_DATA *obj, int iWear));
OD   *get_eq_char      args((CHAR_DATA *ch, int iWear));
void equip_char        args((CHAR_DATA *ch, OBJ_DATA *obj, int iWear));
void unequip_char      args((CHAR_DATA *ch, OBJ_DATA *obj));
int  count_obj_list    args((OBJ_INDEX_DATA *obj, OBJ_DATA *list));
int  count_obj_room    args((OBJ_INDEX_DATA *obj, OBJ_DATA *list));
int  count_obj_room_mob args((OBJ_INDEX_DATA *pObjIndex, ROOM_INDEX_DATA *room));
void obj_from_room     args((OBJ_DATA *obj));
void obj_to_room       args((OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex));
void obj_to_obj        args((OBJ_DATA *obj, OBJ_DATA *obj_to));
void obj_from_obj      args((OBJ_DATA *obj));
void extract_obj       args((OBJ_DATA *obj));
void extract_char      args((CHAR_DATA *ch, bool fPull));
CD   *get_char_room    args((CHAR_DATA *ch, char *argument));
CD   *get_char_world   args((CHAR_DATA *ch, char *argument));
CD   *get_char_area    args((CHAR_DATA *ch, char *argument));
OD   *get_obj_type     args((OBJ_INDEX_DATA *pObjIndexData));
OD   *get_obj_list     args((CHAR_DATA *ch, char *argument, OBJ_DATA *list));
OD   *get_obj_room     args((CHAR_DATA *ch, char *argument, OBJ_DATA *list));
OD   *get_obj_carry    args((CHAR_DATA *ch, char *argument));
OD   *get_obj_wear     args((CHAR_DATA *ch, char *argument));
OD   *get_obj_here     args((CHAR_DATA *ch, char *argument));
OD   *get_obj_here_r   args((CHAR_DATA *ch, char *argument));
OD   *get_obj_world    args((CHAR_DATA *ch, char *argument));
OD   *create_money     args((int amount));
int  get_obj_number    args((OBJ_DATA *obj));
int  get_obj_weight    args((OBJ_DATA *obj));
bool room_is_dark      args((ROOM_INDEX_DATA *pRoomIndex));
bool room_is_private   args((ROOM_INDEX_DATA *pRoomIndex));
bool can_see           args((CHAR_DATA *ch, CHAR_DATA *victim));
bool can_see_obj       args((CHAR_DATA *ch, OBJ_DATA *obj));
bool can_drop_obj      args((CHAR_DATA *ch, OBJ_DATA *obj));
bool can_sac_obj       args((CHAR_DATA *ch, OBJ_DATA *obj));
char *item_type_name   args((OBJ_DATA *obj));
char *affect_loc_name  args((int location));
char *affect_bit_name  args((int vector));
char *raffect_bit_name args((int vector));
char *extra_bit_name   args((int extra_flags));
char *race_name        args((CHAR_DATA *ch));
char *short_race_name  args((CHAR_DATA *ch));
bool can_use           args((CHAR_DATA *ch, OBJ_DATA *obj));
char *who_can_use      args((OBJ_DATA *obj));
void notify            args((char *message, int lv));
void info              args((char *message, int lv));
void log_chan          args((const char *message, int lv));
bool item_has_apply    args((CHAR_DATA *ch, int bit));
int  best_class        args((CHAR_DATA *ch, int sn));
int  best_level        args((CHAR_DATA *ch, int sn));
char *center_text      args((char *text, int width));
void monitor_chan      args((const char *message, int channel));
void set_stun          args((CHAR_DATA *victim, int stunTime));
bool is_shielded       args((CHAR_DATA *ch, int index));
CD   *get_char         args((CHAR_DATA *ch));
void mark_from_room    args((int this_room_vnum, MARK_DATA *mark));
void mark_to_room      args((int this_room_vnum, MARK_DATA *mark));
void char_reference    args((struct char_ref_type *ref));
void char_unreference  args((CHAR_DATA **var));
void obj_reference     args((struct obj_ref_type *ref));
void obj_unreference   args((OBJ_DATA **var));
char *get_path_step    args((char *path, int step));
void join_arena        args((CHAR_DATA *ch));
void leave_arena       args((CHAR_DATA *ch, bool alive));

/* interp.c */
void interpret             args((CHAR_DATA *ch, char *argument));
bool is_number             args((char *arg));
int  number_argument       args((char *argument, char *arg));
char *one_argument         args((char *argument, char *arg_first));
char *one_argument_nolower args((char *argument, char *arg_first));

bool IS_SWITCHED     args((CHAR_DATA *ch));
bool authorized      args((CHAR_DATA *ch, char *skllnm));
bool check_social    args((CHAR_DATA *ch, char *command, char *argument));

/* macros.c */
sh_int get_remort_level         args((CHAR_DATA *ch));
sh_int get_pseudo_level         args((CHAR_DATA *ch));
bool   ok_to_use                args((CHAR_DATA *ch, const struct lookup_type *table, int value));
bool   check_level_use          args((CHAR_DATA *ch, int level));
char   *learnt_name             args((int learnt));
int    exp_to_level             args((CHAR_DATA *ch, int class, int index));
int    exp_for_mobile           args((int level, CHAR_DATA *mob));
int    my_strlen                args((char *text));
int    skill_table_lookup       args((CHAR_DATA *ch, int sn, int return_type));
bool   is_remort                args((CHAR_DATA *ch));
int    exp_to_level_adept       args((CHAR_DATA *ch));
char   *get_adept_name          args((CHAR_DATA *ch));
char   *get_adept_name2         args((CHAR_DATA *ch));
void   reset_gain_stats         args((CHAR_DATA *ch));
int    get_item_value           args((OBJ_DATA *obj));
char   *get_moon_phase_name     args((void));
char   *duration                args((unsigned int dur, char *dest));
void   timeval_diff             args((struct timeval *tv, struct timeval *tv2, int *sec, int *usec));
bool   lootable_item            args((OBJ_DATA *obj));
int    max_orders               args((CHAR_DATA *ch));
void   strip_set                args((char *src, char *list));
void   allow_set                args((char *src, char *list));
void   remove_bad_codes         args((char *line));
char   effective_code           args((char c));
char   last_code_used           args((char *line));
bool   legal_cmd                args((CHAR_DATA *ch, int cmd));
bool   legal_spell              args((CHAR_DATA *ch, int sn));
char   *generate_cookie         args((void));
int    best_learnt              args((CHAR_DATA *ch, int sn));

#if defined(_ANSI_SOURCE) || defined(_POSIX_SOURCE) || defined(__STRICT_ANSI__)
/* -ansi doesn't support strncasecmp, so define it */
int strncasecmp(const char *s1, const char *s2, size_t n);
#endif

typedef int strcasefunc (const char *, const char *, size_t);
char *_strnreplace(const char *search, const char *replace, const char *subject, char *out, int max, strcasefunc *func);
#define     strnreplace(search, replace, subject, out, max) (_strnreplace(search, replace, subject, out, max, strncmp))
#define strncasereplace(search, replace, subject, out, max) (_strnreplace(search, replace, subject, out, max, strncasecmp))

void recalc_playercounts(void);

/* magic.c */
int  mana_cost       args((CHAR_DATA *ch, int sn));
int  skill_lookup    args((const char *name));
int  slot_lookup     args((int slot));
bool saves_spell     args((int level, CHAR_DATA *victim));
void obj_cast_spell  args((int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj));
bool spell_identify  args((int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj));
bool can_summon_charmie args((CHAR_DATA *master));

/* magic4.c */
int avatarlimit_lookup args((char *skill));

/* mob_prog.c */
#ifdef DUNNO_STRSTR
char *strstr                args((const char *s1, const char *s2));
#endif

void mprog_wordlist_check   args((char *arg, CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *object, void *vo, int type));
void mprog_percent_check    args((CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *object, void *vo, int type));
void mprog_act_trigger      args((char *buf, CHAR_DATA *mob, CHAR_DATA *ch, OBJ_DATA *obj, void *vo));
void mprog_bribe_trigger    args((CHAR_DATA *mob, CHAR_DATA *ch, int amount));
void mprog_entry_trigger    args((CHAR_DATA *mob));
void mprog_give_trigger     args((CHAR_DATA *mob, CHAR_DATA *ch, OBJ_DATA *obj));
void mprog_greet_trigger    args((CHAR_DATA *mob));
void mprog_fight_trigger    args((CHAR_DATA *mob, CHAR_DATA *ch));
void mprog_hitprcnt_trigger args((CHAR_DATA *mob, CHAR_DATA *ch));
void mprog_death_trigger    args((CHAR_DATA *mob));
void mprog_random_trigger   args((CHAR_DATA *mob));
void mprog_speech_trigger   args((char *txt, CHAR_DATA *mob));

/* quest.c */
void quest_inform        args((void));
void quest_complete      args((CHAR_DATA *ch));
void quest_cancel        args((void));
void generate_auto_quest args((void));
void clear_quest         args((void));
char *my_left            args((char *src, char *dst, int len));
char *my_right           args((char *src, char *dst, int len));

/* save.c */
void save_char_obj args((CHAR_DATA *ch));
bool load_char_obj args((DESCRIPTOR_DATA *d, char *name, bool syscall));
void save_corpses  args((void));
void fread_corpse  args((FILE *fp));
void save_marks    args((void));
void save_bans     args((void));
void save_shelps   args((void)); /* shelp addition */
void save_notes    args((void));
void save_mudsets  args((void));
void save_renames  args((void));
void load_locker  args((ROOM_INDEX_DATA *room));
void save_locker  args((ROOM_INDEX_DATA *room));
char *initial      args((const char *str));

/* special.c */
SF   *spec_lookup      args((const char *name));
char *rev_spec_lookup  args((void *func));
void print_spec_lookup args((char *buf));

/* social-edit.c  */
void load_social_table args((void));

/* objfun.c */
OBJ_FUN *obj_fun_lookup          args((const char *name));
char    *rev_obj_fun_lookup      args((void *func));
char    *rev_obj_fun_lookup_nice args((void *func));
void    print_obj_fun_lookup     args((char *buf));

/* trigger.c */
void trigger_handler args((CHAR_DATA *ch, OBJ_DATA *obj, int trigger));

/* update.c */
void advance_level   args((CHAR_DATA *ch, int class, bool show, bool remort, int levels));
void gain_exp        args((CHAR_DATA *ch, int gain));
void gain_condition  args((CHAR_DATA *ch, int iCond, int value));
void update_handler  args((void));
bool check_rewield   args((CHAR_DATA *ch));
bool check_re_equip  args((CHAR_DATA *ch));
void auction_update  args((void));
void teleport_update args((void));

/* write.c */
void write_start     args((char **dest, void *retfunc, void *retparm, CHAR_DATA *ch));
void write_interpret args((CHAR_DATA *ch, char *argument));

/* build.c */
void build_strdup      (char **dest, char *src, bool freesrc, CHAR_DATA *ch);
char *build_simpstrdup (char *buf);  /* A plug in alternative to str_dup */
void build_save        args((void));
int  get_dir           (char);
char *show_values      (const struct lookup_type *table, int value, bool fBit);

extern const char * cDirs;

/* buildare.c */
int build_canread  (AREA_DATA *Area, CHAR_DATA *ch, int showerror);
int build_canwrite (AREA_DATA *Area, CHAR_DATA *ch, int showerror);

#define AREA_NOERROR   0
#define AREA_SHOWERROR 1

/* areasave.c */
void area_modified    (AREA_DATA *);
void build_save_flush (void);

/* hunt.c */
void   hunt_victim args((CHAR_DATA *ch));
void   unhunt      args((CHAR_DATA *ch));
int    make_hunt   args((CHAR_DATA *ch, CHAR_DATA * victim));
void   hunt_obj    args((CHAR_DATA *ch));
bool   make_move   args((CHAR_DATA *ch, int vnum));
char   *find_path  args((int, int, CHAR_DATA *, int, int, int));
bool   mob_hunt    args((CHAR_DATA *mob));
void   char_hunt   args((CHAR_DATA *ch));
bool   set_hunt    args((CHAR_DATA *ch, CHAR_DATA *fch, CHAR_DATA *vch, OBJ_DATA *vobj, int set_flags, int rem_flags));
void   end_hunt    args((CHAR_DATA *ch));
sh_int h_find_dir  args((ROOM_INDEX_DATA *room, ROOM_INDEX_DATA *target, int h_flags));

/* update.c */
void init_alarm_handler args((void));
void alarm_update       args((void));

/* ssm.c */
void temp_fread_string args((FILE *fp, char *buf));

/* mpedit addition -- mob prog stuff-Aeria*/
int  mprog_name_to_type  args((char* name));
char *mprog_type_to_name args((int type));

/* rulers.c */
void save_rulers      args((void));
void load_rulers      args((void));
RULER_DATA *get_ruler args((CHAR_DATA *ch));

/* spendqp.c */
void save_brands args((void));

/* act_clan.c (cinfo) */
void save_cinfo   args((void));
void load_cinfo   args((void));
void update_cinfo args((CHAR_DATA *ch, bool remove));
void new_cinfo    args((CHAR_DATA *ch));

/* mccp.c */
bool compressStart     (DESCRIPTOR_DATA *desc);
bool compressEnd       (DESCRIPTOR_DATA *desc);
bool processCompressed (DESCRIPTOR_DATA *desc);
bool writeCompressed   (DESCRIPTOR_DATA *desc, char *txt, int length);

/* trade.c */
void trade_abort args((CHAR_DATA *ch));

/* md5c.c */
char *md5string args((char *text, char *out));

#undef  CD
#undef  MID
#undef  OD
#undef  OID
#undef  RID
#undef  SF
