
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
 *                                                                         *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include "merc.h"

IDSTRING(rcsid, "$Id: const.c,v 1.42 2004/11/07 08:12:15 dave Exp $");

/*
 * Colour table for say, gossip, shout, etc.
 * Put here for ease of editing. -S-
 */
const struct colour_type colour_table[MAX_COLOUR] = {
    {"say",      0},
    {"tell",     1},
    {"gossip",   2},
    {"auction",  3},
    {"music",    4},
    {"shout",    5},
    {"yell",     6},
    {"clan",     7},
    {"race",     8},
    {"flame",    9},
    {"info",    10},
    {"stats",   11},
    {"rooms",   12},
    {"objects", 13},
    {"mobiles", 14},
    {"ooc",     15},
    {"gtell",   16},
    {"hunt",    17},
    {"unused",  18},
    {"unused",  19},
    {"unused",  20},
    {"unused",  21},
    {"unused",  22},
    {"unused",  23},
    {"unused",  24},
    {"unused",  25},
    {"unused",  26},
    {"unused",  27},
    {"unused",  28},
    {"unused",  29},
};

const struct ansi_type ansi_table[MAX_ANSI] = {
    {"gray",         "\033[1;37m",  0, 'g', 7},
    {"red",          "\033[0;31m",  1, 'R', 7},
    {"green",        "\033[0;32m",  2, 'G', 7},
    {"brown",        "\033[0;33m",  3, 'b', 7},
    {"blue",         "\033[0;34m",  4, 'B', 7},
    {"magenta",      "\033[0;35m",  5, 'm', 7},
    {"cyan",         "\033[0;36m",  6, 'c', 7},
    {"black",        "\033[0;30m",  7, 'k', 7},
    {"yellow",       "\033[1;33m",  8, 'y', 7},
    {"white",        "\033[0;37m",  9, 'W', 7},
    {"normal",       "\033[0;0m",  10, 'N', 6},
    {"purple",       "\033[1;35m", 11, 'p', 7},
    {"dark_grey",    "\033[1;30m", 12, 'd', 7},
    {"light_blue",   "\033[1;34m", 13, 'l', 7},
    {"light_green",  "\033[1;32m", 14, 'r', 7},
    {"light_cyan",   "\033[1;36m", 15, 'a', 7},
    {"light_red",    "\033[1;31m", 16, 'e', 7},
    {"bold",         "\033[1m",    17, 'x', 4},
    {"flashing",     "\033[5m",    18, 'f', 4},
    {"inverse",      "\033[7m",    19, 'i', 4},
    {"back_red",     "\033[0;41m", 20, '2', 7},
    {"back_green",   "\033[0;42m", 21, '3', 7},
    {"back_yellow",  "\033[0;43m", 22, '4', 7},
    {"back_blue",    "\033[0;44m", 23, '1', 7},
    {"back_magenta", "\033[0;45m", 24, '5', 7},
    {"back_cyan",    "\033[0;46m", 25, '6', 7},
    {"back_black",   "\033[0;40m", 26, '0', 7},
    {"back_white",   "\033[1;47m", 27, '7', 7},
    {"underline",    "\033[4m",    28, 'u', 4}
};

/*
 * Class table.
 */
const struct class_type class_table[MAX_CLASS] = {
    {
            "Mag", "Mage", APPLY_INT, "Int", 0,
            1107, 90, 18, 10, 1, 2, TRUE, "faerie fire",
    },
    {
            "Cle", "Cleric", APPLY_WIS, "Wis", 0,
            1105, 90, 18, 12, 2, 2, TRUE, "bless"
    },
    {
            "Thi", "Thief", APPLY_DEX, "Dex", 0,
            1106, 90, 18, 8, 1, 3, FALSE, "steal"
    },
    {
            "War", "Warrior", APPLY_STR, "Str", 0,
            1108, 90, 18, 6, 3, 4, FALSE, "punch"
    },
    {
            "Psi", "Psionicist", APPLY_INT, "Int", 0,
            1125, 90, 18, 10, 2, 3, TRUE, "mind flail"
    }
};

/* Table for remort classes.... same format as class_table 
 * Note that alot of stuff is not needed... 
 */

const struct class_type remort_table[MAX_CLASS] = {
    {
            "Sor", "Sorcerer", APPLY_INT, "Int", 0,
            /* guild room -> */ 0, 0, 0, 0, 2, 3, TRUE, ""
    },
    {
            "Ass", "Assassin", APPLY_DEX, "Dex", 0,
            /* guild room -> */ 0, 0, 0, 0, 2, 3, FALSE, ""
    },
    {
            "Kni", "Knight", APPLY_STR, "Str", 0,
            /* guild room -> */ 0, 0, 0, 0, 4, 4, FALSE, ""
    },
    {
            "Nec", "Necromancer", APPLY_WIS, "Wis", 0,
            /* guild room -> */ 0, 0, 0, 0, 2, 3, TRUE, ""
    },
    {
            "Mon", "Monk", APPLY_CON, "Con", 0,
            /* guild_room -> */ 0, 0, 0, 0, 2, 3, TRUE, ""
    }
};

/* clan name, clan symbol, cdon, clan home, boss name, blank, EQ */
const struct clan_type clan_table[MAX_CLAN] = {
    {
            "None", "None ", 0, 0,
            "N/A", " ", {50, 51, 52, 53, 54, -1}
    },
    {
            "@@dS@@Go@@gc@@Wiety @@dof I@@Gs@@gm@@Wa@@G'@@Wili@@N", "@@d}@@G:@@Wo@@G:@@d{@@N",
            3691, 3689,
            "Jacobin/Karrde", " ", {3653, 9963, 9964, 9965, 9966, -1}
    },
    {
            "@@yM@@cu@@ad@@cd@@ae@@cr@@ys @@yA@@cn@@ao@@cn@@ay@@cm@@ao@@cu@@ys@@N", "@@r.@@cA@@aC@@cK@@r.@@N",
            0, 310,
            "Muffin", " ", {-1, -1, -1, -1}
    },
    {
            "@@dCh@@Bildren of Cadm@@dus@@N", "@@d<@@BC@@do@@BC@@d>@@N",
            4607, 4600,
            "Telchar", " ", {4604, 4619, 4620, 4621, 4622, -1}
    },
    {
            "@@RI@@en@@gt@@dr@@Ro@@ed@@gu@@dc@@Rt@@ei@@go@@dn @@Rt@@eo @@gM@@da@@Ry@@eh@@ge@@dm@@N",
            "@@R<@@e|@@g*@@e|@@R>@@N", 4902, 4900,
            "Riggs", " ", {4900, 4912, 4913, 4914, 4915, -1}
    },
    {
            "@@dThe @@cKnight @@dB@@gri@@Wg@@gad@@de@@N",
            "@@c`@@a'@@d-@@g|@@W|@@N", 461, 445,
            "Coen", " ", {451, 458, 459, 460, 461, -1}
    },
    {
            "@@mF@@pl@@gam@@pe@@ms@@d of the @@WR@@di@@gsi@@dn@@Wg @@RP@@eh@@yo@@ge@@yn@@ei@@Rx@@N",
            "@@m^@@p\\@@e'@@p/@@m^@@N", 9958, 9950,
            "Hinata/Converge", " ", {9952, 9959, 9960, 9961, 9962, -1}
    },
    {
            "@@GT@@dhe @@RB@@drother@@GH@@dood@@N", "@@G(@@R\\@@dm@@R/@@G)@@N", 10744, 10697,
            "Kidney", " ", {10720, 10721, 10722, 10723, 10724}
    },
    {
            "@@WV@@ga@@driance @@gof @@WC@@go@@dnviction@@N",
            "@@d:@@g[@@W-@@g]@@d:@@N", 11650, 11631,
            "Norritt", " ", {11654, 11655, 11656, 11657, 11658}
    },
};

const struct race_type race_table[MAX_RACE] = {
    {
            /* 0 */
            "Hmn", "Human", 3001, 0,
            19, 18, 18, 19, 17,
            RACE_MOD_NONE,
            5, {4, 5, 3, 1, 2},
            "War, Thi/Psi,  Mag, Cle", ""
    },
    {
            /* 1 */
            "Hob", "Hobbit", 3001, 0,
            13, 17, 18, 21, 18,
            RACE_MOD_SMALL | RACE_MOD_WOODLAND | RACE_MOD_RESIST_SPELL,
            5, {2, 3, 1, 4, 5},
            "Thi, Mag, Cle, War, Psi", "steal \'pick lock\'"
    },
    {
            /* 2 */
            "Dwf", "Dwarf", 3001, 0,
            18, 13, 17, 14, 20,
            RACE_MOD_WEAK_MAGIC | RACE_MOD_RESIST_SPELL | RACE_MOD_SMALL | RACE_MOD_IMMUNE_POISON,
            5, {5, 2, 4, 1, 3},
            "War, Cle, Psi, Thi, Mag", "smash"
    },
    {
            /* 3 */
            "Elf", "Elf", 9201, 0,
            14, 19, 14, 18, 14,
            RACE_MOD_SMALL | RACE_MOD_WOODLAND,
            5, {1, 5, 2, 3, 4},
            "Mag, Thi, War, Psi, Cle", "\'find doors\'"
    },
    {
            /* 4 */
            "Gno", "Gnome", 6015, 0,
            13, 19, 17, 20, 16,
            RACE_MOD_TINY | RACE_MOD_IMMUNE_POISON | RACE_MOD_DARKNESS,
            5, {3, 1, 4, 5, 2},
            "Cle, Psi, Mag, Thi, War", "\'faerie fire\' sneak"
    },
    {
            /* 5 */
            "Ogr", "Ogre", 3430, 0,
            21, 10, 12, 15, 20,
            RACE_MOD_WEAK_MAGIC | RACE_MOD_RESIST_SPELL | RACE_MOD_LARGE,
            5, {2, 4, 3, 1, 5},
            "War, Mag, Thi, Cle, Psi", "\'shield block\'"
    },
    {
            /* 6 */
            "Drw", "Drow", 9201, 0,
            15, 19, 12, 20, 15,
            RACE_MOD_STRONG_MAGIC | RACE_MOD_SLOW_HEAL | RACE_MOD_DARKNESS,
            5, {1, 5, 4, 3, 2},
            "Psi, Mag, War, Thi, Cle", "\'find doors\' \'mind bolt\'"
    },
    {
            /* 7 */
            "Lam", "Lamia", 3001, 0,
            19, 13, 11, 20, 14,
            RACE_MOD_LARGE | RACE_MOD_IMMUNE_POISON | RACE_MOD_TAIL,
            5, {4, 5, 1, 2, 3},
            "Thi, War, Psi, Mag, Cle", "dodge"
    },
    {
            /* 8 */
            "Drg", "Dragon", 3001, 0,
            20, 19, 11, 12, 17,
            RACE_MOD_STRONG_MAGIC | RACE_MOD_SLOW_HEAL | RACE_MOD_HUGE | RACE_MOD_TAIL,
            5, {1, 2, 5, 3, 4},
            "Mag, Cle, War, Psi, Thi", "fly \'fire breath\'"
    },
    {
            /* 9 */
            "Cen", "Centaur", 3001, 0,
            19, 14, 17, 11, 18,
            RACE_MOD_LARGE | RACE_MOD_WOODLAND,
            5, {2, 4, 3, 1, 5},
            "War/Cle,  Thi, Mag, Psi", "hunt"
    },
    {
            /* 10 */
            "Ttn", "Titan", 3001, 0,
            21, 18, 10, 12, 21,
            RACE_MOD_NO_MAGIC | RACE_MOD_HUGE | RACE_MOD_RESIST_SPELL,
            5, {3, 4, 5, 1, 2},
            "War, Psi, Mag, Cle, Thi", "\'enhanced damage\'"
    },
    {
            /* 11 */
            "Pix", "Pixie", 3001, 0,
            13, 20, 15, 22, 12,
            RACE_MOD_STRONG_MAGIC | RACE_MOD_TINY | RACE_MOD_RESIST_SPELL,
            5, {3, 4, 2, 5, 1},
            "Psi, Thi, Mag, Cle, War", "fly"
    },
    {
            /* 12 */
            "Min", "Minotaur", 3001, 0,
            21, 12, 11, 16, 21,
            RACE_MOD_LARGE | RACE_MOD_DARKNESS,
            5, {2, 3, 5, 1, 4},
            "War, Mag, Cle, Psi, Thi", "\'enhanced damage\'"
    },
    {
            /* 13 */
            "Trl", "Troll", 3001, 0,
            20, 11, 11, 15, 21,
            RACE_MOD_FAST_HEAL | RACE_MOD_WEAK_MAGIC | RACE_MOD_LARGE | RACE_MOD_DARKNESS,
            5, {2, 3, 5, 1, 4},
            "War, Cle, Thi, Psi, Mag", "\'stone skin\'"
    }
};

/* these are the base mods for the various stances.  They are designed to be
 * multipliers for get_pseudo_level/10 ( 20 - get_pseudo_level/10, for bad effects),
 * so don't make them too big :)
 */

/* name, ac_mod, dr_mod, hr_mod, speed_mod   */

const struct stance_app_type stance_app[MAX_STANCE] = {
    {"Warrior",   0,  0,  0,  0},
    {"Mage",     20, -3, -3,  0},
    {"Ninja",   -30,  0,  5,  3},
    {"Shadows", -20, -1, -2,  4},
    {"Essence",   0,  0,  3,  0},
    {"Beast",     0,  3,  0,  0},
    {"Flame",    10,  3,  3,  1},
    {"Spirit",  -10,  3, -2,  1},
    {"Void",    -10, -2,  3,  2},
    {"Dragon",  -10,  2,  2, -1},
    {"Snake",     7, -3, -2,  4},
};

