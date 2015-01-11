
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

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#define __USE_BSD
#include <stdio.h>
#undef __USE_BSD
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>
#include <time.h>
#include <assert.h>
#include "merc.h"
#include "ssm.h"
#include "auction.h"
#include "treasury.h"
#include <unistd.h>                /* for execl */
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

IDSTRING(rcsid, "$Id: db.c,v 1.78 2005/02/17 23:52:52 dave Exp $");

#if !defined(macintosh)
extern int _filbuf  args((FILE *));
#endif

/*
 * Globals.
 */
char                log_buf[2 * MAX_INPUT_LENGTH];
KILL_DATA           kill_table[MAX_LEVEL];
TIME_INFO_DATA      time_info;
WEATHER_DATA        weather_info;
bool                booting_up;
bool                mem_log;


sh_int              gsn_adrenaline;
sh_int              gsn_appraise;
sh_int              gsn_backstab;
sh_int              gsn_bash;
sh_int              gsn_berserk;
sh_int              gsn_bite;
sh_int              gsn_black_hand;
sh_int              gsn_blindness;
sh_int              gsn_change_sex;
sh_int              gsn_charge;
sh_int              gsn_charm_person;
sh_int              gsn_circle;
sh_int              gsn_climb;
sh_int              gsn_cloak_absorb;
sh_int              gsn_cloak_adept;
sh_int              gsn_cloak_flaming;
sh_int              gsn_cloak_mana;
sh_int              gsn_cloak_reflect;
sh_int              gsn_cloak_regen;
sh_int              gsn_cure_critical;
sh_int              gsn_cure_light;
sh_int              gsn_cure_serious;
sh_int              gsn_curse;
sh_int              gsn_decapitate;
sh_int              gsn_detect_magic;
sh_int              gsn_dirt;
sh_int              gsn_disarm;
sh_int              gsn_disguise;
sh_int              gsn_divine_intervention;
sh_int              gsn_dodge;
sh_int              gsn_dualwield;
sh_int              gsn_emotion_control;
sh_int              gsn_emount;
sh_int              gsn_enhanced_damage;
sh_int              gsn_feed;
sh_int              gsn_fifth_attack;
sh_int              gsn_find_doors;
sh_int              gsn_fly;
sh_int              gsn_fourth_attack;
sh_int              gsn_frenzy;
sh_int              gsn_grab;
sh_int              gsn_harm;
sh_int              gsn_headbutt;
sh_int              gsn_heal;
sh_int              gsn_hide;
sh_int              gsn_hunt;
sh_int              gsn_hypnosis;
sh_int              gsn_imprint;
sh_int              gsn_instruct;
sh_int              gsn_invis;
sh_int              gsn_kick;
sh_int              gsn_knee;
sh_int              gsn_lsd;
sh_int              gsn_martial_arts;
sh_int              gsn_mass_invis;
sh_int              gsn_mount;
sh_int              gsn_nodisarm;
sh_int              gsn_notrip;
sh_int              gsn_parry;
sh_int              gsn_peek;
sh_int              gsn_pick_lock;
sh_int              gsn_poison;
sh_int              gsn_punch;
sh_int              gsn_rescue;
sh_int              gsn_scent;
sh_int              gsn_scout;
sh_int              gsn_second_attack;
sh_int              gsn_shadow;
sh_int              gsn_shield_block;
sh_int              gsn_shield_demon;
sh_int              gsn_shield_fire;
sh_int              gsn_shield_ice;
sh_int              gsn_shield_shock;
sh_int              gsn_sixth_attack;
sh_int              gsn_sleep;
sh_int              gsn_smash;
sh_int              gsn_sneak;
sh_int              gsn_steal;
sh_int              gsn_stun;
sh_int              gsn_target;
sh_int              gsn_third_attack;
sh_int              gsn_trip;
sh_int              gsn_unit_tactics;
sh_int              gsn_smokescreen_novice;
sh_int              gsn_smokescreen_intermediate;
sh_int              gsn_smokescreen_advanced;
sh_int              gsn_smokescreen_expert;
sh_int              gsn_smokescreen_master;
sh_int              gsn_sentry_novice;
sh_int              gsn_sentry_intermediate;
sh_int              gsn_sentry_advanced;
sh_int              gsn_sentry_expert;
sh_int              gsn_sentry_master;
sh_int              gsn_stealth;
sh_int              gsn_stealth_novice;
sh_int              gsn_stealth_intermediate;
sh_int              gsn_stealth_advanced;
sh_int              gsn_stealth_expert;
sh_int              gsn_stealth_master;
sh_int              gsn_nutrition;
sh_int              gsn_nutrition_novice;
sh_int              gsn_compassion;
sh_int              gsn_compassion_novice;
sh_int              gsn_compassion_intermediate;
sh_int              gsn_compassion_advanced;
sh_int              gsn_compassion_expert;
sh_int              gsn_compassion_master;
sh_int              gsn_innerflame;
sh_int              gsn_innerflame_novice;
sh_int              gsn_innerflame_intermediate;
sh_int              gsn_innerflame_advanced;
sh_int              gsn_innerflame_expert;
sh_int              gsn_innerflame_master;

extern bool         auto_quest;
extern int          avnuminc;

/*
 * Locals.
 */
MOB_INDEX_DATA     *mob_index_hash[MAX_KEY_HASH];
OBJ_INDEX_DATA     *obj_index_hash[MAX_KEY_HASH];
ROOM_INDEX_DATA    *room_index_hash[MAX_KEY_HASH];
char               *string_hash[MAX_KEY_HASH];

AREA_DATA          *area_used[MAX_AREAS];
AREA_DATA          *area_load;

int                 area_revision = 0;

/* 
 * replaced for SSM
 */

#if 0
char               *string_space = NULL;
char               *top_string = NULL;
char                str_empty[1] = { 0 };
#endif

extern char         str_empty[1];
extern long         sOverFlowString;
extern long         nOverFlowString;
extern bool         Full;

int                 top_affect;
int                 top_area;
int                 top_ed;

/* teleport */
int                 top_tele;

int                 top_exit;
int                 top_mob_index;
int                 top_obj_index;
int                 top_reset;
int                 top_room;
int                 top_shop;
int                 top_mprog;
int                 top_mob;
int                 top_obj;

/*
 * MOBprogram locals
 */

int mprog_name_to_type args((char *name));
void mprog_file_read args((char *f, MOB_INDEX_DATA *pMobIndex));
void load_mobprogs  args((FILE * fp));
void mprog_read_programs args((FILE * fp, MOB_INDEX_DATA *pMobIndex));

/*
 * Memory management.
 * Increase MAX_STRING if you have too.
 * Tune the others only if you understand what you're doing.
 */

#if 0

#define                 MAX_STRING      2097152
/*#define                 MAX_PERM_BLOCK  131072
   #define                 MAX_MEM_LIST    12

   void *                  rgFreeList      [MAX_MEM_LIST];
   const int               rgSizeList      [MAX_MEM_LIST]  =
   {
   16, 32, 64, 128, 256, 1024, 2048, 4096, 8192, 16384, 32768-64, 65536-16
   }; */

int                 nAllocString;
int                 sAllocString;
#endif

extern long         nAllocString;
extern long         sAllocString;

int                 nAllocPerm;
int                 sAllocPerm;

extern long         MAX_STRING;
void                init_string_space(void);
void                boot_done(void);

/*
 * Semi-locals.
 */
bool                fBootDb;
FILE               *fpArea;
FILE               *fpPfile;
char                strArea[MAX_INPUT_LENGTH];
char                strPfile[MAX_INPUT_LENGTH];
/*
 * Local booting procedures.
 */
void init_mm        args((void));

void load_area      args((FILE * fp));
void load_helps     args((FILE * fp));
void load_shelps    args((void));
void load_notes     args((void));
void load_mobiles   args((FILE * fp));
void load_objects   args((FILE * fp));
void load_resets    args((FILE * fp));
void load_rooms     args((FILE * fp));
void load_shops     args((FILE * fp));
void load_specials  args((FILE * fp));
void load_objfuns   args((FILE * fp));
void load_gold      args((void));
void load_corpses   args((void));
void load_bans      args((void));
void load_brands    args((void));
void load_mudset    args((void));
void load_renames   args((void));

void fix_exits      args((void));
void check_resets   args((void));

void reset_area     args((AREA_DATA *pArea));
void copyover_recover args((void));

#define SHOW_AREA \
  if (!previous_bug) \
  { \
    bugf("      In %s.", pArea->filename); \
    previous_bug = 1; \
  }

/*
 * Big mama top level function.
 */
void
boot_db(bool fCopyOver)
{
    int                 a, sec, usec;
    struct timeval      tv, tv2;

    /*
     * Init some data space stuff.
     */

    gettimeofday(&tv, NULL);
    init_string_space();
    fBootDb = TRUE;

    send_to_descrips("Initialising Ack! Mud.  Please Wait....\n\r");

    /*
     * Init random number generator.
     */
    {
        init_mm();
    }

    load_mudset();

    /*
     * Set time and weather.
     */
    {
        long                lhour, lday, lmonth;

        lhour = (current_time - 650336715)
            / (PULSE_TICK / PULSE_PER_SECOND);
        time_info.hour = lhour % 24;
        time_info.minute = time_info.hour % 60;
        lday = lhour / 24;
        time_info.day = lday % 35;
        lmonth = lday / 35;
        time_info.month = lmonth % 17;
        time_info.year = lmonth / 17;
        time_info.moon = 0;
        weather_info.moon_loc = MOON_DOWN;
        weather_info.moon_phase = number_range(MOON_NEW, MOON_FULL);

        if (time_info.hour < 5)
            weather_info.sunlight = SUN_DARK;
        else if (time_info.hour < 6)
            weather_info.sunlight = SUN_RISE;
        else if (time_info.hour < 19)
            weather_info.sunlight = SUN_LIGHT;
        else if (time_info.hour < 20)
            weather_info.sunlight = SUN_SET;
        else
            weather_info.sunlight = SUN_DARK;

        weather_info.change = 0;
        weather_info.mmhg = 960;
        if (time_info.month >= 7 && time_info.month <= 12)
            weather_info.mmhg += number_range(1, 50);
        else
            weather_info.mmhg += number_range(1, 80);

        if (weather_info.mmhg <= 980)
            weather_info.sky = SKY_LIGHTNING;
        else if (weather_info.mmhg <= 1000)
            weather_info.sky = SKY_RAINING;
        else if (weather_info.mmhg <= 1020)
            weather_info.sky = SKY_CLOUDY;
        else
            weather_info.sky = SKY_CLOUDLESS;

    }

    /*
     * Assign gsn's for skills which have them.
     */
    {
        int                 sn;

        for (sn = 0; sn < MAX_SKILL; sn++) {
            if (skill_table[sn].pgsn != 0)
                *skill_table[sn].pgsn = sn;
        }
    }

    /* Clear list of used areas */
    for (a = 0; a < MAX_AREAS; a++) {
        area_used[a] = NULL;
    }

    /*
     *   Read in clan data table
     */

    {
        extern POL_DATA     politics_data;

        FILE               *clanfp;
        char                clan_file_name[MAX_STRING_LENGTH];
        sh_int              x, y;
        char                buf[MAX_STRING_LENGTH];

        sprintf(clan_file_name, "%s", CLAN_FILE);

        sprintf(buf, "Loading %s\n\r", clan_file_name);
        monitor_chan(buf, MONITOR_CLAN);

        if ((clanfp = fopen(clan_file_name, "r")) == NULL) {
            bug("Save Clan Table: fopen");
            perror("failed open of clan_table.dat in load_clan_table");
        }
        else {
            fpArea = clanfp;
            sprintf(strArea, "%s", clan_file_name);

            for (x = 1; x < MAX_CLAN; x++) {
                for (y = 1; y < MAX_CLAN; y++) {
                    politics_data.diplomacy[x][y] = fread_number(clanfp);
                }
            }

            for (x = 1; x < MAX_CLAN; x++) {
                politics_data.treasury[x] = fread_number(clanfp);
            }

            for (x = 1; x < MAX_CLAN; x++) {
                for (y = 1; y < MAX_CLAN; y++) {
                    politics_data.end_current_state[x][y] = fread_number(clanfp);
                }
            }

            fclose(clanfp);
        }
        fpArea = NULL;
        sprintf(buf, "Done Loading %s\n\r", clan_file_name);
        monitor_chan(buf, MONITOR_CLAN);

    }

    /*
     * Read in all the socials.
     */
    {
        load_social_table();

    }

    /*
     * Read in all the area files.
     */
    {
        FILE               *fpList;

        send_to_descrips("Reading Area Files...\n\r");

        if ((fpList = fopen(AREA_LIST, "r")) == NULL) {
            perror(AREA_LIST);
            raise(SIGQUIT);
        }

        for (;;) {
            strcpy(strArea, fread_word(fpList));
            if (strArea[0] == '$')
                break;

            if (strArea[0] == '-') {
                fpArea = stdin;
            }
            else {
                if ((fpArea = fopen(strArea, "r")) == NULL) {
                    log_string(strArea);
                    raise(SIGQUIT);
                }
            }

            for (;;) {
                char               *word;

                if (fread_letter(fpArea) != '#') {
                    bug("Boot_db: # not found.");
                    raise(SIGQUIT);
                }

                word = fread_word(fpArea);

                if (word[0] == '$')
                    break;
                else if (!str_cmp(word, "AREA"))
                    load_area(fpArea);
                else if (!str_cmp(word, "HELPS"))
                    load_helps(fpArea);
                else if (!str_cmp(word, "MOBILES"))
                    load_mobiles(fpArea);
                else if (!str_cmp(word, "MOBPROGS"))
                    load_mobprogs(fpArea);
                else if (!str_cmp(word, "OBJECTS"))
                    load_objects(fpArea);
                else if (!str_cmp(word, "RESETS"))
                    load_resets(fpArea);
                else if (!str_cmp(word, "ROOMS"))
                    load_rooms(fpArea);
                else if (!str_cmp(word, "SHOPS"))
                    load_shops(fpArea);
                else if (!str_cmp(word, "SPECIALS"))
                    load_specials(fpArea);
                else if (!str_cmp(word, "OBJFUNS"))
                    load_objfuns(fpArea);
                else {
                    bug("Boot_db: bad section name.");
                    exit(1);
                }
            }

            if (fpArea != stdin)
                fclose(fpArea);
            fpArea = NULL;
        }
        fclose(fpList);
    }

    /*
     * Fix up exits.
     * MAG Mod: Check resets. (Allows loading objects of later areas.)
     * MAG Mod: Load gold for areas.
     * Declare db booting over.
     * Reset all areas once.
     * Load up the notes file.
     * Set the MOBtrigger.
     */
    {
        send_to_descrips("Fixing Exits......");
        fix_exits();
        send_to_descrips("Done.\n\r");
        fBootDb = FALSE;
        send_to_descrips("Checking Resets...");
        check_resets();
        send_to_descrips("Done.\n\r");
        /*  load_gold(); */
#ifndef BPORT
        send_to_descrips("Updating Areas....");
        area_update();
#endif
        send_to_descrips("Done.\n\r");
        MOBtrigger = TRUE;
        load_corpses();
        booting_up = TRUE;
        booting_up = FALSE;
        load_bans();
        load_rulers();
        load_brands();
#ifndef BPORT
        load_shelps();
#endif
        load_notes();
#ifndef BPORT
        load_cinfo();
#endif

        load_auctions();
        load_renames();
        load_treasuries();

    }
#ifndef BPORT
    {
        extern bool         dbl_xp;

        auto_quest = TRUE;
        dbl_xp = TRUE;
    }
#else
    {
        extern int          wizlock;

        wizlock = TRUE;
        auto_quest = FALSE;
    }
#endif

    {
        LOCKER_DATA *locker;
        int dbsave = fBootDb;

        fBootDb = TRUE;
        for (locker = first_locker; locker != NULL; locker = locker->next)
            load_locker(locker->room);

        fBootDb = dbsave;
    }

    if (fCopyOver) {
        extern bool         disable_timer_abort;

        disable_timer_abort = TRUE;
        copyover_recover();
        disable_timer_abort = FALSE;
    }

    gettimeofday(&tv2, NULL);
    timeval_diff(&tv, &tv2, &sec, &usec);
    xlogf("boot_db(): took %d seconds, %d microseconds", sec, usec);
    return;
}

