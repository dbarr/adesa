/* Mod'ed to save two copies of area... one in Ack! format, and one
 * in 'Standard' Envy format... -S-
 */

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

IDSTRING(rcsid, "$Id: areasave.c,v 1.17 2004/10/16 05:47:53 dave Exp $");

/* Way this works:
   Mud reads in area files, stores details in data lists.
   Edit rooms, objects, resets.
   type savearea.
   Sets bool saving_area to true.
   Incrementally saves an area, using data lists.
 */

#define SAVEQUEUESIZE 200
#define NOT_SAVING 0
#define START_SAVING 1
#define AM_SAVING 2
#define BUILD_OK -1
#define BUILD_CANTSAVE 1
#define BUILD_TOOMANY  2

#define BUILD_SEC_AREA     1
#define BUILD_SEC_ROOMS    2
#define BUILD_SEC_MOBILES  3
#define BUILD_SEC_MOBPROGS 4
#define BUILD_SEC_OBJECTS  5
#define BUILD_SEC_SHOPS    6
#define BUILD_SEC_RESETS   7
#define BUILD_SEC_END      8

struct save_queue_type
{
    AREA_DATA          *area;
    CHAR_DATA          *ch;
    int                 loops;
} SaveQ[SAVEQUEUESIZE];

/* Semi-local vars. */
int                 saving_area = 0;

/* local */
int                 offset;
int                 ToBeSaved = 0;
int                 CurrentSaving = -1;
AREA_DATA          *CurSaveArea = NULL;
CHAR_DATA          *CurSaveChar = NULL;
int                 CurLoops = 1;
int                 Section;
BUILD_DATA_LIST    *Pointer;
RESET_DATA         *ResetPointer;
FILE               *SaveFile;
FILE               *Envy;
int                 AreasModified = 0;

/* Local functions */
/* void build_save(); proto in merc.h */
void                build_save_area(void);
void                build_save_mobs(void);
void                build_save_mobprogs(void);
void                build_save_objects(void);
void                build_save_rooms(void);
void                build_save_shops(void);
void                build_save_resets(void);
void                build_save_end(void);
char               *mprog_type_to_name(int);
void                vuild_save_flush(void);

/*  int convert(int lev); */
/* Convert levels from ack -> envy! */

/* int  convert(int lev)    */
/*  {       */
/*   return( lev - ( lev/5 ) );     */
/* }   */

void
do_savearea(CHAR_DATA *ch, char *argument)
{
    AREA_DATA          *SaveArea;
    int                 loops;

    /*     char first_arg[MAX_INPUT_LENGTH]; unused? */

    if (ch == NULL) {
        SaveArea = (AREA_DATA *) argument;
        loops = 10;
    }
    else {
        if (ch->in_room == NULL) {
            send_to_char("Do not know what room you are in!!, cannot save.\n", ch);
            return;
        }

        SaveArea = (ch->in_room)->area;
        if (SaveArea == NULL) {
            send_to_char("Do not know what area you are in!!, cannot save.\n", ch);
            return;
        }

        if (*argument != '\0') {
            loops = atoi(argument);
            if (loops < 1)
                loops = 1;
        }
        else
            loops = 1;
    }

    if (ToBeSaved == CurrentSaving) {
        send_to_char("Too many areas in queue, please try later.\n", ch);
        return;
    }

    SaveQ[ToBeSaved].area = SaveArea;
    SaveQ[ToBeSaved].ch = ch;
    SaveQ[ToBeSaved].loops = loops;
    ToBeSaved = (ToBeSaved + 1) % SAVEQUEUESIZE;

    if (saving_area == NOT_SAVING)
        saving_area = START_SAVING;
    else
        send_to_char("Save is queued, please wait. \n", ch);

    build_save();
    return;
}

