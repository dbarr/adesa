#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "merc.h"
#include "duel.h"

IDSTRING(rcsid, "$Id: buildtab.c,v 1.30 2005/01/19 23:43:08 dave Exp $");

#define XX 0

const struct lookup_type tab_mob_class[] = {
    {"mage",             0, XX},
    {"cleric",           1, XX},
    {"thief",            2, XX},
    {"warrior",          3, XX},
    {"psionicist",       4, XX},
    {"sorcerer",         5, XX},
    {"assassin",         6, XX},
    {"knight",           7, XX},
    {"necromancer",      8, XX},
    {"monk",             9, XX},
    {NULL,              XX, XX}
};

const struct lookup_type tab_item_types[] = {
    {"light",            1, XX},
    {"scroll",           2, XX},
    {"wand",             3, XX},
    {"staff",            4, XX},
    {"weapon",           5, XX},
    {"beacon",           6, 90},
    {"portal",           7, 90},
    {"treasure",         8, XX},
    {"armor",            9, XX},
    {"potion",          10, XX},
    {"clutch",          11, XX},
    {"furniture",       12, XX},
    {"trash",           13, XX},
    {"trigger",         14, XX},
    {"container",       15, XX},
    {"quest",           16, 90},
    {"drink_con",       17, XX},
    {"key",             18, XX},
    {"food",            19, XX},
    {"money",           20, XX},
    {"nada",            21, XX},
    {"boat",            22, XX},
    {"corpse_npc",      23, XX},
    {"corpse_pc",       24, XX},
    {"fountain",        25, XX},
    {"pill",            26, XX},
    {"board",           27, XX},
    {"soul",            28, XX},
    {"piece",           29, XX},
    {"matrix",          30, XX},
    {"enchantment",     31, XX},
    {NULL,              XX, XX}
};

/* if a value is unused, only let level 90 set it, because normal builders
 * shouldn't be messing with an unused value
 *
 * values are the item type * 10, plus 0 to 3.
 */
