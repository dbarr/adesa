
/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvements copyright (C) 1992, 1993 by Michael         *
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
#include <stdio.h>
#include <stdlib.h>                /* For div_t, div() */
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include "merc.h"
#include "tables.h"
#include "duel.h"

IDSTRING(rcsid, "$Id: fight.c,v 1.139 2005/05/28 16:00:11 dave Exp $");

extern POL_DATA     politics_data;
extern CHAR_DATA   *quest_target;
extern CHAR_DATA   *quest_mob;

extern void abort_writing args((CHAR_DATA *ch));
extern void say_spell     args((CHAR_DATA *ch, int sn));

/*
 * Local functions.
 */
bool check_dodge    args((CHAR_DATA *ch, CHAR_DATA *victim));
void check_killer   args((CHAR_DATA *ch, CHAR_DATA *victim));
bool check_parry    args((CHAR_DATA *ch, CHAR_DATA *victim));
bool check_skills   args((CHAR_DATA *ch, CHAR_DATA *victim));
void dam_message    args((CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt));
void death_message  args((CHAR_DATA *ch, CHAR_DATA *victim, int dt, int max_dt));
void death_cry      args((CHAR_DATA *ch));
void group_gain     args((CHAR_DATA *ch, CHAR_DATA *victim));

void make_corpse    args((CHAR_DATA *ch, char *argument));
void one_hit        args((CHAR_DATA *ch, CHAR_DATA *victim, int dt));
void raw_kill       args((CHAR_DATA *victim, char *argument));
void set_fighting   args((CHAR_DATA *ch, CHAR_DATA *victim, bool check));
void disarm         args((CHAR_DATA *ch, CHAR_DATA *victim));
void trip           args((CHAR_DATA *ch, CHAR_DATA *victim));
void check_adrenaline args((CHAR_DATA *ch, sh_int damage));
void obj_damage     args((OBJ_DATA *obj, CHAR_DATA *victim, int dam));
bool magi_check     args((CHAR_DATA *ch, CHAR_DATA *victim));

bool
magi_check(CHAR_DATA *ch, CHAR_DATA *victim)
{
    if (!ch->pcdata || IS_NPC(ch) || !IS_NPC(victim))
        return FALSE;

    if (!IS_SET(ch->config, PLR_PACIFIST))
        return FALSE;

    if (IS_AFFECTED(victim, AFF_CHARM))
        return TRUE;

    if (victim->pIndexData) {
        switch (victim->pIndexData->vnum) {
            case MOB_VNUM_WATERELEM:
            case MOB_VNUM_SKELETON:
            case MOB_VNUM_FIREELEM:
            case MOB_VNUM_EARTHELEM:
            case MOB_VNUM_IRON_GOLEM:
            case MOB_VNUM_DIAMOND_GOLEM:
            case MOB_VNUM_SOUL_THIEF:
            case MOB_VNUM_HOLY_AVENGER:
            case MOB_VNUM_PEGASUS:
            case MOB_VNUM_NIGHTMARE:
            case MOB_VNUM_ELEM_BEAST:
            case MOB_VNUM_INT_DEVOURER:
            case MOB_VNUM_SHADOW_HOUND:
            case MOB_VNUM_SHADOWDRAGON:
            case MOB_VNUM_ZOMBIE:
                return TRUE;
                break;
            default:
                return FALSE;
                break;
        }
    }

    return FALSE;
}

/*
 * Control the fights going on.
 * Called periodically by update_handler.
 */