/*
 * Snarf an 'area' header line.
 */
void
load_area(FILE * fp)
{
    AREA_DATA          *pArea;
    char                letter;
    int                 a;

    /* BUG: reset area revision, in case an area doesn't have a revision set */
    area_revision = 0;

    GET_FREE(pArea, area_free);
    pArea->first_reset = NULL;
    pArea->last_reset = NULL;
    pArea->name = fread_string(fp);
    pArea->age = 15;
    pArea->reset_rate = 15;
    pArea->nplayer = 0;
    pArea->offset = 0;
    /* MAG Mod */
    pArea->modified = 0;
    pArea->min_vnum = 0;
    pArea->max_vnum = MAX_VNUM;
    pArea->area_num = 0;
    pArea->filename = str_dup(strArea);
    pArea->owner = str_empty;
    pArea->can_read = str_dup("all");
    pArea->can_write = str_dup("all");
    pArea->level_label = str_dup("{?? ??}");
    pArea->keyword = str_dup("none");
    pArea->reset_msg = str_dup("You hear the screams of the Dead within your head.");
    pArea->nocmd = str_dup("");
    pArea->nospell = str_dup("");
    pArea->min_level = 0;
    pArea->max_level = 0;
    pArea->gold = 0;
    pArea->flags = 0;
    pArea->first_area_room = NULL;
    pArea->last_area_room = NULL;
    pArea->first_area_object = NULL;
    pArea->last_area_object = NULL;
    pArea->first_area_mobile = NULL;
    pArea->last_area_mobile = NULL;
    pArea->first_area_shop = NULL;
    pArea->last_area_shop = NULL;
    pArea->first_area_mobprog = NULL;

    /* MAG Mod for optionals additions to area headers. */
    for (;;) {
        letter = fread_letter(fp);
        if (letter == '#') {
            ungetc(letter, fp);
            break;
        }

        switch (letter) {
            case 'Z':
                area_revision = fread_number(fp);
                break;
            case 'C':
                pArea->avnum = fread_number(fp);
                break;
            case 'j':
                pArea->nocmd = fread_string(fp);
                break;
            case 'J':
                pArea->nospell = fread_string(fp);
                break;
            case 'F':
                pArea->reset_rate = fread_number(fp);
                break;
            case 'O':
                pArea->owner = fread_string(fp);
                break;

            case 'U':
                free_string(pArea->reset_msg);
                pArea->reset_msg = fread_string(fp);
                break;
            case 'R':
                free_string(pArea->can_read);
                pArea->can_read = fread_string(fp);
                break;
            case 'W':
                free_string(pArea->can_write);
                pArea->can_write = fread_string(fp);
                break;
            case 'P':
                SET_BIT(pArea->flags, AREA_PAYAREA);
                fread_to_eol(fp);
                break;
            case 'M':
                SET_BIT(pArea->flags, AREA_NO_ROOM_AFF);
                fread_to_eol(fp);
                break;

            case 'X':
                pArea->offset = fread_number(fp);
                break;
            case 'V':
                pArea->min_vnum = fread_number(fp);
                pArea->max_vnum = fread_number(fp);
                break;
            case 'N':
                pArea->area_num = fread_number(fp);
                break;
            case 'T':
                SET_BIT(pArea->flags, AREA_TELEPORT);
                fread_to_eol(fp);
                break;
            case 'B':
                SET_BIT(pArea->flags, AREA_BUILDING);
                fread_to_eol(fp);
                break;
            case 'A':
                SET_BIT(pArea->flags, AREA_BUILDVISIT);
                fread_to_eol(fp);
                break;
            case 'E':
                SET_BIT(pArea->flags, AREA_NOENERGY);
                fread_to_eol(fp);
                break;
            case 'S':
                SET_BIT(pArea->flags, AREA_NOSHOW);
                fread_to_eol(fp);
                break;
            case 'K':
                free_string(pArea->keyword);
                pArea->keyword = fread_string(fp);
                break;
            case 'L':
                free_string(pArea->level_label);
                pArea->level_label = fread_string(fp);
                break;
            case 'I':
                pArea->min_level = fread_number(fp);
                pArea->max_level = fread_number(fp);
                break;

        }
    }

    if (pArea->area_num == 0) {
        /* Find a unique area number */
        for (a = 0; a < MAX_AREAS; a++)
            if (area_used[a] == NULL)
                break;
        pArea->area_num = a;
    }

    if (area_revision < 3) {
        pArea->avnum = avnuminc;
        avnuminc++;
        save_mudsets();
        area_modified(pArea);
    }

    area_used[pArea->area_num] = pArea;
    area_load = pArea;

    LINK(pArea, first_area, last_area, next, prev);

    top_area++;
    return;
}

/*
 * Snarf a help section.
 */
void
load_helps(FILE * fp)
{
    char buf[MSL];

    for (;;) {
        fread_number(fp);
        temp_fread_string(fp, buf);
        if (buf[0] == '$')
            break;

        temp_fread_string(fp, buf);
    }

    return;
}

void
load_corpses(void)
{

    FILE               *corpsefp;
    char                corpse_file_name[MAX_STRING_LENGTH];
    char                buf[MAX_STRING_LENGTH];

    sprintf(corpse_file_name, "%s", CORPSE_FILE);

    sprintf(buf, "Loading %s\n\r", corpse_file_name);
    monitor_chan(buf, MONITOR_CLAN);

    if ((corpsefp = fopen(corpse_file_name, "r")) == NULL) {
        bug("Load corpse Table: fopen");
        perror("failed open of corpse_table.dat in load_corpse_table");
    }

    else {
        fpArea = corpsefp;
        sprintf(strArea, "%s", corpse_file_name);

        for (;;)
        {
            char                letter;
            char               *word;

            letter = fread_letter(corpsefp);
            if (letter == '*') {
                fread_to_eol(corpsefp);
                continue;
            }

            if (letter != '#') {
                monitor_chan("Load_char_obj: # not found.", MONITOR_BAD);
                break;
            }

            word = fread_word(corpsefp);
            if (!str_cmp(word, "OBJECT"))
                fread_corpse(corpsefp);
            else if (!str_cmp(word, "END"))
                break;
            else {
                monitor_chan("Load_char_obj: bad section.", MONITOR_BAD);
                break;
            }
        }

    }

    fclose(corpsefp);
    fpArea = NULL;
    sprintf(buf, "Done Loading %s\n\r", corpse_file_name);
    monitor_chan(buf, MONITOR_CLAN);

}

void
load_bans(void)
{

    FILE               *bansfp;
    char                bans_file_name[MAX_STRING_LENGTH];
    char                buf[MAX_STRING_LENGTH];

    sprintf(bans_file_name, "%s", BANS_FILE);

    sprintf(buf, "Loading %s\n\r", bans_file_name);
    monitor_chan(buf, MONITOR_CLAN);

    if ((bansfp = fopen(bans_file_name, "r")) == NULL) {
        bug("Load bans Table: fopen");
        perror("failed open of bans_table.dat in load_bans_table");
    }
    else {
        fpArea = bansfp;
        sprintf(strArea, "%s", bans_file_name);

        for (;;) {
            char               *word;
            BAN_DATA           *pban;

            word = fread_string(bansfp);
            if (!str_cmp(word, "#BAN")) {
                sh_int              get_bool = FALSE;

                GET_FREE(pban, ban_free);
                get_bool = fread_number(bansfp);
                if (get_bool == 1)
                    pban->newbie = TRUE;
                else
                    pban->newbie = FALSE;
                pban->name = fread_string(bansfp);
                pban->banned_by = fread_string(bansfp);
                LINK(pban, first_ban, last_ban, next, prev);
                free_string(word);
            }
            else if (!str_cmp(word, "#END")) {
                free_string(word);
                break;
            }
            else {
                free_string(word);
                monitor_chan("Load_bans: bad section.", MONITOR_BAD);
                break;
            }
        }

        fclose(bansfp);
        fpArea = NULL;
        sprintf(buf, "Done Loading %s\n\r", bans_file_name);
        monitor_chan(buf, MONITOR_CLAN);

    }
}

/*
 * Snarf a mob section.
 */
void
load_mobiles(FILE * fp)
{
    MOB_INDEX_DATA     *pMobIndex;
    BUILD_DATA_LIST    *pList;

    for (;;) {
        sh_int              vnum;
        char                letter;
        int                 iHash;

        letter = fread_letter(fp);
        if (letter != '#') {
            bug("Load_mobiles: # not found.");
            hang("Loading Mobiles in db.c");
        }

        vnum = fread_number(fp);
        if (vnum == 0)
            break;

        fBootDb = FALSE;
        if (get_mob_index(vnum) != NULL) {
            bugf("Load_mobiles: vnum %d duplicated.", vnum);
            hang("Loading Mobiles in db.c");
        }
        fBootDb = TRUE;

        GET_FREE(pMobIndex, mid_free);
        pMobIndex->vnum = vnum;
        pMobIndex->area = area_load;
        pMobIndex->player_name = fread_string(fp);
        pMobIndex->short_descr = fread_string(fp);
        pMobIndex->long_descr = fread_string(fp);
        pMobIndex->description = fread_string(fp);

        pMobIndex->long_descr[0] = UPPER(pMobIndex->long_descr[0]);
        pMobIndex->description[0] = UPPER(pMobIndex->description[0]);

        pMobIndex->act = fread_number(fp) | ACT_IS_NPC;
        pMobIndex->affected_by = fread_number(fp);
        pMobIndex->pShop = NULL;
        pMobIndex->alignment = fread_number(fp);
        letter = fread_letter(fp);
        pMobIndex->level = fread_number(fp);

        pMobIndex->sex = fread_number(fp);

        pMobIndex->ac_mod = fread_number(fp);    /* read      */
        pMobIndex->hr_mod = fread_number(fp);    /* in the    */
        pMobIndex->dr_mod = fread_number(fp);    /* modifiers */

        if (area_revision > 1) {
            pMobIndex->hp_mod   = fread_number(fp);
            pMobIndex->mana_mod = fread_number(fp);
        }
        else {
            pMobIndex->hp_mod   = 0;
            pMobIndex->mana_mod = 0;
        }

        if (letter != 'S') {
            bugf("Load_mobiles: vnum %d non-S.", vnum);
            hang("Loading Mobiles in db.c");
        }

        /* The "new" additions to mobs start with a ! */
        letter = fread_letter(fp);
        if (letter == '!') {
            pMobIndex->class = fread_number(fp);
            pMobIndex->clan = fread_number(fp);
            pMobIndex->race = fread_number(fp);
            pMobIndex->position = POS_STANDING;
            fread_number(fp);    /* position */
            pMobIndex->skills = fread_number(fp);
            pMobIndex->cast = fread_number(fp);
            pMobIndex->def = fread_number(fp);
        }
        else
            ungetc(letter, fp);

        letter = fread_letter(fp);
        if (letter == 'X') {
            pMobIndex->custom_xp = fread_number(fp);
        }
        else {
            pMobIndex->custom_xp = -1;
            ungetc(letter, fp);
        }

        letter = fread_letter(fp);
        if (letter == 'Y') {
            pMobIndex->custom_minlev = fread_number(fp);
            pMobIndex->custom_maxlev = fread_number(fp);
        }
        else {
            pMobIndex->custom_minlev = -1;
            pMobIndex->custom_maxlev = -1;
            ungetc(letter, fp);
        }

        if ((letter = fread_letter(fp)) == 'P')
            pMobIndex->path = fread_string(fp);
        else {
            pMobIndex->path = NULL;
            ungetc(letter, fp);
        }

        if ((letter = fread_letter(fp)) == 'S')
            pMobIndex->spec_fun = spec_lookup(fread_string(fp));
        else {
            pMobIndex->spec_fun = NULL;
            ungetc(letter, fp);
        }

        letter = fread_letter(fp);
        if (letter == '>') {
            ungetc(letter, fp);
            mprog_read_programs(fp, pMobIndex);
        }
        else
            ungetc(letter, fp);

        iHash = vnum % MAX_KEY_HASH;
        SING_TOPLINK(pMobIndex, mob_index_hash[iHash], next);
        /* MAG Mod */
        GET_FREE(pList, build_free);
        pList->data = pMobIndex;
        LINK(pList, area_load->first_area_mobile, area_load->last_area_mobile, next, prev);

        top_mob_index++;
        kill_table[URANGE(0, pMobIndex->level, MAX_LEVEL - 1)].number++;
    }

    return;
}

/*
 * Snarf an obj section.
 */
void
load_objects(FILE * fp)
{
    OBJ_INDEX_DATA     *pObjIndex;
    BUILD_DATA_LIST    *pList;

    for (;;) {
        sh_int              vnum;
        char                letter;
        int                 iHash;

        letter = fread_letter(fp);
        if (letter != '#') {
            bug("Load_objects: # not found.");
            hang("Loading Objects in db.c");
        }

        vnum = fread_number(fp);
        if (vnum == 0)
            break;

        fBootDb = FALSE;
        if (get_obj_index(vnum) != NULL) {
            bug("Load_objects: vnum %d duplicated.");
            hang("Loading Objects in db.c");
        }
        fBootDb = TRUE;

        GET_FREE(pObjIndex, oid_free);
        pObjIndex->vnum = vnum;
        pObjIndex->area = area_load;
        pObjIndex->name = fread_string(fp);
        pObjIndex->short_descr = fread_string(fp);
        pObjIndex->description = fread_string(fp);

        pObjIndex->short_descr[0] = LOWER(pObjIndex->short_descr[0]);
        pObjIndex->description[0] = UPPER(pObjIndex->description[0]);

        pObjIndex->item_type = fread_number(fp);
        pObjIndex->extra_flags = fread_number(fp);
        pObjIndex->wear_flags = fread_number(fp);
        pObjIndex->item_apply = fread_number(fp);
        pObjIndex->value[0] = fread_number(fp);
        pObjIndex->value[1] = fread_number(fp);
        pObjIndex->value[2] = fread_number(fp);
        pObjIndex->value[3] = fread_number(fp);
        pObjIndex->weight = fread_number(fp);
        pObjIndex->cost = 0;
        pObjIndex->newcost = FALSE;
        pObjIndex->rarity = 0;
        pObjIndex->count = 0;

        if (pObjIndex->item_type == ITEM_POTION)
            SET_BIT(pObjIndex->extra_flags, ITEM_NODROP);

        for (;;) {
            char                letter;

            letter = fread_letter(fp);

            if (letter == 'A') {
                AFFECT_DATA        *paf;

                GET_FREE(paf, affect_free);
                paf->type = -1;
                paf->duration = -1;
                paf->location = fread_number(fp);
                paf->modifier = fread_number(fp);
                paf->bitvector = 0;
                paf->save = TRUE;
                paf->caster = NULL;
                LINK(paf, pObjIndex->first_apply, pObjIndex->last_apply, next, prev);
                top_affect++;
            }

            else if (letter == 'E') {
                EXTRA_DESCR_DATA   *ed;

                GET_FREE(ed, exdesc_free);
                ed->keyword = fread_string(fp);
                ed->description = fread_string(fp);
                LINK(ed, pObjIndex->first_exdesc, pObjIndex->last_exdesc, next, prev);
                top_ed++;
            }

            else if (letter == 'L') {
                pObjIndex->level = fread_number(fp);
            }

            /* alternative cost for individual objects */
            else if (letter == 'C') {
                pObjIndex->cost = fread_number(fp);
                pObjIndex->newcost = TRUE;
            }

            /* rarity level of obj */
            else if (letter == 'R') {
                pObjIndex->rarity = fread_number(fp);
            }
            else if (letter == 'O') {
                pObjIndex->obj_fun = obj_fun_lookup(fread_string(fp));
            }
            else {
                ungetc(letter, fp);
                break;
            }
        }

        /*
         * Translate spell "slot numbers" to internal "skill numbers."
         */
        switch (pObjIndex->item_type) {
            case ITEM_PILL:
            case ITEM_POTION:
            case ITEM_SCROLL:
                pObjIndex->value[1] = slot_lookup(pObjIndex->value[1]);
                pObjIndex->value[2] = slot_lookup(pObjIndex->value[2]);
                pObjIndex->value[3] = slot_lookup(pObjIndex->value[3]);
                break;

            case ITEM_STAFF:
            case ITEM_WAND:
                pObjIndex->value[3] = slot_lookup(pObjIndex->value[3]);
                break;
        }

        iHash = vnum % MAX_KEY_HASH;
        SING_TOPLINK(pObjIndex, obj_index_hash[iHash], next);
        /* MAG Mod */
        GET_FREE(pList, build_free);
        pList->data = pObjIndex;
        LINK(pList, area_load->first_area_object, area_load->last_area_object, next, prev);

        top_obj_index++;
    }

    return;
}

