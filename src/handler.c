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
#include "duel.h"

IDSTRING(rcsid, "$Id: handler.c,v 1.81 2005/02/17 23:52:52 dave Exp $");

extern OBJ_DATA    *quest_object;
extern bool         quest;
extern CHAR_DATA   *quest_mob;
extern CHAR_DATA   *quest_target;
extern int          top_mob;
extern int          top_obj;
extern int          fBootDb;

extern void fwrite_corpse(OBJ_DATA *obj, FILE * fp, int iNest);
/*
 * Local functions.
 */
void affect_modify  args((CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd));
bool is_charmie_of  args((CHAR_DATA *ch, CHAR_DATA *charmie));

/*
 * Updated pointer referencing, curtesy of Spectrum, from Beyond the Veil
 *
 */

struct obj_ref_type *obj_ref_list;

void
obj_reference(struct obj_ref_type *ref)
{
    if (ref->inuse) {
        bug("Reused obj_reference!");
        abort();
    }

    ref->inuse = TRUE;
    ref->next = obj_ref_list;
    obj_ref_list = ref;
}

void
obj_unreference(OBJ_DATA **var)
{
    struct obj_ref_type *p, *last;

    for (p = obj_ref_list, last = NULL; p && p->var != var; last = p, p = p->next);

    if (!p) {
        bug("obj_unreference: var not found");
        return;
    }

    p->inuse = FALSE;

    if (!last)
        obj_ref_list = obj_ref_list->next;
    else
        last->next = p->next;
}

struct char_ref_type *char_ref_list;

void
char_reference(struct char_ref_type *ref)
{
    if (ref->inuse) {
        bug("Reused char_reference!");
        abort();
    }

    ref->inuse = TRUE;
    ref->next = char_ref_list;
    char_ref_list = ref;
}

void
char_unreference(CHAR_DATA **var)
{
    struct char_ref_type *p, *last;

    for (p = char_ref_list, last = NULL; p && p->var != var; last = p, p = p->next);

    if (!p) {
        bug("char_unreference: var not found");
        return;
    }

    p->inuse = FALSE;

    if (!last)
        char_ref_list = char_ref_list->next;
    else
        last->next = p->next;
}

/*
 * Retrieve a character's trusted level for permission checking.
 */
int
get_trust(CHAR_DATA *ch)
{
    if (ch->desc != NULL && ch->desc->original != NULL)
        ch = ch->desc->original;

    if (ch->trust != 0)
        return ch->trust;

    if (IS_NPC(ch) && ch->level >= LEVEL_HERO)
        return LEVEL_HERO - 1;
    else
        return ch->level;
}

/* 
 * Replacement for retrieving a character's age
 * Each tick = 1 mud hr.  (spaced at 1 minute rl)
 * 24 mud hrs = 1 mud day
 * 20 mud days = 1 mud month
 * 8 mud months = 1 mud year
 * Therefore, 24*20*8 = 3840 ticks/mins.
 * Returns a string with age info in for use by score, mst, etc
 */

void
my_get_age(CHAR_DATA *ch, char *buf)
{
    int                 days, years, months;
    int                 base, ticks;

    /* Base = time in seconds ch has been playing... */
    base = ch->played + (int) (current_time - ch->logon);

    ticks = base / 60;            /* 1 tick = 60 seconds */

    days = (ticks / 24) % 20;
    months = (ticks / 480) % 8;
    years = 17 + (ticks / 3840);

    sprintf(buf + strlen(buf), "%d years, %d months and %d days", years, months, days);
    return;
}

/* Simple function to return number of hours a character has played */
int
my_get_hours(CHAR_DATA *ch)
{
    int                 secs;
    int                 hrs;

    secs = ch->played + (int) (current_time - ch->logon);
    hrs = (secs / 3600);

    return hrs;
}

/*
 * Retrieve a character's age.
 */
int
get_age(CHAR_DATA *ch)
{
    return 17 + (ch->played + (int) (current_time - ch->logon)) / 14400;

    /* 12240 assumes 30 second hours, 24 hours a day, 20 day - Kahn */
}

/*
 * Retrieve character's current strength.
 */
int
get_curr_str(CHAR_DATA *ch)
{
    int                 max;
    DUEL_DATA          *duel;
    DUEL_PLAYER_DATA   *player;

    if (IS_NPC(ch)) {
        return (13 + (ch->level / 16));
    }
    else if ((duel = find_duel(ch)) && (player = find_duel_player(ch)) && duel->stage == DUEL_STAGE_GO)
        return player->str;

    max = ch->pcdata->max_str;

    return URANGE(3, ch->pcdata->perm_str + ch->pcdata->mod_str, max);
}

/*
 * Retrieve character's current intelligence.
 */
int
get_curr_int(CHAR_DATA *ch)
{
    int                 max;
    DUEL_DATA          *duel;
    DUEL_PLAYER_DATA   *player;

    if (IS_NPC(ch)) {
        return (15 + number_fuzzy((ch->level / 20)));
    }
    else if ((duel = find_duel(ch)) && (player = find_duel_player(ch)) && duel->stage == DUEL_STAGE_GO)
        return player->intel;

    max = ch->pcdata->max_int;

    return URANGE(3, ch->pcdata->perm_int + ch->pcdata->mod_int, max);
}

/*
 * Retrieve character's current wisdom.
 */
int
get_curr_wis(CHAR_DATA *ch)
{
    int                 max;

    if (IS_NPC(ch)) {
        return (15 + number_fuzzy((ch->level / 20)));
    }

    max = ch->pcdata->max_wis;

    return URANGE(3, ch->pcdata->perm_wis + ch->pcdata->mod_wis, max);
}

/*
 * Retrieve character's current dexterity.
 */
int
get_curr_dex(CHAR_DATA *ch)
{
    int                 max;
    DUEL_DATA          *duel;
    DUEL_PLAYER_DATA   *player;

    if (IS_NPC(ch)) {
        return (16 + number_fuzzy((ch->level / 25)));
    }
    else if ((duel = find_duel(ch)) && (player = find_duel_player(ch)) && duel->stage == DUEL_STAGE_GO)
        return player->dex;

    max = ch->pcdata->max_dex;

    return URANGE(3, ch->pcdata->perm_dex + ch->pcdata->mod_dex, max);
}

/*
 * Retrieve character's current constitution.
 */
int
get_curr_con(CHAR_DATA *ch)
{
    int                 max;

    if (IS_NPC(ch)) {
        return (15 + number_fuzzy((ch->level / 12)));
    }

    max = ch->pcdata->max_con;

    return URANGE(3, ch->pcdata->perm_con + ch->pcdata->mod_con, max);
}

/*
 * Retrieve a character's carry capacity.
 */
int
can_carry_n(CHAR_DATA *ch)
{
    if (!IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL)
        return 500;

    /*   if ( IS_NPC(ch) && IS_SET(ch->act, ACT_PET) )
       return 0;  */

    return MAX_WEAR + 2 * get_curr_dex(ch) / 2;
}

/*
 * Retrieve a character's carry capacity.
 */
int
can_carry_w(CHAR_DATA *ch)
{
    if (!IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL)
        return 9999999;

    /*    if ( IS_NPC(ch) && IS_SET(ch->act, ACT_PET) )
       return 0;   */

    return str_app[get_curr_str(ch)].carry;
}

/*
 * See if a string is one of the names of an object.
 */
/*
 * New is_name sent in by Alander.
 */

bool
is_name(const char *str, char *namelist)
{
    char                name[MAX_INPUT_LENGTH];

    if (str == NULL || namelist == NULL)
        return FALSE;

    for (;;) {
        namelist = one_argument(namelist, name);
        if (name[0] == '\0')
            return FALSE;
        if (!str_cmp(str, name))
            return TRUE;
    }
}

/*
 * Apply or remove an affect to a character.
 */
void
affect_modify(CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd)
{
    char                buf[MAX_STRING_LENGTH];
    OBJ_DATA           *wield;
    int                 mod;

    mod = paf->modifier;

    if (fAdd) {
        SET_BIT(ch->affected_by, paf->bitvector);
    }
    else {
        REMOVE_BIT(ch->affected_by, paf->bitvector);
        mod = 0 - mod;
    }

    if (IS_NPC(ch))
        return;

    switch (paf->location) {
        default:
            bugf("Affect_modify: unknown location %d.", paf->location);
            sprintf(buf, "Affect_modify: called for %s - unknown location %d.", ch->name, paf->location);
            monitor_chan(buf, MONITOR_OBJ);
            return;

        case APPLY_NONE:
            break;
        case APPLY_STR:
            ch->pcdata->mod_str += mod;
            break;
        case APPLY_DEX:
            ch->pcdata->mod_dex += mod;
            break;
        case APPLY_INT:
            ch->pcdata->mod_int += mod;
            break;
        case APPLY_WIS:
            ch->pcdata->mod_wis += mod;
            break;
        case APPLY_CON:
            ch->pcdata->mod_con += mod;
            break;
        case APPLY_SEX:
            ch->sex += mod;
            break;
        case APPLY_CLASS:
            break;
        case APPLY_LEVEL:
            break;
        case APPLY_AGE:
            break;
        case APPLY_HEIGHT:
            break;
        case APPLY_WEIGHT:
            break;
        case APPLY_MANA:
            ch->max_mana += mod;
            break;
        case APPLY_HIT:
            ch->max_hit += mod;
            break;
        case APPLY_MOVE:
            ch->max_move += mod;
            break;
        case APPLY_GOLD:
            break;
        case APPLY_EXP:
            break;
        case APPLY_AC:
            ch->armor += mod;
            break;
        case APPLY_HITROLL:
            ch->hitroll += mod;
            break;
        case APPLY_DAMROLL:
            ch->damroll += mod;
            break;
        case APPLY_SAVING_PARA:
            ch->saving_throw += mod;
            break;
        case APPLY_SAVING_ROD:
            ch->saving_throw += mod;
            break;
        case APPLY_SAVING_PETRI:
            ch->saving_throw += mod;
            break;
        case APPLY_SAVING_BREATH:
            ch->saving_throw += mod;
            break;
        case APPLY_SAVING_SPELL:
            ch->saving_throw += mod;
            break;
    }

    /*
     * Check for weapon wielding.
     * Guard against recursion (for weapons with affects).
     */
    if ((ch->is_quitting == FALSE)
        && (ch->desc != NULL)
        && (ch->desc->connected != CON_SETTING_STATS)) {
        sh_int              i;

        for (i = 0; i < MAX_WEAR; i++) {
            if (((wield = get_eq_char(ch, i)) != NULL)
                && (get_obj_weight(wield) > str_app[get_curr_str(ch)].wield)) {
                static int          depth;

                if (depth == 0) {
                    depth++;
                    act("You stop using $p since it is too heavy.", ch, wield, NULL, TO_CHAR);
                    act("$n stops using $p. since it is too heavy", ch, wield, NULL, TO_ROOM);
                    unequip_char(ch, wield);
                    /*      obj_to_room( wield, ch->in_room );  */
                    depth--;
                }
            }
        }
    }
    return;
}

/* Give an affect to a room */
void
affect_to_room(ROOM_INDEX_DATA *room, ROOM_AFFECT_DATA *raf)
{
    ROOM_AFFECT_DATA   *raf_new;
    char                buf[MAX_STRING_LENGTH];

    GET_FREE(raf_new, raffect_free);
    /* Ramias... Don't copy uninitialized fields: next, prev, is_free */
    /*
     *raf_new = *raf;
     */
    raf_new->duration = raf->duration;
    raf_new->level = raf->level;
    raf_new->type = raf->type;
    raf_new->bitvector = raf->bitvector;
    raf_new->applies_spell = raf->applies_spell;
    raf_new->modifier = raf->modifier;
    raf_new->location = raf->location;
    raf_new->caster = raf->caster;

    if (raf->name == NULL)
        raf_new->name = str_dup("");
    else
        raf_new->name = raf->name;

    LINK(raf_new, room->first_room_affect, room->last_room_affect, next, prev);

    SET_BIT(room->affected_by, raf->bitvector);

    if ((raf->bitvector == ROOM_BV_WARNING_RUNE || raf->bitvector == ROOM_BV_SENTRY) && raf->caster != NULL && !IS_NPC(raf->caster))
        raf->caster->pcdata->runes++;

    if (raf->bitvector != ROOM_BV_HEAL_REGEN && raf->bitvector != ROOM_BV_MANA_REGEN && raf->caster) {
        if (!IS_NPC(raf->caster)) {
            sprintf(buf, "@@e%s@@N has cast @@d%s@@N in @@Narea: @@r%s@@N, @@Nroom: @@r%d@@N.",
                raf->caster->short_descr, raffect_bit_name(raf->bitvector), room->area->name, room->vnum);
            monitor_chan(buf, MONITOR_GEN_MORT);
        }
        else if (IS_AFFECTED(raf->caster, AFF_CHARM) && raf->caster->master) {
            sprintf(buf, "@@e%s@@N (%s@@N) has cast @@d%s@@N in @@Narea: @@r%s@@N, @@Nroom: @@r%d@@N.",
                raf->caster->short_descr, raf->caster->master->short_descr, raffect_bit_name(raf->bitvector), room->area->name, room->vnum);
            monitor_chan(buf, MONITOR_GEN_MORT);
        }

    }

    return;
}