const struct exp_type exp_table[141] = {
    {     0, {   1000,    1000,    1000,    1000,    1000}}, /*  0 */
    {   100, {   3050,    3023,    3022,    3021,    3020}}, /*  1 */
    {   200, {   8081,    8080,    8084,    8083,    8082}}, /*  2 */
    {   300, {  11183,   11182,   11181,   11184,   11185}}, /*  3 */
    {   450, {  20326,   20322,   20323,   20324,   20325}}, /*  4 */
    {   600, {  35504,   35505,   35506,   35507,   35503}}, /*  5 */
    {   850, {  38350,   38725,   38726,   38727,   38728}}, /*  6 */
    {  1080, {  54000,   53000,   54500,   53500,   53800}}, /*  7 */
    {  1300, {  65000,   65500,   64500,   65900,   64300}}, /*  8 */
    {  1550, {  77500,   77600,   77000,   78000,   77200}}, /*  9 */
    {  1800, { 108000,  108500,  109000,  107500,  107000}}, /* 10 */
    {  2100, { 126000,  126500,  125500,  125000,  127000}}, /* 11 */
    {  2450, { 147000,  148000,  146000,  146500,  147500}}, /* 12 */
    {  2800, { 168000,  167500,  168500,  169000,  167000}}, /* 13 */
    {  3150, { 189000,  188000,  190000,  188500,  189500}}, /* 14 */
    {  3500, { 210000,  211000,  205000,  210500,  210000}}, /* 15 */
    {  3900, { 234000,  233000,  234500,  235000,  239000}}, /* 16 */
    {  4350, { 261000,  262000,  261500,  260000,  260500}}, /* 17 */
    {  4800, { 288000,  287000,  287500,  289000,  288000}}, /* 18 */
    {  5300, { 318000,  317000,  317500,  318500,  319000}}, /* 19 */
    {  5800, { 348000,  347000,  349000,  348500,  347500}}, /* 20 */
    {  6350, { 381000,  382000,  380000,  380500,  381500}}, /* 21 */
    {  6950, { 417000,  416500,  417500,  418000,  418500}}, /* 22 */
    {  7550, { 453000,  452000,  454000,  453500,  452500}}, /* 23 */
    {  8200, { 492000,  491000,  493000,  492500,  491500}}, /* 24 */
    {  8800, { 528000,  527000,  528500,  529000,  527500}}, /* 25 */
    {  9500, { 570000,  575000,  565000,  560000,  580000}}, /* 26 */
    { 10200, { 612000,  611000,  613000,  612500,  611500}}, /* 27 */
    { 11000, { 660000,  655000,  665000,  670000,  650000}}, /* 28 */
    { 11900, { 714000,  715000,  714500,  713500,  714000}}, /* 29 */
    { 13000, { 780000,  785000,  790000,  770000,  775000}}, /* 30 */
    { 14000, { 840000,  841000,  840500,  841500,  840500}}, /* 31 */
    { 15300, { 918000,  917500,  918500,  919000,  918200}}, /* 32 */
    { 16500, { 990000,  985000,  987000,  995000,  993000}}, /* 33 */
    { 18000, {1080000, 1090000, 1075000, 1085000, 1087000}}, /* 34 */
    { 19500, {1170000, 1165000, 1175000, 1180000, 1160000}}, /* 35 */
    { 22000, {1320000, 1315000, 1330000, 1325000, 1320000}}, /* 36 */
    { 24500, {1470000, 1465000, 1475000, 1477000, 1460000}}, /* 37 */
    { 27500, {1650000, 1660000, 1655000, 1657000, 1645000}}, /* 38 */
    { 30000, {1800000, 1790000, 1810000, 1815000, 1795000}}, /* 39 */
    { 33000, {1980000, 1975000, 1985000, 1983000, 1977000}}, /* 40 */
    { 34000, {2040000, 2035000, 2045000, 2039000, 2050000}}, /* 41 */
    { 35500, {2130000, 2135000, 2140000, 2125000, 2133000}}, /* 42 */
    { 37500, {2250000, 2240000, 2255000, 2260000, 2245000}}, /* 43 */
    { 40000, {2400000, 2450000, 2480000, 2350000, 2390000}}, /* 44 */
    { 43000, {2580000, 2570000, 2585000, 2589000, 2575000}}, /* 45 */
    { 46000, {2760000, 2755000, 2765000, 2770000, 2763000}}, /* 46 */
    { 49500, {2970000, 2975000, 2980000, 2965000, 2973000}}, /* 47 */
    { 52000, {3120000, 3115000, 3125000, 3200000, 3128000}}, /* 48 */
    { 55000, {3300000, 3290000, 3310000, 3295000, 3300000}}, /* 49 */
    { 56000, {3360000, 3355000, 3365000, 3370000, 3368000}}, /* 50 */
    { 58000, {3480000, 3470000, 3485000, 3488000, 3475000}}, /* 51 */
    { 60000, {3600000, 3610000, 3590000, 3605000, 3640000}}, /* 52 */
    { 63000, {3780000, 3790000, 3770000, 3775000, 3785000}}, /* 53 */
    { 66000, {3960000, 3940000, 3970000, 3965000, 3962000}}, /* 54 */
    { 68000, {4080000, 4095000, 4085000, 4083000, 4073000}}, /* 55 */
    { 71000, {4260000, 4250000, 4265000, 4255000, 4300000}}, /* 56 */
    { 73000, {4380000, 4390000, 4385000, 4375000, 4387000}}, /* 57 */
    { 76000, {4560000, 4570000, 4550000, 4555000, 4565000}}, /* 58 */
    { 79000, {4740000, 4750000, 4745000, 4755000, 4760000}}, /* 59 */
    { 81000, {4860000, 4840000, 4850000, 4855000, 4870000}}, /* 60 */
    { 83000, {4980000, 4990000, 4975000, 4985000, 4990000}}, /* 61 */
    { 85000, {5100000, 5110000, 5150000, 5090000, 5095000}}, /* 62 */
    { 88000, {5280000, 5270000, 5285000, 5290000, 5275000}}, /* 63 */
    { 89000, {5345000, 5350000, 5360000, 5350000, 5343000}}, /* 64 */
    { 91000, {5460000, 5470000, 5450000, 5465000, 5456000}}, /* 65 */
    { 93000, {5580000, 5560000, 5590000, 5575000, 5590000}}, /* 66 */
    { 94000, {5640000, 5650000, 5630000, 5640000, 5620000}}, /* 67 */
    { 95000, {5700000, 5710000, 5690000, 5720000, 5680000}}, /* 68 */
    { 97000, {5820000, 5800000, 5830000, 5820000, 5810000}}, /* 69 */
    { 98000, {5880000, 5820000, 5900000, 5860000, 5870000}}, /* 70 */
    { 99000, {5940000, 5920000, 5980000, 5960000, 5930000}}, /* 71 */
    { 99500, {5970000, 5980000, 6000000, 5800000, 5970000}}, /* 72 */
    {100000, {6000000, 6100000, 5950000, 6150000, 6050000}}, /* 73 */
    {103000, {6180000, 6190000, 6170000, 6190000, 6185000}}, /* 74 */
    {106000, {6360000, 6350000, 6400000, 6340000, 6360000}}, /* 75 */
    {110000, {6600000, 6610000, 6620000, 6590000, 6600000}}, /* 76 */
    {115000, {6910000, 6920000, 6900000, 6850000, 6900000}}, /* 77 */
    {120000, {7200000, 7250000, 7230000, 7190000, 7200000}}, /* 78 */
    {130000, {7800000, 7700000, 7750000, 7780000, 7850000}}, /* 79 */
    {140000, {8000000, 8000000, 8000000, 8000000, 8000000}}, /* 80 */
    {150000, {6600000, 6610000, 6620000, 6590000, 6600000}}, /* 81 */
    {175000, {6910000, 6920000, 6900000, 6850000, 6900000}}, /* 82 */
    {190000, {7200000, 7250000, 7230000, 7190000, 7200000}}, /* 83 */
    {200000, {7800000, 7700000, 7750000, 7780000, 7850000}}, /* 84 */
    {210000, {8000000, 8000000, 8000000, 8000000, 8000000}}, /* 85 */
    {220000, {8000000, 8000000, 8000000, 8000000, 8000000}}, /* 86 */
    {235000, {6600000, 6610000, 6620000, 6590000, 6600000}}, /* 87 */
    {255000, {6910000, 6920000, 6900000, 6850000, 6900000}}, /* 88 */
    {260000, {7200000, 7250000, 7230000, 7190000, 7200000}}, /* 89 */
    {270000, {7800000, 7700000, 7750000, 7780000, 7850000}}, /* 90 */
    {280000, {8000000, 8000000, 8000000, 8000000, 8000000}}, /* 91 */
    {300000, {8000000, 8000000, 8000000, 8000000, 8000000}}, /* 92 */
    {310000, {6600000, 6610000, 6620000, 6590000, 6600000}}, /* 93 */
    {315000, {6910000, 6920000, 6900000, 6850000, 6900000}}, /* 94 */
    {320000, {7200000, 7250000, 7230000, 7190000, 7200000}}, /* 95 */
    {330000, {7800000, 7700000, 7750000, 7780000, 7850000}}, /* 96 */
    {340000, {8000000, 8000000, 8000000, 8000000, 8000000}}, /* 97 */
    {340000, {8000000, 8000000, 8000000, 8000000, 8000000}}, /* 98 */
    {350000, {6600000, 6610000, 6620000, 6590000, 6600000}}, /* 99 */
    {355000, {6910000, 6920000, 6900000, 6850000, 6900000}}, /* 100 */
    {350000, {7200000, 7250000, 7230000, 7190000, 7200000}}, /* 101 */
    {350000, {7800000, 7700000, 7750000, 7780000, 7850000}}, /* 102 */
    {350000, {8000000, 8000000, 8000000, 8000000, 8000000}}, /* 103 */
    {365000, {6910000, 6920000, 6900000, 6850000, 6900000}}, /* 104 */
    {360000, {7200000, 7250000, 7230000, 7190000, 7200000}}, /* 105 */
    {360000, {7800000, 7700000, 7750000, 7780000, 7850000}}, /* 106 */
    {360000, {8000000, 8000000, 8000000, 8000000, 8000000}}, /* 107 */
    {360000, {7200000, 7250000, 7230000, 7190000, 7200000}}, /* 108 */
    {360000, {7800000, 7700000, 7750000, 7780000, 7850000}}, /* 109 */
    {360000, {8000000, 8000000, 8000000, 8000000, 8000000}}, /* 110 */
    {370000, {7200000, 7250000, 7230000, 7190000, 7200000}}, /* 111 */
    {370000, {7800000, 7700000, 7750000, 7780000, 7850000}}, /* 112 */
    {370000, {8000000, 8000000, 8000000, 8000000, 8000000}}, /* 113 */
    {375000, {6910000, 6920000, 6900000, 6850000, 6900000}}, /* 114 */
    {388000, {7200000, 7250000, 7230000, 7190000, 7200000}}, /* 115 */
    {380000, {7800000, 7700000, 7750000, 7780000, 7850000}}, /* 116 */
    {380000, {8000000, 8000000, 8000000, 8000000, 8000000}}, /* 117 */
    {380000, {7200000, 7250000, 7230000, 7190000, 7200000}}, /* 118 */
    {380000, {7800000, 7700000, 7750000, 7780000, 7850000}}, /* 119 */
    {380000, {8000000, 8000000, 8000000, 8000000, 8000000}}, /* 120 */
    {385000, {7800000, 7700000, 7750000, 7780000, 7850000}}, /* 121 */
    {385000, {8000000, 8000000, 8000000, 8000000, 8000000}}, /* 122 */
    {386000, {7200000, 7250000, 7230000, 7190000, 7200000}}, /* 123 */
    {387000, {7800000, 7700000, 7750000, 7780000, 7850000}}, /* 124 */
    {389000, {8000000, 8000000, 8000000, 8000000, 8000000}}, /* 125 */
    {390000, {7800000, 7700000, 7750000, 7780000, 7850000}}, /* 126 */
    {410000, {8000000, 8000000, 8000000, 8000000, 8000000}}, /* 127 */
    {421000, {7200000, 7250000, 7230000, 7190000, 7200000}}, /* 128 */
    {432000, {7800000, 7700000, 7750000, 7780000, 7850000}}, /* 129 */
    {444000, {8000000, 8000000, 8000000, 8000000, 8000000}}, /* 130 */
    {460000, {7800000, 7700000, 7750000, 7780000, 7850000}}, /* 131 */
    {500000, {8000000, 8000000, 8000000, 8000000, 8000000}}, /* 132 */
    {510000, {7200000, 7250000, 7230000, 7190000, 7200000}}, /* 133 */
    {520000, {7800000, 7700000, 7750000, 7780000, 7850000}}, /* 134 */
    {525000, {8000000, 8000000, 8000000, 8000000, 8000000}}, /* 135 */
    {530000, {7800000, 7700000, 7750000, 7780000, 7850000}}, /* 136 */
    {530000, {8000000, 8000000, 8000000, 8000000, 8000000}}, /* 137 */
    {535000, {7200000, 7250000, 7230000, 7190000, 7200000}}, /* 138 */
    {540000, {7800000, 7700000, 7750000, 7780000, 7850000}}, /* 139 */
    {550000, {8000000, 8000000, 8000000, 8000000, 8000000}}, /* 140 */
};

#define CLASS_MAGE       0
#define CLASS_CLERIC     1
#define CLASS_THIEF      2
#define CLASS_WARRIOR    3
#define CLASS_PSI        4
#define CLASS_CONJURER   5
#define CLASS_TEMPLAR    6        /* Yes, i _know_ it's now paladin :P */
#define CLASS_RANGER     7

/*
 * Attribute bonus tables.
 */