/*
 * Snarf a reset section.
 */

/* Spec: it isn't obvious how this works, so I'll add a comment here.
 *
 * load_resets only allows resets that are placed in current rooms - this is
 * the reset_ok code here. It does _not_ check the validity of room and object
 * vnums. It handles linking the resets into the appropriate room-reset-list
 * for OLC.
 *
 * check_resets is called after all areas are loaded. It checks that all
 * resets point to valid mobs and object and spits errors as necessary. It
 * does _not_ touch the room lists any more!
 */
void
load_resets(FILE * fp)
{
    RESET_DATA         *pReset = NULL;
    BUILD_DATA_LIST    *pList = NULL;
    ROOM_INDEX_DATA    *last_mob_room;
    ROOM_INDEX_DATA    *last_obj_room;

    last_mob_room = NULL;
    last_obj_room = NULL;

    if (area_load == NULL) {
        bug("Load_resets: no #AREA seen yet.");
        hang("Loading Resets in db.c");
    }

    for (;;) {
        ROOM_INDEX_DATA    *pRoomIndex;

        /* EXIT_DATA *pexit; unused? */
        char                letter;

        /* Temp values to hold reset, while we see if valid */
        char                Tcommand;
        int                 Tifflag;
        int                 Targ1;
        int                 Targ2;
        int                 Targ3;
        bool                reset_ok;

        reset_ok = FALSE;

        if ((letter = fread_letter(fp)) == 'S')
            break;

        if (letter == '*') {
            fread_to_eol(fp);
            continue;
        }

        Tcommand = letter;
        /* MAG Mod */

        Tifflag = fread_number(fp);    /* Not used */
        Targ1 = fread_number(fp);
        Targ2 = fread_number(fp);
        Targ3 = (letter == 'G' || letter == 'R')
            ? 0 : fread_number(fp);

        pRoomIndex = NULL;        /* Set it to check for room numbers */
        /*
         * Validate parameters.
         * We're calling the index functions for the side effect.
         */
        switch (letter) {
            default:
                bugf("Load_resets: bad command '%c'.", letter);
                reset_ok = FALSE;
                break;

            case 'A':
                reset_ok = FALSE;
                break;            /* obsolete reset, discard */
            case 'M':
                if ((pRoomIndex = get_room_index(Targ3)) != NULL) {
                    reset_ok = TRUE;
                    last_mob_room = pRoomIndex;
                }
                break;
            case 'O':
                if ((pRoomIndex = get_room_index(Targ3)) != NULL) {
                    reset_ok = TRUE;
                    last_obj_room = pRoomIndex;
                }
                break;
            case 'P':            /* apparently these resets are no longer in use */
                /* if ( last_obj_room )
                   {
                   pRoomIndex = last_obj_room;
                   reset_ok = TRUE;
                   } */
                break;
            case 'G':
            case 'E':
            case 'C':
                if (last_mob_room) {
                    pRoomIndex = last_mob_room;
                    reset_ok = TRUE;
                }
                break;

            case 'D':
                if ((pRoomIndex = get_room_index(Targ1)) != NULL) {
                    reset_ok = TRUE;
                }
                break;
            case 'R':
                if ((pRoomIndex = get_room_index(Targ1)) != NULL) {
                    reset_ok = TRUE;
                }
                break;

        }

        if (reset_ok) {            /* i.e. only add if reset is valid. */

            GET_FREE(pReset, reset_free);
            pReset->command = Tcommand;
            pReset->arg1 = Targ1;
            pReset->arg2 = Targ2;
            pReset->arg3 = Targ3;
            pReset->notes = fsave_to_eol(fp);

            LINK(pReset, area_load->first_reset, area_load->last_reset, next, prev);
            top_reset++;
            GET_FREE(pList, build_free);
            pList->data = pReset;
            LINK(pList, pRoomIndex->first_room_reset, pRoomIndex->last_room_reset, next, prev);

        }
    }

    return;
}

/*
 * Snarf a room section.
 */
void
load_rooms(FILE * fp)
{
    ROOM_INDEX_DATA    *pRoomIndex;
    BUILD_DATA_LIST    *pList;

    if (area_load == NULL) {
        bug("Load_rooms: no #AREA seen yet.");
        hang("Loading Rooms in db.c");
    }

    for (;;) {
        sh_int              vnum;
        char                letter;
        int                 door;
        int                 iHash;

        letter = fread_letter(fp);
        if (letter != '#') {
            bug("Load_rooms: # not found.");
            hang("Loading Rooms in db.c");
        }

        vnum = fread_number(fp);
        if (vnum == 0)
            break;

        fBootDb = FALSE;
        if (get_room_index(vnum) != NULL) {
            bugf("Load_rooms: vnum %d duplicated.", vnum);
            hang("Loading Rooms in db.c");
        }
        fBootDb = TRUE;

        GET_FREE(pRoomIndex, rid_free);
        pRoomIndex->first_person = NULL;
        pRoomIndex->last_person = NULL;
        pRoomIndex->first_content = NULL;
        pRoomIndex->last_content = NULL;
        pRoomIndex->first_exdesc = NULL;
        pRoomIndex->last_exdesc = NULL;
        pRoomIndex->area = area_load;
        pRoomIndex->vnum = vnum;
        pRoomIndex->name = fread_string(fp);
        pRoomIndex->description = fread_string(fp);
        pRoomIndex->room_flags = fread_number(fp);
        pRoomIndex->sector_type = fread_number(fp);
        pRoomIndex->nocmd = str_dup("");
        pRoomIndex->nospell = str_dup("");
        pRoomIndex->tele = NULL;
        pRoomIndex->locker = NULL;
        pRoomIndex->light = 0;
        pRoomIndex->affected_by = ROOM_BV_NONE;
        pRoomIndex->first_room_affect = NULL;
        pRoomIndex->last_room_affect = NULL;
        for (door = 0; door <= 5; door++)
            pRoomIndex->exit[door] = NULL;
        pRoomIndex->first_room_reset = NULL;    /* MAG Mod */
        pRoomIndex->last_room_reset = NULL;

        for (;;) {
            letter = fread_letter(fp);

            if (letter == 'S')
                break;

            if (letter == 'D') {
                EXIT_DATA          *pexit;
                int                 locks;

                door = fread_number(fp);
                if (door < 0 || door > 5) {
                    bugf("Fread_rooms: vnum %d has bad door number.", vnum);
                    hang("Loading Rooms in db.c");
                }

                GET_FREE(pexit, exit_free);
                pexit->description = fread_string(fp);
                pexit->keyword = fread_string(fp);
                pexit->exit_info = 0;
                locks = fread_number(fp);
                pexit->key = fread_number(fp);
                pexit->vnum = fread_number(fp);

                /* -S- Mod:  exit_flags now saved as-is, with EX_CLOSED and
                   EX_LOCKED filtered out.  If locks is 2, it's from an
                   'old' area, so handle it, otherwise just set exit_info.
                   This allows new exit flags to be easily added.  :) 
                   (Impossible for new area to have locks  = 2(closed) */

                if (locks == 2)
                    pexit->exit_info = EX_ISDOOR | EX_PICKPROOF;
                else
                    pexit->exit_info = locks;

                pRoomIndex->exit[door] = pexit;
                top_exit++;
            }
            else if (letter == 'E') {
                EXTRA_DESCR_DATA   *ed;

                GET_FREE(ed, exdesc_free);
                ed->keyword = fread_string(fp);
                ed->description = fread_string(fp);
                LINK(ed, pRoomIndex->first_exdesc, pRoomIndex->last_exdesc, next, prev);
                top_ed++;
            }

            /* teleport */
            else if (letter == 'T') {

                TELEPORT_DATA      *tele;

                GET_FREE(tele, tele_free);

                tele->flags = fread_number(fp);
                tele->vnum = fread_number(fp);
                tele->wait = fread_number(fp);
                tele->in = fread_string(fp);
                tele->out = fread_string(fp);
                tele->room = pRoomIndex;

                pRoomIndex->tele = tele;

                LINK(tele, first_tele, last_tele, next, prev);

                top_tele++;
            }
            else if (letter == 'L') {
                LOCKER_DATA      *locker;
                int cnt;

                GET_FREE(locker, locker_free);

                locker->room       = pRoomIndex;
                locker->flags      = fread_number(fp);
                locker->maxitem    = fread_number(fp);
                locker->maxweight  = fread_number(fp);
                locker->valid      = fread_number(fp) ? TRUE : FALSE;

                for (cnt = 0; cnt < MAX_ITEM; cnt++)
                    if (fread_number(fp))
                        locker->types[cnt] = TRUE;
                    else
                        locker->types[cnt] = FALSE;

                pRoomIndex->locker = locker;

                LINK(locker, first_locker, last_locker, next, prev);
            }
            else if (letter == 'C') {
                if (pRoomIndex->nocmd)
                    free_string(pRoomIndex->nocmd);

                pRoomIndex->nocmd = fread_string(fp);
            }
            else if (letter == 'X') {
                if (pRoomIndex->nospell)
                    free_string(pRoomIndex->nospell);

                pRoomIndex->nospell = fread_string(fp);
            }

            else {
                bugf("Load_rooms: vnum %d has flag not 'DES'.", vnum);
                hang("Loading Rooms in db.c");
            }
        }

        iHash = vnum % MAX_KEY_HASH;
        SING_TOPLINK(pRoomIndex, room_index_hash[iHash], next);
        /* MAG Mod */
        GET_FREE(pList, build_free);
        pList->data = pRoomIndex;
        LINK(pList, area_load->first_area_room, area_load->last_area_room, next, prev);

        top_room++;
    }

    return;
}

/*
 * Snarf a shop section.
 */
void
load_shops(FILE * fp)
{
    SHOP_DATA          *pShop;
    BUILD_DATA_LIST    *pList;

    for (;;) {
        MOB_INDEX_DATA     *pMobIndex;
        int                 iTrade;
        int                 keeper;

        keeper = fread_number(fp);
        if (keeper == 0)
            break;
        GET_FREE(pShop, shop_free);
        pShop->keeper = keeper;
        for (iTrade = 0; iTrade < MAX_TRADE; iTrade++)
            pShop->buy_type[iTrade] = fread_number(fp);
        pShop->profit_buy = fread_number(fp);
        pShop->profit_sell = fread_number(fp);
        pShop->open_hour = fread_number(fp);
        pShop->close_hour = fread_number(fp);
        fread_to_eol(fp);
        pMobIndex = get_mob_index(pShop->keeper);
        pMobIndex->pShop = pShop;

        /* MAG Mod */
        GET_FREE(pList, build_free);
        pList->data = pShop;
        LINK(pList, area_load->first_area_shop, area_load->last_area_shop, next, prev);

        LINK(pShop, first_shop, last_shop, next, prev);
        top_shop++;
    }

    return;
}

/*
 * Snarf spec proc declarations.
 */
void
load_specials(FILE * fp)
{
    for (;;) {
        MOB_INDEX_DATA     *pMobIndex;
        char                letter;

        switch (letter = fread_letter(fp)) {
            default:
                bugf("Load_specials: letter '%c' not *, M, or S.", letter);
                hang("Loading Specials in db.c");

            case 'S':
                return;

            case '*':
                break;

            case 'M':
                pMobIndex = get_mob_index(fread_number(fp));
                pMobIndex->spec_fun = spec_lookup(fread_word(fp));
                if (pMobIndex->spec_fun == 0) {
                    bugf("Load_specials(mob): 'M': vnum %d.", pMobIndex->vnum);
                }
                break;
        }
        /* NB. Comments will not be saved when using areasave - MAG. */
        fread_to_eol(fp);
    }
}

/* Snarf objfun declarations.  -S- */

void
load_objfuns(FILE * fp)
{
    for (;;) {
        OBJ_INDEX_DATA     *pObj;
        char                letter;

        switch (letter = fread_letter(fp)) {
            default:
                bugf("Load_objfuns: letter '%c' not *, O or S.", letter);
                hang("Loading Objfuns in db.c");

            case 'S':
                return;

            case '*':
                break;

            case 'O':            /* -S- mod => obj_funs */
                pObj = get_obj_index(fread_number(fp));
                if (pObj == NULL) {
                    fread_word(fp);
                    break;
                }

                pObj->obj_fun = obj_fun_lookup(fread_word(fp));
                if (pObj->obj_fun == 0) {
                    bugf("Load_specials(obj): 'X': vnum %d.", pObj->vnum);
                }
                break;

        }
        /* NB. Comments will not be saved when using areasave - MAG. */
        fread_to_eol(fp);
    }
}

/*
 * Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 * Check for bad reverse exits.
 */
void
fix_exits(void)
{
    /*    extern const sh_int rev_dir []; */
    ROOM_INDEX_DATA    *pRoomIndex;

    /*    ROOM_INDEX_DATA *to_room; */
    EXIT_DATA          *pexit;

    /*    EXIT_DATA *pexit_rev; */
    int                 iHash;
    int                 door;

    for (iHash = 0; iHash < MAX_KEY_HASH; iHash++) {
        for (pRoomIndex = room_index_hash[iHash]; pRoomIndex != NULL; pRoomIndex = pRoomIndex->next) {
            bool                fexit;

            fexit = FALSE;
            for (door = 0; door <= 5; door++) {
                if ((pexit = pRoomIndex->exit[door]) != NULL) {

                    if ((pexit->vnum <= 0)
                        || (get_room_index(pexit->vnum) == NULL)) {
                        bugf("Bad exit vnum %d in room %d", pexit->vnum, pRoomIndex->vnum);
                        free_string(pexit->description);
                        free_string(pexit->keyword);
                        PUT_FREE(pexit, exit_free);
                        pRoomIndex->exit[door] = NULL;
                        /*          pexit->to_room = NULL;   */
                    }

                    else {
                        fexit = TRUE;
                        pexit->to_room = get_room_index(pexit->vnum);
                    }
                }
            }

            if (!fexit)
                SET_BIT(pRoomIndex->room_flags, ROOM_NO_MOB);
        }
    }

    return;
}