void
build_save()
{
    int                 a;
    char                filename[255];
    char                buf[MAX_STRING_LENGTH];

    if (nosave)
        return;

    for (a = 0; a < CurLoops && saving_area > 0; a++) {

        if (saving_area == START_SAVING) {
            CurrentSaving = (CurrentSaving + 1) % SAVEQUEUESIZE;
            CurSaveArea = SaveQ[CurrentSaving].area;
            CurSaveChar = SaveQ[CurrentSaving].ch;
            CurLoops = SaveQ[CurrentSaving].loops;
            send_to_char("Starting Save.\n", CurSaveChar);

            sprintf(filename, "%s.new", CurSaveArea->filename);
            SaveFile = fopen(filename, "w");
            if (SaveFile == NULL) {
                if (CurrentSaving == ToBeSaved)
                    saving_area = NOT_SAVING;
                send_to_char("Can not open file for saving.\n", CurSaveChar);
                return;
            }
            /* Open second file for saving in envy format */

            sprintf(buf, "Starting to save %s", CurSaveArea->filename);
            monitor_chan(buf, MONITOR_AREA_SAVING);

            Section = 1;
            offset = CurSaveArea->offset;
            saving_area = AM_SAVING;
            Pointer = NULL;
            ResetPointer = NULL;
        }

        switch (Section) {
            case BUILD_SEC_AREA:
                build_save_area();
                break;
            case BUILD_SEC_ROOMS:
                build_save_rooms();
                break;
            case BUILD_SEC_MOBILES:
                build_save_mobs();
                break;
            case BUILD_SEC_MOBPROGS:
                build_save_mobprogs();
                break;
            case BUILD_SEC_OBJECTS:
                build_save_objects();
                break;
            case BUILD_SEC_SHOPS:
                build_save_shops();
                break;
            case BUILD_SEC_RESETS:
                build_save_resets();
                break;
            case BUILD_SEC_END:
                build_save_end();
                break;
        }
    }
    return;
}

void
build_save_area()
{
    FPRINTF(SaveFile, "#AREA\n");
    FPRINTF(SaveFile, "%s~\n", CurSaveArea->name);
    FPRINTF(SaveFile, "Z %i\n", AREA_REVISION);
    FPRINTF(SaveFile, "C %i\n", CurSaveArea->avnum);
    FPRINTF(SaveFile, "K %s~\n", CurSaveArea->keyword);
    FPRINTF(SaveFile, "L %s~\n", CurSaveArea->level_label);
    FPRINTF(SaveFile, "N %i\n", CurSaveArea->area_num);
    FPRINTF(SaveFile, "I %i %i\n", CurSaveArea->min_level, CurSaveArea->max_level);
    FPRINTF(SaveFile, "V %i %i\n", CurSaveArea->min_vnum, CurSaveArea->max_vnum);
    FPRINTF(SaveFile, "X %i\n", CurSaveArea->offset);
    FPRINTF(SaveFile, "F %i\n", CurSaveArea->reset_rate);
    FPRINTF(SaveFile, "U %s~\n", CurSaveArea->reset_msg);

    if (CurSaveArea->nocmd && CurSaveArea->nocmd[0] != '\0')
        FPRINTF(SaveFile, "j %s~\n", CurSaveArea->nocmd);
    if (CurSaveArea->nospell && CurSaveArea->nospell[0] != '\0')
        FPRINTF(SaveFile, "J %s~\n", CurSaveArea->nospell);

    if (CurSaveArea->owner != NULL)
        FPRINTF(SaveFile, "O %s~\n", CurSaveArea->owner);
    if (CurSaveArea->can_read != NULL)
        FPRINTF(SaveFile, "R %s~\n", CurSaveArea->can_read);
    if (CurSaveArea->can_write != NULL)
        FPRINTF(SaveFile, "W %s~\n", CurSaveArea->can_write);
    if (IS_SET(CurSaveArea->flags, AREA_PAYAREA))
        FPRINTF(SaveFile, "P This is a pay area.\n");
    if (IS_SET(CurSaveArea->flags, AREA_TELEPORT))
        FPRINTF(SaveFile, "T You can teleport into here\n");
    if (IS_SET(CurSaveArea->flags, AREA_BUILDING))
        FPRINTF(SaveFile, "B Currently building area.\n");
    if (IS_SET(CurSaveArea->flags, AREA_BUILDVISIT))
        FPRINTF(SaveFile, "A Can go to this area even if not owner.\n");
    if (IS_SET(CurSaveArea->flags, AREA_NOSHOW))
        FPRINTF(SaveFile, "S Title not shown on area list.\n");
    if (IS_SET(CurSaveArea->flags, AREA_NO_ROOM_AFF))
        FPRINTF(SaveFile, "M No bad room spells allowed.\n");
    if (IS_SET(CurSaveArea->flags, AREA_NOENERGY))
        FPRINTF(SaveFile, "E Energy does not regenerate.\n");

    /*     FPRINTF( Envy, "#AREA\n" );                      remove save bug */
    /*     FPRINTF( Envy, "%s~\n", CurSaveArea->name );                     */

    Section++;
}