/* Remove an affect from a room */
void
r_affect_remove(ROOM_INDEX_DATA *room, ROOM_AFFECT_DATA *raf)
{
    if (room->first_room_affect == NULL) {
        char                buf[MAX_STRING_LENGTH];

        sprintf(buf, "r_affect_remove: no affect to remove from room %d.", room->vnum);
        monitor_chan(buf, MONITOR_ROOM);

        bugf("R_affect_remove: no affect for room: %d.", room->vnum);
        return;
    }

    REMOVE_BIT(room->affected_by, raf->bitvector);

    if (raf->bitvector == ROOM_BV_WARNING_RUNE || raf->bitvector == ROOM_BV_SENTRY)
        if (raf->caster != NULL && !IS_NPC(raf->caster))
            raf->caster->pcdata->runes--;

    free_string(raf->name);

    UNLINK(raf, room->first_room_affect, room->last_room_affect, next, prev);
    PUT_FREE(raf, raffect_free);
    return;
}

/*
 * Give an affect to a char.
 */
void
affect_to_char(CHAR_DATA *ch, AFFECT_DATA *paf)
{
    AFFECT_DATA        *paf_new;

    GET_FREE(paf_new, affect_free);
    /* Ramias... Don't copy uninitialized fields: next, prev, is_free */
    /*
     *paf_new = *paf;
     */
    paf_new->type = paf->type;
    paf_new->duration = paf->duration;
    paf_new->location = paf->location;
    paf_new->modifier = paf->modifier;
    paf_new->bitvector = paf->bitvector;
    paf_new->caster = paf->caster;
    paf_new->level = paf->level;
    paf_new->save = paf->save;

    LINK(paf_new, ch->first_affect, ch->last_affect, next, prev);

    affect_modify(ch, paf_new, TRUE);

    return;
}

/*
 * Remove an affect from a char.
 */
void
affect_remove(CHAR_DATA *ch, AFFECT_DATA *paf)
{
    sh_int              shield_type;

    if (ch->first_affect == NULL) {
        char                buf[MAX_STRING_LENGTH];

        sprintf(buf, "affect_remove: %s did not have aff %d to remove.", IS_NPC(ch) ? ch->short_descr : ch->name, paf->type);
        monitor_chan(buf, MONITOR_MOB);

        bug("Affect_remove: no affect.");
        return;
    }

    affect_modify(ch, paf, FALSE);
    shield_type = SHIELD_NONE;

    if (paf->type == gsn_shield_fire)
        shield_type = SHIELD_FIRE;
    else if (paf->type == gsn_shield_ice)
        shield_type = SHIELD_ICE;
    else if (paf->type == gsn_shield_shock)
        shield_type = SHIELD_SHOCK;
    else if (paf->type == gsn_shield_demon)
        shield_type = SHIELD_DEMON;

    if (shield_type > SHIELD_NONE) {
        SHIELD_DATA       *shield;

        for (shield = ch->first_shield; shield != NULL; shield = shield->next)
            if (shield->index == shield_type)
                break;

        if (shield != NULL) {
            act(shield_table[shield->index].remove_self, ch, NULL, NULL, TO_CHAR);
            act(shield_table[shield->index].remove_room, ch, NULL, NULL, TO_ROOM);

            UNLINK(shield, ch->first_shield, ch->last_shield, next, prev);
            PUT_FREE(shield, shield_free);
        }
    }

    UNLINK(paf, ch->first_affect, ch->last_affect, next, prev);
    PUT_FREE(paf, affect_free);
    return;
}

/*
 * Strip all affects of a given sn.
 */
void
affect_strip(CHAR_DATA *ch, int sn)
{
    AFFECT_DATA        *paf;
    AFFECT_DATA        *paf_next;

    for (paf = ch->first_affect; paf != NULL; paf = paf_next) {
        paf_next = paf->next;
        if (paf->type == sn)
            affect_remove(ch, paf);
    }

    return;
}

/*
 * Return true if a char is affected by a spell.
 */
bool
is_affected(CHAR_DATA *ch, int sn)
{
    AFFECT_DATA        *paf;

    for (paf = ch->first_affect; paf != NULL; paf = paf->next) {
        if (paf->type == sn)
            return TRUE;
    }

    return FALSE;
}

/*
 * Add or enhance an affect.
 */
void
affect_join(CHAR_DATA *ch, AFFECT_DATA *paf)
{
    AFFECT_DATA        *paf_old;
    bool                found;

    found = FALSE;
    for (paf_old = ch->first_affect; paf_old != NULL; paf_old = paf_old->next) {
        if (paf_old->type == paf->type) {
            paf->duration += paf_old->duration;
            paf->modifier += paf_old->modifier;
            affect_remove(ch, paf_old);
            break;
        }
    }

    affect_to_char(ch, paf);
    return;
}

AFFECT_DATA *find_apply_location(void *obj, int location, bool pindex)
{
    AFFECT_DATA *af;
    OBJ_DATA *ob;
    OBJ_INDEX_DATA *pob;

    if (!pindex) {
        ob = (OBJ_DATA *)obj;
        for (af = ob->first_apply; af != NULL; af = af->next)
            if (af->location == location)
                return af;
    }
    else {
        pob = (OBJ_INDEX_DATA *)obj;
        for (af = pob->first_apply; af != NULL; af = af->next)
            if (af->location == location)
                return af;
    }

    return NULL;
}

/*
 * Move a char out of a room.
 */
void
char_from_room(CHAR_DATA *ch)
{
    OBJ_DATA           *obj;
    DUEL_DATA          *duel;

    if (ch->in_room == NULL) {
        char                buf[MAX_STRING_LENGTH];

        sprintf(buf, "char_from_room: %s in NULL room.", IS_NPC(ch) ? ch->short_descr : ch->name);
        monitor_chan(buf, MONITOR_ROOM);

        bug("Char_from_room: NULL.");
        return;
    }

    if (ch->pcdata && !IS_NPC(ch) && ch->pcdata->trading_room && ch->pcdata->trading_room == ch->in_room)
        trade_abort(ch);

    if (ch->pcdata && !IS_NPC(ch)
        && is_in_duel(ch, DUEL_STAGE_READY)
        && (duel = find_duel(ch))
        && (duel->stage == DUEL_STAGE_READY || duel->stage == DUEL_STAGE_SET)
        )
        cancel_duel(duel, ch, DUEL_END_MOVEROOM);

    if (!IS_NPC(ch) && (duel = find_watching_duel(ch))) {
        DUEL_WATCHER_DATA  *watcher;

        for (watcher = duel->first_watcher; watcher != NULL; watcher = watcher->next)
            if (ch == watcher->ch)
                break;

        if (watcher) {
            send_to_char("You stop watching a spar.\n\r", ch);
            DUNLINK(watcher, duel->first_watcher, duel->last_watcher, next, prev);
            DESTROY_MEMBER(watcher);
        }
    }

    if (!IS_NPC(ch))
        --ch->in_room->area->nplayer;

    if ((obj = get_eq_char(ch, WEAR_LIGHT)) != NULL && obj->item_type == ITEM_LIGHT && obj->value[2] != 0 && ch->in_room->light > 0)
        --ch->in_room->light;

    UNLINK(ch, ch->in_room->first_person, ch->in_room->last_person, next_in_room, prev_in_room);

    if (ch->in_room != NULL && ch->in_room->light == 0 && ch->in_room->first_person != NULL)
        ch->in_room->light++;

    if (ch->in_room != NULL)
        ch->last_room = ch->in_room;

    ch->in_room = NULL;
    ch->next_in_room = NULL;
    ch->prev_in_room = NULL;

    if (ch->fighting != NULL)
        stop_fighting(ch, TRUE);

    return;
}

/*
 * Move a char into a room.
 */
void
char_to_room(CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex)
{
    OBJ_DATA           *obj;
    ROOM_AFFECT_DATA   *raf;
    ROOM_AFFECT_DATA   *raf_next;
    AFFECT_DATA         af;
    char                buf[MSL];
    bool                rune_avoid = FALSE;

    if (pRoomIndex == NULL) {
        char                buf[MAX_STRING_LENGTH];

        sprintf(buf, "char_to_room: Attempted to move %s to a NULL room.", NAME(ch));
        monitor_chan(buf, MONITOR_ROOM);

        bug("Char_to_room: NULL.");
        return;
    }

    ch->in_room = pRoomIndex;
    if (IS_NPC(ch))
        TOPLINK(ch, pRoomIndex->first_person, pRoomIndex->last_person, next_in_room, prev_in_room);
    else
        LINK(ch, pRoomIndex->first_person, pRoomIndex->last_person, next_in_room, prev_in_room);

    if (!IS_NPC(ch))
        ++ch->in_room->area->nplayer;

    if (!IS_NPC(ch) && (IS_SET(ch->in_room->room_flags, ROOM_ARENA) || IS_SET(ch->in_room->room_flags, ROOM_ARENA2)) && !ch->pcdata->in_arena)
        join_arena(ch);

    if (!IS_NPC(ch) && !IS_SET(ch->in_room->room_flags, ROOM_ARENA) && !IS_SET(ch->in_room->room_flags, ROOM_ARENA2) && ch->pcdata->in_arena)
        leave_arena(ch, TRUE);

    if ((obj = get_eq_char(ch, WEAR_LIGHT)) != NULL && obj->item_type == ITEM_LIGHT && obj->value[2] != 0)
        ++ch->in_room->light;

    if (ch->fighting != NULL && ch->fighting->in_room != ch->in_room)
        stop_fighting(ch, TRUE);

    if (!IS_NPC(ch) && ch->stance == STANCE_AMBUSH && ch->pcdata->stealth >= gsn_stealth_intermediate)
        rune_avoid = TRUE;

    if (IS_SET(ch->in_room->affected_by, ROOM_BV_FIRE_RUNE) && !IS_IMMORTAL(ch) && !rune_avoid) {
        send_to_char
            ("@@NAs you step into the room, you fleetingly see a mystical @@eFire@@N Rune suspended in front of you, which then @@eEXPLODES@@N!!!\n\r",
            ch);
        act("@@NThe @@eFire@@N Rune explodes as $n enters the room!", ch, NULL, NULL, TO_ROOM);

        for (raf = ch->in_room->first_room_affect; raf != NULL; raf = raf_next) {
            raf_next = raf->next;

            if (raf->bitvector == ROOM_BV_FIRE_RUNE) {
                ch->hit -= raf->modifier;
                r_affect_remove(ch->in_room, raf);

            }
        }
    }

    if (IS_SET(ch->in_room->affected_by, ROOM_BV_SHOCK_RUNE) && !IS_IMMORTAL(ch)
        && (ch->is_free == FALSE) && !rune_avoid) {
        send_to_char
            ("@@NAs you step into the room, you fleetingly see a mystical @@lShock@@N Rune suspended in front of you, which then @@lZAPS@@N You!!!\n\r",
            ch);
        act("@@NThe @@lShock@@N Rune flashes as $n enters the room!", ch, NULL, NULL, TO_ROOM);

        for (raf = ch->in_room->first_room_affect; raf != NULL; raf = raf_next) {
            raf_next = raf->next;

            if (raf->bitvector == ROOM_BV_SHOCK_RUNE) {
                ch->hit -= raf->modifier;
                r_affect_remove(ch->in_room, raf);

            }
        }
    }

    if (IS_SET(ch->in_room->affected_by, ROOM_BV_POISON_RUNE) && !IS_IMMORTAL(ch)
        && (ch->is_free == FALSE) && !rune_avoid) {
        send_to_char
            ("@@NAs you step into the room, you fleetingly see a mystical @@dPoison@@N Rune suspended in front of you, which then @@dEXPLODES@@N!!!\n\r",
            ch);
        act("@@NThe @@dPoison@@N Rune explodes as $n enters the room!", ch, NULL, NULL, TO_ROOM);

        for (raf = ch->in_room->first_room_affect; raf != NULL; raf = raf_next) {
            sh_int              caster_level = 0;

            raf_next = raf->next;

            if (raf->bitvector == ROOM_BV_POISON_RUNE) {
                if (raf->caster == NULL) {
                    caster_level = get_pseudo_level(ch);
                }
                else {
                    caster_level = raf->caster->level;
                }
                if (!saves_spell(caster_level, ch) && ch->race != RACE_LAM) {
                    af.type = skill_lookup("poison");
                    af.duration = 12 + (caster_level / 10);
                    af.location = APPLY_STR;
                    af.modifier = -2;
                    af.bitvector = AFF_POISON;
                    af.save = TRUE;
                    affect_join(ch, &af);
                    send_to_char("You feel very sick.\n\r", ch);
                    act("$n looks very sick.", ch, NULL, NULL, TO_ROOM);
                }
                r_affect_remove(ch->in_room, raf);

            }
        }
    }

    if (ch->is_free == FALSE) {
        for (raf = ch->in_room->first_room_affect; raf != NULL; raf = raf_next) {
            raf_next = raf->next;

            if (raf->bitvector == ROOM_BV_WARNING_RUNE) {
                if (raf->caster) {
                    if (ch == raf->caster || is_charmie_of(raf->caster, ch) || IS_IMMORTAL(ch) || rune_avoid)
                        continue;

                    sprintf(buf, "Your %s rune senses activity, then fades away.\n\r", raf->name);
                    send_to_char(buf, raf->caster);
                }

                r_affect_remove(ch->in_room, raf);

                if (raf->caster && raf->caster->pcdata)
                    raf->caster->pcdata->runes--;
            }
            else if (raf->bitvector == ROOM_BV_SENTRY) {
                buf[0] = 0;

                if (ch == raf->caster || (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM)) || IS_IMMORTAL(ch))
                    continue;

                if (raf->type == gsn_sentry_novice) {
                    if (!IS_NPC(ch) && ch->pcdata->stealth > 0 && ch->stance == STANCE_AMBUSH)
                        ;
                    else
                        sprintf(buf, "Your %s sentry senses %s activity.\n\r", raf->name, IS_NPC(ch) ? "NPC" : "PC");
                }
                else if (raf->type == gsn_sentry_intermediate || raf->type == gsn_sentry_advanced) {
                    if (IS_NPC(ch))
                        sprintf(buf, "Your %s sentry senses %s.\n\r", raf->name, ch->short_descr);
                    else if (ch->stance == STANCE_AMBUSH && raf->type == gsn_sentry_intermediate && ch->pcdata->stealth >= gsn_stealth_intermediate)
                        ;
                    else if (ch->stance == STANCE_AMBUSH && raf->type == gsn_sentry_advanced && ch->pcdata->stealth >= gsn_stealth_advanced)
                        ;
                    else
                        sprintf(buf, "Your %s sentry senses PC activity.\n\r", raf->name);
                }
                else if (raf->type == gsn_sentry_expert || raf->type == gsn_sentry_master) {
                    if (!IS_NPC(ch) && ch->stance == STANCE_AMBUSH && raf->type == gsn_sentry_expert && ch->pcdata->stealth >= gsn_stealth_expert)
                        ;
                    else
                        sprintf(buf, "Your %s sentry senses %s.\n\r", raf->name, ch->short_descr);
                }
                else {
                    buf[0] = 0;
                }

                if (buf[0] != 0)
                    send_to_char(buf, raf->caster);
            }
        }
    }

    if ((ch->in_room->tele) && (ch->in_room->tele->wait > 0)) {
        ch->tele_timer = ch->in_room->tele->wait;
    }
    else {
        ch->tele_timer = 0;
    }

    if (ch->position == POS_BUILDING && ch->act_build == ACT_BUILD_REDIT && ch->build_vnum > 0) {
        if (ch->in_room->vnum != ch->build_vnum) {
            ch->build_vnum = ch->in_room->vnum;
            send_to_char("@@N@@W>>> redit: vnum changed to current room.@@N\n\r", ch);
        }
    }

    /* mounts will ALWAYS follow their masters -E */
    if (!IS_NPC(ch) && ch->riding && ch->riding->in_room && !IS_SET(ch->in_room->room_flags, ROOM_NO_CHARM) && !IS_SET(ch->in_room->area->flags, AREA_NOCHARM)) {
        char_from_room(ch->riding);
        char_to_room(ch->riding, ch->in_room);
    }

    /* only care about last_room in char_from_room() -> char_to_room() instances, so unset it
     * so it's not misused
     */
    if (ch->last_room)
        ch->last_room = NULL;

    return;
}