const struct lookup_type tab_value_meanings[] = {
    /* light */
    {"Unused",                                       10, 90},
    {"Unused",                                       11, 90},
    {"Hours of light, -1 = infinite",                12, XX},
    {"Unused",                                       13, 90},

    /* scroll */
    {"Level",                                        20, XX},
    {"Spell 1",                                      21, XX},
    {"Spell 2",                                      22, XX},
    {"Spell 3",                                      23, XX},

    /* wand */
    {"Level",                                        30, XX},
    {"Max Charges",                                  31, XX},
    {"Current Charges",                              32, XX},
    {"Spell",                                        33, XX},

    /* staff */
    {"Level",                                        40, XX},
    {"Max Charges",                                  41, XX},
    {"Current Charges",                              42, XX},
    {"Spell",                                        43, XX},

    /* weapon */
    {"Min Damage",                                   50, XX},
    {"Max Damage",                                   51, XX},
    {"Unused",                                       52, 90},
    {"Weapon Type",                                  53, XX},

    /* beacon */
    {"Unused",                                       60, 90},
    {"Unused",                                       61, 90},
    {"Unused",                                       62, 90},
    {"Unused",                                       63, 90},

    /* portal */
    {"Vnum",                                         70, 90},
    {"Unused",                                       71, 90},
    {"0 = window, 1 = portal",                       72, XX},
    {"Unused",                                       73, 90},

    /* treasure */
    {"Unused",                                       80, 90},
    {"Unused",                                       81, 90},
    {"Unused",                                       82, 90},
    {"Unused",                                       83, 90},

    /* armor */
    {"Armor Class",                                  90, 90},
    {"Unused",                                       91, 90},
    {"Unused",                                       92, 90},
    {"Unused",                                       93, 90},

    /* potion */
    {"Level",                                       100, XX},
    {"Spell 1",                                     101, XX},
    {"Spell 2",                                     102, XX},
    {"Spell 3",                                     103, XX},

    /* clutch */
    {"See clutchinfo command",                      110, XX},
    {"See clutchinfo command",                      111, XX},
    {"See clutchinfo command",                      112, XX},
    {"See clutchinfo command",                      113, XX},

    /* furniture */
    {"Number of people that can use",               120, XX},
    {"Unused",                                      121, 90},
    {"Unused",                                      122, 90},
    {"Unused",                                      123, 90},

    /* trash */
    {"Unused",                                      130, 90},
    {"Unused",                                      131, 90},
    {"Unused",                                      132, 90},
    {"Unused",                                      133, 90},

    /* trigger */
    {"Trigger type",                                140, XX},
    {"Action type",                                 141, XX},
    {"Optional argument",                           142, XX},
    {"Optional argument",                           143, XX},

    /* container */
    {"Weight Capacity",                             150, XX},
    {"Flags",                                       151, XX},
    {"Key Vnum",                                    152, XX},
    {"Unused",                                      153, 90},

    /* quest */
    {"Value in Quest Points",                       160, 90},
    {"Value in Practices",                          161, 90},
    {"Value in Gold Pieces",                        162, 90},
    {"Value in Experience Points",                  163, 90},

    /* drink container */
    {"Capacity",                                    170, XX},
    {"Current Quantity",                            171, XX},
    {"Liquid Number",                               172, XX},
    {"Poisoned if non-zero",                        173, XX},

    /* key */
    {"Unused",                                      180, XX},
    {"Unused",                                      181, XX},
    {"Unused",                                      182, XX},
    {"Unused",                                      183, XX},

    /* food */
    {"Hours of Food Value",                         190, XX},
    {"Unused",                                      191, 90},
    {"Unused",                                      192, 90},
    {"Poisoned if non-zero",                        193, XX},

    /* money */
    {"Value in GP",                                 200, XX},
    {"Unused",                                      201, 90},
    {"Unused",                                      202, 90},
    {"Unused",                                      203, 90},

    /* boat */
    {"Unused",                                      220, 90},
    {"Unused",                                      221, 90},
    {"Unused",                                      222, 90},
    {"Unused",                                      223, 90},

    /* NPC corpse */
    {"Unused",                                      230, 90},
    {"Unused",                                      231, 90},
    {"Unused",                                      232, 90},
    {"Unused",                                      233, 90},

    /* PC corpse */
    {"Unused",                                      240, 90},
    {"Unused",                                      241, 90},
    {"Unused",                                      242, 90},
    {"Unused",                                      243, 90},

    /* fountain */
    {"Liquid Number",                               250, XX},
    {"Posioned if non-zero",                        251, XX},
    {"Unused",                                      252, 90},
    {"Unused",                                      253, 90},

    /* pill */
    {"Level",                                       260, XX},
    {"Spell 1",                                     261, XX},
    {"Spell 2",                                     262, XX},
    {"Spell 3",                                     263, XX},

    /* board */
    {"# of days message will last",                 270, XX},
    {"Level to write board",                        271, XX},
    {"Level to read board",                         272, XX},
    {"Board vnum",                                  273, XX},

    /* soul */
    {"Unused",                                      280, 90},
    {"Unused",                                      281, 90},
    {"Unused",                                      282, 90},
    {"Unused",                                      283, 90},

    /* piece */
    {"Pre-connect vnum",                            290, XX},
    {"Post-connect vnum",                           291, XX},
    {"Replacement vnum",                            292, XX},
    {"Unused",                                      293, 90},

    /* matrix */
    {"Unused",                                      300, 90},
    {"Unused",                                      301, 90},
    {"Unused",                                      302, 90},
    {"Unused",                                      303, 90},

    /* enchantment */
    {"-1 extra, -2 apply, -3 objfun, >= 0 stat",    310, XX},
    {"Modify Amount",                               311, XX},
    {"QP Cost",                                     312, XX},
    {"Bitvector for extra/apply",                   313, XX},

    {NULL,                                           XX, XX}
};

const struct lookup_type tab_drink_types[] = {
    {"Water",                0, XX},
    {"Beer",                 1, XX},
    {"Wine",                 2, XX},
    {"Ale",                  3, XX},
    {"Dark Ale",             4, XX},
    {"Whisky",               5, XX},
    {"Lemonade",             6, XX},
    {"Firebreather",         7, XX},
    {"Local Specialty",      8, XX},
    {"Slime Mold Juice",     9, XX},
    {"Milk",                10, XX},
    {"Tea",                 11, XX},
    {"Coffee",              12, XX},
    {"Blood",               13, XX},
    {"Salt Water",          14, XX},
    {"Chocolate Milk",      15, XX},
    {"Mountain Dew",        16, XX},
    {"Bourbon",             17, XX},
    {NULL,                  XX, XX}
};

