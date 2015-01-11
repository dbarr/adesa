
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include "merc.h"
#include "duel.h"

IDSTRING(rcsid, "$Id: magic3.c,v 1.51 2004/11/12 03:09:32 dave Exp $");

extern bool is_charmie_of args((CHAR_DATA *ch, CHAR_DATA *charmie));
extern void show_char_to_char_0 args((CHAR_DATA *victim, CHAR_DATA *ch));

/*
 * This file should contain:
 *  o Adept Spells
 *  o Remort Spells
 *  o Room-affect Spells
 */

#define DEMON_WAGGLE "@@N@@gA strange @@eDemon@@g appears, waggles its finger at you, and shakes its head!@@N\n\r"

bool
spell_seal_room(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    ROOM_INDEX_DATA    *room;
    bool                inroom_qm = FALSE;
    bool                inroom_qt = FALSE;
    bool                inarea_qm = FALSE;
    bool                inarea_qt = FALSE;
    ROOM_AFFECT_DATA    raf;
    extern CHAR_DATA   *quest_mob;
    extern CHAR_DATA   *quest_target;
    extern int          quest_timer;
    extern bool         quest;
    CHAR_DATA          *master;

    room = ch->in_room;
    master = (IS_NPC(ch) && ch->master) ? ch->master : ch;

    if (room == NULL)
        return FALSE;

    if (IS_SET(room->affected_by, ROOM_BV_ENCAPS)) {
        send_to_char("This room is already sealed!\n\r", ch);
        return FALSE;
    }

    if (IS_SET(room->area->flags, AREA_NO_ROOM_AFF) && (IS_NPC(ch) || !IS_IMMORTAL(ch))) {
        send_to_char(DEMON_WAGGLE, ch);
        return FALSE;
    }

    if (quest && quest_mob && quest_mob->in_room && quest_mob->in_room == room)
        inroom_qm = TRUE;

    if (quest && quest_mob && quest_mob->in_room && quest_mob->in_room->area == room->area)
        inarea_qm = TRUE;

    if (quest && quest_target && quest_target->in_room && quest_target->in_room == room)
        inroom_qt = TRUE;

    if (quest && quest_target && quest_target->in_room && quest_target->in_room->area == room->area)
        inarea_qt = TRUE;

    if (inroom_qt && !inroom_qm)
        ;
    else if (inroom_qm && has_quest_item(master))
        ;
    else if (inarea_qm || (inarea_qt && quest_timer >= 7)) {
        send_to_char("You cannot seal this room at this time.\n\r", ch);
        return FALSE;
    }

    act("$n spreads his hands into the air and ejects a web of energy!", ch, NULL, NULL, TO_ROOM);
    send_to_char("You spread your hands into the air and eject a web of energy!\n\r", ch);

    raf.type = sn;
    raf.level = level;
    raf.duration = (level / 20) + 2;
    raf.bitvector = ROOM_BV_ENCAPS;
    raf.caster = ch;
    raf.name = str_dup("");
    affect_to_room(room, &raf);
    return TRUE;
}

bool
spell_deflect_weapon(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (is_affected(victim, sn))
        return FALSE;
    af.type = sn;
    af.duration = 16;
    af.modifier = -get_pseudo_level(ch) / 3;
    af.location = APPLY_AC;
    af.bitvector = 0;
    af.save = TRUE;
    affect_to_char(victim, &af);
    send_to_char("You feel a mind shield form around you.\n\r", victim);
    if (ch != victim)
        send_to_char("Ok.\n\r", ch);
    return TRUE;
}

bool
spell_black_hand(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;

    AFFECT_DATA         af;

    act("You summon a hand of death, and it chokes $N!", ch, NULL, victim, TO_CHAR);
    act("A Black Hand grows from the shadows, and begins to choke $N!", ch, NULL, victim, TO_NOTVICT);
    act("$n summons a Black Hand from the shadows, which begins to choke you!", ch, NULL, victim, TO_VICT);

    if (saves_spell(level, victim) || is_affected(victim, sn)) {
        send_to_char("The Black Hand dissolves back into the shadows!\n\r", victim);
        send_to_char("The Black Hand dissolves back into the shadows!\n\r", ch);
        act("The Black Hand dissolves back into the shadows!", ch, NULL, victim, TO_NOTVICT);
        return TRUE;
    }

    af.type = sn;
    af.duration = 3;
    af.location = APPLY_HIT;
    af.modifier = level;
    af.bitvector = 0;
    af.save = TRUE;
    af.caster = ch;
    affect_to_char(victim, &af);
    act("The Black Hand surrounds $N and begins to choke!", ch, NULL, victim, TO_ROOM);

    return TRUE;
}

bool
spell_throw_needle(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (!saves_spell(level, victim) && victim->race != RACE_LAM) {
        af.type = sn;
        af.duration = 12 + (level / 10);
        af.location = APPLY_STR;
        af.modifier = -get_pseudo_level(ch) / 40;
        af.bitvector = AFF_POISON;
        af.save = TRUE;
        affect_join(victim, &af);
        send_to_char("You feel very sick.\n\r", victim);
        act("$n looks very sick.", victim, NULL, NULL, TO_ROOM);
    }
    damage(ch, (CHAR_DATA *) vo, dice(2, 8) + level / 2, sn);
    return TRUE;
}

bool
spell_morale(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    AFFECT_DATA         af;
    CHAR_DATA          *gch;

    for (gch = ch->in_room->first_person; gch != NULL; gch = gch->next_in_room) {
        if (is_affected(gch, sn) || !is_same_group(ch, gch))
            continue;

        act("$n seems much more willing to fight.", gch, NULL, NULL, TO_ROOM);
        send_to_char("You are inspired to fight better!\n\r", gch);
        af.type = sn;
        af.duration = 4 + (level / 5);
        af.location = APPLY_DAMROLL;
        af.modifier = get_pseudo_level(ch) / 12;
        af.bitvector = 0;
        af.save = TRUE;
        affect_to_char(gch, &af);
    }
    send_to_char("You inspire the troops!\n\r", ch);

    return TRUE;
}

bool
spell_leadership(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    AFFECT_DATA         af;
    CHAR_DATA          *gch;

    for (gch = ch->in_room->first_person; gch != NULL; gch = gch->next_in_room) {
        if (is_affected(gch, sn) || !is_same_group(ch, gch))
            continue;

        act("$n looks more courageous!", gch, NULL, NULL, TO_ROOM);
        send_to_char("You feel courage wash over you!\n\r", gch);
        af.type = sn;
        af.duration = 4 + (level / 5);
        af.location = APPLY_HITROLL;
        af.modifier = get_pseudo_level(ch) / 12;
        af.bitvector = 0;
        af.save = TRUE;
        affect_to_char(gch, &af);
    }
    send_to_char("You inspire the troops!\n\r", ch);

    return TRUE;
}

bool
spell_ice_bolt(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = {
        0,
        0, 0, 0, 0, 0, 0, 0, 0, 25, 28,
        31, 34, 37, 40, 40, 41, 42, 42, 43, 44,
        44, 45, 46, 46, 47, 48, 48, 49, 50, 50,
        51, 52, 52, 53, 54, 54, 55, 56, 56, 57,
        58, 58, 59, 60, 60, 61, 62, 62, 63, 64
    };
    int                 dam;

    level = UMIN(level, sizeof(dam_each) / sizeof(dam_each[0]) - 1);
    level = UMAX(0, level);
    dam = number_range(dam_each[level] / 2, dam_each[level] * 2);
    if (saves_spell(level, victim))
        dam /= 1.2;
    if (obj == NULL) {
        act("A bolt of ice flies from $n's hand!", ch, NULL, NULL, TO_ROOM);
        send_to_char("A bolt of ice flies from your hand!\n\r", ch);
    }
    else {
        act("A bolt of ice flies from $p!", ch, obj, NULL, TO_ROOM);
        act("A bolt of ice flies from $p!", ch, obj, NULL, TO_CHAR);
    }
    act("$n is struck by the ice bolt!!", victim, NULL, NULL, TO_ROOM);
    send_to_char("You are struck by the ice bolt!!\n\r", victim);
    damage(ch, victim, dam, -1);
    return TRUE;
}

bool
spell_waterelem(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *summoned;

    if (IS_NPC(ch))
        return FALSE;

    if (!can_summon_charmie(ch))
        return FALSE;

    act("$n calls upon the elemental forces of @@lwater@@N!", ch, obj, NULL, TO_ROOM);
    act("You call upon the elemental forces of @@lwater@@N.", ch, obj, NULL, TO_CHAR);
    act("A waterfall appears, and an elemental steps from the pool!!", ch, obj, NULL, TO_ROOM);
    act("A waterfall appears, and an elemental steps from the pool!!", ch, obj, NULL, TO_CHAR);

    summoned = create_mobile(get_mob_index(MOB_VNUM_WATERELEM));
    char_to_room(summoned, ch->in_room);
    act("$n floods from the pool, drawing all the water into its body.", summoned, NULL, NULL, TO_ROOM);

    /* don't think we need these  
       summoned->level    = 40;    
       summoned->max_hit  = dice( 8, 40 );
       summoned->hit      = summoned->max_hit;
       summoned->max_move = dice( 10, 40 );
       summoned->move     = summoned->max_move;  
     */
    SET_BIT(summoned->act, ACT_PET);
    SET_BIT(summoned->affected_by, AFF_CHARM);
    summoned->extract_timer = get_pseudo_level(ch) / 3;
    add_follower(summoned, ch);
    return TRUE;
}

bool
spell_skeleton(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *summoned;

    if (IS_NPC(ch))
        return FALSE;

    if (!can_summon_charmie(ch))
        return FALSE;

    act("$n calls upon the @@dNegative Plane@@N!", ch, obj, NULL, TO_ROOM);
    act("You call upon the @@dNegative Plane@@N.", ch, obj, NULL, TO_CHAR);

    act("The ground opens beneath you, and a skeleton crawls out of the graveyard filth!!", ch, obj, NULL, TO_ROOM);
    act("The ground opens beneath you, and a skeleton crawls out of the graveyard filth!!", ch, obj, NULL, TO_CHAR);

    summoned = create_mobile(get_mob_index(MOB_VNUM_SKELETON));
    char_to_room(summoned, ch->in_room);
    act("$n stands erect, and bows towards its master.", summoned, NULL, NULL, TO_ROOM);

    /* 
       summoned->level    = 40;
       summoned->max_hit  = dice( 8, 40 );
       summoned->hit      = summoned->max_hit;
       summoned->max_move = dice( 10, 40 );
       summoned->move     = summoned->max_move;  
     */

    SET_BIT(summoned->act, ACT_PET);
    SET_BIT(summoned->affected_by, AFF_CHARM);
    summoned->extract_timer = get_pseudo_level(ch) / 3;
    add_follower(summoned, ch);
    return TRUE;
}