/*
 * Give an obj to a char.
 */
void
obj_to_char(OBJ_DATA *obj, CHAR_DATA *ch)
{
    extern OBJ_DATA    *quest_object;
    extern CHAR_DATA   *quest_mob;
    extern CHAR_DATA   *quest_target;
    extern unsigned int objid;

    obj->next_in_carry_list = NULL;
    obj->prev_in_carry_list = NULL;

    if (obj == quest_object) {
        bool                valid_questor = FALSE;
        sh_int              personality;

        personality = obj->value[3];

        if (!IS_NPC(ch)) {
            if (personality == 2 && get_pseudo_level(ch) < 95)
                valid_questor = TRUE;
            else if (personality == 3 && get_pseudo_level(ch) > 90)
                valid_questor = TRUE;
        }

        if ((!IS_NPC(ch) && valid_questor == FALSE) || (IS_NPC(ch) && ch != quest_mob && ch != quest_target)) {
            act("$n fumbles, trying to hold $p, and it falls to the ground.", ch, obj, NULL, TO_ROOM);
            act("You try to hold $p, but it seems to come alive, and slips from your grasp and falls to the ground.", ch, obj, NULL, TO_CHAR);
            obj_to_room(obj, ch->in_room);
            return;
        }
    }

    LINK(obj, ch->first_carry, ch->last_carry, next_in_carry_list, prev_in_carry_list);
    obj->carried_by = ch;
    obj->in_room = NULL;
    obj->in_obj = NULL;
    obj->next_in_room = NULL;
    obj->prev_in_room = NULL;
    ch->carry_number += get_obj_number(obj);
    ch->carry_weight += get_obj_weight(obj);

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

    return;
}

/*
 * Take an obj from its character.
 */
void
obj_from_char(OBJ_DATA *obj)
{
    CHAR_DATA          *ch;

    if ((ch = obj->carried_by) == NULL) {
        char                buf[MAX_STRING_LENGTH];

        sprintf(buf, "obj_from_char: NULL ch to remove %s from.", obj->short_descr);
        monitor_chan(buf, MONITOR_OBJ);

        bug("Obj_from_char: null ch.");
        return;
    }

    if (obj->wear_loc != WEAR_NONE)
        unequip_char(ch, obj);

    UNLINK(obj, ch->first_carry, ch->last_carry, next_in_carry_list, prev_in_carry_list);

    obj->carried_by = NULL;
    obj->next_in_carry_list = NULL;
    obj->prev_in_carry_list = NULL;
    obj->next_in_room = NULL;
    obj->prev_in_room = NULL;
    obj->in_room = NULL;
    obj->in_obj = NULL;

    ch->carry_number -= get_obj_number(obj);
    ch->carry_weight -= get_obj_weight(obj);
    return;
}

/*
 * Find the ac value of an obj, including position effect.
 */
int
apply_ac(OBJ_DATA *obj, int iWear)
{
    if (obj->item_type != ITEM_ARMOR)
        return 0;

    switch (iWear) {
        case WEAR_BODY:
            return 3 * obj->value[0];
        case WEAR_HEAD:
            return 2 * obj->value[0];
        case WEAR_LEGS:
            return 2 * obj->value[0];
        case WEAR_FEET:
            return obj->value[0];
        case WEAR_HANDS:
            return obj->value[0];
        case WEAR_ARMS:
            return obj->value[0];
        case WEAR_SHIELD:
            return obj->value[0];
        case WEAR_FINGER_L:
            return obj->value[0];
        case WEAR_FINGER_R:
            return obj->value[0];
        case WEAR_NECK_1:
            return obj->value[0];
        case WEAR_NECK_2:
            return obj->value[0];
        case WEAR_ABOUT:
            return 2 * obj->value[0];
        case WEAR_WAIST:
            return obj->value[0];
        case WEAR_WRIST_L:
            return obj->value[0];
        case WEAR_WRIST_R:
            return obj->value[0];
        case WEAR_HOLD:
            return obj->value[0];
        case WEAR_CLAN:
            return obj->value[0];
    }

    return 0;
}

/*
 * Find a piece of eq on a character.
 */
OBJ_DATA           *
get_eq_char(CHAR_DATA *ch, int iWear)
{
    OBJ_DATA           *obj;

    for (obj = ch->first_carry; obj != NULL; obj = obj->next_in_carry_list) {
        if (obj->wear_loc == iWear)
            return obj;
    }

    return NULL;
}

/*
 * Equip a char with an obj.
 */
void
equip_char(CHAR_DATA *ch, OBJ_DATA *obj, int iWear)
{
    AFFECT_DATA        *paf;
    char                log[MAX_STRING_LENGTH];

    if (((!IS_NPC(ch))
            && (ch->desc != NULL)
            && (ch->desc->connected != CON_SETTING_STATS))
        && (get_eq_char(ch, iWear) != NULL)) {
        sprintf(log, "equip_char: %s (room %d) cannot be equiped with %s, as wear slot (%d) not empty.",
            NAME(ch), ch->in_room->vnum, obj->short_descr, iWear);
        monitor_chan(log, MONITOR_OBJ);

        bug(log);
        return;
    }

    if (((!IS_NPC(ch)) && (ch->desc != NULL) && (ch->desc->connected != CON_SETTING_STATS))
        && ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch))
            || (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch))
            || (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch)))) {
        /*
         * Thanks to Morgenes for the bug fix here!
         */

        if (!IS_NPC(ch) && is_in_duel(ch, DUEL_STAGE_GO)) {
            send_to_char("You can't wear this item in a spar.\n\r", ch);
            return;
        }

        act("You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR);
        act("$n is zapped by $p and drops it.", ch, obj, NULL, TO_ROOM);
        obj_from_char(obj);
        obj_to_room(obj, ch->in_room);
        return;
    }

    ch->armor -= apply_ac(obj, iWear);
    obj->wear_loc = iWear;

    /* Don't need this anymore!
       for ( paf = obj->pIndexData->first_apply; paf != NULL; paf = paf->next )
       affect_modify( ch, paf, TRUE );
       -I think! */

    for (paf = obj->first_apply; paf != NULL; paf = paf->next)
        affect_modify(ch, paf, TRUE);

    /* spec: light bugfix */
    if ((IS_NPC(ch) || !ch->desc || ch->desc->connected != CON_SETTING_STATS)
        && obj->item_type == ITEM_LIGHT && obj->value[2] != 0 && ch->in_room != NULL)
        ++ch->in_room->light;

    /* Check to see if object has magical affects... */

    if (IS_NPC(ch) || (!IS_NPC(ch) && ch->desc && ch->desc->connected != CON_SETTING_STATS)) {

        if (IS_SET(obj->item_apply, ITEM_APPLY_INFRA)) {
            act("$n's eyes glow brightly.", ch, NULL, NULL, TO_ROOM);
            send_to_char("Your eyes glow brightly!\n\r", ch);
        }

        if (IS_SET(obj->item_apply, ITEM_APPLY_INV)) {
            act("$n slowly fades out of existance.", ch, NULL, NULL, TO_ROOM);
            send_to_char("You slowly fade out of existance.\n\r", ch);
        }

        if (IS_SET(obj->item_apply, ITEM_APPLY_DET_INV))
            send_to_char("You feel more aware of your surroundings.\n\r", ch);

        if (IS_SET(obj->item_apply, ITEM_APPLY_SANC)) {
            act("$n is surrounded by a white aura.", ch, NULL, NULL, TO_ROOM);
            send_to_char("You are surrounded by a white aura.\n\r", ch);
        }

        if (IS_SET(obj->item_apply, ITEM_APPLY_SNEAK))
            send_to_char("You feel all sneaky!\n\r", ch);

        if (IS_SET(obj->item_apply, ITEM_APPLY_HIDE))
            send_to_char("You feel almost hidden.\n\r", ch);

        if (IS_SET(obj->item_apply, ITEM_APPLY_PROT))
            send_to_char("You feel more protected.\n\r", ch);

        if (IS_SET(obj->item_apply, ITEM_APPLY_ENHANCED))
            send_to_char("You feel much meaner!\n\r", ch);

        if (IS_SET(obj->item_apply, ITEM_APPLY_DET_MAG)
            || IS_SET(obj->item_apply, ITEM_APPLY_DET_HID)
            || IS_SET(obj->item_apply, ITEM_APPLY_DET_EVIL)
            || IS_SET(obj->item_apply, ITEM_APPLY_KNOW_ALIGN)
            || IS_SET(obj->item_apply, ITEM_APPLY_DET_POISON))
            send_to_char("Your eyes tingle slightly.\n\r", ch);

        if (IS_SET(obj->item_apply, ITEM_APPLY_PASS_DOOR)) {
            act("$n turns turns very pale.", ch, NULL, NULL, TO_ROOM);
            send_to_char("You feel almost weightless!\n\r", ch);
        }

        if (IS_SET(obj->item_apply, ITEM_APPLY_FLY)) {
            act("$n gently floats into the air!", ch, NULL, NULL, TO_ROOM);
            send_to_char("You gently float upwards!\n\r", ch);
        }
    }

    return;
}

/*
 * Unequip a char with an obj.
 */