/* Load up gold into areas */
void
load_gold(void)
{
    FILE               *fpArea;
    int                 area_num;
    int                 gold;
    AREA_DATA          *pArea;

    fpArea = fopen("area.gld", "r");

    if (fpArea == NULL) {
        bug("Could not open area.gld for reading.");
        return;
    }

    for (;;) {
        area_num = fread_number(fpArea);
        if (area_num == -1)
            break;

        gold = fread_number(fpArea);

        if ((pArea = area_used[area_num]) != NULL)
            pArea->gold = gold;
    }

    fclose(fpArea);

}

/* Spec: see the comments on load_resets about what check_resets does */

#define VALID_RESET 0
#define INVAL_ROOM  1
#define INVAL_OBJ   2
#define INVAL_MOB   3
#define INVAL_GEN   4
void
check_resets(void)
{
    AREA_DATA          *pArea;
    RESET_DATA         *pReset;
    RESET_DATA         *nextReset;
    ROOM_INDEX_DATA    *last_mob_room;
    int                 previous_bug = 0;

    for (pArea = first_area; pArea; pArea = pArea->next) {
        last_mob_room = NULL;

        for (pReset = pArea->first_reset; pReset; pReset = nextReset) {
            ROOM_INDEX_DATA    *pRoomIndex = NULL;
            EXIT_DATA          *pexit;
            int                 ValReset = VALID_RESET;

            nextReset = pReset->next;

            switch (pReset->command) {
                default:
                    SHOW_AREA;
                    bugf("Load_resets: bad command '%c'.", pReset->command);
                    ValReset = INVAL_GEN;
                    break;
                case 'A':        /* obsolete.. */
                    if (!get_room_index(pReset->arg1))
                        ValReset = INVAL_ROOM;
                    break;
                case 'M':
                    if (!get_mob_index(pReset->arg1))
                        ValReset = INVAL_MOB;
                    if (!(last_mob_room = get_room_index(pReset->arg3)))
                        ValReset = INVAL_ROOM;
                    break;
                case 'O':
                    if (!get_obj_index(pReset->arg1))
                        ValReset = INVAL_OBJ;
                    if (!get_room_index(pReset->arg3))
                        ValReset = INVAL_ROOM;
                    break;
                case 'P':        /* obsolete.. */
                    if (!get_obj_index(pReset->arg1) || !get_obj_index(pReset->arg3))
                        ValReset = INVAL_OBJ;
                    break;
                case 'G':
                case 'E':
                case 'C':
                    if (!get_obj_index(pReset->arg1))
                        ValReset = INVAL_OBJ;
                    if (!last_mob_room)
                        ValReset = INVAL_ROOM;
                    break;
                case 'D':
                    if (!(pRoomIndex = get_room_index(pReset->arg1))) {
                        ValReset = INVAL_ROOM;
                        break;
                    }
                    if (pReset->arg2 < 0 || pReset->arg2 > 5 || !(pexit = pRoomIndex->exit[pReset->arg2]) || !IS_SET(pexit->exit_info, EX_ISDOOR)) {
                        SHOW_AREA;
                        bugf("Check_resets: 'D': exit %d not door.", pReset->arg2);
                        ValReset = INVAL_GEN;
                    }

                    if (pReset->arg3 < 0 || pReset->arg3 > 2) {
                        SHOW_AREA;
                        bugf("Check_resets: 'D': bad 'locks': %d.", pReset->arg3);
                        ValReset = INVAL_GEN;
                    }
                    break;
                case 'R':
                    if (!(pRoomIndex = get_room_index(pReset->arg1))) {
                        ValReset = INVAL_ROOM;
                        break;
                    }
                    if (pReset->arg2 < 0 || pReset->arg2 > 5) {
                        SHOW_AREA;
                        bugf("Check_resets: 'R': bad exit %d.", pReset->arg2);
                        ValReset = INVAL_GEN;
                    }
                    break;
            }
            if (ValReset != VALID_RESET) {
                char                buf[128];

                switch (ValReset) {
                    case INVAL_ROOM:
                        sprintf(buf, "Invalid room for reset: ");
                        break;
                    case INVAL_OBJ:
                        sprintf(buf, "Invalid obj  for reset: ");
                        break;
                    case INVAL_MOB:
                        sprintf(buf, "Invalid mob  for reset: ");
                        break;
                    case INVAL_GEN:
                    default:
                        sprintf(buf, "Invalid arg  for reset: ");
                        break;
                }
                SHOW_AREA;
                sprintf(buf + strlen(buf), "%c %d %d %d %d.", pReset->command, pReset->ifflag, pReset->arg1, pReset->arg2, pReset->arg3);
                bug(buf);
                UNLINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);
                PUT_FREE(pReset, reset_free);
            }
            /* Spec: We do _not_ want to insert resets into the room list here,
             * its all handled in load_resets. Removed.
             else
             if ( pRoomIndex )
             {
             BUILD_DATA_LIST *pList;

             GET_FREE(pList, build_free);
             pList->data = pReset;
             LINK(pList, pRoomIndex->first_room_reset,
             pRoomIndex->last_room_reset, next, prev);
             }
             *
             */
        }
    }
    return;
}

/*
 * Repopulate areas periodically.
 */
void
area_update(void)
{
    AREA_DATA          *pArea;

    for (pArea = first_area; pArea != NULL; pArea = pArea->next) {
        CHAR_DATA          *pch;

        /*  if ( ++pArea->age < 3 )
           continue;
         */
        pArea->age++;
        /*
         * Check for PC's.
         */
        if ((pArea->nplayer > 0) && (pArea->age >= (pArea->reset_rate - 1))) {
            for (pch = first_char; pch != NULL; pch = pch->next) {
                if (!IS_NPC(pch)
                    && IS_AWAKE(pch)
                    && pch->in_room != NULL && pch->in_room->area == pArea) {
                    sendf(pch, "%s@@N\n\r", pArea->reset_msg);
                }
            }
        }

        /*
         * Check age and reset.
         * Note: Mud School resets every 3 minutes (not 15).
         */
        if (((pArea->nplayer == 0) && (pArea->age >= 5))
            || (pArea->age >= pArea->reset_rate)) {
            reset_area(pArea);
            pArea->age = UMIN(pArea->reset_rate - 1, number_range(0, 3));

        }
    }

    return;
}

/*
 * Reset one area.
 */

void
reset_area(AREA_DATA *pArea)
{
    RESET_DATA         *pReset;
    CHAR_DATA          *mob;
    char                buf[MAX_STRING_LENGTH];
    bool                last;
    int                 level;
    int                 previous_bug = 0;
    int                 cnt = 0;

    sprintf(buf, "Area Reset called for %s.", pArea->name);
    monitor_chan(buf, MONITOR_AREA_UPDATE);

    mob = NULL;
    last = TRUE;
    level = 0;
    for (pReset = pArea->first_reset; pReset != NULL; pReset = pReset->next) {
        ROOM_INDEX_DATA    *pRoomIndex;
        MOB_INDEX_DATA     *pMobIndex;
        OBJ_INDEX_DATA     *pObjIndex;
        OBJ_INDEX_DATA     *pObjToIndex;
        EXIT_DATA          *pexit;
        OBJ_DATA           *obj;
        OBJ_DATA           *obj_to;
        sh_int              num_allowed = 2;
        bool                extracted = FALSE;

        switch (pReset->command) {
            default:
                bugf("Reset_area: bad command %c.", pReset->command);
                break;

            case 'A':
                break;

            case 'M':
                if ((pMobIndex = get_mob_index(pReset->arg1)) == NULL) {
                    SHOW_AREA;
                    bugf("Reset_area: 'M': bad vnum %d.", pReset->arg1);
                    continue;
                }

                if ((pRoomIndex = get_room_index(pReset->arg3)) == NULL) {
                    SHOW_AREA;
                    bugf("Reset_area: 'R': bad vnum %d.", pReset->arg3);
                    continue;
                }

                level = pMobIndex->level;
                if (pMobIndex->count >= pReset->arg2) {
                    last = FALSE;
                    break;
                }

                mob = create_mobile(pMobIndex);

                /*
                 * Check for pet shop.
                 */
                {
                    ROOM_INDEX_DATA    *pRoomIndexPrev;

                    pRoomIndexPrev = get_room_index(pRoomIndex->vnum - 1);
                    if (pRoomIndexPrev != NULL && IS_SET(pRoomIndexPrev->room_flags, ROOM_PET_SHOP))
                        SET_BIT(mob->act, ACT_PET);
                }

                if (room_is_dark(pRoomIndex))
                    SET_BIT(mob->affected_by, AFF_INFRARED);

                if (mob->in_room != NULL)
                    char_to_room(mob, mob->in_room);
                else
                    char_to_room(mob, pRoomIndex);

                level = URANGE(0, mob->level - 2, LEVEL_HERO);
                last = TRUE;
                break;

            case 'O':
                if ((pObjIndex = get_obj_index(pReset->arg1)) == NULL) {
                    SHOW_AREA;
                    bugf("Reset_area: 'O': bad vnum %d.", pReset->arg1);
                    continue;
                }

                if ((pRoomIndex = get_room_index(pReset->arg3)) == NULL) {
                    SHOW_AREA;
                    bugf("Reset_area: 'R': bad vnum %d.", pReset->arg3);
                    continue;
                }
                num_allowed = (pReset->arg2 == 0) ? 1 : pReset->arg2;
                cnt = count_obj_room_mob(pObjIndex, pRoomIndex);

                if (   cnt > num_allowed
                    || (   cnt > 0
                        && (   pObjIndex->item_type == ITEM_BOARD
                            || pObjIndex->item_type == ITEM_FOUNTAIN
                            || pObjIndex->item_type == ITEM_FURNITURE
                            || pObjIndex->item_type == ITEM_PORTAL
                            || pObjIndex->item_type == ITEM_PIECE
/*                            || IS_SET(pObjIndex->extra_flags, ITEM_RARE) */
                           )
                       )
                    ) {
                    last = FALSE;
                    break;
                }

                obj = create_object(pObjIndex, number_fuzzy(level));

                obj_to_room(obj, pRoomIndex);

                if ((pObjIndex->rarity == 0)
                    && (IS_SET(obj->extra_flags, ITEM_RARE))
                    && !((number_percent() < 2) && (number_percent() < 8))
                    ) {
                    extract_obj(obj);
                    extracted = TRUE;
                }

                if (!extracted)
                    if ((pObjIndex->rarity < 0)
                        || ((pObjIndex->rarity != 0)
                            && (IS_SET(obj->extra_flags, ITEM_RARE))
                            && (number_range(1, pObjIndex->rarity) != number_range(1, pObjIndex->rarity))
                        )
                        ) {
                        extract_obj(obj);
                        extracted = TRUE;
                    }

                if (!extracted && (IS_SET(pRoomIndex->room_flags, ROOM_NO_REPOP))
                    && (pRoomIndex->first_person != NULL)) {
                    extract_obj(obj);
                    extracted = TRUE;
                }

                if (obj != NULL && obj->in_room != NULL && IS_SET(pObjIndex->extra_flags, ITEM_RARE)
                    && (pObjIndex->rarity == 0 || pObjIndex->rarity > 50)
                    ) {
                    char                rbuf[MSL];

                    sprintf(rbuf, "%s has populated in room %d. (chance: 1 in %d)", pObjIndex->short_descr, obj->in_room->vnum,
                        (pObjIndex->rarity > 0) ? pObjIndex->rarity : 625);
                    monitor_chan(rbuf, MONITOR_OBJ);
                }

                last = TRUE;
                break;

            case 'P':
                if ((pObjIndex = get_obj_index(pReset->arg1)) == NULL) {
                    SHOW_AREA;
                    bugf("Reset_area: 'P': bad vnum %d.", pReset->arg1);
                    continue;
                }

                if ((pObjToIndex = get_obj_index(pReset->arg3)) == NULL) {
                    SHOW_AREA;
                    bugf("Reset_area: 'P': bad vnum %d.", pReset->arg3);
                    continue;
                }

                if (pArea->nplayer > 0 || (obj_to = get_obj_type(pObjToIndex)) == NULL || count_obj_list(pObjIndex, obj_to->first_in_carry_list) > 0) {
                    last = FALSE;
                    break;
                }

                obj = create_object(pObjIndex, number_fuzzy(obj_to->level));

                /*        if (  ( IS_SET( obj->extra_flags, ITEM_RARE ) )
                   && ( number_percent() < 1 )
                   && ( number_percent() < 50 )   )
                   {
                   obj_to_obj( obj, obj_to );
                   }
                   else
                   {
                   extract_obj( obj );
                   }   */
                obj_to_obj(obj, obj_to);

                last = TRUE;
                break;

            case 'C':
                break;

            case 'G':
            case 'E':
                if ((pObjIndex = get_obj_index(pReset->arg1)) == NULL) {
                    SHOW_AREA;
                    bugf("Reset_area: 'E' or 'G': bad vnum %d.", pReset->arg1);
                    continue;
                }

                if (!last)
                    break;

                if (mob == NULL) {
                    SHOW_AREA;
                    bugf("Reset_area: 'E' or 'G': null mob for vnum %d.", pReset->arg1);
                    last = FALSE;
                    break;
                }

                if (mob->pIndexData->pShop != NULL) {
                    int                 olevel;

                    switch (pObjIndex->item_type) {
                        default:
                            olevel = 0;
                            break;
                        case ITEM_PILL:
                            olevel = number_range(0, 10);
                            break;
                        case ITEM_POTION:
                            olevel = number_range(0, 10);
                            break;
                        case ITEM_SCROLL:
                            olevel = number_range(5, 15);
                            break;
                        case ITEM_WAND:
                            olevel = number_range(10, 20);
                            break;
                        case ITEM_STAFF:
                            olevel = number_range(15, 25);
                            break;
                        case ITEM_ARMOR:
                            olevel = number_range(5, 15);
                            break;
                        case ITEM_WEAPON:
                            olevel = number_range(5, 15);
                            break;
                    }

                    obj = create_object(pObjIndex, olevel);
                    SET_BIT(obj->extra_flags, ITEM_INVENTORY);
                }
                else {
                    obj = create_object(pObjIndex, number_fuzzy(level));
                }
                obj_to_char(obj, mob);
                if (pReset->command == 'E')
                    equip_char(mob, obj, pReset->arg3);
                last = TRUE;
                break;

            case 'D':
                if ((pRoomIndex = get_room_index(pReset->arg1)) == NULL) {
                    bugf("Reset_area: 'D': bad vnum %d.", pReset->arg1);
                    continue;
                }

                if ((pexit = pRoomIndex->exit[pReset->arg2]) == NULL)
                    break;

                switch (pReset->arg3) {
                    case 0:
                        REMOVE_BIT(pexit->exit_info, EX_CLOSED);
                        REMOVE_BIT(pexit->exit_info, EX_LOCKED);
                        break;

                    case 1:
                        SET_BIT(pexit->exit_info, EX_CLOSED);
                        REMOVE_BIT(pexit->exit_info, EX_LOCKED);
                        break;

                    case 2:
                        SET_BIT(pexit->exit_info, EX_CLOSED);
                        SET_BIT(pexit->exit_info, EX_LOCKED);
                        break;
                }

                last = TRUE;
                break;

            case 'R':
                if ((pRoomIndex = get_room_index(pReset->arg1)) == NULL) {
                    SHOW_AREA;
                    bugf("Reset_area: 'R': bad vnum %d.", pReset->arg1);
                    continue;
                }

                {
                    int                 d0;
                    int                 d1;

                    for (d0 = 0; d0 < pReset->arg2 - 1; d0++) {
                        d1 = number_range(d0, pReset->arg2 - 1);
                        pexit = pRoomIndex->exit[d0];
                        pRoomIndex->exit[d0] = pRoomIndex->exit[d1];
                        pRoomIndex->exit[d1] = pexit;
                    }
                }
                break;
        }
    }

    return;
}

