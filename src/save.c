
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include "merc.h"
#include "duel.h"

IDSTRING(rcsid, "$Id: save.c,v 1.66 2004/11/12 01:12:35 dave Exp $");

#if !defined(macintosh)
extern int _filbuf  args((FILE *));
#endif

/* SAVE_REVISION number defines what has changed:
   7 -> 8: boosted MAX_COLOUR from 15 to 30
 */

#define SAVE_REVISION 9
int current_revision = SAVE_REVISION;

/*
 * Array of containers read for proper re-nesting of objects.
 */
#define MAX_NEST        100
static OBJ_DATA    *rgObjNest[MAX_NEST];

extern FILE        *fpPfile;
extern char         strPfile[MAX_INPUT_LENGTH];

extern int fBootDb;
extern OBJ_DATA    *quest_object;

/*
 * Local functions.
 */
void fwrite_char    args((CHAR_DATA *ch, FILE *fp));
void fwrite_obj     args((CHAR_DATA *ch, OBJ_DATA *obj, FILE *fp, int iNest));
void fread_char     args((CHAR_DATA *ch, FILE *fp));
void fread_obj      args((CHAR_DATA *ch, FILE *fp));
bool can_save       args((CHAR_DATA *ch, OBJ_DATA *obj));
void fread_ignore   args((CHAR_DATA *ch, FILE *fp));
void fwrite_ignore  args((CHAR_DATA *ch, FILE *fp));

void fwrite_locker_obj args((LOCKER_DATA *locker, OBJ_DATA *obj, FILE *fp, int iNest));
void fread_locker_obj  args((FILE *fp));
void load_locker  args((ROOM_INDEX_DATA *room));
void save_locker  args((ROOM_INDEX_DATA *room));

void                abort_wrapper(void);

/* Courtesy of Yaz of 4th Realm */
char               *
initial(const char *str)
{
    static char         strint[MAX_STRING_LENGTH];

    strint[0] = LOWER(str[0]);
    return strint;

}

/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
int                 loop_counter;

int                 locker_amount = 0;
int                 locker_weight = 0;

void
save_char_obj(CHAR_DATA *ch)
{
    char                strsave[MAX_INPUT_LENGTH];
    char                tempstrsave[MAX_INPUT_LENGTH];    /* Hold temp filename here.. */
    char                buf[MAX_INPUT_LENGTH];    /* hold misc stuff here.. */
    extern int          loop_counter;
    FILE               *fp;
    char               *nmptr, *bufptr;

    if (nosave)
        return;

    if (!IS_NPC(ch) && is_in_duel(ch, DUEL_STAGE_GO))
        return;

    if (IS_NPC(ch) || ch->level < 2)
        return;

    if (!IS_NPC(ch)
        && ch->desc != NULL && ch->desc->original != NULL)
        ch = ch->desc->original;

    ch->save_time = current_time;
    fclose(fpReserve);

    /* player files parsed directories by Yaz 4th Realm */
#if !defined(machintosh) && !defined(MSDOS)
    if (IS_NPC(ch)) {            /* convert spaces to . */
        for (nmptr = ch->name, bufptr = buf; *nmptr != 0; nmptr++) {
            if (*nmptr == ' ')
                *(bufptr++) = '.';
            else
                *(bufptr++) = *nmptr;
        }
        *(bufptr) = *nmptr;
    }
    else {
        if (IS_SET(ch->pcdata->pflags, PFLAG_SPECIALNAME))
            strcpy(buf, ch->pcdata->origname);
        else
            strcpy(buf, ch->name);
    }
    sprintf(strsave, "%s%s%s%s", PLAYER_DIR, initial(buf), "/", capitalize(buf));
#else
    /* Convert npc names to dos compat name.... yuk */
    if (IS_NPC(ch)) {
        for (nmptr = ch->name, bufptr = buf; *nmptr != 0; nmptr++) {
            if (*nmptr != ' ' && *nmptr != '.')
                *(bufptr++) = *nmptr;
            if (bufptr - buf == 8)
                break;
        }
        *(bufptr) = 0;
    }
    else {
        if (IS_SET(ch->pcdata->pflags, PFLAG_SPECIALNAME))
            strcpy(buf, ch->pcdata->origname);
        else
            strcpy(buf, ch->name);
    }

    sprintf(strsave, "%s%s", IS_NPC(ch) ? NPC_DIR : PLAYER_DIR, capitalize(buf));
#endif
    /* Tack on a .temp to strsave, use as tempstrsave */

    sprintf(tempstrsave, "%s.temp", strsave);

    /* when reading/writing pfiles, lets set up some variables for later usage
     * should a bug occur. */
    strcpy(strPfile, tempstrsave);

    if ((fp = fopen(tempstrsave, "w")) == NULL) {
        writeerrno = errno;
        nosave = TRUE;
        sprintf(buf, "@@eWARNING: @@gThe MUD has automatically entered @@WNOSAVE @@gmode due to unsuccessfully saving a file. Error Message: @@W%s@@g.@@N",
            strerror(writeerrno));

        notify(buf, 1);
    }
    else {
        fpPfile = fp;
        fwrite_char(ch, fp);
        loop_counter = 0;
        if (ch->first_carry != NULL)
            fwrite_obj(ch, ch->first_carry, fp, 0);

        if (!IS_NPC(ch) && ch->pcdata->first_ignore != NULL)
            fwrite_ignore(ch, fp);

        FPRINTF(fp, "#END\n");
        fflush(fp);
        fclose(fp);
        /* Now make temp file the actual pfile... */
        if (!nosave)
            rename(tempstrsave, strsave);
        else
            remove(tempstrsave);
    }

    fpPfile = NULL;
    strPfile[0] = '\0';

    fpReserve = fopen(NULL_FILE, "r");
    return;
}

/*
 * Write the char.
 */