void
violence_update(void)
{
    CHAR_DATA          *ch;

    /*   CHAR_DATA *ch_next;   */
    CHAR_DATA          *victim;
    CHAR_DATA          *rch;
    CHAR_DATA          *rch_next;

    /*   CHAR_DATA *check_char;   */
    extern CHAR_DATA   *violence_marker;
    CHAR_DATA          *marker;
    bool                has_cast = FALSE;

    /*
     * Create dummy object and add to end of list.  This object is
     * only a marker, and will not actually be processed by this
     * routine.
     */
    GET_FREE(marker, char_free);
    LINK(marker, first_char, last_char, next, prev);
    violence_marker = marker;

    /*
     * Repeatedly remove char from front of list, add to tail, and process
     * until the marker is at the head of the list.  That will indicate all
     * chars have been processed.
     */

    while ((ch = first_char) != marker) {

        /* For stunning during combat
           -Damane-    4/26/96 */

        UNLINK(ch, first_char, last_char, next, prev);
        LINK(ch, first_char, last_char, next, prev);

        if (ch->position == POS_STUNNED) {
            if (ch->wait > 0) {
                ch->wait -= 1;
                continue;
            }
            else {
                ch->position = POS_STANDING;
                send_to_char("You are no longer stunned.\n\r", ch);
                continue;
            }
        }
        has_cast = FALSE;

        if (   IS_NPC(ch)
            && ch->fighting != NULL
            && !IS_NPC(ch->fighting)
            && IS_SET(ch->act, ACT_RAND_TARGET)
            && ch->in_room != NULL) {
            CHAR_DATA *fch;
            int tot = 0;
            int cnt;

            for (fch = ch->in_room->first_person; fch != NULL; fch = fch->next_in_room)
                if (!IS_NPC(fch) && fch->fighting == ch)
                    tot++;

            if (tot > 1 && number_percent() < 50) {
                /* if two or more players are fighting ch, pick a random
                 * player for ch to fight. 50% chance each round that
                 * we will attempt to change who ch is fighting.
                 */
                cnt = number_range(1, tot);
                tot = 0;

                for (fch = ch->in_room->first_person; fch != NULL; fch = fch->next_in_room)
                    if (!IS_NPC(fch) && fch->fighting == ch && ++tot == cnt) {
                        ch->fighting = fch;
                        break;
                    }
            }
        }

        /* slight damage for players in a speeded stance, simulates fatigue */

        if (!IS_NPC(ch)
            && (stance_app[ch->stance].speed_mod > 1) && ch->stance != STANCE_AMBUSH) {
            ch->hit = UMAX(10, ch->hit - number_range(get_pseudo_level(ch) * 5 / 1000, get_pseudo_level(ch) * 10 / 1000));
        }

        /*
         *  for when wolves can't cast normal spells, 
         *  increase regen rate mucho to compensate.  
         *  let's make it a skill instead..hehehe
         *
         if ( ch->fighting != NULL )
         ch->hit = ( UMIN( ch->max_hit, ( ch->hit + ( number_range( (20 * ch->max_hit /100) , 

         */

        /* Heated armor damage :) ZEN  */

        if (ch->hit > 0 && ch->in_room != NULL && get_room_index(ch->in_room->vnum) != NULL && (item_has_apply(ch, ITEM_APPLY_HEATED) || item_has_apply(ch, ITEM_APPLY_ARENAHEATED))) {

            OBJ_DATA           *heated_item;
            int                 heat_damage = 0;
            bool                isdead = FALSE;

            for (heated_item = ch->first_carry; heated_item != NULL; heated_item = heated_item->next_in_carry_list) {
                if ((    IS_SET(heated_item->item_apply, ITEM_APPLY_HEATED)
                     || (!IS_NPC(ch) && IS_SET(heated_item->item_apply, ITEM_APPLY_ARENAHEATED) && ch->pcdata->in_arena))
                    && heated_item->wear_loc != WEAR_NONE) {
                    heat_damage = heated_item->level;
                    if (IS_SET(heated_item->extra_flags, ITEM_REMORT))
                        heat_damage += 80;
                    if (IS_SET(heated_item->extra_flags, ITEM_ADEPT))
                        heat_damage += 120;

                    act("@@W   $p@@N you are wearing is @@eBURNING@@N you!!!", ch, heated_item, NULL, TO_CHAR);
                    act("@@W   $p worn by $n is @@eBURNING@@N!!!", ch, heated_item, NULL, TO_ROOM);

                    if (!IS_NPC(ch) && is_in_pk(ch) && ch->hit - heat_damage <= 0)
                        ;
                    else
                        obj_damage(heated_item, ch, heat_damage);

                    if (!ch || ch->is_free == TRUE || ch->position == POS_DEAD) {
                        isdead = TRUE;
                        break;
                    }

                    if (IS_NPC(ch))
                        do_remove(ch, heated_item->name);

                }
            }
            if (isdead)
                continue;

        }

        /* heated damage for duels */
        if (!IS_NPC(ch) && is_in_duel(ch, DUEL_STAGE_GO)) {
            DUEL_PLAYER_DATA   *player = find_duel_player(ch);
            DUEL_OBJ_DATA      *dobj, *dobj_next;
            OBJ_DATA           *mobj;
            int                 heat_damage = 0;

            if (ch->hit > 0 && player && player->first_obj) {
                for (dobj = player->first_obj; dobj != NULL; dobj = dobj_next) {
                    dobj_next = dobj->next;
                    mobj = dobj->obj;

                    if (mobj->wear_loc != WEAR_NONE) {
                        heat_damage = mobj->level;
                        if (IS_SET(mobj->extra_flags, ITEM_REMORT))
                            heat_damage += 80;
                        if (IS_SET(mobj->extra_flags, ITEM_ADEPT))
                            heat_damage += 120;

                        act("@@W   $p@@N you are wearing is @@eBURNING@@N you!!!", ch, mobj, NULL, TO_CHAR);
                        act("@@W   $p worn by $n is @@eBURNING@@N!!!", ch, mobj, NULL, TO_ROOM);
                        obj_damage(mobj, player->ch, heat_damage);
                        if (find_duel_player(ch) == NULL)
                            break;
                    }
                }
            }
        }

        if (ch->stunTimer > 0) {
            ch->stunTimer--;
            continue;
        }

        /*  heal check for  solos ( mob )..   Zen  */

        if ((ch->is_free == FALSE)
            && (IS_NPC(ch))
            && IS_SET(ch->act, ACT_SOLO)
            && ch->hit > 0) {
            if (ch->hit < ch->max_hit * 3 / 4 && ch->mana > mana_cost(ch, gsn_heal)) {
                do_cast(ch, "heal self");
                has_cast = TRUE;
            }
        }
        else
         if (IS_NPC(ch) && ch->hit < 0) {
            ch->position = POS_DEAD;
            if (ch->fighting == NULL)
                if (ch->in_room != NULL)
                    act("Suddenly, $n is enveloped in a @@mBeam of light@@N, and is gone!", ch, NULL, NULL, TO_ROOM);
            stop_fighting(ch, TRUE);
            extract_char(ch, TRUE);
            continue;
        }

        if ((ch->is_free == FALSE)
            && (IS_NPC(ch))
            && (!IS_SET(ch->def, DEF_NONE))
            && ch->hit > 0) {
            if (ch->hit < ch->max_hit * 2 / 3) {
                if (IS_SET(ch->def, DEF_CURE_LIGHT)) {
                    if (ch->mana > mana_cost(ch, gsn_cure_light)) {
                        do_cast(ch, "\'cure light\' self");
                        has_cast = TRUE;
                    }
                }
                else if (IS_SET(ch->def, DEF_CURE_SERIOUS)) {
                    if (ch->mana > mana_cost(ch, gsn_cure_serious)) {
                        do_cast(ch, "\'cure serious\' self");
                        has_cast = TRUE;
                    }
                }
                else if (IS_SET(ch->def, DEF_CURE_CRITIC)) {
                    if (ch->mana > mana_cost(ch, gsn_cure_critical)) {
                        do_cast(ch, "\'cure critical\' self");
                        has_cast = TRUE;
                    }
                }
                else if (IS_SET(ch->def, DEF_CURE_HEAL)) {
                    if (ch->mana > mana_cost(ch, gsn_heal)) {
                        do_cast(ch, "heal self");
                        has_cast = TRUE;
                    }
                }
            }
        }
        else if (IS_NPC(ch) && ch->hit < 0) {
            ch->position = POS_DEAD;
            if (ch->fighting == NULL)
                if (ch->in_room != NULL)
                    act("Suddenly, $n is enveloped in a @@mBeam of light@@N, and is gone!", ch, NULL, NULL, TO_ROOM);
            extract_char(ch, TRUE);
            continue;
        }

        /* Divine Intervention Defensive Flag */
        if (   ch->is_free == FALSE
            && IS_NPC(ch)
            && IS_SET(ch->def, DEF_CURE_DIVINE)
            && ch->hit > 0
            && ch->hit < ch->max_hit) {
            say_spell(ch, gsn_divine_intervention);
            spell_divine_intervention(gsn_divine_intervention, 80, ch, ch, NULL);
            has_cast = TRUE;
        }

        if ((ch->is_free == FALSE)
            && (IS_NPC(ch))
            && (!IS_SET(ch->def, DEF_NONE))
            && (ch->hit > 0)
            && (ch->fighting == NULL || IS_SET(ch->def, DEF_SHIELD_RECAST))) {

            if (   IS_SET(ch->def, DEF_SHIELD_FIRE)
                && !is_affected(ch, gsn_shield_fire)
                && ch->mana > mana_cost(ch, gsn_shield_fire)) {
                do_cast(ch, "fireshield");
                has_cast = TRUE;
            }
            if (   IS_SET(ch->def, DEF_SHIELD_ICE)
                && !is_affected(ch, gsn_shield_ice)
                && ch->mana > mana_cost(ch, gsn_shield_ice)) {
                do_cast(ch, "iceshield");
                has_cast = TRUE;
            }
            if (   IS_SET(ch->def, DEF_SHIELD_SHOCK)
                && !is_affected(ch, gsn_shield_shock)
                && ch->mana > mana_cost(ch, gsn_shield_shock)) {
                do_cast(ch, "shockshield");
                has_cast = TRUE;
            }
            if (   IS_SET(ch->def, DEF_SHIELD_DEMON)
                && !is_affected(ch, gsn_shield_demon)
                && ch->mana > mana_cost(ch, gsn_shield_demon)) {
                do_cast(ch, "demonshield");
                has_cast = TRUE;
            }

            /* Re-cast defensive cloaks. Make these cloaks not expire on their
             * own, so mimic the casting */
            if (   IS_SET(ch->def, DEF_CLOAK_ABSORB)
                && !IS_AFFECTED(ch, AFF_CLOAK_ABSORPTION)
                && ch->mana > mana_cost(ch, gsn_cloak_absorb)) {
                say_spell(ch, gsn_cloak_absorb);
                SET_BIT(ch->affected_by, AFF_CLOAK_ABSORPTION);
                ch->mana = UMAX(0, ch->mana - mana_cost(ch, gsn_cloak_absorb));
                has_cast = TRUE;
            }

            if (   IS_SET(ch->def, DEF_CLOAK_REFLECT)
                && !IS_AFFECTED(ch, AFF_CLOAK_REFLECTION)
                && ch->mana > mana_cost(ch, gsn_cloak_reflect)) {
                say_spell(ch, gsn_cloak_reflect);
                SET_BIT(ch->affected_by, AFF_CLOAK_REFLECTION);
                ch->mana = UMAX(0, ch->mana - mana_cost(ch, gsn_cloak_reflect));
                has_cast = TRUE;
            }

            if (   IS_SET(ch->def, DEF_CLOAK_FLAMING)
                && !IS_AFFECTED(ch, AFF_CLOAK_FLAMING)
                && ch->mana > mana_cost(ch, gsn_cloak_flaming)) {
                say_spell(ch, gsn_cloak_flaming);
                SET_BIT(ch->affected_by, AFF_CLOAK_FLAMING);
                ch->mana = UMAX(0, ch->mana - mana_cost(ch, gsn_cloak_flaming));
                has_cast = TRUE;
            }
        }

        /* Offensive spell handler, only use when actually fighting.. */

        if ((IS_NPC(ch))
            && (ch->is_free == FALSE)
            && (ch->cast > 1)
            && (!has_cast)
            && (ch->position > POS_RESTING)
            && (ch->fighting != NULL)
            && (ch->fighting->is_free != TRUE)
            && (ch->in_room != NULL)
            && (ch->hit > 1)
            && (ch->position == POS_FIGHTING))
        {
            sh_int              cast_frequency;
            sh_int              index;

            cast_frequency = get_pseudo_level(ch) / 2;    /* maybe set in olc later? */
            if ((number_range(0, 99) < cast_frequency)
                && (ch->mana >= (40 * ch->max_mana / 100))) {
                for (index = 1; index < 32; index++) {
                    if ((IS_SET(ch->cast, (1 << index)))
                        && (number_range(0, 99) < (index * 3 + number_range(0, 25)))
                        && (ch->mana > mana_cost(ch, skill_lookup(rev_table_lookup(tab_cast_name, (1 << index)))))) {
                        char                cast_name[MSL];

                        /*            char   mon_message[MSL];   */
                        sprintf(cast_name, "%s %s", rev_table_lookup(tab_cast_name, (1 << index)), ch->fighting->name);
                        do_cast(ch, cast_name);
                        /*           sprintf( mon_message, "casting %s, index == %d", cast_name, ( 1 << index ) );
                           monitor_chan( mon_message, MONITOR_COMBAT );    */
                        has_cast = TRUE;
                        break;
                    }
                }
            }

        }
        /*
         * Hunting mobs.
         * -S- Mod: use flags to work out what to do.... 
         */
        if (IS_NPC(ch)
            && ch->fighting == NULL && IS_AWAKE(ch))
        {
            mob_hunt(ch);
            continue;
        }
        if ((victim = ch->fighting) == NULL || ch->in_room == NULL) {
            continue;
        }

        if ((IS_AWAKE(ch))
            && (ch->is_free == FALSE)
            && (victim->is_free == FALSE)
            && (ch->in_room != NULL)
            && (victim->in_room != NULL)
            && (ch->in_room == victim->in_room))
            multi_hit(ch, victim, TYPE_UNDEFINED);
        else
            stop_fighting(ch, FALSE);

        if ((victim = ch->fighting) == NULL)
            continue;

        mprog_hitprcnt_trigger(ch, victim);
        mprog_fight_trigger(ch, victim);

        /*
         * Fun for the whole family!   RCH is a non-fighting mob
         */
        if (IS_NPC(victim) && (get_pseudo_level(victim) > 15)) {
            for (rch = ch->in_room->first_person; rch != NULL; rch = rch_next) {
                rch_next = rch->next_in_room;
                if (!IS_NPC(rch))
                    continue;

                if (IS_SET(rch->act, ACT_NOASSIST))
                    continue;

                if (IS_AWAKE(rch) && rch->fighting == NULL) {
                    /*
                     * NPC's assist NPC's of same type or 45% chance regardless.
                     */
                    if (!IS_AFFECTED(rch, AFF_CHARM) && rch != quest_mob && rch != quest_target) {
                        if ((rch->pIndexData == victim->pIndexData)    /* is it the same as a target here?  */
                            ||((number_percent() < 20)
                                && (abs(get_pseudo_level(rch) - get_pseudo_level(victim)) < 35))) {
                            CHAR_DATA          *vch;
                            CHAR_DATA          *target;
                            int                 number;

                            target = NULL;
                            number = 0;

                            /* vch is the target of the lazy mob...a player */
                            for (vch = ch->in_room->first_person; vch; vch = vch->next_in_room) {
                                if ((can_see(rch, vch))
                                    && (!IS_NPC(vch))) {
                                    target = vch;
                                    number++;
                                }
                            }

                            if (target != NULL) {
                                if (abs(target->level - rch->level) < 40) {
                                    act("$n screams 'BANZAI!! $N must be assisted!!'", rch, NULL, victim, TO_ROOM);
                                    multi_hit(rch, target, TYPE_UNDEFINED);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    /*
     * All objects have been processed.  Remove the marker object and
     * put it back on the free list.
     */
    UNLINK(marker, first_char, last_char, next, prev);
    PUT_FREE(marker, char_free);
    violence_marker = NULL;
    return;
}

/*
 * Do one group of attacks.
 */
void
multi_hit(CHAR_DATA *ch, CHAR_DATA *victim, int dt)
{
    int                 chance;
    OBJ_DATA           *wield1 = NULL;
    OBJ_DATA           *wield2 = NULL;
    int                 dual_chance = 0;
    bool                multi_hit = FALSE;

    if (ch->position == POS_RIDING)
        ch->position = POS_STANDING;

    one_hit(ch, victim, dt);

    if (ch->fighting != victim)
        return;

    if ((((wield1 = get_eq_char(ch, WEAR_WIELD)) != NULL)
            && (wield1->item_type == ITEM_WEAPON))
        && (((wield2 = get_eq_char(ch, WEAR_WIELD_2)) != NULL)
            && (wield2->item_type == ITEM_WEAPON)))
        dual_chance = 15;

    chance = IS_NPC(ch)
        ? (IS_SET(ch->skills, MOB_SECOND) ? 80 : 0)
        : ((ch->pcdata->learned[gsn_second_attack] / 2) - (dex_app[get_curr_dex(ch)].defensive / 3)
        + (stance_app[ch->stance].speed_mod * get_pseudo_level(ch) / 10)
        + (dual_chance));
    multi_hit = (IS_NPC(ch) ? FALSE : (ch->pcdata->learned[gsn_second_attack] > 0));

    if ((number_percent() < chance)
        && (IS_NPC(ch) || multi_hit)) {
        one_hit(ch, victim, dt);
        if (ch->fighting != victim)
            return;
    }

    chance = IS_NPC(ch)
        ? (IS_SET(ch->skills, MOB_THIRD) ? 70 : 0)
        : ((ch->pcdata->learned[gsn_third_attack] / 4) - (dex_app[get_curr_dex(ch)].defensive / 3)
        + (stance_app[ch->stance].speed_mod * get_pseudo_level(ch) / 10)
        + (dual_chance));
    multi_hit = (IS_NPC(ch) ? FALSE : (ch->pcdata->learned[gsn_third_attack] > 0));

    if ((number_percent() < chance)
        && (IS_NPC(ch) || multi_hit)) {
        one_hit(ch, victim, dt);
        if (ch->fighting != victim)
            return;
    }

    chance = IS_NPC(ch)
        ? (IS_SET(ch->skills, MOB_FOURTH) ? 60 : 0)
        : ((ch->pcdata->learned[gsn_fourth_attack] / 7) - (dex_app[get_curr_dex(ch)].defensive / 3)
        + (stance_app[ch->stance].speed_mod * get_pseudo_level(ch) / 10)
        + (dual_chance));
    multi_hit = (IS_NPC(ch) ? FALSE : (ch->pcdata->learned[gsn_fourth_attack] > 0));

    if ((number_percent() < chance)
        && (IS_NPC(ch) || multi_hit)) {
        one_hit(ch, victim, dt);
        if (ch->fighting != victim)
            return;
    }

    chance = IS_NPC(ch)
        ? (IS_SET(ch->skills, MOB_FIFTH) ? 50 : 0)
        : ((ch->pcdata->learned[gsn_fifth_attack] / 6) - (dex_app[get_curr_dex(ch)].defensive / 3)
        + (stance_app[ch->stance].speed_mod * get_pseudo_level(ch) / 10)
        + (dual_chance));
    multi_hit = (IS_NPC(ch) ? FALSE : (ch->pcdata->learned[gsn_fifth_attack] > 0));

    if ((number_percent() < chance)
        && (IS_NPC(ch) || multi_hit)) {
        one_hit(ch, victim, dt);
        if (ch->fighting != victim)
            return;
    }

    chance = IS_NPC(ch)
        ? (IS_SET(ch->skills, MOB_SIXTH) ? 30 : 0)
        : ((ch->pcdata->learned[gsn_sixth_attack] / 7) - (dex_app[get_curr_dex(ch)].defensive / 3)
        + (stance_app[ch->stance].speed_mod * get_pseudo_level(ch) / 10)
        + (dual_chance));
    multi_hit = (IS_NPC(ch) ? FALSE : (ch->pcdata->learned[gsn_sixth_attack] > 0));

    if ((number_percent() < chance)
        && (IS_NPC(ch) || multi_hit)) {
        one_hit(ch, victim, dt);
        if (ch->fighting != victim)
            return;
    }
    if (!IS_NPC(ch) && IS_SET(race_table[ch->race].race_flags, RACE_MOD_TAIL)) {
        if (number_percent() < 25)
            one_hit(ch, victim, TYPE_HIT + 13);
    }

    if (!IS_NPC(ch)
        && ((ch->stance == STANCE_AMBUSH)
            || (ch->stance == STANCE_AC_BEST))) {
        send_to_char("You step out of the shadows.\n\r", ch);
        ch->stance = STANCE_WARRIOR;
        ch->stance_ac_mod = 0;
        ch->stance_dr_mod = 0;
        ch->stance_hr_mod = 0;
        act("$n steps out of the Shadows!", ch, NULL, NULL, TO_ROOM);
    }
    return;
}

#define ZERO_RATIO 20
#define ONE_STEPS 100l
#define MAX_IX 120
#define MAX_DAM_MOD 1.4

/*
 * Hit one guy once.
 */
void
one_hit(CHAR_DATA *ch, CHAR_DATA *victim, int dt)
{
    OBJ_DATA           *wield, *wield2;
    OBJ_DATA           *shield;
    int                 victim_ac;
    int                 remort_bonus;
    int                 dam;
    int                 diceroll;
    int                 ix;
    float               dam_mod;
    extern const float  hr_damTable[121];

    /*
     * Can't beat a dead char!
     * Guard against weird room-leavings.
     */
    if (victim->position == POS_DEAD || ch->in_room != victim->in_room)
        return;

    if (magi_check(ch, victim))
        return;

    if (IS_NPC(ch) && IS_NPC(victim) && IS_SET(victim->act, ACT_NPCPROT))
        return;

    /*
     * Figure out the type of damage message.
     */
    wield = get_eq_char(ch, WEAR_WIELD);
    wield2 = get_eq_char(ch, WEAR_WIELD_2);

    if (dt == TYPE_UNDEFINED) {
        dt = TYPE_HIT;
        if (wield != NULL && wield->item_type == ITEM_WEAPON)
            dt += wield->value[3];
    }

    if (!IS_NPC(ch) && wield == NULL && wield2 == NULL && ch->pcdata->learned[gsn_martial_arts] > 0)
        dt = TYPE_MARTIAL;

    /*
     * Calculate to-hit-armor-class-0 versus armor.
     */

    victim_ac = GET_AC(victim);
    if (!can_see(ch, victim))
        victim_ac -= 200;

    if (dt == gsn_backstab || dt == gsn_circle)
        victim_ac += 300;

    /*
     * The moment of excitement!
     */
    diceroll = number_range((get_pseudo_level(ch) * 5), (get_pseudo_level(ch) * 21)) + GET_HITROLL(ch);

    /* players get a tohit bonus for now  */

    if (!IS_NPC(ch))
        diceroll += number_range(get_pseudo_level(ch), (get_pseudo_level(ch) * 1));
    if ((remort_bonus = get_pseudo_level(ch) > 100))
        diceroll += remort_bonus * 1;

    if (IS_NPC(ch)) {
        diceroll += (get_pseudo_level(ch) * 5);
        if (IS_SET(ch->act, ACT_SOLO))
            diceroll += (get_pseudo_level(ch) * 5);
    }
    if (IS_AFFECTED(ch, AFF_CLOAK_ADEPT) || (!IS_NPC(ch) && ch->pcdata->avatar))
        diceroll += get_pseudo_level(ch) * 2;

    /* Player vs player bonus, to handle unbalanced hitroll vs ac */
    if (!IS_NPC(ch) && !IS_NPC(victim) && get_pseudo_level(ch) > 80 && get_pseudo_level(victim) > 80)
        diceroll += number_range(1000, 2000);
    if (victim_ac < -3000 && get_pseudo_level(ch) > 110 && (number_range(0, 100) < 10))
        diceroll += 3000;

    if (victim_ac > -100) {
        if (diceroll + victim_ac < 0)
            ix = -1;
        else
            ix = ZERO_RATIO;
    }
    else {
        /* This finds the ratio of excess hit roll to AC, and
           breaks it into steps as defined by constants above,
           for use in damage modifier lookup table.
           Long int calculation avoids overflow problems. */
        ix = -(ONE_STEPS * (long int) (diceroll + victim_ac)) / (long int) victim_ac;
        ix += ZERO_RATIO;
    }

    if (ix < 0) {
        dam_mod = 0.0;

        damage(ch, victim, 0, dt);
        tail_chain();
        return;
    }
    else if (ix <= MAX_IX)
        dam_mod = hr_damTable[ix];
    else
        dam_mod = MAX_DAM_MOD;

    /*
     * Hit.
     * Calc damage.
     * Tried to make it easy for players to hit mobs... --Stephen
     */
    if (IS_NPC(ch)) {
        if (wield && wield->item_type == ITEM_WEAPON)
            dam = number_range(wield->value[1], wield->value[2]);
        else
            dam = number_range(ch->level / 3, ch->level / 2);
        if (IS_SET(ch->act, ACT_SOLO))
            dam *= 1.5;

    }
    else {
        if (wield != NULL && wield->item_type == ITEM_WEAPON)
            dam = number_range(wield->value[1], wield->value[2]);
        else
            dam = UMAX(number_range(2, 4), ch->level / 4);
    }

    /*
     * Bonuses.
     */
    dam += number_range(GET_DAMROLL(ch) * 13 / 20, GET_DAMROLL(ch) * 15 / 20);
    if ((!IS_NPC(ch) && ch->pcdata->learned[gsn_enhanced_damage] > 0)
        || item_has_apply(ch, ITEM_APPLY_ENHANCED)
        || (IS_NPC(ch) && IS_SET(ch->skills, MOB_ENHANCED))) {
        if (IS_NPC(ch))
            dam += dam * 1 / 20;
        else
            dam += dam * ch->pcdata->learned[gsn_enhanced_damage] / 150;
    }
    if (!IS_AWAKE(victim))
        dam *= 1.5;

    /* extra damage from martial arts */
    if (dt == TYPE_MARTIAL)
        dam = (dam * 12) / 11;

    if (dt == gsn_backstab)
        dam *= 1.4;

    if (dt == gsn_circle)
        dam *= 1.1;

    if (IS_AFFECTED(ch, AFF_CLOAK_ADEPT) || (!IS_NPC(ch) && ch->pcdata->avatar))
        dam *= 1.2;

    if (dam <= 0)
        dam = 1;

    if (!IS_NPC(victim) && victim->pcdata->learned[gsn_shield_block] > 0
        && (shield = get_eq_char(victim, WEAR_SHIELD)) != NULL && (number_range(1, 4) != 3)
        && number_percent() < ((victim->pcdata->learned[gsn_shield_block] / 5)
            + (1 * (victim->lvl[3] - ch->level) + victim->lvl2[2] / 8)))
        /* Shield Block! */
    {
        act("$n blocks $N's attack with $p", victim, shield, ch, TO_NOTVICT);
        act("$N blocks your attack with $p", ch, shield, victim, TO_CHAR);
        act("You block $N's attack with $p", victim, shield, ch, TO_CHAR);
        damage(ch, victim, 0, TYPE_IGNORE);
    }
    else {
        dam *= dam_mod;

        if (magi_check(ch, victim));
        else {
            if (   wield
                && dam > 0
                && IS_OBJ_STAT(wield, ITEM_LIFESTEALER)
                && number_range(0, 99) < (IS_OBJ_STAT(wield, ITEM_ADEPT) ? 30 : wield->level / 4)
                && (!IS_NPC(victim) || (IS_NPC(victim) && !IS_SET(victim->act, ACT_NOLIFESTEAL)))
               ) {
                act("@@e$n's@@y s@@Wk@@gi@@dn w@@Wi@@gt@@Wh@@ge@@Wr@@ds and @@Ww@@gr@@di@@gn@@Wk@@gl@@de@@Ws@@d as the @@N@@g@@el@@Ri@@df@@ge@@d is drawn from their very @@Ws@@gou@@Wl@@d.@@N@@N", victim, wield, ch, TO_NOTVICT);
                act("@@dAll @@RCo@@elo@@Rrs@@d fade as $p @@ddevours the @@ee@@Rs@@ds@@ge@@Wn@@gc@@de of @@e$N.@@N", ch, wield, victim, TO_CHAR);
                act("@@dYou feel de@@gvo@@Wid@@d of @@el@@Ri@@df@@ge@@d and @@pf@@me@@pel@@Wi@@mn@@pg@@d as $p steals from your @@ys@@do@@gu@@Wl@@d.@@N", victim, wield, ch, TO_CHAR);

                if (ch->lvl2[3] > 20) {
                    ch->hit = UMIN(ch->max_hit, ch->hit + number_range(dam * .075, dam * 1.72));
                }
                else if (ch->lvl2[0] > 20) {
                    ch->hit = UMIN(ch->max_hit, ch->hit + number_range(dam * .05, dam * 1));
                }
                else {
                    ch->hit = UMIN(ch->max_hit, ch->hit + number_range(dam * .03, dam * .13));
                }

                if (IS_NPC(ch) || !IS_IMMORTAL(ch))
                    ch->alignment = UMAX(-1000, ch->alignment - 50);
            }
            /*mindstealer start */
            if (   wield
                && dam > 0
                && IS_OBJ_STAT(wield, ITEM_MINDSTEALER)
                && number_range(0, 99) < (IS_OBJ_STAT(wield, ITEM_ADEPT) ? 30 : wield->level / 4)
                && (!IS_NPC(victim) || (IS_NPC(victim) && !IS_SET(victim->act, ACT_NOMINDSTEAL)))
               ) {
                sh_int              mana_dam = 0;

                act("@@W$N's@@d eyes@@y g@@Wl@@go@@yw@@d with@@y r@@Wa@@gd@@dia@@gn@@Wc@@de as $E d@@Re@@ev@@Wo@@gu@@drs the @@mm@@pa@@gg@@pi@@mc@@d of @@W$n@@N", victim, wield, ch, TO_NOTVICT);
                act("@@dYou grip $N@@d's head in @@Ra@@eg@@Wo@@en@@Ry@@d, as you rip the very @@We@@gs@@ds@@Be@@dn@@gc@@We@@d of @@mm@@pa@@Wg@@pi@@mc@@d from $S mind.@@N", ch, wield, victim, TO_CHAR);
                act("@@dYou feel your @@Wm@@le@@Bm@@do@@Br@@lie@@Ws @@dslipping @@da@@gw@@Wa@@gy@@d as @@e$p@@d @@N@@dinvades your @@gm@@li@@Bn@@dd, stealing away your mana@@N@@N", victim, wield, ch, TO_CHAR);
                if (ch->lvl2[0] > 20) {
                    mana_dam = number_range(dam / 10, dam / 6);
                    ch->mana = UMIN(ch->max_mana, ch->mana + number_range(dam * .075, dam * 1.72));
                }
                else if (ch->lvl2[3] > 20) {
                    mana_dam = number_range(dam / 10, dam / 6);
                    ch->mana = UMIN(ch->max_mana, ch->mana + number_range(dam * .05, dam * 1));
                }
                else {
                    ch->mana = UMIN(ch->max_mana, ch->mana + number_range(dam * .03, dam * .13));
                }
                victim->mana = UMAX(0, victim->mana - mana_dam);

                if (IS_NPC(ch) || !IS_IMMORTAL(ch))
                    ch->alignment = UMAX(-1000, ch->alignment - 50);
            }
            /*mindstealer stop */
        }

    }

    damage(ch, victim, dam, dt);

    tail_chain();
    return;
}

/*
 * Inflict damage from a hit.
 */
void
damage(CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt)
{
    extern CHAR_DATA   *violence_marker;
    OBJ_DATA           *wield;
    int                 odam;
    DUEL_DATA          *duel = NULL;
    int                 damcap = 3000;
    SHIELD_DATA        *shield, *shield_next;

    /*   char buf[MAX_STRING_LENGTH];   this is unused now -- uni */

    if (                        /* ( victim->position == POS_DEAD )  
                                   || */ (victim == violence_marker)
        || (victim->is_free == TRUE))
        return;

    /*
     * Stop up any residual loopholes.
     */

    if (!IS_NPC(ch) && (duel = find_duel(ch)) && IS_SET(duel->flags, DUEL_DBLDAM)) {
        damcap = 6000;
        dam *= 2;
    }

    if (dam > damcap) {
        char                buf[MAX_STRING_LENGTH];

        sprintf(buf, "Combat: %d damage by %s", dam, IS_NPC(ch) ? ch->short_descr : ch->name);
        if (ch->level < 82)        /* stop imms generating warnings!! */
            monitor_chan(buf, MONITOR_COMBAT);

        if (dt != TYPE_CRACK)
            dam = damcap;
    }

    odam = dam;

    /* crack has already done its job by bypassing the damage limit, so change it to ignore */
    if (dt == TYPE_CRACK)
        dt = TYPE_IGNORE;

    wield = get_eq_char(ch, WEAR_WIELD);
    if (dt == TYPE_UNDEFINED) {
        if (wield != NULL && wield->item_type == ITEM_WEAPON)
            dt = TYPE_HIT + wield->value[3];
        else if (!IS_NPC(ch) && ch->pcdata->learned[gsn_martial_arts] > 0)
            dt = TYPE_MARTIAL;
        else
            dt = TYPE_HIT;
    }

    /* no one takes damage in jail, unless an imp does the damage. */
    if ((victim->in_room != NULL)
        && (victim->in_room->vnum == 1)
        && (ch->level != 90))
        return;

    if (victim != ch) {
        /*
         * Certain attacks are forbidden.
         * Most other attacks are returned.
         */
        if (is_safe(ch, victim, TRUE))
            return;

        if ((victim != ch->fighting) && (victim->fighting != ch))
            check_killer(ch, victim);

        if (victim->position > POS_STUNNED) {
            abort_writing(victim);

            if (victim->fighting == NULL)
                set_fighting(victim, ch, FALSE);
            victim->position = POS_FIGHTING;
        }

        if (victim->position > POS_STUNNED) {
            if (ch->fighting == NULL) {
                set_fighting(ch, victim, TRUE);
                /* check_killer( ch, victim ); */
            }

            /*
             * If victim is charmed, ch might attack victim's master.
             */
            if (IS_NPC(ch)
                && IS_NPC(victim)
                && IS_AFFECTED(victim, AFF_CHARM)
                && victim->master != NULL && victim->master->in_room == ch->in_room && number_bits(3) == 0) {
                stop_fighting(ch, FALSE);
                multi_hit(ch, victim->master, TYPE_UNDEFINED);
                return;
            }

            /* add this later, but fix wanted flag problems associated with it

               if (   !IS_NPC(ch)
               && IS_NPC(victim)
               && IS_AFFECTED(victim, AFF_CHARM)
               && victim->master != NULL
               && ch != victim->master
               && victim->master->in_room == ch->in_room
               && number_bits(2) == 0) {
               stop_fighting(ch, TRUE);
               stop_fighting(victim, TRUE);
               stop_fighting(victim->master, TRUE);
               set_fighting(victim->master, ch, FALSE);
               set_fighting(ch, victim->master, FALSE);
               return;
               }

             */
        }

        /*
         * More charm stuff.
         */
        if (victim->master == ch)
            stop_follower(victim);

        /*
         * Inviso attacks ... not.
         */
        if (IS_AFFECTED(ch, AFF_INVISIBLE)) {
            affect_strip(ch, gsn_invis);
            affect_strip(ch, gsn_mass_invis);
            REMOVE_BIT(ch->affected_by, AFF_INVISIBLE);
            act("$n shimmers into existence.", ch, NULL, NULL, TO_ROOM);
        }

        /*
         * Damage modifiers.
         */
        if (IS_AFFECTED(victim, AFF_SANCTUARY)
            || item_has_apply(victim, ITEM_APPLY_SANC))
            dam /= 2;

        if ((IS_AFFECTED(victim, AFF_PROTECT) || item_has_apply(ch, ITEM_APPLY_PROT))
            && IS_EVIL(ch))
            dam -= dam / 4;

        if (dam < 0)
            dam = 0;

        /*
         * Check for disarm, trip, parry, and dodge.
         */
        if (dt >= TYPE_HIT || dt == TYPE_MARTIAL) {
            if (IS_NPC(ch)
                && (number_percent() < ch->level / 6)
                && IS_SET(ch->skills, MOB_DISARM))
                disarm(ch, victim);

            if (IS_NPC(ch)
                && (number_percent() < ch->level / 6)
                && IS_SET(ch->skills, MOB_TRIP))
                trip(ch, victim);

            if (check_parry(ch, victim))
                return;
            if (check_dodge(ch, victim))
                return;
            if (check_skills(ch, victim))
                return;
        }

        /* if there's a shield the damage might be modified, so only show the damage
           message if they don't have a shield up */
        if (victim->first_shield == NULL || dam == 0)
            dam_message(ch, victim, odam, dt);
    }
    else if (victim->first_shield == NULL || dam == 0)
        dam_message(ch, ch, odam, dt);

    /* for now, can only have one shield up, or alternatively, only the first
       shield does anything    */

    if (victim->first_shield != NULL && ch != victim && dam > 0) {
        int                 attackdam = 0;

        dam_message(ch, victim, odam, dt);

        for (shield = victim->first_shield; shield != NULL; shield = shield_next) {
            shield_next = shield->next;

            shield->hit -= dam;

            /* shield absorb % for mobs */
            if (IS_NPC(victim) && shield_table[shield->index].mprotection > 0) {
                if (shield->index != SHIELD_ICE || (shield->index == SHIELD_ICE && ch->race != RACE_MIN))
                    dam = dam - (int) (dam * ((shield_table[shield->index].mprotection) / 100.0));
            }
            /* shield absorb % for players */
            else if (!IS_NPC(victim) && shield_table[shield->index].protection > 0) {
                if (shield->index != SHIELD_ICE || (shield->index == SHIELD_ICE && ch->race != RACE_MIN))
                    dam = dam - (int) (dam * ((shield_table[shield->index].protection) / 100.0));
            }

            if (shield_table[shield->index].harmful == TRUE) {
                attackdam = shield_table[shield->index].damage;

                if (shield->index == SHIELD_SHOCK && ch->race == RACE_DRW)
                    attackdam = (int) (attackdam * 0.5);
                else if (shield->index == SHIELD_FIRE && ch->race == RACE_DRG)
                    attackdam = (int) (attackdam * 0.5);

                ch->hit = UMAX(10, (ch->hit - attackdam));
            }

            /* special case for demonshield */
            if (shield->index == SHIELD_DEMON) {
                AFFECT_DATA         af;

                if (number_range(0, 99) < 8 && !IS_AFFECTED(ch, AFF_CURSE) && ch->race != RACE_LAM) {
                    af.type = gsn_curse;
                    af.duration = 3;
                    af.location = APPLY_HITROLL;
                    af.modifier = -5;
                    af.bitvector = AFF_CURSE;
                    af.save = TRUE;
                    affect_to_char(ch, &af);

                    send_to_char("You feel unclean.\n\r", ch);
                }
                else if (number_range(0, 99) < 20) {
                    af.type = gsn_blindness;
                    af.location = APPLY_HITROLL;
                    af.modifier = -4;
                    af.duration = 2;
                    af.bitvector = AFF_BLIND;
                    af.save = TRUE;
                    affect_to_char(ch, &af);

                    send_to_char("You are blinded!\n\r", ch);
                }
                else if (number_range(0, 99) < 25 && ch->race != RACE_LAM) {
                    if (number_percent() < 80)
                        af.type = gsn_harm;
                    else
                        af.type = gsn_poison;

                    af.duration = 3;
                    af.location = APPLY_STR;
                    af.modifier = -3;
                    af.bitvector = AFF_POISON;
                    af.save = TRUE;
                    affect_join(ch, &af);
                }
                else if (number_range(0, 99) < 35 && (!IS_NPC(ch) || (IS_NPC(ch) && !IS_SET(ch->act, ACT_NO_FLEE)))) {
                    act("$N screams at $n in horror!!", victim, NULL, ch, TO_ROOM);
                    act("$N screams at you in horror!!", ch, NULL, victim, TO_CHAR);
                    send_to_char("You flip, and look for escape!!\n\r", ch);
                    do_flee(ch, "");
                }

                attackdam = shield_table[shield->index].damage;
            }

            act(shield_table[shield->index].absorb_room,   victim, NULL, ch, TO_NOTVICT);
            act(shield_table[shield->index].absorb_self,   victim, NULL, ch, TO_CHAR);
            act(shield_table[shield->index].absorb_victim, victim, NULL, ch, TO_VICT);

            if (shield->hit <= 0) {
                AFFECT_DATA        *shield_aff;

                for (shield_aff = victim->first_affect; shield_aff != NULL; shield_aff = shield_aff->next)
                    if (    (shield_aff->type == gsn_shield_fire  && shield->index == SHIELD_FIRE)
                         || (shield_aff->type == gsn_shield_ice   && shield->index == SHIELD_ICE)
                         || (shield_aff->type == gsn_shield_shock && shield->index == SHIELD_SHOCK)
                         || (shield_aff->type == gsn_shield_demon && shield->index == SHIELD_DEMON)
                       )
                        break;

                /* affect_remove will remove "shield", so we need "shield_next" as above */
                if (shield_aff != NULL)
                    affect_remove(victim, shield_aff);
            }
        }
    }
    /*
     * Hurt the victim.
     * Inform the victim of his new state.
     */
    /* cloak:mana mod, if they have cloak:mana, damage them differently -Erigol */
    if ((!is_affected(victim, gsn_cloak_mana)) || (ch == victim))
        victim->hit -= dam;
    else {
        /* cloak mana will be variable now, if they are both sor and nec, they get the most benefit
           but if they are one or the other, it's half. also it only works if they are not flaming */
        /* Added by Chewbacca, modified by Erigol */
        if (IS_AFFECTED(victim, AFF_CLOAK_FLAMING)) {
            victim->hit -= dam;
        }
        else if (victim->mana > 10 && victim->lvl2[3] > 70 && victim->lvl2[0] > 70) {
            victim->hit = UMAX(-5, victim->hit - dam * 1 / 3);
            victim->mana = UMAX(0, victim->mana - dam * 2 / 3);
        }
        else if (victim->mana > 10 && ((victim->lvl2[3] > 70) || (victim->lvl2[0] > 70))) {
            victim->hit = UMAX(-5, victim->hit - dam * 1 / 2);
            victim->mana = UMAX(0, victim->mana - dam * 1 / 2);
        }
        else
            victim->hit -= dam;

        if (dam > 0) {
            act(cloak_table[CLOAK_MANA].absorb_room, victim, NULL, NULL, TO_ROOM);
            act(cloak_table[CLOAK_MANA].absorb_self, victim, NULL, NULL, TO_CHAR);
        }

        if (victim->mana == 0) {
            /* cloak mana ran out. strip it */
            affect_strip(victim, gsn_cloak_mana);
        }
    }
    if (!IS_NPC(victim))
        check_adrenaline(victim, dam);

    if (IS_AFFECTED(victim, AFF_CLOAK_FLAMING) && ch != victim) {
        if (dam > 0) {
            act(cloak_table[CLOAK_FLAMING].absorb_room,   victim, NULL, ch, TO_NOTVICT);
            act(cloak_table[CLOAK_FLAMING].absorb_self,   victim, NULL, ch, TO_CHAR);
            act(cloak_table[CLOAK_FLAMING].absorb_victim, ch, NULL, victim, TO_CHAR);
        }

        if (is_shielded(ch, SHIELD_ICE)
            && number_range(1, 100) < 30
            && ch != victim) {
            CHAR_DATA          *rch;
            CHAR_DATA          *rch_next;
            OBJ_DATA           *explosion;
            CHAR_DATA          *elemental;

            act("@@e------------------------@@N", ch, NULL, NULL, TO_CHAR);
            act("@@l************************@@N", ch, NULL, NULL, TO_CHAR);
            act("@@W!!!!!!!!!!!!!!!!!!!!!!!!@@N", ch, NULL, NULL, TO_CHAR);
            act("@@NThe elemental forces of @@eFire@@N and @@aIce@@N destroy each other!!!", ch, NULL, NULL, TO_CHAR);
            act("@@W!!!!!!!!!!!!!!!!!!!!!!!!@@N", ch, NULL, NULL, TO_CHAR);
            act("@@l************************@@N", ch, NULL, NULL, TO_CHAR);
            act("@@e------------------------@@N", ch, NULL, NULL, TO_CHAR);
            act("@@e------------------------@@N", ch, NULL, NULL, TO_ROOM);
            act("@@l************************@@N", ch, NULL, NULL, TO_ROOM);
            act("@@W!!!!!!!!!!!!!!!!!!!!!!!!@@N", ch, NULL, NULL, TO_ROOM);
            act("@@NThe elemental forces of @@eFire@@N and @@aIce@@N destroy each other!!!", ch, NULL, NULL, TO_ROOM);
            act("@@W!!!!!!!!!!!!!!!!!!!!!!!!@@N", ch, NULL, NULL, TO_ROOM);
            act("@@l************************@@N", ch, NULL, NULL, TO_ROOM);
            act("@@e------------------------@@N", ch, NULL, NULL, TO_ROOM);
            act("@@N$N's @@ecloak@@N is ripped to shreds!!!@@N", ch, NULL, victim, TO_ROOM);
            act("@@NYour @@ecloak@@N is ripped to shreds!!!@@N", ch, NULL, victim, TO_VICT);

            affect_strip(ch, gsn_shield_ice);
            affect_strip(victim, gsn_shield_ice);
            affect_strip(ch, gsn_shield_fire);
            affect_strip(victim, gsn_shield_fire);
            affect_strip(ch, gsn_cloak_flaming);
            affect_strip(victim, gsn_cloak_flaming);
            if (IS_SET(ch->affected_by, AFF_CLOAK_FLAMING))
                REMOVE_BIT(ch->affected_by, AFF_CLOAK_FLAMING);
            if (IS_SET(victim->affected_by, AFF_CLOAK_FLAMING))
                REMOVE_BIT(victim->affected_by, AFF_CLOAK_FLAMING);

            if ((!IS_NPC(ch) && is_in_duel(ch, DUEL_STAGE_GO))
                || (!IS_NPC(victim) && is_in_duel(victim, DUEL_STAGE_GO))
                );
            else {
                if ((explosion = create_object(get_obj_index(OBJ_VNUM_CONFLAGRATION), 120)) != NULL)
                    if ((elemental = create_mobile(get_mob_index(MOB_VNUM_COMBAT_ELEMENTAL))) != NULL) {
                        char                bufz[MSL];

                        explosion->level = 120;
                        free_string(explosion->short_descr);
                        explosion->short_descr = str_dup("@@mConflagration@@N");
                        free_string(explosion->description);
                        explosion->description = str_dup("@@N A @@eFlaming @@NStaff of @@aIce@@N is supsended in mid air!");

                        elemental->level = 140;
                        free_string(elemental->name);
                        elemental->name = str_dup(".hidden");
                        free_string(elemental->short_descr);
                        elemental->short_descr = str_dup("@@NThe @@rConflict@@N of @@eFire @@Nand @@aIce@@N");
                        free_string(elemental->long_descr);
                        elemental->long_descr = str_dup("@@NA @@rPillar@@N of @@eFire @@Nand @@aIce@@N immolates itself!\n");
                        elemental->extract_timer = number_range(10, 20);
                        char_to_room(elemental, ch->in_room);
                        obj_to_char(explosion, elemental);
                        sprintf(bufz, "%s", explosion->name);
                        do_wear(elemental, bufz);

                        if (number_range(0, 99) < 40)
                            obj_cast_spell(skill_lookup("Retributive strike"), 100, elemental, NULL, explosion);
                        else

                            for (rch = ch->in_room->first_person; rch != NULL; rch = rch_next) {
                                rch_next = rch->next_in_room;
                                if (get_pseudo_level(rch) < 70)
                                    continue;

                                if (!IS_NPC(rch)
                                    && (IS_IMMORTAL(rch)
                                        || rch->stance == STANCE_AMBUSH || rch->stance == STANCE_AC_BEST)
                                    )
                                    continue;

                                if (IS_NPC(rch) && IS_AFFECTED(rch, AFF_CHARM))
                                    continue;

                                if (!IS_NPC(ch) && is_safe(ch, rch, FALSE))
                                    continue;

                                if (!IS_NPC(victim) && is_safe(victim, rch, FALSE))
                                    continue;

                                if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM) && ch->master && !IS_NPC(ch->master) && is_safe(ch->master, rch, FALSE))
                                    continue;

                                if (IS_NPC(victim) && IS_AFFECTED(victim, AFF_CHARM) && victim->master && !IS_NPC(victim->master)
                                    && is_safe(victim->master, rch, FALSE))
                                    continue;

                                send_to_char("\n\r@@NYou are @@ablasted@@N by the @@econflagration@@N!\n\r", rch);
                                if (number_range(0, 99) < 50)
                                    obj_cast_spell(skill_lookup("frost breath"), 120, elemental, rch, explosion);
                                else
                                    obj_cast_spell(skill_lookup("lava burst"), 120, elemental, rch, explosion);
                            }
                        if ((elemental->fighting != NULL)
                            && (IS_NPC(elemental->fighting))) {
                            stop_fighting(elemental, TRUE);
                            if ((number_range(0, 99) < 50)
                                && (!IS_NPC(ch))) {
                                multi_hit(elemental, ch, TYPE_UNDEFINED);
                            }
                            else if (!IS_NPC(victim)) {
                                multi_hit(elemental, victim, TYPE_UNDEFINED);
                            }
                            /*     extract_char( elemental, TRUE );  */
                        }
                    }
            }
        }
        else {
            int                 flame_damage;

            flame_damage = dam * get_pseudo_level(victim) / 300;

            if (ch->race == RACE_DRG)
                flame_damage = (int) (flame_damage * 0.8);

            if (IS_NPC(ch)) {
                if (IS_SET(ch->act, ACT_SOLO))
                    flame_damage = flame_damage / 5;
                else
                    flame_damage = flame_damage / 3;
            }
            if ((IS_NPC(victim)) && (IS_SET(victim->act, ACT_SOLO)))
                flame_damage = flame_damage * 1.5;

            ch->hit = UMAX(-5, ch->hit - flame_damage);

            /*
             * Make-shift death of a NPC if flame cloak gets em <0 hps
             */

            if (IS_NPC(ch)) {
                update_pos(ch);
                if (ch->position == POS_DEAD) {
                    act("$n burns to a crisp!", ch, 0, 0, TO_ROOM);
                    act("$n is DEAD!!", ch, 0, 0, TO_ROOM);

                    if (!IS_NPC(victim)) {
                        group_gain(victim, ch);
                        victim->pcdata->mkills++;
                    }

                    raw_kill(ch, "");

                    /* AutoLoot/AutoGold/AutoSac stuff */
                    if (!IS_NPC(victim)) {
                        if (IS_SET(victim->act, PLR_AUTOLOOT))
                            do_get(victim, "all corpse");
                        else if (IS_SET(victim->config, PLR_AUTOGOLD)) {
                            do_get(victim, "all.coin* corpse");
                            do_look(victim, "in corpse");
                        }
                        else
                            do_look(victim, "in corpse");

                        if (IS_SET(victim->act, PLR_AUTOSAC))
                            do_sacrifice(victim, "corpse");
                    }
                }
            }

        }

    }
    update_pos(victim);

    switch (victim->position) {
        case POS_MORTAL:
            act("$n is mortally wounded, and will die soon, if not aided.", victim, NULL, NULL, TO_ROOM);
            send_to_char("You are mortally wounded, and will die soon, if not aided.\n\r", victim);
            break;

        case POS_INCAP:
            act("$n is incapacitated and will slowly die, if not aided.", victim, NULL, NULL, TO_ROOM);
            send_to_char("You are incapacitated and will slowly die, if not aided.\n\r", victim);
            break;

        case POS_STUNNED:
            act("$n is too stunned to do anything!", victim, NULL, NULL, TO_ROOM);
            send_to_char("You are too stunned to do anything!\n\r", victim);
            break;

        case POS_DEAD:
            act("$n is DEAD!!", victim, 0, 0, TO_ROOM);
            send_to_char("You have been KILLED!!\n\r\n\r", victim);
            break;

        default:
            if (dam > victim->max_hit / 4)
                send_to_char("That really did HURT!\n\r", victim);
            if (victim->hit < victim->max_hit / 4)
                send_to_char("You sure are BLEEDING!\n\r", victim);
            break;
    }

    /* use our own kill function */
    if ((victim->position == POS_DEAD
            || victim->position == POS_MORTAL
            || victim->position == POS_INCAP || victim->position == POS_STUNNED) && !IS_NPC(victim) && is_in_duel(victim, DUEL_STAGE_SET)) {
        duel_rawkill(ch, victim, DUEL_RAWKILL_NORMAL);
        return;
    }

    if ((victim->position == POS_DEAD || victim->position == POS_MORTAL || victim->position == POS_INCAP || victim->position == POS_STUNNED)
        && !IS_NPC(victim) && victim->pcdata->in_arena) {
        stop_fighting(victim, TRUE);
        if (ch == victim)
            arenaf(victim, "%s suicides in %s!", victim->short_descr, victim->in_room->name);
        else if (!IS_NPC(ch))
            arenaf(ch, "%s kills %s in %s!", ch->short_descr, victim->short_descr, ch->in_room->name);
        else
            arenaf(victim, "%s is killed by a mob in %s!", victim->short_descr, victim->in_room->name);

        leave_arena(victim, FALSE);
        return;
    }

    if (!IS_NPC(ch) && ch->pcdata->in_arena && ch->hit <= 0) {
        act("$n is DEAD!!", ch, 0, 0, TO_ROOM);
        send_to_char("You have been KILLED!!\n\r\n\r", ch);

        stop_fighting(ch, TRUE);
        if (ch == victim)
            arenaf(ch, "%s suicides in %s!", ch->short_descr, ch->in_room->name);
        else if (!IS_NPC(victim))
            arenaf(victim, "%s kills %s in %s!", victim->short_descr, ch->short_descr, victim->in_room->name);
        else
            arenaf(ch, "%s is killed by a mob in %s!", ch->short_descr, ch->in_room->name);

        leave_arena(ch, FALSE);
        return;
    }

    if (!IS_NPC(ch) && is_in_duel(ch, DUEL_STAGE_SET) && ch->hit <= 0) {
        act("$n is DEAD!!", ch, 0, 0, TO_ROOM);
        send_to_char("You have been KILLED!!\n\r\n\r", ch);
        duel_rawkill(victim, ch, DUEL_RAWKILL_NORMAL);
    }

    /*
     * Sleep spells and extremely wounded folks.
     */
    if (!IS_AWAKE(victim))
        stop_fighting(victim, FALSE);

    /*
     * Payoff for killing things.
     */

    if (victim->position == POS_DEAD) {
        bool unlawful = FALSE;

        group_gain(ch, victim);

        if (!IS_NPC(ch) && !IS_NPC(victim) && 
             (   IS_SET(victim->pcdata->pflags, PFLAG_PKOK)
              || IS_SET(victim->act, PLR_KILLER)
              || IS_SET(victim->act, PLR_THIEF)
              || IS_SET(victim->in_room->room_flags, ROOM_PK)
              || (ch->pcdata->clan != 0
                  && victim->pcdata->clan != 0
                  && politics_data.diplomacy[ch->pcdata->clan][victim->pcdata->clan] < -450)
           ))
            unlawful = FALSE;
        else
            unlawful = TRUE;

        /* Sort out kill counts..... */
        if (ch != victim) {
            if (!IS_NPC(victim) && !IS_NPC(ch)) {
                if (!unlawful)
                    ch->pcdata->pkills++;
                else
                    ch->pcdata->unpkills++;
            }
            else if (!IS_NPC(ch))
                ch->pcdata->mkills++;

            if (!IS_NPC(victim)) {
                if (!IS_NPC(ch))
                    victim->pcdata->pkilled++;
                else
                    victim->pcdata->mkilled++;
            }
        }

        if (!IS_NPC(victim)) {
            if (!IS_NPC(ch) && !IS_NPC(victim)
                && IS_SET(ch->pcdata->pflags, PFLAG_PKOK)
                && IS_SET(victim->pcdata->pflags, PFLAG_PKOK)) {

                sprintf(log_buf, "%s @@ekills@@g %s@@e in @@Wmortal combat.@@N", ch->name, victim->name);
                info(log_buf, 1);
            }
            else {
                sprintf(log_buf, "%s @@eturns@@g %s@@e into a corpse.@@N",
                    (IS_NPC(ch) ? ch->short_descr : ch->name), (IS_NPC(victim) ? victim->short_descr : victim->name));
                info(log_buf, 1);
            }

            sprintf(log_buf, "%s killed by %s at %d",
                (IS_NPC(victim) ? victim->short_descr : victim->name), (IS_NPC(ch) ? ch->short_descr : ch->name), victim->in_room->vnum);
            log_string(log_buf);

            notify(log_buf, 82);

            /* As level gain is no longer automatic, a dead char loses
             * 1/2 their gained exp.  -S- 
             * Fixed my bug here too, hehe!
             */

            if (victim->exp > 0 && victim->race != RACE_OGR) {
                /* if it was an unlawful death, only take off 5% of their xp */
                if (ch != victim && !IS_NPC(ch) && !IS_NPC(victim) && unlawful)
                    gain_exp(victim, (0 - (victim->exp / 20)));
                else
                    gain_exp(victim, (0 - (victim->exp / 2)));

                victim->exp = UMAX(victim->exp, 0);
            }

        }

        if (IS_NPC(ch)) {
            char                name_buf[MAX_STRING_LENGTH];

            sprintf(name_buf, "%d", ch->pIndexData->vnum);
            raw_kill(victim, name_buf);
        }
        else {
            char                name_buf[MAX_STRING_LENGTH];

            sprintf(name_buf, "%s", ch->name);
            raw_kill(victim, name_buf);
        }

        if (!IS_NPC(ch) && IS_NPC(victim)) {
            if (IS_SET(ch->act, PLR_AUTOLOOT))
                do_get(ch, "all corpse");
            else if (IS_SET(ch->config, PLR_AUTOGOLD)) {
                do_get(ch, "all.coin* corpse");
                do_look(ch, "in corpse");
            }
            else
                do_look(ch, "in corpse");

            if (IS_SET(ch->act, PLR_AUTOSAC))
                do_sacrifice(ch, "corpse");
        }

        return;
    }

    if (victim == ch)
        return;

    /*
     * Take care of link dead people.
     */
    if (!IS_NPC(victim) && victim->desc == NULL) {
        if (number_range(0, victim->wait) == 0) {
            if (IS_NPC(ch)) {
                do_recall(victim, "");
                return;
            }
        }
    }

    /*
     * Wimp out?
     */
    if (IS_NPC(victim) && dam > 0) {
        if ((IS_SET(victim->act, ACT_WIMPY) && number_bits(1) == 0 && victim->hit < victim->max_hit / 2)
            || (IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL && victim->master->in_room != victim->in_room)) {
            do_flee(victim, "");
        }
    }
    if (!IS_NPC(victim)
        && victim->hit > 0 && victim->hit <= victim->wimpy && victim->wait == 0) {
        if (victim->stunTimer > 0) {
            send_to_char("You cannot flee while stunned!\n\r", victim);
        }
        else {
            do_flee(victim, "");
        }
    }

    tail_chain();
    return;

}

bool
is_safe(CHAR_DATA *ch, CHAR_DATA *victim, bool showmessage)
{
    extern bool         nopk;
    extern bool         quest;
    extern CHAR_DATA   *quest_mob, *quest_target;
    extern OBJ_DATA    *quest_object;

    /* this should never happen, but you never can be too careful */
    if (!ch || !victim || ch->in_room == NULL || victim->in_room == NULL || ch->in_room != victim->in_room)
        return TRUE;

    if (IS_SET(victim->in_room->room_flags, ROOM_SAFE)) {
        if (showmessage)
            send_to_char("Not a chance!  This is a safe room.\n\r", ch);

        return TRUE;
    }

    if (IS_NPC(victim) && IS_SET(victim->act, ACT_SAFE)) {
        if (showmessage)
            act("$N is protected by the gods.", ch, NULL, victim, TO_CHAR);
        return TRUE;
    }

    if (!IS_NPC(ch) && !IS_NPC(victim) && IS_SET(victim->pcdata->pflags, PFLAG_SAFE)) {
        if (showmessage)
            act("@@N@@gYou may not attack $N, $E is a helper. See @@ahelp helper@@g for more details.", ch, NULL, victim, TO_CHAR);

        return TRUE;
    }

    if (!IS_NPC(ch) && !IS_NPC(victim) && IS_SET(ch->pcdata->pflags, PFLAG_SAFE)) {
        if (showmessage)
            send_to_char("Helpers are not permitted to attack players.\n\r", ch);

        return TRUE;
    }

    if (IS_NPC(victim) && quest && victim == quest_mob) {
        if (showmessage)
            send_to_char("The Quest Mob is protected.\n\r", ch);

        return TRUE;
    }

    if (IS_NPC(victim) && quest && victim == quest_target) {
        OBJ_DATA *obj;
        bool found = FALSE;

        for (obj = victim->first_carry; obj; obj = obj->next_in_carry_list)
            if (obj == quest_object) {
                found = TRUE;
                break;
            }

        if (!found) {
            if (showmessage)
                send_to_char("The Quest Thief is protected because it no longer has the quest item.\n\r", ch);

            return TRUE;
        }
    }

    /* player is not in a safe room and they are hitting themselves, so allow it regardless */
    if (ch == victim)
        return FALSE;

    /* charmies have the same protection their masters do, so if the victim is a charmie
     * recursively call is_safe for their master.. since their master isn't a charmie
     * themselves, this will not infinitely loop. also, do the same if ch is a charmie */
    if (IS_NPC(victim) && IS_AFFECTED(victim, AFF_CHARM) && victim->master && !IS_NPC(victim->master) && ch->in_room == victim->master->in_room && is_safe(ch, victim->master, FALSE)) {
        if (showmessage)
            act("$N benefits from $S master's protection.", ch, NULL, victim, TO_CHAR);

        return TRUE;
    }

    if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM) && ch->master && !IS_NPC(ch->master) && ch->master->in_room == victim->in_room && is_safe(ch->master, victim, FALSE))
        return TRUE;

    if (!IS_NPC(ch) && !IS_NPC(victim) && victim->pcdata->in_arena)
        return FALSE;

    if (!IS_NPC(ch) && !IS_NPC(victim) && victim->pcdata->safetimer > 0) {
        char                dbuf[MSL];

        dbuf[0] = 0;

        if (showmessage)
            act("$N is currently under protection. Try again in $t.", ch, duration(abs(victim->pcdata->safetimer - current_time), dbuf), victim, TO_CHAR);

        return TRUE;
    }

    if (!IS_NPC(ch) && !IS_NPC(victim) && ch->pcdata->safetimer > 0) {
        char                buf[MSL], dbuf[MSL];

        dbuf[0] = 0;

        if (showmessage) {
            sprintf(buf, "You are currently under protection. Try again in %s.\n\r",
                duration(abs(ch->pcdata->safetimer - current_time), dbuf));
            send_to_char(buf, ch);
        }

        return TRUE;
    }

    if (!IS_NPC(ch) && !IS_NPC(victim) && has_quest_item(ch)) {
        if (showmessage)
            send_to_char("You have the quest item. You must return it or get rid of it before attacking players!\n\r", ch);

        return TRUE;
    }

    if (!IS_NPC(ch) && !IS_NPC(victim) && has_quest_item(victim)) {
        if (showmessage)
            act("$N has the quest item and must return it or get rid of it before you can attack $M!", ch, NULL, victim, TO_CHAR);

        return TRUE;
    }

    if (!IS_NPC(victim) && (IS_SET(victim->act, PLR_KILLER) || IS_SET(victim->act, PLR_THIEF)))
        return FALSE;

    if (   !IS_NPC(ch)
        && !IS_NPC(victim)
        && IS_SET(victim->pcdata->pflags, PFLAG_PKOK)
        && IS_SET(ch->pcdata->pflags, PFLAG_PKOK)
       )
        return FALSE;

    if (IS_NGR_CHARMIE(ch, victim) && !IS_IMMORTAL(ch)) {
        if (showmessage)
            send_to_char("That charmed mobile belongs to someone you can't group with.\n\r", ch);

        return TRUE;
    }

    if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM) && ch->master && IS_NGR_CHARMIE(ch->master, victim) && !IS_IMMORTAL(ch->master))
        return TRUE;

    if (!IS_NPC(ch) && !IS_NPC(victim) && !can_group(ch, victim) && !IS_IMMORTAL(ch)) {
        if (showmessage)
            send_to_char("Group range PKilling is in effect. Try someone your own size.\n\r", ch);
        return TRUE;
    }

    if (!IS_NPC(ch) && !IS_NPC(victim)
        && !IS_SET(victim->pcdata->pflags, PFLAG_PKOK)
        && !is_in_duel(ch, DUEL_STAGE_GO) && !is_in_duel(victim, DUEL_STAGE_GO)
        && nopk) {
        if (showmessage)
            send_to_char("Killing Non-PKOK players has been turned off by an Implementor at this time.\n\r", ch);

        return TRUE;
    }

    if (!IS_NPC(ch) && !IS_NPC(victim) && (victim->level < 40 || victim->level + 20 < ch->level)) {
        if (showmessage)
            send_to_char("The Gods prevent your foul deed.\n\r", ch);

        return TRUE;
    }

    if (!IS_NPC(ch) && !IS_NPC(victim) && (ch->level < 40 || ch->level + 20 < victim->level)) {
        if (showmessage)
            send_to_char("The Gods prevent your foul deed.\n\r", ch);

        return TRUE;
    }

    return FALSE;
}

struct hunt_mobs_tp
{
    int                 mob_vnum;
    int                 room_vnum;
    int                 min_level;
    char               *name;
}
hunt_mobs[] = {
    {
    3062, 3001, 0, "Bug"}, {
    3561, 3001, 40, "Shadow Dragon"}, {
    18302, 3001, 80, "Bounty Hunter"}, {
    18301, 3001, 100, "The Avenger"}, {
    18306, 3001, 240, "Blayze"}, {
    -1, -1, -1, " "}
};

/* SRZ huntlist!  */
/* Scrapped MAX_HUNTERS - can now change table without 
 * full recompile.  Was unused in checking loop. -S-
 **** LEAVE THE LAST ELEMENT of the table in place! ****/

/*
 * See if an attack justifies a KILLER flag.
 */
void
check_killer(CHAR_DATA *ch, CHAR_DATA *victim)
{
    /*
     * Follow charm thread to responsible character.
     * Attacking someone's charmed char is hostile!
     * -S- Mod:  Set sentence according to difference in levels -
     * this is no. times the player must die before the killer flag
     * will be removed.
     */

    int                 diff;

    if ((ch->fighting == victim) || (victim->fighting == ch)) {
        if (IS_DEBUGGER(ch))
            send_to_char("check_killer: exiting: you're fighting the victim or you're fighting someone else, but the victim is fighting you.\n\r",
                ch);

        if (IS_DEBUGGER(victim))
            send_to_char("check_killer: exiting: you're probably assisting someone.\n\r", victim);

        return;
    }

    /*
     * NPC's are fair game.
     * So are killers and thieves.
     */
    if (IS_NPC(victim)
        || IS_SET(victim->act, PLR_KILLER)
        || IS_SET(victim->in_room->room_flags, ROOM_PK)    /* -S- Mod */
        ||IS_SET(victim->act, PLR_THIEF)
        || (ch == victim)) {
        if (IS_DEBUGGER(ch))
            send_to_char("check_killer: exiting: victim is NPC, killer, pk room, thief.\n\r", ch);

        if (IS_DEBUGGER(victim))
            send_to_char("check_killer: exiting: you are NPC, killer, pk room, thief.\n\r", victim);

        return;
    }

    /* no wanted flags in duels */
    if (is_in_duel(ch, DUEL_STAGE_GO) || is_in_duel(victim, DUEL_STAGE_GO))
        return;

    if (ch->sentence > 18000)
        return;

    /* Check to see if ch & victim are in clans, and enemies */

    if (!IS_NPC(ch) && !IS_NPC(victim)) {

        if ((ch->pcdata->clan != 0) && (victim->pcdata->clan != 0)
            && (politics_data.diplomacy[ch->pcdata->clan][victim->pcdata->clan] < -450))
            return;

        if (IS_SET(ch->pcdata->pflags, PFLAG_PKOK)
            && IS_SET(victim->pcdata->pflags, PFLAG_PKOK))
            return;
    }

    /*
     * NPC's are cool of course (as long as not charmed).
     * Hitting yourself is cool too (bleeding).
     * So is being immortal (Alander's idea).
     * BAH.  Imms get flagged too now, unless pkok.
     * And current killers stay as they are.
     */
    if (IS_NPC(ch)
        || ch == victim)
        return;

    send_to_char("*** You are a PLAYER KILLER!! ***\n\r", ch);

    {
        char                buf[MAX_STRING_LENGTH];

        sprintf(buf, "%s flagged as a KILLER for attack on %s.", ch->name, victim->name);
        monitor_chan(buf, MONITOR_COMBAT);
    }
    diff = 3;
    if (get_pseudo_level(ch) > get_pseudo_level(victim)) {
        diff += (get_pseudo_level(ch) - get_pseudo_level(victim)) / 7;
        if (diff > 5)
            diff = 5;
    }
    ch->sentence += diff * get_pseudo_level(ch) * 3;    /* Magic # - Ramias */
    if (ch->adept_level > 0)
        ch->sentence += diff * get_pseudo_level(ch) * 2;

    SET_BIT(ch->act, PLR_KILLER);
    save_char_obj(ch);

    /* MAG Create a hunter for the person */
    diff = get_pseudo_level(ch);

    /* Added if check back... meant to penalize for attacking lower
     * level players -S-
     */

    if (get_pseudo_level(ch) > get_pseudo_level(victim))
        diff += get_pseudo_level(ch) - get_pseudo_level(victim);

    return;
}

/*
 * Check for parry.
 */
bool
check_parry(CHAR_DATA *ch, CHAR_DATA *victim)
{
    int                 chance = 0;

    if (!IS_AWAKE(victim))
        return FALSE;

    if (IS_NPC(victim) && !IS_SET(victim->skills, MOB_PARRY))
        return FALSE;

    if (IS_NPC(victim)) {
        /* Tuan was here.  :) */
        chance = get_pseudo_level(victim) / 3.2 + get_curr_str(victim) * 2 / 5;
        if (IS_SET(victim->act, ACT_SOLO))
            chance += 15;
    }
    else {
        if (get_eq_char(victim, WEAR_WIELD) == NULL && get_eq_char(victim, WEAR_WIELD_2) == NULL)
            return FALSE;

        chance = (victim->pcdata->learned[gsn_parry] / 3.5) + get_curr_str(victim) * 3 / 5;
    }
    if (IS_AFFECTED(victim, AFF_CLOAK_ADEPT) || (!IS_NPC(victim) && victim->pcdata->avatar))
        chance += 5;

    if (number_percent() < (chance + (get_pseudo_level(victim) - get_pseudo_level(ch)) / 2)) {

        /*  act( "You parry $n's attack.",  ch, NULL, victim, TO_VICT    );
           act( "$N parries your attack.", ch, NULL, victim, TO_CHAR    );  */
        return TRUE;
    }
    return FALSE;
}

/*
 * Check for dodge.
 */
bool
check_dodge(CHAR_DATA *ch, CHAR_DATA *victim)
{
    int                 chance = 0;

    if (!IS_AWAKE(victim))
        return FALSE;

    if (IS_NPC(victim) && !IS_SET(victim->skills, MOB_DODGE))
        return FALSE;

    if (IS_NPC(victim)) {
        /* Tuan was here.  :) */
        chance = get_pseudo_level(victim) / 3.1 + get_curr_dex(victim) * 2 / 5;
        if (IS_SET(victim->act, ACT_SOLO))
            chance += 15;
    }
    else {
        chance = (victim->pcdata->learned[gsn_dodge] / 3.5) + get_curr_dex(victim) * 3 / 5;
        if (ch->lvl2[4] > 0)    /* Monk  */
            chance += ch->lvl2[4] / 8;
    }
    if (IS_AFFECTED(victim, AFF_CLOAK_ADEPT) || (!IS_NPC(victim) && victim->pcdata->avatar))
        chance += 5;

    if (number_percent() < (chance + (get_pseudo_level(victim) - get_pseudo_level(ch)) / 2)) {

        /* act( "You dodge $n's attack.", ch, NULL, victim, TO_VICT    );
           act( "$N dodges your attack.", ch, NULL, victim, TO_CHAR    );  */
        return TRUE;
    }
    return FALSE;
}

/*
 * Check_skills : if IS_NPC(ch) then check ch->skills to see if there are
 * any extra attack skills available for use --Stephen
 */

bool
check_skills(CHAR_DATA *ch, CHAR_DATA *victim)
{
    int                 cnt, check;

    if (!IS_NPC(ch))
        return FALSE;

    if (number_percent() < 30 + (ch->level - victim->level))
        return FALSE;

    /* Count how many of the attack skills are available */

    cnt = 0;
    if (IS_SET(ch->skills, MOB_PUNCH))
        cnt++;
    if (IS_SET(ch->skills, MOB_HEADBUTT))
        cnt++;
    if (IS_SET(ch->skills, MOB_KNEE))
        cnt++;
    if (IS_SET(ch->skills, MOB_DIRT))
        cnt++;
    if (IS_SET(ch->skills, MOB_CHARGE))
        cnt++;

    if (cnt == 0)
        return FALSE;            /* There were no attack skills set */

    check = number_range(1, cnt);

    cnt = 0;
    if (IS_SET(ch->skills, MOB_PUNCH) && (++cnt == check)) {
        do_punch(ch, "");
        return TRUE;
    }
    if (IS_SET(ch->skills, MOB_HEADBUTT) && (++cnt == check)) {
        do_headbutt(ch, "");
        return TRUE;
    }
    if (IS_SET(ch->skills, MOB_KNEE) && (++cnt == check)) {
        do_knee(ch, "");
        return TRUE;
    }
    if (IS_SET(ch->skills, MOB_DIRT) && (++cnt == check)) {
        do_dirt(ch, "");
        return TRUE;
    }
    if (IS_SET(ch->skills, MOB_CHARGE) && (++cnt == check)) {
        do_charge(ch, "");
        return TRUE;
    }

    return FALSE;
}

/*
 * Set position of a victim.
 */
void
update_pos(CHAR_DATA *victim)
{
    if (victim->hit > 0) {
        if (victim->position <= POS_STUNNED && victim->stunTimer == 0) {
            act("$n stands, and gets to $s feet.", victim, NULL, NULL, TO_ROOM);
            victim->position = POS_STANDING;
        }
        return;
    }

    if (IS_NPC(victim) && victim->hit <= 0) {
        victim->position = POS_DEAD;
        return;
    }
    if (victim->hit <= -10)
        victim->position = POS_DEAD;
    else if (victim->hit <= -6)
        victim->position = POS_MORTAL;
    else if (victim->hit <= -3)
        victim->position = POS_INCAP;
    else
        victim->position = POS_STUNNED;

    return;
}

/*
 * Start fights.
 */
void
set_fighting(CHAR_DATA *ch, CHAR_DATA *victim, bool check)
{
    extern CHAR_DATA *quest_mob, *quest_target;

    if (ch->fighting != NULL) {
        return;
    }

    /* Check here for killer flag */
    if (check)
        check_killer(ch, victim);

    /*    if ( IS_NPC(victim) && IS_SET(victim->act, ACT_HUNTER ) )
       make_hunt( victim, ch ); *//* fun fun FUN! */

    if (IS_AFFECTED(ch, AFF_SLEEP))
        affect_strip(ch, gsn_sleep);

    abort_writing(ch);
    abort_writing(victim);

    ch->fighting = victim;
    ch->position = POS_FIGHTING;

    /* Check if mob has ACT_REMEMBER (ch to attack) SET */

    if (IS_NPC(victim) && !IS_NPC(ch) && IS_SET(victim->act, ACT_REMEMBER) && victim != quest_mob && victim != quest_target) {
        /* Then set victim->target to player's name... */
        if (victim->target != NULL)
            free_string(victim->target);
        if (ch != NULL && ch->is_free == FALSE)
            victim->target = str_dup(ch->name);
    }

    return;
}

/*
 * Stop fights.
 */
void
stop_fighting(CHAR_DATA *ch, bool fBoth)
{
    CHAR_DATA          *fch;
    extern int         fighttimer;

    ch->fighting = NULL;
    ch->position = POS_STANDING;
    if (!IS_NPC(ch))
        ch->pcdata->fighttimer = fighttimer;

    update_pos(ch);

    if (!fBoth)

        return;

    for (fch = first_char; fch != NULL; fch = fch->next) {
        extern CHAR_DATA   *violence_marker;

        if ((fch != violence_marker) && fch->fighting == ch) {
            fch->fighting = NULL;
            fch->position = POS_STANDING;
            if (!IS_NPC(fch))
                fch->pcdata->fighttimer = fighttimer;

            update_pos(fch);
        }
    }

    return;
}

/*
 * Make a corpse out of a character.
 */
void
make_corpse(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    char                arg[MAX_INPUT_LENGTH];
    OBJ_DATA           *corpse;
    OBJ_DATA           *obj;
    OBJ_DATA           *obj_next;
    CHAR_DATA          *target = NULL;
    CHAR_DATA          *wch;
    char               *name;
    int                 target_vnum = 0;
    bool                leave_corpse = FALSE;

    /*   int counter, num;   */
    extern OBJ_DATA    *quest_object;
    extern int          quest_timer;
    extern CHAR_DATA   *quest_target;

    one_argument(argument, arg);

    if (is_number(arg))
        target_vnum = atoi(arg);

    if (IS_NPC(ch)) {
        if ((ch->in_room != NULL)
            && IS_SET(ch->in_room->affected_by, ROOM_BV_SOUL_NET)) {
            ROOM_INDEX_DATA    *room;
            ROOM_AFFECT_DATA   *raf;
            ROOM_AFFECT_DATA   *raf_next;

            corpse = create_object(get_obj_index(OBJ_VNUM_CAPTURED_SOUL), ch->level);
            corpse->level = ch->level;

            sprintf(buf, corpse->short_descr, ch->short_descr);
            free_string(corpse->short_descr);
            corpse->short_descr = str_dup(buf);

            obj_to_room(corpse, ch->in_room);
            for (obj = ch->first_carry; obj != NULL; obj = obj_next) {
                obj_next = obj->next_in_carry_list;
                obj_from_char(obj);
                extract_obj(obj);
            }
            act("@@eAs $n's soul attempts to fade from the room, the @@dSoul Net@@e quickly collapses, entombing the soul into a small figurine!!",
                ch, NULL, NULL, TO_ROOM);
            room = ch->in_room;
            for (raf = room->first_room_affect; raf != NULL; raf = raf_next) {
                raf_next = raf->next;
                if (raf->bitvector == ROOM_BV_SOUL_NET) {
                    r_affect_remove(room, raf);
                }
            }
            return;
        }
        else {                    /* still NPC, no soul net */

            int                 gold;
            time_t              lifetime;

            name = ch->short_descr;
            corpse = create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC), 0);
            corpse->timer = number_range(3, 6);
            corpse->level = ch->level;    /* for animate spell */
            /* Takes a mob 2 rl hours to gain full gold. */
            lifetime = current_time - (ch->logon);
            gold = 5 * (ch->level) * (UMIN(100, lifetime * 100 / (2 * 3600))) / 100;
            /* Then take from 1/5 of maximum (i.e. level) to maximum gold. */
            gold = number_range(gold / 5, gold);
            /* Add special gold. */
            gold += ch->gold;

            if (IS_NPC(ch) && ch->pIndexData && ch->pIndexData->pShop != NULL)
                gold = 0;

            if (gold > 0) {
                obj_to_obj(create_money(gold), corpse);
            }
            ch->gold = 0;
        }
    }
    else {                        /* player */

        name = ch->name;
        corpse = create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
        corpse->timer = 60;

        sprintf(buf, "%s", ch->name);
        free_string(corpse->owner);
        corpse->owner = str_dup(buf);

        if ((arg[0] != '\0') && (!IS_NPC(ch))) {
            target = NULL;
            for (wch = first_char; wch != NULL; wch = wch->next) {
                extern CHAR_DATA   *violence_marker;

                if ((wch != violence_marker) && !IS_NPC(wch) && is_name(arg, wch->name)) {
                    target = wch;
                    break;
                }
            }

            if ((target != NULL) && !IS_NPC(target)) {
                if (IS_SET(ch->pcdata->pflags, PFLAG_PKOK))
                    corpse->value[0] = 1;
                if (ch->pcdata->clan > 0) {
                    if (target->pcdata->clan != ch->pcdata->clan && (politics_data.diplomacy[ch->pcdata->clan][target->pcdata->clan] < -450)) {
                        corpse->value[2] = target->pcdata->clan;
                    }
                }
                else
                    corpse->value[2] = -1;
                corpse->value[3] = number_range(2, 5);
            }
        }
    }                            /* end of player only */

    if (   !IS_NPC(ch)
        && (   IS_SET(ch->pcdata->pflags, PFLAG_PKOK)
            || (target != NULL && target->pcdata->clan != ch->pcdata->clan && politics_data.diplomacy[ch->pcdata->clan][target->pcdata->clan] < -450)
            || (ch->level > 30 && (IS_SET(ch->act, PLR_KILLER) || IS_SET(ch->act, PLR_THIEF)))
           )
       )
        leave_corpse = TRUE;

    if (IS_IMMORTAL(ch)) {
        leave_corpse = FALSE;
        corpse->timer = 10000;
    }

    /* target is only ever a player, so if we were passed a vnum in argument, then a mob killed ch,
     * so find out if the mob was an executioner and if it was, set their corpse accordingly */
    if (!IS_NPC(ch) && (target_vnum == 0 || (!target && target_vnum > 0)) && (IS_SET(ch->act, PLR_KILLER) || IS_SET(ch->act, PLR_THIEF))) {
        MOB_INDEX_DATA *pMob;

        if (target_vnum == 0 || ((pMob = get_mob_index(target_vnum)) && pMob->spec_fun && !str_cmp(rev_spec_lookup((void *)pMob->spec_fun), "spec_executioner"))) {
            corpse->value[0] = 1;
            corpse->value[3] = number_range(3, 6);
        }
    }

    sprintf(buf, corpse->short_descr, name);
    free_string(corpse->short_descr);
    corpse->short_descr = str_dup(buf);

    sprintf(buf, corpse->description, name);
    free_string(corpse->description);
    corpse->description = str_dup(buf);

    if (IS_NPC(ch) && ch->pIndexData) {
        switch (ch->pIndexData->vnum) {
            case MOB_VNUM_WATERELEM:
            case MOB_VNUM_SKELETON:
            case MOB_VNUM_FIREELEM:
            case MOB_VNUM_EARTHELEM:
            case MOB_VNUM_IRON_GOLEM:
            case MOB_VNUM_DIAMOND_GOLEM:
            case MOB_VNUM_SOUL_THIEF:
            case MOB_VNUM_HOLY_AVENGER:
            case MOB_VNUM_PEGASUS:
            case MOB_VNUM_NIGHTMARE:
            case MOB_VNUM_ELEM_BEAST:
            case MOB_VNUM_INT_DEVOURER:
            case MOB_VNUM_SHADOW_HOUND:
            case MOB_VNUM_SHADOWDRAGON:
                corpse->level = 0;
                break;
            default:
                break;
        }
    }

    for (obj = ch->first_carry; obj != NULL; obj = obj_next) {
        obj_next = obj->next_in_carry_list;

        if (IS_NPC(ch) || leave_corpse || (!IS_NPC(ch) && !IS_SET(obj->extra_flags, ITEM_UNIQUE))) {
            obj_from_char(obj);
            if ((obj == quest_object)
                && (ch == quest_target)
                ) {
                obj->value[0] = UMAX(1, obj->value[0] * UMAX(quest_timer, 10) / 10);
                obj->value[1] = UMAX(1, obj->value[1] * UMAX(quest_timer, 10) / 10);
                obj->value[2] = UMAX(1, obj->value[2] * UMAX(quest_timer, 10) / 10);
            }
            if (IS_SET(obj->extra_flags, ITEM_INVENTORY))
                extract_obj(obj);
            else
                obj_to_obj(obj, corpse);
        }
        else
            REMOVE_BIT(obj->item_apply, ITEM_APPLY_HEATED);
    }

    if (!IS_NPC(ch)) {
        corpse->level = get_pseudo_level(ch);

        if (leave_corpse) {
            ch->pcdata->safetimer = current_time + 120;
            obj_to_room(corpse, ch->in_room);

            if (IS_SET(ch->in_room->affected_by, ROOM_BV_ENCAPS)) {
                ROOM_AFFECT_DATA   *raf;

                for (raf = ch->in_room->first_room_affect; raf != NULL; raf = raf->next)
                    if (raf->bitvector == ROOM_BV_ENCAPS)
                        raf->duration = 0;
            }
        }
        else
            obj_to_room(corpse, get_room_index(ROOM_VNUM_MORGUE));

        {
            CORPSE_DATA        *this_corpse;

            GET_FREE(this_corpse, corpse_free);
            this_corpse->next = NULL;
            this_corpse->prev = NULL;
            this_corpse->this_corpse = corpse;
            LINK(this_corpse, first_corpse, last_corpse, next, prev);
        }
        save_corpses();
        return;
    }
    else {
        RESET_DATA         *pReset;
        MOB_INDEX_DATA     *pMobIndex = NULL;
        ROOM_INDEX_DATA    *pRoomIndex = NULL;
        OBJ_INDEX_DATA     *pObjIndex = NULL;
        int                 chance;

        if (ch->in_room != NULL) {
            for (pReset = ch->in_room->area->first_reset; pReset != NULL; pReset = pReset->next) {
                if (pReset->command == 'M') {
                    if ((pMobIndex = get_mob_index(pReset->arg1)) == NULL)
                        continue;
                    if ((pRoomIndex = get_room_index(pReset->arg3)) == NULL)
                        continue;
                    if (pMobIndex->vnum != ch->pIndexData->vnum)
                        continue;
                    if (pRoomIndex->vnum != ch->in_room->vnum)
                        continue;
                }
                if (pReset->command == 'C') {
                    if (pMobIndex == NULL || (pMobIndex && pMobIndex != ch->pIndexData)
                        )
                        continue;

                    if ((pObjIndex = get_obj_index(pReset->arg1)) == NULL)
                        continue;

                    if ((obj = create_object(pObjIndex, pObjIndex->level)) == NULL)
                        continue;

                    obj_to_obj(obj, corpse);

                    chance = pReset->arg3;

                    if (IS_SET(pObjIndex->extra_flags, ITEM_RARE)) {
                        if (pObjIndex->rarity == 0)
                            chance = 625;
                        else
                            chance = pObjIndex->rarity;
                    }
                    if (number_range(1, chance) != number_range(1, chance))
                        extract_obj(obj);
                }
            }
        }

        obj_to_room(corpse, ch->in_room);
        return;
    }

}