const struct lookup_type tab_weapon_types[] = {
    {"hit",          0, XX},
    {"slice",        1, XX},
    {"stab",         2, XX},
    {"slash",        3, XX},
    {"whip",         4, XX},
    {"claw",         5, XX},
    {"blast",        6, XX},
    {"pound",        7, XX},
    {"crush",        8, XX},
    {"grep",         9, XX},
    {"bite",        10, XX},
    {"pierce",      11, XX},
    {"suction",     12, XX},
    {NULL,          XX, XX}
};

const struct lookup_type tab_mob_flags[] = {
    {"nada",                BIT_0,  90},
    {"is_npc",              BIT_1,  90},
    {"sentinel",            BIT_2,  XX},
    {"scavenger",           BIT_3,  XX},
    {"remember",            BIT_4,  XX},
    {"no_flee",             BIT_5,  XX},
    {"aggressive",          BIT_6,  XX},
    {"stay_area",           BIT_7,  XX},
    {"wimpy",               BIT_8,  XX},
    {"pet",                 BIT_9,  90},
    {"train",               BIT_10, XX},
    {"practice",            BIT_11, XX},
    {"npcprot",             BIT_12, XX},
    {"heal",                BIT_13, XX},
    {"adapt",               BIT_14, XX},
    {"undead",              BIT_15, XX},
    {"bank",                BIT_16, XX},
    {"no_body",             BIT_17, XX},
    {"hunter",              BIT_18, XX},
    {"no_mind",             BIT_19, XX},
    {"no_dispel",           BIT_20, XX},
    {"rewield",             BIT_21, XX},
    {"reequip",             BIT_22, XX},
    {"noassist",            BIT_23, XX},
    {"rand_target",         BIT_24, XX},
    {"safe",                BIT_25, XX},
    {"solo",                BIT_26, XX},
    {"nolifesteal",         BIT_27, XX},
    {"mount",               BIT_28, XX},
    {"nomindsteal",         BIT_29, XX},
    {"always_aggr",         BIT_30, XX},
    {"no_rescue",           BIT_31, XX},
    {NULL,                  XX,     XX}
};

const struct lookup_type tab_pc_act_flags[] = {
    {"nada",                BIT_0,  XX},
    {"is_npc",              BIT_1,  XX},
    {"bought_pet",          BIT_2,  XX},
    {"clan_leader",         BIT_3,  XX},
    {"autoexit",            BIT_4,  XX},
    {"autoloot",            BIT_5,  XX},
    {"autosac",             BIT_6,  XX},
    {"blank",               BIT_7,  XX},
    {"brief",               BIT_8,  XX},
    {"nopray",              BIT_9,  XX},
    {"combine",             BIT_10, XX},
    {"prompt",              BIT_11, XX},
    {"telnet_ga",           BIT_12, XX},
    {"holylight",           BIT_13, XX},
    {"wizinvis",            BIT_14, XX},
    {"builder",             BIT_15, XX},
    {"silence",             BIT_16, XX},
    {"noemote",             BIT_17, XX},
    {"colour",              BIT_18, XX},
    {"notell",              BIT_19, XX},
    {"log",                 BIT_20, XX},
    {"deny",                BIT_21, XX},
    {"freeze",              BIT_22, XX},
    {"thief",               BIT_23, XX},
    {"killer",              BIT_24, XX},
    {"nosummon",            BIT_25, XX},
    {"novisit",             BIT_26, XX},
    {"questing",            BIT_27, XX},
    {"autoassist",          BIT_29, XX},
    {NULL,                  XX,     XX}
};

/* New bits to handle how mobs act */