void
fwrite_char(CHAR_DATA *ch, FILE * fp)
{
    /* UUURRRGGGGHHHHHH!  When writing out ch->lvl[x] no loop used,
     * instead, the values are just done 0,1,2,etc.. yuck.  -S- 
     */

    AFFECT_DATA        *paf;
    int                 cnt;
    int                 sn;
    int                 foo;

    if (IS_NPC(ch) || !ch->pcdata)
        return;

    /* Really cool fix for m/c prob.. *laugh* */
    for (cnt = 0; cnt < MAX_CLASS; cnt++)
        if (ch->lvl[cnt] < 0 || ch->lvl[cnt] == 0)
            ch->lvl[cnt] = -1;

    FPRINTF(fp, "#PLAYER\n");

    FPRINTF(fp, "Revision      %d\n",  SAVE_REVISION);
    FPRINTF(fp, "Name          %s~\n", ch->name);
    FPRINTF(fp, "ShortDescr    %s~\n", ch->short_descr);
    FPRINTF(fp, "LongDescr     %s~\n", ch->long_descr_orig);
    FPRINTF(fp, "Description   %s~\n", ch->description);
    FPRINTF(fp, "Prompt        %s~\n", ch->pcdata->prompt);
    FPRINTF(fp, "Battleprompt  %s~\n", ch->pcdata->battleprompt);
    FPRINTF(fp, "Noteprompt    %s~\n", ch->pcdata->noteprompt);
    FPRINTF(fp, "Sex           %d\n",  ch->sex);
    FPRINTF(fp, "LoginSex      %d\n",  ch->login_sex);
    FPRINTF(fp, "Afkmsg        %s~\n", ch->afk_msg);
    FPRINTF(fp, "Class         %d\n",  ch->class);
    FPRINTF(fp, "Race          %d\n",  ch->race);
    FPRINTF(fp, "Level         %d\n",  ch->level);
    FPRINTF(fp, "Sentence      %d\n",  ch->sentence);
    FPRINTF(fp, "Invis         %d\n",  ch->invis);

    FPRINTF(fp, "m/c           ");
    for (cnt = 0; cnt < MAX_CLASS; cnt++)
        FPRINTF(fp, "%2d ", ch->lvl[cnt]);
    FPRINTF(fp, "\n");

    FPRINTF(fp, "Remort        ");
    for (cnt = 0; cnt < MAX_CLASS; cnt++)
        FPRINTF(fp, "%2d ", ch->lvl2[cnt]);
    FPRINTF(fp, "\n");
    FPRINTF(fp, "Adeptlevel    ");
    FPRINTF(fp, "%2d\n", ch->adept_level);
    FPRINTF(fp, "Avatar        %d\n",  ch->pcdata->avatar);
    FPRINTF(fp, "Trust         %d\n",  ch->trust);
    FPRINTF(fp, "Wizbit        %d\n",  ch->wizbit);
    FPRINTF(fp, "Played        %d\n",  ch->played + (int) (current_time - ch->logon));
    FPRINTF(fp, "NewsLastRead  %d\n",  (int)ch->pcdata->news_last_read);

    /* check for limbo idlers and prevent quitting out in noquit rooms */
    if (!ch->in_room)
        FPRINTF(fp, "Room          %d\n", ROOM_VNUM_LIMBO);
    else if (ch->in_room == get_room_index(ROOM_VNUM_LIMBO) && ch->was_in_room != NULL && !IS_SET(ch->was_in_room->room_flags, ROOM_NO_QUIT))
        FPRINTF(fp, "Room          %d\n", ch->was_in_room->vnum);
    else if (!IS_SET(ch->in_room->room_flags, ROOM_NO_QUIT))
        FPRINTF(fp, "Room          %d\n", ch->in_room->vnum);
    else
        FPRINTF(fp, "Room          %d\n", ROOM_VNUM_LIMBO);

    FPRINTF(fp, "HpManaMove    %d %d %d %d %d %d\n", ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move);
    FPRINTF(fp, "Energy        %d %d\n", ch->energy, ch->max_energy);
    FPRINTF(fp, "EnergyLevel   %d\n", ch->pcdata->energy_level);
    FPRINTF(fp, "EnergyUsed    %d\n", ch->pcdata->energy_used);
    FPRINTF(fp, "Gold          %d\n", ch->gold);
    FPRINTF(fp, "Balance       %d\n", ch->balance);
    FPRINTF(fp, "Exp           %d\n", ch->exp);
    FPRINTF(fp, "Act           %d\n", ch->act);
    FPRINTF(fp, "Config        %d\n", ch->config);
    FPRINTF(fp, "AffectedBy    %d\n", ch->affected_by);

    /* only store certain positions on pfiles.. fighting/writing/building == bad */
    FPRINTF(fp, "Position      %d\n", (ch->position == POS_FIGHTING || ch->position == POS_WRITING || ch->position == POS_BUILDING) ? POS_STANDING : ch->position);

    FPRINTF(fp, "Practice      %d\n", ch->practice);
    FPRINTF(fp, "SavingThrow   %d\n", ch->saving_throw);
    FPRINTF(fp, "Alignment     %d\n", ch->alignment);
    FPRINTF(fp, "Hitroll       %d\n", ch->hitroll);
    FPRINTF(fp, "Damroll       %d\n", ch->damroll);
    FPRINTF(fp, "Armor         %d\n", ch->armor);
    FPRINTF(fp, "Wimpy         %d\n", ch->wimpy);
    FPRINTF(fp, "Deaf          %d\n", ch->deaf);
    FPRINTF(fp, "Deaf2         %d\n", ch->deaf2);

    FPRINTF(fp, "Generation    %d\n", ch->pcdata->generation);
    FPRINTF(fp, "Clan          %d\n", ch->pcdata->clan);
    FPRINTF(fp, "Order         %d %d %d %d %d\n",
        ch->pcdata->order[0], ch->pcdata->order[1], ch->pcdata->order[2], ch->pcdata->order[3], ch->pcdata->order[4]);

    FPRINTF(fp, "Index         %d %d %d %d %d\n",
        ch->pcdata->index[0], ch->pcdata->index[1], ch->pcdata->index[2], ch->pcdata->index[3], ch->pcdata->index[4]);

    FPRINTF(fp, "Mkills        %d\n", ch->pcdata->mkills);
    FPRINTF(fp, "Mkilled       %d\n", ch->pcdata->mkilled);
    FPRINTF(fp, "Circlesatt    %d\n", ch->pcdata->circles_attempted);
    FPRINTF(fp, "Circlesland   %d\n", ch->pcdata->circles_landed);
    FPRINTF(fp, "Pkills        %d\n", ch->pcdata->pkills);
    FPRINTF(fp, "Unpkills      %d\n", ch->pcdata->unpkills);
    FPRINTF(fp, "Pkilled       %d\n", ch->pcdata->pkilled);
    FPRINTF(fp, "Password      %s~\n", ch->pcdata->pwd);
    FPRINTF(fp, "Bamfin        %s~\n", ch->pcdata->bamfin);
    FPRINTF(fp, "Bamfout       %s~\n", ch->pcdata->bamfout);
    FPRINTF(fp, "Roomenter     %s~\n", ch->pcdata->room_enter);
    FPRINTF(fp, "Roomexit      %s~\n", ch->pcdata->room_exit);
    FPRINTF(fp, "Title         %s~\n", ch->pcdata->title);
    FPRINTF(fp, "Immskll       %s~\n", ch->pcdata->immskll);
    /* We add a 'W' to preserve leading spaces... strip W on load */
    FPRINTF(fp, "Whoname       W%s~\n", ch->pcdata->who_name);
    FPRINTF(fp, "Monitor       %d\n", ch->pcdata->monitor);
    FPRINTF(fp, "Host          %s~\n", ch->pcdata->host);
    FPRINTF(fp, "Failures      %d\n", ch->pcdata->failures);
    FPRINTF(fp, "LastLogint    %d\n", (int) ch->pcdata->lastlogint);
    FPRINTF(fp, "Deserttime    %d\n", (int) ch->pcdata->desert_time);
    FPRINTF(fp, "HiCol         %c~\n", ch->pcdata->hicol);
    FPRINTF(fp, "DimCol        %c~\n", ch->pcdata->dimcol);

    for (cnt = 0; cnt < MAX_ALIASES; cnt++) {
        FPRINTF(fp, "Alias_Name%d   %s~\n", cnt, ch->pcdata->alias_name[cnt]);
        FPRINTF(fp, "Alias%d        %s~\n", cnt, ch->pcdata->alias[cnt]);
    }

    FPRINTF(fp, "Colours      ");
    for (foo = 0; foo < MAX_COLOUR; foo++) {
        if (foo < MAX_COLOUR - 1)
            FPRINTF(fp, "%d ", ch->pcdata->colour[foo]);
        else
            FPRINTF(fp, "%d\n", ch->pcdata->colour[foo]);
    }

    FPRINTF(fp, "AttrPerm      %d %d %d %d %d\n",
        ch->pcdata->perm_str, ch->pcdata->perm_int, ch->pcdata->perm_wis, ch->pcdata->perm_dex, ch->pcdata->perm_con);

    FPRINTF(fp, "AttrMod       %d %d %d %d %d\n", 0, 0, 0, 0, 0);

    FPRINTF(fp, "AttrMax       %d %d %d %d %d\n",
        ch->pcdata->max_str, ch->pcdata->max_int, ch->pcdata->max_wis, ch->pcdata->max_dex, ch->pcdata->max_con);

    FPRINTF(fp, "Questpoints   %d\n", ch->quest_points);
    FPRINTF(fp, "RecallVnum    %d\n", ch->pcdata->recall_vnum);
    FPRINTF(fp, "KeepVnum      %d\n", ch->pcdata->keep_vnum);
    FPRINTF(fp, "KdonVnum      %d\n", ch->pcdata->kdon_vnum);
    FPRINTF(fp, "GainMana      %d\n", ch->pcdata->mana_from_gain);
    FPRINTF(fp, "GainHp        %d\n", ch->pcdata->hp_from_gain);
    FPRINTF(fp, "GainMove      %d\n", ch->pcdata->move_from_gain);
    FPRINTF(fp, "Condition     %d %d %d\n", ch->pcdata->condition[0], ch->pcdata->condition[1], ch->pcdata->condition[2]);

    FPRINTF(fp, "Pagelen       %d\n", ch->pcdata->pagelen);
    FPRINTF(fp, "Pflags        %d\n", ch->pcdata->pflags);
    FPRINTF(fp, "Autostance    %s~\n", ch->pcdata->autostance);

    for (sn = 0; sn < MAX_SKILL; sn++) {
        if (skill_table[sn].name != NULL && ch->pcdata->learned[sn] > 0) {
            FPRINTF(fp, "Skill         %d '%s'\n", ch->pcdata->learned[sn], skill_table[sn].name);
        }
    }

    for (paf = ch->first_affect; paf != NULL; paf = paf->next) {
        if (paf->type == gsn_emount || paf->save == FALSE)
            continue;

        FPRINTF(fp, "Affect        %d %d %d %d %d\n", paf->type, paf->duration, paf->modifier, paf->location, paf->bitvector);
    }

    FPRINTF(fp, "End\n\n");
    return;
}

/*
 * Write an object and its contents.
 */
void
fwrite_obj(CHAR_DATA *ch, OBJ_DATA *obj, FILE * fp, int iNest)
{
    EXTRA_DESCR_DATA   *ed;
    AFFECT_DATA        *paf;

    /*
     * Slick recursion to write lists backwards,
     *   so loading them will load in forwards order.
     */

    if (obj->next_in_carry_list != NULL)
        fwrite_obj(ch, obj->next_in_carry_list, fp, iNest);

    /*
     * Castrate storage characters.
     */

    /* Also bypass no-save objects -S- */

    if (!can_save(ch, obj))
        return;

    loop_counter++;

    if (loop_counter > 650)
        return;

    FPRINTF(fp, "#OBJECT\n");
    FPRINTF(fp, "Nest         %d\n", iNest);

    if (obj->id > 0)
        FPRINTF(fp, "Id           %u\n", obj->id);

    FPRINTF(fp, "Name         %s~\n", obj->name);
    FPRINTF(fp, "ShortDescr   %s~\n", obj->short_descr);
    FPRINTF(fp, "Description  %s~\n", obj->description);
    FPRINTF(fp, "Vnum         %d\n", obj->pIndexData->vnum);
    FPRINTF(fp, "ExtraFlags   %d\n", obj->extra_flags);
    FPRINTF(fp, "WearFlags    %d\n", obj->wear_flags);
    FPRINTF(fp, "WearLoc      %d\n", obj->wear_loc);
    if (obj->obj_fun != NULL)
        FPRINTF(fp, "Objfun       %s~\n", rev_obj_fun_lookup((void *) obj->obj_fun));

    FPRINTF(fp, "ItemApply    %d\n", obj->item_apply);
    FPRINTF(fp, "ItemType     %d\n", obj->item_type);
    FPRINTF(fp, "Weight       %d\n", obj->weight);
    FPRINTF(fp, "Level        %d\n", obj->level);
    FPRINTF(fp, "Timer        %d\n", obj->timer);
    FPRINTF(fp, "Cost         %d\n", obj->cost);
    FPRINTF(fp, "Values       %d %d %d %d\n", obj->value[0], obj->value[1], obj->value[2], obj->value[3]);

    switch (obj->item_type) {
        case ITEM_POTION:
        case ITEM_SCROLL:
            if (obj->value[1] > 0) {
                FPRINTF(fp, "Spell 1      '%s'\n", skill_table[obj->value[1]].name);
            }

            if (obj->value[2] > 0) {
                FPRINTF(fp, "Spell 2      '%s'\n", skill_table[obj->value[2]].name);
            }

            if (obj->value[3] > 0) {
                FPRINTF(fp, "Spell 3      '%s'\n", skill_table[obj->value[3]].name);
            }

            break;

        case ITEM_PILL:
        case ITEM_STAFF:
        case ITEM_WAND:
            if (obj->value[3] > 0) {
                FPRINTF(fp, "Spell 3      '%s'\n", skill_table[obj->value[3]].name);
            }

            break;
    }

    for (paf = obj->first_apply; paf != NULL; paf = paf->next) {
        FPRINTF(fp, "Affect       %d %d %d %d %d\n", paf->type, paf->duration, paf->modifier, paf->location, paf->bitvector);
    }

    for (ed = obj->first_exdesc; ed != NULL; ed = ed->next) {
        FPRINTF(fp, "ExtraDescr   %s~ %s~\n", ed->keyword, ed->description);
    }

    FPRINTF(fp, "End\n\n");

    if (obj->first_in_carry_list != NULL)
        fwrite_obj(ch, obj->first_in_carry_list, fp, iNest + 1);

    return;
}