/* Nasty hack to get back the address of a loaded mob */
extern CHAR_DATA   *loaded_mob_addr;

/*
 * Create an instance of a mobile.
 */
CHAR_DATA          *
create_mobile(MOB_INDEX_DATA *pMobIndex)
{
    CHAR_DATA          *mob;
    int                 level;

    if (pMobIndex == NULL) {
        bug("Create_mobile: NULL pMobIndex.");
        hang("Create Mobile in db.c");
    }

    GET_FREE(mob, char_free);

    clear_char(mob);
    mob->pIndexData = pMobIndex;

    mob->name = str_dup(pMobIndex->player_name);
    mob->short_descr = str_dup(pMobIndex->short_descr);
    mob->long_descr = str_dup(pMobIndex->long_descr);
    mob->description = str_dup(pMobIndex->description);
    mob->spec_fun = pMobIndex->spec_fun;
    mob->sitting = NULL;
    mob->first_shield = NULL;
    mob->last_shield = NULL;
    mob->last_room = NULL;

    /* Changed exp value for mobs */

    level = number_fuzzy(pMobIndex->level);

    mob->level = level;

    mob->act = pMobIndex->act;
    mob->affected_by = pMobIndex->affected_by;
    mob->alignment = pMobIndex->alignment;
    mob->sex = pMobIndex->sex;
    mob->ac_mod = pMobIndex->ac_mod;
    mob->hr_mod = pMobIndex->hr_mod;
    mob->dr_mod = pMobIndex->dr_mod;
    /*  mob->move_to    = NO_VNUM; */

    mob->armor = interpolate(mob->level / 2, 100, -100);

    if (pMobIndex->hp_mod == 0)
        mob->max_hit = mob->level * 15 + number_range(mob->level * mob->level / 2, mob->level * mob->level / 1);
    else
        mob->max_hit = pMobIndex->hp_mod;

    mob->hit = mob->max_hit;

    mob->exp = exp_for_mobile(mob->level, mob);

    /* mana for mobs... */
    if (pMobIndex->mana_mod == 0)
        mob->max_mana = level * 25;
    else
        mob->max_mana = pMobIndex->mana_mod;

    mob->mana = mob->max_mana;

    mob->skills = pMobIndex->skills;
    mob->cast = pMobIndex->cast;
    mob->def = pMobIndex->def;
    mob->class = pMobIndex->class;
    mob->clan = pMobIndex->clan;
    mob->race = pMobIndex->race;
    mob->position = POS_STANDING;
    mob->is_quitting = FALSE;
    mob->extract_timer = -1;
    mob->path_steps = 0;
    mob->in_room = NULL;        /* to distinguish between loaded mobs */
    /* and creted mobs     */

    /*
     * Insert in list.
     */
    mob->next = NULL;
    mob->prev = NULL;
    LINK(mob, first_char, last_char, next, prev);
    pMobIndex->count++;
    top_mob++;

    return mob;
}

/*
 * Create an instance of an object.
 */
OBJ_DATA           *
create_object(OBJ_INDEX_DATA *pObjIndex, int level)
{
    static OBJ_DATA     obj_zero;
    OBJ_DATA           *obj;
    AFFECT_DATA        *af;
    AFFECT_DATA        *new_af;
    int                 new_cost;

    if (level < 0)
        level = 1;

    if (pObjIndex == NULL) {
        bug("Create_object: NULL pObjIndex.");
        hang("Creating Objext in db.c");
    }

    GET_FREE(obj, obj_free);
    *obj = obj_zero;
    obj->pIndexData = pObjIndex;
    obj->in_room = NULL;
    if (pObjIndex->level < 3) {
        if (pObjIndex->level == 2) {
            obj->level = 1;
        }
        else {
            obj->level = level;
        }
    }
    else {
        obj->level = pObjIndex->level;
    }

    obj->wear_loc = -1;
    obj->id = 0;
    obj->name = str_dup(pObjIndex->name);
    obj->short_descr = str_dup(pObjIndex->short_descr);
    obj->description = str_dup(pObjIndex->description);
    obj->item_type = pObjIndex->item_type;
    obj->extra_flags = pObjIndex->extra_flags;
    obj->wear_flags = pObjIndex->wear_flags;
    obj->item_apply = pObjIndex->item_apply;
    obj->obj_fun = pObjIndex->obj_fun;
    obj->first_in_room = NULL;
    obj->next_in_room = NULL;
    obj->last_in_room = NULL;
    obj->prev_in_room = NULL;
    obj->first_in_carry_list = NULL;
    obj->next_in_carry_list = NULL;
    obj->last_in_carry_list = NULL;
    obj->prev_in_carry_list = NULL;
    obj->first_content = NULL;
    obj->next_content = NULL;
    obj->last_content = NULL;
    obj->prev_content = NULL;

    obj->in_obj = NULL;

    /* We want to copy pObjIndex->affected to obj->index */
    for (af = pObjIndex->first_apply; af != NULL; af = af->next) {
        /* Create new_af, or use a free_affect */
        GET_FREE(new_af, affect_free);

        /* Now initialize the contents of new_af */
        new_af->type = -1;
        new_af->duration = -1;
        new_af->location = af->location;
        new_af->modifier = af->modifier;
        new_af->bitvector = 0;
        new_af->save = TRUE;
        new_af->caster = NULL;
        LINK(new_af, obj->first_apply, obj->last_apply, next, prev);
    }

    obj->value[0] = pObjIndex->value[0];
    obj->value[1] = pObjIndex->value[1];
    obj->value[2] = pObjIndex->value[2];
    obj->value[3] = pObjIndex->value[3];
    obj->weight = pObjIndex->weight;
    obj->newcost = pObjIndex->newcost;

    /* only set cost if individual cost isnt defined */
    if (obj->newcost == FALSE) {
        new_cost = get_item_value(obj);
        if (IS_SET(obj->extra_flags, ITEM_REMORT))
            new_cost *= 2;
        if (IS_SET(obj->extra_flags, ITEM_ADEPT))
            new_cost *= 3;
        if (IS_SET(obj->extra_flags, ITEM_RARE))
            new_cost *= 10;
        obj->cost = new_cost;

        if (obj->item_type == ITEM_FOOD)
            obj->cost /= 200;
        obj->cost = UMAX(10, obj->cost);
    }
    else {
        obj->cost = pObjIndex->cost;
    }

    /*
     * Mess with object properties.
     */
    switch (obj->item_type) {
        default:
            bugf("create_object: vnum %d bad type %d.", pObjIndex->vnum, obj->item_type);
            break;
        case ITEM_SPELL_MATRIX:
        case ITEM_ENCHANTMENT:
        case ITEM_LIGHT:
        case ITEM_BEACON:
        case ITEM_TREASURE:
        case ITEM_FURNITURE:
        case ITEM_TRASH:
        case ITEM_CLUTCH:
            break;
        case ITEM_CONTAINER:
            obj->wear_flags = ITEM_TAKE;
        case ITEM_DRINK_CON:
        case ITEM_BOARD:
        case ITEM_KEY:
        case ITEM_FOOD:
        case ITEM_BOAT:
        case ITEM_CORPSE_NPC:
        case ITEM_CORPSE_PC:
        case ITEM_FOUNTAIN:
        case ITEM_PORTAL:
        case ITEM_PIECE:
        case ITEM_SOUL:
        case ITEM_TRIGGER:
        case ITEM_QUEST:
            break;

        case ITEM_SCROLL:
            obj->value[0] = number_fuzzy(obj->value[0]);
            break;

        case ITEM_WAND:
        case ITEM_STAFF:
            obj->value[0] = number_fuzzy(obj->value[0]);
            obj->value[1] = number_fuzzy(obj->value[1]);
            obj->value[2] = obj->value[1];
            break;

        case ITEM_WEAPON:
            obj->value[1] = number_fuzzy(number_fuzzy(1 * level / 8 + 3));
            obj->value[2] = number_fuzzy(number_fuzzy(3 * level / 8 + 4));
            break;

        case ITEM_ARMOR:
            obj->value[0] = number_fuzzy(level / 12 + 4);
            break;

        case ITEM_POTION:
        case ITEM_PILL:
            obj->value[0] = number_fuzzy(number_fuzzy(obj->value[0]));
            break;

        case ITEM_MONEY:
            obj->value[0] = obj->cost;
            break;
    }

    LINK(obj, first_obj, last_obj, next, prev);
    pObjIndex->count++;
    top_obj++;

    if (pObjIndex->count >= 1000) {
        sprintf(log_buf, "@@eWarning: @@Ncurrently @@a%d @@N%s@@N's on the mud!", pObjIndex->count, pObjIndex->short_descr);
        monitor_chan(log_buf, MONITOR_OBJ);
    }

    return obj;
}

/*
 * Clear a new character.
 */
void
clear_char(CHAR_DATA *ch)
{
    static CHAR_DATA    ch_zero;

    *ch = ch_zero;
    ch->name = &str_empty[0];
    ch->short_descr = &str_empty[0];
    ch->short_descr_orig = &str_empty[0];
    ch->long_descr = &str_empty[0];
    ch->long_descr_orig = &str_empty[0];
    ch->description = &str_empty[0];
    ch->logon = current_time;
    ch->armor = 100;
    ch->position = POS_STANDING;
    ch->practice = 21;
    ch->hit = 20;
    ch->max_hit = 20;
    ch->mana = 100;
    ch->max_mana = 100;
    ch->move = 100;
    ch->sitting = NULL;
    ch->max_move = 100;
    ch->act_build = 0;
    ch->build_vnum = -1;
    ch->sentence = 0;
    ch->first_shield = NULL;
    ch->last_shield = NULL;
    ch->stunTimer = 0;
    ch->last_room = NULL;

    /*    ch->pcdata->recall_vnum = 3001;     */

    return;
}

/*
 * Free a character.
 */
void
free_char(CHAR_DATA *ch)
{
    MPROG_ACT_LIST     *mpact, *mpact_next;
    CHAR_DATA          *rch;
    IGNORE_DATA        *ignore_list = NULL;
    ANSWERING_DATA     *message;

    int                 cnt;

    ch->is_quitting = TRUE;
    while (ch->first_carry != NULL)
        extract_obj(ch->first_carry);

    while (ch->first_affect != NULL)
        affect_remove(ch, ch->first_affect);

    for (mpact = first_mpact; mpact != NULL; mpact = mpact_next) {
        mpact_next = mpact->next;

        if (mpact->mob != ch)
            continue;

        UNLINK(mpact, first_mpact, last_mpact, next, prev);
        free_string(mpact->buf);
        PUT_FREE(mpact, mpact_free);
    }

    for (rch = first_char; rch; rch = rch->next) {
        AFFECT_DATA        *paf;

        if (rch->master == ch)
            rch->master = NULL;
        if (rch->leader == ch)
            rch->leader = NULL;
        if (rch->fighting == ch)
            rch->fighting = NULL;
        if (rch->reply == ch)
            rch->reply = NULL;
        if (rch->ireply == ch)
            rch->ireply = NULL;
        if (rch->hunting == ch) {
            if (IS_NPC(rch) && !IS_NPC(ch)) {
                free_string(rch->searching);
                rch->searching = str_dup(ch->name);
            }
            else if (!IS_NPC(rch)) {
                send_to_char("@@RYou seem to have lost your prey.@@N\n\r", rch);
            }
            end_hunt(rch);
        }
        if (rch->hunt_for == ch)
            rch->hunt_for = NULL;

        for (paf = rch->first_affect; paf; paf = paf->next)
            if (paf->caster == ch)
                paf->caster = NULL;
    }

    {
        ROOM_INDEX_DATA    *room;
        int                 rvnum;
        ROOM_AFFECT_DATA   *raf;

        for (rvnum = 0; rvnum < MAX_KEY_HASH; rvnum++)
            for (room = room_index_hash[rvnum]; room; room = room->next)
                for (raf = room->first_room_affect; raf; raf = raf->next)
                    if (raf->caster == ch)
                        raf->caster = NULL;
    }

    free_string(ch->searching);
    free_string(ch->name);
    free_string(ch->short_descr);
    free_string(ch->short_descr_orig);
    free_string(ch->long_descr);
    free_string(ch->long_descr_orig);
    free_string(ch->description);
    free_string(ch->afk_msg);
    free_string(ch->target);

    while (ch->first_message != NULL) {
        message = ch->last_message;

        free_string(message->message);
        free_string(message->name);
        UNLINK(message, ch->first_message, ch->last_message, next, prev);
        PUT_FREE(message, answering_free);
    }

    if (ch->current_brand) {
        free_string(ch->current_brand->branded);
        free_string(ch->current_brand->branded_by);
        free_string(ch->current_brand->dt_stamp);
        free_string(ch->current_brand->message);
        free_string(ch->current_brand->priority);
        PUT_FREE(ch->current_brand, brand_data_free);
    }


    if (ch->pcdata != NULL) {
        free_string(ch->pcdata->pwd);
        free_string(ch->pcdata->bamfin);
        free_string(ch->pcdata->room_enter);
        free_string(ch->pcdata->room_exit);
        free_string(ch->pcdata->bamfout);
        free_string(ch->pcdata->title);
        free_string(ch->pcdata->immskll);
        free_string(ch->pcdata->host);
        free_string(ch->pcdata->ip);
        free_string(ch->pcdata->accept_name);
        free_string(ch->pcdata->header);
        free_string(ch->pcdata->message);

        for (cnt = 0; cnt < MAX_ALIASES; cnt++) {
            free_string(ch->pcdata->alias_name[cnt]);
            free_string(ch->pcdata->alias[cnt]);
        }

        while ((ignore_list = ch->pcdata->first_ignore) != NULL) {
            UNLINK(ignore_list, ch->pcdata->first_ignore, ch->pcdata->last_ignore, next, prev);

            free_string(ignore_list->char_ignored);

            PUT_FREE(ignore_list, ignore_free);
        }

        free_string(ch->pcdata->who_name);
        free_string(ch->pcdata->load_msg);

        for (cnt = 0; cnt < 5; cnt++) {
            free_string(ch->pcdata->pedit_string[cnt]);
        }

        free_string(ch->pcdata->origname);
        free_string(ch->pcdata->prompt);
        free_string(ch->pcdata->battleprompt);
        free_string(ch->pcdata->noteprompt);

        if (ch->pcdata->rename != NULL) {
            free_string(ch->pcdata->rename->playername);
            free_string(ch->pcdata->rename->oldshort);
            free_string(ch->pcdata->rename->oldlong);
            free_string(ch->pcdata->rename->oldkeyword);
            free_string(ch->pcdata->rename->newshort);
            free_string(ch->pcdata->rename->newlong);
            free_string(ch->pcdata->rename->newkeyword);

            PUT_FREE(ch->pcdata->rename, rename_free);
            send_to_char("Rename cancelled.\n\r", ch);
        }

        free_string(ch->pcdata->autostance);
        PUT_FREE(ch->pcdata, pcd_free);
    }

    PUT_FREE(ch, char_free);
    return;
}

/*
 * Get an extra description from a list.
 */
char               *
get_extra_descr(const char *name, EXTRA_DESCR_DATA *ed)
{
    for (; ed != NULL; ed = ed->next) {
        if (is_name(name, ed->keyword))
            return ed->description;
    }
    return NULL;
}

/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
MOB_INDEX_DATA     *
get_mob_index(int vnum)
{
    MOB_INDEX_DATA     *pMobIndex;

    for (pMobIndex = mob_index_hash[vnum % MAX_KEY_HASH]; pMobIndex != NULL; pMobIndex = pMobIndex->next) {
        if (pMobIndex->vnum == vnum)
            return pMobIndex;
    }

    if (fBootDb) {
        bugf("Get_mob_index: bad vnum %d.", vnum);
        hang("Get mob index in db.c");
    }

    return NULL;
}