const struct lookup_type tab_mob_skill[] = {
    {"nada",                BIT_1,  90},
    {"2_attack",            BIT_2,  XX},
    {"3_attack",            BIT_3,  XX},
    {"4_attack",            BIT_4,  XX},
    {"punch",               BIT_5,  XX},
    {"headbutt",            BIT_6,  XX},
    {"knee",                BIT_7,  XX},
    {"disarm",              BIT_8,  XX},
    {"trip",                BIT_9,  XX},
    {"nodisarm",            BIT_10, XX},
    {"notrip",              BIT_11, XX},
    {"dodge",               BIT_12, XX},
    {"parry",               BIT_13, XX},
    {"martial",             BIT_14, XX},
    {"enhanced",            BIT_15, XX},
    {"dualwield",           BIT_16, XX},
    {"dirt",                BIT_17, XX},
    {"5_attack",            BIT_18, XX},
    {"6_attack",            BIT_19, XX},
    {"charge",              BIT_20, XX},
    {NULL,                  XX,     XX}
};

const struct lookup_type tab_mob_cast[] = {
    {"nada",                BIT_0,  90},
    {"placeholder",         BIT_1,  90},
    {"mag_missile",         BIT_2,  XX},
    {"shock_grasp",         BIT_3,  XX},
    {"burn_hands",          BIT_4,  XX},
    {"col_spray",           BIT_5,  XX},
    {"fireball",            BIT_6,  XX},
    {"hellspawn",           BIT_7,  XX},
    {"acid_blast",          BIT_8,  XX},
    {"chain_light",         BIT_9,  XX},
    {"faerie_fire",         BIT_10, XX},
    {"flare",               BIT_11, XX},
    {"flamestrike",         BIT_12, XX},
    {"earthquake",          BIT_13, XX},
    {"mind_flail",          BIT_14, XX},
    {"planergy",            BIT_15, XX},
    {"phobia",              BIT_16, XX},
    {"mind_bolt",           BIT_17, XX},
    {"static",              BIT_18, XX},
    {"ego_whip",            BIT_19, XX},
    {"bloody_tears",        BIT_20, XX},
    {"mindflame",           BIT_21, XX},
    {"suffocate",           BIT_22, XX},
    {"nerve_fire",          BIT_23, XX},
    {"light_bolt",          BIT_24, XX},
    {"heat_armor",          BIT_25, XX},
    {"lava_burst",          BIT_26, XX},
    {NULL,                  XX,     XX}
};

const struct lookup_type tab_cast_name[] = {
    {"nada",                    BIT_0,  90},
    {"placeholder",             BIT_1,  90},
    {"\'magic missile\'",       BIT_2,  XX},
    {"\'shocking grasp\'",      BIT_3,  XX},
    {"\'burning hands\'",       BIT_4,  XX},
    {"\'colour spray\'",        BIT_5,  XX},
    {"fireball",                BIT_6,  XX},
    {"hellspawn",               BIT_7,  XX},
    {"\'acid blast\'",          BIT_8,  XX},
    {"\'chain lightning\'",     BIT_9,  XX},
    {"\'faerie fire\'",         BIT_10, XX},
    {"flare",                   BIT_11, XX},
    {"flamestrike",             BIT_12, XX},
    {"earthquake",              BIT_13, XX},
    {"\'mind flail\'",          BIT_14, XX},
    {"planergy",                BIT_15, XX},
    {"phobia",                  BIT_16, XX},
    {"\'mind bolt\'",           BIT_17, XX},
    {"static",                  BIT_18, XX},
    {"\'ego whip\'",            BIT_19, XX},
    {"\'bloody tears\'",        BIT_20, XX},
    {"mindflame",               BIT_21, XX},
    {"suffocate",               BIT_22, XX},
    {"\'nerve fire\'",          BIT_23, XX},
    {"\'light bolt\'",          BIT_24, XX},
    {"\'heat armor\'",          BIT_25, XX},
    {"\'lava burst\'",          BIT_26, XX},
    {NULL,                      XX,     XX}
};

const struct lookup_type tab_mob_def[] = {
    {"nada",                BIT_1,  90},
    {"cure_light",          BIT_2,  XX},
    {"cure_serious",        BIT_3,  XX},
    {"cure_critic",         BIT_4,  XX},
    {"heal",                BIT_5,  XX},
    {"fireshield",          BIT_6,  XX},
    {"iceshield",           BIT_7,  XX},
    {"shockshield",         BIT_8,  XX},
    {"demonshield",         BIT_9,  XX},
    {"divine_int",          BIT_10, XX},
    {"cloak_absorb",        BIT_11, XX},
    {"cloak_reflect",       BIT_12, XX},
    {"cloak_flaming",       BIT_13, XX},
    {"shield_recast",       BIT_14, XX},
    {NULL,                  XX,     XX}
};