bool
can_save(CHAR_DATA *ch, OBJ_DATA *obj)
{
    sh_int              cnt;
    OBJ_INDEX_DATA     *pObj;
    int                 clan = 0;

    if (get_pseudo_level(ch) + 5 < (obj->level)

        || obj->item_type == ITEM_KEY || obj->item_type == ITEM_BEACON || IS_SET(obj->extra_flags, ITEM_NOSAVE))
        return FALSE;

    if (IS_IMMORTAL(ch))
        return TRUE;

    if (IS_SET(obj->extra_flags, ITEM_ADEPT)) {
        if (ch->adept_level >= obj->level)
            return TRUE;
        else
            return FALSE;
    }

    if (!IS_SET(obj->extra_flags, ITEM_CLAN_EQ))
        return TRUE;

    /* Helper clan gets treated as clan none when it comes to clan objects */
    if (ch->pcdata->clan != 2)
        clan = ch->pcdata->clan;
    else
        clan = 0;

    for (cnt = 0; cnt < MAX_CLAN_EQ; cnt++) {
        if (clan_table[clan].eq[cnt] != -1 && ((pObj = get_obj_index(clan_table[clan].eq[cnt])) != NULL)) {
            if (obj->pIndexData->vnum == pObj->vnum)
                return TRUE;
        }
    }

    return FALSE;
}

/* so the stack doesn't get hosed */
void
abort_wrapper(void)
{
    abort();
}

/* Nasty hack for db.c to get back address of ch */
CHAR_DATA          *loaded_mob_addr;

/*
 * Load a char and inventory into a new ch structure.
 */
bool
load_char_obj(DESCRIPTOR_DATA *d, char *name, bool syscall)
{
    int                 cnt;
    static PC_DATA      pcdata_zero;
    char                strsave[MAX_INPUT_LENGTH];
    CHAR_DATA          *ch;
    FILE               *fp;
    bool                found;
    int                 foo;

    GET_FREE(ch, char_free);
    clear_char(ch);

    GET_FREE(ch->pcdata, pcd_free);
    *ch->pcdata = pcdata_zero;

    d->character = ch;

    ch->pcdata->host                    = str_dup("Unknown!");
    ch->pcdata->lastlogint              = (time_t)0;
    ch->pcdata->desert_time             = (time_t)0;
    ch->pcdata->who_name                = str_dup("off");
    ch->pcdata->accept_name             = str_dup("");
    ch->pcdata->pwd                     = str_dup("");
    ch->pcdata->bamfin                  = str_dup("");
    ch->pcdata->bamfout                 = str_dup("");
    ch->pcdata->room_enter              = str_dup("");
    ch->pcdata->room_exit               = str_dup("");
    ch->pcdata->title                   = str_dup("");
    ch->pcdata->immskll                 = str_dup("");
    ch->pcdata->perm_str                = 13;
    ch->pcdata->perm_int                = 13;
    ch->pcdata->perm_wis                = 13;
    ch->pcdata->perm_dex                = 13;
    ch->pcdata->perm_con                = 13;
    ch->pcdata->condition[COND_THIRST]  = 48;
    ch->pcdata->pagelen                 = 20;
    ch->pcdata->condition[COND_FULL]    = 48;
    ch->pcdata->pkills                  = 0;
    ch->pcdata->unpkills                = 0;
    ch->pcdata->pkilled                 = 0;
    ch->pcdata->mkills                  = 0;
    ch->pcdata->mkilled                 = 0;
    ch->pcdata->circles_attempted       = 0;
    ch->pcdata->circles_landed          = 0;
    ch->pcdata->pflags                  = 0;
    ch->pcdata->recall_vnum             = 3001;
    ch->pcdata->keep_vnum               = 0;
    ch->pcdata->kdon_vnum               = 0;
    ch->pcdata->mana_from_gain          = -1;
    ch->pcdata->hp_from_gain            = -1;
    ch->pcdata->move_from_gain          = -1;
    ch->pcdata->hicol                   = 'a';
    ch->pcdata->dimcol                  = 'c';
    ch->pcdata->origname                = str_dup(name);
    ch->pcdata->prompt                  = str_dup("%n<%hhp %mm %vmv> ");
    ch->pcdata->battleprompt            = str_dup("@@N@@g[%n @@d(%b@@d)@@g] [%N @@d(%B@@d)@@g]@@N%c");
    ch->pcdata->noteprompt              = str_dup("@@N@@R[@@e[@@W[@@g You have @@y%a @@gnew note%s @@W]@@e]@@R]@@N%c");
    ch->pcdata->safetimer               = 0;
    ch->pcdata->fighttimer              = 0;
    ch->pcdata->in_arena                = FALSE;
    ch->pcdata->deimm                   = FALSE;
    ch->pcdata->arena_save_hp           = 1;
    ch->pcdata->arena_save_mana         = 1;
    ch->pcdata->arena_save_move         = 1;
    ch->pcdata->arena_save_energy       = 1;
    ch->pcdata->arena_save_first_affect = NULL;
    ch->pcdata->arena_save_last_affect  = NULL;
    ch->pcdata->avatar                  = FALSE;
    ch->pcdata->energy_level            = 0;
    ch->pcdata->energy_used             = 0;
    ch->pcdata->idlecheck               = (time_t)0;

    for (foo = 0; foo < 5; foo++)
        ch->pcdata->pedit_string[foo]   = str_dup("none");

    ch->quest_points = 0;
    for (foo = 0; foo < MAX_CLASS; foo++)
        ch->lvl2[foo] = -1;
    ch->adept_level = -1;

    for (cnt = 0; cnt < MAX_ALIASES; cnt++) {
        ch->pcdata->alias_name[cnt]     = str_dup("<none>");
        ch->pcdata->alias[cnt]          = str_dup("<none>");
    }

    ch->pcdata->autostance              = str_dup("");
    ch->pcdata->news_last_read          = (time_t)0;
    ch->pcdata->stealth                 = 0;

    {
        /* set hunt colour attribute to red by default */

        int col = -1;
        int ansi = -1;

        /* find hunt colour_table index */
        for (foo = 0; foo < MAX_COLOUR; foo++) {
            if (!str_cmp("hunt", colour_table[foo].name)) {
                col = colour_table[foo].index;
                break;
            }
        }

        /* find red ansi_table index */
        for (foo = 0; foo < MAX_ANSI; foo++) {
            if (!str_cmp("red", ansi_table[foo].name)) {
                ansi = ansi_table[foo].index;
                break;
            }
        }

        /* set hunt to red */
        if (col > -1 && ansi > -1)
            ch->pcdata->colour[col] = ansi;
    }

    for (cnt = 0; cnt < MAX_TRADEITEMS; cnt++) {
        ch->pcdata->trading_objs[cnt]   = NULL;
    }

    for (cnt = 0; cnt < MAX_COOKIES; cnt++) {
        ch->pcdata->cookies[cnt][0]     = '\0';
        ch->pcdata->cookiesexpire[cnt]  = (time_t)0;
    }

    ch->pcdata->trading_with            = NULL;
    ch->pcdata->trading_accepts         = FALSE;
    ch->pcdata->trading_room            = NULL;
    ch->pcdata->rename                  = NULL;

    ch->stunTimer                       = 0;
    ch->first_shield                    = NULL;
    ch->last_shield                     = NULL;
    ch->deaf                            = 0;
    ch->deaf2                           = 0;
    ch->desc                            = d;
    free_string(ch->name);
    ch->name                            = str_dup(name);
    ch->act                             =     PLR_BLANK
                                            | PLR_COMBINE
                                            | PLR_PROMPT
                                            | PLR_NOSUMMON
                                            | PLR_AUTOEXIT
                                            | PLR_COLOUR
                                            | PLR_NOVISIT;
    ch->config                          = PLR_AUTOGOLD;
    ch->sex                             = SEX_NEUTRAL;
    ch->login_sex                       = -1;
    ch->current_brand                   = NULL;
    ch->stance                          = 0;
    ch->stance_ac_mod                   = 0;
    ch->stance_dr_mod                   = 0;
    ch->stance_hr_mod                   = 0;
    ch->tele_timer                      = 0;
    ch->energy                          = 0;
    ch->max_energy                      = 0;
    ch->energy_wait                     = 3;
    ch->energy_wait_count               = 3;

    found = FALSE;
    fclose(fpReserve);

    /* parsed player file directories by Yaz of 4th Realm */
    /* decompress if .gz file exists - Thx Alander */

    sprintf(strsave, "%s%s%s%s", PLAYER_DIR, initial(name), "/", capitalize(name));
    strcpy(strPfile, strsave);

    if ((fp = fopen(strsave, "r")) != NULL) {
        int                 iNest;

        fpPfile = fp;

        for (iNest = 0; iNest < MAX_NEST; iNest++)
            rgObjNest[iNest] = NULL;

        found = TRUE;
        for (;;) {
            char                letter;
            char               *word;

            letter = fread_letter(fp);
            if (letter == '*') {
                fread_to_eol(fp);
                continue;
            }

            if (letter != '#') {
                monitor_chan("Load_char_obj: # not found.", MONITOR_BAD);
                break;
            }

            word = fread_word(fp);
            if (!str_cmp(word, "PLAYER"))
                fread_char(ch, fp);
            else if (!str_cmp(word, "MOB"))
                fread_char(ch, fp);
            else if (!str_cmp(word, "OBJECT"))
                fread_obj(ch, fp);
            else if (!str_cmp(word, "IGNORE"))
                fread_ignore(ch, fp);
            else if (!str_cmp(word, "END"))
                break;
            else {
                monitor_chan("Load_char_obj: bad section.", MONITOR_BAD);
                break;
            }
        }
        fclose(fp);
        fpPfile = NULL;
    }

    strPfile[0] = '\0';

    fpReserve = fopen(NULL_FILE, "r");

    tail_chain();
    return found;
}

/*
 * Read in a char.
 */

#if defined(KEY)
#undef KEY
#endif

#define KEY(literal, field, value)  if (!str_cmp(word, literal)) { field = value; fMatch = TRUE; break; }
#define SKEY(literal, field, value) if (!str_cmp(word, literal)) { if (field != NULL) free_string(field); field = value; fMatch = TRUE; break; }