/*
 * Improved Death_cry contributed by Diavolo.
 */
void
death_cry(CHAR_DATA *ch)
{
    /* FIXME:  *ONLY* PKOK victims get a head loaded. */
    /* Make this give 'cry' message, don't load anything */
    ROOM_INDEX_DATA    *was_in_room;
    char               *msg;
    int                 door;

    if (IS_NPC(ch))
        msg = "You hear something's death cry.";
    else
        msg = "You hear someone's death cry.";

    was_in_room = ch->in_room;
    if ((was_in_room == NULL)
        || (was_in_room->vnum == 0))
        return;

    for (door = 0; door <= 5; door++) {
        EXIT_DATA          *pexit;

        if ((pexit = was_in_room->exit[door]) != NULL && pexit->to_room != NULL && pexit->to_room != was_in_room) {
            ch->in_room = pexit->to_room;
            act(msg, ch, NULL, NULL, TO_ROOM);
        }
    }
    ch->in_room = was_in_room;

    return;
}

void
raw_kill(CHAR_DATA *victim, char *argument)
{
    CHAR_DATA          *check;
    char                arg[MAX_STRING_LENGTH];
    char                buf[MAX_STRING_LENGTH];
    QUEST_DATA         *questinfo;
    QUEST_DATA         *questcheck;
    extern OBJ_DATA    *quest_object;
    extern char        *quest_target_name;
    sh_int              questcount = 0;

    one_argument(argument, arg);

    stop_fighting(victim, TRUE);
    mprog_death_trigger(victim);
    if (victim == quest_mob) {

        /* weedermod */
        if (arg[0] != '\0' && !is_number(arg)) {
            arg[0] = UPPER(arg[0]);
            sprintf(buf, "@@lSOMEONE SAVE ME!!! @@e%s@@l is @@RKILLING@@l me and the @@yquest@@l will soon be @@eOVER!!@@N", arg);
        }
        else
            sprintf(buf, "Oh well, I guess the quest is over, since I am about to @@eDIE!!!!!@@N");

        do_crusade(victim, buf);

        /* qinfo */
        for (questcheck = first_quest; questcheck != NULL; questcheck = questcheck->next) {
            questcount++;
        }

        questcheck = first_quest;

        if (questcount >= MAX_QINFO && questcheck != NULL) {
            UNLINK(questcheck, first_quest, last_quest, next, prev);
            free_string(questcheck->mob);
            free_string(questcheck->thief);
            free_string(questcheck->ch);
            PUT_FREE(questcheck, quest_free);
        }

        GET_FREE(questinfo, quest_free);
        if (quest_mob) {
            questinfo->mob = str_dup(NAME(quest_mob));
        }
        else
            questinfo->mob = str_dup("");
        if (quest_target_name != '\0')
            questinfo->thief = str_dup(quest_target_name);
        else
            questinfo->thief = str_dup("");

        if (!is_number(arg))
            questinfo->ch = str_dup(arg);
        else
            questinfo->ch = str_dup("");

        if (quest_object)
            questinfo->qp = quest_object->value[0];
        else
            questinfo->qp = -1;

        questinfo->flags = 0;
        SET_BIT(questinfo->flags, QUEST_KILLED);
        LINK(questinfo, first_quest, last_quest, next, prev);
        /* qinfo end */

        clear_quest();
    }

    if (victim->is_free == FALSE && victim->in_room != NULL)
        make_corpse(victim, arg);

    /* weedermod: unset quest target AFTER corpse is made */
    if (victim == quest_target)
        quest_target = NULL;

    for (check = first_char; check != NULL; check = check->next) {
        extern CHAR_DATA   *violence_marker;

        if ((check != violence_marker) && check->hunting == victim)
            end_hunt(check);
        /*        unhunt(check); */
    }

    /* Check ch->sentence to see if time to remove flags.. :) */

    /* Obsolete with new sentence code... Ramias

       if ( !IS_NPC( victim ) )
       {
       if ( IS_SET( victim->act, PLR_THIEF ) )
       {
       if ( victim->sentence > 0 )
       victim->sentence--;
       if ( victim->sentence == 0 )
       REMOVE_BIT( victim->act, PLR_THIEF );
       }

       if ( IS_SET( victim->act, PLR_KILLER ) )
       {
       if ( victim->sentence > 0 )
       victim->sentence--;
       if ( victim->sentence == 0 )
       REMOVE_BIT( victim->act, PLR_KILLER ); 
       }     
       }

     */

    if (IS_NPC(victim)) {
        victim->pIndexData->killed++;
        kill_table[URANGE(0, victim->level, MAX_LEVEL - 1)].killed++;
        extract_char(victim, TRUE);
        return;

    }

    extract_char(victim, FALSE);
    while (victim->first_affect)
        affect_remove(victim, victim->first_affect);
    victim->affected_by = 0;

/* this shouldn't be necessary as all objects (except sometimes uniques, now) are removed
 * from the char, and all affects are removed.. so ->armor shouldn't need to be reset.
    victim->armor = 100;
*/
    victim->position = POS_RESTING;
    victim->hit = UMAX(1, victim->hit);
    victim->mana = UMAX(1, victim->mana);
    victim->move = UMAX(1, victim->move);
    save_char_obj(victim);
    return;
}