void
build_save_mobs()
{
    MOB_INDEX_DATA     *pMobIndex;
    MPROG_DATA         *mprg;
    int                 finish_progs;

    if (Pointer == NULL) {        /* Start */
        if (CurSaveArea->first_area_mobile == NULL) {
            Section++;
            return;
        }
        send_to_char("Saving mobs.\n", CurSaveChar);
        FPRINTF(SaveFile, "#MOBILES\n");
        Pointer = CurSaveArea->first_area_mobile;
    }

    pMobIndex = Pointer->data;
    FPRINTF(SaveFile, "#%i\n", pMobIndex->vnum);
    FPRINTF(SaveFile, "%s~\n", pMobIndex->player_name);
    FPRINTF(SaveFile, "%s~\n", pMobIndex->short_descr);
    FPRINTF(SaveFile, "%s~\n", pMobIndex->long_descr);
    FPRINTF(SaveFile, "%s~\n", pMobIndex->description);
    FPRINTF(SaveFile, "%i %i %i S\n", pMobIndex->act, pMobIndex->affected_by, pMobIndex->alignment);
    FPRINTF(SaveFile, "%i %i\n", pMobIndex->level, pMobIndex->sex);
    FPRINTF(SaveFile, "%i %i %i %i %i\n", pMobIndex->ac_mod, pMobIndex->hr_mod, pMobIndex->dr_mod, pMobIndex->hp_mod, pMobIndex->mana_mod);

    FPRINTF(SaveFile, "! %i %i %i %i %i %i %i\n",
        pMobIndex->class, pMobIndex->clan, pMobIndex->race, pMobIndex->position, pMobIndex->skills, pMobIndex->cast, pMobIndex->def);

    if (pMobIndex->custom_xp != -1)
        FPRINTF(SaveFile, "X %i\n", pMobIndex->custom_xp);

    if (pMobIndex->custom_minlev != -1 && pMobIndex->custom_maxlev != -1)
        FPRINTF(SaveFile, "Y %i %i\n", pMobIndex->custom_minlev, pMobIndex->custom_maxlev);

    if (pMobIndex->path != NULL && pMobIndex->path[0] != '\0')
        FPRINTF(SaveFile, "P %s~\n", pMobIndex->path);

    if (pMobIndex->spec_fun != NULL)
        FPRINTF(SaveFile, "S %s~\n", rev_spec_lookup((void *)pMobIndex->spec_fun));

    mprg = pMobIndex->first_mprog;
    finish_progs = 0;
    while (mprg) {
        if (mprg->filename == NULL) {
            FPRINTF(SaveFile, ">%s ", mprog_type_to_name(mprg->type));
            FPRINTF(SaveFile, "%s~\n", mprg->arglist);
            FPRINTF(SaveFile, "%s~\n", mprg->comlist);
            finish_progs = 1;
        }
        mprg = mprg->next;
    }
    if (finish_progs) {
        FPRINTF(SaveFile, "|\n");
    }

    Pointer = Pointer->next;
    if (Pointer == NULL) {        /* End */
        FPRINTF(SaveFile, "#0\n");
        Section++;
    }
    return;
}