void
unequip_char(CHAR_DATA *ch, OBJ_DATA *obj)
{
    AFFECT_DATA        *paf;

    if (obj->wear_loc == WEAR_NONE) {
        char                buf[MAX_STRING_LENGTH];

        sprintf(buf, "unequip_char: %s is not wearing %s.", NAME(ch), obj->short_descr);
        monitor_chan(buf, MONITOR_OBJ);

        bug("Unequip_char: already unequipped.");
        return;
    }

    ch->armor += apply_ac(obj, obj->wear_loc);
    obj->wear_loc = -1;

    /*
       for ( paf = obj->pIndexData->first_apply; paf != NULL; paf = paf->next )
       affect_modify( ch, paf, FALSE );
     */
    for (paf = obj->first_apply; paf != NULL; paf = paf->next)
        affect_modify(ch, paf, FALSE);

    if (obj->item_type == ITEM_LIGHT && obj->value[2] != 0 && ch->in_room != NULL && ch->in_room->light > 0)
        --ch->in_room->light;

    /* Check to see if object has magical affects... */

    if (IS_NPC(ch) || (!IS_NPC(ch) && ch->is_quitting == FALSE)) {
        if (IS_SET(obj->item_apply, ITEM_APPLY_INFRA)) {
            act("$n's eyes become dim.", ch, NULL, NULL, TO_ROOM);
            send_to_char("Your eyes become dim!\n\r", ch);
        }

        if (IS_SET(obj->item_apply, ITEM_APPLY_INV)) {
            act("$n slowly fades back into existance.", ch, NULL, NULL, TO_ROOM);
            send_to_char("You slowly fade into existance.\n\r", ch);
        }

        if (IS_SET(obj->item_apply, ITEM_APPLY_DET_INV))
            send_to_char("You feel less aware of your surroundings.\n\r", ch);

        if (IS_SET(obj->item_apply, ITEM_APPLY_SANC)) {
            act("The white aura around $n fades.", ch, NULL, NULL, TO_ROOM);
            send_to_char("The white aura around you fades.\n\r", ch);
        }

        if (IS_SET(obj->item_apply, ITEM_APPLY_SNEAK))
            send_to_char("You feel less sneaky!\n\r", ch);

        if (IS_SET(obj->item_apply, ITEM_APPLY_HIDE))
            send_to_char("You feel less hidden.\n\r", ch);

        if (IS_SET(obj->item_apply, ITEM_APPLY_PROT))
            send_to_char("You feel less protected.\n\r", ch);

        if (IS_SET(obj->item_apply, ITEM_APPLY_ENHANCED))
            send_to_char("You feel much wimpier!\n\r", ch);

        if (IS_SET(obj->item_apply, ITEM_APPLY_DET_MAG)
            || IS_SET(obj->item_apply, ITEM_APPLY_DET_HID)
            || IS_SET(obj->item_apply, ITEM_APPLY_DET_EVIL)
            || IS_SET(obj->item_apply, ITEM_APPLY_KNOW_ALIGN)
            || IS_SET(obj->item_apply, ITEM_APPLY_DET_POISON))
            send_to_char("Your feel less well-informed.\n\r", ch);

        if (IS_SET(obj->item_apply, ITEM_APPLY_PASS_DOOR)) {
            act("$n becomes solid again.", ch, NULL, NULL, TO_ROOM);
            send_to_char("You feel more solid!\n\r", ch);
        }

        if (IS_SET(obj->item_apply, ITEM_APPLY_FLY)) {
            act("$n gently floats to the ground!", ch, NULL, NULL, TO_ROOM);
            send_to_char("You gently float to the ground!\n\r", ch);
        }
    }

    if (ch->mana > ch->max_mana)
        ch->mana = ch->max_mana;
    if (ch->hit > ch->max_hit)
        ch->hit = ch->max_hit;

    return;
}

/*
 * Count occurrences of an obj in a list.
 */
int
count_obj_list(OBJ_INDEX_DATA *pObjIndex, OBJ_DATA *list)
{
    OBJ_DATA           *obj;
    int                 nMatch;

    nMatch = 0;
    for (obj = list; obj != NULL; obj = obj->next_in_carry_list) {
        if (obj->pIndexData == pObjIndex)
            nMatch++;
    }

    return nMatch;
}

/*
 * Count occurrences of an obj in a list.
 */
int
count_obj_room(OBJ_INDEX_DATA *pObjIndex, OBJ_DATA *list)
{
    OBJ_DATA           *obj;
    int                 nMatch;

    nMatch = 0;
    for (obj = list; obj != NULL; obj = obj->next_in_room) {
        if (obj->pIndexData == pObjIndex)
            nMatch++;
    }

    return nMatch;
}

int
count_obj_room_mob(OBJ_INDEX_DATA *pObjIndex, ROOM_INDEX_DATA *room)
{
    int                 nMatch;
    CHAR_DATA          *ch;

    nMatch = 0;

    nMatch += count_obj_room(pObjIndex, room->first_content);

    for (ch = room->first_person; ch != NULL; ch = ch->next_in_room) {
         if (!IS_NPC(ch))
             continue;

         nMatch += count_obj_list(pObjIndex, ch->first_carry);
    }

    return nMatch;
}

/*
 * Move an obj out of a room.
 */
void
obj_from_room(OBJ_DATA *obj)
{
    ROOM_INDEX_DATA    *in_room;

    if ((in_room = obj->in_room) == NULL) {
        char                buf[MAX_STRING_LENGTH];

        sprintf(buf, "obj_from_room: %s in NULL room.", obj->short_descr);
        monitor_chan(buf, MONITOR_OBJ);

        bug("obj_from_room: NULL.");
        /* attempt to recover by moving obj to another room */
        if (obj->carried_by != NULL)
            obj_from_char(obj);
        else if (obj->in_obj != NULL)
            obj_from_obj(obj);

        obj_to_room(obj, get_room_index(ROOM_VNUM_LIMBO));
        if ((in_room = obj->in_room) == NULL) {
            sprintf(buf, "obj_from_room, %s really screwed up, failed attempts to move to Limbo.", obj->short_descr);
            monitor_chan(buf, MONITOR_OBJ);
            return;
        }
        /* code to save everyone here  Zen */
    }

    UNLINK(obj, in_room->first_content, in_room->last_content, next_in_room, prev_in_room);

    obj->in_room = NULL;
    obj->next_in_room = NULL;
    obj->prev_in_room = NULL;
    obj->next_in_carry_list = NULL;
    obj->prev_in_carry_list = NULL;
    obj->carried_by = NULL;
    obj->in_obj = NULL;

    if (in_room && !fBootDb)
        save_locker(in_room);

    return;
}

/*
 * Move an obj into a room.
 */
void
obj_to_room(OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex)
{
    obj->next_in_room = NULL;
    obj->prev_in_room = NULL;

    TOPLINK(obj, pRoomIndex->first_content, pRoomIndex->last_content, next_in_room, prev_in_room);
    obj->in_room = pRoomIndex;
    obj->carried_by = NULL;
    obj->in_obj = NULL;
    obj->next_in_carry_list = NULL;
    obj->prev_in_carry_list = NULL;

    if (!fBootDb)
        save_locker(pRoomIndex);

    return;
}

/*
 * Move an object into an object.
 */
void
obj_to_obj(OBJ_DATA *obj, OBJ_DATA *obj_to)
{
    OBJ_DATA *roomobj;

    obj->next_in_carry_list = NULL;
    obj->prev_in_carry_list = NULL;

    roomobj = obj_to;
    while (roomobj->in_obj)
        roomobj = roomobj->in_obj;

    TOPLINK(obj, obj_to->first_in_carry_list, obj_to->last_in_carry_list, next_in_carry_list, prev_in_carry_list);
    obj->in_obj = obj_to;
    obj->in_room = NULL;
    obj->carried_by = NULL;
    obj->next_in_room = NULL;
    obj->prev_in_room = NULL;
    for (; obj_to != NULL; obj_to = obj_to->in_obj)
        if (obj_to->carried_by != NULL) {
            /*     obj_to->carried_by->carry_number += get_obj_number( obj ); */
            obj_to->carried_by->carry_weight += get_obj_weight(obj);
        }

    if (roomobj && roomobj->in_room && !fBootDb)
        save_locker(roomobj->in_room);

    return;
}

/*
 * Move an object out of an object.
 */
void
obj_from_obj(OBJ_DATA *obj)
{
    OBJ_DATA           *roomobj;
    OBJ_DATA           *obj_from;

    if ((obj_from = obj->in_obj) == NULL) {
        char                buf[MAX_STRING_LENGTH];

        sprintf(buf, "obj_from_obj: %s not in another object.", obj->short_descr);
        monitor_chan(buf, MONITOR_OBJ);
        bug("Obj_from_obj: null obj_from.");
        return;
    }

    roomobj = obj_from;
    while (roomobj->in_obj)
        roomobj = roomobj->in_obj;

    UNLINK(obj, obj_from->first_in_carry_list, obj_from->last_in_carry_list, next_in_carry_list, prev_in_carry_list);

    obj->next_in_carry_list = NULL;
    obj->prev_in_carry_list = NULL;
    obj->in_obj = NULL;
    obj->next_in_room = NULL;
    obj->prev_in_room = NULL;
    obj->carried_by = NULL;
    obj->in_room = NULL;

    for (; obj_from != NULL; obj_from = obj_from->in_obj) {
        if (obj_from->carried_by != NULL) {
            /*      obj_from->carried_by->carry_number -= get_obj_number( obj );  */
            obj_from->carried_by->carry_weight -= get_obj_weight(obj);
        }
    }

    if (roomobj && roomobj->in_room && !fBootDb)
        save_locker(roomobj->in_room);

    return;
}

/*
 * Extract an obj from the world.
 */
void
extract_obj(OBJ_DATA *obj)
{
    CHAR_DATA          *wch;
    OBJ_DATA           *obj_content;
    struct obj_ref_type *ref;
    ROOM_INDEX_DATA    *drop_room = NULL;
    DUEL_DATA          *duel;
    DUEL_PLAYER_DATA   *player;
    DUEL_OBJ_DATA      *duelob, *duelob_next;

    for (ref = obj_ref_list; ref; ref = ref->next)
        if (*ref->var == obj)
            switch (ref->type) {
                case OBJ_NEXT:
                    *ref->var = obj->next;
                    break;
                case OBJ_NEXTCONTENT:
                    *ref->var = obj->next_in_carry_list;
                    break;
                case OBJ_NULL:
                    *ref->var = NULL;
                    break;
                default:
                    bugf("Bad obj_ref_list type %d", ref->type);
                    break;
            }

    if ((obj == quest_object) && quest) {
        if ((obj->in_obj != NULL)
            && ((obj->in_obj->item_type == ITEM_CORPSE_NPC)
                || (obj->in_obj->item_type == ITEM_CORPSE_PC)
                || (obj->in_obj->item_type == ITEM_CONTAINER))) {
            drop_room = obj->in_obj->in_room;
            obj_from_obj(obj);
        }
        else if (obj->carried_by != NULL) {
            drop_room = obj->carried_by->in_room;
            obj_from_char(obj);
        }
        else if (obj->in_room != NULL) {
            drop_room = obj->in_room;
            obj_from_room(obj);
        }

        obj_to_room(obj, (drop_room != NULL ? drop_room : get_room_index(ROOM_VNUM_TEMPLE)));
        return;
    }

    /* make a log of a player corpse that expires that has something in it. ugh, no newbie corpses,
     * too spammy! */
    if (obj->item_type == ITEM_CORPSE_PC && obj->last_in_carry_list != NULL && obj->level >= 40) {
        FILE *fp;

        if (!nosave && (fp = fopen(CORPSELOG_FILE, "a")) != NULL) {
            FPRINTF(fp, "#DATESTAMP %s", ctime(&current_time));
            fwrite_corpse(obj, fp, 0);
            fclose(fp);
        }
    }

    if (obj->carried_by != NULL)
        obj_from_char(obj);
    else if (obj->in_room != NULL)
        obj_from_room(obj);
    else if (obj->in_obj != NULL)
        obj_from_obj(obj);

    while ((obj_content = obj->last_in_carry_list) != NULL)
        extract_obj(obj_content);

    UNLINK(obj, first_obj, last_obj, next, prev);

    {
        AFFECT_DATA        *paf;

        while ((paf = obj->first_apply) != NULL) {
            obj->first_apply = paf->next;
            PUT_FREE(paf, affect_free);
        }
    }

    {
        EXTRA_DESCR_DATA   *ed;

        while ((ed = obj->first_exdesc) != NULL) {
            obj->first_exdesc = ed->next;
            free_string(ed->description);
            free_string(ed->keyword);
            PUT_FREE(ed, exdesc_free);
        }
    }

    /* check if object is burning in a duel */
    for (duel = first_duel; duel != NULL; duel = duel->next) {
        for (player = duel->first_player; player != NULL; player = player->next) {
            for (duelob = player->first_obj; duelob != NULL; duelob = duelob_next) {
                duelob_next = duelob->next;

                if (duelob->obj == obj) {
                    /* this object is burning in a duel, remove it from the duel */
                    DUNLINK(duelob, player->first_obj, player->last_obj, next, prev);
                    DESTROY_MEMBER(duelob);
                }
            }
        }
    }

    for (wch = first_char; wch; wch = wch->next) {
        MPROG_ACT_LIST     *mpact;

        if (wch->hunt_obj == obj)
            end_hunt(wch);
        /*        wch->hunt_obj = NULL; */
        if (wch->sitting == obj)
            do_stand(wch, "");
        for (mpact = first_mpact; mpact; mpact = mpact->next) {
            if (mpact->obj == obj)
                mpact->obj = NULL;
            if (mpact->vo == obj)
                mpact->vo = NULL;
        }
    }

    if (obj->item_type == ITEM_CORPSE_PC) {
        CORPSE_DATA        *this_corpse;

        for (this_corpse = first_corpse; this_corpse != NULL; this_corpse = this_corpse->next)
            if (this_corpse->this_corpse == obj)
                break;
        if (this_corpse != NULL) {
            UNLINK(this_corpse, first_corpse, last_corpse, next, prev);
            PUT_FREE(this_corpse, corpse_free);
        }
        save_corpses();
    }

    free_string(obj->owner);
    free_string(obj->name);
    free_string(obj->description);
    free_string(obj->short_descr);
    --obj->pIndexData->count;
    top_obj--;

    PUT_FREE(obj, obj_free);
    return;
}