void
fread_char(CHAR_DATA *ch, FILE * fp)
{
    char                buf[MAX_STRING_LENGTH];
    char               *word;
    char               *specname;
    bool                fMatch;
    int                 cnt;
    int                 ignore;
    char               *ignorec;

    specname = str_dup("");
    ignorec = str_dup("");

    ch->balance = 0;

    for (cnt = 0; cnt < MAX_CLASS; cnt++)
        ch->lvl[cnt] = -1;

    for (;;) {
        word = feof(fp) ? "End" : fread_word(fp);
        fMatch = FALSE;

        switch (UPPER(word[0])) {
            case '*':
                fMatch = TRUE;
                fread_to_eol(fp);
                break;

            case 'A':
                KEY("Act", ch->act, fread_number(fp));
                KEY("AffectedBy", ch->affected_by, fread_number(fp));
                KEY("Alignment", ch->alignment, fread_number(fp));
                KEY("Armor", ch->armor, fread_number(fp));
                SKEY("Afkmsg", ch->afk_msg, fread_string(fp));
                KEY("Adeptlevel", ch->adept_level, fread_number(fp));

                SKEY("Alias_Name0", ch->pcdata->alias_name[0], fread_string(fp));
                SKEY("Alias_Name1", ch->pcdata->alias_name[1], fread_string(fp));
                SKEY("Alias_Name2", ch->pcdata->alias_name[2], fread_string(fp));
                SKEY("Alias_Name3", ch->pcdata->alias_name[3], fread_string(fp));
                SKEY("Alias_Name4", ch->pcdata->alias_name[4], fread_string(fp));
                SKEY("Alias_Name5", ch->pcdata->alias_name[5], fread_string(fp));
                SKEY("Alias_Name6", ch->pcdata->alias_name[6], fread_string(fp));
                SKEY("Alias_Name7", ch->pcdata->alias_name[7], fread_string(fp));
                SKEY("Alias_Name8", ch->pcdata->alias_name[8], fread_string(fp));
                SKEY("Alias_Name9", ch->pcdata->alias_name[9], fread_string(fp));
                SKEY("Alias_Name10", ch->pcdata->alias_name[10], fread_string(fp));
                SKEY("Alias_Name11", ch->pcdata->alias_name[11], fread_string(fp));
                SKEY("Alias0", ch->pcdata->alias[0], fread_string(fp));
                SKEY("Alias1", ch->pcdata->alias[1], fread_string(fp));
                SKEY("Alias2", ch->pcdata->alias[2], fread_string(fp));
                SKEY("Alias3", ch->pcdata->alias[3], fread_string(fp));
                SKEY("Alias4", ch->pcdata->alias[4], fread_string(fp));
                SKEY("Alias5", ch->pcdata->alias[5], fread_string(fp));
                SKEY("Alias6", ch->pcdata->alias[6], fread_string(fp));
                SKEY("Alias7", ch->pcdata->alias[7], fread_string(fp));
                SKEY("Alias8", ch->pcdata->alias[8], fread_string(fp));
                SKEY("Alias9", ch->pcdata->alias[9], fread_string(fp));
                SKEY("Alias10", ch->pcdata->alias[10], fread_string(fp));
                SKEY("Alias11", ch->pcdata->alias[11], fread_string(fp));

                if (!str_cmp(word, "Affect")) {
                    AFFECT_DATA        *paf;

                    GET_FREE(paf, affect_free);
                    paf->type = fread_number(fp);
                    paf->duration = fread_number(fp);
                    paf->modifier = fread_number(fp);
                    paf->location = fread_number(fp);
                    paf->bitvector = fread_number(fp);
                    paf->caster = NULL;

                    if (paf->type != -1)
                        LINK(paf, ch->first_saved_aff, ch->last_saved_aff, next, prev);
                    else
                        PUT_FREE(paf, affect_free);

                    fMatch = TRUE;
                    break;
                }

                if (!str_cmp(word, "AttrMod")) {
                    ch->pcdata->mod_str = fread_number(fp);
                    ch->pcdata->mod_int = fread_number(fp);
                    ch->pcdata->mod_wis = fread_number(fp);
                    ch->pcdata->mod_dex = fread_number(fp);
                    ch->pcdata->mod_con = fread_number(fp);
                    fMatch = TRUE;
                    break;
                }

                if (!str_cmp(word, "AttrMax")) {
                    ch->pcdata->max_str = fread_number(fp);
                    ch->pcdata->max_int = fread_number(fp);
                    ch->pcdata->max_wis = fread_number(fp);
                    ch->pcdata->max_dex = fread_number(fp);
                    ch->pcdata->max_con = fread_number(fp);
                    fMatch = TRUE;
                    break;
                }

                if (!str_cmp(word, "AttrPerm")) {
                    ch->pcdata->perm_str = fread_number(fp);
                    ch->pcdata->perm_int = fread_number(fp);
                    ch->pcdata->perm_wis = fread_number(fp);
                    ch->pcdata->perm_dex = fread_number(fp);
                    ch->pcdata->perm_con = fread_number(fp);
                    fMatch = TRUE;
                    break;
                }

                SKEY("Autostance", ch->pcdata->autostance, fread_string(fp));
                KEY("Avatar", ch->pcdata->avatar, fread_number(fp));
                break;

            case 'B':
                KEY("Balance", ch->balance, fread_number(fp));
                SKEY("Battleprompt", ch->pcdata->battleprompt, fread_string(fp));
                KEY("Bloodlust", ignore, fread_number(fp));
                KEY("Bloodlustmax", ignore, fread_number(fp));
                KEY("Bprompt", ignore, fread_number(fp));
                SKEY("Bamfin", ch->pcdata->bamfin, fread_string(fp));
                SKEY("Bamfout", ch->pcdata->bamfout, fread_string(fp));
                break;

            case 'C':
                KEY("Clan", ch->pcdata->clan, fread_number(fp));
                KEY("Class", ch->class, fread_number(fp));
                KEY("Config", ch->config, fread_number(fp));
                KEY("Circlesatt", ch->pcdata->circles_attempted, fread_number(fp));
                KEY("Circlesland", ch->pcdata->circles_landed, fread_number(fp));

                if (!str_cmp(word, "Colours")) {
                    int                 foo;
                    int                 maxcol;

                    if (current_revision < 8)
                        maxcol = 15;
                    else
                        maxcol = MAX_COLOUR;

                    for (foo = 0; foo < maxcol; foo++)
                        ch->pcdata->colour[foo] = fread_number(fp);

                    if (current_revision < 9) {
                        /* set hunt colour attribute to red by default */

                        int col = -1;
                        int ansi = -1;

                        /* find hunt colour_table index */
                        for (foo = 0; foo < MAX_COLOUR; foo++) {
                            if (!str_cmp("hunt", colour_table[foo].name)) {
                                col = colour_table[foo].index;
                                break;
                            }
                        }

                        /* find red ansi_table index */
                        for (foo = 0; foo < MAX_ANSI; foo++) {
                            if (!str_cmp("red", ansi_table[foo].name)) {
                                ansi = ansi_table[foo].index;
                                break;
                            }
                        }

                        /* set hunt to red */
                        if (col > -1 && ansi > -1)
                            ch->pcdata->colour[col] = ansi;
                    }

                    fMatch = TRUE;
                    break;
                }

                if (!str_cmp(word, "Condition")) {
                    ch->pcdata->condition[0] = fread_number(fp);
                    ch->pcdata->condition[1] = fread_number(fp);
                    ch->pcdata->condition[2] = fread_number(fp);
                    fMatch = TRUE;
                    break;
                }

                break;

            case 'D':
                KEY("Damroll", ch->damroll, fread_number(fp));
                KEY("Deaf", ch->deaf, fread_number(fp));
                KEY("Deaf2", ch->deaf2, fread_number(fp));
                KEY("Deserttime", ch->pcdata->desert_time, (time_t) fread_number(fp));
                SKEY("Description", ch->description, fread_string(fp));

                if (!str_cmp(word, "DimCol")) {
                    char               *temp;

                    temp = fread_string(fp);
                    ch->pcdata->dimcol = temp[0];
                    free_string(temp);
                    fMatch = TRUE;
                    break;
                }

                break;

            case 'E':
                if (!str_cmp(word, "End")) {
                    if (ch->login_sex < 0)
                        ch->login_sex = ch->sex;

                    if (!ch->pcdata->pagelen)
                        ch->pcdata->pagelen = 20;

                    if (!ch->pcdata->prompt || ch->pcdata->prompt[0] == '\0')
                        ch->pcdata->prompt = str_dup("%n<%hh %mm %vv> ");

                    if (IS_SET(ch->pcdata->pflags, PFLAG_SPECIALNAME)) {
                        free_string(ch->name);
                        ch->name = specname;
                    }
                    else
                        free_string(specname);

                    if (ch->long_descr_orig)
                        free_string(ch->long_descr_orig);
                    ch->long_descr_orig = str_dup(ch->long_descr);

                    if (ch->short_descr == NULL || ch->short_descr[0] == '\0') {
                        if (ch->short_descr)
                            free_string(ch->short_descr);
                        ch->short_descr = str_dup(ch->pcdata->origname);
                    }
                    else if (str_cmp(ch->pcdata->origname, ch->short_descr) == 0) {
                        if (ch->short_descr)
                            free_string(ch->short_descr);
                        ch->short_descr = str_dup(ch->pcdata->origname);
                    }

                    tail_chain();
                    return;
                    break;
                }

                KEY("Exp", ch->exp, fread_number(fp));

                if (!str_cmp(word, "Energy")) {
                    ch->energy = fread_number(fp);
                    ch->max_energy = fread_number(fp);
                    fMatch = TRUE;
                    break;
                }

                KEY("EnergyLevel", ch->pcdata->energy_level, fread_number(fp));
                KEY("EnergyUsed",  ch->pcdata->energy_used, fread_number(fp));
                break;

            case 'F':
                KEY("Failures", ch->pcdata->failures, fread_number(fp));
                break;

            case 'G':
                KEY("GainMana", ch->pcdata->mana_from_gain, fread_number(fp));
                KEY("GainHp", ch->pcdata->hp_from_gain, fread_number(fp));
                KEY("GainMove", ch->pcdata->move_from_gain, fread_number(fp));
                KEY("Gold", ch->gold, fread_number(fp));
                KEY("Generation", ch->pcdata->generation, fread_number(fp));
                break;

            case 'H':
                KEY("Hitroll", ch->hitroll, fread_number(fp));
                SKEY("Host", ch->pcdata->host, fread_string(fp));
                KEY("Hasexpfix", ignore, fread_number(fp));

                if (!str_cmp(word, "HiCol")) {
                    char               *temp;

                    temp = fread_string(fp);
                    ch->pcdata->hicol = temp[0];
                    free_string(temp);
                    fMatch = TRUE;
                    break;
                }

                if (!str_cmp(word, "HpManaMove")) {
                    ch->hit = fread_number(fp);
                    ch->max_hit = fread_number(fp);
                    ch->mana = fread_number(fp);
                    ch->max_mana = fread_number(fp);
                    ch->move = fread_number(fp);
                    ch->max_move = fread_number(fp);
                    fMatch = TRUE;
                    break;
                }

                break;

            case 'I':
                SKEY("Immskll", ch->pcdata->immskll, fread_string(fp));
                KEY("IMC", ignore, fread_number(fp));
                KEY("IMCAllow", ignore, fread_number(fp));
                KEY("IMCDeny", ignore, fread_number(fp));
                SKEY("ICEListen", ignorec, fread_string(fp));
                KEY("Incog", ignore, fread_number(fp));
                KEY("Invis", ch->invis, fread_number(fp));
                KEY("Ignore", ignore, fread_number(fp));

                if (!str_cmp(word, "Index")) {
                    int                 i;

                    for (i = 0; i < MAX_CLASS; i++)
                        ch->pcdata->index[i] = fread_number(fp);

                    fMatch = TRUE;
                    break;
                }

                break;

            case 'K':
                KEY("keepVNUM", ch->pcdata->keep_vnum, fread_number(fp));
                KEY("KdonVnum", ch->pcdata->kdon_vnum, fread_number(fp));
                break;

            case 'L':
                KEY("Level", ch->level, fread_number(fp));
                SKEY("LongDescr", ch->long_descr, fread_string(fp));
                KEY("LoginSex", ch->login_sex, fread_number(fp));

                if (!str_cmp(word, "LastLogin")) {
                    char bluff[MSL];
                    temp_fread_string(fp, bluff);
                }

                KEY("LastLogint", ch->pcdata->lastlogint, (time_t) fread_number(fp));
                break;

            case 'M':
                KEY("Mkills", ch->pcdata->mkills, fread_number(fp));
                KEY("Mkilled", ch->pcdata->mkilled, fread_number(fp));
                KEY("Monitor", ch->pcdata->monitor, fread_number(fp));

                if (!str_cmp(word, "m/c")) {
                    for (cnt = 0; cnt < MAX_CLASS; cnt++)
                        ch->lvl[cnt] = fread_number(fp);

                    fMatch = TRUE;
                    break;
                }

                break;

            case 'N':
                SKEY("Name", specname, fread_string(fp));
                KEY("NewsLastRead", ch->pcdata->news_last_read, (time_t)fread_number(fp));
                KEY("Note", ignore, fread_number(fp));
                SKEY("Noteprompt", ch->pcdata->noteprompt, fread_string(fp));
                break;

            case 'O':
                if (!str_cmp(word, "Order")) {
                    int                 i;

                    for (i = 0; i < MAX_CLASS; i++)
                        ch->pcdata->order[i] = fread_number(fp);

                    fMatch = TRUE;
                    break;
                }

                break;

            case 'P':
                KEY("Pagelen", ch->pcdata->pagelen, fread_number(fp));
                SKEY("Password", ch->pcdata->pwd, fread_string(fp));
                KEY("Pkills", ch->pcdata->pkills, fread_number(fp));
                KEY("Pkilled", ch->pcdata->pkilled, fread_number(fp));
                KEY("Pflags", ch->pcdata->pflags, fread_number(fp));
                KEY("Played", ch->played, fread_number(fp));
                KEY("Position", ch->position, fread_number(fp));
                KEY("Practice", ch->practice, fread_number(fp));
                SKEY("Prompt", ch->pcdata->prompt, fread_string(fp));
                break;

            case 'Q':
                KEY("Questpoints", ch->quest_points, fread_number(fp));
                break;

            case 'R':
                KEY("Race", ch->race, fread_number(fp));
                KEY("Revision", current_revision, fread_number(fp));
                SKEY("Roomenter", ch->pcdata->room_enter, fread_string(fp));
                SKEY("Roomexit", ch->pcdata->room_exit, fread_string(fp));
                KEY("RulerRank", ignore, fread_number(fp));

                if (!str_cmp(word, "Remort")) {
                    for (cnt = 0; cnt < MAX_CLASS; cnt++)
                        ch->lvl2[cnt] = fread_number(fp);

                    fMatch = TRUE;
                    break;
                }

                if (!str_cmp(word, "Room")) {
                    ch->in_room = get_room_index(fread_number(fp));

                    if (ch->in_room == NULL)
                        ch->in_room = get_room_index(ROOM_VNUM_LIMBO);

                    fMatch = TRUE;
                    break;
                }

                KEY("RecallVnum", ch->pcdata->recall_vnum, fread_number(fp));
                break;

            case 'S':
                KEY("SavingThrow", ch->saving_throw, fread_number(fp));
                KEY("Sentence", ch->sentence, fread_number(fp));
                KEY("Sex", ch->sex, fread_number(fp));
                SKEY("ShortDescr", ch->short_descr, fread_string(fp));

                if (!str_cmp(word, "Skill")) {
                    int                 sn;
                    int                 value;

                    value = fread_number(fp);
                    sn = skill_lookup(fread_word(fp));

                    if (sn < 0)
                        ;
                    else
                        ch->pcdata->learned[sn] = value;

                    fMatch = TRUE;
                }

                break;

            case 'T':
                KEY("Trust", ch->trust, fread_number(fp));

                if (!str_cmp(word, "Title")) {
                    if (ch->pcdata->title != NULL)
                        free_string(ch->pcdata->title);

                    ch->pcdata->title = fread_string(fp);

                    if (isalpha(ch->pcdata->title[0]) || isdigit(ch->pcdata->title[0])) {
                        sprintf(buf, " %s", ch->pcdata->title);
                        free_string(ch->pcdata->title);
                        ch->pcdata->title = str_dup(buf);
                    }

                    fMatch = TRUE;
                    break;
                }

                break;

            case 'U':
                KEY("Unpkills", ch->pcdata->unpkills, fread_number(fp));
                break;

            case 'V':
                KEY("Vamplevel", ignore, fread_number(fp));
                KEY("Vampexp", ignore, fread_number(fp));
                KEY("Vampbloodline", ignore, fread_number(fp));
                KEY("Vampskillnum", ignore, fread_number(fp));
                KEY("Vampskillmax", ignore, fread_number(fp));
                KEY("Vamppracs", ignore, fread_number(fp));
                break;

            case 'W':
                KEY("Wimpy", ch->wimpy, fread_number(fp));
                KEY("Wizbit", ch->wizbit, fread_number(fp));

                if (!str_cmp(word, "Whoname")) {
                    if (ch->pcdata->who_name != NULL)
                        free_string(ch->pcdata->who_name);

                    ch->pcdata->who_name = fread_string(fp);
                    sprintf(buf, "%s", ch->pcdata->who_name + 1);
                    free_string(ch->pcdata->who_name);
                    ch->pcdata->who_name = str_dup(buf);
                    fMatch = TRUE;
                    break;
                }

                break;
        }

        if (!fMatch) {
            monitor_chan("fread_char(): no match.", MONITOR_BAD);
            bug("fread_char(): no match.");
            tail_chain();
            fread_to_eol(fp);
            tail_chain();
        }
    }

    tail_chain();
}