bool
spell_poison_weapon(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    OBJ_DATA           *ob = (OBJ_DATA *) vo;
    AFFECT_DATA        *paf;

    /* Quick way to stop imms (Bash?) enchanting weapons for players */
    if (IS_IMMORTAL(ch) && ch->level != 90) {
        send_to_char("Nothing Happens.\n\r", ch);
        return FALSE;
    }

    if (ob->item_type != ITEM_WEAPON) {
        send_to_char("That is not a weapon!.\n\r", ch);
        return FALSE;
    }

    /* This is gonna fuck up a lot.. paf->type isnt saved for objects in
       pfiles.. -- Alty */

    for (paf = ob->first_apply; paf != NULL; paf = paf->next) {
        if (paf->type == skill_table[sn].slot) {
            send_to_char("That weapon is already poisoned!\n\r", ch);
            return FALSE;
        }
    }

    GET_FREE(paf, affect_free);
    /* slots don't change, but sn's do! argh! */
    paf->type = skill_table[sn].slot;
    paf->duration = -1;
    paf->location = APPLY_DAMROLL;
    paf->modifier = UMIN((level / 30) + 1, ob->level);
    paf->bitvector = 0;
    paf->save = TRUE;
    paf->caster = NULL;
    LINK(paf, ob->first_apply, ob->last_apply, next, prev);

    return TRUE;
}

void
do_disguise(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_STRING_LENGTH];
    char                buf[MSL];

    if (!IS_NPC(ch) && ch->pcdata->learned[gsn_disguise] == 0) {
        send_to_char("You are not trained in this skill!\n\r", ch);
        return;
    }

    one_argument(argument, arg);

    if (arg[0] == '\0') {
        send_to_char(" Enter the disguise name, or reset to reset your description to normal.\n\r", ch);
        return;
    }

    if (!str_cmp(arg, "reset")) {
        free_string(ch->long_descr);
        ch->long_descr = str_dup(ch->long_descr_orig);
        return;
    }
    else {
        if (my_strlen(argument) > 128)
            strcpy(argument, my_left(argument, buf, 128));

        free_string(ch->long_descr);

        strcpy(buf, "(");
        safe_strcat(MAX_STRING_LENGTH, buf, argument);
        safe_strcat(MAX_STRING_LENGTH, buf, "\n\r");
        ch->long_descr = str_dup(buf);
        send_to_char("You are now Disguised!!!\n\r", ch);
        return;
    }

    return;
}

bool
spell_fireshield(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    SHIELD_DATA         *shield;
    AFFECT_DATA         af;
    bool                char_login = FALSE;

    if ((!IS_NPC(ch))
        && (ch->desc != NULL)
        && (ch->desc->connected == CON_SETTING_STATS))
        char_login = TRUE;

    if (ch->first_shield != NULL && !IS_IMMORTAL(ch) && (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM)))
        return FALSE;

    if (is_shielded(ch, SHIELD_FIRE))
        return FALSE;

    af.type = sn;
    af.duration = get_pseudo_level(ch) / 10;
    if (char_login)
        af.duration /= 2;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = 0;
    af.save = TRUE;
    affect_to_char(ch, &af);

    GET_FREE(shield, shield_free);
    shield->index    = SHIELD_FIRE;
    shield->hit      = 5000 + (get_pseudo_level(ch) * 10);

    if (char_login)
        shield->hit /= (number_range(2, 10));

    LINK(shield, ch->first_shield, ch->last_shield, next, prev);

    act(shield_table[SHIELD_FIRE].add_self, ch, NULL, NULL, TO_CHAR);
    act(shield_table[SHIELD_FIRE].add_room, ch, NULL, NULL, TO_ROOM);

    return TRUE;
}

bool
spell_iceshield(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    SHIELD_DATA         *shield;
    AFFECT_DATA         af;
    bool                char_login = FALSE;

    if ((!IS_NPC(ch))
        && (ch->desc != NULL)
        && (ch->desc->connected == CON_SETTING_STATS))
        char_login = TRUE;

    if (ch->first_shield != NULL && !IS_IMMORTAL(ch) && (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM)))
        return FALSE;

    if (is_shielded(ch, SHIELD_ICE))
        return FALSE;

    af.type = sn;
    af.duration = get_pseudo_level(ch) / 15;
    if (char_login)
        af.duration /= 2;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = 0;
    af.save = TRUE;
    affect_to_char(ch, &af);

    GET_FREE(shield, shield_free);
    shield->index    = SHIELD_ICE;
    shield->hit      = 15000 + (get_pseudo_level(ch) * 50);

    if (char_login)
        shield->hit /= (number_range(2, 10));

    LINK(shield, ch->first_shield, ch->last_shield, next, prev);

    act(shield_table[SHIELD_ICE].add_self, ch, NULL, NULL, TO_CHAR);
    act(shield_table[SHIELD_ICE].add_room, ch, NULL, NULL, TO_ROOM);

    return TRUE;
}

bool
spell_shockshield(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    SHIELD_DATA         *shield;
    AFFECT_DATA         af;
    bool                char_login = FALSE;

    if ((!IS_NPC(ch))
        && (ch->desc != NULL)
        && (ch->desc->connected == CON_SETTING_STATS))
        char_login = TRUE;

    if (ch->first_shield != NULL && !IS_IMMORTAL(ch) && (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM)))
        return FALSE;

    if (is_shielded(ch, SHIELD_SHOCK))
        return FALSE;

    af.type = sn;
    af.duration = get_pseudo_level(ch) / 7;
    if (char_login)
        af.duration /= 2;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = 0;
    af.save = TRUE;
    affect_to_char(ch, &af);

    GET_FREE(shield, shield_free);
    shield->index    = SHIELD_SHOCK;
    shield->hit      = 8000 + get_pseudo_level(ch) * 20;
    if (char_login)
        shield->hit /= (number_range(2, 10));

    LINK(shield, ch->first_shield, ch->last_shield, next, prev);

    act(shield_table[SHIELD_SHOCK].add_self, ch, NULL, NULL, TO_CHAR);
    act(shield_table[SHIELD_SHOCK].add_room, ch, NULL, NULL, TO_ROOM);

    return TRUE;
}

bool
spell_demonshield(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    SHIELD_DATA         *shield;
    AFFECT_DATA         af;
    bool                char_login = FALSE;

    if ((!IS_NPC(ch))
        && (ch->desc != NULL)
        && (ch->desc->connected == CON_SETTING_STATS))
        char_login = TRUE;

    if (ch->first_shield != NULL && !IS_IMMORTAL(ch) && (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM)))
        return FALSE;

    if (is_shielded(ch, SHIELD_DEMON))
        return FALSE;

    af.type = sn;
    af.duration = get_pseudo_level(ch) / 10;
    if (char_login)
        af.duration /= 2;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = 0;
    af.save = TRUE;
    affect_to_char(ch, &af);

    GET_FREE(shield, shield_free);
    shield->index    = SHIELD_DEMON;
    shield->hit      = 6000 + (get_pseudo_level(ch) * 10);

    if (char_login)
        shield->hit /= (number_range(2, 10));

    LINK(shield, ch->first_shield, ch->last_shield, next, prev);
    act(shield_table[SHIELD_DEMON].add_self, ch, NULL, NULL, TO_CHAR);
    act(shield_table[SHIELD_DEMON].add_room, ch, NULL, NULL, TO_ROOM);
    return TRUE;
}

bool
spell_shadowshield(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
/*
    AFFECT_DATA         af;
    bool                char_login = FALSE;

    if ((!IS_NPC(ch))
        && (ch->desc != NULL)
        && (ch->desc->connected == CON_SETTING_STATS))
        char_login = TRUE;

    if (ch->first_shield != NULL) {
        return FALSE;
    }

    af.type = sn;
    af.duration = get_pseudo_level(ch) / 10;
    if (char_login)
        af.duration /= 2;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = 0;
    af.save = TRUE;
    affect_to_char(ch, &af);

    GET_FREE(shield, shield_free);
    shield->name = str_dup("@@dSHADOW@@N");
    shield->type = FLAME_SHIELD;
    shield->harmfull = TRUE;
    shield->attack_dam = number_range((level * 3), (level * 5));
    shield->percent = 0;
    shield->hits = 5000 + (get_pseudo_level(ch) * 10);
    shield->sn = sn;
    if (char_login)
        shield->hits /= (number_range(2, 10));

    shield->absorb_message_room = str_dup("@@N$n's @@dShield@@N darkens, and sucks the @@dlife@@N out of $N@@N!!");
    shield->absorb_message_victim = str_dup("@@N$N's @@dShield@@N darkens, and sucks at your @@dlifeforce@@N!!");
    shield->absorb_message_self = str_dup("@@NYour @@dShield@@N darkens, and sucks out $N's @@dlifeforce@@N!!!");
    shield->wearoff_room = str_dup("@@N$n's @@dShield@@N @@WDISSIPATES@@N!!!!");
    shield->wearoff_self = str_dup("@@NYour @@dShield@@N @@WDISSIPATES@@N!!!!");

    LINK(shield, ch->first_shield, ch->last_shield, next, prev);

    send_to_char("You are surrounded by a @@ddark shadow@@N!\n\r", ch);
    act("$n becomes shrouded by a @@ddark shadow@@N!", ch, NULL, NULL, TO_ROOM);
*/
    return TRUE;
}

bool
spell_thoughtshield(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
/*
    AFFECT_DATA         af;
    bool                char_login = FALSE;

    if ((!IS_NPC(ch))
        && (ch->desc != NULL)
        && (ch->desc->connected == CON_SETTING_STATS))
        char_login = TRUE;

    if (ch->first_shield != NULL) {
        return FALSE;
    }

    af.type = sn;
    af.duration = get_pseudo_level(ch) / 10;
    if (char_login)
        af.duration /= 2;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = 0;
    af.save = TRUE;
    affect_to_char(ch, &af);

    GET_FREE(shield, shield_free);
    shield->name = str_dup("@@mTHOUGHT@@N");
    shield->type = FLAME_SHIELD;
    shield->harmfull = TRUE;
    shield->attack_dam = number_range((level * 2), (level * 4.5));
    shield->percent = 20;
    shield->hits = 3000;
    shield->sn = sn;
    if (char_login)
        shield->hits /= (number_range(2, 10));

    shield->absorb_message_room = str_dup("@@N$n's @@mShield@@N feeds upon $N's @@mpsionic energy@@N!!");
    shield->absorb_message_victim = str_dup("@@N$N's @@mShield@@N feeds upon your @@mpsionic energy@@N!!");
    shield->absorb_message_self = str_dup("@@NYour @@mShield@@N feeds upon $N's @@mpsionic energy@@N!!!");
    shield->wearoff_room = str_dup("@@N$n's @@mShield@@N @@dDISSIPATES@@N!!!!");
    shield->wearoff_self = str_dup("@@NYour @@mShield@@N @@dDISSIPATES@@N!!!!");

    LINK(shield, ch->first_shield, ch->last_shield, next, prev);

    send_to_char("You are surrounded by a @@mpsionic shield@@N!\n\r", ch);
    act("$n is surrounded by a @@mpsionic shield@@N!", ch, NULL, NULL, TO_ROOM);
*/
    return TRUE;
}