/*
 * Extract a char from the world.
 */
void
extract_char(CHAR_DATA *ch, bool fPull)
{
    CHAR_DATA          *wch;
    OBJ_DATA           *this_object;
    extern CHAR_DATA   *quest_mob;
    extern CHAR_DATA   *quest_target;
    struct char_ref_type *ref;
    extern CHAR_DATA   *auction_owner;
    extern CHAR_DATA   *auction_bidder;
    DUEL_DATA          *duel;
    DUEL_WATCHER_DATA  *watcher;

    /*
     * Updated pointer referencing, curtesy of Spectrum, from Beyond the Veil
     *
     */

    if (ch->in_room == NULL) {
        char                buf[MAX_STRING_LENGTH];

        sprintf(buf, "extract_char: %s in NULL room., Moved to room 2", NAME(ch));
        monitor_chan(buf, MONITOR_MOB);

        bug("Extract_char: NULL.");

        /*     char_to_room( ch, get_room_index( ROOM_VNUM_LIMBO ) );  */

        return;
    }

    for (ref = char_ref_list; ref; ref = ref->next)
        if (*ref->var == ch)
            switch (ref->type) {
                case CHAR_NEXT:
                    *ref->var = ch->next;
                    break;
                case CHAR_NEXTROOM:
                    *ref->var = ch->next_in_room;
                    break;
                case CHAR_NULL:
                    *ref->var = NULL;
                    break;
                default:
                    bugf("Bad char_ref_list type %d", ref->type);
                    break;
            }

    if ((ch == quest_mob)
        || (ch == quest_target))
        quest_cancel();

    if (fPull)
        die_follower(ch);

    if (ch == auction_owner)
        auction_owner = NULL;

    if (ch == auction_bidder)
        auction_bidder = NULL;

    stop_fighting(ch, TRUE);
    ch->is_quitting = TRUE;

    if (fPull)
        while ((this_object = ch->last_carry) != NULL)
            extract_obj(this_object);

    char_from_room(ch);

    if (!fPull) {
        OBJ_DATA   *next_object;

        for (this_object = ch->first_carry; this_object != NULL; this_object = next_object) {
            next_object = this_object->next_in_carry_list;

            if (IS_NPC(ch) || (!IS_NPC(ch) && !IS_SET(this_object->extra_flags, ITEM_UNIQUE)))
                extract_obj(this_object);
        }

        char_to_room(ch, get_room_index(ROOM_VNUM_MORIBUND));
        return;
    }

    if (IS_NPC(ch))
        --ch->pIndexData->count;

    if (ch->desc != NULL && ch->desc->original != NULL)
        do_return(ch, "");

    for (wch = first_char; wch != NULL; wch = wch->next) {
        AFFECT_DATA        *paf;

        if (wch->reply == ch)
            wch->reply = NULL;
        if (wch->hunting == ch || wch->hunt_for == ch)
            end_hunt(wch);
        if (!str_cmp(wch->target, ch->name)) {
            free_string(wch->target);
            wch->target = NULL;    /* spec- fix the evil nasty duplicate frees */
        }
        if (wch->riding == ch) {
            do_dismount(wch, "");
            wch->riding = NULL;
        }
        if (wch->rider == ch)
            wch->rider = NULL;
        for (paf = wch->first_affect; paf; paf = paf->next)
            if (paf->caster == ch)
                paf->caster = NULL;
    }

    /* free up any shields  */

    if (ch->first_shield != NULL) {
        SHIELD_DATA       *shield, *shield_next;

        for (shield = ch->first_shield; shield != NULL; shield = shield_next) {
            shield_next = shield->next;

            UNLINK(shield, ch->first_shield, ch->last_shield, next, prev);
            PUT_FREE(shield, shield_free);
        }
    }

    if (!IS_NPC(ch) && ch->pcdata && (duel = find_watching_duel(ch)) && (watcher = find_duel_watcher(ch))) {
        DUNLINK(watcher, duel->first_watcher, duel->last_watcher, next, prev);
        DESTROY_MEMBER(watcher);
    }

    UNLINK(ch, first_char, last_char, next, prev);
    if (IS_NPC(ch))
        top_mob--;

    if (!IS_NPC(ch)) {
        UNLINK(ch, first_player, last_player, next_player, prev_player);

        ch->next_player = NULL;
        ch->prev_player = NULL;
    }

    if (ch->desc)
        ch->desc->character = NULL;
    free_char(ch);
    return;
}

/*
 * Find a char in the room.
 */
CHAR_DATA          *
get_char_room(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    CHAR_DATA          *rch;
    int                 number;
    int                 count;

    number = number_argument(argument, arg);
    count = 0;
    if (arg[0] == '\0')
        return NULL;

    if (!str_cmp(arg, "self"))
        return ch;

    if (!str_cmp(arg, "mount")) {
        if (ch->riding == NULL) {
            send_to_char("You aren't riding anything!\n\r", ch);
            return NULL;
        }
        else if (ch->in_room == ch->riding->in_room)
            return ch->riding;
        else
            return NULL;
    }

    if (!str_cmp(arg, "tank")) {
        if (ch->fighting == NULL) {
            send_to_char("You aren't fighting anyone!\n\r", ch);
            return NULL;
        }
        else if (ch->fighting->fighting == NULL) {
            send_to_char("Hmm, that's weird.. where did he go?\n\r", ch);
            return NULL;
        }
        else {
            return ch->fighting->fighting;
        }
    }

    if (!str_cmp(arg, "enemy")) {
        if (ch->fighting == NULL) {
            send_to_char("You aren't fighting anyone!\n\r", ch);
            return NULL;
        }
        /*
           else 
           if ( ch->fighting->fighting == NULL )
           {
           send_to_char( "Hmm, that's wierd..where did he go?\n\r", ch );
           return NULL;
           }
           if ( ch->fighting->fighting->fighting == NULL )
           {
           send_to_char( "Hmm, that's wierd..where did he go?\n\r", ch );
           return NULL;
           }
         */
        else if (!ch->in_room || !ch->fighting->in_room || ch->in_room != ch->fighting->in_room)
            return NULL;
        else
            return ch->fighting;
    }

    for (rch = ch->in_room->first_person; rch != NULL; rch = rch->next_in_room) {
        if (!can_see(ch, rch) || !is_name(arg, rch->name))
            continue;
        if (++count == number)
            return rch;
    }

    return NULL;
}

/*
 * Find a char in the world.
 */
CHAR_DATA          *
get_char_world(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    CHAR_DATA          *wch;
    int                 number;
    int                 count;

    if ((wch = get_char_room(ch, argument)) != NULL)
        return wch;

    number = number_argument(argument, arg);
    count = 0;
    for (wch = first_char; wch != NULL; wch = wch->next) {
        if (!can_see(ch, wch) || !is_name(arg, wch->name))
            continue;
        if (++count == number)
            return wch;
    }

    return NULL;
}

CHAR_DATA          *
get_char_area(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    CHAR_DATA          *ach;
    int                 number;
    int                 count;

    if ((ach = get_char_room(ch, argument)) != NULL)
        return ach;

    number = number_argument(argument, arg);
    count = 0;
    for (ach = first_char; ach != NULL; ach = ach->next) {
        if (ach->in_room->area != ch->in_room->area || !can_see(ch, ach) || !is_name(arg, ach->name))
            continue;
        if (++count == number)
            return ach;
    }

    return NULL;
}

/* Used mainly for Imtlset ---Flar */
CHAR_DATA          *
get_char(CHAR_DATA *ch)
{
    if (!ch->pcdata)
        return ch->desc->original;
    else
        return ch;
}

/*
 * Find some object with a given index data.
 * Used by area-reset 'P' command.
 */
OBJ_DATA           *
get_obj_type(OBJ_INDEX_DATA *pObjIndex)
{
    OBJ_DATA           *obj;

    for (obj = first_obj; obj != NULL; obj = obj->next) {
        if (obj->pIndexData == pObjIndex)
            return obj;
    }

    return NULL;
}

/*
 * Find an obj in a room.
 */
OBJ_DATA           *
get_obj_room(CHAR_DATA *ch, char *argument, OBJ_DATA *list)
{
    char                arg[MAX_INPUT_LENGTH];
    OBJ_DATA           *obj;
    int                 number;
    int                 count;

    number = number_argument(argument, arg);
    count = 0;
    for (obj = list; obj != NULL; obj = obj->next_in_room) {
        if (can_see_obj(ch, obj) && is_name(arg, obj->name)) {
            if (++count == number)
                return obj;
        }
    }

    return NULL;
}

OBJ_DATA           *
get_obj_room_r(CHAR_DATA *ch, char *argument, OBJ_DATA *list, int number, int *start)
{
    OBJ_DATA           *obj;

    for (obj = list; obj != NULL; obj = obj->next_in_room) {
        if (can_see_obj(ch, obj) && is_name(argument, obj->name)) {
            if (++(*start) == number)
                return obj;
        }
    }

    return NULL;
}

/*
 * Find an obj in a room.
 */
OBJ_DATA           *
get_obj_list(CHAR_DATA *ch, char *argument, OBJ_DATA *list)
{
    char                arg[MAX_INPUT_LENGTH];
    OBJ_DATA           *obj;
    int                 number;
    int                 count;

    number = number_argument(argument, arg);
    count = 0;
    for (obj = list; obj != NULL; obj = obj->next_in_carry_list) {
        if (can_see_obj(ch, obj) && is_name(arg, obj->name)) {
            if (++count == number)
                return obj;
        }
    }

    return NULL;
}

/*
 * Find an obj in player's inventory.
 */
OBJ_DATA           *
get_obj_carry(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    OBJ_DATA           *obj;
    int                 number;
    int                 count;

    number = number_argument(argument, arg);
    count = 0;
    for (obj = ch->first_carry; obj != NULL; obj = obj->next_in_carry_list) {
        if (obj->wear_loc == WEAR_NONE && can_see_obj(ch, obj)
            && is_name(arg, obj->name)) {
            if (++count == number)
                return obj;
        }
    }

    return NULL;
}

OBJ_DATA           *
get_obj_carry_r(CHAR_DATA *ch, char *argument, int number, int *start)
{
    OBJ_DATA           *obj;

    for (obj = ch->first_carry; obj != NULL; obj = obj->next_in_carry_list) {
        if (obj->wear_loc == WEAR_NONE && can_see_obj(ch, obj)
            && is_name(argument, obj->name)) {
            if (++(*start) == number)
                return obj;
        }
    }

    return NULL;
}

/*
 * Find an obj in player's equipment.
 */
OBJ_DATA           *
get_obj_wear(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    OBJ_DATA           *obj;
    int                 number;
    int                 count;

    number = number_argument(argument, arg);
    count = 0;
    for (obj = ch->first_carry; obj != NULL; obj = obj->next_in_carry_list) {
        if (obj->wear_loc != WEAR_NONE && can_see_obj(ch, obj)
            && is_name(arg, obj->name)) {
            if (++count == number)
                return obj;
        }
    }

    return NULL;
}

OBJ_DATA           *
get_obj_wear_r(CHAR_DATA *ch, char *argument, int number, int *start)
{
    OBJ_DATA           *obj;

    for (obj = ch->first_carry; obj != NULL; obj = obj->next_in_carry_list) {
        if (obj->wear_loc != WEAR_NONE && can_see_obj(ch, obj)
            && is_name(argument, obj->name)) {
            if (++(*start) == number)
                return obj;
        }
    }

    return NULL;
}