#define TEMP_VNUM 3090

extern int          top_obj_index;
extern int          top_obj;

void
fread_obj(CHAR_DATA *ch, FILE * fp)
{
    static OBJ_DATA     obj_zero;
    extern unsigned int objid;
    OBJ_DATA           *obj;
    char               *word;
    int                 iNest;
    bool                fMatch;
    bool                fNest;
    bool                fVnum;
    int                 Temp_Obj = 0, OldVnum = 0;

    GET_FREE(obj, obj_free);
    *obj = obj_zero;
    obj->name = str_dup("");
    obj->short_descr = str_dup("");
    obj->description = str_dup("");
    obj->id = 0;

    fNest = FALSE;
    fVnum = TRUE;
    iNest = 0;

    for (;;) {
        word = feof(fp) ? "End" : fread_word(fp);
        fMatch = FALSE;

        switch (UPPER(word[0])) {
            case '*':
                fMatch = TRUE;
                fread_to_eol(fp);
                break;

            case 'A':
                if (!str_cmp(word, "Affect")) {
                    AFFECT_DATA        *paf;

                    GET_FREE(paf, affect_free);
                    paf->type = fread_number(fp);
                    paf->duration = fread_number(fp);
                    paf->modifier = fread_number(fp);
                    paf->location = fread_number(fp);
                    paf->bitvector = fread_number(fp);

                    /* ugh, sn's vary if you change the skill_table. that's why
                     * they have slots, but infuse/poison weapon were using sn's NOT
                     * slots for ->type, so now we check for old sn types on objs and
                     * switch them to the new system. this can be safely removed when
                     * all pfiles have been switched over. */
                    if (paf->type == 202 || paf->type == 203) {
                        int sn = skill_lookup("infuse");

                        if (sn > 0)
                             paf->type = skill_table[sn].slot;
                    }

                    if (paf->type == 181 || paf->type == 182) {
                        int sn = skill_lookup("poison weapon");

                        if (sn > 0)
                             paf->type = skill_table[sn].slot;
                    }

                    LINK(paf, obj->first_apply, obj->last_apply, next, prev);
                    fMatch = TRUE;
                    break;
                }
                break;

            case 'C':
                KEY("Cost", obj->cost, fread_number(fp));
                KEY("ClassFlags", obj->item_apply, fread_number(fp));

                break;

            case 'D':
                SKEY("Description", obj->description, fread_string(fp));
                break;

            case 'E':
                KEY("ExtraFlags", obj->extra_flags, fread_number(fp));

                if (!str_cmp(word, "ExtraDescr")) {
                    EXTRA_DESCR_DATA   *ed;

                    GET_FREE(ed, exdesc_free);
                    ed->keyword = fread_string(fp);
                    ed->description = fread_string(fp);
                    LINK(ed, obj->first_exdesc, obj->last_exdesc, next, prev);
                    fMatch = TRUE;
                }

                if (!str_cmp(word, "End")) {
                    if (!fNest || !fVnum) {
                        AFFECT_DATA        *paf;
                        EXTRA_DESCR_DATA   *ed;

                        monitor_chan("Fread_obj: incomplete object.", MONITOR_BAD);
                        free_string(obj->name);
                        free_string(obj->description);
                        free_string(obj->short_descr);
                        while ((paf = obj->first_apply) != NULL) {
                            obj->first_apply = paf->next;
                            PUT_FREE(paf, affect_free);
                        }
                        while ((ed = obj->first_exdesc) != NULL) {
                            obj->first_exdesc = ed->next;
                            free_string(ed->keyword);
                            free_string(ed->description);
                            PUT_FREE(ed, exdesc_free);
                        }
                        PUT_FREE(obj, obj_free);
                        tail_chain();
                        return;
                    }
                    else {
                        if (obj->id == 0 && !IS_NPC(ch)) {
                            switch (obj->item_type) {
                                case ITEM_LIGHT:
                                case ITEM_WEAPON:
                                case ITEM_ARMOR:
                                case ITEM_POTION:
                                case ITEM_CLUTCH:
                                case ITEM_CONTAINER:
                                case ITEM_PIECE:
                                case ITEM_SPELL_MATRIX:
                                    obj->id = objid;
                                    objid++;
                                    save_mudsets();
                                    break;
                                default:
                                    break;
                            }
                        }

                        LINK(obj, first_obj, last_obj, next, prev);

                        if (Temp_Obj) {
                            int                 newvnum;
                            OBJ_INDEX_DATA     *pObjIndex;
                            int                 nMatch = 0;
                            int                 vnum;

                            /* One of three things:
                               Obj Vnum was deleted
                               Obj Vnum was moved
                               Obj Vnum was previously deleted */
                            newvnum = TEMP_VNUM;

                            if (newvnum == TEMP_VNUM) {
                                /* Scan through objects, trying to find a matching description */
                                for (vnum = 0; nMatch < top_obj_index; vnum++) {
                                    if ((pObjIndex = get_obj_index(vnum)) != NULL) {
                                        nMatch++;
                                        if (!str_cmp(obj->short_descr, pObjIndex->short_descr)) {
                                            obj->pIndexData = pObjIndex;
                                            break;
                                        }
                                    }
                                }
                            }

                        }

                        if (iNest == 0 || rgObjNest[iNest] == NULL)
                            obj_to_char(obj, ch);
                        else    /*
                                   if ( rgObjNest[iNest-1] == obj )
                                   obj_to_char( obj, ch );
                                   else */
                            obj_to_obj(obj, rgObjNest[iNest - 1]);

                        if (obj->pIndexData) {
                            obj->pIndexData->count++;

                            /* do updates to realm eq here */
                            if (obj->pIndexData->vnum == 13 || obj->pIndexData->vnum == 14) {
                                REMOVE_BIT(obj->extra_flags, ITEM_NOREMOVE);
                                if (!is_name("realmeq", obj->name)) {
                                    char buf[MSL];
                                    sprintf(buf, "%s ^realmeq", obj->name);
                                    free_string(obj->name);
                                    obj->name = str_dup(buf);
                                }
                            }
                        }

                        tail_chain();
                        return;
                    }
                }
                break;

            case 'I':
                KEY("Id",       obj->id,        fread_unumber(fp));
                KEY("ItemApply", obj->item_apply, fread_number(fp));
                KEY("ItemType", obj->item_type, fread_number(fp));
                break;

            case 'L':
                KEY("Level", obj->level, fread_number(fp));
                break;

            case 'N':
                SKEY("Name", obj->name, fread_string(fp));

                if (!str_cmp(word, "Nest")) {
                    iNest = fread_number(fp);
                    if (iNest < 0 || iNest >= MAX_NEST) {
                        monitor_chan("Fread_obj: bad nest.", MONITOR_BAD);
                    }
                    else {
                        rgObjNest[iNest] = obj;
                        fNest = TRUE;
                    }
                    fMatch = TRUE;
                }
                break;

            case 'O':
                /*     KEY( "Objfun", obj->obj_fun,  obj_fun_lookup( fread_string( fp ) ) );  */
                if (!str_cmp(word, "Objfun")) {
                    char               *dumpme;

                    dumpme = fread_string(fp);
                    obj->obj_fun = obj_fun_lookup(dumpme);
                    free_string(dumpme);
                    fMatch = TRUE;
                }
                break;

            case 'S':
                SKEY("ShortDescr", obj->short_descr, fread_string(fp));

                if (!str_cmp(word, "Spell")) {
                    int                 iValue;
                    int                 sn;

                    iValue = fread_number(fp);
                    sn = skill_lookup(fread_word(fp));
                    if (iValue < 0 || iValue > 3) {
                        monitor_chan("Fread_obj: bad iValue ", MONITOR_BAD);
                    }
                    else if (sn < 0) {
                        monitor_chan("Fread_obj: unknown skill.", MONITOR_BAD);
                    }
                    else {
                        obj->value[iValue] = sn;
                    }
                    fMatch = TRUE;
                    break;
                }

                break;

            case 'T':
                KEY("Timer", obj->timer, fread_number(fp));
                break;

            case 'V':
                if (!str_cmp(word, "Values")) {
                    obj->value[0] = fread_number(fp);
                    obj->value[1] = fread_number(fp);
                    obj->value[2] = fread_number(fp);
                    obj->value[3] = fread_number(fp);
                    fMatch = TRUE;
                    break;
                }

                if (!str_cmp(word, "Vnum")) {
                    int                 vnum;

                    vnum = fread_number(fp);

                    if ((obj->pIndexData = get_obj_index(vnum)) == NULL || vnum == TEMP_VNUM) {
                        /* Set flag saying that object is temporary */
                        Temp_Obj = 1;
                        OldVnum = vnum;
                        vnum = TEMP_VNUM;
                        obj->pIndexData = get_obj_index(vnum);
                    }
                    else
                        fVnum = TRUE;
                    fMatch = TRUE;
                    break;
                }
                break;

            case 'W':
                KEY("WearFlags", obj->wear_flags, fread_number(fp));
                KEY("WearLoc", obj->wear_loc, fread_number(fp));
                KEY("Weight", obj->weight, fread_number(fp));
                break;

        }

        if (!fMatch) {
            monitor_chan("Fread_obj: no match.", MONITOR_BAD);
            fread_to_eol(fp);
        }
    }
}
void
fread_corpse(FILE * fp)
{
    static OBJ_DATA     obj_zero;
    OBJ_DATA           *obj;
    char               *word;
    int                 iNest;
    bool                fMatch;
    bool                fNest;
    bool                fVnum;
    int                 Temp_Obj = 0, OldVnum = 0;
    int                 this_room_vnum;

    GET_FREE(obj, obj_free);
    *obj = obj_zero;
    obj->name = str_dup("");
    obj->short_descr = str_dup("");
    obj->description = str_dup("");

    fNest = FALSE;
    fVnum = TRUE;
    iNest = 0;
    this_room_vnum = 0;

    for (;;) {
        word = feof(fp) ? "End" : fread_word(fp);
        fMatch = FALSE;

        switch (UPPER(word[0])) {
            case '*':
                fMatch = TRUE;
                fread_to_eol(fp);
                break;

            case 'A':
                if (!str_cmp(word, "Affect")) {
                    AFFECT_DATA        *paf;

                    GET_FREE(paf, affect_free);
                    paf->type = fread_number(fp);
                    paf->duration = fread_number(fp);
                    paf->modifier = fread_number(fp);
                    paf->location = fread_number(fp);
                    paf->bitvector = fread_number(fp);
                    LINK(paf, obj->first_apply, obj->last_apply, next, prev);
                    fMatch = TRUE;
                    break;
                }
                break;

            case 'C':
                KEY("Cost", obj->cost, fread_number(fp));
                KEY("ClassFlags", obj->item_apply, fread_number(fp));

                break;

            case 'D':
                SKEY("Description", obj->description, fread_string(fp));
                break;

            case 'E':
                KEY("ExtraFlags", obj->extra_flags, fread_number(fp));

                if (!str_cmp(word, "ExtraDescr")) {
                    EXTRA_DESCR_DATA   *ed;

                    GET_FREE(ed, exdesc_free);
                    ed->keyword = fread_string(fp);
                    ed->description = fread_string(fp);
                    LINK(ed, obj->first_exdesc, obj->last_exdesc, next, prev);
                    fMatch = TRUE;
                }

                if (!str_cmp(word, "End")) {
                    if (!fNest || !fVnum) {
                        AFFECT_DATA        *paf;
                        EXTRA_DESCR_DATA   *ed;

                        monitor_chan("Fread_obj: incomplete object.", MONITOR_BAD);
                        free_string(obj->name);
                        free_string(obj->description);
                        free_string(obj->short_descr);
                        while ((paf = obj->first_apply) != NULL) {
                            obj->first_apply = paf->next;
                            PUT_FREE(paf, affect_free);
                        }
                        while ((ed = obj->first_exdesc) != NULL) {
                            obj->first_exdesc = ed->next;
                            free_string(ed->keyword);
                            free_string(ed->description);
                            PUT_FREE(ed, exdesc_free);
                        }
                        PUT_FREE(obj, obj_free);
                        tail_chain();
                        return;
                    }
                    else {
                        LINK(obj, first_obj, last_obj, next, prev);

                        if (Temp_Obj) {
                            int                 newvnum;
                            OBJ_INDEX_DATA     *pObjIndex;
                            int                 nMatch = 0;
                            int                 vnum;

                            /* One of three things:
                               Obj Vnum was deleted
                               Obj Vnum was moved
                               Obj Vnum was previously deleted */
                            newvnum = TEMP_VNUM;

                            if (newvnum == TEMP_VNUM) {
                                /* Scan through objects, trying to find a matching description */
                                for (vnum = 0; nMatch < top_obj_index; vnum++) {
                                    if ((pObjIndex = get_obj_index(vnum)) != NULL) {
                                        nMatch++;
                                        if (!str_cmp(obj->short_descr, pObjIndex->short_descr)) {
                                            obj->pIndexData = pObjIndex;
                                            break;
                                        }
                                    }
                                }
                            }

                        }

                        if (iNest == 0 || rgObjNest[iNest] == NULL)
                            obj_to_room(obj, get_room_index(this_room_vnum));
                        else
                            obj_to_obj(obj, rgObjNest[iNest - 1]);

                        if (obj->pIndexData)
                            obj->pIndexData->count++;

                        tail_chain();
                        return;
                    }
                }
                break;

            case 'I':
                KEY("Id",       obj->id,        fread_unumber(fp));
                KEY("ItemApply", obj->item_apply, fread_number(fp));
                KEY("ItemType", obj->item_type, fread_number(fp));
                break;

            case 'L':
                KEY("Level", obj->level, fread_number(fp));
                break;

            case 'N':
                SKEY("Name", obj->name, fread_string(fp));

                if (!str_cmp(word, "Nest")) {
                    iNest = fread_number(fp);
                    if (iNest < 0 || iNest >= MAX_NEST) {
                        monitor_chan("Fread_obj: bad nest.", MONITOR_BAD);
                    }
                    else {
                        rgObjNest[iNest] = obj;
                        fNest = TRUE;
                    }
                    fMatch = TRUE;
                }
                break;

            case 'O':
                /*     KEY( "Objfun", obj->obj_fun,  obj_fun_lookup( fread_string( fp ) ) );  */
                if (!str_cmp(word, "Objfun")) {
                    char               *dumpme;

                    dumpme = fread_string(fp);
                    obj->obj_fun = obj_fun_lookup(dumpme);
                    free_string(dumpme);
                    fMatch = TRUE;
                }
                break;

            case 'R':
                KEY("Revision",    current_revision, fread_number(fp));
                break;

            case 'S':
                SKEY("ShortDescr", obj->short_descr, fread_string(fp));

                if (!str_cmp(word, "Spell")) {
                    int                 iValue;
                    int                 sn;

                    iValue = fread_number(fp);
                    sn = skill_lookup(fread_word(fp));
                    if (iValue < 0 || iValue > 3) {
                        monitor_chan("Fread_obj: bad iValue ", MONITOR_BAD);
                    }
                    else if (sn < 0) {
                        monitor_chan("Fread_obj: unknown skill.", MONITOR_BAD);
                    }
                    else {
                        obj->value[iValue] = sn;
                    }
                    fMatch = TRUE;
                    break;
                }

                break;

            case 'T':
                KEY("Timer", obj->timer, fread_number(fp));
                break;

            case 'V':
                if (!str_cmp(word, "Values")) {
                    obj->value[0] = fread_number(fp);
                    obj->value[1] = fread_number(fp);
                    obj->value[2] = fread_number(fp);
                    obj->value[3] = fread_number(fp);
                    fMatch = TRUE;
                    break;
                }

                if (!str_cmp(word, "Vnum")) {
                    int                 vnum;

                    vnum = fread_number(fp);

                    if ((obj->pIndexData = get_obj_index(vnum)) == NULL || vnum == TEMP_VNUM) {
                        /* Set flag saying that object is temporary */
                        Temp_Obj = 1;
                        OldVnum = vnum;
                        vnum = TEMP_VNUM;
                        obj->pIndexData = get_obj_index(vnum);
                    }
                    else
                        fVnum = TRUE;
                    fMatch = TRUE;
                    break;
                }
                break;

            case 'W':
                KEY("WearFlags", obj->wear_flags, fread_number(fp));
                KEY("WearLoc", obj->wear_loc, fread_number(fp));
                KEY("Weight", obj->weight, fread_number(fp));
                KEY("WhereVnum", this_room_vnum, fread_number(fp));
                break;

        }

        if (!fMatch) {
            monitor_chan("Fread_obj: no match.", MONITOR_BAD);
            fread_to_eol(fp);
        }
    }
}