/*
 * Translates mob virtual number to its obj index struct.
 * Hash table lookup.
 */
OBJ_INDEX_DATA     *
get_obj_index(int vnum)
{
    OBJ_INDEX_DATA     *pObjIndex;

    for (pObjIndex = obj_index_hash[vnum % MAX_KEY_HASH]; pObjIndex != NULL; pObjIndex = pObjIndex->next) {
        if (pObjIndex->vnum == vnum)
            return pObjIndex;
    }

    if (fBootDb) {
        bugf("Get_obj_index: bad vnum %d.", vnum);
        hang("Get Object Index in db.c");
    }

    return NULL;
}

/*
 * Translates mob virtual number to its room index struct.
 * Hash table lookup.
 */
ROOM_INDEX_DATA    *
get_room_index(int vnum)
{
    ROOM_INDEX_DATA    *pRoomIndex;

    for (pRoomIndex = room_index_hash[vnum % MAX_KEY_HASH]; pRoomIndex != NULL; pRoomIndex = pRoomIndex->next) {
        if (pRoomIndex->vnum == vnum)
            return pRoomIndex;
    }

    if (fBootDb) {
        bugf("Get_room_index: bad vnum %d.", vnum);
        /*  exit( 1 );   */
    }

    return NULL;
}

/*
 * Read a letter from a file.
 */
char
fread_letter(FILE * fp)
{
    char                c;

    do {
        c = getc(fp);
    }
    while (isspace(c) || c == '\n' || c == '\r');

    return c;
}

/*
 * Read a number from a file.
 */
int
fread_number(FILE * fp)
{
    int                 number;
    bool                sign;
    char                c;

    do {
        c = getc(fp);
    }
    while (isspace(c) || c == '\n' || c == '\r');

    number = 0;

    sign = FALSE;
    if (c == '+') {
        c = getc(fp);
    }
    else if (c == '-') {
        sign = TRUE;
        c = getc(fp);
    }

    if (!isdigit(c)) {
        bugf("Fread_number: looking for a digit, found a %c.", c);
        hang("Error in fread_number");
    }

    while (isdigit(c)) {
        number = number * 10 + c - '0';
        c = getc(fp);
    }

    if (sign)
        number = 0 - number;

    if (c == '|')
        number += fread_number(fp);
    else if (c != ' ')
        ungetc(c, fp);

    return number;
}

unsigned int
fread_unumber(FILE * fp)
{
    unsigned int        number;
    char                c;

    do {
        c = getc(fp);
    }
    while (isspace(c) || c == '\n' || c == '\r');

    number = 0;

    if (!isdigit(c)) {
        bugf("Fread_unumber: looking for a digit, found a %c.", c);
        hang("Error in fread_unumber");
    }

    while (isdigit(c)) {
        number = number * 10 + c - '0';
        c = getc(fp);
    }

    if (c == '|')
        number += fread_unumber(fp);
    else if (c != ' ')
        ungetc(c, fp);

    return number;
}

/* read an array of strings from a file and allocate the necessary member to
   contain them; in the format: [size,x,x,x,x...] */
int                *
fread_number_array(FILE * fp)
{
    char                c;
    int                 amt, cnt;
    int                *arr;

    do {
        c = getc(fp);
    }
    while (isspace(c));

    if (c != '[') {
        char                buf[MSL];

        sprintf(buf, "fread_number_array(): looking for [, found %c(%d)", c, c);
        log_string(buf);
        hang("error in fread_number_array()");
        return NULL;
    }

    amt = fread_number(fp);

    if (amt <= 0) {
        log_string("fread_number_array(): array size was <= 0");
        hang("error in fread_number_array()");
        return NULL;
    }

    arr = (int *) getmem(sizeof(int) * (amt + 1));
    arr[0] = amt;
    c = getc(fp);

    for (cnt = 1; cnt <= amt; cnt++) {
        arr[cnt] = fread_number(fp);
        c = getc(fp);
    }

    if (c != ']') {
        char                buf[MSL];

        sprintf(buf, "fread_number_array(): looking for ], found %c(%d)", c, c);
        log_string(buf);
        hang("error in fread_number_array()");
        return NULL;
    }

    return arr;
}

/* return a string with a formatted integer array output, suitable for
   printing in files */
char               *
print_number_array(int *arr)
{
    static char         buf[MSL];
    int                 c;

    buf[0] = 0;

    sprintf(buf, "[%d,", arr[0]);

    for (c = 1; c <= arr[0]; c++)
        sprintf(buf + strlen(buf), "%d,", arr[c]);

    buf[strlen(buf)] = 0;
    buf[strlen(buf) - 1] = 0;
    buf[strlen(buf)] = ']';

    return buf;
}

#if 0

/*
 * Read and allocate space for a string from a file.
 * These strings are read-only and shared.
 * Strings are hashed:
 *   each string prepended with hash pointer to prev string,
 *   hash code is simply the string length.
 * This function takes 40% to 50% of boot-up time.
 */
char               *
fread_string(FILE * fp)
{
    char               *plast;
    char                c;

    plast = top_string + sizeof(char *);
    if (plast > &string_space[MAX_STRING - MAX_STRING_LENGTH]) {
        bugf("Fread_string: MAX_STRING %d exceeded.", MAX_STRING);
        hang("Fread String in db.c");
    }

    /*
     * Skip blanks.
     * Read first char.
     */
    do {
        c = getc(fp);
    }
    while (isspace(c));

    if ((*plast++ = c) == '~')
        return &str_empty[0];

    for (;;) {
        /*
         * Back off the char type lookup,
         *   it was too dirty for portability.
         *   -- Furey
         */
        switch (*plast = getc(fp)) {
            default:
                plast++;
                break;

            case EOF:
                bug("Fread_string: EOF");
                /* exit( 1 ); */
                ungetc('~', fp);
                break;

            case '\n':
                plast++;
                *plast++ = '\r';
                break;

            case '\r':
                break;

            case '~':
                plast++;
                {
                    union
                    {
                        char               *pc;
                        char                rgc[sizeof(char *)];
                    } u1;
                    int                 ic;
                    int                 iHash;
                    char               *pHash;
                    char               *pHashPrev;
                    char               *pString;

                    plast[-1] = '\0';
                    iHash = UMIN(MAX_KEY_HASH - 1, plast - 1 - top_string);
                    for (pHash = string_hash[iHash]; pHash; pHash = pHashPrev) {
                        for (ic = 0; ic < sizeof(char *); ic++)
                            u1.rgc[ic] = pHash[ic];
                        pHashPrev = u1.pc;
                        pHash += sizeof(char *);

                        if (top_string[sizeof(char *)] == pHash[0]
                            && !strcmp(top_string + sizeof(char *) + 1, pHash + 1))
                            return pHash;
                    }

                    if (fBootDb) {
                        pString = top_string;
                        top_string = plast;
                        u1.pc = string_hash[iHash];
                        for (ic = 0; ic < sizeof(char *); ic++)
                            pString[ic] = u1.rgc[ic];
                        string_hash[iHash] = pString;

                        nAllocString += 1;
                        sAllocString += top_string - pString;
                        return pString + sizeof(char *);
                    }
                    else {
                        return str_dup(top_string + sizeof(char *));
                    }
                }
        }
    }
}

#endif

/*
 * Read to end of line (for comments).
 */

/* Spec: fixed to handle EOF more gracefully */

void
_fread_to_eol(FILE * fp, const char *caller)
{
    int             c;

    do {
        c = getc(fp);
    }
    while (c != '\n' && c != '\r' && c != EOF);

    while (c == '\n' || c == '\r') {
        c = getc(fp);
    }

    if (c == EOF) {
        bugf("fread_to_eol: EOF (%s)", caller);
        return;
    }

    ungetc(c, fp);
    return;
}

/* Same as above, but returns the rest of the line */

/* Spec: fixed to handle EOF more gracefully */

char               *
fsave_to_eol(FILE * fp)
{
    char                string[MAX_INPUT_LENGTH + 1];
    int                 a;
    char                c;

    a = 0;
    do {
        string[a++] = getc(fp);

    }
    while (a < MAX_INPUT_LENGTH && string[a - 1] != '\n' && string[a - 1] != '\r' && string[a - 1] != EOF);

    if (string[a - 1] == EOF) {
        bugf("fsave_to_eol: EOF");
        string[a - 1] = 0;
        return str_dup(string);
    }

    if (a == MAX_INPUT_LENGTH)
        fread_to_eol(fp);
    else {
        c = string[a - 1];
        while (c == '\n' || c == '\r')
            c = getc(fp);

        if (c == EOF)
            bugf("fsave_to_eol: EOF");
        else
            ungetc(c, fp);
    }
    string[a] = '\0';

    return str_dup(string);
}

/*
 * Read one word (into static buffer).
 */
char               *
fread_word(FILE * fp)
{
    static char         word[MAX_INPUT_LENGTH];
    char               *pword;
    char                cEnd;

    do {
        if (feof(fp)) {
            bug("fread_word: EOF encountered on read.");
            if (fBootDb)
                raise(SIGQUIT);
            strcpy(word, "End");
            return word;
        }
        cEnd = getc(fp);
    }
    while (!feof(fp) && isspace(cEnd));

    if (feof(fp)) {
        bug("fread_word: EOF encountered on read v1.");
        strcpy(word, "End");
        return word;
    }

    if (cEnd == '\'' || cEnd == '"') {
        pword = word;
    }
    else {
        word[0] = cEnd;
        pword = word + 1;
        cEnd = ' ';
    }

    for (; pword < word + MAX_INPUT_LENGTH; pword++) {
        if (feof(fp)) {
            bug("fread_word: EOF encountered on read v2.");
            if (fBootDb)
                raise(SIGQUIT);
            *pword = '\0';
            return word;
        }
        *pword = getc(fp);
        if (cEnd == ' ' ? isspace(*pword) : *pword == cEnd) {
            if (cEnd == ' ')
                ungetc(*pword, fp);
            *pword = '\0';
            return word;
        }
    }

    bug("Fread_word: word too long");
    raise(SIGQUIT);
    return NULL;
}

void               *
_getmem(int size, const char *caller, int log)
{
    void               *mem;

    if (!(mem = malloc(size))) {
        FPRINTF(stdout, "Out of memory.\n");
        raise(SIGSEGV);
    }
    memset(mem, 0, size);

    if (log && mem_log)
        xlogf("getmem(%d)=%p from %s", size, mem, caller);

    return mem;
}

#if 0

void               *
getmem(int size)
{
    void               *mem;

    if (!(mem = calloc(1, size))) {
        FPRINTF(stdout, "Out of memory; size %d requested.\n", size);
        raise(SIGSEGV);
    }
    nAllocPerm++;
    sAllocPerm += size;
    memset(mem, 0, size);
    return mem;
}

void
dispose(void *mem, int size)
{
    if (!mem)
        return;
    nAllocPerm--;
    sAllocPerm -= size;
    free(mem);
    return;
}

#endif

#if 0
/*
 * Duplicate a string into dynamic memory.
 * Fread_strings are read-only and shared.
 */
#define STRING_FREELIST
#ifdef STRING_FREELIST
#define MAX_SIZE_LIST    13
static const int    sizelist[MAX_SIZE_LIST] = { 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768 };
struct text_data
{
    struct text_data   *next;
    char               *text;
};
static struct text_data *text_free[MAX_SIZE_LIST] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL
};
static struct text_data *tdf_free = NULL;
#endif
char               *
str_dup(const char *str)
{
    char               *str_new;

#ifdef STRING_FREELIST
    sh_int              size;
    int                 len;
#endif

    if (!str || !*str)
        return &str_empty[0];

    if (str >= string_space && str < top_string)
        return (char *) str;

#ifdef STRING_FREELIST
    len = strlen(str) + 1;
    for (size = 0; size < MAX_SIZE_LIST; size++)
        if (len < sizelist[size])
            break;
    if (size < MAX_SIZE_LIST) {
        if (text_free[size] != NULL) {
            struct text_data   *tdf = text_free[size];

            text_free[size] = tdf->next;
            str_new = tdf->text;
            tdf->next = tdf_free;
            tdf_free = tdf;
        }
        else
            str_new = getmem(sizelist[size]);
    }
    else
        str_new = getmem(len);
#else
    str_new = getmem(strlen(str) + 1);
#endif
    strcpy(str_new, str);
    return str_new;
}

/*
 * Free a string.
 * Null is legal here to simplify callers.
 * Read-only shared strings are not touched.
 */
void
free_string(char *pstr)
{
#ifdef STRING_FREELIST
    sh_int              size;
    int                 len;
#endif

    if (pstr == NULL || pstr == &str_empty[0]
        || (pstr >= string_space && pstr < top_string))
        return;

#ifdef STRING_FREELIST
    len = strlen(pstr) + 1;
    for (size = 0; size < MAX_SIZE_LIST; size++)
        if (len < sizelist[size])
            break;
    if (size < MAX_SIZE_LIST) {
        struct text_data   *tdf;

        if (tdf_free) {
            tdf = tdf_free;
            tdf_free = tdf->next;
        }
        else
            tdf = getmem(sizeof(*tdf));
        tdf->text = pstr;
        tdf->next = text_free[size];
        text_free[size] = tdf;
    }
    else
        dispose(pstr);
#else
    dispose(pstr);
#endif
    return;
}

#endif

void
do_areas(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    char                msg[MAX_STRING_LENGTH];
    char                arg1[MSL];
    sh_int              foo;
    AREA_DATA          *pArea;
    bool                fall = FALSE;

    argument = one_argument(argument, arg1);

    if (!str_cmp(arg1, "all"))
        fall = TRUE;
    sprintf(buf, "@@W" mudnamecolor " AREA LISTING\n\r");
    safe_strcat(MAX_STRING_LENGTH, buf, "+-------+------------+------------------------------------------------+\n\r");
    safe_strcat(MAX_STRING_LENGTH, buf, "| @@yLevel@@W |            |                                                |\n\r");
    safe_strcat(MAX_STRING_LENGTH, buf, "| @@yrange@@W |   @@yAuthor@@W   |      @@yName of Area@@W                              |\n\r");
    safe_strcat(MAX_STRING_LENGTH, buf, "+-------+------------+------------------------------------------------+\n\r");

    foo = 0;
    for (pArea = first_area; pArea != NULL; pArea = pArea->next) {
        if ((IS_SET(pArea->flags, AREA_NOSHOW))
            || (IS_SET(pArea->flags, AREA_BUILDING)))
            continue;            /* for non-finished areas - don't show */
        if ((!fall)
            && ((pArea->min_level > (get_pseudo_level(ch)))
                || (pArea->max_level < (get_pseudo_level(ch)))))
            continue;

        foo++;
        sprintf(msg, " %s %12s          %s\n\r", pArea->level_label, capitalize(pArea->owner), pArea->name);
        safe_strcat(MAX_STRING_LENGTH, buf, msg);
    }
    sprintf(msg, "@@R%d Areas listed.\n\r@@N Type areas all to list the entire " mudnamecolor " realm.\n\r@@N", foo);
    safe_strcat(MAX_STRING_LENGTH, buf, msg);
    send_to_char(buf, ch);
    return;
}

void
perm_update()
{
    /* output perm usage to log file */
    FILE               *po;
    char               *strtime;

    if (nosave)
        return;

    po = fopen("perm.out", "a");

    strtime = ctime(&current_time);
    strtime[strlen(strtime) - 1] = '\0';

    FPRINTF(po, "%s :: Perms   %5d blocks  of %7d bytes.\n\r", strtime, nAllocPerm, sAllocPerm);
    fclose(po);
    return;
}