/*
 * Find an obj in the room or in inventory.
 */
OBJ_DATA           *
get_obj_here(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA           *obj;

    obj = get_obj_room(ch, argument, ch->in_room->first_content);
    if (obj != NULL)
        return obj;

    if ((obj = get_obj_carry(ch, argument)) != NULL)
        return obj;

    if ((obj = get_obj_wear(ch, argument)) != NULL)
        return obj;

    return NULL;
}

OBJ_DATA           *
get_obj_here_r(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA           *obj;
    char                arg[MAX_INPUT_LENGTH];
    int                 number = number_argument(argument, arg);
    int                 _start = 0;
    int                *start = &_start;

    if ((obj = get_obj_carry_r(ch, arg, number, start)) != NULL)
        return obj;

    if ((obj = get_obj_wear_r(ch, arg, number, start)) != NULL)
        return obj;

    if ((obj = get_obj_room_r(ch, arg, ch->in_room->first_content, number, start)) != NULL)
        return obj;

    return NULL;
}

/*
 * Find an obj in the world.
 */
OBJ_DATA           *
get_obj_world(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    OBJ_DATA           *obj;
    int                 number;
    int                 count;

    if ((obj = get_obj_here(ch, argument)) != NULL)
        return obj;

    number = number_argument(argument, arg);
    count = 0;
    for (obj = first_obj; obj != NULL; obj = obj->next) {
        if (obj->carried_by == NULL && obj->in_obj == NULL && obj->in_room == NULL)
            continue;

        if (can_see_obj(ch, obj) && is_name(arg, obj->name)) {
            if (++count == number)
                return obj;
        }
    }

    return NULL;
}

/*
 * Create a 'money' obj.
 */
OBJ_DATA           *
create_money(int amount)
{
    char                buf[MAX_STRING_LENGTH];
    OBJ_DATA           *obj;

    if (amount <= 0) {
        char                buf[MAX_STRING_LENGTH];

        sprintf(buf, "create_money: %d provided as amount.", amount);
        monitor_chan(buf, MONITOR_OBJ);

        bugf("Create_money: zero or negative money %d.", amount);
        amount = 1;
    }

    if (amount == 1) {
        obj = create_object(get_obj_index(OBJ_VNUM_MONEY_ONE), 0);
        obj->value[0] = amount;
    }
    else {
        obj = create_object(get_obj_index(OBJ_VNUM_MONEY_SOME), 0);
        sprintf(buf, obj->short_descr, number_comma(amount));
        free_string(obj->short_descr);
        obj->short_descr = str_dup(buf);
        obj->value[0] = amount;
    }

    return obj;
}

/*
 * Return # of objects which an object counts as.
 * Thanks to Tony Chamberlain for the correct recursive code here.
 */
int
get_obj_number(OBJ_DATA *obj)
{
    int                 number;

    /*   OBJ_DATA *vobj;  */

    number = 1;                    /*set to one since bag will count as 1 item */
    /*    if ( obj->item_type == ITEM_CONTAINER )
       {
       for ( vobj = obj->first_in_carry_list; vobj != NULL; vobj = vobj->next_in_carry_list )
       { 
       number = number - 1;
       }
       }

     */
    /* containers should count as one item!  
       if ( obj->item_type == ITEM_CONTAINER )
       for ( obj = obj->contains; obj != NULL; obj = obj->next_content )
       number += get_obj_number( obj );
       else
       number = 1;
       Zen */
    return number;
}

/*
 * Return weight of an object, including weight of contents.
 */
int
get_obj_weight(OBJ_DATA *obj)
{
    int                 weight;

    if (obj->item_type == ITEM_MONEY) {
        weight = obj->value[0] / 100000;
        return weight;
    }
    weight = obj->weight;
    for (obj = obj->first_in_carry_list; obj != NULL; obj = obj->next_in_carry_list)
        weight += get_obj_weight(obj);

    return weight;
}

/*
 * True if room is dark.
 */
bool
room_is_dark(ROOM_INDEX_DATA *pRoomIndex)
{
    if (pRoomIndex->light > 0)
        return FALSE;

    if (IS_SET(pRoomIndex->room_flags, ROOM_DARK))
        return TRUE;

    if (pRoomIndex->sector_type == SECT_INSIDE || pRoomIndex->sector_type == SECT_CITY)
        return FALSE;

    if (weather_info.moon_phase == MOON_FULL && (weather_info.moon_loc >= MOON_RISE && weather_info.moon_loc <= MOON_FALL))
        return FALSE;

    if (weather_info.sunlight == SUN_SET || weather_info.sunlight == SUN_DARK)
        return TRUE;

    return FALSE;
}

/*
 * True if room is private.
 */
bool
room_is_private(ROOM_INDEX_DATA *pRoomIndex)
{
    CHAR_DATA          *rch;
    int                 count;

    count = 0;
    for (rch = pRoomIndex->first_person; rch != NULL; rch = rch->next_in_room)
        count++;

    if (IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE) && count >= 2)
        return TRUE;

    if (IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY) && count >= 1)
        return TRUE;

    return FALSE;
}

/*
 * True if char can see victim.
 */
bool
can_see(CHAR_DATA *ch, CHAR_DATA *victim)
{
    extern bool quest;
    extern CHAR_DATA *quest_mob;

    if (IS_AFFECTED(ch, AFF_BLIND))
        return FALSE;

    if (ch == victim)
        return TRUE;
    if (is_same_group(ch, victim))
        return TRUE;
    if (victim->leader == ch)
        return TRUE;

    if (!IS_NPC(victim)
        && IS_SET(victim->act, PLR_WIZINVIS)
        && get_trust(ch) < victim->invis)
        return FALSE;

    if (!IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT))
        return TRUE;

    if (IS_NPC(victim) && quest && victim == quest_mob)
        return TRUE;

    if ((room_is_dark(ch->in_room) && !IS_AFFECTED(ch, AFF_INFRARED))
        && ch->in_room == victim->in_room)
        return FALSE;

    if (!IS_NPC(victim)
        && (victim->stance == STANCE_AMBUSH))
        return FALSE;

    /* stance shadows means people below your pseudo level can't see you */
    if (!IS_NPC(victim)
        && (victim->stance == STANCE_AC_BEST)
        && (get_pseudo_level(ch) < get_pseudo_level(victim)))
        return FALSE;

    if ((IS_AFFECTED(victim, AFF_INVISIBLE) || item_has_apply(victim, ITEM_APPLY_INV))
        && (!IS_AFFECTED(ch, AFF_DETECT_INVIS) && !item_has_apply(ch, ITEM_APPLY_DET_INV)))
        return FALSE;

    if (IS_AFFECTED(victim, AFF_INVISIBLE)
        && (IS_AFFECTED(ch, AFF_DETECT_INVIS) || item_has_apply(ch, ITEM_APPLY_DET_INV))
        && get_pseudo_level(victim) - 10 > get_pseudo_level(ch))
        return FALSE;

    /*   if ( ( IS_AFFECTED( victim, AFF_SNEAK ) || item_has_apply( victim, ITEM_APPLY_SNEAK ) )
       && ( number_percent() < 50 + ( 5 * ( get_pseudo_level( victim ) - get_pseudo_level( ch ) ) ) ) )
       return FALSE;  */

    if ((IS_AFFECTED(victim, AFF_HIDE) || item_has_apply(victim, ITEM_APPLY_HIDE))
        && (!IS_AFFECTED(ch, AFF_DETECT_HIDDEN) && !item_has_apply(ch, ITEM_APPLY_DET_HID))
        && victim->fighting == NULL && (IS_NPC(ch) ? !IS_NPC(victim) : IS_NPC(victim)))
        return FALSE;

    return TRUE;
}

/*
 * True if char can see obj.
 */
bool
can_see_obj(CHAR_DATA *ch, OBJ_DATA *obj)
{
    if (!IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT))
        return TRUE;
    /*    if ( obj->item_type == ITEM_TRIGGER )
       return TRUE;  */
    if (obj->item_type == ITEM_POTION)
        return TRUE;

    if (IS_AFFECTED(ch, AFF_BLIND))
        return FALSE;

    if (obj->item_type == ITEM_LIGHT && obj->value[2] != 0)
        return TRUE;

    if (room_is_dark(ch->in_room)
        && (!IS_AFFECTED(ch, AFF_INFRARED)) && !item_has_apply(ch, ITEM_APPLY_INFRA))
        return FALSE;

    if (IS_SET(obj->extra_flags, ITEM_INVIS)
        && (!IS_AFFECTED(ch, AFF_DETECT_INVIS) && !item_has_apply(ch, ITEM_APPLY_DET_INV)))
        return FALSE;

    return TRUE;
}

/*
 * True if char can drop obj.
 */
bool
can_drop_obj(CHAR_DATA *ch, OBJ_DATA *obj)
{
    if (!IS_SET(obj->extra_flags, ITEM_NODROP))
        return TRUE;

    if (!IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL)
        return TRUE;

    return FALSE;
}

bool
can_sac_obj(CHAR_DATA *ch, OBJ_DATA *obj)
{
    if (IS_SET(obj->extra_flags, ITEM_NOSAC))
        return FALSE;
    else
        return TRUE;
}

/*
 * Return ascii name of an item type.
 */
char               *
item_type_name(OBJ_DATA *obj)
{
    char                log[MAX_STRING_LENGTH];

    switch (obj->item_type) {
        case ITEM_LIGHT:
            return "light";
        case ITEM_SCROLL:
            return "scroll";
        case ITEM_WAND:
            return "wand";
        case ITEM_STAFF:
            return "staff";
        case ITEM_BEACON:
            return "beacon";
        case ITEM_WEAPON:
            return "weapon";
        case ITEM_TREASURE:
            return "treasure";
        case ITEM_ARMOR:
            return "armor";
        case ITEM_POTION:
            return "potion";
        case ITEM_FURNITURE:
            return "furniture";
        case ITEM_TRASH:
            return "trash";
        case ITEM_CONTAINER:
            return "container";
        case ITEM_QUEST:
            return "quest";
        case ITEM_DRINK_CON:
            return "drink container";
        case ITEM_KEY:
            return "key";
        case ITEM_FOOD:
            return "food";
        case ITEM_MONEY:
            return "money";
        case ITEM_BOAT:
            return "boat";
        case ITEM_CORPSE_NPC:
            return "npc corpse";
        case ITEM_CORPSE_PC:
            return "pc corpse";
        case ITEM_FOUNTAIN:
            return "fountain";
        case ITEM_PILL:
            return "pill";
        case ITEM_BOARD:
            return "bulletin board";
        case ITEM_CLUTCH:
            return "clutch";
        case ITEM_TRIGGER:
            return "trigger";
        case ITEM_SOUL:
            return "soul";
        case ITEM_PIECE:
            return "piece";
        case ITEM_SPELL_MATRIX:
            return "spell matrix";
        case ITEM_ENCHANTMENT:
            return "enchantment";
        case ITEM_PORTAL:
            return "portal";

    }

    sprintf(log, "Item_type_name: Object: %d.  Unknown Type: %d", obj->pIndexData->vnum, obj->item_type);
    monitor_chan(log, MONITOR_OBJ);
    bug(log);
    return "(unknown)";
}

/*
 * Return ascii name of an affect location.
 */
char               *
affect_loc_name(int location)
{
    char                buf[MAX_STRING_LENGTH];

    switch (location) {
        case APPLY_NONE:
            return "none";
        case APPLY_STR:
            return "strength";
        case APPLY_DEX:
            return "dexterity";
        case APPLY_INT:
            return "intelligence";
        case APPLY_WIS:
            return "wisdom";

        case APPLY_CON:
            return "constitution";
        case APPLY_SEX:
            return "sex";
        case APPLY_CLASS:
            return "class";
        case APPLY_LEVEL:
            return "level";
        case APPLY_AGE:
            return "age";

        case APPLY_HEIGHT:
            return "height";
        case APPLY_WEIGHT:
            return "weight";

        case APPLY_MANA:
            return "mana";
        case APPLY_HIT:
            return "hp";
        case APPLY_MOVE:
            return "moves";
        case APPLY_GOLD:
            return "gold";
        case APPLY_EXP:
            return "experience";

        case APPLY_AC:
            return "armor class";
        case APPLY_HITROLL:
            return "hit roll";
        case APPLY_DAMROLL:
            return "damage roll";
        case APPLY_SAVING_PARA:
        case APPLY_SAVING_ROD:
        case APPLY_SAVING_PETRI:
        case APPLY_SAVING_BREATH:
        case APPLY_SAVING_SPELL:
            return "save vs spell";
    }

    sprintf(buf, "affect_location_name: location %d unknown.", location);
    monitor_chan(buf, MONITOR_OBJ);

    bugf("Affect_location_name: unknown location %d.", location);
    return "(unknown)";
}