bool
spell_ethereal(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{

    /* Ethereal travel :)
     * 
     * Zenithar
     */

    if (IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)) {
        send_to_char("You failed.\n\r", ch);
        return FALSE;
    }

    /* Check is ch screws up, and ends up in limbo... <grin> */

    if ((number_percent() < 15)
        && (!IS_NPC(ch))) {        /* 15% chance */
        send_to_char("@@NYou get distracted, and appear in the middle of @@dnowhere@@N!\n\r", ch);
        act("@@N$n begins to disperse into @@lghostly @@Nparticles, then disappears!!", ch, NULL, NULL, TO_ROOM);
        char_from_room(ch);
        char_to_room(ch, get_room_index(ROOM_VNUM_LIMBO));
        if (ch->riding != NULL) {
            char_from_room(ch->riding);
            char_to_room(ch->riding, get_room_index(ROOM_VNUM_LIMBO));
        }

        act("@@N$n coelesces from a @@lghostly@@N image into the room.", ch, NULL, NULL, TO_ROOM);
        do_look(ch, "auto");
        return TRUE;
    }

    act("@@N$n begins to disperse into @@lghostly @@Nparticles, then disappears!!", ch, NULL, NULL, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, get_room_index(ROOM_VNUM_ETHEREAL_PLANE));
    if (ch->riding != NULL) {
        char_from_room(ch->riding);
        char_to_room(ch->riding, get_room_index(ROOM_VNUM_ETHEREAL_PLANE));
    }

    act("@@N$n coelesces from a @@lghostly@@N image into the room.", ch, NULL, NULL, TO_ROOM);
    send_to_char("@@NYou enter the @@lEthereal Plane@@N!\n\r", ch);
    do_look(ch, "auto");
    return TRUE;
}

bool
spell_fireelem(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *summoned;

    if (IS_NPC(ch))
        return FALSE;

    if (!can_summon_charmie(ch))
        return FALSE;

    act("$n calls upon the elemental forces of @@efire@@N!", ch, obj, NULL, TO_ROOM);
    act("You call upon the elemental forces of @@efire@@N.", ch, obj, NULL, TO_CHAR);

    act("@@NA @@ebonfire@@N appears, and an elemental steps from the flames!!", ch, obj, NULL, TO_ROOM);
    act("@@NA @@ebonfire@@N appears, and an elemental steps from the flames!!", ch, obj, NULL, TO_CHAR);

    summoned = create_mobile(get_mob_index(MOB_VNUM_FIREELEM));
    char_to_room(summoned, ch->in_room);
    act("$n leaps from the bonfire, drawing all the @@eflames@@N into its body.", summoned, NULL, NULL, TO_ROOM);

    /* don't think we need these  
       summoned->level    = 40;    
       summoned->max_hit  = dice( 8, 40 );
       summoned->hit      = summoned->max_hit;
       summoned->max_move = dice( 10, 40 );
       summoned->move     = summoned->max_move;  

     */
    SET_BIT(summoned->act, ACT_PET);
    SET_BIT(summoned->affected_by, AFF_CHARM);
    summoned->extract_timer = get_pseudo_level(ch) / 3;
    add_follower(summoned, ch);
    return TRUE;
}

bool
spell_rune_fire(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    ROOM_INDEX_DATA    *room;
    ROOM_AFFECT_DATA    raf;

    room = ch->in_room;

    if (room == NULL)
        return FALSE;

    if (IS_SET(room->affected_by, ROOM_BV_FIRE_RUNE)) {
        send_to_char("@@NThere is already a @@eFire@@N Rune operating here!\n\r", ch);
        return FALSE;
    }

    if (IS_SET(room->area->flags, AREA_NO_ROOM_AFF) && (IS_NPC(ch) || !IS_IMMORTAL(ch))) {
        send_to_char(DEMON_WAGGLE, ch);
        return FALSE;
    }

    act("$n draws a mystical @@eFire@@N Rune in the air.", ch, NULL, NULL, TO_ROOM);
    send_to_char("@@NYou draw a @@eFire@@N Rune in the air.\n\r", ch);

    raf.type = sn;
    raf.duration = (level) + number_range(2, 20);
    raf.level = level;
    raf.bitvector = ROOM_BV_FIRE_RUNE;
    raf.caster = ch;
    raf.modifier = (level * 3) - number_range(10, 50);
    raf.name = str_dup("");
    affect_to_room(room, &raf);
    return TRUE;
}

bool
spell_rune_poison(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    ROOM_INDEX_DATA    *room;
    ROOM_AFFECT_DATA    raf;

    room = ch->in_room;

    if (room == NULL)
        return FALSE;

    if (IS_SET(room->affected_by, ROOM_BV_POISON_RUNE)) {
        send_to_char("@@NThere is already a @@dPoison@@N Rune operating here!\n\r", ch);
        return FALSE;
    }
    if (IS_SET(room->area->flags, AREA_NO_ROOM_AFF) && (IS_NPC(ch) || !IS_IMMORTAL(ch))) {
        send_to_char(DEMON_WAGGLE, ch);
        return FALSE;
    }
    act("$n draws a mystical @@dPoison@@N Rune in the air.", ch, NULL, NULL, TO_ROOM);
    send_to_char("@@NYou draw a @@dPoison@@N Rune in the air.\n\r", ch);

    raf.type = sn;
    raf.duration = (level) + number_range(2, 20);
    raf.level = level;
    raf.bitvector = ROOM_BV_POISON_RUNE;
    raf.caster = ch;
    raf.modifier = 0;
    raf.name = str_dup("");
    affect_to_room(room, &raf);
    return TRUE;
}

bool
spell_rune_shock(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    ROOM_INDEX_DATA    *room;
    ROOM_AFFECT_DATA    raf;

    room = ch->in_room;

    if (room == NULL)
        return FALSE;

    if (IS_SET(room->affected_by, ROOM_BV_SHOCK_RUNE)) {
        send_to_char("@@NThere is already a @@lShock@@N Rune operating here!\n\r", ch);
        return FALSE;
    }

    if (IS_SET(room->area->flags, AREA_NO_ROOM_AFF) && (IS_NPC(ch) || !IS_IMMORTAL(ch))) {
        send_to_char(DEMON_WAGGLE, ch);
        return FALSE;
    }
    act("$n draws a mystical @@lShock@@N Rune in the air.", ch, NULL, NULL, TO_ROOM);
    send_to_char("@@NYou draw a @@lShock@@N Rune in the air.\n\r", ch);

    raf.type = sn;
    raf.duration = (level) + number_range(2, 20);
    raf.level = level;
    raf.bitvector = ROOM_BV_SHOCK_RUNE;
    raf.caster = ch;
    raf.modifier = (level * 3) - number_range(10, 50);
    raf.name = str_dup("");

    affect_to_room(room, &raf);
    return TRUE;
}

bool
spell_healing_light(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    ROOM_INDEX_DATA    *room;
    ROOM_AFFECT_DATA    raf;

    room = ch->in_room;
    if (room == NULL)
        return FALSE;

    if (IS_SET(room->affected_by, ROOM_BV_HEAL_REGEN)) {
        send_to_char("@@NThere is already a @@mHealing Light@@N operating here!\n\r", ch);
        return FALSE;
    }
    act("@@NA majestic @@mHealing Light@@N fills the room.", ch, NULL, NULL, TO_ROOM);
    send_to_char("@@NYou fill the room with a majestic @@mHealing Light@@N.\n\r", ch);
    raf.type = sn;
    raf.duration = (level / 8) + number_range(2, 10);
    raf.level = level;
    raf.bitvector = ROOM_BV_HEAL_REGEN;
    raf.caster = ch;
    raf.modifier = 0;
    raf.name = str_dup("");
    affect_to_room(room, &raf);

    return TRUE;
}

bool
spell_wither_shadow(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    ROOM_INDEX_DATA    *room;
    ROOM_AFFECT_DATA    raf;

    room = ch->in_room;
    if (room == NULL)
        return FALSE;
    if (IS_SET(room->affected_by, ROOM_BV_HEAL_STEAL)) {
        send_to_char("@@NThere is already a @@dWithering Shadow@@N operating here!\n\r", ch);
        return FALSE;
    }

    act("@@NA deadly @@dWithering Shadow@@N fills the room.", ch, NULL, NULL, TO_ROOM);
    send_to_char("@@NYou fill the room with a deadly @@dWithering Shadow@@N.\n\r", ch);

    raf.type = sn;
    raf.duration = (level / 16) + number_range(2, 5);
    raf.level = level;
    raf.bitvector = ROOM_BV_HEAL_STEAL;
    raf.caster = ch;
    raf.modifier = 0;
    raf.name = str_dup("");

    affect_to_room(room, &raf);
    return TRUE;
}

bool
spell_mana_flare(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    ROOM_INDEX_DATA    *room;
    ROOM_AFFECT_DATA    raf;

    room = ch->in_room;

    if (room == NULL)
        return FALSE;

    if (IS_SET(room->affected_by, ROOM_BV_MANA_REGEN)) {
        send_to_char("@@NThere is already a @@eMana Flare@@N operating here!\n\r", ch);
        return FALSE;
    }

    act("@@NA powerful @@eMana Flare@@N encompasses the room.", ch, NULL, NULL, TO_ROOM);
    send_to_char("@@NYou fill the room with a powerful @@eMana Flare@@N.\n\r", ch);

    raf.type = sn;
    raf.duration = (level / 8) + number_range(2, 10);
    raf.level = level;
    raf.bitvector = ROOM_BV_MANA_REGEN;
    raf.caster = ch;
    raf.modifier = 0;
    raf.name = str_dup("");

    affect_to_room(room, &raf);
    return TRUE;
}

bool
spell_mana_drain(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    ROOM_INDEX_DATA    *room;
    ROOM_AFFECT_DATA    raf;

    room = ch->in_room;

    if (room == NULL)
        return FALSE;

    if (IS_SET(room->affected_by, ROOM_BV_MANA_STEAL)) {
        send_to_char("@@NThere is already a @@dMana Drain@@N operating here!\n\r", ch);
        return FALSE;
    }

    if (IS_SET(room->area->flags, AREA_NO_ROOM_AFF) && (IS_NPC(ch) || !IS_IMMORTAL(ch))) {
        send_to_char(DEMON_WAGGLE, ch);
        return FALSE;
    }
    act("@@NA mind-sapping @@dMana Drain@@N fills the room.", ch, NULL, NULL, TO_ROOM);
    send_to_char("@@NYou fill the room with a mind-sapping @@dMana Drain@@N.\n\r", ch);

    raf.type = sn;
    raf.duration = (level / 8) + number_range(2, 10);
    raf.level = level;
    raf.bitvector = ROOM_BV_MANA_STEAL;
    raf.caster = ch;
    raf.modifier = 0;
    raf.name = str_dup("");

    affect_to_room(room, &raf);
    return TRUE;
}

bool
spell_cage(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    ROOM_INDEX_DATA    *room;
    ROOM_AFFECT_DATA    raf;
    CHAR_DATA          *caster;
    int                 lvl;

    /* initial support for group-range caging */
    if (IS_NPC(ch))
        if (is_charmie_of(ch->master, ch))
            caster = ch->master;
        else
            caster = ch;
    else
        caster = ch;

    lvl = get_pseudo_level(caster);

    room = ch->in_room;

    if (room == NULL)
        return FALSE;

    if (IS_SET(room->affected_by, ROOM_BV_HOLD)) {
        send_to_char("@@NThere is already a @@rCage@@N operating here!\n\r", ch);
        return FALSE;
    }
    /*   if ( IS_SET( room->area->flags, AREA_NO_ROOM_AFF ) )
       {
       send_to_char( "@@gA strange @@ySexy @@eDeamon@@g appears, waggles her finger at you, and shakes her head!@@N\n\r", ch );
       return FALSE;
       }
     */
    act("@@NA paralysing @@rCage@@N surrounds the room.", ch, NULL, NULL, TO_ROOM);
    send_to_char("@@NYou surround the room with a paralyzing @@rCage@@N.\n\r", ch);

    raf.type = sn;
    raf.duration = (level / 20) + number_range(2, 10);
    raf.level = level;
    raf.bitvector = ROOM_BV_HOLD;
    raf.caster = ch;
    raf.modifier = lvl;
    raf.name = str_dup("");

    affect_to_room(room, &raf);
    return TRUE;
}