void
group_gain(CHAR_DATA *ch, CHAR_DATA *victim)
{
    char                buf[MAX_STRING_LENGTH];
    CHAR_DATA          *gch;
    CHAR_DATA          *lch;
    int                 members;
    int                 huggy;    /* To work out exp gained */
    int                 funky;    /* Hope you LOVE these var names, Mag */
    int                 base;
    extern bool         dbl_xp;

    /* If a mob kills something, or the victim was a player, or the victim was
     * themselves, no xp is awarded */
    if (IS_NPC(ch) || !IS_NPC(victim) || victim == ch)
        return;

    members = 0;
    huggy = 0;

    base = victim->exp;            /* Now share this out... */

    for (gch = ch->in_room->first_person; gch != NULL; gch = gch->next_in_room) {
        if (is_same_group(gch, ch)) {
            members++;
            huggy += UMAX(get_pseudo_level(gch), (get_pseudo_level(ch) - 25));
        }
    }

    if (members == 0) {
        bugf("Group_gain: members [%d]", members);
        members = 1;
        huggy = get_pseudo_level(ch);
    }

    /* Bonus for grouping */

    if (members < 2);            /* Changed from funky */
    else if (members < 3)
        base *= (5 / 2);        /* funky wasn't initialised */
    else if (members < 4)
        base *= 3;                /* anyways.. hmm MAG */
    else if (members < 5)
        base *= (7 / 2);
    else if (members < 6)
        base *= 4;
    else
        base *= (9 / 2);

    /* Question is, if someone's exp is capped, do you dole out the rest
       to the other people? or just reduce the total amount given to the
       group (by not doling it out).
       As it is, the total exp is reduced. I'll leave it that way.

     */

    lch = (ch->leader != NULL) ? ch->leader : ch;

    if (IS_DEBUGGER(ch))
        sendf(ch, "Base[%d] Members[%d] Huggy[%d]\n\r", base, members, huggy);

    for (gch = ch->in_room->first_person; gch != NULL; gch = gch->next_in_room) {
        OBJ_DATA           *obj;
        OBJ_DATA           *obj_next;
        int                 align;

        if (!is_same_group(gch, ch))
            continue;

        /* Calc each char's xp seperately, but mult by ch->lev/tot_group_lev. */

        if (members > 1)
            funky = (base / huggy) * get_pseudo_level(gch);    /* gch's % of exp gained */
        else
            funky = base;

        if (IS_DEBUGGER(ch))
            sendf(ch, "Funky[%d]\n\r", funky);

        /* Capping changed.  -S- */
        /* Changed YET again -S- */
        /*
           funky = UMIN( funky, ( 1.5 * exp_table[gch->level].mob_base ) );        
           funky = UMAX( 0, funky );        
         */

        /* Now the max is just 250K */
        if (funky < 0)
            funky = 823421;

        /* only cap if they don't have custom xp set */
        if (victim->pIndexData->custom_xp == -1)
            funky = UMIN(funky, 1500000);
        else
            funky = UMIN(funky, victim->pIndexData->custom_xp);

        if (victim->pIndexData->custom_minlev != -1 && victim->pIndexData->custom_maxlev != -1) {
            if (get_pseudo_level(gch) >= victim->pIndexData->custom_minlev && get_pseudo_level(gch) <= victim->pIndexData->custom_maxlev)
                ;
            else {
                funky = (funky / 5000);
            }
        }
        else if ((abs((get_pseudo_level(gch) - get_pseudo_level(victim))) > 23)
            || (get_pseudo_level(gch) > (get_pseudo_level(victim) + 17))
            ) {
            funky = (funky / 5000);
        }

        if (gch->adept_level > 0)
            funky /= 1000;

        if (dbl_xp) {
            funky = funky * 2;
        }

        if (!IS_NPC(gch) && gch->desc == NULL)
            funky = 0;

        /* a negative result means we would have wrapped into negative territory */
        if (funky > 0 && gch->exp + funky < 0) {
            send_to_char("You have reached the XP cap. You will receive no more XP!\n\r", gch);
            gch->exp = 2147483647;
        }
        else {
            sprintf(buf, "@@NYou receive @@c%s @@gexperience points.@@N\n\r", number_comma(funky));
            send_to_char(buf, gch);

            gain_exp(gch, funky);
        }

        if (!IS_NPC(gch) && (gch->pcdata->learned[gsn_emotion_control] < 73)) {
            align = gch->alignment - (victim->alignment * (80 - gch->pcdata->learned[gsn_emotion_control]) / 100);

            if (align > 500)
                gch->alignment = UMIN(gch->alignment + (align - 500) / 4, 1000);
            else if (align < -500)
                gch->alignment = UMAX(gch->alignment + (align + 500) / 4, -1000);
            else
                gch->alignment -= gch->alignment / 4;
        }
        for (obj = ch->first_carry; obj != NULL; obj = obj_next) {
            obj_next = obj->next_in_carry_list;
            if (obj->wear_loc == WEAR_NONE)
                continue;

            if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch))
                || (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch))
                || (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch))) {

                act("You are zapped by $p.", ch, obj, NULL, TO_CHAR);
                act("$n is zapped by $p.", ch, obj, NULL, TO_ROOM);
                obj_from_char(obj);
                obj_to_room(obj, ch->in_room);
            }
        }
    }

    return;
}