char               *
raffect_bit_name(int vector)
{
    static char         rbuf[512];

    rbuf[0] = '\0';

    if (vector & ROOM_BV_NONE)
        safe_strcat(MAX_STRING_LENGTH, rbuf, " NONE");
    if (vector & ROOM_BV_SILENCE)
        safe_strcat(MAX_STRING_LENGTH, rbuf, " Silence");
    if (vector & ROOM_BV_SAFE)
        safe_strcat(MAX_STRING_LENGTH, rbuf, " Safe");
    if (vector & ROOM_BV_ENCAPS)
        safe_strcat(MAX_STRING_LENGTH, rbuf, " Seal Room");
    if (vector & ROOM_BV_HEAL_REGEN)
        safe_strcat(MAX_STRING_LENGTH, rbuf, " @@mHealing Light@@N");
    if (vector & ROOM_BV_HEAL_STEAL)
        safe_strcat(MAX_STRING_LENGTH, rbuf, " @@dWither Shadow@@N");
    if (vector & ROOM_BV_MANA_REGEN)
        safe_strcat(MAX_STRING_LENGTH, rbuf, " @@eMana Flare@@N");
    if (vector & ROOM_BV_MANA_STEAL)
        safe_strcat(MAX_STRING_LENGTH, rbuf, " @@dMana Drain@@N");
    if (vector & ROOM_BV_FIRE_RUNE)
        safe_strcat(MAX_STRING_LENGTH, rbuf, " @@eFire @@NRune");
    if (vector & ROOM_BV_FIRE_TRAP)
        safe_strcat(MAX_STRING_LENGTH, rbuf, " @@eFire @@NTrap");
    if (vector & ROOM_BV_DAMAGE_TRAP)
        safe_strcat(MAX_STRING_LENGTH, rbuf, " @@dDamage @@NTrap");
    if (vector & ROOM_BV_SHOCK_RUNE)
        safe_strcat(MAX_STRING_LENGTH, rbuf, " @@lShock @@NRune");
    if (vector & ROOM_BV_SHOCK_TRAP)
        safe_strcat(MAX_STRING_LENGTH, rbuf, " @@lShock @@NTrap");
    if (vector & ROOM_BV_HOLD)
        safe_strcat(MAX_STRING_LENGTH, rbuf, " @@rCage@@N");
    if (vector & ROOM_BV_POISON_RUNE)
        safe_strcat(MAX_STRING_LENGTH, rbuf, " @@mPoison@@N Rune");
    if (vector & ROOM_BV_WARNING_RUNE)
        safe_strcat(MAX_STRING_LENGTH, rbuf, " @@dWarning Rune@@N");
    if (vector & ROOM_BV_SOUL_NET)
        safe_strcat(MAX_STRING_LENGTH, rbuf, " @@dSoul Net@@N");
    if (vector & ROOM_BV_SMOKESCREEN)
        safe_strcat(MAX_STRING_LENGTH, rbuf, " @@dSmoke Screen@@N");
    if (vector & ROOM_BV_SMOKESCREEN_AREA)
        safe_strcat(MAX_STRING_LENGTH, rbuf, " @@dArea Smoke Screen@@N");

    return (rbuf[0] != '\0') ? rbuf + 1 : "none";

}

/*
 * Return ascii name of an affect bit vector.
 */
char               *
affect_bit_name(int vector)
{
    static char         buf[512];

    buf[0] = '\0';
    if (vector & AFF_BLIND)
        safe_strcat(MAX_STRING_LENGTH, buf, " blind");
    if (vector & AFF_INVISIBLE)
        safe_strcat(MAX_STRING_LENGTH, buf, " invisible");
    if (vector & AFF_DETECT_EVIL)
        safe_strcat(MAX_STRING_LENGTH, buf, " detect_evil");
    if (vector & AFF_DETECT_INVIS)
        safe_strcat(MAX_STRING_LENGTH, buf, " detect_invis");
    if (vector & AFF_DETECT_MAGIC)
        safe_strcat(MAX_STRING_LENGTH, buf, " detect_magic");
    if (vector & AFF_DETECT_HIDDEN)
        safe_strcat(MAX_STRING_LENGTH, buf, " detect_hidden");
    if (vector & AFF_HOLD)
        safe_strcat(MAX_STRING_LENGTH, buf, " hold");
    if (vector & AFF_SANCTUARY)
        safe_strcat(MAX_STRING_LENGTH, buf, " sanctuary");
    if (vector & AFF_FAERIE_FIRE)
        safe_strcat(MAX_STRING_LENGTH, buf, " faerie_fire");
    if (vector & AFF_INFRARED)
        safe_strcat(MAX_STRING_LENGTH, buf, " infrared");
    if (vector & AFF_CURSE)
        safe_strcat(MAX_STRING_LENGTH, buf, " curse");
    if (vector & AFF_CLOAK_FLAMING)
        safe_strcat(MAX_STRING_LENGTH, buf, " flaming");
    if (vector & AFF_POISON)
        safe_strcat(MAX_STRING_LENGTH, buf, " poison");
    if (vector & AFF_PROTECT)
        safe_strcat(MAX_STRING_LENGTH, buf, " protect");
    if (vector & AFF_PARALYSIS)
        safe_strcat(MAX_STRING_LENGTH, buf, " paralysis");
    if (vector & AFF_SLEEP)
        safe_strcat(MAX_STRING_LENGTH, buf, " sleep");
    if (vector & AFF_SNEAK)
        safe_strcat(MAX_STRING_LENGTH, buf, " sneak");
    if (vector & AFF_HIDE)
        safe_strcat(MAX_STRING_LENGTH, buf, " hide");
    if (vector & AFF_CHARM)
        safe_strcat(MAX_STRING_LENGTH, buf, " charm");
    if (vector & AFF_FLYING)
        safe_strcat(MAX_STRING_LENGTH, buf, " flying");
    if (vector & AFF_PASS_DOOR)
        safe_strcat(MAX_STRING_LENGTH, buf, " pass_door");

    if (vector & AFF_CLOAK_REFLECTION)
        safe_strcat(MAX_STRING_LENGTH, buf, " cloak:reflection");
    if (vector & AFF_CLOAK_ABSORPTION)
        safe_strcat(MAX_STRING_LENGTH, buf, " cloak:absorption");
    if (vector & AFF_CLOAK_MANA)
        safe_strcat(MAX_STRING_LENGTH, buf, " cloak:mana");

    if (vector & AFF_CLOAK_ADEPT)
        safe_strcat(MAX_STRING_LENGTH, buf, " cloak:adept");
    if (vector & AFF_CLOAK_REGEN)
        safe_strcat(MAX_STRING_LENGTH, buf, " cloak:regeneration");

    return (buf[0] != '\0') ? buf + 1 : "none";
}

/*
 * Return ascii name of extra flags vector.
 */
char               *
extra_bit_name(int extra_flags)
{
    static char         buf[512];

    buf[0] = '\0';
    if (extra_flags & ITEM_GLOW)
        safe_strcat(MAX_STRING_LENGTH, buf, " glow");
    if (extra_flags & ITEM_HUM)
        safe_strcat(MAX_STRING_LENGTH, buf, " hum");
    if (extra_flags & ITEM_NODISARM)
        safe_strcat(MAX_STRING_LENGTH, buf, " nodisarm");
    if (extra_flags & ITEM_LOCK)
        safe_strcat(MAX_STRING_LENGTH, buf, " lock");
    if (extra_flags & ITEM_EVIL)
        safe_strcat(MAX_STRING_LENGTH, buf, " evil");
    if (extra_flags & ITEM_INVIS)
        safe_strcat(MAX_STRING_LENGTH, buf, " invis");
    if (extra_flags & ITEM_MAGIC)
        safe_strcat(MAX_STRING_LENGTH, buf, " magic");
    if (extra_flags & ITEM_NODROP)
        safe_strcat(MAX_STRING_LENGTH, buf, " nodrop");
    if (extra_flags & ITEM_BLESS)
        safe_strcat(MAX_STRING_LENGTH, buf, " bless");
    if (extra_flags & ITEM_ANTI_GOOD)
        safe_strcat(MAX_STRING_LENGTH, buf, " anti-good");
    if (extra_flags & ITEM_ANTI_EVIL)
        safe_strcat(MAX_STRING_LENGTH, buf, " anti-evil");
    if (extra_flags & ITEM_ANTI_NEUTRAL)
        safe_strcat(MAX_STRING_LENGTH, buf, " anti-neutral");
    if (extra_flags & ITEM_NOREMOVE)
        safe_strcat(MAX_STRING_LENGTH, buf, " noremove");
    if (extra_flags & ITEM_INVENTORY)
        safe_strcat(MAX_STRING_LENGTH, buf, " inventory");
    if (extra_flags & ITEM_NOLOOT)
        safe_strcat(MAX_STRING_LENGTH, buf, " noloot");
    if (extra_flags & ITEM_NOSAC)
        safe_strcat(MAX_STRING_LENGTH, buf, " nosac");

    if (extra_flags & ITEM_REMORT)
        safe_strcat(MAX_STRING_LENGTH, buf, " remort");
    if (extra_flags & ITEM_ADEPT)
        safe_strcat(MAX_STRING_LENGTH, buf, " adept");
    if (extra_flags & ITEM_CLAN_EQ)
        safe_strcat(MAX_STRING_LENGTH, buf, " claneq");
    if (extra_flags & ITEM_NOSAVE)
        safe_strcat(MAX_STRING_LENGTH, buf, " nosave");

    if (extra_flags & ITEM_NO_AUCTION)
        safe_strcat(MAX_STRING_LENGTH, buf, " no_auction");
    if (extra_flags & ITEM_RARE)
        safe_strcat(MAX_STRING_LENGTH, buf, " rare");
    if (extra_flags & ITEM_UNIQUE)
        safe_strcat(MAX_STRING_LENGTH, buf, " unique");
    if (extra_flags & ITEM_TRIG_DESTROY)
        safe_strcat(MAX_STRING_LENGTH, buf, " trigger:destroy");
    if (extra_flags & ITEM_LIFESTEALER)
        safe_strcat(MAX_STRING_LENGTH, buf, " lifestealer");
    if (extra_flags & ITEM_MINDSTEALER)
        safe_strcat(MAX_STRING_LENGTH, buf, " mindstealer");
    if (extra_flags & ITEM_NODESTROY)
        safe_strcat(MAX_STRING_LENGTH, buf, " nodestroy");
    if (extra_flags & ITEM_NOSELL)
        safe_strcat(MAX_STRING_LENGTH, buf, " nosell");
    if (extra_flags & ITEM_NOSTEAL)
        safe_strcat(MAX_STRING_LENGTH, buf, " nosteal");
    if (extra_flags & ITEM_NODISPEL)
        safe_strcat(MAX_STRING_LENGTH, buf, " nodispel");

    return (buf[0] != '\0') ? buf + 1 : "none";
}

/* Return the ASCII name of a character's race
 * -- Stephen
 */

char               *
race_name(CHAR_DATA *ch)
{

    if (IS_NPC(ch))
        return ("NPC");

    if (ch->race >= MAX_RACE || ch->race < 0) {
        /* error reporting here one day... maybe */
        return ("???");
    }

    return (race_table[ch->race].race_title);
}

char               *
short_race_name(CHAR_DATA *ch)
{
    static char         buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
        sprintf(buf, "NPC");
    else if (ch->race >= MAX_RACE || ch->race < 0) {
        /* error reporting here one day... maybe */
        sprintf(buf, "unknown!");
    }
    else
        sprintf(buf, "%s", race_table[ch->race].race_name);

    return (buf);
}

bool
can_use(CHAR_DATA *ch, OBJ_DATA *obj)
{
    return (TRUE);
}

/*
 * Return names of classes which can use an object
 * -- Stephen
 */

char               *
who_can_use(OBJ_DATA *obj)
{
    return (" all classes.");
}

void
notify(char *message, int lv)
{
    /* This function sends <message>
     * to all players of level (lv) and above
     * -- Stephen
     */

    DESCRIPTOR_DATA    *d;
    char                buf[MAX_STRING_LENGTH];

    sprintf(buf, "[NOTE]: %s\n\r", message);
    for (d = first_desc; d; d = d->next)
        if ((d->connected == CON_PLAYING)
            && (d->character->level >= lv)
            && !IS_NPC(d->character)
            && !IS_SET(d->character->deaf, CHANNEL_NOTIFY))
            send_to_char(buf, d->character);
    return;
}

void
auction(char *message)
{
    DESCRIPTOR_DATA    *d;
    char                buf[MAX_STRING_LENGTH];

    sprintf(buf, "[AUCTION]: %s\n\r", message);
    for (d = first_desc; d; d = d->next)
        if ((d->connected == CON_PLAYING)
            && !IS_NPC(d->character)
            && !IS_SET(d->character->deaf, CHANNEL_AUCTION))
            send_to_char(buf, d->character);
    return;
}