#define SHOW_WASTED_BYTES(x, y, z)                                      \
do {                                                                    \
    x *moo;                                                             \
    for (cnt = 0, moo = y; moo; moo = moo->next)                        \
        cnt++;                                                          \
                                                                        \
    sendf(ch, "Number of free %-18s in game: %d (%d wasted bytes)\n\r", \
        z, cnt, sizeof(x) * cnt);                                       \
} while(0)

void
do_memory(CHAR_DATA *ch, char *argument)
{
    int num_mob, num_obj;
    int cnt;
    CHAR_DATA          *mob;
    OBJ_DATA           *obj;

    if (!str_cmp(argument, "defrag")) {
        send_to_char("Defragmenting SSM heap.\n\r", ch);
        xlogf("SSM: %s called defrag_heap.", ch->name);
        defrag_heap();
        return;
    }

    if (!str_cmp(argument, "log")) {
        if (get_trust(ch) < MAX_LEVEL) {
            send_to_char("Not at your level.\n\r", ch);
            return;
        }

        if (mem_log) {
            mem_log = FALSE;
            send_to_char("Memory logging is now OFF.\n\r", ch);
            xlogf("%s turned off memory logging", ch->name);
            return;
        }
        else {
            mem_log = TRUE;
            send_to_char("Memory logging is now ON.. remember to turn it off!\n\r", ch);
            xlogf("%s turned on memory logging", ch->name);
            return;
        }
    }

    sendf(ch, "Affects %8d (%10d bytes)\n\r", top_affect,    sizeof(AFFECT_DATA)      * top_affect);
    sendf(ch, "Areas   %8d (%10d bytes)\n\r", top_area,      sizeof(AREA_DATA)        * top_area);
    sendf(ch, "ExDes   %8d (%10d bytes)\n\r", top_ed,        sizeof(EXTRA_DESCR_DATA) * top_ed);
    sendf(ch, "Exits   %8d (%10d bytes)\n\r", top_exit,      sizeof(EXIT_DATA)        * top_exit);
    sendf(ch, "Mobs    %8d (%10d bytes)\n\r", top_mob_index, sizeof(MOB_INDEX_DATA)   * top_mob_index);
    sendf(ch, "Objs    %8d (%10d bytes)\n\r", top_obj_index, sizeof(OBJ_INDEX_DATA)   * top_obj_index);
    sendf(ch, "Resets  %8d (%10d bytes)\n\r", top_reset,     sizeof(RESET_DATA)       * top_reset);
    sendf(ch, "Rooms   %8d (%10d bytes)\n\r", top_room,      sizeof(ROOM_INDEX_DATA)  * top_room);
    sendf(ch, "Shops   %8d (%10d bytes)\n\r", top_shop,      sizeof(SHOP_DATA)        * top_shop);

    for (num_mob = 0, mob = first_char; mob != NULL; mob = mob->next)
        if (IS_NPC(mob))
            num_mob++;

    sendf(ch, "Number of mobs in game: %d\n\r", num_mob);

    for (num_obj = 0, obj = first_obj; obj != NULL; obj = obj->next)
        num_obj++;

    sendf(ch, "Number of objs in game: %d\n\r", num_obj);

    send_to_char("Shared String Info:\n\r", ch);
    sendf(ch, "Strings           %5ld strings of %7ld bytes (max %ld).\n\r", nAllocString, sAllocString, MAX_STRING);
    sendf(ch, "Overflow Strings  %5ld strings of %7ld bytes.\n\r", nOverFlowString, sOverFlowString);
    sendf(ch, "Perms   %5d blocks  of %7d bytes.\n\r\n\r", nAllocPerm, sAllocPerm);

    SHOW_WASTED_BYTES(AREA_DATA,        area_free,       "areas");
    SHOW_WASTED_BYTES(TELEPORT_DATA,    tele_free,       "teleporters");
    SHOW_WASTED_BYTES(AFFECT_DATA,      affect_free,     "affects");
    SHOW_WASTED_BYTES(ROOM_AFFECT_DATA, raffect_free,    "room affects");
    SHOW_WASTED_BYTES(QUEST_DATA,       quest_free,      "qinfos");
    SHOW_WASTED_BYTES(CINFO_DATA,       cinfo_free,      "cinfos");
    SHOW_WASTED_BYTES(ANSWERING_DATA,   answering_free,  "answering machines");
    SHOW_WASTED_BYTES(IGNORE_DATA,      ignore_free,     "ignores");
    SHOW_WASTED_BYTES(BAN_DATA,         ban_free,        "bans");
    SHOW_WASTED_BYTES(CHAR_DATA,        char_free,       "characters");
    SHOW_WASTED_BYTES(DESCRIPTOR_DATA,  desc_free,       "descriptors");
    SHOW_WASTED_BYTES(EXIT_DATA,        exit_free,       "exits");
    SHOW_WASTED_BYTES(EXTRA_DESCR_DATA, exdesc_free,     "extra descriptions");
    SHOW_WASTED_BYTES(SHELP_DATA,       shelp_free,      "shelps");
    SHOW_WASTED_BYTES(MOB_INDEX_DATA,   mid_free,        "mob indexes");
    SHOW_WASTED_BYTES(NOTE_DATA,        note_free,       "notes");
    SHOW_WASTED_BYTES(OBJ_DATA,         obj_free,        "objects");
    SHOW_WASTED_BYTES(OBJ_INDEX_DATA,   oid_free,        "object indexes");
    SHOW_WASTED_BYTES(PC_DATA,          pcd_free,        "pcdatas");
    SHOW_WASTED_BYTES(RESET_DATA,       reset_free,      "resets");
    SHOW_WASTED_BYTES(ROOM_INDEX_DATA,  rid_free,        "room indexes");
    SHOW_WASTED_BYTES(SHOP_DATA,        shop_free,       "shops");
    SHOW_WASTED_BYTES(MPROG_DATA,       mprog_free,      "mobprogs");
    SHOW_WASTED_BYTES(MPROG_ACT_LIST,   mpact_free,      "mobprog actions");
    SHOW_WASTED_BYTES(BUILD_DATA_LIST,  build_free,      "build data");
    SHOW_WASTED_BYTES(CORPSE_DATA,      corpse_free,     "player corpses");
    SHOW_WASTED_BYTES(RULER_DATA,       ruler_free,      "rulers");
    SHOW_WASTED_BYTES(BRAND_DATA,       brand_data_free, "brands");
    SHOW_WASTED_BYTES(LOCKER_DATA,      locker_free,     "lockers");
    SHOW_WASTED_BYTES(RENAME_DATA,      rename_free,     "renames");
    SHOW_WASTED_BYTES(BUF_DATA_STRUCT,  buf_free,        "bufs");

    if (Full) {
        send_to_char("Shared String Heap is full, increase MAX_STRING.\n\r", ch);
        sendf(ch, "Overflow high-water-mark is %ld bytes.\n\r", hwOverFlow);
    }

    return;
}

void
do_status(CHAR_DATA *ch, char *argument)
{
    send_to_char("             Status Report for Ack! Mud:\n\r", ch);
    send_to_char("             ---------------------------\n\r\n\r", ch);
    do_time(ch, "");

    send_to_char("\n\r", ch);
    send_to_char("The following counts are for *distinct* mobs/objs/rooms, not a count\n\r", ch);
    send_to_char("of how many are actually in the game at this time.\n\r", ch);
    send_to_char("NB. Areas count will include areas used as help files.\n\r\n\r", ch);

    sendf(ch, "Areas   %5d\n\r", top_area);
    sendf(ch, "Mobs    %5d\n\r", top_mob_index);
    sendf(ch, "Objs    %5d\n\r", top_obj_index);
    sendf(ch, "Rooms   %5d\n\r", top_room);

    return;
}

/*
 * Stick a little fuzz on a number.
 */
int
number_fuzzy(int number)
{
    switch (number_bits(2)) {
        case 0:
            number -= 1;
            break;
        case 3:
            number += 1;
            break;
    }

    return UMAX(1, number);
}

/*
 * Generate a random number.
 */
int
number_range(int from, int to)
{
    int                 power;
    int                 number;

    if ((to = to - from + 1) <= 1)
        return from;

    for (power = 2; power < to; power <<= 1);

    while ((number = number_mm() & (power - 1)) >= to);

    return from + number;
}

/*
 * Generate a percentile roll.
 */
int
number_percent(void)
{
    int                 percent;

    while ((percent = number_mm() & (128 - 1)) > 99);

    return 1 + percent;
}

/*
 * Generate a random door.
 */
int
number_door(void)
{
    int                 door;

    while ((door = number_mm() & (8 - 1)) > 5);

    return door;
}

int
number_bits(int width)
{
    return number_mm() & ((1 << width) - 1);
}

/*
 * I've gotten too many bad reports on OS-supplied random number generators.
 * This is the Mitchell-Moore algorithm from Knuth Volume II.
 * Best to leave the constants alone unless you've read Knuth.
 * -- Furey
 */
static int          rgiState[2 + 55];

void
init_mm()
{
    int                *piState;
    int                 iState;

    piState = &rgiState[2];

    piState[-2] = 55 - 55;
    piState[-1] = 55 - 24;

    piState[0] = ((int) current_time) & ((1 << 30) - 1);
    piState[1] = 1;
    for (iState = 2; iState < 55; iState++) {
        piState[iState] = (piState[iState - 1] + piState[iState - 2])
            & ((1 << 30) - 1);
    }
    return;
}

int
number_mm(void)
{
    int                *piState;
    int                 iState1;
    int                 iState2;
    int                 iRand;

    piState = &rgiState[2];
    iState1 = piState[-2];
    iState2 = piState[-1];
    iRand = (piState[iState1] + piState[iState2])
        & ((1 << 30) - 1);
    piState[iState1] = iRand;
    if (++iState1 == 55)
        iState1 = 0;
    if (++iState2 == 55)
        iState2 = 0;
    piState[-2] = iState1;
    piState[-1] = iState2;
    return iRand >> 6;
}

/*
 * Roll some dice.
 */
int
dice(int number, int size)
{
    int                 idice;
    int                 sum;

    switch (size) {
        case 0:
            return 0;
        case 1:
            return number;
    }

    for (idice = 0, sum = 0; idice < number; idice++)
        sum += number_range(1, size);

    return sum;
}

/*
 * Simple linear interpolation.
 */
int
interpolate(int level, int value_00, int value_32)
{
    return value_00 + level * (value_32 - value_00) / 32;
}

/*
 * Removes the tildes from a string.
 * Used for player-entered strings that go into disk files.
 */
void
smash_tilde(char *str)
{
    for (; *str != '\0'; str++) {
        if (*str == '~')
            *str = '-';
    }

    return;
}

/*
 * Compare strings, case insensitive.
 * Return TRUE if different
 *   (compatibility with historical functions).
 */
bool
str_cmp(const char *astr, const char *bstr)
{
    int                 count = 0;
    bool                notrunc = FALSE;

    if (astr == NULL) {
        return TRUE;
    }

    if (bstr == NULL) {
        return TRUE;
    }
    if (*astr == '^') {
        notrunc = TRUE;
        astr++;
    }
    if (*bstr == '^') {
        notrunc = TRUE;
        bstr++;
    }
    if (astr == NULL) {
        return TRUE;
    }

    if (bstr == NULL) {
        return TRUE;
    }

    for (; *astr || *bstr; astr++, bstr++) {
        if (!notrunc)
            if (*astr == '*' && count)
                return FALSE;

        if (LOWER(*astr) != LOWER(*bstr))
            return TRUE;
        count++;
    }

    return FALSE;
}

/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool
str_prefix(const char *astr, const char *bstr)
{
    if (astr == NULL) {
        bug("str_prefix: null astr.");
        return TRUE;
    }

    if (bstr == NULL) {
        return TRUE;
    }

    for (; *astr; astr++, bstr++) {
        if (LOWER(*astr) != LOWER(*bstr))
            return TRUE;
    }

    return FALSE;
}

bool
str_iprefix(const char *astr, const char *bstr)
{
    if (astr == NULL) {
        bug("str_iprefix: null astr.");
        return TRUE;
    }

    if (bstr == NULL) {
        bug("str_iprefix: null bstr.");
        return TRUE;
    }

    for (; *astr; astr++, bstr++) {
        if (*astr != *bstr)
            return TRUE;
    }

    return FALSE;
}

/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns TRUE is astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool
str_infix(const char *astr, const char *bstr)
{
    int                 sstr1;
    int                 sstr2;
    int                 ichar;
    char                c0;

    if ((c0 = LOWER(astr[0])) == '\0')
        return FALSE;

    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);

    for (ichar = 0; ichar <= sstr2 - sstr1; ichar++) {
        if (c0 == LOWER(bstr[ichar]) && !str_prefix(astr, bstr + ichar))
            return FALSE;
    }

    return TRUE;
}

int
strpos(const char *haystack, const char *needle)
{
    int                 sstr1;
    int                 sstr2;
    int                 ichar;
    char                c0;

    if ((c0 = needle[0]) == '\0')
        return FALSE;

    sstr1 = strlen(needle);
    sstr2 = strlen(haystack);

    if (sstr1 > sstr2)
        return -1;

    for (ichar = 0; ichar <= sstr2 - sstr1; ichar++) {
        if (c0 == haystack[ichar] && !str_iprefix(needle, haystack + ichar))
            return ichar;
    }

    return -1;
}

/*
 * Compare strings, case insensitive, for suffix matching.
 * Return TRUE if astr not a suffix of bstr
 *   (compatibility with historical functions).
 */
bool
str_suffix(const char *astr, const char *bstr)
{
    int                 sstr1;
    int                 sstr2;

    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);
    if (sstr1 <= sstr2 && !str_cmp(astr, bstr + sstr2 - sstr1))
        return FALSE;
    else
        return TRUE;
}

/*
 * Returns an initial-capped string.
 */
char               *
capitalize(const char *str)
{
    static char         strcap[MAX_STRING_LENGTH];
    int                 i;

    for (i = 0; str[i] != '\0'; i++)
        strcap[i] = LOWER(str[i]);
    strcap[i] = '\0';
    strcap[0] = UPPER(strcap[0]);
    return strcap;
}

/* integer to comma'ed string.
 *               --- Erigol
 */

char               *
number_comma(int x)
{
    char                buf1[16] = "";
    static char         buf2[16] = "";
    char               *a = buf1;
    char               *p = buf2;
    int                 num = x;
    int                 count = 0;

    do {
        *a++ = '0' + abs(num) % 10;
        count++;
        if (count % 3 == 0 && num / 10)
            *a++ = ',';
    }
    while (num /= 10);

    if (x < 0)
        *p++ = '-';

    do
        *p++ = *--a;
    while (a != buf1);

    *p++ = '\0';

    return buf2;
}

char *
number_comma_r(int x, char *dst)
{
    char                buf1[16] = "";
    char               *a = buf1;
    char               *p = dst;
    int                 num = x;
    int                 count = 0;

    do {
        *a++ = '0' + abs(num) % 10;
        count++;
        if (count % 3 == 0 && num / 10)
            *a++ = ',';
    }
    while (num /= 10);

    if (x < 0)
        *p++ = '-';

    do
        *p++ = *--a;
    while (a != buf1);

    *p++ = '\0';

    return dst;
}

/*
 * Append a string to a file.
 */
void
append_file(CHAR_DATA *ch, char *file, char *str)
{
    FILE               *fp;

    if (nosave)
        return;

    if (IS_NPC(ch) || str[0] == '\0' || !file || file[0] == '\0')
        return;

    fclose(fpReserve);
    if ((fp = fopen(file, "a")) == NULL) {
        perror(file);
        send_to_char("Could not open the file!\n\r", ch);
    }
    else {
        FPRINTF(fp, "[%5d] %s: %s\n", ch->in_room ? ch->in_room->vnum : 0, ch->name, str);
        fclose(fp);
    }

    fpReserve = fopen(NULL_FILE, "r");
    return;
}