const struct str_app_type str_app[26] = {
    {-5, -4,   0,  0}, /*  0 */
    {-5, -4,   3,  1}, /*  1 */
    {-3, -2,   3,  2}, /*  2 */
    {-3, -1,  10,  3}, /*  3 */
    {-2, -1,  25,  4}, /*  4 */
    {-2, -1,  55,  5}, /*  5 */
    {-1,  0,  80,  6}, /*  6 */
    {-1,  0,  90,  7}, /*  7 */
    { 0,  0, 100,  8}, /*  8 */
    { 0,  0, 100,  9}, /*  9 */
    { 0,  0, 115, 10}, /* 10 */
    { 0,  0, 115, 11}, /* 11 */
    { 0,  0, 140, 12}, /* 12 */
    { 0,  0, 140, 13}, /* 13 */
    { 0,  1, 170, 14}, /* 14 */
    { 1,  1, 170, 15}, /* 15 */
    { 1,  2, 195, 16}, /* 16 */
    { 2,  3, 220, 22}, /* 17 */
    { 2,  4, 250, 25}, /* 18 */
    { 3,  5, 300, 30}, /* 19 */
    { 3,  6, 350, 35}, /* 20 */
    { 4,  7, 400, 40}, /* 21 */
    { 5,  7, 450, 45}, /* 22 */
    { 6,  8, 500, 50}, /* 23 */
    { 8, 10, 550, 55}, /* 24 */
    {10, 12, 600, 60}  /* 25 */
};

const struct int_app_type int_app[26] = {
    { 3, -50,  0}, /*  0 */
    { 6, -50,  0}, /*  1 */
    { 6, -50,  0}, /*  2 */
    { 7, -50,  0}, /*  3 */
    { 7, -50,  0}, /*  4 */
    { 8, -50,  0}, /*  5 */
    { 8, -40,  2}, /*  6 */
    { 9, -40,  2}, /*  7 */
    { 9, -40,  2}, /*  8 */
    {10, -35,  2}, /*  9 */
    {10, -35,  3}, /* 10 */
    {11, -30,  4}, /* 11 */
    {12, -30,  5}, /* 12 */
    {13, -20,  6}, /* 13 */
    {13, -20,  7}, /* 14 */
    {15, -15,  8}, /* 15 */
    {20,  -5,  9}, /* 16 */
    {23,   0, 10}, /* 17 */
    {25,  10, 10}, /* 18 */
    {28,  10, 11}, /* 19 */
    {34,  15, 11}, /* 20 */
    {39,  20, 12}, /* 21 */
    {45,  25, 13}, /* 22 */
    {52,  30, 15}, /* 23 */
    {60,  35, 17}, /* 24 */
    {70,  50, 20}  /* 25 */
};

const struct wis_app_type wis_app[26] = {
    {0,  50}, /*  0 */
    {0,  50}, /*  1 */
    {0,  50}, /*  2 */
    {0,  50}, /*  3 */
    {0,  50}, /*  4 */
    {1,  50}, /*  5 */
    {1,  50}, /*  6 */
    {1,  35}, /*  7 */
    {1,  30}, /*  8 */
    {2,  25}, /*  9 */
    {2,  20}, /* 10 */
    {2,  20}, /* 11 */
    {2,  15}, /* 12 */
    {2,  10}, /* 13 */
    {2,   5}, /* 14 */
    {3,   0}, /* 15 */
    {3,   0}, /* 16 */
    {4,  -5}, /* 17 */
    {4,  -5}, /* 18 */
    {5,  -5}, /* 19 */
    {5, -10}, /* 20 */
    {6, -15}, /* 21 */
    {7, -20}, /* 22 */
    {7, -25}, /* 23 */
    {7, -30}, /* 24 */
    {8, -40}  /* 25 */
};

const struct dex_app_type dex_app[26] = {
    {  60}, /*  0 */
    {  50}, /*  1 */
    {  50}, /*  2 */
    {  40}, /*  3 */
    {  30}, /*  4 */
    {  20}, /*  5 */
    {  10}, /*  6 */
    {   0}, /*  7 */
    {   0}, /*  8 */
    {   0}, /*  9 */
    {   0}, /* 10 */
    {   0}, /* 11 */
    {   0}, /* 12 */
    {   0}, /* 13 */
    {   0}, /* 14 */
    { -10}, /* 15 */
    { -15}, /* 16 */
    { -20}, /* 17 */
    { -30}, /* 18 */
    { -40}, /* 19 */
    { -50}, /* 20 */
    { -65}, /* 21 */
    { -75}, /* 22 */
    { -90}, /* 23 */
    {-105}, /* 24 */
    {-120}  /* 25 */
};

const struct con_app_type con_app[26] = {
    {-4, 20}, /*  0 */
    {-3, 25}, /*  1 */
    {-2, 30}, /*  2 */
    {-2, 35}, /*  3 */
    {-1, 40}, /*  4 */
    {-1, 45}, /*  5 */
    {-1, 50}, /*  6 */
    { 0, 55}, /*  7 */
    { 0, 60}, /*  8 */
    { 0, 65}, /*  9 */
    { 0, 70}, /* 10 */
    { 0, 75}, /* 11 */
    { 0, 80}, /* 12 */
    { 0, 85}, /* 13 */
    { 0, 88}, /* 14 */
    { 1, 90}, /* 15 */
    { 2, 95}, /* 16 */
    { 2, 97}, /* 17 */
    { 3, 99}, /* 18 */
    { 3, 99}, /* 19 */
    { 4, 99}, /* 20 */
    { 4, 99}, /* 21 */
    { 5, 99}, /* 22 */
    { 6, 99}, /* 23 */
    { 7, 99}, /* 24 */
    { 8, 99}  /* 25 */
};

/*
 * Liquid properties.
 * Used in world.obj.
 */

const struct liq_type liq_table[LIQ_MAX] = {
    {"water",                "clear",     { 0, 1, 10}},    /*  0 */
    {"beer",                 "amber",     { 3, 2,  5}},    /*  1 */
    {"wine",                 "rose",      { 5, 2,  5}},    /*  2 */
    {"ale",                  "brown",     { 2, 2,  5}},    /*  3 */
    {"dark ale",             "dark",      { 1, 2,  5}},    /*  4 */
    {"whisky",               "golden",    { 6, 1,  4}},    /*  5 */
    {"lemonade",             "pink",      { 0, 1,  8}},    /*  6 */
    {"firebreather",         "boiling",   {10, 0,  0}},    /*  7 */
    {"local specialty",      "everclear", { 3, 3,  3}},    /*  8 */
    {"slime mold juice",     "green",     { 0, 4, -8}},    /*  9 */
    {"milk",                 "white",     { 0, 3,  6}},    /* 10 */
    {"tea",                  "tan",       { 0, 1,  6}},    /* 11 */
    {"coffee",               "black",     { 0, 1,  6}},    /* 12 */
    {"blood",                "red",       { 0, 2, -1}},    /* 13 */
    {"salt water",           "clear",     { 0, 1, -2}},    /* 14 */
    {"@@bchocolate milk@@N", "creamy",    { 0, 3,  6}},    /* 15 */
    {"mountain dew",         "bubbly",    { 0, 1,  5}},    /* 16 */
    {"bourbon",              "golden",    { 6, 1,  4}},    /* 17 */
};

const float         hr_damTable[121] = {
    0.311,    0.325,    0.341,    0.358,    0.378,    0.399,    0.423,    0.449,    0.478,    0.509,
    0.543,    0.579,    0.618,    0.659,    0.703,    0.749,    0.797,    0.846,    0.897,    0.948,
    1.000,    1.012,    1.024,    1.036,    1.048,    1.060,    1.071,    1.083,    1.094,    1.105,
    1.117,    1.127,    1.138,    1.149,    1.159,    1.169,    1.178,    1.188,    1.197,    1.206,
    1.215,    1.223,    1.231,    1.239,    1.247,    1.254,    1.261,    1.268,    1.274,    1.281,
    1.287,    1.292,    1.298,    1.303,    1.308,    1.313,    1.317,    1.322,    1.326,    1.330,
    1.333,    1.337,    1.340,    1.344,    1.347,    1.350,    1.352,    1.355,    1.357,    1.360,
    1.362,    1.364,    1.366,    1.368,    1.370,    1.372,    1.373,    1.375,    1.376,    1.377,
    1.379,    1.380,    1.381,    1.382,    1.383,    1.384,    1.385,    1.386,    1.387,    1.387,
    1.388,    1.389,    1.389,    1.390,    1.391,    1.391,    1.392,    1.392,    1.393,    1.393,
    1.393,    1.394,    1.394,    1.395,    1.395,    1.395,    1.395,    1.396,    1.396,    1.396,
    1.396,    1.397,    1.397,    1.397,    1.397,    1.397,    1.397,    1.398,    1.398,    1.398,
    1.398
};

/* The shield table! */
const struct shield_type shield_table[] = {
    { "",           FALSE, 0, 0, 0,                                       /* 0: SHIELD_NONE */
      "", "", "", "", "", "", ""
    },
    { "@@eFIRE@@N", TRUE, 300, 0, 0,                                      /* 1: SHIELD_FIRE */
      "@@NYour @@eshield@@N flares, and envelops $N with @@eflame@@N!!!", /* absorb self */
      "@@N$n's @@eshield@@N flares, and envelops you with @@eflame@@N!!", /* asborb victim */
      "@@N$n's @@eshield@@N flares and envelops $N in @@eflames@@N!!",    /* asborb room */

      "@@NYou @@eburst@@N into @@Rflames@@N!",                            /* add self */
      "@@N$n @@ebursts@@N into @@Rflames@@N!",                            /* add room */

      "@@NYour @@eshield@@N @@yFLAMES OUT@@N!!!!!",                       /* remove self */
      "@@N$n's @@eshield@@N @@yFLAMES OUT@@N!!!!!"                        /* remove room */
    },
    { "@@aICE@@N", FALSE, 0, 30, 10,                                      /* 2: SHIELD_ICE */
      "@@NYour @@ashield@@N absorbs the blow@@N!!!",
      "@@N$n's @@ashield@@N absorbs the blow@@N!!",
      "@@N$n's @@ashield@@N absorbs the blow@@N!!",

      "@@NYou are encased in @@aIce@@N!",
      "@@N$n is encased in @@aIce@@N!!",

      "@@NYour @@ashield@@N is @@rSHATTERED@@N!!!",
      "@@N$n's @@ashield @@Nis @@rSHATTERED@@N!!!"
    },
    { "@@lSHOCK@@N", TRUE, 200, 0, 0,                                     /* 3: SHIELD_SHOCK */
      "@@NYour @@lshield@@N @@ysparks@@N, and zaps $N@@N!!!",
      "@@N$n's @@lshield@@N @@ysparks@@N, and zaps you@@N!!",
      "@@N$n's @@lshield@@N @@ysparks,@@N and zaps $N@@N!!",

      "@@NYou are surrounded by an @@lelectric field@@N!!",
      "@@N$n is surrounded in an @@lelectric field@@N!!",

      "@@NYour @@lshield@@N @@dDISSIPATES@@N!!!!",
      "@@N$n's @@lshield@@N @@dDISSIPATES@@N!!!!"
    },
    { "@@dDEMON@@N", TRUE, 300, 0, 0,                                     /* 4: SHIELD_DEMON */
      "@@dYour s@@Rh@@eie@@Rl@@dd flares, and envelops $N@@D with the @@RS@@eo@@duls @@go@@df @@gt@@dh@@ge @@RD@@ee@@dad@@d!!@@N",
      "$n@@d's s@@Rh@@eie@@Rl@@dd flares, and envelops you with the @@RS@@eo@@duls @@go@@df @@gt@@dh@@ge @@RD@@ee@@dad@@d!!@@N",
      "$n@@d's s@@Rh@@eie@@Rl@@dd flares and envelops $N@@d in the @@RS@@eo@@duls @@go@@df @@gt@@dh@@ge @@RD@@ee@@dad@@d!!@@N",

      "@@dYou @@ms@@pu@@gmm@@po@@mn @@dthe @@RS@@eo@@du@@el@@R's@@d of @@RP@@eu@@Rr@@eg@@ya@@et@@Ro@@er@@Ry!@@e!@@R!@@N",
      "@@d$n @@ms@@pu@@gmm@@po@@mn@@d'@@ms @@dthe @@RS@@eo@@du@@el@@R's@@d of @@RP@@eu@@Rr@@eg@@ya@@et@@Ro@@er@@Ry!@@e!@@R!@@N",

      "@@dThe @@RS@@eo@@du@@el@@Rs@@d stop circling you and return to @@RP@@eu@@Rr@@eg@@ya@@et@@Ro@@er@@Ry!@@e!@@R!@@N",
      "@@dThe @@RS@@eo@@du@@el@@Rs@@d circling $n stop and return to @@RP@@eu@@Rr@@eg@@ya@@et@@Ro@@er@@Ry!@@e!@@R!@@N"
    }
};