const struct lookup_type tab_affected_by[] = {
    {"nada",                    BIT_0,  90},
    {"blind",                   BIT_1,  XX},
    {"invisible",               BIT_2,  XX},
    {"detect_evil",             BIT_3,  XX},
    {"detect_invis",            BIT_4,  XX},
    {"detect_magic",            BIT_5,  XX},
    {"detect_hidden",           BIT_6,  XX},
    {"cloak:reflection",        BIT_7,  XX},
    {"sanctuary",               BIT_8,  XX},
    {"faerie_fire",             BIT_9,  XX},
    {"infrared",                BIT_10, XX},
    {"curse",                   BIT_11, XX},
    {"cloak:flaming",           BIT_12, XX},
    {"poison",                  BIT_13, XX},
    {"protect",                 BIT_14, XX},
    {"cloak:absorption",        BIT_15, XX},
    {"sneak",                   BIT_16, XX},
    {"hide",                    BIT_17, XX},
    {"sleep",                   BIT_18, XX},
    {"charm",                   BIT_19, XX},
    {"flying",                  BIT_20, XX},
    {"pass_door",               BIT_21, XX},
    {NULL,                      XX,     XX}
};

const struct lookup_type tab_obj_flags[] = {
    {"nada",                BIT_0,  90},
    {"glow",                BIT_1,  XX},
    {"hum",                 BIT_2,  XX},
    {"nodisarm",            BIT_3,  XX},
    {"lock",                BIT_4,  XX},
    {"evil",                BIT_5,  XX},
    {"invis",               BIT_6,  XX},
    {"magic",               BIT_7,  XX},
    {"nodrop",              BIT_8,  XX},
    {"bless",               BIT_9,  XX},
    {"anti_good",           BIT_10, XX},
    {"anti_evil",           BIT_11, XX},
    {"anti_neutral",        BIT_12, XX},
    {"noremove",            BIT_13, XX},
    {"inventory",           BIT_14, XX},
    {"nosave",              BIT_15, XX},
    {"claneq",              BIT_16, 90},
    {"trigger:destroy",     BIT_17, XX},
    {"no_auction",          BIT_18, XX},
    {"remort",              BIT_19, XX},
    {"adept",               BIT_20, XX},
    {"rare",                BIT_21, XX},
    {"nodispel",            BIT_22, XX},
    {"noloot",              BIT_23, XX},
    {"nosac",               BIT_24, XX},
    {"unique",              BIT_25, XX},
    {"lifestealer",         BIT_26, XX},
    {"mindstealer",         BIT_28, XX},
    {"nosell",              BIT_29, XX},
    {"nodestroy",           BIT_30, XX},
    {"nosteal",             BIT_31, XX},
    {NULL,                  XX,     XX}
};

const struct lookup_type tab_wear_flags[] = {
    {"take",        BIT_1,  XX},
    {"finger",      BIT_2,  XX},
    {"neck",        BIT_3,  XX},
    {"body",        BIT_4,  XX},
    {"head",        BIT_5,  XX},
    {"legs",        BIT_6,  XX},
    {"feet",        BIT_7,  XX},
    {"hands",       BIT_8,  XX},
    {"arms",        BIT_9,  XX},
    {"shield",      BIT_10, XX},
    {"about",       BIT_11, XX},
    {"waist",       BIT_12, XX},
    {"wrist",       BIT_13, XX},
    {"wield",       BIT_14, XX},
    {"hold",        BIT_15, XX},
    {"face",        BIT_16, XX},
    {"ear",         BIT_17, XX},
    {"clutch",      BIT_18, XX},
    {"wield2",      BIT_19, XX},
    {"claneq",      BIT_20, 90},
    {NULL,          XX,     XX}
};