/*
 * Nice little functions that limit the amount of typing you have to do with    
 * each and every log File entry and bug report.---Flar
 */
void
bugf(char *fmt, ...)
{
    char                buf[MSL];
    va_list             args;

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    bug(buf);
}

void
xlogf(char *fmt, ...)
{
    char                buf[2 * MSL];
    va_list             args;

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    log_string(buf);
}

void
sendf(CHAR_DATA *ch, char *fmt, ...)
{
    char                buf[MSL * 10];
    va_list             args;

    if (!ch)
        return;

    buf[0] = '\0';

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    send_to_char(buf, ch);
}

void
FPRINTF(FILE *fp, char *fmt, ...)
{
    char                buf[2 * MSL];
    va_list             args;
    size_t              ret;
    int                 len;

    if (nosave)
        return;

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    len = strlen(buf);

    ret = fwrite(buf, len, 1, fp);

    if (ret <= 0) {
        writeerrno = errno;
        nosave = TRUE;
        sprintf(buf, "@@eWARNING: @@gThe MUD has automatically entered @@WNOSAVE @@gmode due to unsuccessfully saving a file. Error Message: @@W%s@@g.@@N",
            strerror(writeerrno));

        notify(buf, 1);
    }
}

/*
 * Reports a bug.
 */
void
bug(const char *str)
{
    char                buf[MAX_STRING_LENGTH];
    FILE               *fp, *fplog;
    char               *strtime, *strlog;

    fplog = fpPfile ? fpPfile : fpArea ? fpArea : NULL;
    strlog = strPfile && strPfile[0] != '\0' ? strPfile : strArea && strArea[0] != '\0' ? strArea : NULL;

    if (fplog != NULL) {
        int                 iLine;
        int                 iChar;

        iChar = ftell(fplog);
        fseek(fplog, 0, 0);

        for (iLine = 0; ftell(fplog) < iChar; iLine++)
            while (getc(fplog) != '\n');

        fseek(fplog, iChar, 0);

        if (strlog) {
            if (strlog == strArea)
                sprintf(buf, "[AreaFileError] FILE: %s LINE: %d", strlog, iLine);
            else
                sprintf(buf, "[PlayerFileError] FILE: %s LINE: %d", strlog, iLine);

            log_string(buf);
        }
    }

    strtime = ctime(&current_time);
    strtime[strlen(strtime) - 1] = '\0';
    sprintf(buf, "[%s] %s\n", strtime, str);

/*    fclose(fpReserve); */

    if (!nosave && (fp = fopen(BUG_FILE, "a")) != NULL) {
        FPRINTF(fp, "%s", buf);
        fclose(fp);
    }

/*    fpReserve = fopen(NULL_FILE, "r"); */

    return;
}

/*
 * Writes a string to the log.
 */
void
log_string(const char *str)
{
    char               *strtime;

    strtime = ctime(&current_time);
    strtime[strlen(strtime) - 1] = '\0';
    printf("%s :: %s\n", strtime, str);
    return;
}

/*
 * This function is here to aid in debugging.
 * If the last expression in a function is another function call,
 *   gcc likes to generate a JMP instead of a CALL.
 * This is called "tail chaining."
 * It hoses the debugger call stack for that call.
 * So I make this the last call in certain critical functions,
 *   where I really need the call stack to be right for debugging!
 *
 * If you don't understand this, then LEAVE IT ALONE.
 * Don't remove any calls to tail_chain anywhere.
 *
 * -- Furey
 */
void
tail_chain(void)
{
    return;
}

/*
 * MOBprogram code block
 */
/* the functions */

/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. This allows the use of the words in the
 *  mob/script files.
 */
int
mprog_name_to_type(name)
char               *name;
{
    if (!str_cmp(name, "in_file_prog"))
        return IN_FILE_PROG;
    if (!str_cmp(name, "act_prog"))
        return ACT_PROG;
    if (!str_cmp(name, "speech_prog"))
        return SPEECH_PROG;
    if (!str_cmp(name, "rand_prog"))
        return RAND_PROG;
    if (!str_cmp(name, "fight_prog"))
        return FIGHT_PROG;
    if (!str_cmp(name, "hitprcnt_prog"))
        return HITPRCNT_PROG;
    if (!str_cmp(name, "death_prog"))
        return DEATH_PROG;
    if (!str_cmp(name, "entry_prog"))
        return ENTRY_PROG;
    if (!str_cmp(name, "greet_prog"))
        return GREET_PROG;
    if (!str_cmp(name, "all_greet_prog"))
        return ALL_GREET_PROG;
    if (!str_cmp(name, "give_prog"))
        return GIVE_PROG;
    if (!str_cmp(name, "bribe_prog"))
        return BRIBE_PROG;
    return (ERROR_PROG);
}

/* Had to redo mobprog loading to work with double linked lists. -- Altrag */
void
mprog_file_read(char *f, MOB_INDEX_DATA *pMobIndex)
{
    MPROG_DATA         *mprog;
    FILE               *fp;
    char                letter;
    char                name[128];
    char               *permf;
    int                 type;

    sprintf(name, "%s%s", MOB_DIR, f);
    if (!(fp = fopen(name, "r"))) {
        bugf("Mob: %d couldn't open mobprog file.", pMobIndex->vnum);
        return;
    }
    permf = str_dup(f);
    for (;;) {
        if ((letter = fread_letter(fp)) == '|')
            break;
        else if (letter != '>') {
            bugf("Mprog_file_read: Invalid letter mob %d.", pMobIndex->vnum);
            return;
        }
        switch ((type = fread_number(fp))) {
            case ERROR_PROG:
                bugf("Mob %d: in file prog error.", pMobIndex->vnum);
                return;
            case IN_FILE_PROG:
                bugf("Mob %d: nested in file progs.", pMobIndex->vnum);
                return;
            default:
                SET_BIT(pMobIndex->progtypes, type);
                GET_FREE(mprog, mprog_free);
                mprog->type = type;
                mprog->arglist = fread_string(fp);
                fread_to_eol(fp);
                mprog->comlist = fread_string(fp);
                fread_to_eol(fp);
                mprog->filename = permf;
                break;
        }
    }
    fclose(fp);
    return;
}

void
load_mobprogs(FILE * fp)
{
    char                letter;
    MOB_INDEX_DATA     *iMob;
    int                 value;

    for (;;)
        switch (LOWER(letter = fread_letter(fp))) {
            default:
                bugf("Load_mobprogs: bad command '%c'.", letter);
                fread_to_eol(fp);
                break;
            case 's':
                fread_to_eol(fp);
                return;
            case '*':
                fread_to_eol(fp);
                break;
            case 'm':
                value = fread_number(fp);
                if (!(iMob = get_mob_index(value))) {
                    bugf("Load_mobprogs: vnum %d doesn't exist.", value);
                    fread_to_eol(fp);
                    break;
                }
                mprog_file_read(fread_word(fp), iMob);
                fread_to_eol(fp);
                break;
        }
    return;
}

void
mprog_read_programs(FILE * fp, MOB_INDEX_DATA *pMobIndex)
{
    MPROG_DATA         *mprog;
    char                letter;
    int                 type;

    for (;;) {
        if ((letter = fread_letter(fp)) == '|')
            break;
        else if (letter != '>') {
            bugf("Load_mobiles: vnum %d MOBPROG char", pMobIndex->vnum);
            ungetc(letter, fp);
            return;
        }
        type = mprog_name_to_type(fread_word(fp));
        switch (type) {
            case ERROR_PROG:
                bugf("Load_mobiles: vnum %d MOBPROG type.", pMobIndex->vnum);
                fread_to_eol(fp);
                return;
            case IN_FILE_PROG:
                mprog_file_read(fread_string(fp), pMobIndex);
                fread_to_eol(fp);
                break;
            default:
                SET_BIT(pMobIndex->progtypes, type);
                GET_FREE(mprog, mprog_free);
                mprog->type = type;
                mprog->arglist = fread_string(fp);
                fread_to_eol(fp);
                mprog->comlist = fread_string(fp);
                fread_to_eol(fp);
                LINK(mprog, pMobIndex->first_mprog, pMobIndex->last_mprog, next, prev);
                break;
        }
    }
    return;
}

void
safe_strcat(int max_len, char *dest, char *source)
{
    int                 a;
    char                c;
    char               *orig_dest;
    char               *orig_source;

    if (dest == NULL && source == NULL)
        return;

    if (dest == NULL) {
        FPRINTF(stdout, "safe_strcat: Null dest string for source : %s\n", source);
        return;
    }

    if (source == NULL) {
        FPRINTF(stdout, "safe_strcat: NULL source for dest : %s\n", dest);
        return;
    }

    orig_dest = dest;
    orig_source = source;

    while (*dest != '\0')        /* Check to see if dest is already over limit. */
        dest++;

    a = dest - orig_dest;
    if (a > max_len) {
        FPRINTF(stdout, "WARNING: dest string already too long:\nsource: %s\ndest: %s\n", source, orig_dest);
        return;
    }

    if (a == max_len) {
        FPRINTF(stdout, "safe_strcat: string too long, source is : %s\n", orig_source);
        return;
    }

    while ((c = *(source++)) != 0) {
        *(dest++) = c;
        a++;
        if (a == max_len) {
            *(--dest) = '\0';
            FPRINTF(stdout, "safe_strcat: string too long, source is : %s\n", orig_source);
            return;
        }
    }

    *dest = '\0';
    return;
}

void
message_update(void)
{
    RESET_DATA         *pReset;
    AREA_DATA          *pArea;
    ROOM_INDEX_DATA    *pRoom;
    CHAR_DATA          *ch;

    for (pArea = first_area; pArea != NULL; pArea = pArea->next) {
        for (pReset = pArea->first_reset; pReset != NULL; pReset = pReset->next) {
            if (pReset->command != 'A')
                continue;

            if ((pRoom = get_room_index(pReset->arg1)) == NULL) {
                bugf("message_update, no room %d found.", pReset->arg1);
                continue;
            }

            for (ch = pRoom->first_person; ch != NULL; ch = ch->next_in_room)
                if (ch->level >= pReset->arg2 && ch->level <= pReset->arg3)
                    send_to_char(pReset->notes, ch);
        }
    }
    return;
}

/*
 * START: shelp addition
 */

/*****
 * wrap words to a formatted output (line1, line2)
 *                                 --- Erigol
 *****/

char               *
wordwrap(char *source, char *line1, char *line2, int length)
{
    static char         output[MSL];
    char                current_line[MSL];
    char                buf[MSL];

    char               *ptr = source;
    char               *line = current_line;
    int                 count = 0;
    int                 first_line = 0;
    int                 found_space = 0;

    output[0] = '\0';
    current_line[0] = '\0';
    buf[0] = '\0';

    for (; *ptr; ptr++) {
        count++;

        *line++ = *ptr;

        if (*ptr == ' ')
            found_space = 1;

        if (count > length) {
            line--;
            while (*line != ' ' && found_space) {
                *line-- = *--ptr;
                count--;
            }

            if (!found_space)
                ptr--;

            while (count++ <= length)
                *line++ = ' ';

            *line++ = '\0';

            if (!first_line) {
                sprintf(buf, line1, current_line);
                first_line = 1;
            }
            else
                sprintf(buf, line2, current_line);

            safe_strcat(MSL, output, buf);

            count = 0;
            found_space = 0;
            current_line[0] = '\0';
            line = current_line;
        }
    }

    while (count++ < length)
        *line++ = ' ';
    *line++ = '\0';

    if (!first_line)
        sprintf(buf, line1, current_line);
    else
        sprintf(buf, line2, current_line);

    safe_strcat(MSL, output, buf);

    return output;
}

void
load_shelps(void)
{
    FILE               *fp;

    if ((fp = fopen(SHELP_FILE, "r")) == NULL)
        return;

    for (;;) {
        SHELP_DATA         *shelp;
        char                letter;

        do {
            letter = getc(fp);
            if (feof(fp)) {
                fclose(fp);
                return;
            }
        }
        while (isspace(letter));
        ungetc(letter, fp);

        GET_FREE(shelp, shelp_free);

        shelp->name = fread_string(fp);
        shelp->duration = fread_string(fp);
        shelp->modify = fread_string(fp);
        shelp->type = fread_string(fp);
        shelp->target = fread_string(fp);
        shelp->desc = fread_string(fp);
        LINK(shelp, first_shelp, last_shelp, next, prev);
    }

    return;
}

/*
 * FINISH: shelp addition
 */

void
load_notes(void)
{
    FILE               *fp;

    if ((fp = fopen(NOTE_FILE, "r")) == NULL)
        return;

    for (;;) {
        NOTE_DATA          *note;
        char                letter;

        do {
            letter = getc(fp);
            if (feof(fp)) {
                fclose(fp);
                return;
            }
        }
        while (isspace(letter));

        ungetc(letter, fp);

        GET_FREE(note, note_free);

        note->from = fread_string(fp);
        note->to = fread_string(fp);
        note->subject = fread_string(fp);
        note->text = fread_string(fp);
        note->date_stamp = fread_number(fp);
        note->unread = (fread_number(fp)) ? TRUE : FALSE;

        /* note expiration */
        if (note->date_stamp > current_time - (60 * 60 * 24 * 200))
            LINK(note, first_note, last_note, next, prev);
        else {
            free_string(note->from);
            free_string(note->to);
            free_string(note->subject);
            free_string(note->text);
            PUT_FREE(note, note_free);
        }
    }

    return;
}

void
load_mudset(void)
{
    FILE               *fp;
    char               *word;
    int                 cnt;
    bool                found = FALSE;
    int                 num;

    if ((fp = fopen(MUDSET_FILE, "r")) == NULL)
        return;

    for (;;) {
        word = feof(fp) ? "End" : fread_word(fp);

        if (!strcmp(word, "End"))
            break;

        for (cnt = 0; mudset_table[cnt].name[0] != '\0'; cnt++)
            if (!str_cmp(word, mudset_table[cnt].name)) {
                found = TRUE;
                break;
            }

        if (found) {
            switch (mudset_table[cnt].type) {
                bool           *type_bool;
                unsigned int   *type_uint;
                char          **type_string;

                case MUDSET_TYPE_BOOL:
                    type_bool = (bool *)mudset_table[cnt].var;

                    num = fread_number(fp);

                    *type_bool = (num) ? TRUE : FALSE;
                    break;

                case MUDSET_TYPE_INT:
                    type_uint = (unsigned int *)mudset_table[cnt].var;
                    num = fread_number(fp);

                    *type_uint = num;
                    break;

                case MUDSET_TYPE_STRING:
                    type_string = (char **)mudset_table[cnt].var;

                    free_string(*type_string);
                    *type_string = fread_string(fp);
                    break;

                default:
                    break;
            }
        }
    }

    fclose(fp);
    return;
}

void
load_renames(void)
{
    FILE               *fp;
    char               *word;
    RENAME_DATA        *rename;

    if ((fp = fopen(RENAME_FILE, "r")) == NULL)
        return;

    for (;;) {
        word = feof(fp) ? "End" : fread_word(fp);

        if (!strcmp(word, "End"))
            break;

        if (!strcmp(word, "Rename")) {
            GET_FREE(rename, rename_free);
            memset(rename, 0, sizeof(RENAME_DATA));

            rename->playername = fread_string(fp);
            rename->oldshort   = fread_string(fp);
            rename->oldlong    = fread_string(fp);
            rename->oldkeyword = fread_string(fp);
            rename->newshort   = fread_string(fp);
            rename->newlong    = fread_string(fp);
            rename->newkeyword = fread_string(fp);
            rename->id         = fread_unumber(fp);

            LINK(rename, first_rename, last_rename, next, prev);
        }
    }

    fclose(fp);
    return;
}