/* The cloak table */
const struct shield_type cloak_table[] = {
    { "",           FALSE, 0, 0, 0,                                       /* 0: CLOAK_NONE */
      "", "", "", "", "", "", ""
    },
    { "@@eFLAMING@@N", TRUE, 0, 0, 0,                                     /* 1: CLOAK_FLAMING */
      "@@NYour @@ecloak@@N flares, and envelops $N with @@eflame@@N!!!",  /* absorb self */
      "@@N$N's @@ecloak@@N flares, and envelops you with @@eflame@@N!!",  /* asborb victim */
      "@@N$n's @@ecloak@@N flares and envelops $N in @@eflames@@N!!",     /* asborb room */

      "@@NThe power of @@efire @@Ncloaks you!",                           /* add self */
      "@@NThe power of @@efire @@Ncloaks $n@@N!",                         /* add room */

      "@@NThe @@ecloak@@N around your body fades.",                       /* remove self */
      "@@NThe @@ecloak@@N around $n's body fades."                        /* remove room */
    },
    { "@@WADEPT@@N", FALSE, 0, 0, 0,                                      /* 2: CLOAK_ADEPT */
      "",
      "",
      "",

      "@@NYou pull the cloak of the @@WAdepts@@N around you.",
      "@@N$n@@N is cloaked by the @@WAdepts@@N.",

      "@@NThe @@Wcloak@@N around your body fades.",
      "@@NThe @@Wcloak@@N around $n's body fades."
    },
    { "@@yMANA@@N", FALSE, 0, 0, 0,                                       /* 3: CLOAK_MANA */
      "@@NYour @@yC@@Wl@@yo@@Wa@@yk@@N flares, and envelops you with a @@WMagical @@yPresence@@N!!",
      "@@N$n's @@yC@@Wl@@yo@@Wa@@yk@@N flares, and envelops $m in a @@WMagical @@yPresence@@N!!",
      "@@N$n's @@yC@@Wl@@yo@@Wa@@yk@@N flares, and envelops $m in a @@WMagical @@yPresence@@N!!",

      "@@NYou are surrounded by a @@WMagical @@yPresence@@N!!",
      "@@N$n is surrounded by a @@WMagical @@yPresence@@N!!",

      "@@NThe @@yc@@Wl@@yo@@Wa@@yk@@N around your body fades.",
      "@@NThe @@yc@@Wl@@yo@@Wa@@yk@@N around $n's body fades."
    },
    { "@@lABSORB@@N", FALSE, 0, 0, 0,                                     /* 4: CLOAK_ABSORB */
      "@@NYour @@lcloak@@N glows brightly, and absorbs $N's spell@@N!!!",
      "@@N$N's @@lcloak@@N glows brightly, and absorbs your spell@@N!!",
      "@@N$n's @@lcloak@@N glows brightly as $N's spell hits it, then fades@@N!!",

      "@@NYou pull the cloak of @@lAbsorption @@Naround you.",
      "@@N$n@@N is cloaked by @@lAbsorption@@N.",

      "@@NThe @@lcloak@@N around your body fades.",
      "@@NThe @@lcloak@@N around $n's body fades."
    },
    { "@@mREFLECT@@N", FALSE, 0, 0, 0,                                    /* 5: CLOAK_REFLECT */
      "@@NYour @@lc@@el@@ro@@ya@@ak@@N glows brightly as $N's spell hits it@@N!!!",
      "@@N$N's @@lc@@el@@ro@@ya@@ak@@N glows brightly as your spell hits it@@N!!",
      "@@N$n's @@lc@@el@@ro@@ya@@ak@@N glows brightly as $N's spell hits it@@N!!",

      "@@NYou pull the cloak of @@mReflection @@Naround you.",
      "@@N$n@@N is cloaked by @@mReflection@@N.",

      "@@NThe @@lc@@el@@ro@@ya@@ak@@N around your body fades.",
      "@@NThe @@lc@@el@@ro@@ya@@ak@@N around $n's body fades."
    },
    { "@@rREGEN@@N", FALSE, 0, 0, 0,                                      /* 6: CLOAK_REGEN */
      "",
      "",
      "",

      "@@NYou pull the cloak of @@rRegeneration @@Naround you.",
      "@@N$n@@N is cloaked by @@rRegeneration@@N.",

      "@@NThe @@rcloak@@N around your body fades.",
      "@@NThe @@rcloak@@N around $n's body fades."
    }
};

const struct avatarlimit_type avatarlimit_table[] = {
    { "tranquility", 100 },
    { "smokescreen", 100 },
    { "sentry",      100 },
    { "coldwave",    100 },
    { "stealth",     100 },
    { "nutrition",     9 },
    { "compassion",  100 },
    { "innerflame",  100 },
    { "",              0 }
};

/*
 * The skill and spell table.
 * Slot numbers must never be changed as they appear in #OBJECTS sections.
 */

#define SLOT(n) n
#define XX 82
/*
 * Magic spells.
 */