const struct lookup_type tab_item_apply[] = {
    {"nada",            BIT_1,  90},
    {"infra",           BIT_2,  XX},
    {"invis",           BIT_3,  XX},
    {"det_invis",       BIT_4,  XX},
    {"sanc",            BIT_5,  XX},
    {"sneak",           BIT_6,  XX},
    {"hide",            BIT_7,  XX},
    {"prot",            BIT_8,  XX},
    {"enhanced",        BIT_9,  XX},
    {"det_magic",       BIT_10, XX},
    {"det_hidden",      BIT_11, XX},
    {"det_evil",        BIT_12, XX},
    {"pass_door",       BIT_13, XX},
    {"det_poison",      BIT_14, XX},
    {"fly",             BIT_15, XX},
    {"know_align",      BIT_16, XX},
    {"det_undead",      BIT_17, XX},
    {"heated",          BIT_18, XX},
    {"arenaheated",     BIT_19, 90},
    {NULL,              XX,     XX}
};

const struct lookup_type tab_wear_loc[] = {
    {"light",            0, XX},
    {"finger_l",         1, XX},
    {"finger_r",         2, XX},
    {"neck_1",           3, XX},
    {"neck_2",           4, XX},
    {"body",             5, XX},
    {"head",             6, XX},
    {"legs",             7, XX},
    {"feet",             8, XX},
    {"hands",            9, XX},
    {"arms",            10, XX},
    {"shield",          11, XX},
    {"about",           12, XX},
    {"waist",           13, XX},
    {"wrist_l",         14, XX},
    {"wrist_r",         15, XX},
    {"wield",           16, XX},
    {"hold",            17, XX},
    {"face",            18, XX},
    {"ear",             19, XX},
    {"clutch",          20, XX},
    {"wield2",          21, XX},
    {"claneq",          22, 90},
    {"max_wear",        23, 90},
    {NULL,              XX, XX}
};

const struct lookup_type tab_obj_aff[] = {
    {"nada",                 0, 90},
    {"str",                  1, XX},
    {"dex",                  2, XX},
    {"int",                  3, XX},
    {"wis",                  4, XX},
    {"con",                  5, XX},
    {"sex",                  6, XX},
    {"class",                7, XX},
    {"level",                8, XX},
    {"age",                  9, XX},
    {"height",              10, XX},
    {"weight",              11, XX},
    {"mana",                12, XX},
    {"hit",                 13, XX},
    {"move",                14, XX},
    {"gold",                15, XX},
    {"exp",                 16, XX},
    {"ac",                  17, XX},
    {"hitroll",             18, XX},
    {"damroll",             19, XX},
    {"saving_para",         20, XX},
    {"saving_rod",          21, XX},
    {"saving_petri",        22, XX},
    {"saving_breath",       23, XX},
    {"saving_spell",        24, XX},
    {NULL,                  XX, XX}
};

const struct lookup_type tab_room_flags[] = {
    {"nada",                BIT_0,  90},
    {"dark",                BIT_1,  XX},
    {"regen",               BIT_2,  XX},
    {"no_mob",              BIT_3,  XX},
    {"indoors",             BIT_4,  XX},
    {"no_magic",            BIT_5,  XX},
    {"hot",                 BIT_6,  XX},
    {"cold",                BIT_7,  XX},
    {"pk",                  BIT_8,  XX},
    {"quiet",               BIT_9,  XX},
    {"private",             BIT_10, XX},
    {"safe",                BIT_11, XX},
    {"solitary",            BIT_12, XX},
    {"pet_shop",            BIT_13, 90},
    {"no_recall",           BIT_14, XX},
    {"no_teleport",         BIT_15, XX},
    {"hunt_hunt",           BIT_16, 90},
    {"no_charm",            BIT_17, XX},
    {"no_portal",           BIT_18, XX},
    {"no_repop",            BIT_19, XX},
    {"no_quit",             BIT_20, XX},
    {"anti_portal",         BIT_21, XX},
    {"arena",               BIT_22, XX},
    {"arena2",              BIT_23, XX},
    {NULL,                  XX,     XX}
};