struct dam_table_str
{
    int                 min_dam;
    const char         *col;
    const char         *vs;
    const char         *vp;
    const char         *str;
};
struct dam_table_str dam_table[] = {
    {2900, "@@l", "!!!!VIVISECT!!!!",   "!!!!VIVISECTS!!!!",   " into a living corpse with"},
    {2400, "@@m", "!!!!OBLITERATE!!!!", "!!!!OBLITERATES!!!!", " into disappearing particles with"},
    {2100, "@@m", "!!!!DEVASTATE!!!!",  "!!!!DEVASTATES!!!!",  "'s very existence with"},
    {1700, "@@m", "!!!!MUTILATE!!!!",   "!!!!MUTILATES!!!!",   " with"},
    {1450, "@@e", "****ANNIHILATE****", "****ANNIHILATES****", " into the bloody ground with"},
    {1375, "@@e", "***EVISCERATE***",   "***EVISCERATES***",   " into lots of small pieces with"},
    {1300, "@@e", "**DEMOLISH**",       "**DEMOLISHES**",      ", spraying $S guts all over the ground with"},
    {1200, "@@R", "*PULVERISE*",        "*PULVERISES*",        ", spreading blood and gore over the floor with"},
    {1150, "@@R", "THWACK",             "THWACKS",             ", leaving $M dazed and reeling with"},
    {1100, "@@R", "annihilate",         "annihilates",         " with"},
    { 900, "@@p", "eviscerate",         "eviscerates",         " with"},
    { 850, "@@p", "demolish",           "demolishes",          " with"},
    { 800, "@@p", "pulverise",          "pulverises",          " with"},
    { 700, "@@y", "thwack",             "thwacks",             " with"},
    { 650, "@@y", "flay",               "flays",               "'s body mercillesly with"},
    { 600, "@@y", "lacerate",           "lacerates",           " into a map of the mud with"},
    { 500, "@@G", "maul",               "mauls",               " with great vengeance and FURIOUS anger from"},
    { 450, "@@G", "tear",               "tears",               "'s skin into shreds with"},
    { 400, "@@r", "rip apart",          "rips apart",          "'s skull with"},
    { 350, "@@r", "remove",             "removes",             "'s ability to have children from"},
    { 300, "@@c", "smash",              "smashes",             " forcefully with"},
    { 250, "@@c", "cause",              "causes",              " to scream in agony from"},
    { 200, "@@c", "laugh at",           "laughs at",           " as $e draws trails of blood with"},
    { 100, "@@a", "make",               "makes",               " wince in pain from"},
    {  50, "@@a", "mark",               "marks",               " with an X from"},
    {  28, "@@a", "wound",              "wounds",              " painfully with"},
    {  20, "@@b", "graze",              "grazes",              " with"},
    {  10, "@@b", "bonk",               "bonks",               " on the head with"},
    {   7, "@@b", "scratch",            "scratches",           " roughly with"},
    {   5, "@@b", "nick",               "nicks",               " gently with"},
    {   2, "@@b", "tickle",             "tickles",             " softly with"},
    {   0, "@@g", "fail to hit",        "fails to hit",        " with"}
};

void
dam_message(CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt)
{
    static char        *const attack_table[] = {
        "hit",
        "slice", "stab", "slash", "whip", "claw",
        "blast", "pound", "crush", "grip", "bite",
        "pierce", "suction", "tail whip"
    };

    char                buf1[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH], buf3[MAX_STRING_LENGTH];
    const char         *vs;        /* Singular */
    const char         *vp;        /* Plural   */
    const char         *str;    /* Strength */
    const char         *attack;
    const char         *col;
    char                punct;

    int                 dam_table_num, check_dt;
    bool                dead = FALSE;
    int                 odam;

    odam = dam;

    if (IS_AFFECTED(victim, AFF_SANCTUARY)
        || item_has_apply(victim, ITEM_APPLY_SANC))
        dam /= 2;

    if ((IS_AFFECTED(victim, AFF_PROTECT) || item_has_apply(ch, ITEM_APPLY_PROT))
        && IS_EVIL(ch))
        dam -= dam / 4;

    if (dam < 0)
        dam = 0;

    if (dam >= victim->hit)
        dead = TRUE;

    for (dam_table_num = 0; dam_table[dam_table_num].min_dam > dam; dam_table_num++);

    col = dam_table[dam_table_num].col;
    vs = dam_table[dam_table_num].vs;
    vp = dam_table[dam_table_num].vp;
    str = dam_table[dam_table_num].str;

    punct = (dam <= 64) ? '.' : '!';

    /* Use combat skills, etc to make unarmed combat more fun :) */

    if (dt == TYPE_MARTIAL) {
        col = "@@g";

        if (dam == 0) { vs = "miss"; vp = "misses"; }
        else
            switch (number_range(0, 13)) {
                case  0: vs = "head punch";   vp = "head punches";   break;
                case  1: vs = "high kick";    vp = "high kicks";     break;
                case  2: vs = "vital kick";   vp = "vital kicks";     break;
                case  3: vs = "head bash";    vp = "head bashes";    break;
                case  4: vs = "side kick";    vp = "side kicks";     break;
                case  5: vs = "elbow";        vp = "elbows";         break;
                case  6: vs = "body punch";   vp = "body punches";   break;
                case  7: vs = "low kick";     vp = "low kicks";      break;
                case  8: vs = "graze";        vp = "grazes";         break;
                case  9: vs = "knee smash";   vp = "knee smashes";   break;
                case 10: vs = "kidney punch"; vp = "kidney punches"; break;
                case 11: vs = "arm twist";    vp = "arm twists";     break;
                case 12: vs = "uppercut";     vp = "uppercuts";      break;
                case 13: vs = "rabbit punch"; vp = "rabbit punches"; break;
                default: vs = "foot sweep";   vp = "foot sweeps";    break;
            }
    }
    else if (dt == TYPE_HIT) {
        col = "@@g";

        if      (dam ==   0) { vs = "miss";    vp = "misses";   }
        else if (dam <=  10) { vs = "tickle";  vp = "tickles";  }
        else if (dam <=  30) { vs = "glance";  vp = "glances";  }
        else if (dam <=  50) { vs = "strike";  vp = "strikes";  }
        else if (dam <=  75) { vs = "whallop"; vp = "whallops"; }
        else if (dam <= 100) { vs = "maul";    vp = "mauls";    }
        else                 { vs = "thwack";  vp = "thwacks";  }
    }
    
    /* TYPE_IGNORE means hide message */
    if (dt != TYPE_IGNORE) {
        if (dt >= 0 && dt < MAX_SKILL)
            attack = skill_table[dt].noun_damage;
        else if (dt >= TYPE_HIT && dt < TYPE_HIT + sizeof(attack_table) / sizeof(attack_table[0])) {
            check_dt = UMAX(0, (dt - TYPE_HIT));
            check_dt = UMIN(check_dt, 12);
            attack = attack_table[dt - TYPE_HIT];
        }
        else if (dt != TYPE_MARTIAL) {
            bugf("Dam_message: bad dt %d.", dt);
            dt = TYPE_HIT;
            attack = attack_table[0];
        }

        if (dt != TYPE_MARTIAL && dt != TYPE_HIT) {
            /* buf1 is shown to people in the room. they never see the damage, unless its a spar */
            sprintf(buf1, "%s$n %s%s $N%s%s $s %s%c%s@@N", col, col, vp, col, str, attack, punct, showdamage(NULL, ch, victim, odam, FALSE));

            /* buf2 is shown to the person doing the damage */
            sprintf(buf2, "%sYou %s%s $N%s%s your %s%c%s@@N", col, col, vs, col, str, attack, punct, showdamage(ch, ch, victim, odam, FALSE));

            /* buf3 is shown to the person receiving the damage */
            if (*str == '\'')
                sprintf(buf3, "%s$n %s%s your%s%s $s %s%c%s@@N", col, col, vp, col, str + 2, attack, punct, showdamage(victim, ch, victim, odam, FALSE));
            else
                sprintf(buf3, "%s$n %s%s you%s%s $s %s%c%s@@N", col, col, vp, col, str, attack, punct, showdamage(victim, ch, victim, odam, FALSE));
        }
        else {
            /* mob attack, or martial attack (show no 'attack type') */
            sprintf(buf1, "%s$n %s%s $N%s%c%s@@N", col, col, vp, col, punct, showdamage(NULL, ch, victim, odam, FALSE));
            sprintf(buf2, "%sYou %s%s $N%s%c%s@@N", col, col, vs, col, punct, showdamage(ch, ch, victim, odam, FALSE));
            sprintf(buf3, "%s$n %s%s you%s%c%s@@N", col, col, vp, col, punct, showdamage(victim, ch, victim, odam, FALSE));
        }

        act(buf1, ch, NULL, victim, TO_NOTVICT);
        act(buf2, ch, NULL, victim, TO_CHAR);
        act(buf3, ch, NULL, victim, TO_VICT);
    }

    if (dead) {
        int                 foo;

        foo = sizeof(attack_table) / sizeof(attack_table[0]);
        death_message(ch, victim, dt, foo);
    }

    return;
}

/*
 * Disarm a creature.
 * Caller must check for successful attack.
 */
void
disarm(CHAR_DATA *ch, CHAR_DATA *victim)
{
    OBJ_DATA           *obj, *obj2;
    int                 chance;

    set_fighting(ch, victim, TRUE);

    obj = get_eq_char(victim, WEAR_WIELD);
    obj2 = get_eq_char(victim, WEAR_WIELD_2);

    if (obj == NULL /* && obj2 == NULL */) {
        send_to_char("Your opponent is not wielding a weapon.\n\r", ch);
        return;
    }

    /* if they're dualwielding, pick a random weapon instead of always
     * trying to disarm their main one */

/*  players may be used to having a secondary weapon not being able to be
    disarmed, and thus might not have set it as nodisarm.. add this when
    you can be assured everyone knows.

    if (obj && obj2 && number_bits(1))
        obj = obj2;
    else if (!obj && obj2)
        obj = obj2;
*/

    if (IS_SET(obj->extra_flags, ITEM_NODISARM)) {
        send_to_char("Your opponents weapon cannot be disarmed.\n\r", ch);
        return;
    }

    /* can't be disarmed in a duel */
    if (is_in_duel(victim, DUEL_STAGE_GO))
        return;

    chance = IS_NPC(victim) ? IS_SET(victim->skills, MOB_NODISARM) ? 90 : 0 : victim->pcdata->learned[gsn_nodisarm];

    if (number_percent() < chance && number_bits(3)) {
        act("You dodge $n's disarm attempt!", ch, NULL, victim, TO_VICT);
        act("You fail to disarm $N!", ch, NULL, victim, TO_CHAR);
        act("$N dodges $n's disarm attempt!", ch, NULL, victim, TO_NOTVICT);
        return;
    }

    act("$n DISARMS you!", ch, NULL, victim, TO_VICT);
    act("You disarm $N!", ch, NULL, victim, TO_CHAR);
    act("$n DISARMS $N!", ch, NULL, victim, TO_NOTVICT);

    obj_from_char(obj);
    obj_to_room(obj, victim->in_room);

    return;
}

/*
 * Trip a creature.
 * Caller must check for successful attack.
 */
void
trip(CHAR_DATA *ch, CHAR_DATA *victim)
{
    if (victim->wait == 0) {
        int                 chance;

        chance = IS_NPC(victim)
            ? IS_SET(victim->skills, MOB_NOTRIP) ? 75 : 0 : victim->pcdata->learned[gsn_notrip];

        /* give executioners a better chance at tripping *grin* */
        if (IS_NPC(ch) && ch->spec_fun && !str_cmp(rev_spec_lookup((void *) ch->spec_fun), "spec_executioner"))
            chance = 50;

        /* Check for no-trip */
        if (number_percent() < chance) {
            act("You sidestep $n's attempt to trip you!", ch, NULL, victim, TO_VICT);
            act("$N sidesteps your attempt to trip $M!", ch, NULL, victim, TO_CHAR);
            act("$N sidesteps $n's attempt to trip $m!", ch, NULL, victim, TO_NOTVICT);
            return;
        }

        act("$n trips you and you go down!", ch, NULL, victim, TO_VICT);
        act("You trip $N and $N goes down!", ch, NULL, victim, TO_CHAR);
        act("$n trips $N and $N goes down!", ch, NULL, victim, TO_NOTVICT);

        WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
        WAIT_STATE(victim, 2 * PULSE_VIOLENCE);
    }

    return;
}

void
do_kill(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
        send_to_char("Kill whom?\n\r", ch);
        return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (!IS_NPC(victim)) {
        if (!IS_SET(victim->act, PLR_KILLER)
            && !IS_SET(victim->act, PLR_THIEF)) {
            send_to_char("You must MURDER a player.\n\r", ch);
            return;
        }
    }
    else {
        if (IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL) {
            send_to_char("You must MURDER a charmed creature.\n\r", ch);
            return;
        }
    }

    if (victim == ch) {
        send_to_char("You hit yourself.  Ouch!\n\r", ch);
        multi_hit(ch, ch, TYPE_UNDEFINED);
        return;
    }

    if (is_safe(ch, victim, TRUE))
        return;

    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
        act("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (ch->position == POS_FIGHTING) {
        send_to_char("You do the best you can!\n\r", ch);
        return;
    }

    WAIT_STATE(ch, 1 * PULSE_VIOLENCE);
    check_killer(ch, victim);
    multi_hit(ch, victim, TYPE_UNDEFINED);
    return;
}