void
fwrite_corpse(OBJ_DATA *obj, FILE * fp, int iNest)
{
    EXTRA_DESCR_DATA   *ed;
    AFFECT_DATA        *paf;
    OBJ_DATA           *mainobj;
    int                 where_vnum = 3300;

    /*
     * Slick recursion to write lists backwards,
     *   so loading them will load in forwards order.
     */
    if (obj->next_in_carry_list != NULL)
        fwrite_corpse(obj->next_in_carry_list, fp, iNest);

    mainobj = obj;
    while (mainobj->in_obj)
        mainobj = mainobj->in_obj;

    if (mainobj != NULL && mainobj->in_room != NULL)
        where_vnum = mainobj->in_room->vnum;
    else if (obj->in_room != NULL)
        where_vnum = obj->in_room->vnum;
    else
        where_vnum = 1;

    if (obj->in_room == NULL && obj->in_obj == NULL)
        obj->in_room = get_room_index(ROOM_VNUM_LIMBO);

    FPRINTF(fp, "#OBJECT\n");
    FPRINTF(fp, "Revision     %d\n", SAVE_REVISION);
    FPRINTF(fp, "WhereVnum    %d\n", where_vnum);

    FPRINTF(fp, "Nest         %d\n", iNest);

    if (obj->id > 0) FPRINTF(fp, "Id           %u\n", obj->id);

    FPRINTF(fp, "Name         %s~\n", obj->name);
    FPRINTF(fp, "ShortDescr   %s~\n", obj->short_descr);
    FPRINTF(fp, "Description  %s~\n", obj->description);
    FPRINTF(fp, "Vnum         %d\n", obj->pIndexData->vnum);
    FPRINTF(fp, "ExtraFlags   %d\n", obj->extra_flags);
    FPRINTF(fp, "WearFlags    %d\n", obj->wear_flags);
    FPRINTF(fp, "WearLoc      %d\n", obj->wear_loc);
    if (obj->obj_fun != NULL)
        FPRINTF(fp, "Objfun       %s~\n", rev_obj_fun_lookup((void *) obj->obj_fun));

    FPRINTF(fp, "ItemApply    %d\n", obj->item_apply);
    FPRINTF(fp, "ItemType     %d\n", obj->item_type);
    FPRINTF(fp, "Weight       %d\n", obj->weight);
    FPRINTF(fp, "Level        %d\n", obj->level);
    FPRINTF(fp, "Timer        %d\n", obj->timer);
    FPRINTF(fp, "Cost         %d\n", obj->cost);
    FPRINTF(fp, "Values       %d %d %d %d\n", obj->value[0], obj->value[1], obj->value[2], obj->value[3]);

    switch (obj->item_type) {
        case ITEM_POTION:
        case ITEM_SCROLL:
            if (obj->value[1] > 0) {
                FPRINTF(fp, "Spell 1      '%s'\n", skill_table[obj->value[1]].name);
            }

            if (obj->value[2] > 0) {
                FPRINTF(fp, "Spell 2      '%s'\n", skill_table[obj->value[2]].name);
            }

            if (obj->value[3] > 0) {
                FPRINTF(fp, "Spell 3      '%s'\n", skill_table[obj->value[3]].name);
            }

            break;

        case ITEM_PILL:
        case ITEM_STAFF:
        case ITEM_WAND:
            if (obj->value[3] > 0) {
                FPRINTF(fp, "Spell 3      '%s'\n", skill_table[obj->value[3]].name);
            }

            break;
    }

    for (paf = obj->first_apply; paf != NULL; paf = paf->next) {
        FPRINTF(fp, "Affect       %d %d %d %d %d\n", paf->type, paf->duration, paf->modifier, paf->location, paf->bitvector);
    }

    for (ed = obj->first_exdesc; ed != NULL; ed = ed->next) {
        FPRINTF(fp, "ExtraDescr   %s~ %s~\n", ed->keyword, ed->description);
    }

    FPRINTF(fp, "End\n\n");

    if (obj->first_in_carry_list != NULL)
        fwrite_corpse(obj->first_in_carry_list, fp, iNest + 1);

    return;
}