const struct lookup_type tab_sector_types[] = {
    {"nada",                 0, XX},
    {"inside",               0, XX},
    {"city",                 1, XX},
    {"field",                2, XX},
    {"forest",               3, XX},
    {"hills",                4, XX},
    {"mountain",             5, XX},
    {"water_swim",           6, XX},
    {"water_noswim",         7, XX},
    {"recall_set",           8, XX},
    {"air",                  9, XX},
    {"desert",              10, XX},
    {"max",                 11, XX},
    {NULL,                  XX, XX}
};

const struct lookup_type tab_door_types[] = {
    {"door",            BIT_1,  XX},
    {"closed",          BIT_2,  XX},
    {"locked",          BIT_3,  XX},
    {"climb",           BIT_4,  XX},
    {"immortal",        BIT_5,  90},
    {"pickproof",       BIT_6,  XX},
    {"smashproof",      BIT_7,  XX},
    {"passproof",       BIT_8,  XX},
    {"nodetect",        BIT_9,  XX},
    {"realmlord",       BIT_10, 90},
    {NULL,              XX,     XX}
};

const struct lookup_type tab_door_states[] = {
    {"open",     0, XX},
    {"closed",   1, XX},
    {"locked",   2, XX},
    {NULL,      XX, XX}
};

const struct lookup_type tab_player_flags[] = {
    {"nada",            BIT_0,  90},
    {"pkok",            BIT_1,  XX},
    {"afk",             BIT_2,  XX},
    {"diplomat",        BIT_5,  XX},
    {"boss",            BIT_6,  XX},
    {"treasurer",       BIT_7,  XX},
    {"armorer",         BIT_8,  XX},
    {"leader",          BIT_9,  XX},
    {"no_body",         BIT_16, XX},
    {"debug",           BIT_17, XX},
    {"specialname",     BIT_18, XX},
    {"xafk",            BIT_19, XX},
    {"mainleader",      BIT_20, XX},
    {"deserter",        BIT_21, XX},
    {"leaver",          BIT_22, XX},
    {"trusted",         BIT_23, XX},
    {"safe",            BIT_24, XX},
    {NULL, 0}
};

/* START:
 * mpedit addition
 */

const struct lookup_type tab_mprog_types[] = {
    {"nada",        0,                  90},
    {"in_file",     IN_FILE_PROG,       XX},
    {"act",         ACT_PROG,           XX},
    {"speech",      SPEECH_PROG,        XX},
    {"rand",        RAND_PROG,          XX},
    {"fight",       FIGHT_PROG,         XX},
    {"hitprcnt",    HITPRCNT_PROG,      XX},
    {"death",       DEATH_PROG,         XX},
    {"entry",       ENTRY_PROG,         XX},
    {"greet",       GREET_PROG,         XX},
    {"allgreet",    ALL_GREET_PROG,     XX},
    {"give",        GIVE_PROG,          XX},
    {"bribe",       BRIBE_PROG,         XX},
    {NULL,          XX,                 XX}
};

/* FINISH:
 * mpedit addition
 */

const struct lookup_type tab_locker_types[] = {
    {"none",    0,              XX},
    {"saveall", LOCKER_SAVEALL, XX},
    {NULL,      XX,             XX}
};

const struct lookup_type tab_duel_types[] = {
/*  { "charm",      DUEL_CHARM,     XX}, TODO */
    { "nomagic",    DUEL_NOMAGIC,   XX},
/*  { "noobj",      DUEL_NOOBJ,     XX}, TODO */
    { "dbldam",     DUEL_DBLDAM,    XX},
    { "dblheal",    DUEL_DBLHEAL,   XX},
    { "supercast",  DUEL_SUPERCAST, XX},
    { "randstat",   DUEL_RANDSTAT,  XX},
    { "randrace",   DUEL_RANDRACE,  XX},
    { NULL,         XX,             XX}
};

/* Now for the functions */

unsigned long int
table_lookup(const struct lookup_type *table, char *name)
{
    int                 a;

    if (name[0] == '\0')
        return /* table[0].value-1 */ 0;

    for (a = 0; table[a].text != NULL; a++)
        if (!str_prefix(name, table[a].text))
            return (!str_cmp(table[a].text, "nada") || !str_cmp(table[a].text, "none")) ? 0 : table[a].value;
    return /* table[0].value-1  */ 0;
}