const struct skill_type skill_table[MAX_SKILL] = {
    { MORTAL, NORM, "reserved",            {99, 99, 99, 99, 99}, spell_null,                TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(0),      0,  0, "", "", "" },
    { MORTAL, NORM, "acid blast",          {50, XX, XX, XX, XX}, spell_acid_blast,          TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(70),    20, 12, "acid blast", "!Acid Blast!", "" },
    { MORTAL, NORM, "armor",               { 3, XX, XX, XX, XX}, spell_armor,               TAR_CHAR_DEFENSIVE, POS_STANDING, NULL,                 SLOT(1),      5, 12, "", "You feel less protected.", "" },
    { REMORT, NORM, "animate",             {XX, XX, XX, 70, XX}, spell_animate,             TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(525),   50, 24, "", "!Animate!", "" },
    { MORTAL, NORM, "bad breath",          { 4, XX, XX, XX, XX}, spell_badbreath,           TAR_CHAR_OFFENSIVE, POS_STANDING, NULL,                 SLOT(502),   10, 12, "bad breath", "!Bad Breath!", "" },
    { MORTAL, NORM, "bark skin",           {XX, 20, XX, XX, XX}, spell_bark_skin,           TAR_CHAR_SELF,      POS_STANDING, NULL,                 SLOT(514),   20, 24, "", "Your skin feels softer.", "$n's skin looks less wooden." },
    { MORTAL, NORM, "bless",               {XX,  8, XX, XX, XX}, spell_bless,               TAR_CHAR_DEFENSIVE, POS_STANDING, NULL,                 SLOT(3),      5, 12, "", "You feel less righteous.", "$n looks less Holy." },
    { MORTAL, NORM, "blindness",           {16, XX, XX, XX, XX}, spell_blindness,           TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_blindness,       SLOT(4),      5, 12, "", "You can see again.", "$n's vision returns." },
    { MORTAL, NORM, "bloody tears",        {XX, XX, XX, XX, 26}, spell_bloody_tears,        TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(518),   12, 12, "Bloody Tears", "!Bloody Tears!", "" },
    { MORTAL, NORM, "burning hands",       { 6, XX, XX, XX, XX}, spell_burning_hands,       TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(5),     15, 12, "burning hands", "!Burning Hands!", "" },
    { MORTAL, NORM, "call lightning",      {XX, 36, XX, XX, XX}, spell_call_lightning,      TAR_IGNORE,         POS_FIGHTING, NULL,                 SLOT(6),     15, 12, "lightning bolt", "!Call Lightning!", "" },
    { MORTAL, NORM, "calm",                {XX, XX, XX, XX, 52}, spell_calm,                TAR_IGNORE,         POS_FIGHTING, NULL,                 SLOT(531),   30, 24, "", "!Calm!", "" },
    { MORTAL, NORM, "cause critical",      {XX, 58, XX, XX, XX}, spell_cause_critical,      TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(63),    20, 12, "spell", "!Cause Critical!", "" },
    { MORTAL, NORM, "cause light",         {XX,  4, XX, XX, XX}, spell_cause_light,         TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(62),    15, 12, "spell", "!Cause Light!", "" },
    { MORTAL, NORM, "cause serious",       {XX, 40, XX, XX, XX}, spell_cause_serious,       TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(64),    17, 12, "spell", "!Cause Serious!", "" },
    { MORTAL, NORM, "change sex",          {66, XX, XX, XX, XX}, spell_change_sex,          TAR_CHAR_DEFENSIVE, POS_FIGHTING, &gsn_change_sex,      SLOT(XX),    15, 12, "", "Your body feels familiar again.", "$n looks $mself again." },
    { MORTAL, NORM, "charm person",        {52, XX, XX, XX, XX}, spell_charm_person,        TAR_CHAR_OFFENSIVE, POS_STANDING, &gsn_charm_person,    SLOT(7),      5, 12, "", "You feel more self-confident.", "The glazed look in $n's eyes fades." },
    { MORTAL, NORM, "chill touch",         { 8, XX, XX, XX, XX}, spell_chill_touch,         TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(8),     15, 12, "chilling touch", "You feel less cold.", "" },
    { MORTAL, NORM, "colour spray",        { 7, XX, XX, XX, XX}, spell_colour_spray,        TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(10),    15, 12, "colour spray", "!Colour Spray!", "" },
    { MORTAL, NORM, "continual light",     {11, 27, XX, XX, XX}, spell_continual_light,     TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(57),     7, 12, "", "!Continual Light!", "" },
    { MORTAL, NORM, "control weather",     {58, 79, XX, XX, XX}, spell_control_weather,     TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(11),    25, 12, "", "!Control Weather!", "" },
    { MORTAL, NORM, "create food",         {XX, 26, XX, XX, XX}, spell_create_food,         TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(12),     5, 12, "", "!Create Food!", "" },
    { MORTAL, NORM, "create spring",       {XX, 65, XX, XX, XX}, spell_create_spring,       TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(80),    20, 12, "", "!Create Spring!", "" },
    { MORTAL, NORM, "create water",        {XX,  9, XX, XX, XX}, spell_create_water,        TAR_OBJ_INV,        POS_STANDING, NULL,                 SLOT(13),     5, 12, "", "!Create Water!", "" },
    { MORTAL, NORM, "cure blindness",      {XX, 28, XX, XX, XX}, spell_cure_blindness,      TAR_CHAR_DEFENSIVE, POS_FIGHTING, NULL,                 SLOT(14),     5, 12, "", "!Cure Blindness!", "" },
    { MORTAL, NORM, "cure critical",       {XX, 59, XX, XX, XX}, spell_cure_critical,       TAR_CHAR_DEFENSIVE, POS_FIGHTING, &gsn_cure_critical,   SLOT(15),    20, 12, "", "!Cure Critical!", "" },
    { MORTAL, NORM, "cure light",          {XX,  5, XX, XX, XX}, spell_cure_light,          TAR_CHAR_DEFENSIVE, POS_FIGHTING, &gsn_cure_light,      SLOT(16),    10, 12, "", "!Cure Light!", "" },
    { MORTAL, NORM, "cure poison",         {XX, 32, XX, XX, XX}, spell_cure_poison,         TAR_CHAR_DEFENSIVE, POS_STANDING, NULL,                 SLOT(43),     5, 12, "", "!Cure Poison!", "" },
    { MORTAL, NORM, "cure serious",        {XX, 41, XX, XX, XX}, spell_cure_serious,        TAR_CHAR_DEFENSIVE, POS_FIGHTING, &gsn_cure_serious,    SLOT(61),    15, 12, "", "!Cure Serious!", "" },
    { MORTAL, NORM, "curse",               {35, 26, XX, XX, XX}, spell_curse,               TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_curse,           SLOT(17),    20, 12, "curse", "The curse wears off.", "$n starts to look more Holy." },
    { MORTAL, NORM, "detect evil",         {12,  8, XX, XX, XX}, spell_detect_evil,         TAR_CHAR_SELF,      POS_STANDING, NULL,                 SLOT(18),     5, 12, "", "The red in your vision disappears.", "" },
    { MORTAL, NORM, "detect hidden",       {17, 16, XX, XX, XX}, spell_detect_hidden,       TAR_CHAR_SELF,      POS_STANDING, NULL,                 SLOT(44),     5, 12, "", "You feel less aware of your suroundings.", "" },
    { MORTAL, NORM, "detect invis",        {19, XX, XX, XX, XX}, spell_detect_invis,        TAR_CHAR_SELF,      POS_STANDING, NULL,                 SLOT(19),     5, 12, "", "You no longer see invisible objects.", "" },
    { MORTAL, NORM, "detect magic",        { 7,  7, XX, XX, XX}, spell_detect_magic,        TAR_CHAR_SELF,      POS_STANDING, &gsn_detect_magic,    SLOT(20),     5, 12, "", "The detect magic wears off.", "" },
    { MORTAL, NORM, "detect poison",       {XX, 10, XX, XX, XX}, spell_detect_poison,       TAR_OBJ_INV,        POS_STANDING, NULL,                 SLOT(21),     5, 12, "", "!Detect Poison!", "" },
    { MORTAL, NORM, "detect undead",       { 8, XX, XX, XX, XX}, spell_detect_undead,       TAR_CHAR_SELF,      POS_STANDING, NULL,                 SLOT(513),    8, 12, "", "You no longer sense undead beings.", "" },
    { MORTAL, NORM, "detection",           {XX, XX, XX, XX, 22}, spell_detection,           TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(527),   12, 24, "", "!Detection!", ""},
    { MORTAL, NORM, "dimension blade",     {XX, XX, XX, XX, XX}, spell_dimension_blade,     TAR_OBJ_INV,        POS_STANDING, NULL,                 SLOT(523),  100, 41, "", "!Dimension Blade!", "" },
    { MORTAL, NORM, "dispel magic",        {68, XX, XX, XX, XX}, spell_dispel_magic,        TAR_IGNORE,         POS_FIGHTING, NULL,                 SLOT(59),    15, 12, "", "!Dispel Magic!", "" },
    { MORTAL, NORM, "earthquake",          {XX, 78, XX, XX, XX}, spell_earthquake,          TAR_IGNORE,         POS_FIGHTING, NULL,                 SLOT(23),    15, 12, "earthquake", "!Earthquake!", ""},
    { MORTAL, NORM, "ego whip",            {XX, XX, XX, XX, 25}, spell_ego_whip,            TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(535),   35, 24, "ego whip", "!Ego Whip!", "" },
    { MORTAL, NORM, "enchant weapon",      {39, XX, XX, XX, XX}, spell_enchant_weapon,      TAR_OBJ_INV,        POS_STANDING, NULL,                 SLOT(24),   100, 24, "", "!Enchant Weapon!", "" },
    { MORTAL, NORM, "enhance weapon",      {XX, XX, XX, XX, XX}, spell_enhance_weapon,      TAR_OBJ_INV,        POS_STANDING, NULL,                 SLOT(517),   50, 24, "", "!Enhance Weapon!", "" },
    { MORTAL, NORM, "faerie fire",         { 3, XX, XX, XX, XX}, spell_faerie_fire,         TAR_CHAR_DEFENSIVE, POS_FIGHTING, NULL,                 SLOT(72),     5, 12, "faerie fire", "The pink aura around you fades away.", "The pink aura around $n fades away." },
    { MORTAL, NORM, "faerie fog",          {XX, 14, XX, XX, XX}, spell_faerie_fog,          TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(73),    12, 12, "faerie fog", "!Faerie Fog!", "" },
    { MORTAL, NORM, "fighting trance",     {XX, XX, XX, XX, 60}, spell_fighting_trance,     TAR_CHAR_SELF,      POS_STANDING, NULL,                 SLOT(521),   60, 24, "", "Your fighting trance fades.", "$n's fighting trance fades." },
    { MORTAL, NORM, "fireball",            {28, XX, XX, XX, XX}, spell_fireball,            TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(26),    15, 12, "fireball", "!Fireball!", "" },
    { MORTAL, NORM, "fire blade",          {XX, XX, XX, XX, 50}, spell_fire_blade,          TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(528),   50, 24, "", "!FireBlade!", "" },
    { MORTAL, NORM, "flamestrike",         {XX, 54, XX, XX, XX}, spell_flamestrike,         TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(65),    20, 12, "flamestrike", "!Flamestrike!", "" },
    { MORTAL, NORM, "flare",               {XX, 20, XX, XX, XX}, spell_flare,               TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(591),   18, 12, "", "Your vision returns.", "$n blinks, and starts to see again." },
    { MORTAL, NORM, "fly",                 {24, XX, XX, XX, XX}, spell_fly,                 TAR_CHAR_DEFENSIVE, POS_STANDING, &gsn_fly,             SLOT(56),    10, 18, "", "You slowly float to the ground.", "$n slowly floats to the ground." },
    { MORTAL, NORM, "gate",                {XX, XX, XX, XX, XX}, spell_gate,                TAR_CHAR_DEFENSIVE, POS_FIGHTING, NULL,                 SLOT(83),    50, 12, "", "!Gate!", "" },
    { MORTAL, NORM, "giant strength",      {18, XX, XX, XX, XX}, spell_giant_strength,      TAR_CHAR_DEFENSIVE, POS_STANDING, NULL,                 SLOT(39),    20, 12, "", "You feel weaker.", "$n looks weaker." },
    { MORTAL, NORM, "harm",                {XX, 74, XX, XX, XX}, spell_harm,                TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_harm,            SLOT(27),    35, 12, "harm spell", "You feel better.", ""},
    { MORTAL, NORM, "heal",                {XX, 75, XX, XX, XX}, spell_heal,                TAR_CHAR_DEFENSIVE, POS_FIGHTING, &gsn_heal,            SLOT(28),    50, 12, "", "!Heal!", "" },
    { MORTAL, NORM, "hypnosis",            {XX, XX, XX, XX, 50}, spell_hypnosis,            TAR_CHAR_OFFENSIVE, POS_STANDING, &gsn_hypnosis,        SLOT(532),   30, 24, "", "!Hypnosis!", "" },
    { MORTAL, NORM, "identify",            { 9, XX, XX, XX, XX}, spell_identify,            TAR_OBJ_INV,        POS_STANDING, NULL,                 SLOT(53),    12, 24, "", "!Identify!", "" },
    { MORTAL, NORM, "infravision",         {20, XX, XX, XX, XX}, spell_infravision,         TAR_CHAR_DEFENSIVE, POS_STANDING, NULL,                 SLOT(77),     5, 18, "", "You no longer see in the dark.", "" },
    { MORTAL, NORM, "influx",              {XX, 65, XX, XX, XX}, spell_influx,              TAR_CHAR_DEFENSIVE, POS_FIGHTING, NULL,                 SLOT(503),   75, 24, "", "!Influx!", "" },
    { MORTAL, NORM, "invis",               {17, XX, XX, XX, XX}, spell_invis,               TAR_IGNORE,         POS_STANDING, &gsn_invis,           SLOT(29),     5, 12, "", "You are no longer invisible.", "$n's invisibilty fades." },
    { MORTAL, NORM, "know alignment",      {13, 22, XX, XX, XX}, spell_know_alignment,      TAR_CHAR_DEFENSIVE, POS_FIGHTING, NULL,                 SLOT(58),     9, 12, "", "!Know Alignment!", "" },
    { MORTAL, NORM, "know weakness",       {XX, XX, XX, XX, 18}, spell_know_weakness,       TAR_CHAR_SELF,      POS_FIGHTING, NULL,                 SLOT(530),   15, 12, "", "You are less aware of your enemy's weaknesses.", "" },
    { MORTAL, NORM, "know critical",       {XX, XX, XX, XX, 28}, spell_know_weakness,       TAR_CHAR_SELF,      POS_FIGHTING, NULL,                 SLOT(530),   15, 12, "", "You are less aware of critical damage points.", "" },
    { MORTAL, NORM, "know item",           {XX, XX, XX, XX,  9}, spell_know_item,           TAR_OBJ_INV,        POS_STANDING, NULL,                 SLOT(533),   20, 24, "", "!Know Item!", "" },
    { MORTAL, NORM, "laser bolt",          {XX, XX, XX, XX, XX}, spell_laserbolt,           TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(511),   35, 12, "laserbolt", "!laserbolt!", ""},
    { MORTAL, NORM, "lightning bolt",      {24, XX, XX, XX, XX}, spell_lightning_bolt,      TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(30),    15, 12, "lightning bolt", "!Lightning Bolt!", "" },
    { MORTAL, NORM, "locate object",       {16, 30, XX, XX, XX}, spell_locate_object,       TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(31),    20, 18, "", "!Locate Object!", "" },
    { MORTAL, NORM, "magic missile",       { 3, XX, XX, XX, XX}, spell_magic_missile,       TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(32),    15, 12, "magic missile", "!Magic Missile!", "" },
    { MORTAL, NORM, "mass invis",          {60, XX, XX, XX, XX}, spell_mass_invis,          TAR_IGNORE,         POS_STANDING, &gsn_mass_invis,      SLOT(69),    20, 24, "", "!Mass Invis!", "" },
    { MORTAL, NORM, "mind flail",          {XX, XX, XX, XX,  4}, spell_mind_flail,          TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(536),   12, 24, "mind flail", "!Mind Flail!", "" },
    { MORTAL, NORM, "mystic armour",       {10, XX, XX, XX, XX}, spell_mystic_armor,        TAR_CHAR_DEFENSIVE, POS_STANDING, NULL,                 SLOT(590),   18, 24, "" "Your Mystic Armour fades away", "The Mystic Armour around someone in the room fades away." },
    { MORTAL, NORM, "nerve fire",          {XX, XX, XX, XX, 60}, spell_nerve_fire,          TAR_IGNORE,         POS_FIGHTING, NULL,                 SLOT(520),   50, 24, "nerve fire", "!Nerve Fire!", "" },
    { MORTAL, NORM, "night vision",        {XX, XX, XX, XX, 19}, spell_night_vision,        TAR_CHAR_SELF,      POS_STANDING, NULL,                 SLOT(538),   17, 24, "", "Your eyes feel weaker.", "" },
    { MORTAL, NORM, "pass door",           {46, XX, XX, XX, XX}, spell_pass_door,           TAR_CHAR_SELF,      POS_STANDING, NULL,                 SLOT(74),    20, 12, "", "You feel solid again.", "$n's body becomes more solid." },
    { MORTAL, NORM, "phase",               {XX, XX, XX, XX, 45}, spell_phase,               TAR_CHAR_SELF,      POS_STANDING, NULL,                 SLOT(522),   20, 12, "", "You feel solid again.", "$n's body becomes more solid." },
    { MORTAL, NORM, "physic crush",        {XX, XX, XX, XX, 19}, spell_physic_thrust,       TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(537),   33, 24, "physic crush", "!Physic Crush!", "" },
    { MORTAL, NORM, "physic thrust",       {XX, XX, XX, XX,  7}, spell_physic_thrust,       TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(537),   22, 24, "physic thrust", "!Physic Thrust!", "" },
    { MORTAL, NORM, "poison",              {XX, 31, XX, XX, XX}, spell_poison,              TAR_CHAR_OFFENSIVE, POS_STANDING, &gsn_poison,          SLOT(33),    10, 12, "poison", "You feel less sick.", "$n looks less sick." },
    { MORTAL, NORM, "produce food",        {XX, XX, XX, XX, 24}, spell_produce_food,        TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(524),   16, 24, "", "!Produce Food!", "" },
    { MORTAL, NORM, "protection",          { 9, 12, XX, XX, XX}, spell_protection,          TAR_CHAR_SELF,      POS_STANDING, NULL,                 SLOT(34),     5, 12, "", "You feel less protected.", "" },
    { MORTAL, NORM, "refresh",             {XX, 11, XX, XX, XX}, spell_refresh,             TAR_CHAR_DEFENSIVE, POS_STANDING, NULL,                 SLOT(81),    12, 18, "refresh", "!Refresh!", "" },
    { MORTAL, NORM, "remove curse",        {36, 27, XX, XX, XX}, spell_remove_curse,        TAR_CHAR_DEFENSIVE, POS_STANDING, NULL,                 SLOT(35),     5, 12, "", "!Remove Curse!", "" },
    { MORTAL, NORM, "sanctuary",           {XX, 12, XX, XX, XX}, spell_sanctuary,           TAR_CHAR_DEFENSIVE, POS_STANDING, NULL,                 SLOT(36),    75, 12, "", "The white aura around your body fades.", "The white aura around $n's body fades." },
    { MORTAL, NORM, "seal room",           {70, 50, XX, XX, 50}, spell_seal_room,           TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(541),   75, 12, "", "The Energy web around this room fades.", "" },
    { MORTAL, NORM, "see magic",           {XX, XX, XX, XX,  7}, spell_see_magic,           TAR_CHAR_SELF,      POS_STANDING, NULL,                 SLOT(526),    8, 12, "", "You no longer see magical auras.", "" },
    { MORTAL, NORM, "sense evil",          {XX, XX, XX, XX, 10}, spell_sense_evil,          TAR_CHAR_SELF,      POS_STANDING, NULL,                 SLOT(515),   12, 12, "", "You no longer sense evil.", "" },
    { MORTAL, NORM, "shield",              { 9, XX, XX, XX, XX}, spell_shield,              TAR_CHAR_DEFENSIVE, POS_STANDING, NULL,                 SLOT(67),    12, 18, "", "Your force shield shimmers, then fades away.", "$n's force field shimmers, then fades away." },
    { MORTAL, NORM, "shocking grasp",      { 5, XX, XX, XX, XX}, spell_shocking_grasp,      TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(XX),    15, 12, "shocking grasp", "!Shocking Grasp!", "" },
    { MORTAL, NORM, "sight",               {30, XX, XX, XX, XX}, spell_cure_blindness,      TAR_CHAR_DEFENSIVE, POS_STANDING, NULL,                 SLOT(592),   20, 24, "", "!Sight!", "" },
    { MORTAL, NORM, "sleep",               { 9, XX, XX, XX, XX}, spell_sleep,               TAR_CHAR_OFFENSIVE, POS_STANDING, &gsn_sleep,           SLOT(38),    15, 12, "", "You feel less tired.", "$n starts looking much more awake." },
    { MORTAL, NORM, "stalker",             {XX, XX, XX, XX, XX}, spell_null,                TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(401),  100, 24, "", "!Stalker!", "" },
    { MORTAL, NORM, "stone skin",          {36, XX, XX, XX, XX}, spell_stone_skin,          TAR_CHAR_SELF,      POS_STANDING, NULL,                 SLOT(66),    12, 18, "", "Your skin feels soft again.", "$n's skin loses its stone-like look." },
    { MORTAL, NORM, "suffocate",           {XX, XX, XX, XX, 46}, spell_suffocate,           TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(516),   30, 24, "", "!Suffocate!", "" },
    { MORTAL, NORM, "summon",              {35, XX, XX, XX, XX}, spell_summon,              TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(40),    50, 12, "", "!Summon!", "" },
    { MORTAL, NORM, "teleport",            {40, XX, XX, XX, XX}, spell_teleport,            TAR_CHAR_SELF,      POS_FIGHTING, NULL,                 SLOT(2),     35, 12, "", "!Teleport!", "" },
    { MORTAL, NORM, "travel",              {XX, XX, XX, XX, XX}, spell_travel,              TAR_CHAR_SELF,      POS_STANDING, NULL,                 SLOT(599),   50, 24, "", "!Travel!", "" },
    { MORTAL, NORM, "ventriloquate",       { 3, XX, XX, XX, XX}, spell_ventriloquate,       TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(41),     5, 12, "", "!Ventriloquate!", "" },
    { MORTAL, NORM, "warcry",              {XX, XX, XX,  5, XX}, spell_warcry,              TAR_CHAR_SELF,      POS_STANDING, NULL,                 SLOT(501),    5, 12, "", "You feel less protected.", "" },
    { MORTAL, NORM, "weaken",              {XX, 20, XX, XX, XX}, spell_weaken,              TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(68),    20, 12, "spell", "You feel stronger.", "$n looks stronger." },
/*100*/{MORTAL,NORM,"beacon",              {70, XX, XX, XX, 70}, spell_beacon,              TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(399),   25, 12, "", "!Beacon!", "" },
    { MORTAL, NORM, "portal",              {80, XX, XX, XX, XX}, spell_portal,              TAR_IGNORE,         POS_FIGHTING, NULL,                 SLOT(334),  100, 12, "", "!Portal!", "" },
    { MORTAL, NORM, "window",              {70, XX, XX, XX, 70}, spell_window,              TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(333),  100, 12, "", "!Window!", "" },
    { MORTAL, NORM, "word of recall",      {XX, 66, XX, XX, XX}, spell_word_of_recall,      TAR_CHAR_SELF,      POS_RESTING,  NULL,                 SLOT(42),     5, 12, "", "!Word of Recall!", "" },
    { MORTAL, NORM, "hellspawn",           {43, XX, XX, XX, XX}, spell_hellspawn,           TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(512),   50, 12, "HellSpawn", "!Hellspawn!", "" },
    { MORTAL, NORM, "planergy",            {XX, XX, XX, XX,  5}, spell_planergy,            TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(504),   25, 24, "energy", "!planergy!", "" },
    { MORTAL, NORM, "visit",               {XX, XX, XX, XX, 32}, spell_visit,               TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(505),   50, 48, "", "!visit!", "" },
    { MORTAL, NORM, "barrier",             {XX, XX, XX, XX,  8}, spell_barrier,             TAR_CHAR_SELF,      POS_STANDING, NULL,                 SLOT(506),   30, 24, "", "Your barrier slowly fades.", "The barrier around $n fades." },
    { MORTAL, NORM, "phobia",              {XX, XX, XX, XX, 10}, spell_phobia,              TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(507),   32, 24, "phobia attack", "!phobia!", "" },
    { MORTAL, NORM, "mind bolt",           {XX, XX, XX, XX, 12}, spell_mind_bolt,           TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(519),   40, 12, "Mind Bolt", "!MindBolt!", "" },
    { MORTAL, NORM, "mindflame",           {XX, XX, XX, XX, 35}, spell_mindflame,           TAR_IGNORE,         POS_FIGHTING, NULL,                 SLOT(508),   40, 24, "mindflame", "!mindflame!", "" },
    { MORTAL, NORM, "chain lightning",     {65, XX, XX, XX, XX}, spell_chain_lightning,     TAR_IGNORE,         POS_FIGHTING, NULL,                 SLOT(509),   25, 24, "bolt", "!chain-light!", "" },
    { MORTAL, NORM, "static",              {XX, XX, XX, XX, 20}, spell_static,              TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(510),   40, 24, "discharge", "!static!", "" },
    { REMORT, NORM, "cloak:absorption",    {60, XX, XX, XX, 43}, spell_cloak_absorb,        TAR_CHAR_DEFENSIVE, POS_STANDING, &gsn_cloak_absorb,    SLOT(221),  500, 12, "", "", "" },
    { REMORT, NORM, "cloak:reflection",    {48, XX, XX, 70, 61}, spell_cloak_reflect,       TAR_CHAR_DEFENSIVE, POS_STANDING, &gsn_cloak_reflect,   SLOT(222),  500, 12, "", "", "" },
    { REMORT, NORM, "cloak:flaming",       {70, XX, XX, 60, XX}, spell_cloak_flaming,       TAR_CHAR_DEFENSIVE, POS_STANDING, &gsn_cloak_flaming,   SLOT(223),  750, 12, "", "", "" },
    { REMORT, NORM, "cloak:mana",          {70, XX, XX, 70, XX}, spell_cloak_mana,          TAR_CHAR_DEFENSIVE, POS_STANDING, &gsn_cloak_mana,      SLOT(672),  750, 12, "", "", "" },
    {  ADEPT, NORM, "cloak:adept",         { 1, XX, XX, XX, XX}, spell_cloak_adept,         TAR_CHAR_DEFENSIVE, POS_STANDING, &gsn_cloak_adept,     SLOT(226),  500, 12, "", "", "" },
    { REMORT, NORM, "cloak:regeneration",  {XX, XX, 52, XX, 73}, spell_cloak_regen,         TAR_CHAR_DEFENSIVE, POS_STANDING, &gsn_cloak_regen,     SLOT(227),  500, 12, "", "", "" },
    { MORTAL, NORM, "acid breath",         {XX, XX, XX, XX, XX}, spell_acid_breath,         TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(200),    0,  4, "blast of acid", "!Acid Breath!", "" },
    { MORTAL, NORM, "fire breath",         {XX, XX, XX, XX, XX}, spell_fire_breath,         TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(201),    0,  8, "blast of flame", "!Fire Breath!", "" },
    { MORTAL, NORM, "frost breath",        {XX, XX, XX, XX, XX}, spell_frost_breath,        TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(202),    0,  4, "blast of frost", "!Frost Breath!", "" },
    { MORTAL, NORM, "gas breath",          {XX, XX, XX, XX, XX}, spell_null,                TAR_IGNORE,         POS_FIGHTING, NULL,                 SLOT(203),    0,  4, "blast of gas", "!Gas Breath!", "" },
    { MORTAL, NORM, "lightning breath",    {XX, XX, XX, XX, XX}, spell_lightning_breath,    TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(204),    0,  4, "blast of lightning", "!Lightning Breath!", "" },
    { MORTAL, NORM, "appraise",            {XX, XX, 12, XX, XX}, spell_null,                TAR_IGNORE,         POS_STANDING, &gsn_appraise,        SLOT(0),      0, 24, "", "!Appraise!", "" },
    { MORTAL, NORM, "bash",                {XX, XX, XX, 28, XX}, spell_null,                TAR_IGNORE,         POS_FIGHTING, &gsn_bash,            SLOT(0),      0, 24, "bash", "!bash!", "" },
    { MORTAL, NORM, "berserk",             {XX, XX, XX, 34, XX}, spell_null,                TAR_IGNORE,         POS_FIGHTING, &gsn_berserk,         SLOT(0),      0, 24, "", "You calm down!", "$n looks calmer!" },
    { MORTAL, NORM, "climb",               {XX, XX, 25, XX, XX}, spell_null,                TAR_IGNORE,         POS_FIGHTING, &gsn_climb,           SLOT(0),      0, 24, "", "!Climb!", "" },
    { MORTAL, NORM, "dirt",                {XX, XX, XX, 35, XX}, spell_null,                TAR_IGNORE,         POS_FIGHTING, &gsn_dirt,            SLOT(0),      0, 24, "", "!dirt!", "" },
    { MORTAL, NORM, "nodisarm",            {XX, XX, 40, 30, XX}, spell_null,                TAR_IGNORE,         POS_FIGHTING, &gsn_nodisarm,        SLOT(0),      0,  0, "", "!nodisarm!", "" },
    { MORTAL, NORM, "notrip",              {XX, XX, 30, 40, XX}, spell_null,                TAR_IGNORE,         POS_FIGHTING, &gsn_notrip,          SLOT(0),      0,  0, "", "!notrip!", "" },
    { MORTAL, NORM, "smash",               {XX, XX, XX, 30, XX}, spell_null,                TAR_IGNORE,         POS_STANDING, &gsn_smash,           SLOT(0),      0,  0, "", "!smash!", "" },
    { MORTAL, NORM, "trip",                {XX, XX, 18, 25, XX}, spell_null,                TAR_IGNORE,         POS_FIGHTING, &gsn_trip,            SLOT(0),      0,  0, "", "!trip!", "" },
    { MORTAL, NORM, "circle",              {XX, XX, 19, XX, XX}, spell_null,                TAR_IGNORE,         POS_FIGHTING, &gsn_circle,          SLOT(0),      0, 24, "backstab", "!circle!", "" },
    { MORTAL, NORM, "backstab",            {XX, XX, 10, XX, XX}, spell_null,                TAR_IGNORE,         POS_STANDING, &gsn_backstab,        SLOT(0),      0, 24, "backstab", "!Backstab!", "" },
    { MORTAL, NORM, "disarm",              {XX, XX, XX, 12, XX}, spell_null,                TAR_IGNORE,         POS_FIGHTING, &gsn_disarm,          SLOT(0),      0, 24, "", "!Disarm!", "" },
    { MORTAL, NORM, "dodge",               {XX, XX,  7, 16, XX}, spell_null,                TAR_IGNORE,         POS_FIGHTING, &gsn_dodge,           SLOT(0),      0,  0, "", "!Dodge!", "" },
    { REMORT, NORM, "dualwield",           {XX,  8,  5, XX,  7}, spell_null,                TAR_IGNORE,         POS_FIGHTING, &gsn_dualwield,       SLOT(0),      0,  0, "", "!DualWield!", "" },
    { MORTAL, NORM, "enhanced damage",     {XX, XX, XX, 42, XX}, spell_null,                TAR_IGNORE,         POS_FIGHTING, &gsn_enhanced_damage, SLOT(0),      0,  0, "", "!Enhanced Damage!", "" },
    { MORTAL, NORM, "find doors",          {XX, XX, 16, XX, XX}, spell_null,                TAR_IGNORE,         POS_FIGHTING, &gsn_find_doors,      SLOT(0),      0,  0, "", "!Find Doors!", "" },
    { MORTAL, NORM, "fourth attack",       {XX, XX, XX, 50, XX}, spell_null,                TAR_IGNORE,         POS_FIGHTING, &gsn_fourth_attack,   SLOT(0),      0,  0, "", "!Fourth Attack!", "" },
    { MORTAL, NORM, "headbutt",            {XX, XX, XX,  6, XX}, spell_null,                TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_headbutt,        SLOT(0),      0, 24, "headbutt", "!HeadButt!", "" },
    { MORTAL, NORM, "hide",                {XX, XX, 12, XX, XX}, spell_null,                TAR_IGNORE,         POS_RESTING,  &gsn_hide,            SLOT(0),      0, 12, "", "!Hide!", "" },
    { MORTAL, NORM, "hunt",                {XX, XX, 70, XX, XX}, spell_null,                TAR_IGNORE,         POS_RESTING,  &gsn_hunt,            SLOT(0),      0, 12, "", "!Hunt!", "" },
    { MORTAL, NORM, "kick",                {XX, XX, XX, 14, XX}, spell_null,                TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_kick,            SLOT(0),      0, 24, "kick", "!Kick!", "" },
    { MORTAL, NORM, "knee",                {XX, XX, XX,  8, XX}, spell_null,                TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_knee,            SLOT(0),      0, 24, "knee", "!Knee", "" },
    { MORTAL, NORM, "martial arts",        {XX, XX, 30, 20, 30}, spell_null,                TAR_IGNORE,         POS_FIGHTING, &gsn_martial_arts,    SLOT(0),      0,  0, "", "!Martial Arts!", "" },
    { MORTAL, NORM, "parry",               {XX, XX, XX, 10, XX}, spell_null,                TAR_IGNORE,         POS_FIGHTING, &gsn_parry,           SLOT(0),      0,  0, "", "!Parry!", "" },
    { MORTAL, NORM, "peek",                {XX, XX, 20, XX, XX}, spell_null,                TAR_IGNORE,         POS_STANDING, &gsn_peek,            SLOT(0),      0,  0, "", "!Peek!", "" },
    { MORTAL, NORM, "pick lock",           {XX, XX, 25, XX, XX}, spell_null,                TAR_IGNORE,         POS_STANDING, &gsn_pick_lock,       SLOT(0),      0, 12, "", "!Pick!", "" },
    { MORTAL, NORM, "punch",               {XX, XX, XX,  5, XX}, spell_null,                TAR_IGNORE,         POS_STANDING, &gsn_punch,           SLOT(0),      0, 12, "", "!Punch!", "" },
    { MORTAL, NORM, "rescue",              {XX, XX, XX, 15, XX}, spell_null,                TAR_IGNORE,         POS_FIGHTING, &gsn_rescue,          SLOT(0),      0, 12, "", "!Rescue!", "" },
    { MORTAL, NORM, "second attack",       {XX, XX, 15, 10, XX}, spell_null,                TAR_IGNORE,         POS_FIGHTING, &gsn_second_attack,   SLOT(0),      0,  0, "", "!Second Attack!", "" },
    { MORTAL, NORM, "shadowform",          {XX, XX, XX, XX,  9}, spell_null,                TAR_IGNORE,         POS_STANDING, &gsn_shadow,          SLOT(0),      0,  0, "", NULL, "" },
    { MORTAL, NORM, "shield block",        {XX, XX, XX, 40, XX}, spell_null,                TAR_IGNORE,         POS_STANDING, &gsn_shield_block,    SLOT(0),      0, 12, "", "!Shield Block!", "" },
    { MORTAL, NORM, "sneak",               {XX, XX,  5, XX, XX}, spell_null,                TAR_IGNORE,         POS_STANDING, &gsn_sneak,           SLOT(0),      0, 12, "", NULL, "" },
    { MORTAL, NORM, "steal",               {XX, XX,  1, XX, XX}, spell_null,                TAR_IGNORE,         POS_STANDING, &gsn_steal,           SLOT(0),      0, 24, "", "!Steal!", "" },
    { REMORT, NORM, "stun",                {XX, XX, 18, XX, 25}, spell_null,                TAR_IGNORE,         POS_FIGHTING, &gsn_stun,            SLOT(0),      0, 24, "", "", "" },
    { MORTAL, NORM, "third attack",        {XX, XX, 60, 25, XX}, spell_null,                TAR_IGNORE,         POS_FIGHTING, &gsn_third_attack,    SLOT(0),      0,  0, "", "!Third Attack!", "" },
    { REMORT, NORM, "disguise",            {XX, 25, XX, XX, XX}, spell_null,                TAR_IGNORE,         POS_STANDING, &gsn_disguise,        SLOT(0),      0,  0, "", "!Disguise!", "" },
    { REMORT, NORM, "frenzy",              {XX, XX, 23, XX, 18}, spell_null,                TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_frenzy,          SLOT(0),      0, 24, "frenzy", "!FRENZY!", "" },
    { MORTAL, NORM, "emotion control",     {XX, XX, XX, XX, 75}, spell_null,                TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_emotion_control, SLOT(0),      0, 24, "emotion control", "!EMOTION CONTROL!", "" },
    { MORTAL, NORM, "general purpose",     {XX, XX, XX, XX, XX}, spell_general_purpose,     TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(205),    0, 12, "general purpose ammo", "!General Purpose Ammo!", "" },
    { MORTAL, NORM, "high explosive",      {XX, XX, XX, XX, XX}, spell_high_explosive,      TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(206),    0, 12, "high explosive ammo", "!High Explosive Ammo!", "" },
    { REMORT, NORM, "deflect weapon",      {XX, XX, XX, XX, 10}, spell_deflect_weapon,      TAR_CHAR_DEFENSIVE, POS_STANDING, NULL,                 SLOT(600),   25, 12, "", "Your mind shield melts away.", "" },
    { REMORT, NORM, "black hand",          {XX, XX, XX,  7, XX}, spell_black_hand,          TAR_CHAR_OFFENSIVE, POS_STANDING, &gsn_black_hand,      SLOT(601),   50, 12, "", "The hand dissolves from around your throat into nothingness.", "" },
    { REMORT, NORM, "throwing needle",     {XX,  5, XX, XX, XX}, spell_throw_needle,        TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(602),   40, 12, "Throwing Needle", "!Throwing Needle!", "" },
    { REMORT, NORM, "morale",              {XX, XX, 10, XX, XX}, spell_morale,              TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(603),   75, 24, "Morale", "!Morale!", "" },
    { REMORT, NORM, "leadership",          {XX, XX, 13, XX, XX}, spell_leadership,          TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(604),   75, 24, "Leadership", "!Leadership!", "" },
    { REMORT, NORM, "ice bolt",            { 3, XX, XX, XX, XX}, spell_ice_bolt,            TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(605),   20, 12, "Ice Bolt", "!Ice Bolt!", "" },
    { REMORT, NORM, "water elemental",     { 6, XX, XX, XX, XX}, spell_waterelem,           TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(606),   80, 24, "", "!Stalker!", "" },
    { REMORT, NORM, "skeleton",            {XX, XX, XX,  9, XX}, spell_skeleton,            TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(607),   80, 24, "", "!Stalker!", "" },
    { REMORT, NORM, "poison weapon",       {XX, 18, XX, 25, XX}, spell_poison_weapon,       TAR_OBJ_INV,        POS_STANDING, NULL,                 SLOT(608),  100, 24, "", "!Enchant Weapon!", "" },
    { REMORT, NORM, "ethereal travel",     {30, XX, 30, 40, 24}, spell_ethereal,            TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(611),  250, 48, "", "ETHEREAL TRAVEL!", "" },
    { REMORT, NORM, "adrenaline",          {XX, XX, 38, XX, 28}, spell_null,                TAR_IGNORE,         POS_FIGHTING, &gsn_adrenaline,      SLOT(0),      0,  0, "", "!ADRENALINE!", "" },
    { REMORT, NORM, "throwing star",       {XX, 43, XX, XX, XX}, spell_throw_needle,        TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(628),   85, 12, "Throwing Star", "!Throwing Star!", "" },
    { REMORT, NORM, "adrenaline bonus",    {XX, XX, XX, XX, XX}, spell_null,                TAR_IGNORE,         POS_FIGHTING, NULL,                 SLOT(612),    0,  0, "", "!ADRENALINE BONUS!", "" },
    { REMORT, NORM, "fire elemental",      {11, XX, XX, XX, XX}, spell_fireelem,            TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(612),  120, 24, "", "!FIRE ELEMENTAL!", "" },
    { REMORT, NORM, "rune:fire",           {11, XX, XX, 15, XX}, spell_rune_fire,           TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(613),  150, 12, "", "The rune dissipates.", "" },
    { REMORT, NORM, "rune:shock",          {13, XX, XX, XX, 15}, spell_rune_shock,          TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(614),  150, 12, "", "The rune dissipates.", "" },
    { REMORT, NORM, "rune:poison",         {XX, 15, XX, 12, XX}, spell_rune_poison,         TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(615),   80, 12, "", "The rune dissipates.", "" },
    { REMORT, NORM, "healing light",       {XX, XX, 25, XX, 33}, spell_healing_light,       TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(616),  150, 12, "", "@@NThe @@mHealing Light@@N dissipates.", "" },
    { REMORT, NORM, "withering shadow",    {XX, 35, XX, 17, XX}, spell_wither_shadow,       TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(617),  150, 12, "", "@@NThe @@dWithering Shadow@@N dissipates.", "" },
    { REMORT, NORM, "mana flare",          {18, XX, XX, 37, 22}, spell_mana_flare,          TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(618),   80, 12, "", "@@NThe @@eMana Flare@@N dissipates.", "" },
    { REMORT, NORM, "mana drain",          {11, XX, XX, 15, XX}, spell_mana_drain,          TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(619),  150, 12, "", "@@NThe @@dMana Drain@@N dissipates.", "" },
    { REMORT, NORM, "cage",                {18, 31, 40, 19, 13}, spell_cage,                TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(620),  150, 12, "", "@@NThe @@rCage@@N dissipates.", "" },
    { REMORT, NORM, "room dispel",         {22, 37, 33, 24, 23}, spell_room_dispel,         TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(625),  120, 12, "", "", "" },
    { REMORT, NORM, "soul net",            {XX, XX, XX, 58, XX}, spell_soul_net,            TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(629),  350, 12, "", "@@NThe @@dSoul Net@@N dissipates.", "" },
    { REMORT, NORM, "condense soul",       {XX, XX, XX, 78, XX}, spell_condense_soul,       TAR_OBJ_INV,        POS_STANDING, NULL,                 SLOT(630),  800, 24, "", "!Soul Potion!", "" },
    { REMORT, NORM, "restoration",         {XX, XX, XX, XX, XX}, spell_restoration,         TAR_CHAR_DEFENSIVE, POS_FIGHTING, NULL,                 SLOT(632),   50, 12, "", "!restoration!", "" },
    { REMORT, NORM, "infuse",              {XX, XX, XX, 71, XX}, spell_infuse,              TAR_OBJ_INV,        POS_STANDING, NULL,                 SLOT(633), 1000, 24, "", "!Infuse Soul!", "" },
    { REMORT, NORM, "fifth attack",        {XX, 40, 29, XX, 37}, spell_null,                TAR_IGNORE,         POS_FIGHTING, &gsn_fifth_attack,    SLOT(0),      0,  0, "", "!Fifth Attack!", "" },
    { REMORT, NORM, "sixth attack",        {XX, 60, 49, XX, 58}, spell_null,                TAR_IGNORE,         POS_FIGHTING, &gsn_sixth_attack,    SLOT(0),      0,  0, "", "!Sixth Attack!", "" },
    { REMORT, NORM, "holy light",          {XX, XX, 43, XX, XX}, spell_holy_light,          TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(634),   75, 24, "Holy Light", "!Holy Light!", "" },
    { REMORT, NORM, "target",              {XX, XX, 20, XX, XX}, spell_null,                TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_target,          SLOT(0),      0, 24, "target", "!target", "" },
    { REMORT, NORM, "charge",              {XX, XX, 65, XX, XX}, spell_null,                TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_charge,          SLOT(0),      0, 24, "Charge", "!Charge", "" },
    { REMORT, NORM, "scout",               {XX, 26, 45, XX, XX}, spell_null,                TAR_IGNORE,         POS_STANDING, &gsn_scout,           SLOT(0),      0,  0, "", "", "" },
    { MORTAL, NORM, "mount",               {XX, XX, XX, 20, XX}, spell_null,                TAR_IGNORE,         POS_STANDING, &gsn_mount,           SLOT(0),      0,  0, "", "", "" },
    { REMORT, NORM, "divine intervention", {XX, XX, 70, XX, XX}, spell_divine_intervention, TAR_CHAR_DEFENSIVE, POS_FIGHTING, &gsn_divine_intervention, SLOT(635),  200, 12, "", "!Intervention!", "" },
    { REMORT, NORM, "holy armor",          {XX, XX, 30, XX, XX}, spell_holy_armor,          TAR_CHAR_DEFENSIVE, POS_STANDING, NULL,                 SLOT(636),  100, 12, "", "Your armor is no longer blessed.", "" },
/*200*/{REMORT,NORM,"unit tactics",        {XX, XX, 16, XX, XX}, spell_null,                TAR_IGNORE,         POS_STANDING, &gsn_unit_tactics,    SLOT(0),      0, 24, "Unit Tactics", "!Unit Tactics!", "" },
    { REMORT, NORM, "earth elemental",     {52, XX, XX, XX, XX}, spell_earthelem,           TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(637),  500, 24, "", "!EARTH ELEMENTAL!", "" },
    { REMORT, NORM, "iron golem",          {63, XX, XX, XX, XX}, spell_iron_golem,          TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(638),  800, 24, "", "!IRON GOLEM!", "" },
    { REMORT, NORM, "diamond golem",       {77, XX, XX, XX, XX}, spell_diamond_golem,       TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(639), 1100, 24, "", "!DIAMOND GOLEM!", "" },
    { REMORT, NORM, "soul thief",          {XX, XX, XX, 67, XX}, spell_soul_thief,          TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(640),  900, 24, "", "!SOUL THIEF!", "" },
    { REMORT, NORM, "holy avenger",        {XX, XX, 76, XX, XX}, spell_holy_avenger,        TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(641), 1200, 24, "", "!HOLY AVENGER!", "" },
    { REMORT, NORM, "heat armor",          {57, XX, XX, XX, 78}, spell_heat_armor,          TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(642),  350, 12, "Heat Armor", "!Heat Armor!", "" },
    { REMORT, NORM, "retributive strike",  {72, XX, XX, 78, XX}, spell_retri_strike,        TAR_IGNORE,         POS_FIGHTING, NULL,                 SLOT(643),  800, 12, "Retributive strike", "!Retributive Strike!", "" },
    { REMORT, NORM, "lava burst",          {40, XX, XX, XX, XX}, spell_lava_burst,          TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL,                 SLOT(644),  350, 12, "Lava Burst", "!Lava Burst!", "" },
    { REMORT, NORM, "fireshield",          {80, XX, XX, XX, XX}, spell_fireshield,          TAR_CHAR_DEFENSIVE, POS_FIGHTING, &gsn_shield_fire,     SLOT(645),  400, 12, "", "", "" },
    { REMORT, NORM, "iceshield",           {65, XX, XX, XX, XX}, spell_iceshield,           TAR_CHAR_DEFENSIVE, POS_FIGHTING, &gsn_shield_ice,      SLOT(646),  350, 12, "", "", "" },
    { REMORT, NORM, "shockshield",         {55, XX, XX, XX, XX}, spell_shockshield,         TAR_CHAR_DEFENSIVE, POS_FIGHTING, &gsn_shield_shock,    SLOT(647),  400, 12, "", "", "" },
    { REMORT, NORM, "demonshield",         {XX, XX, XX, 80, XX}, spell_demonshield,         TAR_CHAR_DEFENSIVE, POS_FIGHTING, &gsn_shield_demon,    SLOT(671),  400, 12, "", "", "" },
    { REMORT, NORM, "shadowshield",        {XX, XX, XX, XX, XX}, spell_null,                TAR_CHAR_DEFENSIVE, POS_FIGHTING, NULL,                 SLOT(648),  400, 12, "", "", "" },
    { REMORT, NORM, "thoughtshield",       {XX, XX, XX, XX, XX}, spell_null,                TAR_CHAR_DEFENSIVE, POS_FIGHTING, NULL,                 SLOT(649),  400, 12, "", "", "" },
    { REMORT, NORM, "summon pegasus",      {XX, XX, 60, XX, XX}, spell_summon_pegasus,      TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(659), 1200, 24, "", "", "" },
    { REMORT, NORM, "summon nightmare",    {XX, XX, XX, 60, XX}, spell_summon_nightmare,    TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(660), 1200, 24, "", "", "" },
    { REMORT, NORM, "summon beast",        {60, XX, XX, XX, XX}, spell_summon_beast,        TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(661), 1200, 24, "", "", "" },
    { REMORT, NORM, "summon devourer",     {XX, XX, XX, XX, 60}, spell_summon_devourer,     TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(662), 1200, 24, "", "", "" },
    { REMORT, NORM, "summon shadow",       {XX, 60, XX, XX, XX}, spell_summon_shadow,       TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(663), 1200, 24, "", "", "" },
    { REMORT, NORM, "creature bond",       {77, XX, XX, XX, 55}, spell_creature_bond,       TAR_CHAR_DEFENSIVE, POS_FIGHTING, NULL,                 SLOT(664),  100, 12, "", "", "" },
    { REMORT, NORM, "corrupt bond",        {XX, XX, XX, 33, XX}, spell_corrupt_bond,        TAR_CHAR_DEFENSIVE, POS_FIGHTING, NULL,                 SLOT(665),  100, 12, "", "", "" },
    {  ADEPT, NORM, "group heal",          { 5, XX, XX, XX, XX}, spell_group_heal,          TAR_CHAR_DEFENSIVE, POS_FIGHTING, NULL,                 SLOT(666),  200, 12, "", "", "" },
    {  ADEPT, NORM, "carapace",            {15, XX, XX, XX, XX}, spell_carapace,            TAR_CHAR_SELF,      POS_STANDING, NULL,                 SLOT(667),  200, 12, "", "@@gThe skeleton encasing you crumbles to the ground.@@N", "@@g$n's@@g skeletal armour crumbles to the ground.@@N" },
    {  ADEPT, NORM, "summon shadowdragon", {19, XX, XX, XX, XX}, spell_summon_shadowdragon, TAR_IGNORE,         POS_STANDING, NULL,                 SLOT(668),  300, 24, "", "", "" },
    { REMORT, NORM, "grab",                {XX, 25, XX, XX, XX}, spell_null,                TAR_IGNORE,         POS_FIGHTING, &gsn_grab,            SLOT(0),      0, 24, "", "", "" },
    { MORTAL, NORM, "lsd",                 {XX, XX, XX, XX, XX}, spell_lsd,                 TAR_CHAR_DEFENSIVE, POS_STANDING, &gsn_lsd,             SLOT(669),   30, 24, "", "@@NYou feel... less trippy!", "" },
    { REMORT, NORM, "hellfire",            {XX, XX, XX, 75, XX}, spell_hellfire,            TAR_IGNORE,         POS_FIGHTING, NULL,                 SLOT(670),  200, 15, "hellfire", "!hellfire!", "" },
    { REMORT, NORM, "equestrian proficiency", {XX,XX,39,XX, XX}, spell_emount,              TAR_IGNORE,         POS_FIGHTING, &gsn_emount,          SLOT(674),    0,  0, "", "", "" },
    {  ADEPT, NORM, "rune:warning",        {10, XX, XX, XX, XX}, spell_warning_rune,        TAR_IGNORE,         POS_FIGHTING, NULL,                 SLOT(673),  150, 12, "", "@@N@@gYour warning rune %s @@N@@gfades away.@@N\n\r", "" },
    { MORTAL, NORM, "gaze mirror",         {57, XX, XX, XX, XX}, spell_gaze_mirror,         TAR_CHAR_SELF,      POS_STANDING, NULL,                 SLOT(675),   40, 24, "", "", "" },

/* FIVE (5) AVATAR SUB's must be directly following an AVATAR NORM! */

    { AVATAR, NORM, "tranquility",         {XX, XX, XX, XX, XX}, spell_avatar_default,      TAR_CHAR_DEFENSIVE, POS_STANDING, NULL,                 SLOT(700),    0, 12, "", "", "" },
    { AVATAR,  SUB, "tranquility novice",  {1, XX, XX, XX, XX}, spell_tranquility_novice,  TAR_CHAR_DEFENSIVE, POS_STANDING, NULL,                 SLOT(701),   90, 12, "", "", "" },
    { AVATAR,  SUB, "tranquility intermediate", {2,XX,XX,XX,XX},spell_tranquility_intermediate, TAR_CHAR_DEFENSIVE, POS_STANDING, NULL,            SLOT(702),  250, 12, "", "", "" },
    { AVATAR,  SUB, "tranquility advanced",{3, XX, XX, XX, XX}, spell_tranquility_advanced,TAR_CHAR_DEFENSIVE, POS_STANDING, NULL,                 SLOT(703),  400, 12, "", "", "" },
    { AVATAR,  SUB, "tranquility expert",  {4, XX, XX, XX, XX}, spell_tranquility_expert,  TAR_CHAR_DEFENSIVE, POS_STANDING, NULL,                 SLOT(704),  500, 12, "", "", "" },
    { AVATAR,  SUB, "tranquility master",  {5, XX, XX, XX, XX}, spell_tranquility_master,  TAR_CHAR_DEFENSIVE, POS_STANDING, NULL,                 SLOT(705),  650, 12, "", "", "" },

    { AVATAR, NORM, "smokescreen",         {XX, XX, XX, XX, XX}, spell_avatar_default,      TAR_CHAR_DEFENSIVE, POS_STANDING, NULL,                 SLOT(706),    0, 12, "", "", "" },
    { AVATAR,  SUB, "smokescreen novice",  {1, XX, XX, XX, XX}, spell_smokescreen_novice,  TAR_IGNORE,         POS_STANDING, &gsn_smokescreen_novice,       SLOT(707),   90, 12, "", "", "" },
    { AVATAR,  SUB, "smokescreen intermediate", {2,XX,XX,XX,XX},spell_smokescreen_intermediate, TAR_IGNORE,    POS_STANDING, &gsn_smokescreen_intermediate, SLOT(708),  250, 12, "", "", "" },
    { AVATAR,  SUB, "smokescreen advanced",{3, XX, XX, XX, XX}, spell_smokescreen_advanced,TAR_IGNORE,         POS_STANDING, &gsn_smokescreen_advanced,     SLOT(709),  400, 12, "", "", "" },
    { AVATAR,  SUB, "smokescreen expert",  {4, XX, XX, XX, XX}, spell_smokescreen_expert,  TAR_IGNORE,         POS_STANDING, &gsn_smokescreen_expert,       SLOT(710),  500, 12, "", "", "" },
    { AVATAR,  SUB, "smokescreen master",  {5, XX, XX, XX, XX}, spell_smokescreen_master,  TAR_IGNORE,         POS_STANDING, &gsn_smokescreen_master,       SLOT(711),  650, 12, "", "", "" },

    { AVATAR, NORM, "sentry",              {XX, XX, XX, XX, XX}, spell_avatar_default,      TAR_CHAR_DEFENSIVE, POS_STANDING, NULL,                 SLOT(712),    0, 12, "", "", "" },
    { AVATAR,  SUB, "sentry novice",       {1, XX, XX, XX, XX}, spell_sentry_novice,       TAR_IGNORE,         POS_STANDING, &gsn_sentry_novice,            SLOT(713),  90, 12, "", "@@N@@gYour @@N%s@@N@@g sentry fades away.@@N\n\r", "" },
    { AVATAR,  SUB, "sentry intermediate", {2, XX, XX, XX, XX}, spell_sentry_intermediate, TAR_IGNORE,         POS_STANDING, &gsn_sentry_intermediate,      SLOT(714), 250, 12, "", "@@N@@gYour @@N%s@@N@@g sentry fades away.@@N\n\r", "" },
    { AVATAR,  SUB, "sentry advanced",     {3, XX, XX, XX, XX}, spell_sentry_advanced,     TAR_IGNORE,         POS_STANDING, &gsn_sentry_advanced,          SLOT(715), 400, 12, "", "@@N@@gYour @@N%s@@N@@g sentry fades away.@@N\n\r", "" },
    { AVATAR,  SUB, "sentry expert",       {4, XX, XX, XX, XX}, spell_sentry_expert,       TAR_IGNORE,         POS_STANDING, &gsn_sentry_expert,            SLOT(716), 500, 12, "", "@@N@@gYour @@N%s@@N@@g sentry fades away.@@N\n\r", "" },
    { AVATAR,  SUB, "sentry master",       {5, XX, XX, XX, XX}, spell_sentry_master,       TAR_IGNORE,         POS_STANDING, &gsn_sentry_master,            SLOT(717), 650, 12, "", "@@N@@gYour @@N%s@@N@@g sentry fades away.@@N\n\r", "" },

    { AVATAR, NORM, "coldwave",            {XX, XX, XX, XX, XX}, spell_avatar_default,      TAR_CHAR_DEFENSIVE, POS_FIGHTING, NULL,                 SLOT(718),    0, 12, "", "", "" },
    { AVATAR,  SUB, "coldwave novice",     {1, XX, XX, XX, XX}, spell_coldwave_novice,     TAR_IGNORE,         POS_FIGHTING, NULL,                 SLOT(719),   90, 12, "", "", "" },
    { AVATAR,  SUB, "coldwave intermediate",{2, XX, XX, XX, XX},spell_coldwave_intermediate,TAR_IGNORE,        POS_FIGHTING, NULL,                 SLOT(720),  150, 12, "", "", "" },
    { AVATAR,  SUB, "coldwave advanced",   {3, XX, XX, XX, XX}, spell_coldwave_advanced,   TAR_IGNORE,         POS_FIGHTING, NULL,                 SLOT(721),  220, 12, "", "", "" },
    { AVATAR,  SUB, "coldwave expert",     {4, XX, XX, XX, XX}, spell_coldwave_expert,     TAR_IGNORE,         POS_FIGHTING, NULL,                 SLOT(722),  280, 12, "", "", "" },
    { AVATAR,  SUB, "coldwave master",     {5, XX, XX, XX, XX}, spell_coldwave_master,     TAR_IGNORE,         POS_FIGHTING, NULL,                 SLOT(723),  350, 12, "", "", "" },

    { AVATAR, NORM, "stealth",             {XX, XX, XX, XX, XX}, spell_null,               TAR_CHAR_DEFENSIVE, POS_STANDING, &gsn_stealth,         SLOT(724),   0, 12, "", "", "" },
    { AVATAR,  SUB, "stealth novice",      {1, XX, XX, XX, XX}, spell_null,                TAR_IGNORE,         POS_STANDING, &gsn_stealth_novice,  SLOT(725),   0, 12, "", "", "" },
    { AVATAR,  SUB, "stealth intermediate",{2, XX, XX, XX, XX}, spell_null,                TAR_IGNORE,         POS_STANDING, &gsn_stealth_intermediate,  SLOT(726),   0, 12, "", "", "" },
    { AVATAR,  SUB, "stealth advanced",    {3, XX, XX, XX, XX}, spell_null,                TAR_IGNORE,         POS_STANDING, &gsn_stealth_advanced,SLOT(727),   0, 12, "", "", "" },
    { AVATAR,  SUB, "stealth expert",      {4, XX, XX, XX, XX}, spell_null,                TAR_IGNORE,         POS_STANDING, &gsn_stealth_expert,  SLOT(728),   0, 12, "", "", "" },
    { AVATAR,  SUB, "stealth master",      {5, XX, XX, XX, XX}, spell_null,                TAR_IGNORE,         POS_STANDING, &gsn_stealth_master,  SLOT(729),   0, 12, "", "", "" },

    { AVATAR, NORM, "nutrition",             {XX, XX, XX, XX, XX}, spell_null,             TAR_CHAR_DEFENSIVE, POS_STANDING, &gsn_nutrition,         SLOT(730),   0, 12, "", "", "" },
    { AVATAR,  SUB, "nutrition novice",      {1, XX, XX, XX, XX}, spell_null,              TAR_IGNORE,         POS_STANDING, &gsn_nutrition_novice,  SLOT(731),   0, 12, "", "", "" },
    { AVATAR,  SUB, "nutrition intermediate",{2, XX, XX, XX, XX}, spell_null,              TAR_IGNORE,         POS_STANDING, NULL,               SLOT(732),   0, 12, "", "", "" },
    { AVATAR,  SUB, "nutrition advanced",    {3, XX, XX, XX, XX}, spell_null,              TAR_IGNORE,         POS_STANDING, NULL,               SLOT(733),   0, 12, "", "", "" },
    { AVATAR,  SUB, "nutrition expert",      {4, XX, XX, XX, XX}, spell_null,              TAR_IGNORE,         POS_STANDING, NULL,               SLOT(734),   0, 12, "", "", "" },
    { AVATAR,  SUB, "nutrition master",      {5, XX, XX, XX, XX}, spell_null,              TAR_IGNORE,         POS_STANDING, NULL,               SLOT(735),   0, 12, "", "", "" },

    { AVATAR, NORM, "compassion",            {XX, XX, XX, XX, XX}, spell_null,             TAR_CHAR_DEFENSIVE, POS_STANDING, &gsn_compassion,             SLOT(736),   0, 12, "", "", "" },
    { AVATAR,  SUB, "compassion novice",      {1, XX, XX, XX, XX}, spell_null,             TAR_IGNORE,         POS_STANDING, &gsn_compassion_novice,      SLOT(737),   0, 12, "", "", "" },
    { AVATAR,  SUB, "compassion intermediate",{2, XX, XX, XX, XX}, spell_null,             TAR_IGNORE,         POS_STANDING, &gsn_compassion_intermediate,SLOT(738),   0, 12, "", "", "" },
    { AVATAR,  SUB, "compassion advanced",    {3, XX, XX, XX, XX}, spell_null,             TAR_IGNORE,         POS_STANDING, &gsn_compassion_advanced,    SLOT(739),   0, 12, "", "", "" },
    { AVATAR,  SUB, "compassion expert",      {4, XX, XX, XX, XX}, spell_null,             TAR_IGNORE,         POS_STANDING, &gsn_compassion_expert,      SLOT(740),   0, 12, "", "", "" },
    { AVATAR,  SUB, "compassion master",      {5, XX, XX, XX, XX}, spell_null,             TAR_IGNORE,         POS_STANDING, &gsn_compassion_master,      SLOT(741),   0, 12, "", "", "" },

    { AVATAR, NORM, "innerflame",          {XX, XX, XX, XX, XX}, spell_avatar_default,      TAR_CHAR_DEFENSIVE, POS_STANDING, NULL,                 SLOT(742),    0, 12, "", "", "" },
    { AVATAR,  SUB, "innerflame novice",     {1, XX, XX, XX, XX}, spell_innerflame_novice,     TAR_IGNORE,         POS_STANDING, &gsn_innerflame_novice,       SLOT(743),   90, 12, "", "You lose your inner flame.", "" },
    { AVATAR,  SUB, "innerflame intermediate",{2, XX, XX, XX, XX},spell_innerflame_intermediate,TAR_IGNORE,        POS_STANDING, &gsn_innerflame_intermediate, SLOT(744),  250, 12, "", "You lose your inner flame.", "" },
    { AVATAR,  SUB, "innerflame advanced",   {3, XX, XX, XX, XX}, spell_innerflame_advanced,   TAR_IGNORE,         POS_STANDING, &gsn_innerflame_advanced,     SLOT(745),  400, 12, "", "You lose your inner flame.", "" },
    { AVATAR,  SUB, "innerflame expert",     {4, XX, XX, XX, XX}, spell_innerflame_expert,     TAR_IGNORE,         POS_STANDING, &gsn_innerflame_expert,       SLOT(746),  500, 12, "", "You lose your inner flame.", "" },
    { AVATAR,  SUB, "innerflame master",     {5, XX, XX, XX, XX}, spell_innerflame_master,     TAR_IGNORE,         POS_STANDING, &gsn_innerflame_master,       SLOT(747),  650, 12, "", "You lose your inner flame.", "" },

};

#undef XX