void
fread_locker_obj(FILE *fp)
{
    fread_corpse(fp);
    return;
}

void
fwrite_locker_obj(LOCKER_DATA *locker, OBJ_DATA *obj, FILE *fp, int iNest)
{
    EXTRA_DESCR_DATA   *ed;
    AFFECT_DATA        *paf;
    OBJ_DATA           *mainobj;
    int                 where_vnum = 3300;
    bool                goahead = TRUE;

    mainobj = obj;
    while (mainobj->in_obj)
        mainobj = mainobj->in_obj;

    if (mainobj != NULL && mainobj->in_room != NULL)
        where_vnum = mainobj->in_room->vnum;
    else if (obj->in_room != NULL)
        where_vnum = obj->in_room->vnum;
    else
        where_vnum = 1;

    if (obj->in_room == NULL && obj->in_obj == NULL)
        obj->in_room = get_room_index(ROOM_VNUM_LIMBO);

    if (locker->valid && !locker->types[obj->item_type])
        goahead = FALSE;

    if (!locker->valid && locker->types[obj->item_type])
        goahead = FALSE;

    if (!IS_SET(locker->flags, LOCKER_SAVEALL)) {
        if (obj->item_type == ITEM_KEY)
            goahead = FALSE;

        if (IS_SET(obj->extra_flags, ITEM_NOSAVE))
            goahead = FALSE;
    }

    if (obj == quest_object)
        goahead = FALSE;

    if (goahead) {
        FPRINTF(fp, "#OBJECT\n");
        FPRINTF(fp, "Revision     %d\n", SAVE_REVISION);
        FPRINTF(fp, "WhereVnum    %d\n", where_vnum);

        FPRINTF(fp, "Nest         %d\n", iNest);

        if (obj->id > 0) FPRINTF(fp, "Id           %u\n", obj->id);

        FPRINTF(fp, "Name         %s~\n", obj->name);
        FPRINTF(fp, "ShortDescr   %s~\n", obj->short_descr);
        FPRINTF(fp, "Description  %s~\n", obj->description);
        FPRINTF(fp, "Vnum         %d\n", obj->pIndexData->vnum);
        FPRINTF(fp, "ExtraFlags   %d\n", obj->extra_flags);
        FPRINTF(fp, "WearFlags    %d\n", obj->wear_flags);
        FPRINTF(fp, "WearLoc      %d\n", obj->wear_loc);
        if (obj->obj_fun != NULL)
            FPRINTF(fp, "Objfun       %s~\n", rev_obj_fun_lookup((void *) obj->obj_fun));

        FPRINTF(fp, "ItemApply    %d\n", obj->item_apply);
        FPRINTF(fp, "ItemType     %d\n", obj->item_type);
        FPRINTF(fp, "Weight       %d\n", obj->weight);
        FPRINTF(fp, "Level        %d\n", obj->level);
        FPRINTF(fp, "Timer        %d\n", obj->timer);
        FPRINTF(fp, "Cost         %d\n", obj->cost);
        FPRINTF(fp, "Values       %d %d %d %d\n", obj->value[0], obj->value[1], obj->value[2], obj->value[3]);

        switch (obj->item_type) {
            case ITEM_POTION:
            case ITEM_SCROLL:
                if (obj->value[1] > 0) {
                    FPRINTF(fp, "Spell 1      '%s'\n", skill_table[obj->value[1]].name);
                }

                if (obj->value[2] > 0) {
                    FPRINTF(fp, "Spell 2      '%s'\n", skill_table[obj->value[2]].name);
                }

                if (obj->value[3] > 0) {
                    FPRINTF(fp, "Spell 3      '%s'\n", skill_table[obj->value[3]].name);
                }

                break;

            case ITEM_PILL:
            case ITEM_STAFF:
            case ITEM_WAND:
                if (obj->value[3] > 0) {
                    FPRINTF(fp, "Spell 3      '%s'\n", skill_table[obj->value[3]].name);
                }

                break;
        }

        for (paf = obj->first_apply; paf != NULL; paf = paf->next) {
            FPRINTF(fp, "Affect       %d %d %d %d %d\n", paf->type, paf->duration, paf->modifier, paf->location, paf->bitvector);
        }

        for (ed = obj->first_exdesc; ed != NULL; ed = ed->next) {
            FPRINTF(fp, "ExtraDescr   %s~ %s~\n", ed->keyword, ed->description);
        }

        FPRINTF(fp, "End\n\n");

        locker_amount++;
        locker_weight += obj->weight;

        if (   (locker->maxitem > 0 && locker_amount >= locker->maxitem)
            || (locker->maxweight > 0 && locker_weight >= locker->maxweight))
            return;
    }

    if (obj->next_in_carry_list != NULL)
        fwrite_locker_obj(locker, obj->next_in_carry_list, fp, iNest);

    if (goahead && obj->first_in_carry_list != NULL)
        fwrite_locker_obj(locker, obj->first_in_carry_list, fp, iNest + 1);

    return;
}