char               *
rev_table_lookup(const struct lookup_type *table, unsigned long int number)
{
    int                 a;

    for (a = 0; table[a].text != NULL; a++)
        if (table[a].value == number)
            return table[a].text;
    return "";
}

int
level_table_lookup(const struct lookup_type *table, unsigned long int number)
{
    int                 a;

    for (a = 0; table[a].text != NULL; a++)
        if (table[a].value == number)
            return table[a].level;

    return 0;
}

/* spec: fixed to not assume contiguous bit use */

char               *
bit_table_lookup(const struct lookup_type *table, unsigned long int number)
{
    int                 a;
    static char         buf[MAX_STRING_LENGTH];

    buf[0] = '\0';

    for (a = 0; number && table[a].text; a++) {
        if ((number & table[a].value) == table[a].value && str_cmp(table[a].text, "nada") && str_cmp(table[a].text, "placeholder") && str_cmp(table[a].text, "none")) {
            strcat(buf, table[a].text);
            strcat(buf, ", ");
            number &= ~table[a].value;
        }
    }

    if (buf[0] == '\0')
        strcat(buf, "none.");
    else {
        buf[strlen(buf) - 2] = '\0';
    }

    return buf;
}

char *bit_table_lookup2 (const struct lookup_type *table, unsigned long int number)
{
    int                 a;
    static char         buf[MAX_STRING_LENGTH];

    buf[0] = '\0';

    for (a = 0; number && table[a].text; a++) {
        if ((number & table[a].value) == table[a].value && str_cmp(table[a].text, "nada") && str_cmp(table[a].text, "placeholder") && str_cmp(table[a].text, "none")) {
            strcat(buf, "@@g");
            strcat(buf, table[a].text);
            strcat(buf, "@@d|");
            number &= ~table[a].value;
        }
    }

    if (buf[0] == '\0')
        buf[0] = '\0';
    else
        buf[strlen(buf) - 1] = '\0';

    return buf;
}

void
table_printout(const struct lookup_type *table, char *buf)
{
    int                 a;

    a = 0;
    buf[0] = '\0';

    for (a = 0; table[a].text != NULL; a++) {
        if (   strcmp(table[a].text, "nada")
            && strcmp(table[a].text, "placeholder")
            && strcmp(table[a].text, "none")
           ) {    /* If not an invalid choice */
            strcat(buf, "          ");
            strcat(buf, table[a].text);
            strcat(buf, "\n\r");
        }
    }

    return;
}

void
wide_table_printout(const struct lookup_type *table, char *buf)
{
    /* Like table_printout, but formats into columns */

    char                tmp[MAX_STRING_LENGTH];
    int                 a;
    int                 foo;    /* work out how many values shown in a row */

    a = 0;
    foo = 0;
    buf[0] = '\0';

    for (a = 0; table[a].text != NULL; a++) {
        if ((strcmp(table[a].text, "nada"))    /* If not an invalid choice */
            &&(strcmp(table[a].text, "none"))) {
            sprintf(tmp, " %15s", table[a].text);
            strcat(buf, tmp);
            if (++foo % 4 == 0)
                strcat(buf, "\n\r");
        }
    }
    strcat(buf, "\n\r");
    return;
}

char               *
show_values(const struct lookup_type *table, int value, bool fBit)
{

    char                tmp[MAX_STRING_LENGTH];
    static char         buf[MAX_STRING_LENGTH];
    int                 a;
    int                 foo;    /* work out how many values shown in a row */

    a = 0;
    foo = 0;
    buf[0] = '\0';

    for (a = 0; table[a].text != NULL; a++) {
        if ((strcmp(table[a].text, "nada"))    /* If not an invalid choice */
            &&(strcmp(table[a].text, "placeholder"))
            &&(strcmp(table[a].text, "none"))) {
            strcat(buf, "     ");
            sprintf(tmp, "%s%-13s",
                fBit ? (IS_SET(value, table[a].value) ? "@@y*" : "@@g ") : (value == table[a].value ? "@@y*" : "@@g "), table[a].text);
            strcat(buf, tmp);
            if (++foo % 4 == 0)
                strcat(buf, "\n\r");
        }
    }
    strcat(buf, "@@g\n\r");
    return (buf);
}