void
info(char *message, int lv)
{
    /* This function sends <message>
     * to all players of level (lv) and above
     * Used mainly to send level gain, death info, etc to mortals.
     * - Stephen
     */
    DESCRIPTOR_DATA    *d;
    char                buf[MAX_STRING_LENGTH];

    for (d = first_desc; d; d = d->next)
        if ((d->connected == CON_PLAYING)
            && (d->character->level >= lv)
            && !IS_NPC(d->character)
            && !IS_SET(d->character->deaf, CHANNEL_INFO)) {
            sprintf(buf, "%s[INFO]: %s%s\n\r", colour_string(d->character, "info"), message, colour_string(d->character, "normal"));
            send_to_char(buf, d->character);
        }
    return;
}

void
challenge(char *message)
{
    DESCRIPTOR_DATA    *d;
    char                buf[MSL];

    for (d = first_desc; d; d = d->next)
        if (d->connected == CON_PLAYING && d->character && !IS_NPC(d->character) && !IS_SET(d->character->deaf2, CHANNEL2_CHALLENGE)) {
            sprintf(buf, "%s@@d[@@gCHALLENGE@@d]: @@g%s@@N\n\r", colour_string(d->character, "info"), message);
            send_to_char(buf, d->character);
        }

    return;
}

void
challengef(char *fmt, ...)
{
    char                buf[MSL];
    va_list             args;

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    challenge(buf);
}

void
auctionf(char *fmt, ...)
{
    char                buf[MSL];
    va_list             args;

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    auction(buf);
}

void
gain(char *message)
{
    DESCRIPTOR_DATA    *d;
    char                buf[MSL];

    for (d = first_desc; d; d = d->next)
        if (d->connected == CON_PLAYING && d->character && !IS_NPC(d->character) && !IS_SET(d->character->deaf2, CHANNEL2_GAIN)) {
            sprintf(buf, "%s@@y[@@gGAIN@@y]@@g: %s@@N\n\r", colour_string(d->character, "info"), message);
            send_to_char(buf, d->character);
        }

    return;
}

void
gainf(char *fmt, ...)
{
    char                buf[MSL];
    va_list             args;

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    gain(buf);
}

void
arena(char *message)
{
    DESCRIPTOR_DATA    *d;
    char                buf[MSL];

    for (d = first_desc; d; d = d->next)
        if (d->connected == CON_PLAYING && d->character && !IS_NPC(d->character) && !IS_SET(d->character->deaf2, CHANNEL2_ARENA)) {
            sprintf(buf, "%s@@y[@@gARENA@@y]@@g: %s@@N\n\r", colour_string(d->character, "info"), message);
            send_to_char(buf, d->character);
        }

    return;
}

void
arenaf(CHAR_DATA *ch, char *fmt, ...)
{
    char                buf[MSL];
    CHAR_DATA           *vch;
    va_list             args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (ch->in_room && IS_SET(ch->in_room->room_flags, ROOM_ARENA))
        arena(buf);
    else if (ch->last_room && IS_SET(ch->last_room->room_flags, ROOM_ARENA))
        arena(buf);
    else if (ch->in_room) {
        for (vch = first_player; vch != NULL; vch = vch->next_player) {
            if (!IS_SET(vch->deaf2, CHANNEL2_ARENA) &&
                (   (ch->in_room && IS_SET(ch->in_room->room_flags, ROOM_ARENA2) && vch->in_room->area == ch->in_room->area)
                 || (ch->last_room && IS_SET(ch->last_room->room_flags, ROOM_ARENA2) && vch->in_room->area == ch->last_room->area)
                ))
                sendf(vch, "@@N@@y[@@WPRIVATE @@gARENA@@y]@@g: %s@@N\n\r", buf);
        }
    }
}

void
log_chan(const char *message, int lv)
{
    /* Used to send messages to Immortals.  
     * Level is used to determine WHO gets the message... */
    DESCRIPTOR_DATA    *d;
    char                buf[MAX_STRING_LENGTH];

    sprintf(buf, "[LOG]: %s\n\r", message);
    for (d = first_desc; d; d = d->next)
        if ((d->connected == CON_PLAYING)
            && (get_trust(d->character) == MAX_LEVEL)
            && (!IS_NPC(d->character))
            && (d->character->level >= lv)
            && (!IS_SET(d->character->deaf, CHANNEL_LOG)))
            send_to_char(buf, d->character);
    return;
}

bool
item_has_apply(CHAR_DATA *ch, int bit)
{
    /* Used to see if ch is HOLDING any object(s) with the specified
     * ITEM_APPLY bit set.  
     * -S-
     */

    OBJ_DATA           *obj;

    for (obj = ch->first_carry; obj != NULL; obj = obj->next_in_carry_list)
        if (IS_SET(obj->item_apply, bit) && obj->wear_loc != WEAR_NONE)
            return TRUE;

    return FALSE;
}

/* This is for immrotal authorized skills. Enables imms to set which skillks lower imms may use. handy for abuse control --Flar 
 */
bool
authorized(CHAR_DATA *ch, char *skllnm)
{

    /*    char buf [ MAX_STRING_LENGTH ]; */

    if ((!IS_NPC(ch) && str_infix(skllnm, ch->pcdata->immskll))
        || IS_NPC(ch)) {
        /*        sprintf( buf, "Sorry, you are not authorized to use %s.\n\r", skllnm );
           send_to_char( buf, ch );

           make it say "Huh?" instead for interpret() :P
         */
        return FALSE;
    }

    return TRUE;

}

/* A Function to "center" text, and return a string with
 * the required amount of white space either side of the
 * original text.  --Stephen
 */

char               *
center_text(char *text, int width)
{
    /* This could do with a LOT of improvement sometime! */
    /* Improvements done.. -- Altrag */
    static char         foo[MAX_STRING_LENGTH];
    int                 len, diff;

    /* Don't bother with strings too long.. */
    if ((len = my_strlen(text)) >= width)
        return text;

    diff = strlen(text) - len;

    /* Add the spaces and terminate string */
    memset(foo, ' ', width + diff);
    foo[width + diff] = '\0';

    /* Copy old string over */
    memcpy(&foo[(width - len) / 2], text, len + diff);

    return foo;
}

bool is_shielded(CHAR_DATA *ch, int index)
{
    SHIELD_DATA *shield;

    for (shield = ch->first_shield; shield != NULL; shield = shield->next)
        if (shield->index == index)
            return TRUE;

    return FALSE;
}

void
set_stun(CHAR_DATA *victim, int stunTime)
{
    /* Sets the victim's wait_state and position
       -Damane-        4/26/96 */
    if (victim->position != POS_SLEEPING)
        victim->position = POS_STUNNED;

    victim->stunTimer = stunTime;
    return;
}

/* return how many levels of nesting a container has. */
int
deepest_nest(OBJ_DATA *obj, int iNestMax)
{
    OBJ_DATA *myobj, *inobj;
    int c = 0;

    if (obj->first_in_carry_list == NULL)
        return 0;

    for (myobj = obj->first_in_carry_list; myobj != NULL; myobj = myobj->next_in_carry_list) {
        for (c = 0, inobj = myobj->in_obj; inobj != NULL; inobj = inobj->in_obj)
            c++;

        if (c > iNestMax)
            iNestMax = c;

        if (myobj->first_in_carry_list) {
            c = deepest_nest(myobj, iNestMax);

            if (c > iNestMax)
                iNestMax = c;
        }
    }

    return iNestMax;
}

bool
is_charmie_of(CHAR_DATA *ch, CHAR_DATA *charmie)
{
    if (!IS_NPC(charmie) || IS_NPC(ch))
        return FALSE;

    if (!IS_AFFECTED(charmie, AFF_CHARM))
        return FALSE;

    if (charmie->master != ch)
        return FALSE;

    return TRUE;
}

char               *
get_path_step(char *path, int step)
{
    static char         out[MSL];
    char               *p_out = out;
    char               *p_path = path;
    int                 cnt = 0;

    out[0] = '\0';

    while (*p_path) {
        switch (*p_path) {
            case 'n':
            case 'N':
            case 'e':
            case 'E':
            case 's':
            case 'S':
            case 'w':
            case 'W':
            case 'u':
            case 'U':
            case 'd':
            case 'D':
            case '.':
                if (cnt == step) {
                    *p_out++ = *p_path;
                    *p_out = '\0';
                    return out;
                }

                cnt++;
                break;
            case '(':
                if (cnt == step) {
                    p_path++;
                    while (*p_path && *p_path != ')')
                        *p_out++ = *p_path++;

                    *p_out = '\0';
                    return out;
                }
                else
                    while (*p_path && *p_path++ != ')');

                cnt++;
                continue;
            default:
                cnt++;
        }

        p_path++;
    }

    return NULL;
}

void join_arena(CHAR_DATA *ch)
{
    AFFECT_DATA *af;
    AFFECT_DATA *naf;

    if (IS_NPC(ch))
        return;

    ch->pcdata->arena_save_hp           = ch->hit;
    ch->pcdata->arena_save_mana         = ch->mana;
    ch->pcdata->arena_save_move         = ch->move;
    ch->pcdata->arena_save_energy       = ch->energy;
    ch->pcdata->arena_save_first_affect = NULL;
    ch->pcdata->arena_save_last_affect  = NULL;
    ch->pcdata->arena_save_room         = ch->last_room;

    for (af = ch->first_affect; af != NULL; af = af->next) {
        if (af->type == gsn_shield_fire
            || af->type == gsn_shield_ice
            || af->type == gsn_shield_shock
            || af->type == gsn_shield_demon
            )
            continue;

        GET_FREE(naf, affect_free);
        naf->type      = af->type;
        naf->duration  = af->duration;
        naf->location  = af->location;
        naf->modifier  = af->modifier;
        naf->bitvector = af->bitvector;
        naf->caster    = af->caster;
        naf->level     = af->level;
        naf->save      = af->save;
        LINK(naf, ch->pcdata->arena_save_first_affect, ch->pcdata->arena_save_last_affect, next, prev);
    }

    if (!IS_IMMORTAL(ch)) {
        send_to_char("@@eReminder@@g: Items you consume in the arena will save, it is like any other room. See @@Whelp arena@@g for more information.@@N\n\r", ch);
        arenaf(ch, "%s enters %s!", ch->short_descr, ch->in_room->name);
    }

    ch->pcdata->in_arena = TRUE;
    return;
}

void leave_arena(CHAR_DATA *ch, bool alive)
{
    AFFECT_DATA *af;
    OBJ_DATA    *obj;
    CHAR_DATA   *charmie;
    int         vnum = 3168;

    if (IS_NPC(ch))
        return;

    ch->hit  = UMIN(ch->pcdata->arena_save_hp,   ch->max_hit);
    ch->mana = UMIN(ch->pcdata->arena_save_mana, ch->max_mana);
    ch->move = UMIN(ch->pcdata->arena_save_move, ch->max_move);
    ch->energy = UMIN(ch->pcdata->arena_save_energy, ch->max_energy);
    ch->energy_wait_count = ch->energy_wait;

    while (ch->first_affect)
        affect_remove(ch, ch->first_affect);

    ch->affected_by = 0;

    for (af = ch->pcdata->arena_save_first_affect; af != NULL; af = af->next)
        affect_to_char(ch, af);

    while (ch->pcdata->arena_save_first_affect) {
        af = ch->pcdata->arena_save_first_affect;
        UNLINK(af, ch->pcdata->arena_save_first_affect, ch->pcdata->arena_save_last_affect, next, prev);
        PUT_FREE(af, affect_free);
    }

    for (obj = ch->first_carry; obj; obj = obj->next_in_carry_list)
        if (IS_SET(obj->item_apply, ITEM_APPLY_ARENAHEATED))
            REMOVE_BIT(obj->item_apply, ITEM_APPLY_ARENAHEATED);

    if (alive && !IS_IMMORTAL(ch) && ch->last_room)
        arenaf(ch, "%s leaves %s!", ch->short_descr, ch->last_room->name);

    ch->pcdata->in_arena = FALSE;

    if (!alive) {
        ch->position = POS_STANDING;
        ch->stunTimer = 0;
        char_from_room(ch);

        if (ch->pcdata->arena_save_room && ch->pcdata->arena_save_room->is_free == FALSE)
            char_to_room(ch, ch->pcdata->arena_save_room);
        else if (get_room_index(vnum))
            char_to_room(ch, get_room_index(vnum));
        else
            char_to_room(ch, get_room_index(2));

        do_look(ch, "auto");
    }

    for (charmie = first_char; charmie; charmie = charmie->next)
        if (   IS_NPC(charmie)
            && IS_AFFECTED(charmie, AFF_CHARM)
            && charmie->master == ch
            && charmie->in_room
            && IS_SET(charmie->in_room->room_flags, ROOM_ARENA)) {
            char_from_room(charmie);

            if (ch->pcdata->arena_save_room && ch->pcdata->arena_save_room->is_free == FALSE)
                char_to_room(charmie, ch->pcdata->arena_save_room);
            else if (get_room_index(vnum))
                char_to_room(charmie, get_room_index(vnum));
            else
                char_to_room(charmie, get_room_index(2));
        }

    return;
}