void
do_target(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim;

    if ((!IS_NPC(ch))
        && (ch->pcdata->learned[gsn_target] < 65)) {
        send_to_char("You are not trained enough in this skill!!\n\r", ch);
        return;
    }
    one_argument(argument, arg);

    if (arg[0] == '\0') {
        send_to_char("Target whom?\n\r", ch);
        return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (!IS_NPC(victim)) {
        if (!IS_SET(victim->act, PLR_KILLER)
            && !IS_SET(victim->act, PLR_THIEF)) {
            send_to_char("You must MURDER a player.\n\r", ch);
            return;
        }
    }
    else {
        if (IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL) {
            send_to_char("You must MURDER a charmed creature.\n\r", ch);
            return;
        }
    }

    if (victim == ch) {
        send_to_char("You hit yourself.  Ouch!\n\r", ch);
        one_hit(ch, ch, TYPE_UNDEFINED);
        return;
    }

    if (is_safe(ch, victim, TRUE))
        return;

    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
        act("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (ch->position == POS_FIGHTING) {
        send_to_char("@@rTracking, tracking, tracking...@@eGOT HIM!!!@@N\n\r", ch);
        stop_fighting(ch, FALSE);
    }

    WAIT_STATE(ch, 1 * PULSE_VIOLENCE);
    check_killer(ch, victim);
    one_hit(ch, victim, TYPE_UNDEFINED);
    return;
}

void
do_murde(CHAR_DATA *ch, char *argument)
{
    send_to_char("If you want to MURDER, spell it out.\n\r", ch);
    return;
}

void
do_murder(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    char                arg[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
        send_to_char("Murder whom?\n\r", ch);
        return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (victim == ch) {
        send_to_char("Suicide is a mortal sin.\n\r", ch);
        return;
    }

    if (is_safe(ch, victim, TRUE)) {
        send_to_char("Not here.\n\r", ch);
        return;
    }

    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
        act("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (ch->position == POS_FIGHTING) {
        send_to_char("You do the best you can!\n\r", ch);
        return;
    }

    WAIT_STATE(ch, 1 * PULSE_VIOLENCE);

    if (IS_NPC(ch) || IS_NPC(victim) || !IS_SET(ch->pcdata->pflags, PFLAG_PKOK) || !IS_SET(victim->pcdata->pflags, PFLAG_PKOK)) {
        /* If not pkok people, do yell. */
        sprintf(buf, "Help! I'M BEING ATTACKED!!! ARRRGGGHHHHHH!");
        do_yell(victim, buf);
    }

    check_killer(ch, victim);
    multi_hit(ch, victim, TYPE_UNDEFINED);
    return;
}

void
do_backstab(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim;
    OBJ_DATA           *obj;
    int                 cnt;
    int                 best = -1;
    int                 level = 0;
    int                 mult;
    int                 chance;
    int                 dam;
    bool                crack = FALSE;
    extern int          pixbonus;
    int                 beats = 0;

    if (!IS_NPC(ch)) {
        for (cnt = 0; cnt < MAX_CLASS; cnt++)
            if (ch->lvl[cnt] >= skill_table[gsn_backstab].skill_level[cnt] && ch->lvl[cnt] > level) {
                best = cnt;
                level = ch->lvl[cnt];
            }

        if (ch->lvl2[1] > 0)
            level = level + ch->lvl2[1] / 2;
    }
    else {
        best = ch->class;
        level = ch->level;
    }

    if (best == -1) {
        send_to_char("You better leave the assassin trade to thieves.\n\r", ch);
        return;
    }

    one_argument(argument, arg);

    if (arg[0] == '\0') {
        send_to_char("Backstab whom?\n\r", ch);
        return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }
    if (victim == NULL || victim->is_free != FALSE)
        return;

    if ((IS_NPC(victim) && IS_SET(victim->act, ACT_NO_BODY)) || (!IS_NPC(victim) && IS_SET(victim->pcdata->pflags, PFLAG_NO_BODY))) {
        act("$N has no body to backstab!", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (victim == ch) {
        send_to_char("How can you sneak up on yourself?\n\r", ch);
        return;
    }

    if (is_safe(ch, victim, TRUE))
        return;

    if ((obj = get_eq_char(ch, WEAR_WIELD)) == NULL || ((obj->value[3] != 2) && (obj->value[3] != 11))) {
        send_to_char("You need to wield a piercing or a stabbing weapon.\n\r", ch);
        return;
    }

    if (victim->fighting != NULL) {
        send_to_char("You can't backstab a fighting person.\n\r", ch);
        return;
    }

    /* Backstab Counters by -Ogma- */
    if (!IS_NPC(ch) && !IS_NPC(victim)) {
        ch->pcdata->circles_attempted++;
    }

    chance = (IS_NPC(ch) ? 50 : ch->pcdata->learned[gsn_backstab] / 2);

    if (!IS_AWAKE(victim))
        chance += 75;

    if (IS_AFFECTED(victim, AFF_SNEAK) || item_has_apply(victim, ITEM_APPLY_SNEAK))
        chance -= 10;

    if (IS_AFFECTED(ch, AFF_SNEAK) || item_has_apply(ch, ITEM_APPLY_SNEAK))
        chance += 20;

    if (IS_NPC(victim))
        chance += (IS_AFFECTED(victim, AFF_DETECT_INVIS) ? -20 : 20);

    if (IS_AFFECTED(victim, AFF_INVISIBLE) || item_has_apply(victim, ITEM_APPLY_INV))
        chance -= 10;

    if (!IS_NPC(victim))
        chance += 20;

    if (get_pseudo_level(ch) >= get_pseudo_level(victim))
        chance += 10;
    else
        chance -= 10;

    /* Work out multiplier */
    mult = 1 + (level >= 20) + (level >= 50) + (level >= 90) + (level >= 120);

    /* Work out damage */
    if (obj->item_type == ITEM_WEAPON)
        dam = number_range(obj->value[1], obj->value[2]);
    else
        dam = number_range(ch->level / 3, ch->level / 2);

    dam += number_range(level / 2, level * 2) + GET_DAMROLL(ch) / 2;
    dam *= mult;
    check_killer(ch, victim);

    if (!IS_NPC(victim) && victim->race == RACE_PIX && number_percent() < pixbonus) {
        act("$n tries to backstab $N, but MISSES!",  ch, NULL, victim, TO_NOTVICT);
        act("You try to backstab $N, but MISS!",     ch, NULL, victim, TO_CHAR);
        act("$N tries to backstab you, but MISSES!", victim, NULL, ch, TO_CHAR);
        damage(ch, victim, 0, TYPE_IGNORE);
    }
    else if (chance < number_percent()) {
        act("$n tries to backstab $N, but misses!",  ch, NULL, victim, TO_NOTVICT);
        act("You try to backstab $N, but miss!",     ch, NULL, victim, TO_CHAR);
        act("$N tries to backstab you, but misses!", victim, NULL, ch, TO_CHAR);
        damage(ch, victim, 0, TYPE_IGNORE);
    }
    else {
        char buf[MSL];

        if (!IS_NPC(ch) && ch->pcdata->order[0] == 2 && (number_range(1, 150) == number_range(1, 150))) {
            dam *= 2;
            crack = TRUE;
        }

        /* Backstab Counters added by -Ogma- */
        if (!IS_NPC(ch) && !IS_NPC(victim)) {
            ch->pcdata->circles_landed++;
        }

        sprintf(buf, "$n places $p into the back of $N!!%s@@N", showdamage(NULL, ch, victim, dam, crack));
        act(buf, ch, obj, victim, TO_NOTVICT);

        sprintf(buf, "You place $p into the back of $N!!%s@@N", showdamage(ch, ch, victim, dam, crack));
        act(buf, ch, obj, victim, TO_CHAR);

        sprintf(buf, "$N places $p into your back.  OUCH!%s@@N", showdamage(victim, ch, victim, dam, crack));
        act(buf, victim, obj, ch, TO_CHAR);

        if (crack) {
            send_to_room("You hear a large CRACK!\n\r", ch->in_room);

            if (ch->lvl2[1] > 50) {
                send_to_char("@@NThe CRACK has left you @@aSTUNNED@@N!!!\n\r", victim);
                if (victim->stunTimer == 0)
                    victim->stunTimer += number_range(1, 2);
            }

            damage(ch, victim, dam, TYPE_CRACK);
        }
        else
            damage(ch, victim, dam, TYPE_IGNORE);
    }

    beats = skill_table[gsn_backstab].beats;

    if (!IS_NPC(ch) && is_affected(ch, gsn_innerflame_novice))
        beats -= 2;
    else if (!IS_NPC(ch) && is_affected(ch, gsn_innerflame_intermediate))
        beats -= 4;
    else if (!IS_NPC(ch) && is_affected(ch, gsn_innerflame_advanced))
        beats -= 6;
    else if (!IS_NPC(ch) && is_affected(ch, gsn_innerflame_expert))
        beats -= 8;
    else if (!IS_NPC(ch) && is_affected(ch, gsn_innerflame_master))
        beats -= 12;

    WAIT_STATE(ch, beats);
    return;
}

void
do_flee(CHAR_DATA *ch, char *argument)
{
    ROOM_INDEX_DATA    *was_in;
    ROOM_INDEX_DATA    *now_in;
    CHAR_DATA          *victim;
    char                buf[MAX_STRING_LENGTH];
    int                 attempt;
    int                 cost;    /* xp cost for a flee */

    if ((victim = ch->fighting) == NULL) {
        if (ch->position == POS_FIGHTING)
            ch->position = POS_STANDING;
        send_to_char("You aren't fighting anyone.\n\r", ch);
        return;
    }

    if (IS_NPC(victim) && IS_SET(victim->act, ACT_NO_FLEE)) {
        send_to_char("You can't flee from this mob!\n\r", ch);
        return;
    }

    if (IS_NPC(ch) && IS_SET(ch->act, ACT_SENTINEL))
        return;

    /* Check if mob will "allow" ch to flee... */

    if (IS_SET(victim->act, ACT_NO_FLEE) && !IS_NPC(ch) && IS_NPC(victim)) {
        send_to_char("You attempt to flee from battle, but fail!\n\r", ch);
        sprintf(buf, "%s tells you 'No way will you escape ME!!'\n\r", victim->short_descr);
        send_to_char(buf, ch);
        return;
    }

    if (!IS_NPC(ch) && is_in_duel(ch, DUEL_STAGE_GO))
        return;

    was_in = ch->in_room;
    for (attempt = 0; attempt < 6; attempt++) {
        EXIT_DATA          *pexit;
        int                 door;

        door = number_door();
        if ((pexit = was_in->exit[door]) == 0 || pexit->to_room == NULL || IS_SET(pexit->exit_info, EX_CLOSED)
            || (IS_NPC(ch)
                && (IS_SET(pexit->to_room->room_flags, ROOM_NO_MOB)
                    || (IS_SET(ch->act, ACT_STAY_AREA)
                        && pexit->to_room->area != ch->in_room->area))))
            continue;

        move_char(ch, door);
        if ((now_in = ch->in_room) == was_in)
            continue;

        ch->in_room = was_in;
        act("$n has fled!", ch, NULL, NULL, TO_ROOM);
        ch->in_room = now_in;

        if (!IS_NPC(ch)) {
            cost = number_range(ch->exp / 15, ch->exp / 10);
            if (ch->race == RACE_OGR)
                cost = ch->exp / 50;

            if (ch->adept_level > 0)
                cost /= 50;
            cost = UMIN(cost, ch->exp);

            if (!ch->pcdata->in_arena) {
                sprintf(buf, "You flee from combat!  You lose %d exps.\n\r", cost);
                send_to_char(buf, ch);
                gain_exp(ch, (0 - cost));
            }
            else
                send_to_char("You flee from combat!\n\r", ch);
        }

        stop_fighting(ch, TRUE);
        /* 75% chance that mobs will hunt fleeing people. -- Alty */
        if (IS_NPC(victim) && !IS_SET(victim->act, ACT_SENTINEL) && number_bits(2) > 0) {
            if (IS_NPC(victim) && !IS_SET(victim->act, ACT_AGGRESSIVE)) {
                set_hunt(victim, NULL, ch, NULL, HUNT_WORLD | HUNT_INFORM | HUNT_OPENDOOR, HUNT_MERC | HUNT_CR);
            }
            else {
                set_hunt(victim, NULL, ch, NULL, HUNT_OPENDOOR, HUNT_MERC | HUNT_CR);
            }
        }

        return;
    }

    cost = get_pseudo_level(ch) * 3;
    if (ch->adept_level > 0)
        cost = 0;
    cost = UMIN(cost, ch->exp);

    if (!IS_NPC(ch) && !ch->pcdata->in_arena) {
        sprintf(buf, "You failed!  You lose %d exps.\n\r", cost);
        send_to_char(buf, ch);
        gain_exp(ch, (0 - cost));
    }
    else
        send_to_char("You failed!\n\r", ch);

    return;
}

void
do_rescue(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    char                buf[MAX_STRING_LENGTH];
    CHAR_DATA          *victim;
    CHAR_DATA          *fch;
    int                 best;
    int                 cnt;

    best = -1;

    if (!IS_NPC(ch)) {
        for (cnt = 0; cnt < MAX_CLASS; cnt++)
            if (ch->lvl[cnt] >= skill_table[gsn_rescue].skill_level[cnt]
                && ch->lvl[cnt] >= best)
                best = cnt;
    }
    else
        best = ch->level;

    if (best == -1) {
        send_to_char("You don't know how to rescue!\n\r", ch);
        return;
    }

    one_argument(argument, arg);

    if (arg[0] == '\0') {
        send_to_char("Rescue whom?\n\r", ch);
        return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (victim == ch) {
        send_to_char("What about fleeing instead?\n\r", ch);
        return;
    }

    if (!IS_NPC(ch) && IS_NPC(victim)) {
        act("$N doesn't need your help!", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (!IS_NPC(ch) && !IS_NPC(victim)
        && ch->level < 60 && get_pseudo_level(victim) > 99) {
        send_to_char("I think they can manage on their own, but thanks for the thought.\n\r", ch);
        return;
    }

    if (!IS_NPC(victim) && IS_SET(victim->config, PLR_NORESCUE) && (!IS_NPC(ch) || (IS_AFFECTED(ch, AFF_CHARM) && ch->master != victim))) {
        send_to_char("You failed.\n\r", ch);
        return;
    }

    if (ch->fighting == victim) {
        send_to_char("Too late.\n\r", ch);
        return;
    }

    if ((fch = victim->fighting) == NULL) {
        send_to_char("That person is not fighting right now.\n\r", ch);
        return;
    }

    if (IS_NPC(fch) && IS_SET(fch->act, ACT_NORESCUE)) {
        act("$n tries to rescue you, but an unknown force prevents it.", ch, NULL, victim, TO_VICT);
        send_to_char("An unknown force prevents your rescue attempt.\n\r", ch);
        return;
    }

    if (!IS_NPC(ch) && !IS_NPC(victim) && !IS_NPC(fch)
        && !can_group(ch, fch) && !ch->pcdata->in_arena
        ) {
        sprintf(buf, "You can't rescue %s from %s, because you can't group with %s.\n\r", victim->short_descr, fch->short_descr, fch->short_descr);

        send_to_char(buf, ch);
        return;
    }

    if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM)
        && !IS_NPC(victim) && !IS_NPC(fch)
        && ch->master && !IS_NPC(ch->master) && !can_group(ch->master, fch) && !ch->master->pcdata->in_arena
        ) {
        sprintf(buf, "%s I can't rescue %s from %s because you can't group with %s!", ch->master->short_descr, victim->short_descr, fch->short_descr, fch->short_descr);
        do_tell(ch, buf);
        return;
    }

    if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM)
        && !IS_NPC(victim) && IS_NPC(fch)
        && ch->master && !IS_NPC(ch->master) && fch->master && !can_group(ch->master, fch->master) && !ch->master->pcdata->in_arena)
        return;

    if (!IS_NPC(ch) && !IS_NPC(victim) && ch == victim->fighting) {
        send_to_char("You can't rescue your enemy.\n\r", ch);
        return;
    }

    if (IS_NGR_CHARMIE(ch, fch)) {
        send_to_char("They're fighting a charmed mobile that belongs to someone you can't group with!\n\r", ch);
        return;
    }

    WAIT_STATE(ch, skill_table[gsn_rescue].beats);
    if (!IS_NPC(ch) && number_percent() > ch->pcdata->learned[gsn_rescue]) {
        send_to_char("You fail the rescue.\n\r", ch);
        act("$n tries to rescue you, but fails!", ch, NULL, victim, TO_VICT);
        act("$N tries to rescue $n, but fails!", victim, NULL, ch, TO_ROOM);
        return;
    }

    act("You rescue $N!", ch, NULL, victim, TO_CHAR);
    act("$n rescues you!", ch, NULL, victim, TO_VICT);
    act("$n rescues $N!", ch, NULL, victim, TO_NOTVICT);

    stop_fighting(fch, FALSE);
    stop_fighting(victim, FALSE);

    set_fighting(ch, fch, TRUE);
    set_fighting(fch, ch, FALSE);
    return;
}

void
do_disarm(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA          *victim;
    int                 best;
    int                 cnt;

    best = -1;

    if (!IS_NPC(ch)) {
        for (cnt = 0; cnt < MAX_CLASS; cnt++)
            if (ch->lvl[cnt] >= skill_table[gsn_disarm].skill_level[cnt]
                && ch->lvl[cnt] >= best)
                best = cnt;
    }
    else
        best = ch->level;

    if (best == -1) {
        send_to_char("You don't know how to disarm!\n\r", ch);
        return;
    }

    if (get_eq_char(ch, WEAR_WIELD) == NULL) {
        send_to_char("You must wield a weapon to disarm.\n\r", ch);
        return;
    }

    if ((victim = ch->fighting) == NULL) {
        send_to_char("You aren't fighting anyone.\n\r", ch);
        return;
    }

    WAIT_STATE(ch, skill_table[gsn_disarm].beats);

    /* disarm() determines the chance of success */
    disarm(ch, victim);
    return;
}

void
do_sla(CHAR_DATA *ch, char *argument)
{
    send_to_char("If you want to SLAY, spell it out.\n\r", ch);
    return;
}

void
do_slay(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA          *victim;
    char                arg[MAX_INPUT_LENGTH];

    one_argument(argument, arg);
    if (arg[0] == '\0') {
        send_to_char("Slay whom?\n\r", ch);
        return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (ch == victim) {
        send_to_char("Suicide is a mortal sin.\n\r", ch);
        return;
    }

    if (IS_HERO(victim)) {
        send_to_char("Not on other Immortal / Adept players!\n\r", ch);
        return;
    }

    if (!IS_NPC(victim) && victim->level >= ch->level) {
        send_to_char("You failed.\n\r", ch);
        return;
    }

    act("You suck the life energy out of $M!", ch, NULL, victim, TO_CHAR);
    act("$n sucks out your life energy!", ch, NULL, victim, TO_VICT);
    act("$n sucks out $N's life energy!", ch, NULL, victim, TO_NOTVICT);
    raw_kill(victim, "");
    return;
}

void
do_circle(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim;
    OBJ_DATA           *obj;
    int                 cnt;
    int                 best;
    int                 level;
    int                 mult;
    int                 chance;
    int                 dam;
    bool                crack = FALSE;
    extern int          pixbonus;
    int                 beats = 0;

    /******************************************************************
     * Modified:  'damage' may now be called with sn = -1, in order to *
     * stop damage message being displayed.  Handle ALL the related    *
     * calculations here, including how many hits.  Check for critical *
     * hits too, as well as things like invis and sneak.               * 
      ******************************************************************/

    best = -1;
    level = 0;

    if (!IS_NPC(ch)) {
        for (cnt = 0; cnt < MAX_CLASS; cnt++)
            if (ch->lvl[cnt] >= skill_table[gsn_circle].skill_level[cnt]
                && ch->lvl[cnt] > level) {
                best = cnt;
                level = ch->lvl[cnt];
            }
        if (ch->lvl2[1] > 0)
            level += ch->lvl2[1] / 2;
    }
    else {
        best = ch->class;
        level = ch->level;
    }

    if (best == -1) {
        send_to_char("You better leave the assassin trade to thieves.\n\r", ch);
        return;
    }

    one_argument(argument, arg);

    if (arg[0] == '\0') {
        send_to_char("Backstab whom?\n\r", ch);
        return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }
    if ((victim == NULL)
        || (victim->is_free != FALSE))
        return;

    if ((IS_NPC(victim) && IS_SET(victim->act, ACT_NO_BODY)) || (!IS_NPC(victim) && IS_SET(victim->pcdata->pflags, PFLAG_NO_BODY))
        ) {
        act("$N has no body to backstab!", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (victim == ch) {
        send_to_char("How can you sneak up on yourself?\n\r", ch);
        return;
    }

    if (is_safe(ch, victim, TRUE))
        return;

    if ((obj = get_eq_char(ch, WEAR_WIELD)) == NULL || ((obj->value[3] != 2) && (obj->value[3] != 11))) {
        send_to_char("You need to wield a piercing or a stabbing weapon.\n\r", ch);
        return;
    }

    if (victim->fighting == NULL) {
        set_fighting(ch, victim, TRUE);
    }

    /* Circle Counters by -Ogma- */
    if (!IS_NPC(ch) && !IS_NPC(victim)) {
        ch->pcdata->circles_attempted++;
    }

    chance = (IS_NPC(ch) ? 50 : ch->pcdata->learned[gsn_circle] / 4.5);

    /* Handle Modifiers -- chance will affect thac, etc */

    if (!IS_AWAKE(victim))
        chance += 75;

    if (IS_AFFECTED(victim, AFF_SNEAK)
        || item_has_apply(victim, ITEM_APPLY_SNEAK))
        chance -= 10;

    if (IS_AFFECTED(ch, AFF_SNEAK)
        || item_has_apply(ch, ITEM_APPLY_SNEAK))
        chance += 10;

    /* removed due to abuse.
       if ( IS_AFFECTED( ch, AFF_INVISIBLE )
       || item_has_apply( ch, ITEM_APPLY_INV) )
       chance += ( IS_AFFECTED( victim, AFF_DETECT_INVIS ) ? -20 : 20 ); 
     */

    if (IS_NPC(victim))
        chance += (IS_AFFECTED(victim, AFF_DETECT_INVIS) ? -20 : 20);

    if (!IS_NPC(victim))
        chance += 20;

    if (IS_AFFECTED(victim, AFF_INVISIBLE)
        || item_has_apply(victim, ITEM_APPLY_INV))
        chance -= 10;

    if (get_pseudo_level(ch) >= get_pseudo_level(victim))
        chance += 10;
    else
        chance -= 10;

    /* Work out multiplier */
    mult = 1 + (level >= 20) + (level >= 50) + (level >= 90) + (level >= 120);

    /* Work out damage */
    if (obj->item_type == ITEM_WEAPON)
        dam = number_range(obj->value[1], obj->value[2]);
    else
        dam = number_range(ch->level / 3, ch->level / 2);

    dam += number_range(level / 2, level * 2) + GET_DAMROLL(ch) / 2;
    dam *= mult;
    if (victim != ch->fighting && victim->fighting != ch)
        check_killer(ch, victim);

    if (!IS_NPC(victim) && victim->race == RACE_PIX && number_percent() < pixbonus) {
        act("$n tries to backstab $N, but MISSES!",  ch, NULL, victim, TO_NOTVICT);
        act("You try to backstab $N, but MISS!",     ch, NULL, victim, TO_CHAR);
        act("$N tries to backstab you, but MISSES!", victim, NULL, ch, TO_CHAR);
        damage(ch, victim, 0, TYPE_IGNORE);
    }
    else if (chance < number_percent()) {
        act("$n tries to backstab $N, but misses!", ch, NULL, victim, TO_NOTVICT);
        act("You try to backstab $N, but miss!", ch, NULL, victim, TO_CHAR);
        act("$N tries to backstab you, but misses!", victim, NULL, ch, TO_CHAR);
        damage(ch, victim, 0, TYPE_IGNORE);
    }
    else {
        char buf[MSL];

        if (!IS_NPC(ch) && ch->pcdata->order[0] == 2 && (number_range(1, 150) == number_range(1, 150))) {
            dam *= 2;
            crack = TRUE;
        }

        /* Circle Counters by -Ogma- */
        if (!IS_NPC(ch) && !IS_NPC(victim)) {
            ch->pcdata->circles_landed++;
        }

        sprintf(buf, "$n places $p into the back of $N!!%s@@N", showdamage(NULL, ch, victim, dam, crack));
        act(buf, ch, obj, victim, TO_NOTVICT);

        sprintf(buf, "You place $p into the back of $N!!%s@@N", showdamage(ch, ch, victim, dam, crack));
        act(buf, ch, obj, victim, TO_CHAR);

        sprintf(buf, "$N places $p into your back.  OUCH!%s@@N", showdamage(victim, ch, victim, dam, crack));
        act(buf, victim, obj, ch, TO_CHAR);

        if (crack) {
            send_to_room("You hear a large CRACK!\n\r", ch->in_room);

            if (ch->lvl2[1] > 50) {
                send_to_char("@@NThe CRACK has left you @@aSTUNNED@@N!!!\n\r", victim);
                if (victim->stunTimer == 0)
                    victim->stunTimer = number_range(2, 4);
            }

            damage(ch, victim, dam, TYPE_CRACK);
        }
        else
            damage(ch, victim, dam, TYPE_IGNORE);
    }

    beats = skill_table[gsn_circle].beats;

    if (!IS_NPC(ch) && is_affected(ch, gsn_innerflame_novice))
        beats -= 2;
    else if (!IS_NPC(ch) && is_affected(ch, gsn_innerflame_intermediate))
        beats -= 4;
    else if (!IS_NPC(ch) && is_affected(ch, gsn_innerflame_advanced))
        beats -= 6;
    else if (!IS_NPC(ch) && is_affected(ch, gsn_innerflame_expert))
        beats -= 8;
    else if (!IS_NPC(ch) && is_affected(ch, gsn_innerflame_master))
        beats -= 12;

    WAIT_STATE(ch, beats);
    return;
}

void
do_trip(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim;
    int                 best;

    /*    int cnt;  */

    best = -1;

    if (!IS_NPC(ch)) {
        /*      for ( cnt = 0; cnt < MAX_CLASS; cnt++ )
           if ( ch->lvl[cnt] >= skill_table[gsn_trip].skill_level[cnt] 
           && ch->lvl[cnt] >= best )
           best = cnt;  */
        best = UMAX(50, get_pseudo_level(ch));
    }
    else
        best = ch->level;

    if (best == -1) {
        send_to_char("You don't know of such a skill.\n\r", ch);
        return;
    }

    one_argument(argument, arg);

    if (arg[0] == '\0') {
        send_to_char("Trip whom?\n\r", ch);
        return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (victim == ch) {
        send_to_char("That wouldn't even be funny.\n\r", ch);
        return;
    }

    if (is_safe(ch, victim, TRUE))
        return;

    if (victim->fighting == NULL) {
        send_to_char("You can't trip someone who isn't fighting.\n\r", ch);
        return;
    }

    check_killer(ch, victim);

    if (number_percent() < (IS_NPC(ch) ? 50 : 2 * ch->pcdata->learned[gsn_trip])) {
        check_killer(ch, victim);
        trip(ch, victim);
        WAIT_STATE(ch, skill_table[gsn_trip].beats);

    }

    return;
}

void
do_dirt(CHAR_DATA *ch, char *argument)
{

    AFFECT_DATA         af;
    char                arg[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim;
    int                 best;

    /*   int cnt;   */

    best = -1;

    if (!IS_NPC(ch)) {
        /*      for ( cnt = 0; cnt < MAX_CLASS; cnt++ )
           if ( ch->lvl[cnt] >= skill_table[gsn_dirt].skill_level[cnt] 
           && ch->lvl[cnt] >= best )
           best = cnt;  */
        best = UMAX(50, get_pseudo_level(ch));
    }
    else
        best = ch->level;

    if (best == -1) {
        send_to_char("You don't know of such a skill.\n\r", ch);
        return;
    }

    one_argument(argument, arg);

    if (ch->fighting == NULL) {
        send_to_char("You must be fighting!\n\r", ch);
        return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL && ch->fighting == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (victim == NULL)
        victim = ch->fighting;

    if (victim == ch) {
        send_to_char("That wouldn't be too smart, would it??.\n\r", ch);
        return;
    }

    if (is_safe(ch, victim, TRUE))
        return;

    if (victim->fighting == NULL) {
        send_to_char("Try doing this to them, when fighting...\n\r", ch);
        return;
    }

    if (IS_AFFECTED(victim, AFF_BLIND))
        return;

    WAIT_STATE(ch, skill_table[gsn_dirt].beats);

    check_killer(ch, victim);

    if (number_percent() > (IS_NPC(ch) ? 50 : ch->pcdata->learned[gsn_dirt])) {
        act("You kick dirt at $M but miss!", ch, NULL, victim, TO_CHAR);
        act("$n kicks dirt at you but misses!", ch, NULL, victim, TO_VICT);
        act("$n kicks dirt at $N but misses!", ch, NULL, victim, TO_NOTVICT);
        return;
    }
    else {
        act("You kick dirt at $S eyes!", ch, NULL, victim, TO_CHAR);
        act("$n kicks dirt in your eyes!", ch, NULL, victim, TO_VICT);
        act("$n kicks dirt at $N's eyes!", ch, NULL, victim, TO_NOTVICT);

        af.type = gsn_blindness;
        af.location = APPLY_HITROLL;
        af.modifier = -2;
        af.duration = 1;
        af.bitvector = AFF_BLIND;
        af.save = TRUE;
        affect_to_char(victim, &af);
    }
    return;
}

void
do_bash(CHAR_DATA *ch, char *argument)
{

    char                arg[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim;
    int                 best;

    /*    int cnt;  */

    best = -1;

    if (!IS_NPC(ch)) {
        /*      for ( cnt = 0; cnt < MAX_CLASS; cnt++ )
           if ( ch->lvl[cnt] >= skill_table[gsn_dirt].skill_level[cnt] 
           && ch->lvl[cnt] >= best )
           best = cnt;  */
        best = UMAX(50, get_pseudo_level(ch));
    }
    else
        best = ch->level;

    if (best == -1) {
        send_to_char("You don't know of such a skill.\n\r", ch);
        return;
    }

    one_argument(argument, arg);

    if (arg[0] == '\0') {
        send_to_char("Bash whom?\n\r", ch);
        return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (victim == ch) {
        send_to_char("Forget it!\n\r", ch);
        return;
    }

    if (is_safe(ch, victim, TRUE))
        return;

    if (victim->fighting == NULL) {
        send_to_char("It might help if you were fighting.....\n\r", ch);
        return;
    }

    WAIT_STATE(ch, skill_table[gsn_bash].beats);

    check_killer(ch, victim);

    if ((IS_NPC(ch) && (number_percent() > 75 + (ch->level) / 2))
        || (!IS_NPC(ch) && (2 * number_percent() > ch->pcdata->learned[gsn_bash]))
        ) {
        act("Your bash misses $M, and you fall!", ch, NULL, victim, TO_CHAR);
        act("$n tries to bash you, misses, and falls!", ch, NULL, victim, TO_VICT);
        act("$n tries to bash $N but misses, and falls!", ch, NULL, victim, TO_NOTVICT);
        return;
    }
    else {
        act("Your bash sends $M flying!", ch, NULL, victim, TO_CHAR);
        act("$n bashes you, sending you flying!", ch, NULL, victim, TO_VICT);
        act("$n's bash sends $N's flying!", ch, NULL, victim, TO_NOTVICT);

        /* If victim very weak, set pos to stunned, stop fighting */
        if (victim->hit < (victim->max_hit / 50) + 1) {
            act("$N stays on the floor.", ch, NULL, victim, TO_CHAR);
            act("You are unable to get up.", ch, NULL, victim, TO_VICT);
            act("$N stays on the floor.", ch, NULL, victim, TO_NOTVICT);
            stop_fighting(victim, TRUE);    /* MAG: might del this? -S- */
            victim->position = POS_RESTING;
            update_pos(victim);
        }
        else {
            /* Do some damage instead... */
            damage(ch, victim, (best + 12) * 2, TYPE_IGNORE);
        }

    }

    return;

}

void
do_berserk(CHAR_DATA *ch, char *argument)
{
    AFFECT_DATA         af;
    int                 best;
    int                 level;

    bool                prime;

    prime = FALSE;
    best = -1;

    if (IS_NPC(ch))
        return;

    if (ch->pcdata->order[0] == 3)
        prime = TRUE;

    level = ch->lvl[3];

    if (ch->fighting == NULL) {
        send_to_char("You can only go berserk when fighting!\n\r", ch);
        return;
    }

    if (is_affected(ch, gsn_berserk)) {
        send_to_char("You are already berserk!!\n\r", ch);
        return;
    }

    if (ch->pcdata->learned[gsn_berserk] == 0) {
        send_to_char("You don't know how to use this skill!\n\r", ch);
        return;
    }

    WAIT_STATE(ch, skill_table[gsn_berserk].beats);

    if (number_percent() < (ch->pcdata->learned[gsn_berserk] / 2)) {
        /* Failure */
        act("$n calls on the Dark Powers, but they don't answer!", ch, NULL, NULL, TO_ROOM);
        send_to_char("You call on the Dark Powers.  They don't answer!\n\r", ch);
        return;
    }

    af.type = gsn_berserk;
    af.duration = (prime) ? 6 : 4;
    af.location = APPLY_AC;
    af.modifier = (prime) ? level : (level * 2);
    af.bitvector = AFF_BERSERK;
    af.save = TRUE;
    affect_to_char(ch, &af);

    af.location = APPLY_HITROLL;
    af.modifier = (prime)
        ? (12 + (level >= 20) + (level >= 40) + (level >= 60) + (level >= 80))
        : (8 + (level >= 25) + (level >= 50));
    affect_to_char(ch, &af);

    af.location = APPLY_DAMROLL;
    af.modifier = (prime)
        ? (12 + (level >= 20) + (level >= 40) + (level >= 60) + (level >= 80))
        : (8 + (level >= 25) + (level >= 50));
    affect_to_char(ch, &af);

    af.location = APPLY_SAVING_SPELL;
    af.modifier = (prime)
        ? (10 - (level >= 20) - (level >= 40) - (level >= 60) - (level >= 80))
        : (20 - (level >= 25) - (level >= 50));
    affect_to_char(ch, &af);

    act("$n calls on the Dark Powers, who answer!!!", ch, NULL, NULL, TO_ROOM);
    send_to_char("You call on the Dark Powers.  They answer!!!\n\r", ch);
    return;
}

void
do_punch(CHAR_DATA *ch, char *argument)
{

    CHAR_DATA          *victim;
    int                 dam;
    bool                prime;
    int                 chance;

    prime = FALSE;

    if (!IS_NPC(ch) && ch->pcdata->learned[gsn_punch] == 0) {
        send_to_char("You are not trained in this skill!\n\r", ch);
        return;
    }

    if (((victim = get_char_room(ch, argument)) == NULL)
        && ch->fighting == NULL) {
        send_to_char("No such victim!\n\r", ch);
        return;
    }

    if (victim == NULL)
        victim = ch->fighting;

    if (is_safe(ch, victim, TRUE))
        return;

    if (!IS_NPC(ch) && ch->pcdata->order[0] == 3)
        prime = TRUE;

    if (IS_NPC(ch))
        dam = number_range(ch->level, ch->level);
    else
        dam = number_range(ch->lvl[3], ch->lvl[3] * (prime ? 2 : 1));

    chance = (IS_NPC(ch) ? 50 : ch->pcdata->learned[gsn_punch] / 2);

    chance += (ch->lvl[3] - victim->level);

    check_killer(ch, victim);

    WAIT_STATE(ch, skill_table[gsn_punch].beats);

    check_killer(ch, victim);
    if (number_percent() < chance) {
        char buf[MSL];

        act("$n punches $N really hard!", ch, NULL, victim, TO_NOTVICT);

        sprintf(buf, "You punch $N really hard!%s@@N", showdamage(ch, ch, victim, dam, FALSE));
        act(buf, ch, NULL, victim, TO_CHAR);

        sprintf(buf, "$N punches you really hard!%s@@N", showdamage(victim, ch, victim, dam, FALSE));
        act(buf, victim, NULL, ch, TO_CHAR);

        damage(ch, victim, dam, TYPE_IGNORE);
    }
    else {
        /* MISS */
        act("$n tries to punch $N, but misses!", ch, NULL, victim, TO_NOTVICT);
        act("$N tries to punch you, but misses!", victim, NULL, ch, TO_CHAR);
        act("You try to punch $N, but miss!", ch, NULL, victim, TO_CHAR);
        damage(ch, victim, 0, TYPE_IGNORE);
    }
    return;
}

void
do_headbutt(CHAR_DATA *ch, char *argument)
{

    CHAR_DATA          *victim;
    int                 dam;
    bool                prime;
    int                 chance;

    prime = FALSE;

    if (!IS_NPC(ch) && ch->pcdata->learned[gsn_headbutt] == 0) {
        send_to_char("You are not trained in this skill!\n\r", ch);
        return;
    }

    if (((victim = get_char_room(ch, argument)) == NULL)
        && ch->fighting == NULL) {
        send_to_char("No such victim!\n\r", ch);
        return;
    }

    if (victim == NULL)
        victim = ch->fighting;

    if (victim == ch) {
        send_to_char("You can't reach!\n\r", ch);
        return;
    }

    if (is_safe(ch, victim, TRUE))
        return;

    if (!IS_NPC(ch) && ch->pcdata->order[0] == 3)
        prime = TRUE;

    if (IS_NPC(ch))
        dam = number_range(ch->level / 3, ch->level / 2);
    else
        dam = number_range(ch->lvl[3] / 2, ch->lvl[3] * (prime ? 2 : 1));

    chance = (IS_NPC(ch) ? 50 : ch->pcdata->learned[gsn_headbutt] / 2);

    chance += (ch->lvl[3] - victim->level);

    WAIT_STATE(ch, skill_table[gsn_headbutt].beats);

    check_killer(ch, victim);

    if (number_percent() < chance) {
        char buf[MSL];

        act("$n headbutts $N in the face!@@N", ch, NULL, victim, TO_NOTVICT);

        sprintf(buf, "You headbutt $N in the face!%s@@N", showdamage(ch, ch, victim, dam, FALSE));
        act(buf, ch, NULL, victim, TO_CHAR);

        sprintf(buf, "$N headbutts you in the face!%s@@N", showdamage(victim, ch, victim, dam, FALSE));
        act(buf, victim, NULL, ch, TO_CHAR);

        damage(ch, victim, (dam * 4) / 5, TYPE_IGNORE);
    }
    else {
        /* MISS */
        check_killer(ch, victim);
        act("$n tries to headbutt $N, but misses!", ch, NULL, victim, TO_NOTVICT);
        act("$N tries to headbutt you, but misses!", victim, NULL, ch, TO_CHAR);
        act("You try to headbutt $N, but miss!", ch, NULL, victim, TO_CHAR);
        damage(ch, victim, 0, TYPE_IGNORE);
    }
    return;
}

void
do_charge(CHAR_DATA *ch, char *argument)
{

    CHAR_DATA          *victim;
    int                 dam;
    int                 moves = 0;
    bool                prime;
    int                 chance;

    prime = FALSE;

    if (!IS_NPC(ch) && ch->pcdata->learned[gsn_charge] == 0) {
        send_to_char("You are not trained in this skill!\n\r", ch);
        return;
    }

    if ((IS_NPC(ch))
        && (get_pseudo_level(ch) < 80))
        return;

    if (((victim = get_char_room(ch, argument)) == NULL)
        && ch->fighting == NULL) {
        send_to_char("No such victim!\n\r", ch);
        return;
    }

    if (victim == NULL)
        victim = ch->fighting;

    if (victim == ch) {
        send_to_char("You can't reach!\n\r", ch);
        return;
    }

    if (!IS_NPC(ch) && ch->move < 100 && ch->position == POS_FIGHTING) {
        send_to_char("You're too tired to charge!\n\r", ch);
        return;
    }

    if (is_safe(ch, victim, TRUE))
        return;

    if (!IS_NPC(ch) && ch->pcdata->order[0] == 3) {
        prime = TRUE;
        dam = number_range(get_pseudo_level(ch) * 2, get_pseudo_level(ch) * 4);
    }
    else
        dam = number_range(get_pseudo_level(ch), get_pseudo_level(ch) * 2);

    if (!IS_NPC(ch))
        chance = ch->pcdata->learned[gsn_charge] / 2;
    else
        chance = 50;

    if (((ch->move / 10) < 75) && ch->move >= 75)
        moves = 75;
    else
        moves = ch->move / 10;

    chance += ((get_pseudo_level(ch) - (get_pseudo_level(victim) - 30)) / 2);

    WAIT_STATE(ch, skill_table[gsn_charge].beats);

    check_killer(ch, victim);

    if (number_percent() < chance) {
        char buf[MSL];

        act("@@a$n @@acharges $N@@a, and knocks $M over!@@N", ch, NULL, victim, TO_NOTVICT);

        sprintf(buf, "@@aYou charge right into $N@@a, and knock $M over!%s@@N", showdamage(ch, ch, victim, dam, FALSE));
        act(buf, ch, NULL, victim, TO_CHAR);

        sprintf(buf, "@@a$N @@acharges right into you!%s@@N", showdamage(victim, ch, victim, dam, FALSE));
        act(buf, victim, NULL, ch, TO_CHAR);

        damage(ch, victim, dam, TYPE_IGNORE);
        ch->move = UMAX(0, ch->move - 300);
    }
    else {
        /* MISS */
        act("$n@@b charges $N@@b, but runs right past!@@N", ch, NULL, victim, TO_NOTVICT);
        act("$N @@bcharges you, but runs right past!@@N", victim, NULL, ch, TO_CHAR);
        act("@@bYou try to charge $N@@b, but run past him. DOH!!@@N", ch, NULL, victim, TO_CHAR);
        damage(ch, victim, 0, TYPE_IGNORE);
        ch->move = UMAX(0, ch->move - 50);
    }
    return;
}

void
do_knee(CHAR_DATA *ch, char *argument)
{

    CHAR_DATA          *victim;
    int                 dam;
    bool                prime;
    int                 chance;

    prime = FALSE;

    if (!IS_NPC(ch) && ch->pcdata->learned[gsn_knee] == 0) {
        send_to_char("You are not trained in this skill!\n\r", ch);
        return;
    }

    if (((victim = get_char_room(ch, argument)) == NULL)
        && ch->fighting == NULL) {
        send_to_char("No such victim!\n\r", ch);
        return;
    }

    if (victim == NULL)
        victim = ch->fighting;

    if ((IS_NPC(victim) && IS_SET(victim->act, ACT_NO_BODY)) || (!IS_NPC(victim) && IS_SET(victim->pcdata->pflags, PFLAG_NO_BODY))
        ) {
        act("$N doesn't have a definable body to knee!", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (is_safe(ch, victim, TRUE))
        return;

    if (!IS_NPC(ch) && ch->pcdata->order[0] == 3)
        prime = TRUE;

    if (IS_NPC(ch))
        dam = number_range(ch->level / 3, ch->level / 2);
    else
        dam = number_range(ch->lvl[3] / 2, ch->lvl[3] * (prime ? 2 : 1));

    chance = (IS_NPC(ch) ? 50 : ch->pcdata->learned[gsn_knee] / 2);

    chance += (ch->lvl[3] - victim->level);

    WAIT_STATE(ch, skill_table[gsn_knee].beats);

    check_killer(ch, victim);

    if (number_percent() < chance) {
        char buf[MSL];

        act("$n grabs $N and knees $M in the groin!@@N", ch, NULL, victim, TO_NOTVICT);

        sprintf(buf, "You grab $M and knee $M in the groin!%s@@N", showdamage(ch, ch, victim, dam, FALSE));
        act(buf, ch, NULL, victim, TO_CHAR);

        sprintf(buf, "$N grabs you, and knees you in the groin!%s@@N", showdamage(victim, ch, victim, dam, FALSE));
        act(buf, victim, NULL, ch, TO_CHAR);

        damage(ch, victim, (dam * 3) / 5, TYPE_IGNORE);
    }

    else {
        /* MISS */
        act("$n tries to grab $N, but $E twists free!", ch, NULL, victim, TO_NOTVICT);
        act("$N tries to grab you, but you twist free!", victim, NULL, ch, TO_CHAR);
        act("You try to grab $N, but $E twists free!", ch, NULL, victim, TO_CHAR);
        damage(ch, victim, 0, TYPE_IGNORE);
    }
    return;
}

void
do_kick(CHAR_DATA *ch, char *argument)
{

    CHAR_DATA          *victim;
    int                 dam;
    bool                prime;
    int                 chance;

    prime = FALSE;

    if (!IS_NPC(ch) && ch->pcdata->learned[gsn_kick] == 0) {
        send_to_char("You are not trained in this skill!\n\r", ch);
        return;
    }

    if (ch->in_room->vnum == ROOM_VNUM_JAIL) {
        send_to_char("Isn't jail punishment enough for you?\n\r", ch);
        return;
    }

    if (((victim = get_char_room(ch, argument)) == NULL)
        && ch->fighting == NULL) {
        send_to_char("No such victim!\n\r", ch);
        return;
    }

    if (victim == NULL)
        victim = ch->fighting;

    if (is_safe(ch, victim, TRUE))
        return;

    if (ch == victim) {
        send_to_char("You kick yourself. Ouch.\n\r", ch);
        return;
    }

    if (!IS_NPC(ch) && ch->pcdata->order[0] == 3)
        prime = TRUE;

    if (IS_NPC(ch))
        dam = number_range(ch->level / 3, ch->level / 2);
    else
        dam = number_range(ch->lvl[3] / 2, ch->lvl[3] * (prime ? 2 : 1));

    chance = (IS_NPC(ch) ? 50 : ch->pcdata->learned[gsn_kick] / 2);

    chance += (ch->lvl[3] - (victim->level + 5));

    WAIT_STATE(ch, skill_table[gsn_kick].beats);

    check_killer(ch, victim);

    if (number_percent() < chance) {
        char buf[MSL];

        act("$n kicks $N really hard!@@N", ch, NULL, victim, TO_NOTVICT);

        sprintf(buf, "You kick $N really hard!%s@@N", showdamage(ch, ch, victim, dam, FALSE));
        act(buf, ch, NULL, victim, TO_CHAR);

        sprintf(buf, "$N kicks you really hard!%s@@N", showdamage(victim, ch, victim, dam, FALSE));
        act(buf, victim, NULL, ch, TO_CHAR);

        damage(ch, victim, dam, TYPE_IGNORE);
    }
    else {
        /* MISS */
        act("$n tries to kick $N, but misses!", ch, NULL, victim, TO_NOTVICT);
        act("$N tries to kick you, but misses!", victim, NULL, ch, TO_CHAR);
        act("You try to kick $N, but miss!", ch, NULL, victim, TO_CHAR);
        damage(ch, victim, 0, TYPE_IGNORE);
    }
    return;
}

/* -S- Addition: Like damage() but called by objects, no message */
/* WARNING: No killer checks made, etc.  Only use if no keeper for obj */

void
obj_damage(OBJ_DATA *obj, CHAR_DATA *victim, int dam)
{
    extern CHAR_DATA   *violence_marker;
    CHAR_DATA          *ch;

    if ((victim->position == POS_DEAD)
        || (victim == violence_marker)
        || (victim->is_free == TRUE))
        return;

    ch = victim->fighting;

    /*
     * Damage modifiers.
     */
    if (IS_AFFECTED(victim, AFF_SANCTUARY)
        || item_has_apply(victim, ITEM_APPLY_SANC))
        dam /= 2;

    if ((IS_AFFECTED(victim, AFF_PROTECT) || item_has_apply(victim, ITEM_APPLY_PROT))
        && IS_SET(obj->extra_flags, ITEM_EVIL))
        dam -= dam / 4;

    if (dam < 0)
        dam = 0;

    /*
     * Hurt the victim.
     * Inform the victim of his new state.
     */
    victim->hit -= dam;

    update_pos(victim);

    switch (victim->position) {
        case POS_MORTAL:
            act("$n is mortally wounded, and will die soon, if not aided.", victim, NULL, NULL, TO_ROOM);
            send_to_char("You are mortally wounded, and will die soon, if not aided.\n\r", victim);
            break;

        case POS_INCAP:
            act("$n is incapacitated and will slowly die, if not aided.", victim, NULL, NULL, TO_ROOM);
            send_to_char("You are incapacitated and will slowly die, if not aided.\n\r", victim);
            break;

        case POS_STUNNED:
            act("$n is stunned, but will probably recover.", victim, NULL, NULL, TO_ROOM);
            send_to_char("You are stunned, but will probably recover.\n\r", victim);
            break;

        case POS_DEAD:
            act("$n is DEAD!!", victim, 0, 0, TO_ROOM);
            send_to_char("You have been KILLED!!\n\r\n\r", victim);
            break;

        default:
            if (dam > victim->max_hit / 4)
                send_to_char("That really did HURT!\n\r", victim);
            if (victim->hit < victim->max_hit / 4)
                send_to_char("You sure are BLEEDING!\n\r", victim);
            break;
    }

    /*
     * Payoff for killing things.
     */
    if ((victim->position == POS_DEAD
            || victim->position == POS_MORTAL
            || victim->position == POS_INCAP || victim->position == POS_STUNNED) && !IS_NPC(victim) && is_in_duel(victim, DUEL_STAGE_SET)) {
        duel_rawkill(NULL, victim, DUEL_RAWKILL_OBJ);
        return;
    }

    if ((victim->position == POS_DEAD || victim->position == POS_MORTAL || victim->position == POS_INCAP || victim->position == POS_STUNNED)
        && !IS_NPC(victim) && victim->pcdata->in_arena) {
        stop_fighting(victim, TRUE);

        if (ch && !IS_NPC(ch))
            arenaf(ch, "%s kills %s in %s!", ch->short_descr, victim->short_descr, ch->in_room->name);
        else if (ch)
            arenaf(victim, "%s is killed by a mob in %s!", victim->short_descr, victim->in_room->name);
        else
            arenaf(victim, "%s is killed by an object in %s!", victim->short_descr, victim->in_room->name);

        leave_arena(victim, FALSE);
        return;
    }

    if (victim->position == POS_DEAD) {

        if (!IS_NPC(victim)) {

            sprintf(log_buf, "%s killed by %s at %d", victim->name, obj->short_descr, victim->in_room->vnum);
            log_string(log_buf);

            notify(log_buf, 82);

            /* As level gain is no longer automatic, a dead char loses
             * 1/2 their gained exp.  -S- 
             * Fixed my bug here too, hehe!
             */

            if (victim->exp > 0 && victim->race != RACE_OGR) {
                gain_exp(victim, (0 - (victim->exp / 2)));
                victim->exp = UMAX(victim->exp, 0);
            }

        }

        raw_kill(victim, "");

        return;
    }

    tail_chain();
    return;
}

void
death_message(CHAR_DATA *ch, CHAR_DATA *victim, int dt, int max_dt)
{
    /* Used to display assorted death messages, based on dt 
     * max_dt == number of entries in attack table in dam_message,
     * so we know if kill was unarmed, armed, or through spells or skills.
     * Stephen
     */

    char                buf1[MAX_STRING_LENGTH];
    char                buf2[MAX_STRING_LENGTH];
    char                buf3[MAX_STRING_LENGTH];

    if (dt == 0 || dt == TYPE_MARTIAL) {
        if (!IS_NPC(victim) || (IS_NPC(victim) && !IS_SET(victim->act, ACT_NO_BODY))) {
            switch (number_range(0, 9)) {
                case 0:
                    sprintf(buf1, "$n grabs $N's neck, and twists until there is a loud SNAP!");
                    sprintf(buf2, "You grab $N's neck, and twist until there is a loud SNAP!");
                    sprintf(buf3, "$n grabs your neck, and twists until there is a loud SNAP!");
                    break;
                case 1:
                    sprintf(buf1, "$n slams $s fist into $N and crushes $S heart!");
                    sprintf(buf2, "You slam your fist into $N and crush $S heart!");
                    sprintf(buf3, "$n slams $s fist into you, and crushes your heart!");
                    break;
                case 2:
                    sprintf(buf1, "$n rams $s hand into $N's ribcage, and rips out $S guts!");
                    sprintf(buf2, "You ram your hand into $N's ribcage, and rip out $S guts!");
                    sprintf(buf3, "$n rams $s hand into your ribcage, and rips out your guts!");
                    break;
                case 3:
                    sprintf(buf1, "$n grabs $N, and rips $S head clean off!");
                    sprintf(buf2, "You grab $N, and rip his head clean off!");
                    sprintf(buf3, "$n grabs you, and rips your head clean off!");
                    break;
                case 4:
                    sprintf(buf1, "$n reaches behind $N, and rips $S spine out of $S back!");
                    sprintf(buf2, "You reach behind $N, and rip $S spine out of $S back!");
                    sprintf(buf3, "$n reaches behind you, and rips your spine out of your back!");
                    break;
                case 5:
                    sprintf(buf1, "$n rips $N's face open via $S eye sockets!");
                    sprintf(buf2, "You rip $N's face open via $S eye sockets!");
                    sprintf(buf3, "$n rips your face open via your eye sockets!");
                    break;
                case 6:
                    sprintf(buf1, "$n rips off $N's head and vomits down $S throat!");
                    sprintf(buf2, "You rip off $N's head and vomit down $S throat!");
                    sprintf(buf3, "$n rips off your head and vomits down your throat!");
                    break;
                case 7:
                    sprintf(buf1, "$N splurts blood as $n rips open $S chest with $s teeth!");
                    sprintf(buf2, "$N splurts blood as you rip open $S chest with your teeth!");
                    sprintf(buf3, "You splurt blood as $n rips open your chest with $s teeth!");
                    break;
                case 8:
                    sprintf(buf1, "$n wrenches $N's arms out from their sockets!");
                    sprintf(buf2, "You wrench $N's arms out from their sockets!");
                    sprintf(buf3, "$n wrenches your arms out of thier sockets!");
                    break;
                case 9:
                    sprintf(buf1, "$n shatters $N's skull with a punch.  Brains leak out.");
                    sprintf(buf2, "You shatter $N's skull with a punch.  Brains leak out.");
                    sprintf(buf3, "$n shatters your skull with a punch.  Brains leak out.");
                    break;
            }
        }
        else {                    /* Unarmed, mob has no_body */
            sprintf(buf1, "$n shatters $N's skull with a punch.  Brains leak out.");
            sprintf(buf2, "You shatter $N's skull with a punch.  Brains leak out.");
            sprintf(buf3, "$n shatters your skull with a punch.  Brains leak out.");
        }
    }
    else if (dt > 0 && dt >= TYPE_HIT) {
        switch (dt - TYPE_HIT) {
            case 1:            /* slice */
            case 3:            /* slash */
                switch (number_range(0, 4)) {
                    case 0:
                        sprintf(buf1, "$n slices $N's head clean from $S neck");
                        sprintf(buf2, "You slice $N's head clean from $S neck");
                        sprintf(buf3, "$n slices your head clean from your neck");
                        break;
                    case 1:
                        sprintf(buf1, "$n slashes open $N's chest; $S entrails pour out!");
                        sprintf(buf2, "You slash open $N's chest; $S entrails pour out!");
                        sprintf(buf3, "$n slashes open your chest; your entrails pour out!");
                        break;
                    case 2:
                        sprintf(buf1, "$n slices $N's throat open.  Blood sprays out wildly!");
                        sprintf(buf2, "You slice $N's throat open.  Blood sprays out wildly!");
                        sprintf(buf3, "$n slices your throat open.  Blood sprays out wildly!");
                        break;
                    case 3:
                        sprintf(buf1, "$n slices $N's legs off, leaving two bloody stumps!");
                        sprintf(buf2, "You slice $N's legs off, leaving two bloody stumps!");
                        sprintf(buf3, "$n slices your legs off, leaving two bloody stumps!");
                        break;
                    case 4:
                        sprintf(buf1, "$n slashes $N's eyeballs out!");
                        sprintf(buf2, "You slash $N's eyeballs out!");
                        sprintf(buf3, "$n slashes your eyeballs out!");
                        break;
                }
                break;
            case 2:            /* Stab */
            case 11:            /* Pierce */
                switch (number_range(0, 4)) {
                    case 0:
                        sprintf(buf1, "$n rips a gaping hole down $N's back!");
                        sprintf(buf2, "You rip a gaping hole down $N's back!");
                        sprintf(buf3, "$n rips a gaping hole down your back!");
                        break;
                    case 1:
                        sprintf(buf1, "$n stabs into $N, and cuts $S heart out!");
                        sprintf(buf2, "You stab into $N, and cut $S heart out!");
                        sprintf(buf3, "$n stabs into you, and cuts your heart out!");
                        break;
                    case 2:
                        sprintf(buf1, "$n stabs into $N's back, and wrenches out $S spine!");
                        sprintf(buf2, "You stab into $N's back, and wrench out $S spine!");
                        sprintf(buf3, "$n stabs into your back, and wrenches out your spine!");
                        break;
                    case 3:
                        sprintf(buf1, "$n plunges $s weapon into $N's head!");
                        sprintf(buf2, "You plunge your weapon into $N's head!");
                        sprintf(buf3, "$n plunges $s weapon into your head!");
                        break;
                    case 4:
                        sprintf(buf1, "$n stabs into $N's chest, skewering $S heart!");
                        sprintf(buf1, "$n stabs into $N's chest, skewering $S heart!");
                        sprintf(buf2, "You stab into $N's chest, skewering $S heart!");
                        sprintf(buf2, "You stab into $N's chest, skewering $S heart!");
                        sprintf(buf3, "$n stabs into your chest, skewering your heart!");
                        break;
                }
                break;
            case 4:            /* Whip */
                switch (number_range(0, 3)) {
                    case 0:
                        sprintf(buf1, "$n whips out $N's eyes, spraying blood everywhere!");
                        sprintf(buf2, "You whip out $N's eyes, spraying blood everywhere!");
                        sprintf(buf3, "$n whips out your eyes, spraying blood everywhere!");
                        break;
                    case 1:
                        sprintf(buf1, "$n's whip catches $N's head, and rips it off!");
                        sprintf(buf2, "Your whip catches $N's head, and rips it off!");
                        sprintf(buf3, "$n's whip catches your head, and rips it off!");
                        break;
                    case 2:
                        sprintf(buf1, "$n's whip wraps around $N's arms, yanking them off!");
                        sprintf(buf2, "Your whip wraps around $N's arms, yanking them off!");
                        sprintf(buf3, "$n's whip wraps around your arms, yanking them off!");
                        break;
                    case 3:
                        sprintf(buf1, "$n's whip cuts open $N's main artery, spraying blood!");
                        sprintf(buf2, "Your whip cuts open $N's main artery, spraying blood!");
                        sprintf(buf3, "$n's whip cuts open your main artery, spraying blood!");
                        break;
                }
                break;
            case 5:            /* Claw */
                switch (number_range(0, 4)) {
                    case 0:
                        sprintf(buf1, "$n claws out $N's heart!");
                        sprintf(buf2, "You claw out $N's heart!");
                        sprintf(buf3, "$n claws out your heart!");
                        break;
                    case 1:
                        sprintf(buf1, "$n's claw catches $N's back, slicing it open!");
                        sprintf(buf2, "Your claw catches $N's back, slicing it open!");
                        sprintf(buf3, "$n's claw catches your back, slicing it open!");
                        break;
                    case 2:
                        sprintf(buf1, "$N screams in agony, as $n's claw removes $S eyes!");
                        sprintf(buf1, "$N screams in agony, as $n's claw removes $S eyes!");
                        sprintf(buf2, "$N screams in agony, as your claw removes $S eyes!");
                        sprintf(buf2, "$N screams in agony, as your claw removes $S eyes!");
                        sprintf(buf3, "You scream in agony, as $n's claw removes your eyes!");
                        break;
                    case 3:
                        sprintf(buf1, "$n's claw ruptures $N's ribcage, shredding $S heart!");
                        sprintf(buf2, "Your claw ruptures $N's ribcage, shredding $S heart!");
                        sprintf(buf3, "$n's claw ruptures your ribcage, shredding your heart!");
                        break;
                    case 4:
                        sprintf(buf1, "$n's claw slashes $N's neck, decapitating $M!");
                        sprintf(buf2, "Your claw slashes $N's neck, decapitating $M!");
                        sprintf(buf3, "$n's claw slashes your neck, decapitating you!");
                        break;
                }
                break;
            case 7:            /* Pound */
            case 8:            /* Crush */
                switch (number_range(0, 4)) {
                    case 0:
                        sprintf(buf1, "$n pounds $N's head; $S brains leak from $S ears!");
                        sprintf(buf2, "You pound $N's head; $S brains leak from $S ears!");
                        sprintf(buf3, "$n pounds your head; your brains leak from your ears!");
                        break;
                    case 1:
                        sprintf(buf1, "$n crushes $N's ribcage, and $S entrails slop out!");
                        sprintf(buf2, "You crush $N's ribcage, and $S entrails slop out!");
                        sprintf(buf3, "$n crushes your ribcage, and your entrails slop out!");
                        break;
                    case 2:
                        sprintf(buf1, "$n pounds $N's spine until you hear it CRACK!");
                        sprintf(buf2, "You pound $N's spine until you hear it CRACK!");
                        sprintf(buf3, "$n pounds your spine until you hear it CRACK!");
                        break;
                    case 3:
                        sprintf(buf1, "$n pounds $N's face, forcing $S nose into $S brain!");
                        sprintf(buf2, "You pound $N's face, forcing $S nose into $S brain!");
                        sprintf(buf3, "$n pounds your face, forcing your nose into your brain!");
                        break;
                    case 4:
                        sprintf(buf1, "$n crushes $N's head down into $S neck!");
                        sprintf(buf2, "You crush $N's head down into $S neck!");
                        sprintf(buf3, "$n crushes your head down into your neck!");
                        break;
                }
                break;
            case 6:            /* Blast */
                switch (number_range(0, 4)) {
                    case 0:
                        sprintf(buf1, "$n's blast totally obliterates $N's head!");
                        sprintf(buf2, "Your blast totally obliterates $N's head!");
                        sprintf(buf3, "$n's blast totally obliterates your head!");
                        break;
                    case 1:
                        sprintf(buf1, "$n's blast makes $N's head fly into the air!");
                        sprintf(buf2, "Your blast makes $N's head fly into the air!");
                        sprintf(buf3, "$n's blast makes your head fly into the air!");
                        break;
                    case 2:
                        sprintf(buf1, "$n blasts $N's stomach open - entrails plop out!");
                        sprintf(buf2, "You blast $N's stomach open - entrails plop out!");
                        sprintf(buf3, "$n blasts your stomach open - entrails plop out!");
                        break;
                    case 3:
                        sprintf(buf1, "$n's blast removes $N's legs from $S body!");
                        sprintf(buf2, "Your blast removes $N's legs from $S body!");
                        sprintf(buf3, "$n's blast removes your legs from your body!");
                        break;
                    case 4:
                        sprintf(buf1, "$n's blast splits $N's back, and $S spine falls out!");
                        sprintf(buf2, "Your blast splits $N's back, and $S spine falls out!");
                        sprintf(buf3, "$n's blast splits your back, and your spine falls out!");
                        break;
                }
                break;
            default:            /* grep, bite, suction */
                switch (number_range(0, 3)) {
                    case 0:
                        sprintf(buf1, "$n pulls $N's heart clean from $S ribcage!");
                        sprintf(buf2, "You pull $N's heart clean from $S ribcage!");
                        sprintf(buf3, "$n pulls your heart clean from your ribcage!");
                        break;
                    case 1:
                        sprintf(buf1, "$n snags $N's spine, and rips it out!");
                        sprintf(buf2, "You snag $N's spine, and rip it out!");
                        sprintf(buf3, "$n snags your spine, and rips it out!");
                        break;
                    case 2:
                        sprintf(buf1, "$N's stomach splits open, and $S entrails slip out!");
                        sprintf(buf2, "$N's stomach splits open, and $S entrails slip out!");
                        sprintf(buf3, "Your stomach splits open, and your entrails slip out!");
                        break;
                    case 3:
                        sprintf(buf1, "$n pulls $N's heart from $S chest!");
                        sprintf(buf2, "You pull $N's heart from $S chest!");
                        sprintf(buf3, "$n pulls your heart from your chest!");
                        break;
                }
                break;
        }
    }
    else {
        /*  remove this for now.. it's.. ugly..!
        sprintf(buf1, "$n kills $N with $s skill/spell!");
        sprintf(buf2, "You kill $N with your skill/spell!");
        sprintf(buf3, "$n kills you with $s skill/spell!");
        */
        return;
    }

    act(buf1, ch, NULL, victim, TO_NOTVICT);
    act(buf2, ch, NULL, victim, TO_CHAR);
    act(buf3, ch, NULL, victim, TO_VICT);

    return;
}

void
do_assist(CHAR_DATA *ch, char *argument)
{
    /* Allow players to join in fight, by typing assist, 
     * or assist <name>.  Will only ever allow players to
     * assist a group member  -- Stephen
     */

    CHAR_DATA          *assist;
    CHAR_DATA          *ppl;

    if (argument[0] != '\0') {    /* then check for assist <name> */
        if ((assist = get_char_room(ch, argument)) == NULL) {
            send_to_char("They don't seem to be here right now...\n\r", ch);
            return;
        }
        if (assist == ch) {
            send_to_char("You sure need help!\n\r", ch);
            return;
        }
        if (!is_same_group(ch, assist)) {
            act("Sorry, $N isn't in your group...", ch, NULL, assist, TO_CHAR);
            return;
        }
        if ((assist->fighting != NULL)
            && (ch->fighting == NULL)) {
            act("$n screams 'BANZAI!! $N must be assisted!!'", ch, NULL, assist, TO_ROOM);
            act("You scream 'BANZAI!! $N must be assisted!!'", ch, NULL, assist, TO_CHAR);
            set_fighting(ch, assist->fighting, TRUE);
            return;
        }
        send_to_char("Doesn't look like anyone needs your help right now...\n\r", ch);
        return;
    }

    /* Check first for any group members in room AND fighting */
    for (ppl = ch->in_room->first_person; ppl != NULL; ppl = ppl->next_in_room)
        if ((ppl != ch)
            && (is_same_group(ch, ppl))
            && (!IS_NPC(ppl)))
            break;

    if (ppl == NULL) {
        send_to_char("Doesn't look like anyone needs your help right now...\n\r", ch);
        return;
    }

    /* Assisting leader is main priority */
    if ((ch->leader != NULL)
        && (ch->leader->in_room == ch->in_room)
        && (ch->leader->fighting != NULL)
        && (ch->fighting == NULL)
        && (ch->leader != ch)) {
        /* BANZAI!! */
        act("$n screams 'BANZAI!! $N must be assisted!!'", ch, NULL, ch->leader, TO_ROOM);
        act("You scream 'BANZAI!! $N must be assisted!!'", ch, NULL, ch->leader, TO_CHAR);
        set_fighting(ch, ch->leader->fighting, TRUE);
        return;
    }

    /* No leader, look for player groupies */
    for (ppl = ch->in_room->first_person; ppl != NULL; ppl = ppl->next_in_room)
        if ((is_same_group(ch, ppl))
            && (!IS_NPC(ppl))
            && (ppl != ch)
            && (ppl->fighting != NULL)
            && (ch->fighting == NULL)) {
            act("$n screams 'BANZAI!! $N must be assisted!!'", ch, NULL, ppl, TO_ROOM);
            act("You scream 'BANZAI!! $N must be assisted!!'", ch, NULL, ppl, TO_CHAR);
            set_fighting(ch, ppl->fighting, TRUE);
            return;
        }

    /* No player groupies, look for mob groupies */
    for (ppl = ch->in_room->first_person; ppl != NULL; ppl = ppl->next_in_room)
        if ((is_same_group(ch, ppl))
            && (IS_NPC(ppl))
            && (ppl != ch)
            && (ppl->fighting != NULL)
            && (ch->fighting == NULL)) {
            act("$n screams 'BANZAI!! $N must be assisted!!'", ch, NULL, ppl, TO_ROOM);
            act("You scream 'BANZAI!! $N must be assisted!!'", ch, NULL, ppl, TO_CHAR);
            set_fighting(ch, ppl->fighting, TRUE);
            return;
        }

    send_to_char("Doesn't look like anyone needs your help right now...\n\r", ch);
    return;
}

void
do_stun(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA          *victim;
    int                 chance;
    int                 chance2;
    extern int          elfbonus;
    int                 beats = 0;

    if (IS_NPC(ch))                /* for now */
        return;

    if (((victim = ch->fighting) == NULL)
        || (victim->in_room == NULL) || victim->in_room != ch->in_room) {
        send_to_char("You must be fighting someone first!\n\r", ch);
        return;
    }
    if (!IS_NPC(ch) && (ch->pcdata->learned[gsn_stun] == 0)) {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    if (is_safe(ch, victim, TRUE))
        return;

    chance = 45 - (100 * victim->hit / victim->max_hit);
    /* The lower the victim's hp, the greater the chance */

    chance2 = IS_NPC(ch) ? ch->level * 2 : ch->pcdata->learned[gsn_stun];

    chance += chance2;

    beats = skill_table[gsn_stun].beats;

    if (!IS_NPC(ch) && is_affected(ch, gsn_innerflame_novice))
        beats -= 2;
    else if (!IS_NPC(ch) && is_affected(ch, gsn_innerflame_intermediate))
        beats -= 4;
    else if (!IS_NPC(ch) && is_affected(ch, gsn_innerflame_advanced))
        beats -= 6;
    else if (!IS_NPC(ch) && is_affected(ch, gsn_innerflame_expert))
        beats -= 8;
    else if (!IS_NPC(ch) && is_affected(ch, gsn_innerflame_master))
        beats -= 12;

    WAIT_STATE(ch, beats);

    if (!IS_NPC(victim) && victim->race == RACE_ELF && number_percent() < elfbonus) {
        act("You try to slam into $N, but MISS and fall onto your face!", ch, NULL, victim, TO_CHAR);
        act("$n tries to slam into you, but MISSES and falls onto $s face!", ch, NULL, victim, TO_VICT);
        act("$n tries to slam into $N, but MISSES and falls onto $s face!", ch, NULL, victim, TO_NOTVICT);
        damage(ch, ch, number_range(5, 20), TYPE_IGNORE);
        return;
    }

    if ((number_percent() + number_percent()) < chance) {
        char                buf[MSL];
        char               *msg;
        int                 stun;

        stun = number_range(1, get_pseudo_level(ch) / 30);
        if (ch->lvl2[4] > 40)
            stun += number_range(1, 2);

        if (stun <= 4)
            msg = "stunned";
        else
            msg = "STUNNED";

        sprintf(buf, "You slam into $N, leaving $M %s.", msg);
        act(buf, ch, NULL, victim, TO_CHAR);

        act("$n slams into you, leaving you stunned.", ch, NULL, victim, TO_VICT);
        act("$n slams into $N, leaving $M stunned.", ch, NULL, victim, TO_NOTVICT);

        victim->stunTimer = stun;
    }
    else {
        /* Ooops! */
        act("You try to slam into $N, but miss and fall onto your face!", ch, NULL, victim, TO_CHAR);
        act("$n tries to slam into you, but misses and falls onto $s face!", ch, NULL, victim, TO_VICT);
        act("$n tries to slam into $N, but misses and falls onto $s face!", ch, NULL, victim, TO_NOTVICT);
        damage(ch, ch, number_range(5, 20), TYPE_IGNORE);
        return;
    }
    return;
}

void
check_adrenaline(CHAR_DATA *ch, sh_int damage)
{

    AFFECT_DATA         af;

    if (damage > 200 && ch->pcdata->learned[gsn_adrenaline] > 70) {

        af.type = gsn_adrenaline;
        af.duration = 1;
        af.location = APPLY_DAMROLL;
        af.modifier = 1;
        af.bitvector = 0;
        af.save = TRUE;
        affect_join(ch, &af);
        send_to_char("You feel a surge of @@eadrenaline@@N!!!\n\r", ch);
    }
    return;

}

void
do_frenzy(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA          *vch;
    int                 moves = 0;
    int                 dam = 0;

    if (IS_NPC(ch))
        return;

    if (!IS_NPC(ch) && ch->pcdata->learned[gsn_frenzy] == 0) {
        send_to_char("You are not trained in this skill!\n\r", ch);
        return;
    }

    if (!IS_NPC(ch) && (ch->in_room != NULL) && (ch->in_room->vnum == 1))
        return;

    /* no more safe room frenzying, therefore gaining free ls/ms -dave */
    if ((ch->in_room != NULL)
        && IS_SET(ch->in_room->room_flags, ROOM_SAFE)) {
        send_to_char("Not a chance!  This is a safe room.\n\r", ch);
        return;
    }

    if (!IS_NPC(ch) && ch->move < 100 && ch->position == POS_FIGHTING) {
        send_to_char("You're too tired to go into a frenzy!\n\r", ch);
        return;
    }

    if (!IS_NPC(ch) && number_percent() > ch->pcdata->learned[gsn_frenzy]) {
        send_to_char("You try to go into a frenzy, but nearly cut your leg off!\n\r", ch);
        return;
    }

    if (((ch->move / 10) < 75) && ch->move >= 75)
        moves = number_range(80, 160);
    else
        moves = ch->move / 10;

    if (ch->hit > 200)
        dam = number_range(20, get_pseudo_level(ch) * 2);

    WAIT_STATE(ch, skill_table[gsn_frenzy].beats);

    if (!IS_NPC(ch) && ch->position == POS_FIGHTING)
        ch->move -= moves;

    ch->hit -= dam;

    for (vch = ch->in_room->first_person; vch != NULL; vch = vch->next_in_room) {
        extern CHAR_DATA   *violence_marker;

        if (vch == violence_marker)
            continue;

        if (vch->in_room == NULL)
            continue;

        if (vch->in_room == ch->in_room) {
            if (is_safe(ch, vch, FALSE))
                continue;

            if (vch != ch && (vch->in_room == ch->in_room)
                && (IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch))
                && (vch->master != ch)
                && (!is_same_group(ch, vch))) {
                char buf[MSL];

                act("$n goes into a FRENZY, @@Wgoring@@N $N!!!", ch, NULL, vch, TO_ROOM);

                sprintf(buf, "$N is @@Rgored@@N by your FRENZY!!!%s@@N", showdamage(ch, ch, vch, dam, FALSE));
                act(buf, ch, NULL, vch, TO_CHAR);

                sprintf(buf, "You are @@Rgored@@N by $N's FRENZY!!!%s@@N", showdamage(vch, ch, vch, dam, FALSE));
                act(buf, vch, NULL, ch, TO_CHAR);

                check_killer(ch, vch);
                damage(ch, vch, dam, TYPE_IGNORE);
                continue;
            }
        }
    }

    return;
}

void
do_grab(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA          *victim;
    int                 chance;
    int                 dam = 0;
    int                 moves = 0;
    int                 chance2;
    extern int          elfbonus;
    int                 beats = 0;

    if (IS_NPC(ch))
        return;

    if ((victim = ch->fighting) == NULL || victim->in_room == NULL || victim->in_room != ch->in_room) {
        send_to_char("You must be fighting someone first!\n\r", ch);
        return;
    }

    if (ch->pcdata->learned[gsn_grab] == 0) {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    chance = 45 - (100 * victim->hit / victim->max_hit);

    /* The lower the victim's hp, the greater the chance */
    chance2 = IS_NPC(ch) ? ch->level * 2 : ch->pcdata->learned[gsn_grab];
    chance += chance2;

    beats = skill_table[gsn_grab].beats;

    if (!IS_NPC(ch) && is_affected(ch, gsn_innerflame_novice))
        beats -= 2;
    else if (!IS_NPC(ch) && is_affected(ch, gsn_innerflame_intermediate))
        beats -= 4;
    else if (!IS_NPC(ch) && is_affected(ch, gsn_innerflame_advanced))
        beats -= 6;
    else if (!IS_NPC(ch) && is_affected(ch, gsn_innerflame_expert))
        beats -= 8;
    else if (!IS_NPC(ch) && is_affected(ch, gsn_innerflame_master))
        beats -= 12;

    WAIT_STATE(ch, beats);

    if (!IS_NPC(ch) && ch->move < 100 && ch->position == POS_FIGHTING) {
        send_to_char("You're too tired to try and grab anyone!\n\r", ch);
        return;
    }

    if (((ch->move / 10) < 75) && ch->move >= 75)
        moves = ch->move / 200;
    else
        moves = ch->move / 100;

    if (!IS_NPC(victim) && victim->race == RACE_ELF && number_percent() < elfbonus) {
        act("You try to grab $N by the head, but FAIL miserably!", ch, NULL, victim, TO_CHAR);
        act("$n tries to snap your neck, but MISSES and stumbles!", ch, NULL, victim, TO_VICT);
        act("$n tries to grab $N by the head, but MISSES and stumbles!", ch, NULL, victim, TO_NOTVICT);

        damage(ch, ch, number_range(5, 20), TYPE_IGNORE);
        ch->move -= 100;
        return;
    }

    if ((number_percent() + number_percent()) < chance) {
        char buf[MSL];

        dam = number_range(get_pseudo_level(ch), get_pseudo_level(ch) * 2);

        act("$n grabs $N by the head and twists it, snapping $S neck.", ch, NULL, victim, TO_NOTVICT);

        sprintf(buf, "You grab $N by the head and twist, and hear a loud CRACK!%s@@N", showdamage(ch, ch, victim, dam, FALSE));
        act(buf, ch, NULL, victim, TO_CHAR);

        sprintf(buf, "$n twists your head, snapping your neck.%s@@N", showdamage(victim, ch, victim, dam, FALSE));
        act(buf, ch, NULL, victim, TO_VICT);

        damage(ch, victim, dam, TYPE_IGNORE);

        victim->stunTimer = number_range(1, get_pseudo_level(ch) / 30);
        ch->move -= 200;
    }
    else {
        /* Ooops! */
        act("You try to grab $N by the head, but fail miserably!", ch, NULL, victim, TO_CHAR);
        act("$n tries to snap your neck, but misses and stumbles!", ch, NULL, victim, TO_VICT);
        act("$n tries to grab $N by the head, but misses and stumbles!", ch, NULL, victim, TO_NOTVICT);

        damage(ch, ch, number_range(5, 20), TYPE_IGNORE);
        ch->move -= 100;
        return;
    }

    return;
}

bool
is_in_pk(CHAR_DATA *ch)
{
    CHAR_DATA          *victim;

    if (!ch || !ch->in_room || !ch->in_room->first_person || IS_NPC(ch))
        return FALSE;

    if (ch->fighting && !IS_NPC(ch->fighting))
        return TRUE;

    for (victim = ch->in_room->first_person; victim != NULL; victim = victim->next_in_room)
        if (!IS_NPC(victim) && victim->fighting == ch)
            return TRUE;

    return FALSE;
}

char *showdamage(CHAR_DATA *to, CHAR_DATA *ch, CHAR_DATA *victim, int dam, bool crack)
{
    static char         buf[64];
    const char         *damout;
    const char         *damin;
    DUEL_DATA          *duel = NULL;
    SHIELD_DATA        *shield;
    int                 damcap = 3000;
    int                 sdam = dam;

    if      (IS_GOOD(ch)) { damout = "@@l"; damin = "@@W"; }
    else if (IS_EVIL(ch)) { damout = "@@R"; damin = "@@d"; }
    else                  { damout = "@@W"; damin = "@@r"; }

    if (!IS_NPC(ch) && (duel = find_duel(ch)) && IS_SET(duel->flags, DUEL_DBLDAM)) {
        damcap = 6000;
        sdam *= 2;
    }

    if (sdam > damcap && !crack)
        sdam = damcap;

    if (IS_AFFECTED(victim, AFF_SANCTUARY) || item_has_apply(victim, ITEM_APPLY_SANC))
        sdam /= 2;

    if ((IS_AFFECTED(victim, AFF_PROTECT) || item_has_apply(ch, ITEM_APPLY_PROT)) && IS_EVIL(ch))
        sdam -= sdam / 4;

    if (sdam < 0)
        sdam = 0;

    for (shield = victim->first_shield; shield != NULL; shield = shield->next) {
        /* shield absorb % for mobs */
        if (IS_NPC(victim) && shield_table[shield->index].mprotection > 0) {
            if (shield->index != SHIELD_ICE || (shield->index == SHIELD_ICE && ch->race != RACE_MIN))
                sdam = sdam - (int) (sdam * ((shield_table[shield->index].mprotection) / 100.0));
        }
        /* shield absorb % for players */
        else if (!IS_NPC(victim) && shield_table[shield->index].protection > 0) {
            if (shield->index != SHIELD_ICE || (shield->index == SHIELD_ICE && ch->race != RACE_MIN))
                sdam = sdam - (int) (sdam * ((shield_table[shield->index].protection) / 100.0));
        }
    }

    if (!IS_AFFECTED(victim, AFF_CLOAK_FLAMING) && is_affected(victim, gsn_cloak_mana)) {
        if (victim->mana > 10 && victim->lvl2[3] > 70 && victim->lvl2[0] > 70)
            sdam /= 3;
        else if (victim->mana > 10 && ((victim->lvl2[3] > 70) || (victim->lvl2[0] > 70)))
            sdam /= 2;
    }

    if (to == NULL && !IS_NPC(ch) && is_in_duel(ch, DUEL_STAGE_GO) && sdam > 0)
        sprintf(buf, " %s(%s%d%s)", damout, damin, sdam, damout);
    else if (to == NULL || IS_NPC(to) || !IS_SET(to->config, PLR_SHOWDAMAGE) || sdam == 0)
        buf[0] = '\0';
    else
        sprintf(buf, " %s(%s%d%s)", damout, damin, sdam, damout);

    return buf;
}