void
build_save_mobprogs()
{
    MOB_INDEX_DATA     *pMobIndex;
    MOBPROG_ITEM       *pItem;

    if (Pointer == NULL) {        /* Start */
        if (CurSaveArea->first_area_mobprog == NULL) {
            Section++;
            return;
        }
        send_to_char("Saving mobprogs.\n", CurSaveChar);
        FPRINTF(SaveFile, "#MOBPROGS\n");

        Pointer = CurSaveArea->first_area_mobprog;
    }

    pItem = Pointer->data;
    pMobIndex = pItem->mob;

    FPRINTF(SaveFile, "M %i %s\n", pMobIndex->vnum, pItem->filename);

    Pointer = Pointer->next;
    if (Pointer == NULL) {        /* End */
        FPRINTF(SaveFile, "S\n");
        Section++;
    }
    return;
}

void
build_save_objects()
{
    OBJ_INDEX_DATA     *pObject;
    AFFECT_DATA        *pAf;
    EXTRA_DESCR_DATA   *pEd;
    int                 val0, val1, val2, val3;

    if (Pointer == NULL) {        /* Start */
        if (CurSaveArea->first_area_object == NULL) {
            Section++;
            return;
        }
        send_to_char("Saving objects.\n", CurSaveChar);
        FPRINTF(SaveFile, "#OBJECTS\n");
        Pointer = CurSaveArea->first_area_object;
    }

    pObject = Pointer->data;

    FPRINTF(SaveFile, "#%i\n", pObject->vnum);
    FPRINTF(SaveFile, "%s~\n", pObject->name);
    FPRINTF(SaveFile, "%s~\n", pObject->short_descr);
    FPRINTF(SaveFile, "%s~\n", pObject->description);
    FPRINTF(SaveFile, "%i %i %i %i\n", pObject->item_type, pObject->extra_flags, pObject->wear_flags, pObject->item_apply);

    /* Check for pills, potions, scrolls, staffs and wands.  */
    val0 = pObject->value[0];
    val1 = pObject->value[1];
    val2 = pObject->value[2];
    val3 = pObject->value[3];
    switch (pObject->item_type) {
        case ITEM_PILL:
        case ITEM_POTION:
        case ITEM_SCROLL:
            val1 = val1 < 0 ? -1 : skill_table[val1].slot;
            val2 = val2 < 0 ? -1 : skill_table[val2].slot;
            val3 = val3 < 0 ? -1 : skill_table[val3].slot;
            break;

        case ITEM_STAFF:
        case ITEM_WAND:
            val3 = val3 < 0 ? -1 : skill_table[val3].slot;
            break;
    }
    FPRINTF(SaveFile, "%i %i %i %i\n", val0, val1, val2, val3);
    FPRINTF(SaveFile, "%i\n", pObject->weight);

    pAf = pObject->first_apply;
    while (pAf) {
        FPRINTF(SaveFile, "A %i %i\n", pAf->location, pAf->modifier);
        pAf = pAf->next;
    }

    pEd = pObject->first_exdesc;
    while (pEd) {
        FPRINTF(SaveFile, "E\n");
        smash_tilde(pEd->keyword);
        FPRINTF(SaveFile, "%s~\n", pEd->keyword);
        smash_tilde(pEd->description);
        FPRINTF(SaveFile, "%s~\n", pEd->description);
        pEd = pEd->next;
    }

    if ((pObject->level > 1) && (pObject->level < 130)) {
        FPRINTF(SaveFile, "L %d\n", pObject->level);
    }
    else {
        FPRINTF(SaveFile, "L 1\n");
    }

    /* alternative cost */
    if ((pObject->newcost == TRUE) && (pObject->cost > 0)) {
        FPRINTF(SaveFile, "C %d\n", pObject->cost);
    }

    /* rarity */
    if (pObject->rarity != 0) {
        FPRINTF(SaveFile, "R %d\n", pObject->rarity);
    }

    if (pObject->obj_fun != NULL) {
        FPRINTF(SaveFile, "O %s~\n", rev_obj_fun_lookup((void *)pObject->obj_fun));
    }

    /* Now for Envy... taken from my OLC :P */

    Pointer = Pointer->next;
    if (Pointer == NULL) {        /* End */
        FPRINTF(SaveFile, "#0\n");
        Section++;
    }

    return;
}