void load_locker(ROOM_INDEX_DATA *room)
{
    char letter;
    char *word;
    FILE *fp;
    char file[MSL];

    if (!room || !room->locker || room->vnum <= 0)
        return;

    sprintf(file, "%slocker.%d", LOCKER_DIR, room->vnum);

    if ((fp = fopen(file, "r")) == NULL)
        return;

    for (;;) {
        letter = fread_letter(fp);

        if (letter == '*') {
            fread_to_eol(fp);
            continue;
        }

        if (letter != '#') {
            monitor_chan("load_locker: # not found.", MONITOR_BAD);
            break;
        }

        word = fread_word(fp);
        if (!str_cmp(word, "OBJECT")) {
            fread_locker_obj(fp);
        }
        else if (!str_cmp(word, "END")) {
            break;
        }
        else {
            monitor_chan("load_locker: bad section.", MONITOR_BAD);
            break;
        }
    }

    fclose(fp);
}

void save_locker(ROOM_INDEX_DATA *room)
{
    FILE *fp;
    OBJ_DATA *obj, *obj_next;
    char file1[MSL], file2[MSL];
    bool bad = FALSE;

    if (nosave)
        return;

    if (!room || !room->locker || room->vnum <= 0 || fBootDb)
        return;

    sprintf(file1, "%slocker.%d.new", LOCKER_DIR, room->vnum);
    sprintf(file2, "%slocker.%d", LOCKER_DIR, room->vnum);

    if ((fp = fopen(file1, "w")) == NULL)
        return;

    locker_amount = 0;
    locker_weight = 0;

    for (obj = room->first_content; obj != NULL; obj = obj_next) {
        obj_next = obj->next_in_room;

        bad = FALSE;
        switch (obj->item_type) {
            case ITEM_BEACON:
            case ITEM_PORTAL:
            case ITEM_CORPSE_NPC:
            case ITEM_CORPSE_PC:
            case ITEM_FOUNTAIN:
            case ITEM_BOARD:
                bad = TRUE;
                break;
            default:
                break;
        }

        if (bad)
            continue;

        if (room->locker->valid && !room->locker->types[obj->item_type])
            continue;

        if (!room->locker->valid && room->locker->types[obj->item_type])
            continue;

        if (!IS_SET(room->locker->flags, LOCKER_SAVEALL)) {
            if (obj->item_type == ITEM_KEY)
                continue;

            if (IS_SET(obj->extra_flags, ITEM_NOSAVE))
                continue;
        }

        if (obj == quest_object)
            continue;

        fwrite_locker_obj(room->locker, obj, fp, 0);

        if (   (room->locker->maxitem > 0 && locker_amount >= room->locker->maxitem)
            || (room->locker->maxweight > 0 && locker_weight >= room->locker->maxweight))
            break;

    }

    FPRINTF(fp, "#END\n\n");
    fflush(fp);
    fclose(fp);
    rename(file1, file2);
    return;
}

void
save_corpses()
{

    FILE               *fp;
    char                corpse_file_name[MAX_STRING_LENGTH];
    CORPSE_DATA        *this_corpse;

    if (nosave)
        return;

    fclose(fpReserve);
    sprintf(corpse_file_name, "%s", CORPSE_FILE);

    if ((fp = fopen(corpse_file_name, "w")) == NULL) {
        bug("Save Corpses: fopen");
        perror("failed open of corpse_file in save_corpses");
    }
    else {
        for (this_corpse = first_corpse; this_corpse != NULL; this_corpse = this_corpse->next) {
            fwrite_corpse(this_corpse->this_corpse, fp, 0);
        }
        FPRINTF(fp, "#END\n\n");

        fflush(fp);
        fclose(fp);
    }
    fpReserve = fopen(NULL_FILE, "r");
    return;

}

void
save_bans()
{

    FILE               *fp;
    char                ban_file_name[MAX_STRING_LENGTH];
    BAN_DATA           *pban;

    if (nosave)
        return;

    fclose(fpReserve);
    sprintf(ban_file_name, "%s", BANS_FILE);

    if ((fp = fopen(ban_file_name, "w")) == NULL) {
        bug("Save ban list: fopen");
        perror("failed open of bans.lst in save_ban");
    }
    else {
        for (pban = first_ban; pban != NULL; pban = pban->next) {
            FPRINTF(fp, "#BAN~\n");
            FPRINTF(fp, "%d\n", (pban->newbie ? 1 : 0));
            FPRINTF(fp, "%s~\n", pban->name);
            FPRINTF(fp, "%s~\n", pban->banned_by);
        }
        FPRINTF(fp, "#END~\n\n");
    }

    fflush(fp);
    fclose(fp);

    fpReserve = fopen(NULL_FILE, "r");
    return;

}

/*
 * START: shelp addition
 */

void
save_shelps()
{

    FILE               *fp;
    char                shelp_file_name[MAX_STRING_LENGTH];
    SHELP_DATA         *shelp;

    if (nosave)
        return;

    fclose(fpReserve);
    sprintf(shelp_file_name, "%s", SHELP_FILE);

    if ((fp = fopen(shelp_file_name, "w")) == NULL) {
        bug("Save shelp list: fopen");
        perror("failed open of shelp.lst in save_shelp");
    }
    else {
        for (shelp = first_shelp; shelp != NULL; shelp = shelp->next) {
            FPRINTF(fp, "%s~\n", shelp->name);
            FPRINTF(fp, "%s~\n", shelp->duration);
            FPRINTF(fp, "%s~\n", shelp->modify);
            FPRINTF(fp, "%s~\n", shelp->type);
            FPRINTF(fp, "%s~\n", shelp->target);
            FPRINTF(fp, "%s~\n", shelp->desc);
        }
    }

    fflush(fp);
    fclose(fp);

    fpReserve = fopen(NULL_FILE, "r");
    return;

}

/*
 * FINISH: shelp addition
 */

void
save_notes()
{

    FILE               *fp;
    NOTE_DATA          *note;

    if (nosave)
        return;

    fclose(fpReserve);

    if ((fp = fopen(NOTE_FILE, "w")) == NULL) {
        bug("Save note list: fopen");
        perror("failed open of notes.lst in save_notes");
    }
    else {
        for (note = first_note; note != NULL; note = note->next) {
            FPRINTF(fp, "%s~\n", note->from);
            FPRINTF(fp, "%s~\n", note->to);
            FPRINTF(fp, "%s~\n", note->subject);
            FPRINTF(fp, "%s~\n", note->text);
            FPRINTF(fp, "%d\n", (int) note->date_stamp);
            FPRINTF(fp, "%d\n", (note->unread) ? TRUE : FALSE);
        }
    }

    fflush(fp);
    fclose(fp);

    fpReserve = fopen(NULL_FILE, "r");
    return;
}

void
fread_ignore(CHAR_DATA *ch, FILE * fp)
{
    char               *word;
    IGNORE_DATA        *ignore;

    for (;;) {
        word = feof(fp) ? "End" : fread_word(fp);

        if (!str_cmp(word, "ignore")) {
            GET_FREE(ignore, ignore_free);

            ignore->char_ignored = fread_string(fp);
            LINK(ignore, ch->pcdata->first_ignore, ch->pcdata->last_ignore, next, prev);
            /* do ignore here */
        }
        else if (!str_cmp(word, "End"))
            break;
        else
            break;
    }

    tail_chain();
    return;
}

void
fwrite_ignore(CHAR_DATA *ch, FILE * fp)
{
    IGNORE_DATA        *ignore;

    FPRINTF(fp, "#IGNORE\n");

    for (ignore = ch->pcdata->first_ignore; ignore != NULL; ignore = ignore->next) {
        FPRINTF(fp, "ignore %s~\n", ignore->char_ignored);
    }

    FPRINTF(fp, "End\n\n");

    tail_chain();
    return;
}

void
save_mudsets()
{

    FILE               *fp;
    int                 cnt;

    if (nosave)
        return;

    if ((fp = fopen(MUDSET_FILE, "w")) == NULL) {
        bug("save_mudsets: fopen");
        perror("failed open of mudsets.dat in save_mudsets");
        return;
    }

    for (cnt = 0; mudset_table[cnt].name[0] != '\0'; cnt++) {
        bool           *type_bool;
        unsigned int   *type_uint;
        char          **type_string;
        
        switch (mudset_table[cnt].type) {
            case MUDSET_TYPE_BOOL:
                type_bool = (bool *)mudset_table[cnt].var;
                FPRINTF(fp, "%s %d\n", mudset_table[cnt].name, (*type_bool) ? TRUE : FALSE);
                break;
            case MUDSET_TYPE_INT:
                type_uint = (unsigned int *)mudset_table[cnt].var;
                FPRINTF(fp, "%s %u\n", mudset_table[cnt].name, *type_uint);
                break;
            case MUDSET_TYPE_STRING:
                type_string = (char **)mudset_table[cnt].var;
                FPRINTF(fp, "%s %s~\n", mudset_table[cnt].name, *type_string);
                break;
            default:
                break;
        }
    }

    FPRINTF(fp, "End\n");
    fclose(fp);
    return;
}

void
save_renames(void)
{

    FILE               *fp;
    RENAME_DATA        *rename;

    if (nosave)
        return;

    if ((fp = fopen(RENAME_FILE, "w")) == NULL) {
        bug("save_renames: fopen");
        perror("failed open of rename.dat in save_renames");
        return;
    }

    for (rename = first_rename; rename != NULL; rename = rename->next) {
        FPRINTF(fp, "Rename %s~ %s~ %s~ %s~ %s~ %s~ %s~ %li\n",
            rename->playername,
            rename->oldshort,
            rename->oldlong,
            rename->oldkeyword,
            rename->newshort,
            rename->newlong,
            rename->newkeyword,
            rename->id);
    }

    FPRINTF(fp, "End\n");
    fclose(fp);
    return;
}