bool
spell_cloak_flaming(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    AFFECT_DATA         af;

    if ((IS_AFFECTED(ch, AFF_CLOAK_MANA))) {
        send_to_char("You can't use cloak:@@eFlaming@@N while using Cloak:@@yMana@@N.\n\r", ch);
        return FALSE;
    }

    if (is_affected(ch, sn))
        return FALSE;

    af.type = sn;
    af.duration = ch->level / 8;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = AFF_CLOAK_FLAMING;
    af.save = TRUE;
    affect_to_char(ch, &af);

    act(cloak_table[CLOAK_FLAMING].add_self, ch, NULL, NULL, TO_CHAR);
    act(cloak_table[CLOAK_FLAMING].add_room, ch, NULL, NULL, TO_ROOM);

    return TRUE;
}

bool
spell_cloak_reflect(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{

    AFFECT_DATA         af;

    if (is_affected(ch, sn))
        return FALSE;

    af.type = sn;
    af.duration = ch->level / 8;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = AFF_CLOAK_REFLECTION;
    af.save = TRUE;
    affect_to_char(ch, &af);

    act(cloak_table[CLOAK_REFLECT].add_self, ch, NULL, NULL, TO_CHAR);
    act(cloak_table[CLOAK_REFLECT].add_room, ch, NULL, NULL, TO_ROOM);

    return TRUE;
}

bool
spell_cloak_absorb(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{

    AFFECT_DATA         af;

    if (is_affected(ch, sn))
        return FALSE;
    af.type = sn;
    af.duration = ch->level / 8;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = AFF_CLOAK_ABSORPTION;
    af.save = TRUE;
    affect_to_char(ch, &af);

    act(cloak_table[CLOAK_ABSORB].add_self, ch, NULL, NULL, TO_CHAR);
    act(cloak_table[CLOAK_ABSORB].add_room, ch, NULL, NULL, TO_ROOM);

    return TRUE;
}

bool
spell_cloak_adept(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{

    AFFECT_DATA         af;

    if (is_affected(ch, sn))
        return FALSE;

    if (IS_NPC(ch))
        return FALSE;

    if (ch->pcdata->avatar) {
        send_to_char("Avatars have a permanent adept cloak.\n\r", ch);
        return FALSE;
    }

    af.type = sn;
    af.duration = get_pseudo_level(ch) / 5;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = AFF_CLOAK_ADEPT;
    af.save = TRUE;
    affect_to_char(ch, &af);

    act(cloak_table[CLOAK_ADEPT].add_self, ch, NULL, NULL, TO_CHAR);
    act(cloak_table[CLOAK_ADEPT].add_room, ch, NULL, NULL, TO_ROOM);

    return TRUE;
}

bool
spell_cloak_mana(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{

    AFFECT_DATA         af;

    if (is_affected(ch, sn))
        return FALSE;

    if ((IS_AFFECTED(ch, AFF_CLOAK_FLAMING))) {
        send_to_char("You can't use cloak: @@yMana@@N while using cloak: @@eFlaming@@N.\n\r", ch);
        return FALSE;
    }

    af.type = sn;
    af.duration = ch->level / 6;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = AFF_CLOAK_MANA;
    af.save = TRUE;
    affect_to_char(ch, &af);

    act(cloak_table[CLOAK_MANA].add_self, ch, NULL, NULL, TO_CHAR);
    act(cloak_table[CLOAK_MANA].add_room, ch, NULL, NULL, TO_ROOM);

    return TRUE;
}

bool
spell_cloak_regen(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    AFFECT_DATA         af;

    if (is_affected(ch, sn))
        return FALSE;

    af.type = sn;
    af.duration = get_pseudo_level(ch) / 10;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = AFF_CLOAK_REGEN;
    af.save = TRUE;
    affect_to_char(ch, &af);

    act(cloak_table[CLOAK_REGEN].add_self, ch, NULL, NULL, TO_CHAR);
    act(cloak_table[CLOAK_REGEN].add_room, ch, NULL, NULL, TO_ROOM);

    return TRUE;
}

bool
spell_room_dispel(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    ROOM_INDEX_DATA    *room;
    ROOM_AFFECT_DATA   *raf;
    ROOM_AFFECT_DATA   *raf_next;
    int                 chance = 0;

    room = ch->in_room;

    if (room == NULL)
        return FALSE;

    chance = ch->level + 20;

    act("$n gestures demonically at the magical spells around the room.", ch, NULL, NULL, TO_ROOM);
    send_to_char("@@NYou gesture demonically!\n\r", ch);

    for (raf = room->first_room_affect; raf != NULL; raf = raf_next) {
        raf_next = raf->next;

        if (raf->bitvector == ROOM_BV_SMOKESCREEN || raf->bitvector == ROOM_BV_SMOKESCREEN_AREA || raf->bitvector == ROOM_BV_SENTRY)
            continue;

        if (number_percent() < chance) {
            r_affect_remove(room, raf);
            chance = (2 * chance) / 3;
        }
        else
            break;
    }

    return TRUE;
}

bool
spell_throw_star(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (!saves_spell(level, victim) && victim->race != RACE_LAM) {
        af.type = sn;
        af.duration = 12 + (level / 10);
        af.location = APPLY_STR;
        af.modifier = level / 12 + 1;
        af.bitvector = AFF_POISON;
        af.save = TRUE;
        affect_join(victim, &af);
        send_to_char("You feel very sick.\n\r", victim);
        act("$n looks very sick.", victim, NULL, NULL, TO_ROOM);
    }
    damage(ch, (CHAR_DATA *) vo, number_range(level / 2, level * 5), sn);

    return TRUE;
}

bool
spell_soul_net(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    ROOM_INDEX_DATA    *room;
    ROOM_AFFECT_DATA    raf;

    room = ch->in_room;

    if (room == NULL)
        return FALSE;
    if (IS_SET(room->affected_by, ROOM_BV_SOUL_NET)) {
        send_to_char("@@NThere is already a @@dSoul Net@@N operating here!\n\r", ch);
        return FALSE;
    }
    act("@@NA demonic @@dSoul Net@@N enshrouds the room.", ch, NULL, NULL, TO_ROOM);
    send_to_char("@@NYou enshroud the room with a demonic @@dSoul Net@@N.\n\r", ch);

    raf.type = sn;
    raf.duration = (level / 8);
    raf.level = level;
    raf.bitvector = ROOM_BV_SOUL_NET;
    raf.caster = ch;
    raf.modifier = 0;
    raf.name = str_dup("");

    affect_to_room(room, &raf);
    return TRUE;
}

bool
spell_condense_soul(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    OBJ_DATA           *ob = (OBJ_DATA *) vo;
    OBJ_DATA           *potion = NULL;

    if (IS_NPC(ch))
        return FALSE;

    if (ob->item_type != ITEM_SOUL) {
        send_to_char("That is not a soul!\n\r", ch);
        return FALSE;
    }
    if (ob->level < 80) {
        send_to_char("Bah! That soul is too weak to use!\n\r", ch);
        return FALSE;
    }
    if (ch->pcdata->hp_from_gain <= 75) {
        send_to_char("You have exhausted your life force, and are unable to" " control the necromantic forces necessary for this act.\n\r", ch);
        return FALSE;
    }

    if ((potion = create_object(get_obj_index(OBJ_VNUM_SOUL_POTION), level)) == NULL) {
        send_to_char("Unable to create a soul potion.\n\r", ch);
        return FALSE;
    }

    extract_obj(ob);
    obj_to_char(potion, ch);
    act("@@N$n gestures diabolically, and his captured soul condenses into a @@dSoul Potion@@N.", ch, NULL, NULL, TO_ROOM);
    send_to_char("@@NYou condense the soul and some of your life force into a @@dSoul potion@@N.\n\r", ch);
    ch->max_hit -= 75;
    ch->pcdata->hp_from_gain -= 75;

    sprintf(log_buf, "%s uses condense soul", ch->name);
    monitor_chan(log_buf, MONITOR_BAD);
    log_string(log_buf);
    return TRUE;
}

bool
spell_restoration(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    OBJ_DATA           *ob;

    if (IS_NPC(ch))
        return FALSE;

    ch->hit = ch->max_hit;
    ch->mana = ch->max_mana;
    ch->move = ch->max_move;

    for (ob = ch->first_carry; ob != NULL; ob = ob->next_in_carry_list)
        if (IS_SET(ob->item_apply, ITEM_APPLY_HEATED)) {
            act("@@N@@g$p@@N@@g cools down!@@N", ch, ob, NULL, TO_CHAR);
            REMOVE_BIT(ob->item_apply, ITEM_APPLY_HEATED);
        }

    send_to_char("@@eThe life force restores you!@@N\n\r", ch);

    if (level != 666) {
        sprintf(log_buf, "%s has used a restoration spell.", ch->name);
        monitor_chan(log_buf, MONITOR_BAD);
    }

    return TRUE;
}

bool
spell_infuse(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    OBJ_DATA           *ob = (OBJ_DATA *) vo;
    AFFECT_DATA        *paf;
    OBJ_DATA           *obj_soul;

    if (IS_NPC(ch))
        return FALSE;

    /* Quick way to stop imms (Bash?) enchanting weapons for players */
    if (IS_IMMORTAL(ch) && ch->level != 90) {
        send_to_char("Nothing Happens.\n\r", ch);
        return FALSE;
    }

    if (IS_NPC(ch))
        return FALSE;

    if ((obj_soul = get_eq_char(ch, WEAR_MAGIC)) == NULL) {
        send_to_char("@@NYou must be holding a @@eSoul@@N to cast this spell!!\n\r", ch);
        return FALSE;
    }

    if (obj_soul->item_type != ITEM_SOUL) {
        send_to_char("@@NYou are not holding a @@eSoul@@N!!\n\r", ch);
        return FALSE;
    }

    if (ch->pcdata->hp_from_gain <= 100) {
        send_to_char("You have exhausted your life force, and are unable to" " control the necromantic forces necessary for this act.\n\r", ch);
        return FALSE;
    }

    if (ob->item_type != ITEM_WEAPON) {
        send_to_char("That is not a weapon!\n\r", ch);
        return FALSE;
    }

    for (paf = ob->first_apply; paf != NULL; paf = paf->next) {
        if (paf->type == skill_table[sn].slot) {
            send_to_char("That weapon is already infused with a soul!\n\r", ch);
            return FALSE;
        }
    }

    GET_FREE(paf, affect_free);
    /* slots don't change, but sn's do! argh! */
    paf->type = skill_table[sn].slot;
    paf->duration = -1;
    paf->location = APPLY_DAMROLL;
    paf->modifier = (obj_soul->level / 8);
    paf->bitvector = 0;
    paf->save = TRUE;
    paf->caster = NULL;

    LINK(paf, ob->first_apply, ob->last_apply, next, prev);

    GET_FREE(paf, affect_free);
    paf->type = skill_table[sn].slot;
    paf->duration = -1;
    paf->location = APPLY_HITROLL;
    paf->modifier = (obj_soul->level / 8);
    paf->bitvector = 0;
    paf->save = TRUE;
    paf->caster = NULL;

    LINK(paf, ob->first_apply, ob->last_apply, next, prev);

    SET_BIT(ob->extra_flags, ITEM_NODISARM);

    ob->obj_fun = obj_fun_lookup("objfun_infused_soul");
    extract_obj(obj_soul);

    act("@@N$n gestures diabolically, and his captured @@esoul@@N is infused into his $p.", ch, NULL, ob, TO_ROOM);
    send_to_char("@@NYou condense the soul and some of your life force into your weapon.\n\r", ch);
    ch->max_hit -= 100;
    ch->pcdata->hp_from_gain -= 100;

    xlogf("%s uses infuse on %s", ch->name, ob->short_descr);
    return TRUE;
}

bool
spell_holy_light(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *gch;

    for (gch = ch->in_room->first_person; gch != NULL; gch = gch->next_in_room) {
        if (!is_same_group(ch, gch))
            continue;
        act("$N is invigorated by the light shining off of $n!", ch, NULL, gch, TO_NOTVICT);
        send_to_char("You feel a warm light invigorate you!\n\r", gch);
        gch->hit = UMIN(gch->max_hit, (gch->hit + number_range(level, level * 2.5)));
    }
    send_to_char("You invigorate the troops!\n\r", ch);
    return TRUE;
}

bool
spell_holy_armor(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (is_affected(victim, sn))
        return FALSE;
    af.type = sn;
    af.duration = 12;
    af.modifier = -80;
    af.location = APPLY_AC;
    af.bitvector = 0;
    af.save = TRUE;
    affect_to_char(victim, &af);
    send_to_char("Your armor is now blessed!.\n\r", victim);
    if (ch != victim)
        send_to_char("Ok.\n\r", ch);
    return TRUE;
}

bool
spell_divine_intervention(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    DUEL_DATA          *duel;
    int                 heal;

    heal = UMAX(300, number_range(level * 4, level * 7));
    if (!IS_NPC(ch) && (duel = find_duel(ch)) && IS_SET(duel->flags, DUEL_DBLHEAL))
        heal *= 2;

    victim->hit = UMIN(victim->hit + heal, victim->max_hit);
    update_pos(victim);
    send_to_char("You feel the hand of your God invigorate your soul!\n\r", victim);
    if (ch != victim)
        send_to_char("Ok.\n\r", ch);
    return TRUE;
}

bool
spell_earthelem(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *summoned;

    if (IS_NPC(ch))
        return FALSE;

    if (!can_summon_charmie(ch))
        return FALSE;

    act("$n calls upon the elemental forces of @@bearth@@N!", ch, obj, NULL, TO_ROOM);
    act("You call upon the elemental forces of @@bearth@@N.", ch, obj, NULL, TO_CHAR);
    act("@@NA huge mound of @@bearth@@N appears, and an elemental steps from the boulders!!", ch, obj, NULL, TO_ROOM);
    act("@@NA huge mound of @@bearth@@N appears, and an elemental steps from the boulders!!", ch, obj, NULL, TO_CHAR);

    summoned = create_mobile(get_mob_index(MOB_VNUM_EARTHELEM));
    char_to_room(summoned, ch->in_room);
    act("@@N$n emerges from the mound, assuming the @@bearth@@N into its body.", summoned, NULL, NULL, TO_ROOM);
    /* don't think we need these  
       summoned->level    = 40;    
       summoned->max_hit  = dice( 8, 40 );
       summoned->hit      = summoned->max_hit;
       summoned->max_move = dice( 10, 40 );
       summoned->move     = summoned->max_move;  
     */
    SET_BIT(summoned->act, ACT_PET);
    SET_BIT(summoned->affected_by, AFF_CHARM);
    summoned->extract_timer = get_pseudo_level(ch) / 3;
    add_follower(summoned, ch);
    return TRUE;
}

bool
spell_iron_golem(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *summoned;

    if (IS_NPC(ch))
        return FALSE;

    if (!can_summon_charmie(ch))
        return FALSE;

    act("@@N$n calls upon the @@aalchemical@@N forces of @@dmetal@@N!", ch, obj, NULL, TO_ROOM);
    act("You call upon the @@aalchemical@@N forces of @@dmetal@@N.", ch, obj, NULL, TO_CHAR);

    act("@@NA large slab of @@diron@@N appears, and a golem steps from the mass!!", ch, obj, NULL, TO_ROOM);
    act("@@NA large slab of @@diron@@N appears, and a golem steps from the mass!!", ch, obj, NULL, TO_CHAR);

    summoned = create_mobile(get_mob_index(MOB_VNUM_IRON_GOLEM));
    char_to_room(summoned, ch->in_room);
    act("@@N$n mutates from the @@dslab of iron@@N, drawing all the metal into its body.", summoned, NULL, NULL, TO_ROOM);
    /* don't think we need these  
       summoned->level    = 40;    
       summoned->max_hit  = dice( 8, 40 );
       summoned->hit      = summoned->max_hit;
       summoned->max_move = dice( 10, 40 );
       summoned->move     = summoned->max_move;  
     */
    SET_BIT(summoned->act, ACT_PET);
    SET_BIT(summoned->affected_by, AFF_CHARM);
    summoned->extract_timer = get_pseudo_level(ch) / 3;
    add_follower(summoned, ch);
    return TRUE;
}

bool
spell_soul_thief(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *summoned;

    if (IS_NPC(ch))
        return FALSE;

    if (!can_summon_charmie(ch))
        return FALSE;

    act("$n calls upon the @@dNegative Plane@@N!", ch, obj, NULL, TO_ROOM);
    act("You call upon the @@dNegative Plane@@N.", ch, obj, NULL, TO_CHAR);

    act("@@NThe ground opens beneath you, and a @@lSoul @@eThief@@N crawls out of the graveyard filth!!", ch, obj, NULL, TO_ROOM);
    act("@@NThe ground opens beneath you, and a @@lSoul @@eThief@@N crawls out of the graveyard filth!!", ch, obj, NULL, TO_CHAR);

    summoned = create_mobile(get_mob_index(MOB_VNUM_SOUL_THIEF));
    char_to_room(summoned, ch->in_room);
    act("$n Stands erect, and bow's towards its master.", summoned, NULL, NULL, TO_ROOM);
    /* don't think we need these  
       summoned->level    = 40;    
       summoned->max_hit  = dice( 8, 40 );
       summoned->hit      = summoned->max_hit;
       summoned->max_move = dice( 10, 40 );
       summoned->move     = summoned->max_move;  
     */
    SET_BIT(summoned->act, ACT_PET);
    SET_BIT(summoned->affected_by, AFF_CHARM);
    summoned->extract_timer = get_pseudo_level(ch) / 3;
    add_follower(summoned, ch);
    return TRUE;
}

bool
spell_holy_avenger(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *summoned;

    if (IS_NPC(ch))
        return FALSE;

    if (!can_summon_charmie(ch))
        return FALSE;

    act("$n calls upon the holy forces of @@Wlight@@N!", ch, obj, NULL, TO_ROOM);
    act("You call upon the holy forces of @@Wlight@@N.", ch, obj, NULL, TO_CHAR);
    act("@@NA shimmering halo appears, and a @@yHoly @@WAvenger@@N steps from the light!!", ch, obj, NULL, TO_ROOM);
    act("@@NA shimmering halo appears, and a @@yHoly @@WAvenger@@N steps from the light!!", ch, obj, NULL, TO_CHAR);
    summoned = create_mobile(get_mob_index(MOB_VNUM_HOLY_AVENGER));
    char_to_room(summoned, ch->in_room);
    act("$n steps from the light, drawing all the power into its body.", summoned, NULL, NULL, TO_ROOM);
    /* don't think we need these  
       summoned->level    = 40;    
       summoned->max_hit  = dice( 8, 40 );
       summoned->hit      = summoned->max_hit;
       summoned->max_move = dice( 10, 40 );
       summoned->move     = summoned->max_move;  
     */
    SET_BIT(summoned->act, ACT_PET);
    SET_BIT(summoned->affected_by, AFF_CHARM);
    summoned->extract_timer = get_pseudo_level(ch) / 3;
    add_follower(summoned, ch);
    return TRUE;
}

bool
spell_diamond_golem(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *summoned;

    if (IS_NPC(ch))
        return FALSE;

    if (!can_summon_charmie(ch))
        return FALSE;

    act("@@N$n calls upon the @@aalchemical@@N forces of @@ylight@@N!", ch, obj, NULL, TO_ROOM);
    act("@@NYou call upon the @@aalchemical@@N forces of @@ylight@@N.", ch, obj, NULL, TO_CHAR);

    act("A huge gemstone appears, and a golem steps from the diamond!!", ch, obj, NULL, TO_ROOM);
    act("A huge gemstone appears, and a golem steps from the diamond!!", ch, obj, NULL, TO_CHAR);

    summoned = create_mobile(get_mob_index(MOB_VNUM_DIAMOND_GOLEM));
    char_to_room(summoned, ch->in_room);
    act("$n forms from the gemstone, drawing all the hardness into its body.", summoned, NULL, NULL, TO_ROOM);

    /* don't think we need these  
       summoned->level    = 40;    
       summoned->max_hit  = dice( 8, 40 );
       summoned->hit      = summoned->max_hit;
       summoned->max_move = dice( 10, 40 );
       summoned->move     = summoned->max_move;  

     */
    SET_BIT(summoned->act, ACT_PET);
    SET_BIT(summoned->affected_by, AFF_CHARM);
    summoned->extract_timer = get_pseudo_level(ch) / 3;
    add_follower(summoned, ch);
    return TRUE;
}

bool
spell_summon_pegasus(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *summoned;

    if (IS_NPC(ch))
        return FALSE;

    if (!can_summon_charmie(ch))
        return FALSE;

    act("$n calls upon the holy forces of @@Wlight@@N!", ch, obj, NULL, TO_ROOM);
    act("You call upon the holy forces of @@Wlight@@N.", ch, obj, NULL, TO_CHAR);

    act("@@NA shimmering pyramid appears, and a @@mMagestic @@WPegasus@@N steps from the light!!", ch, obj, NULL, TO_ROOM);
    act("@@NA shimmering pyramid appears, and a @@mMagestic @@WPegasus@@N steps from the light!!", ch, obj, NULL, TO_CHAR);

    summoned = create_mobile(get_mob_index(MOB_VNUM_PEGASUS));
    char_to_room(summoned, ch->in_room);
    act("$n steps from the light, drawing all the power into its body.", summoned, NULL, NULL, TO_ROOM);

    /* don't think we need these  
       summoned->level    = 40;    
       summoned->max_hit  = dice( 8, 40 );
       summoned->hit      = summoned->max_hit;
       summoned->max_move = dice( 10, 40 );
       summoned->move     = summoned->max_move;  

     */
    SET_BIT(summoned->act, ACT_PET);
    SET_BIT(summoned->affected_by, AFF_CHARM);
    summoned->extract_timer = get_pseudo_level(ch) / 3;
    add_follower(summoned, ch);
    return TRUE;
}

bool
spell_summon_nightmare(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *summoned;

    if (IS_NPC(ch))
        return FALSE;

    if (!can_summon_charmie(ch))
        return FALSE;

    act("$n calls upon the unholy forces of @@eEvil@@N!", ch, obj, NULL, TO_ROOM);
    act("You call upon the unholy forces of @@eEvil@@N.", ch, obj, NULL, TO_CHAR);

    act("@@NA flaming pit appears, and a @@eDemonic @@dNightmare@@N steps from the heat!!", ch, obj, NULL, TO_ROOM);
    act("@@NA flaming pit appears, and a @@eDemonic @@dNightmare@@N steps from the heat!!", ch, obj, NULL, TO_CHAR);

    summoned = create_mobile(get_mob_index(MOB_VNUM_NIGHTMARE));
    char_to_room(summoned, ch->in_room);
    act("$n steps from the flames, drawing all the power into its body.", summoned, NULL, NULL, TO_ROOM);

    /* don't think we need these  
       summoned->level    = 40;    
       summoned->max_hit  = dice( 8, 40 );
       summoned->hit      = summoned->max_hit;
       summoned->max_move = dice( 10, 40 );
       summoned->move     = summoned->max_move;  

     */
    SET_BIT(summoned->act, ACT_PET);
    SET_BIT(summoned->affected_by, AFF_CHARM);
    summoned->extract_timer = get_pseudo_level(ch) / 3;
    add_follower(summoned, ch);
    return TRUE;
}

bool
spell_summon_beast(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *summoned;

    if (IS_NPC(ch))
        return FALSE;

    if (!can_summon_charmie(ch))
        return FALSE;

    act("$n calls upon the elemental forces of @@lAir@@N!", ch, obj, NULL, TO_ROOM);
    act("You call upon the elemental forces of @@'Air@@N.", ch, obj, NULL, TO_CHAR);

    act("@@NA whirlwind appears, and an @@lElemental @@aBeast@@N steps from the winds!!", ch, obj, NULL, TO_ROOM);
    act("@@NA whirlwind appears, and an @@lElemental @@aBeast@@N steps from the winds!!", ch, obj, NULL, TO_CHAR);

    summoned = create_mobile(get_mob_index(MOB_VNUM_ELEM_BEAST));
    char_to_room(summoned, ch->in_room);
    act("$n steps from the whirlwind, drawing all the power into its body.", summoned, NULL, NULL, TO_ROOM);

    /* don't think we need these  
       summoned->level    = 40;    
       summoned->max_hit  = dice( 8, 40 );
       summoned->hit      = summoned->max_hit;
       summoned->max_move = dice( 10, 40 );
       summoned->move     = summoned->max_move;  

     */
    SET_BIT(summoned->act, ACT_PET);
    SET_BIT(summoned->affected_by, AFF_CHARM);
    summoned->extract_timer = get_pseudo_level(ch) / 3;
    add_follower(summoned, ch);
    return TRUE;
}

bool
spell_summon_devourer(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *summoned;

    if (IS_NPC(ch))
        return FALSE;

    if (!can_summon_charmie(ch))
        return FALSE;

    act("$n calls upon the psychic forces of the @@rMind@@N!", ch, obj, NULL, TO_ROOM);
    act("You call upon the psychic forces of the @@rMind@@N!", ch, obj, NULL, TO_CHAR);

    act("@@NA vortex appears, and an @@rIntellect @@bDevourer@@N steps from the circle!!", ch, obj, NULL, TO_ROOM);
    act("@@NA vortex appears, and an @@rIntellect @@bDevourer@@N steps from the circle!!", ch, obj, NULL, TO_CHAR);

    summoned = create_mobile(get_mob_index(MOB_VNUM_INT_DEVOURER));
    char_to_room(summoned, ch->in_room);
    act("$n steps from the vortex, drawing all the power into its body.", summoned, NULL, NULL, TO_ROOM);

    /* don't think we need these  
       summoned->level    = 40;    
       summoned->max_hit  = dice( 8, 40 );
       summoned->hit      = summoned->max_hit;
       summoned->max_move = dice( 10, 40 );
       summoned->move     = summoned->max_move;  

     */
    SET_BIT(summoned->act, ACT_PET);
    SET_BIT(summoned->affected_by, AFF_CHARM);
    summoned->extract_timer = get_pseudo_level(ch) / 3;
    add_follower(summoned, ch);
    return TRUE;
}

bool
spell_summon_shadow(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *summoned;

    if (IS_NPC(ch))
        return FALSE;

    if (!can_summon_charmie(ch))
        return FALSE;

    act("$n calls upon the shadowy forces of @@dDarkness@@N!", ch, obj, NULL, TO_ROOM);
    act("You call upon the shadowy forces of @@dDarkness@@N!", ch, obj, NULL, TO_CHAR);

    act("@@NA tunnel of shadow appears, and a @@dShadow @@RHound@@N steps from the depths!!", ch, obj, NULL, TO_ROOM);
    act("@@NA tunnel of shadow appears, and a @@dShadow @@RHound@@N steps from the depths!!", ch, obj, NULL, TO_CHAR);

    summoned = create_mobile(get_mob_index(MOB_VNUM_SHADOW_HOUND));
    char_to_room(summoned, ch->in_room);
    act("$n steps from the tunnel, drawing all the power into its body.", summoned, NULL, NULL, TO_ROOM);

    /* don't think we need these  
       summoned->level    = 40;    
       summoned->max_hit  = dice( 8, 40 );
       summoned->hit      = summoned->max_hit;
       summoned->max_move = dice( 10, 40 );
       summoned->move     = summoned->max_move;  

     */
    SET_BIT(summoned->act, ACT_PET);
    SET_BIT(summoned->affected_by, AFF_CHARM);
    summoned->extract_timer = get_pseudo_level(ch) / 3;
    add_follower(summoned, ch);
    return TRUE;
}

bool
spell_lava_burst(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    OBJ_DATA           *heated_item = NULL;
    OBJ_DATA           *burnobj;
    DUEL_PLAYER_DATA   *player;
    DUEL_OBJ_DATA      *dobj;
    DUEL_DATA          *duel;
    int                 save_mod = 1;
    int                 cnt = 0;
    int                 total = 0;
    int                 tot = 0;
    int                 burncnt = 0;

    if (saves_spell(level, victim))
        save_mod = .75;

    if (is_safe(ch, victim, TRUE))
        return FALSE;

    if (victim->race != RACE_CEN) {
        for (heated_item = victim->first_carry; heated_item != NULL; heated_item = heated_item->next_in_carry_list) {
            if (heated_item->wear_loc == WEAR_NONE)
                continue;
            else {
                if (   IS_SET(heated_item->item_apply, ITEM_APPLY_HEATED)
                    || (!IS_NPC(victim) && victim->pcdata->in_arena && IS_SET(heated_item->item_apply, ITEM_APPLY_ARENAHEATED)))
                    burncnt++;

                total++;
            }
        }

        if ((duel = find_duel(victim))
            && (player = find_duel_player(victim))
            && duel->stage == DUEL_STAGE_GO) {
            for (dobj = player->first_obj; dobj != NULL; dobj = dobj->next)
                if (dobj->obj && dobj->obj->wear_loc != WEAR_NONE)
                    burncnt++;
        }

        tot = number_range(1, total);

        for (heated_item = victim->first_carry; heated_item != NULL; heated_item = heated_item->next_in_carry_list) {
            if (heated_item->wear_loc == WEAR_NONE)
                continue;
            else if (++cnt == tot)
                break;
        }
    }

    burnobj = (heated_item) ? heated_item : NULL;

    if (burnobj != NULL && !IS_SET(burnobj->extra_flags, ITEM_NODESTROY)
        && number_percent() < number_range(70, 80)
        && (burncnt < number_range(3, 4) || burncnt < number_range(3, 4))
        ) {
        if (IS_NPC(victim) || !is_in_duel(victim, DUEL_STAGE_GO)) {
            if (IS_NPC(victim) || !victim->pcdata->in_arena)
                SET_BIT(burnobj->item_apply, ITEM_APPLY_HEATED);
            else
                SET_BIT(burnobj->item_apply, ITEM_APPLY_ARENAHEATED);
        }
        else {
            /* duel burning */
            if ((player = find_duel_player(victim)) == NULL)
                return FALSE;

            for (dobj = player->first_obj; dobj != NULL; dobj = dobj->next) {
                if (dobj->obj == burnobj)
                    break;
            }

            if (dobj == NULL) {
                /* object isn't already 'burning' */
                CREATE_MEMBER(DUEL_OBJ_DATA, dobj);

                if (!dobj)
                    return FALSE;

                dobj->obj = burnobj;
                DLINK(dobj, player->first_obj, player->last_obj, next, prev);
            }
        }

        return TRUE;
    }

    damage(ch, (CHAR_DATA *) vo, number_range(get_pseudo_level(ch) * 4, get_pseudo_level(ch) * 8) * save_mod, sn);
    return TRUE;
}

bool
spell_heat_armor(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    return spell_lava_burst(sn, level, ch, vo, obj);
}

bool
spell_retri_strike(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *vch;
    CHAR_DATA          *vch_next;
    OBJ_DATA           *staff_obj = NULL;

    if ((staff_obj = get_eq_char(ch, WEAR_HOLD)) == NULL) {
        send_to_char("You must be holding a @@rstaff@@N for this spell!\n\r", ch);
        return FALSE;
    }

    if (staff_obj->item_type != ITEM_STAFF) {
        send_to_char("That is not a staff you are holding, you fool!!\n\r", ch);
        return FALSE;
    }

    if (obj == NULL) {
        send_to_char("The earth trembles beneath your feet!\n\r", ch);
        act("$n makes the earth tremble and shiver.", ch, NULL, NULL, TO_ROOM);
    }
    else {
        act("$p vibrates violently, making the earth tremble!", ch, obj, NULL, TO_CHAR);
        act("$p vibrates violently, making the earth around $n tremble!", ch, obj, NULL, TO_ROOM);
    }

    CREF(vch_next, CHAR_NEXT);

    for (vch = first_char; vch != NULL; vch = vch_next) {
        vch_next = vch->next;
        if (vch->in_room == NULL)
            continue;

        /* ninjafix -dave */
        if (!IS_NPC(vch)
            && (IS_IMMORTAL(vch)
                || vch->stance == STANCE_AMBUSH || vch->stance == STANCE_AC_BEST)
            )
            continue;

        if (vch->level < 60)
            continue;

        if (vch->in_room == ch->in_room) {
            if (is_safe(ch, vch, FALSE))
                continue;

            if (vch != ch) {
                act("@@W$n @@Wis @@eimmolated@@N by the fury released from the $p@@W, and falls to the ground!", vch, staff_obj, NULL, TO_ROOM);
                act("@@WYou are @@eimmolated@@N by the fury released from the $p@@W, and fall to the ground!", vch, staff_obj, NULL, TO_CHAR);
                damage(ch, vch, number_range(staff_obj->level * 6, staff_obj->level * 10), sn);

                {
                    OBJ_DATA           *heated_item = NULL;
                    OBJ_DATA           *prev_carried = NULL;

                    for (heated_item = vch->first_carry; heated_item != NULL; heated_item = heated_item->next_in_carry_list) {
                        if (heated_item->wear_loc == WEAR_NONE)
                            continue;
                        prev_carried = heated_item;
                        if (number_range(0, 100) < 25)
                            continue;
                        else
                            break;
                    }
                }
            }
            else {
                act("@@W$n @@Wis protected by the fury of the @@W$p@@W.", vch, staff_obj, NULL, TO_ROOM);
            }
            continue;
        }
        if (vch->in_room->area == ch->in_room->area)
            send_to_char("@@WSuddenly, a @@ybright flash@@W sears your eyes, then is gone.@@N\n\r", vch);
    }

    CUREF(vch_next);
    extract_obj(staff_obj);
    return TRUE;
}

void
do_stance(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_STRING_LENGTH];
    bool                legal_stance = FALSE;
    sh_int              i;

    if (IS_NPC(ch)) {
        send_to_char("Not for Npcs!\n\r", ch);
        return;
    }

    one_argument(argument, arg);

    if (arg[0] == '\0') {
        char                cat_buf[MSL];
        char                msg_buf[MSL];

        sprintf(msg_buf, "\n\r%s\n\r", "Fighting Stances available to you:\n\r");

        for (i = 0; i < MAX_STANCE; i++) {
            switch (i) {
                case STANCE_WARRIOR:
                    sprintf(cat_buf, "%s\n\r", stance_app[i].name);
                    break;
                case STANCE_CASTER:
                    if ((ch->lvl2[0] > 10)    /* sorc */
                        ||(ch->lvl2[3] > 10))    /* necro */
                        sprintf(cat_buf, "%s\n\r", stance_app[i].name);
                    break;
                case STANCE_AMBUSH:
                    if (ch->lvl2[1] > 30 || (ch->pcdata->avatar && ch->pcdata->learned[gsn_stealth] >= 9))    /* assassin */
                        sprintf(cat_buf, "%s\n\r", stance_app[i].name);
                    break;
                case STANCE_AC_BEST:
                    if ((ch->lvl2[2] > 65)    /* knight */
                        ||(ch->lvl2[4] > 30))    /* monk */
                        sprintf(cat_buf, "%s\n\r", stance_app[i].name);
                    break;
                case STANCE_HR_BEST:
                    if ((ch->lvl2[2] > 45)    /* knight */
                        ||(ch->lvl2[4] > 20))    /* monk */
                        sprintf(cat_buf, "%s\n\r", stance_app[i].name);
                    break;
                case STANCE_DR_BEST:
                    if ((ch->lvl2[2] > 35)    /* knight */
                        ||(ch->lvl2[4] > 10))    /* monk */
                        sprintf(cat_buf, "%s\n\r", stance_app[i].name);
                    break;
                case STANCE_AC_WORST:
                    if (ch->lvl2[4] > 45)    /* monk */
                        sprintf(cat_buf, "%s\n\r", stance_app[i].name);
                    break;
                case STANCE_HR_WORST:
                    if (ch->lvl2[4] > 60)    /* monk */
                        sprintf(cat_buf, "%s\n\r", stance_app[i].name);
                    break;
                case STANCE_DR_WORST:
                    if (ch->lvl2[4] > 70)    /* monk */
                        sprintf(cat_buf, "%s\n\r", stance_app[i].name);
                    break;
                case STANCE_SUPER_FIGHTER:
                    if ((ch->lvl2[4] > 79)
                        && (ch->lvl2[2] > 79))    /* both knight and monk */
                        sprintf(cat_buf, "%s\n\r", stance_app[i].name);
                    break;
                case STANCE_SUPER_SPEED:
                    if ((ch->lvl2[4] > 70)
                        && (ch->lvl2[2] > 70))    /* both knight and monk */
                        sprintf(cat_buf, "%s\n\r", stance_app[i].name);
                    break;
            }
            safe_strcat(MSL, msg_buf, cat_buf);
            sprintf(cat_buf, "%s", "");
        }
        sprintf(cat_buf, "%s",
            "Type stance <stance name> to change your current fighting stance.\n\r You may place your current Stance in your prompt with a %%s\n\r");
        safe_strcat(MSL, msg_buf, cat_buf);
        send_to_char(msg_buf, ch);
        return;
    }

    for (i = 0; i < MAX_STANCE; i++)
        if (!str_prefix(arg, stance_app[i].name))
            break;
    if (i == MAX_STANCE) {
        do_stance(ch, "");
        act("$n poses in a strange fashion, looking rather silly.", ch, NULL, NULL, TO_ROOM);
        send_to_char("You twist about wildly, but are unable to figure out just the right Stance.\n\r", ch);
        return;
    }

    else {
        switch (i) {
            case STANCE_WARRIOR:
            {
                legal_stance = TRUE;
                break;
            }
            case STANCE_CASTER:
                if ((ch->lvl2[0] > 10)    /* sorc */
                    ||(ch->lvl2[3] > 10)) {    /* necro */
                    legal_stance = TRUE;
                    break;
                }
                break;
            case STANCE_AMBUSH:
                if (ch->lvl2[1] > 30 || (ch->pcdata->avatar && ch->pcdata->learned[gsn_stealth] >= 9)) {    /* assassin */
                    CHAR_DATA          *other;

                    for (other = ch->in_room->first_person; other != NULL; other = other->next_in_room)
                        if (other != ch && !IS_IMMORTAL(other) && can_see(ch, other))
                            break;

                    if (other != NULL) {
                        send_to_char("You can't set an ambush with people watching you!\n\r", ch);
                    }
                    else {
                        legal_stance = TRUE;
                    }
                    break;
                }
                break;
            case STANCE_AC_BEST:
                if ((ch->lvl2[2] > 65)    /* knight */
                    ||(ch->lvl2[4] > 30)) {    /* monk */
                    legal_stance = TRUE;
                    break;
                }
                break;

            case STANCE_HR_BEST:
                if ((ch->lvl2[2] > 45)    /* knight */
                    ||(ch->lvl2[4] > 20)) {    /* monk */
                    legal_stance = TRUE;
                    break;
                }
                break;
            case STANCE_DR_BEST:
                if ((ch->lvl2[2] > 35)    /* knight */
                    ||(ch->lvl2[4] > 10)) {    /* monk */
                    legal_stance = TRUE;
                    break;
                }
                break;
            case STANCE_AC_WORST:
                if (ch->lvl2[4] > 45) {    /* monk */
                    legal_stance = TRUE;
                    break;
                }
                break;

            case STANCE_HR_WORST:
                if (ch->lvl2[4] > 60) {    /* monk */
                    legal_stance = TRUE;
                    break;
                }
                break;
            case STANCE_DR_WORST:
                if (ch->lvl2[4] > 70) {    /* monk */
                    legal_stance = TRUE;
                    break;
                }
                break;
            case STANCE_SUPER_FIGHTER:
                if ((ch->lvl2[4] > 79)
                    && (ch->lvl2[2] > 79)) {    /* both knight and monk */
                    legal_stance = TRUE;
                    break;
                }
                break;
            case STANCE_SUPER_SPEED:
                if ((ch->lvl2[4] > 70)
                    && (ch->lvl2[2] > 70)) {    /* both knight and monk */
                    legal_stance = TRUE;
                    break;
                }
                break;
        }
        if (legal_stance && !IS_NPC(ch) && i == STANCE_AMBUSH && ch->pcdata->stealth > 0 && ch->energy < 20) {
            send_to_char("You must have at least 20 energy to begin stealth.\n\r", ch);
            return;
        }

        if (legal_stance && !IS_NPC(ch) && i == STANCE_AMBUSH && ch->pcdata->stealth > 0 && ch->energy >= 20) {
            char *type = NULL;
            int sn = ch->pcdata->stealth;

            if      (sn == gsn_stealth_novice)       type = "novice";
            else if (sn == gsn_stealth_intermediate) type = "intermediate";
            else if (sn == gsn_stealth_advanced)     type = "advanced";
            else if (sn == gsn_stealth_expert)       type = "expert";
            else if (sn == gsn_stealth_master)       type = "master";
            sendf(ch, "Using %s stealth.\n\r", type);
        }

        if (legal_stance) {
            char                stance_buf[MSL];

            if (stance_app[i].ac_mod > 0)
                ch->stance_ac_mod = (stance_app[i].ac_mod * (20 - get_pseudo_level(ch) / 12));
            else
                ch->stance_ac_mod = stance_app[i].ac_mod * get_pseudo_level(ch) / 12;
            if (stance_app[i].dr_mod < 0)
                ch->stance_dr_mod = (stance_app[i].dr_mod * (20 - get_pseudo_level(ch) / 12));
            else
                ch->stance_dr_mod = stance_app[i].dr_mod * get_pseudo_level(ch) / 10;

            if (stance_app[i].hr_mod < 0)
                ch->stance_hr_mod = (stance_app[i].hr_mod * (20 - get_pseudo_level(ch) / 12));
            else
                ch->stance_hr_mod = stance_app[i].hr_mod * get_pseudo_level(ch) / 10;
            ch->stance = i;
            sprintf(stance_buf, "$n assumes the Stance of the %s!", stance_app[i].name);
            act(stance_buf, ch, NULL, NULL, TO_ROOM);
            sprintf(stance_buf, "You assume the Stance of the %s!\n\r", stance_app[i].name);
            send_to_char(stance_buf, ch);
        }

        if (ch->desc && ch->desc->connected == CON_PLAYING)
            WAIT_STATE(ch, 2 * PULSE_VIOLENCE);

        return;
    }
}
bool
spell_creature_bond(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    CHAR_DATA          *master;
    bool                compassion = FALSE;
    int                 perc = 0;

    if (IS_NPC(ch)) {
        send_to_char("Not for Npcs.\n\r", ch);
        return FALSE;
    }
    if ((!IS_NPC(victim))
        || (!IS_SET(victim->affected_by, AFF_CHARM))) {
        send_to_char("That is not a charmed creature!\n\r", ch);
        return FALSE;
    }

    master = victim->master ? victim->master : victim;
    if ((master != NULL)
        && (get_pseudo_level(ch) < (get_pseudo_level(master) - 20))) {
        send_to_char("The current bond is too strong for you to overcome.\n\r", ch);
        return FALSE;
    }

    /* if the person is trying to bond their linkdead alt's charmies, deny them */
    if (!IS_NPC(ch) && ch->pcdata && !IS_NPC(master) && master->pcdata && master->desc == NULL && !str_cmp(master->pcdata->host, ch->pcdata->host)) {
        send_to_char("The current bond is too strong for you to overcome.\n\r", ch);
        return FALSE;
    }

    if (!IS_NPC(ch) && ch->pcdata && !IS_NPC(master) && master->pcdata && !can_group(ch, master) && IS_SET(master->config, PLR_NOBOND)) {
        send_to_char("The current bond is too strong for you to overcome.\n\r", ch);
        return FALSE;
    }

    if (!IS_NPC(master) && master->pcdata->learned[gsn_compassion] >= 9 && ch != master) {
        if (master->pcdata->learned[gsn_compassion_master] > 0)
            perc = 75;
        else if (master->pcdata->learned[gsn_compassion_expert] > 0)
            perc = 60;
        else if (master->pcdata->learned[gsn_compassion_advanced] > 0)
            perc = 50;
        else if (master->pcdata->learned[gsn_compassion_intermediate] > 0)
            perc = 40;
        else if (master->pcdata->learned[gsn_compassion_novice] > 0)
            perc = 30;

        if (number_percent() < perc)
            compassion = TRUE;
        else
            compassion = FALSE;
    }

    if (!IS_NPC(ch)) {
        sh_int              charmies;

        charmies = max_orders(ch);

        /* they're rebonding one of their own, so let them, even if they're full! */
        if (ch == master)
            charmies++;

        if (charmies <= ch->num_followers) {
            send_to_char("You have already charmed too many creatures!\n\r", ch);
            return FALSE;
        }
    }

    if (!compassion && number_range(0, 105) < (level + (get_pseudo_level(ch) - get_pseudo_level(master)))) {
        if (saves_spell(level, victim)) {
            do_say(victim, "How dare you!, I LOVE my master!");
            multi_hit(victim, ch, TYPE_UNDEFINED);
            return TRUE;
        }
        stop_follower(victim);
        SET_BIT(victim->act, ACT_PET);
        SET_BIT(victim->affected_by, AFF_CHARM);
        victim->extract_timer = get_pseudo_level(ch) / 3;
        add_follower(victim, ch);

        /* eligibility checks are done in spell_emount, so its safe to call it
         * for everyone */
        spell_emount(gsn_emount, 79, ch, NULL, NULL);
    }
    else {
        do_say(victim, "How dare you!, I LOVE my master!");
        multi_hit(victim, ch, TYPE_UNDEFINED);
        return TRUE;
    }
    return TRUE;
}

bool
spell_corrupt_bond(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    CHAR_DATA          *master = NULL;
    bool                compassion = FALSE;
    int                 perc = 0;

    if (IS_NPC(ch)) {
        send_to_char("Not for Npcs.\n\r", ch);
        return FALSE;
    }
    if ((!IS_NPC(victim))
        || (!IS_SET(victim->affected_by, AFF_CHARM))) {
        send_to_char("That is not a charmed creature!\n\r", ch);
        return FALSE;
    }

    if (IS_NGR_CHARMIE(ch, victim)) {
        send_to_char("The bond is too strong from you to corrupt.\n\r", ch);
        return FALSE;
    }

    master = victim->master ? victim->master : victim;
    if ((master != NULL)
        && (get_pseudo_level(ch) < (get_pseudo_level(master) - 20))) {
        send_to_char("The current bond is too strong for you to corrupt.\n\r", ch);
        return FALSE;
    }

    if (!IS_NPC(master) && master->pcdata->learned[gsn_compassion] >= 9 && ch != master) {
        if (master->pcdata->learned[gsn_compassion_master] > 0)
            perc = 75;
        else if (master->pcdata->learned[gsn_compassion_expert] > 0)
            perc = 60;
        else if (master->pcdata->learned[gsn_compassion_advanced] > 0)
            perc = 50;
        else if (master->pcdata->learned[gsn_compassion_intermediate] > 0)
            perc = 40;
        else if (master->pcdata->learned[gsn_compassion_novice] > 0)
            perc = 30;

        if (number_percent() < perc)
            compassion = TRUE;
        else
            compassion = FALSE;
    }

    if (!compassion && number_range(0, 105) < (level + (get_pseudo_level(ch) - get_pseudo_level(master)))) {
        if (saves_spell(level, victim)) {
            do_say(victim, "How dare you!, I LOVE my master!");
            multi_hit(victim, ch, TYPE_UNDEFINED);
            return TRUE;
        }

        stop_follower(victim);
        if (victim->in_room == master->in_room) {
            do_say(victim, "Now I shall have my revenge for being charmed!!!");
            multi_hit(victim, master, TYPE_UNDEFINED);
            return TRUE;
        }
        else {
            do_say(victim, "AARRGH!  I HATE being charmed! Now I shall have my revenge!");
            victim->hunt_flags = HUNT_WORLD | HUNT_OPENDOOR | HUNT_PICKDOOR | HUNT_UNLOCKDOOR | HUNT_INFORM;
            set_hunt(victim, NULL, master, NULL, HUNT_WORLD | HUNT_OPENDOOR | HUNT_PICKDOOR | HUNT_UNLOCKDOOR | HUNT_INFORM, HUNT_MERC | HUNT_CR);
            return TRUE;
        }
    }
    else {
        do_say(victim, "How dare you!, I LOVE my master!");
        multi_hit(victim, ch, TYPE_UNDEFINED);
        return TRUE;
    }
    return TRUE;
}

bool
spell_group_heal(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *gch;
    int                 heal;

    heal = UMAX(200, number_range(level * 3, level * 7));
    for (gch = ch->in_room->first_person; gch != NULL; gch = gch->next_in_room) {
        if (is_affected(gch, sn) || !is_same_group(ch, gch))
            continue;
        act("$n's wounds close up before your eyes.", gch, NULL, NULL, TO_ROOM);
        send_to_char("Your wounds close up and heal!\n\r", gch);
        gch->hit = UMIN(gch->hit + heal, gch->max_hit);
        update_pos(gch);
    }
    send_to_char("You blanket your group in a healing aura!\n\r", ch);
    return TRUE;
}

        /* Spell by us */
bool
spell_carapace(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{

    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (is_affected(victim, sn))
        return FALSE;

    af.type = sn;
    af.duration = 6 + (level / 20);
    af.location = APPLY_HITROLL;
    af.modifier = get_pseudo_level(ch) / 4;
    af.bitvector = 0;
    af.save = TRUE;
    affect_to_char(victim, &af);
    af.location = APPLY_SAVING_SPELL;
    af.modifier = 0 - level / 6;
    affect_to_char(victim, &af);
    af.location = APPLY_DAMROLL;
    af.modifier = get_pseudo_level(ch) / 4;
    affect_to_char(victim, &af);
    af.location = APPLY_AC;
    af.modifier = -get_pseudo_level(ch);
    affect_to_char(victim, &af);
    send_to_char("@@WA skeletal beast rises from the ground, encasing your armour.@@N\n\r", victim);
    act("@@g$n@@g is encased by a skeletal beast.@@N", victim, NULL, NULL, TO_ROOM);
    if (ch != victim)
        send_to_char("Ok.\n\r", ch);
    return TRUE;
}

/*Borlein */
bool
spell_summon_shadowdragon(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *summoned;

    if (IS_NPC(ch))
        return FALSE;

    if (!can_summon_charmie(ch))
        return FALSE;

    act("$n kneels down and draws a @@dpentagram@@N in the @@bsand@@N.", ch, obj, NULL, TO_ROOM);
    act("You kneel down drawing a @@dpentagram@@N in the @@bsand@@N.", ch, obj, NULL, TO_CHAR);
    act("@@RUnholy light bursts forth from the ground@@g and a @@dGreat ShadowDragon@@g emerges from the @@Runderworld@@N!!", ch, obj, NULL, TO_ROOM);
    act("@@RUnholy light bursts forth from the ground@@g and a @@dGreat ShadowDragon@@g emerges from the @@Runderworld@@N!!", ch, obj, NULL, TO_CHAR);
    summoned = create_mobile(get_mob_index(MOB_VNUM_SHADOWDRAGON));
    char_to_room(summoned, ch->in_room);
    act("$n @@gshrieks in @@Ranger@@g at being summoned from the @@dabyss@@g, acknowledges its master and pays homage by bowing.", summoned, NULL,
        NULL, TO_ROOM);
    SET_BIT(summoned->act, ACT_PET);
    SET_BIT(summoned->affected_by, AFF_CHARM);
    summoned->extract_timer = get_pseudo_level(ch) / 3;
    add_follower(summoned, ch);
    return TRUE;
}

/* following the two previous spells, i'll say *Dave* :P */
bool
spell_lsd(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    AFFECT_DATA         af;

    if (is_affected(ch, sn))
        return FALSE;
    if (IS_NPC(ch))
        return FALSE;
    af.type = sn;
    af.duration = 10;
    af.location = APPLY_AC;
    af.modifier = -20;
    af.bitvector = 0;
    af.save = TRUE;
    affect_to_char(ch, &af);
    send_to_char("@@eYou blink, and suddenly the world changes before your eyes!@@N\n\r", ch);
    return TRUE;
}

bool
spell_hellfire(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *vch;
    CHAR_DATA          *vch_next;

    if (obj == NULL) {
        send_to_char("Hellfire falls from the sky.\n\r", ch);
        act("$n unleashes the power of hellfire.", ch, NULL, NULL, TO_ROOM);
    }
    else {
        act("$p gestures and makes the sky go black.", ch, obj, NULL, TO_CHAR);
        act("$p gestures, making the sky go black around you.", ch, obj, NULL, TO_ROOM);
    }
    CREF(vch_next, CHAR_NEXT);
    for (vch = first_char; vch != NULL; vch = vch_next) {
        vch_next = vch->next;
        if (vch->in_room == NULL)
            continue;

        /* ninjafix -dave */
        if (!IS_NPC(vch)
            && (IS_IMMORTAL(vch)
                || vch->stance == STANCE_AMBUSH || vch->stance == STANCE_AC_BEST)
            )
            continue;

        if (vch->in_room == ch->in_room) {
            if (vch != ch && (IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch))
                && vch->master != ch
                && !is_same_group(ch, vch)
                && !is_safe(ch, vch, FALSE))
            {
                act("$n loses $s footing, and crunches down to avoid the hellfire.", vch, NULL, NULL, TO_ROOM);
                send_to_char("You lose your footing trying to avoid the hellfire.\n\r", vch);
                damage(ch, vch, get_pseudo_level(ch) * 3, -1);
            }
            else {
                act("$n keeps $s footing, and stays where $e is.", vch, NULL, NULL, TO_ROOM);
                send_to_char("You keep your footing.\n\r", vch);
            }
            continue;
        }

        if (vch->in_room->area == ch->in_room->area)
            send_to_char("The sky turns dark and rains Hellfire.\n\r", vch);
    }
    CUREF(vch_next);
    return TRUE;
}

bool
spell_warning_rune(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    ROOM_INDEX_DATA    *room;
    ROOM_AFFECT_DATA   *raf;
    ROOM_AFFECT_DATA    rafadd;

    if (IS_NPC(ch))
        return FALSE;
    room = ch->in_room;

    if (room == NULL)
        return FALSE;

    for (raf = room->first_room_affect; raf != NULL; raf = raf->next) {
        if (raf->bitvector == ROOM_BV_WARNING_RUNE && raf->caster == ch) {
            send_to_char("You already have a warning rune operating in this room.\n\r", ch);
            return FALSE;
        }
    }

    if (ch->pcdata->runes >= 10) {
        send_to_char("You already have the maximum amount of runes.\n\r", ch);
        return FALSE;
    }

    if (*target_name == '\0') {
        send_to_char("You must give your warning rune a name.\n\r", ch);
        return FALSE;
    }

    act("$n spreads $s hands magestically and a hovering rune appears!", ch, NULL, NULL, TO_ROOM);
    send_to_char("You spread your hands magestically and a hovering rune appears!\n\r", ch);

    rafadd.type = sn;
    rafadd.level = level;
    rafadd.duration = 30;
    rafadd.bitvector = ROOM_BV_WARNING_RUNE;
    rafadd.caster = ch;
    rafadd.name = str_dup(target_name);
    affect_to_room(room, &rafadd);
    return TRUE;
}

bool
spell_emount(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    AFFECT_DATA         af1;

    if (is_affected(ch, sn) || IS_NPC(ch)) {
        return FALSE;
    }
    if (ch->lvl2[2] >= 39 && (ch->pcdata->learned[sn] > 10) && (ch->riding != NULL)
        && (ch->in_room == ch->riding->in_room)) {

        af1.type = sn;
        af1.duration = -1;

        af1.location = APPLY_DAMROLL;
        af1.modifier = get_pseudo_level(ch) / 4;
        af1.bitvector = 0;
        af1.save = FALSE;
        affect_to_char(ch, &af1);

        af1.location = APPLY_HITROLL;
        af1.modifier = get_pseudo_level(ch) / 4;
        af1.bitvector = 0;
        af1.save = FALSE;
        affect_to_char(ch, &af1);

        af1.location = APPLY_AC;
        af1.modifier = -get_pseudo_level(ch);
        af1.bitvector = 0;
        af1.save = FALSE;
        affect_to_char(ch, &af1);

        return TRUE;
    }
    return FALSE;
}

bool
spell_gaze_mirror(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    send_to_char("You create a mirror and gaze into it.\n\r\n\r", ch);
    show_char_to_char_0(ch, ch);

    return TRUE;
}