void
build_save_rooms()
{
    ROOM_INDEX_DATA    *pRoomIndex;
    EXTRA_DESCR_DATA   *pEd;

    /* teleport */
    TELEPORT_DATA      *tele;
    LOCKER_DATA        *locker;

    int                 d;
    EXIT_DATA          *pexit;
    int                 locks = 0;

    if (Pointer == NULL) {        /* Start */
        if (CurSaveArea->first_area_room == NULL) {
            Section++;
            return;
        }
        send_to_char("Saving rooms.\n", CurSaveChar);
        FPRINTF(SaveFile, "#ROOMS\n");
        Pointer = CurSaveArea->first_area_room;
    }

    pRoomIndex = Pointer->data;

    FPRINTF(SaveFile, "#%i\n", pRoomIndex->vnum);
    FPRINTF(SaveFile, "%s~\n", pRoomIndex->name);
    FPRINTF(SaveFile, "%s~\n", pRoomIndex->description);
    FPRINTF(SaveFile, "%i %i\n", pRoomIndex->room_flags, pRoomIndex->sector_type);

    /* Now do doors. */
    for (d = 0; d < 6; d++) {
        if (pRoomIndex->exit[d]) {

            FPRINTF(SaveFile, "D%i\n", d);
            pexit = pRoomIndex->exit[d];
            FPRINTF(SaveFile, "%s~\n", pexit->description);
            FPRINTF(SaveFile, "%s~\n", pexit->keyword);
            /* Deal with locks */
            /* -S- Mod: Filter out EX_LOCKED and EX_CLOSED and save exit_info */
            locks = pexit->exit_info;
            if (IS_SET(locks, EX_CLOSED))
                REMOVE_BIT(locks, EX_CLOSED);
            if (IS_SET(locks, EX_LOCKED))
                REMOVE_BIT(locks, EX_LOCKED);

#if 0
            /* BUG: before, an isdoor OR pickproof flag alone would give locks=1 */
            if (pexit->exit_info & EX_ISDOOR)
                locks++;
            if (pexit->exit_info & EX_PICKPROOF)
                locks++;
#endif

            FPRINTF(SaveFile, "%i %i %i\n", locks, pexit->key, pexit->vnum);
        }
    }

    /* Now do extra descripts.. */

    pEd = pRoomIndex->first_exdesc;
    while (pEd) {
        FPRINTF(SaveFile, "E\n");
        smash_tilde(pEd->keyword);
        FPRINTF(SaveFile, "%s~\n", pEd->keyword);
        smash_tilde(pEd->description);
        FPRINTF(SaveFile, "%s~\n", pEd->description);
        pEd = pEd->next;
    }

    /* teleport */
    if (pRoomIndex->tele) {
        tele = pRoomIndex->tele;

        FPRINTF(SaveFile, "T\n");
        FPRINTF(SaveFile, "%d %d %d\n", tele->flags, tele->vnum, tele->wait);
        FPRINTF(SaveFile, "%s~\n", tele->in);
        FPRINTF(SaveFile, "%s~\n", tele->out);

    }

    if (pRoomIndex->locker) {
        int cnt;

        locker = pRoomIndex->locker;

        FPRINTF(SaveFile, "L\n");
        FPRINTF(SaveFile, "%d %d %d %d\n", locker->flags, locker->maxitem, locker->maxweight, locker->valid ? TRUE : FALSE);

        for (cnt = 0; cnt < MAX_ITEM; cnt++)
            FPRINTF(SaveFile, "%d ", locker->types[cnt] ? TRUE : FALSE);

        FPRINTF(SaveFile, "\n");
    }

    if (pRoomIndex->nocmd && strcmp(pRoomIndex->nocmd, "")) {
        smash_tilde(pRoomIndex->nocmd);
        FPRINTF(SaveFile, "C\n");
        FPRINTF(SaveFile, "%s~\n", pRoomIndex->nocmd);
    }
    if (pRoomIndex->nospell && strcmp(pRoomIndex->nospell, "")) {
        smash_tilde(pRoomIndex->nospell);
        FPRINTF(SaveFile, "X\n");
        FPRINTF(SaveFile, "%s~\n", pRoomIndex->nospell);
    }

    /* End of one room */
    FPRINTF(SaveFile, "S\n");

    Pointer = Pointer->next;
    if (Pointer == NULL) {        /* End */
        FPRINTF(SaveFile, "#0\n");
        Section++;
    }
    return;
}

void
build_save_shops()
{
    SHOP_DATA          *pShop;
    int                 iTrade;

    if (Pointer == NULL) {        /* Start */
        if (CurSaveArea->first_area_shop == NULL) {
            Section++;
            return;
        }
        send_to_char("Saving shops.\n", CurSaveChar);
        FPRINTF(SaveFile, "#SHOPS\n");
        Pointer = CurSaveArea->first_area_shop;
    }

    pShop = Pointer->data;
    FPRINTF(SaveFile, "%i ", pShop->keeper);
    for (iTrade = 0; iTrade < MAX_TRADE; iTrade++)
        FPRINTF(SaveFile, "%i ", pShop->buy_type[iTrade]);
    FPRINTF(SaveFile, "%i %i %i %i\n", pShop->profit_buy, pShop->profit_sell, pShop->open_hour, pShop->close_hour);

    Pointer = Pointer->next;
    if (Pointer == NULL) {        /* End */
        FPRINTF(SaveFile, "0\n");
        Section++;
    }
    return;
}

void
build_save_resets()
{
    if (ResetPointer == NULL) {    /* Start */
        if (CurSaveArea->first_reset == NULL) {
            Section++;
            return;
        }
        send_to_char("Saving resets.\n", CurSaveChar);
        FPRINTF(SaveFile, "#RESETS\n");
        ResetPointer = CurSaveArea->first_reset;
    }

    FPRINTF(SaveFile, "%c %i %i %i ", ResetPointer->command, ResetPointer->ifflag, ResetPointer->arg1, ResetPointer->arg2);
    if (ResetPointer->command == 'G' || ResetPointer->command == 'R')
        FPRINTF(SaveFile, "%s\n", ResetPointer->notes);
    else
        FPRINTF(SaveFile, "%i %s\n", ResetPointer->arg3, ResetPointer->notes);

    ResetPointer = ResetPointer->next;
    if (ResetPointer == NULL) {    /* End */
        FPRINTF(SaveFile, "S\n");
        Section++;
    }
    return;
}

void
build_save_end()
{
    char                filename[255];
    char                buf[MAX_STRING_LENGTH];

    if (nosave)
        return;

    sprintf(buf, "Finished saving %s", CurSaveArea->filename);
    monitor_chan(buf, MONITOR_AREA_SAVING);

    FPRINTF(SaveFile, "#$\n");
    send_to_char("Finished saving.\n", CurSaveChar);
    fclose(SaveFile);
    remove(CurSaveArea->filename);
    /* And rename .new to area filename */
    sprintf(filename, "%s.new", CurSaveArea->filename);
    rename(filename, CurSaveArea->filename);

    Section = 0;
    if (ToBeSaved == (CurrentSaving + 1) % SAVEQUEUESIZE)
        saving_area = NOT_SAVING;
    else
        saving_area = START_SAVING;
}

void
build_save_flush()
{
    AREA_DATA          *pArea;

    if (AreasModified == 0)
        return;

    for (pArea = first_area; pArea != NULL; pArea = pArea->next) {
        if (pArea->modified) {
            pArea->modified = 0;
            do_savearea(NULL, (char *) pArea);
        }
    }

    AreasModified = 0;
}

void
area_modified(AREA_DATA *pArea)
{
    pArea->modified = 1;
    AreasModified = 1;
}
