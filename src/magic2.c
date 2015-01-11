
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

/* Original 'magic.c' now contains the first half of the file, while the
   second half is in this 'magic2.c' file.  -S- */

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

IDSTRING(rcsid, "$Id: magic2.c,v 1.22 2004/06/18 19:39:07 dave Exp $");

bool
spell_invis(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim;
    OBJ_DATA           *ob;
    AFFECT_DATA         af;

    if (target_name[0] == '\0') {
        send_to_char("Make who or what invisible?\n\r", ch);
        return FALSE;
    }

    if ((victim = get_char_room(ch, target_name)) != NULL) {
        if (IS_AFFECTED(victim, AFF_INVISIBLE) || item_has_apply(victim, ITEM_APPLY_INV))
            return (ch == victim ? FALSE : TRUE);

        act("$n fades out of existence.", victim, NULL, NULL, TO_ROOM);
        af.type = sn;
        af.duration = 4 + (level / 5);
        af.location = APPLY_NONE;
        af.modifier = 0;
        af.bitvector = AFF_INVISIBLE;
        af.save = TRUE;
        affect_to_char(victim, &af);
        send_to_char("You fade out of existence.\n\r", victim);
        return TRUE;
    }
    else if ((ob = get_obj_carry(ch, target_name)) != NULL) {
        if (!IS_SET(ob->extra_flags, ITEM_INVIS)) {
            SET_BIT(ob->extra_flags, ITEM_INVIS);
            act("$p shimmers out of visibility.", ch, ob, NULL, TO_CHAR);
            act("$p shimmers out of visibility.", ch, ob, NULL, TO_ROOM);
            return TRUE;
        }
        else {
            act("$p is already invisible!", ch, ob, NULL, TO_CHAR);
            return FALSE;
        }
    }
    else {
        send_to_char("Couldn't find that person or object.\n\r", ch);
        return FALSE;
    }
    return FALSE;
}

bool
spell_know_alignment(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    char               *msg;
    int                 ap;

    ap = victim->alignment;

    if (ap > 700)
        msg = "$N has an aura as white as the driven snow.";
    else if (ap > 350)
        msg = "$N is of excellent moral character.";
    else if (ap > 100)
        msg = "$N is often kind and thoughtful.";
    else if (ap > -100)
        msg = "$N doesn't have a firm moral commitment.";
    else if (ap > -350)
        msg = "$N lies to $S friends.";
    else if (ap > -700)
        msg = "$N's slash DISEMBOWELS you!";
    else
        msg = "I'd rather just not say anything at all about $N.";

    act(msg, ch, NULL, victim, TO_CHAR);
    return TRUE;
}

bool
spell_lightning_bolt(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
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
    dam = number_range(dam_each[level] * 2, dam_each[level] * 5);
    if (saves_spell(level, victim))
        dam /= 2;
    if (obj == NULL) {
        act("A bolt of lightning flies from $n's finger!", ch, NULL, NULL, TO_ROOM);
        send_to_char("A bolt of lightning flies from your finger!\n\r", ch);
    }
    else {
        act("A bolt of lightning flies from $p!", ch, obj, NULL, TO_ROOM);
        act("A bolt of lightning flies from $p!", ch, obj, NULL, TO_CHAR);
    }
    act("$n is struck by the lightning bolt!!", victim, NULL, NULL, TO_ROOM);
    send_to_char("You are struck by the lightning bolt!!\n\r", victim);
    damage(ch, victim, dam, -1);
    return TRUE;
}

/* support for combined locate object added -Erigol */
bool
spell_locate_object(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    char                buf[MAX_INPUT_LENGTH];
    OBJ_DATA           *ob;
    OBJ_DATA           *in_obj;
    bool                found;

    char              **strShow;
    int                *countShow;

    int                 nShow;
    int                 iShow;
    int                 count;
    bool                fCombine;

    found = FALSE;
    count = 0;

    send_to_char("@@N", ch);

    for (ob = first_obj; ob != NULL; ob = ob->next) {
        if (!can_see_obj(ch, ob) || !is_name(target_name, ob->name)
            || IS_SET(ob->extra_flags, ITEM_RARE)
            || (ob->item_type == ITEM_PIECE)
            || (IS_SET(ob->extra_flags, ITEM_UNIQUE))
            || (!str_prefix(target_name, "unique"))
            )
            continue;

        for (in_obj = ob; in_obj->in_obj != NULL; in_obj = in_obj->in_obj);

        if (in_obj->carried_by != NULL && IS_IMMORTAL(in_obj->carried_by))
            continue;

        if (in_obj->carried_by != NULL && !IS_NPC(in_obj->carried_by)
            && !can_see(ch, in_obj->carried_by)
            )
            continue;

        if (in_obj->carried_by == NULL && in_obj->in_room == NULL)
            continue;

        count++;
    }

    strShow = qgetmem(count * sizeof(char *));
    countShow = qgetmem(count * sizeof(int));
    nShow = 0;

    for (ob = first_obj; ob != NULL; ob = ob->next) {
        if (!can_see_obj(ch, ob) || !is_name(target_name, ob->name)
            || IS_SET(ob->extra_flags, ITEM_RARE)
            || (ob->item_type == ITEM_PIECE)
            || (IS_SET(ob->extra_flags, ITEM_UNIQUE))
            || (!str_prefix(target_name, "unique"))
            )
            continue;

        for (in_obj = ob; in_obj->in_obj != NULL; in_obj = in_obj->in_obj);

        if (in_obj->carried_by != NULL && IS_IMMORTAL(in_obj->carried_by))
            continue;

        if (in_obj->carried_by != NULL && !IS_NPC(in_obj->carried_by)
            && !can_see(ch, in_obj->carried_by)
            )
            continue;

        if (in_obj->carried_by == NULL && in_obj->in_room == NULL)
            continue;

        fCombine = FALSE;

        if (in_obj->carried_by != NULL) {
            found = TRUE;

            sprintf(buf, "%s carried by %s.@@N\n\r", ob->short_descr, can_see(ch, in_obj->carried_by) ? PERS(in_obj->carried_by, ch) : "someone");
        }
        else {
            found = TRUE;
            sprintf(buf, "%s in %s.\n\r", ob->short_descr, in_obj->in_room->name);
        }

        if (IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE)) {
            for (iShow = nShow - 1; iShow >= 0; iShow--) {
                if (!strcmp(buf, strShow[iShow])) {
                    countShow[iShow]++;
                    fCombine = TRUE;
                    break;
                }
            }
        }

        if (!fCombine) {
            strShow[nShow] = str_dup(buf);
            countShow[nShow] = 1;
            nShow++;
        }
    }

    for (iShow = 0; iShow < nShow; iShow++) {
        if (IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE)) {
            if (countShow[iShow] > 1) {
                sprintf(buf, "(%3d) ", countShow[iShow]);
                send_to_char(buf, ch);
            }
            else {
                send_to_char("      ", ch);
            }
        }

        send_to_char(strShow[iShow], ch);

        free_string(strShow[iShow]);
    }

    if (!found)
        send_to_char("Nothing like that in hell, earth, or heaven.\n\r", ch);

    qdispose(strShow);
    qdispose(countShow);

    return TRUE;
}

bool
spell_magic_missile(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    int                 cnt;
    int                 hits;
    static const sh_int dam_each[] = {
        0,
        3, 3, 4, 4, 5, 6, 6, 6, 6, 6,
        7, 7, 7, 7, 7, 8, 8, 8, 8, 8,
        9, 9, 9, 9, 9, 10, 10, 10, 10, 10,
        11, 11, 11, 11, 11, 12, 12, 12, 12, 12,
        13, 13, 13, 13, 13, 14, 14, 14, 14, 14
    };
    int                 dam;

    level = UMIN(level, sizeof(dam_each) / sizeof(dam_each[0]) - 1);
    level = UMAX(0, level);
    dam = number_range(dam_each[level] * 2, dam_each[level] * 5);
    if (saves_spell(level, victim))
        dam /= 2;
    cnt = 1 + (level >= 30) + (level >= 60) + (level >= 80);
    for (hits = 0; hits < cnt; hits++) {
        if (obj == NULL) {
            act("$n fires a magical dart which hits $M!", ch, NULL, victim, TO_NOTVICT);
            act("$N fires a magical dart, which hits you!", victim, NULL, ch, TO_CHAR);
            act("You fire a magical dart, which hits $N!", ch, NULL, victim, TO_CHAR);
        }
        else {
            act("A magical dart flies from $p, hitting $n!", victim, obj, NULL, TO_ROOM);
            act("A magical dart flies from $p, hitting you!", victim, obj, NULL, TO_CHAR);
        }
    }
    damage(ch, victim, dam * cnt, -1);

    return TRUE;
}

bool
spell_mass_invis(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    AFFECT_DATA         af;
    CHAR_DATA          *gch;

    for (gch = ch->in_room->first_person; gch != NULL; gch = gch->next_in_room) {
        if (IS_AFFECTED(gch, AFF_INVISIBLE)
            || item_has_apply(gch, ITEM_APPLY_INV))
            continue;
        act("$n slowly fades out of existence.", gch, NULL, NULL, TO_ROOM);
        send_to_char("You slowly fade out of existence.\n\r", gch);
        af.type = sn;
        af.duration = 4 + (level / 5);
        af.location = APPLY_NONE;
        af.modifier = 0;
        af.bitvector = AFF_INVISIBLE;
        af.save = TRUE;
        affect_to_char(gch, &af);
    }
    send_to_char("Ok.\n\r", ch);

    return TRUE;
}

bool
spell_null(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    send_to_char("That's not a spell!\n\r", ch);
    return FALSE;
}

bool
spell_pass_door(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (IS_AFFECTED(victim, AFF_PASS_DOOR) || item_has_apply(victim, ITEM_APPLY_PASS_DOOR))
        return FALSE;
    af.type = sn;
    af.duration = 2 + (level / 20);
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_PASS_DOOR;
    af.save = TRUE;
    affect_to_char(victim, &af);
    act("$n turns translucent.", victim, NULL, NULL, TO_ROOM);
    send_to_char("You turn translucent.\n\r", victim);
    return TRUE;
}

bool
spell_poison(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (saves_spell(level, victim))
        return TRUE;

    if (victim->race == RACE_LAM)
        return TRUE;

    af.type = sn;
    af.duration = 12 + (level / 10);
    af.location = APPLY_STR;
    af.modifier = -2;
    af.bitvector = AFF_POISON;
    af.save = TRUE;
    affect_join(victim, &af);
    send_to_char("You feel very sick.\n\r", victim);
    act("$n looks very sick.", victim, NULL, NULL, TO_ROOM);
    return TRUE;
}

bool
spell_protection(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (IS_AFFECTED(victim, AFF_PROTECT) || item_has_apply(victim, ITEM_APPLY_PROT))
        return (ch == victim ? FALSE : TRUE);
    af.type = sn;
    af.duration = 8 + (level / 5);
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_PROTECT;
    af.save = TRUE;
    affect_to_char(victim, &af);
    send_to_char("You feel protected.\n\r", victim);
    act("$n looks better protected.", victim, NULL, NULL, TO_ROOM);
    return TRUE;
}

bool
spell_refresh(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;

    if (ch->carry_weight >= can_carry_w(ch)) {
        send_to_char("That's not going to help, you are carrying too much weight!\n\r", ch);
        return FALSE;
    }
    victim->move = UMIN(victim->move + level, victim->max_move);
    send_to_char("You feel less tired.\n\r", victim);
    act("$n looks less tired.", victim, NULL, NULL, TO_ROOM);
    return TRUE;
}

bool
spell_remove_curse(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;

    if (is_affected(victim, gsn_curse)) {
        affect_strip(victim, gsn_curse);
        send_to_char("You feel better.\n\r", victim);
        act("$n looks more Holy.", victim, NULL, NULL, TO_ROOM);
    }

    return TRUE;
}

bool
spell_sanctuary(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (IS_AFFECTED(victim, AFF_SANCTUARY) || item_has_apply(victim, ITEM_APPLY_SANC))
        return (ch == victim ? FALSE : TRUE);
    af.type = sn;
    af.duration = 5 + (level / 20);
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_SANCTUARY;
    af.save = TRUE;
    affect_to_char(victim, &af);
    act("$n is surrounded by a white aura.", victim, NULL, NULL, TO_ROOM);
    send_to_char("You are surrounded by a white aura.\n\r", victim);
    return TRUE;
}

bool
spell_sense_evil(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (IS_AFFECTED(victim, AFF_DETECT_EVIL))
        return FALSE;
    af.type = sn;
    af.duration = 5 + (level / 10);
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_DETECT_EVIL;
    af.save = TRUE;
    affect_to_char(victim, &af);
    send_to_char("Your eyes tingle.\n\r", victim);
    act("$n's eyes glow briefly.", victim, NULL, NULL, TO_ROOM);
    return TRUE;
}

bool
spell_shield(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (is_affected(victim, sn))
        return FALSE;
    af.type = sn;
    af.duration = 4 + (level / 5);
    af.location = APPLY_AC;
    af.modifier = -get_pseudo_level(ch) / 5;
    af.bitvector = 0;
    af.save = TRUE;
    affect_to_char(victim, &af);
    act("$n is surrounded by a force shield.", victim, NULL, NULL, TO_ROOM);
    send_to_char("You are surrounded by a force shield.\n\r", victim);
    return TRUE;
}

bool
spell_shocking_grasp(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    static const int    dam_each[] = {
        0,
        0, 0, 0, 0, 0, 0, 20, 25, 29, 33,
        36, 39, 39, 39, 40, 40, 41, 41, 42, 42,
        43, 43, 44, 44, 45, 45, 46, 46, 47, 47,
        48, 48, 49, 49, 50, 50, 51, 51, 52, 52,
        53, 53, 54, 54, 55, 55, 56, 56, 57, 57
    };
    int                 dam;

    level = UMIN(level, sizeof(dam_each) / sizeof(dam_each[0]) - 1);
    level = UMAX(0, level);
    dam = number_range(dam_each[level] / 2, dam_each[level] * 2);
    if (saves_spell(level, victim))
        dam /= 2;
    if (obj == NULL) {
        act("$n discharges a powerful electrical shock into $N!", ch, NULL, victim, TO_NOTVICT);
        act("$n discharges a powerful electrical shock into you!", ch, NULL, victim, TO_VICT);
        act("You discharge a powerful electrical shock into $N!", ch, NULL, victim, TO_CHAR);
    }
    else {
        act("$n receives a powerful electrical shock from $p!", victim, obj, NULL, TO_ROOM);
        act("You receive a powerful electrical shock from $p!", victim, obj, NULL, TO_ROOM);
    }
    damage(ch, victim, dam, -1);
    return TRUE;
}

bool
spell_sleep(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (IS_AFFECTED(victim, AFF_SLEEP)
        || level < victim->level || saves_spell(level, victim))
        return TRUE;

    /* no more adepts continually sleeping innocent newbies! :P */
    if (get_pseudo_level(ch) > (get_pseudo_level(victim) + 20))
        return TRUE;

    af.type = sn;
    af.duration = 4 + (level / 15);
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_SLEEP;
    af.save = TRUE;
    affect_join(victim, &af);

    if (IS_AWAKE(victim)) {
        send_to_char("You feel very sleepy ..... zzzzzz.\n\r", victim);

        /* will stop the char from attackin' right away (4-24-96)
           -Damane- */
        if (victim->position == POS_FIGHTING)
            stop_fighting(victim, TRUE);
        do_sleep(victim, "");
    }

    return TRUE;
}

bool
spell_stone_skin(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (is_affected(ch, sn) || is_affected(ch, skill_lookup("stone skin")))
        return FALSE;
    af.type = sn;
    af.duration = 5 + (level / 12);
    af.location = APPLY_AC;
    af.modifier = -get_pseudo_level(ch) / 3;
    af.bitvector = 0;
    af.save = TRUE;
    affect_to_char(victim, &af);
    act("$n's skin turns to stone.", victim, NULL, NULL, TO_ROOM);
    send_to_char("Your skin turns to stone.\n\r", victim);
    return TRUE;
}

bool
spell_summon(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim;

    if ((victim = get_char_world(ch, target_name)) == NULL
        || victim == ch || victim->in_room == NULL || IS_SET(victim->in_room->room_flags, ROOM_SAFE)
        || IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
        || !(IS_SET(victim->in_room->area->flags, AREA_TELEPORT))
        || victim->level >= level + 10 || victim->fighting != NULL || (IS_NPC(victim) && saves_spell(level, victim))
        || (IS_NPC(victim) && IS_SET(victim->act, ACT_AGGRESSIVE))
        || room_is_private(ch->in_room)
        || (ch->in_room->area->max_level < (victim->level - 15))) {
        send_to_char("You failed.\n\r", ch);
        return TRUE;
    }

    if (ch->in_room->vnum == 7699 || ch->in_room->vnum == 7698) {
        send_to_char("You failed.\n\r", ch);
        return TRUE;
    }

    if ((IS_SET(victim->act, PLR_NOSUMMON))
        || (IS_NPC(victim) && (victim->level > (level - 21)))) {
        send_to_char("You seemed unable to snatch your victim!\n\r", ch);
        send_to_char("You feel a slight tugging sensation.\n\r", victim);
        return TRUE;
    }

    act("$n disappears suddenly.", victim, NULL, NULL, TO_ROOM);
    char_from_room(victim);
    char_to_room(victim, ch->in_room);
    act("$n arrives suddenly.", victim, NULL, NULL, TO_ROOM);
    act("$n has summoned you!", ch, NULL, victim, TO_VICT);
    do_look(victim, "auto");
    return TRUE;
}

bool
spell_teleport(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    ROOM_INDEX_DATA    *pRoomIndex;

    if (victim->in_room == NULL || IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
        || IS_SET(victim->in_room->room_flags, ROOM_NO_TELEPORT)
        || (!IS_NPC(ch) && victim->fighting != NULL)
        || (victim != ch && (saves_spell(level, victim) || saves_spell(level, victim)))) {
        send_to_char("You failed.\n\r", ch);
        return TRUE;
    }

    for (;;) {
        pRoomIndex = get_room_index(number_range(0, 32767));
        if (pRoomIndex == NULL)
            continue;
        if (!IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)
            && !IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY)
            && !IS_SET(pRoomIndex->room_flags, ROOM_NO_TELEPORT)
            && IS_SET(pRoomIndex->area->flags, AREA_TELEPORT))
            break;
    }

    act("$n slowly fades out of existence.", victim, NULL, NULL, TO_ROOM);
    char_from_room(victim);
    char_to_room(victim, pRoomIndex);
    act("$n slowly fades into existence.", victim, NULL, NULL, TO_ROOM);
    do_look(victim, "auto");
    return TRUE;

}

bool
spell_ventriloquate(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    char                buf1[MAX_STRING_LENGTH];
    char                buf2[MAX_STRING_LENGTH];
    char                speaker[MAX_INPUT_LENGTH];
    CHAR_DATA          *vch;

    target_name = one_argument(target_name, speaker);

    sprintf(buf1, "%s says '%s'.\n\r", speaker, target_name);
    sprintf(buf2, "Someone makes %s say '%s'.\n\r", speaker, target_name);
    buf1[0] = UPPER(buf1[0]);

    for (vch = ch->in_room->first_person; vch != NULL; vch = vch->next_in_room) {
        if (!is_name(speaker, vch->name))
            send_to_char(saves_spell(level, vch) ? buf2 : buf1, vch);
    }

    return TRUE;
}

bool
spell_warcry(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)

/* --Stephen 
 * FIXME: make this a warrior skill, check for bless.
 */
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (victim->position == POS_FIGHTING || is_affected(victim, sn))
        return FALSE;
    af.type = sn;
    af.duration = 4 + level;
    af.location = APPLY_HITROLL;
    af.modifier = get_pseudo_level(ch) / 10;
    af.bitvector = 0;
    af.save = TRUE;
    affect_to_char(victim, &af);

    af.location = APPLY_SAVING_SPELL;
    af.modifier = 0 - get_pseudo_level(ch) / 10;
    affect_to_char(victim, &af);
    send_to_char("You feel righteous.\n\r", victim);
    if (ch != victim)
        send_to_char("Ok.\n\r", ch);
    return TRUE;
}

bool
spell_weaken(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (is_affected(victim, sn) || saves_spell(level, victim))
        return TRUE;
    af.type = sn;
    af.duration = 1 + (level / 16);
    af.location = APPLY_STR;
    af.modifier = -2;
    af.bitvector = 0;
    af.save = TRUE;
    affect_to_char(victim, &af);
    send_to_char("You feel weaker.\n\r", victim);
    if (ch != victim)
        send_to_char("Ok.\n\r", ch);
    return TRUE;
}

/*
 * For Ack! : Level 66 Priest spell - allow 100% recall.
 */
bool
spell_word_of_recall(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    ROOM_INDEX_DATA    *location;

    act("$n requests Holy transport!", victim, NULL, NULL, TO_ROOM);
    if (IS_AFFECTED(victim, AFF_CURSE)) {
        send_to_char("The Gods do not transport cursed players!\n\r", ch);
        return FALSE;
    }

    if (IS_NPC(victim) || victim->level < 6)
        location = get_room_index(ROOM_VNUM_TEMPLE);
    else
        location = get_room_index(race_table[victim->race].recall);

    if (location == NULL) {
        send_to_char("You are completely lost.\n\r", victim);
        return FALSE;
    }

    if (victim->in_room == location)
        return FALSE;

    if (IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)) {
        send_to_char("Some strange force prevents your transport.\n\r", victim);
        return TRUE;
    }

    if (IS_SET(victim->in_room->affected_by, ROOM_BV_HOLD)) {
        {
            bool                found = FALSE;
            ROOM_AFFECT_DATA   *raf;

            for (raf = victim->in_room->first_room_affect; raf != NULL; raf = raf->next)
                if (raf->bitvector == ROOM_BV_HOLD)
                    if ((IS_NPC(victim) && !IS_AFFECTED(victim, AFF_CHARM)) || (!IS_NPC(victim) && can_group_level(victim, raf->modifier)))
                        found = TRUE;

            if (found && !IS_IMMORTAL(victim) && number_range(1, 100) > 80) {
                send_to_char("Your attempts to travel through the bars was not successful, try again.\n\r", victim);
                return TRUE;
            }
        }

    }
    if (IS_SET(victim->in_room->affected_by, ROOM_BV_ENCAPS)) {
        {
            bool                found = FALSE;
            ROOM_AFFECT_DATA   *raf;

            for (raf = victim->in_room->first_room_affect; raf != NULL; raf = raf->next)
                if (raf->bitvector == ROOM_BV_ENCAPS)
                    if ((IS_NPC(victim) && !IS_AFFECTED(victim, AFF_CHARM)) || (!IS_NPC(victim) && can_group_level(victim, raf->modifier)))
                        found = TRUE;

            if (found && !IS_IMMORTAL(victim) && number_range(1, 100) > 80) {
                send_to_char("Your attempts to travel through the magical web was not successful, try again.\n\r", victim);
                return TRUE;
            }
        }
    }

    if (victim->fighting != NULL) {
        stop_fighting(victim->fighting, FALSE);
        stop_fighting(victim, FALSE);
    }

    act("$n is engulfed by a stream of blue light!", victim, NULL, NULL, TO_ROOM);
    char_from_room(victim);
    char_to_room(victim, location);
    act("$n arrives, carried by a blue stream of light!", victim, NULL, NULL, TO_ROOM);
    do_look(victim, "auto");
    return TRUE;
}

/*
 * NPC spells.
 */
bool
spell_acid_breath(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    /* Fixed the nice bug that referenced the _caster's_ equipment,
     * so that it checks the victim's instead *laugh* -S-
     */

    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    OBJ_DATA           *obj_lose;
    OBJ_DATA           *obj_next;
    int                 dam;
    int                 hpch;

    if (number_percent() < 2 * level && !saves_spell(level, victim)) {
        OREF(obj_next, OBJ_NEXTCONTENT);
        for (obj_lose = victim->first_carry; obj_lose != NULL; obj_lose = obj_next) {
            int                 iWear;

            obj_next = obj_lose->next_in_carry_list;

            if (number_bits(2) != 0)
                continue;

            if (IS_SET(obj_lose->extra_flags, ITEM_NODESTROY))
                continue;
            switch (obj_lose->item_type) {
                case ITEM_ARMOR:
                    if (obj_lose->value[0] > 0) {
                        act("$p is pitted and etched!", victim, obj_lose, NULL, TO_CHAR);
                        if ((iWear = obj_lose->wear_loc) != WEAR_NONE)
                            victim->armor -= apply_ac(obj_lose, iWear);
                        obj_lose->value[0] -= 1;
                        obj_lose->cost = 0;
                        if (iWear != WEAR_NONE)
                            victim->armor += apply_ac(obj_lose, iWear);
                    }
                    break;

                case ITEM_CONTAINER:
                    act("$p fumes and dissolves!", victim, obj_lose, NULL, TO_CHAR);
                    extract_obj(obj_lose);
                    break;
            }
        }
        OUREF(obj_next);
    }

    hpch = UMAX(10, ch->hit);
    dam = number_range(hpch / 16 + 1, hpch / 8);
    if (saves_spell(level, victim))
        dam /= 2;
    damage(ch, victim, dam, sn);
    return TRUE;
}

bool
spell_fire_breath(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    OBJ_DATA           *obj_lose;
    OBJ_DATA           *obj_next;
    int                 dam;
    int                 hpch;

    if (!IS_NPC(ch)) {
        dam = number_range(get_pseudo_level(ch) * 1.2, get_pseudo_level(ch) * 3);
        if (saves_spell(level, victim))
            dam /= 1.3;
        damage(ch, victim, dam, sn);
        return TRUE;
    }

    if (number_percent() < 2 * level && !saves_spell(level, victim)) {
        OREF(obj_next, OBJ_NEXTCONTENT);
        for (obj_lose = victim->first_carry; obj_lose != NULL; obj_lose = obj_next) {
            char               *msg = NULL;

            obj_next = obj_lose->next_in_carry_list;
            if (number_bits(2) != 0)
                continue;

            if (IS_SET(obj_lose->extra_flags, ITEM_NODESTROY))
                continue;
            switch (obj_lose->item_type) {
                default:
                    continue;
                case ITEM_CONTAINER:
                    msg = "$p ignites and burns!";
                    break;
                case ITEM_POTION:
                    msg = "$p bubbles and boils!";
                    break;
                case ITEM_SCROLL:
                    msg = "$p crackles and burns!";
                    break;
                case ITEM_STAFF:
                    msg = "$p smokes and chars!";
                    break;
                case ITEM_WAND:
                    msg = "$p sparks and sputters!";
                    break;
                case ITEM_FOOD:
                    msg = "$p blackens and crisps!";
                    break;
                case ITEM_PILL:
                    msg = "$p melts and drips!";
                    break;
            }
            act(msg, victim, obj_lose, NULL, TO_CHAR);
            extract_obj(obj_lose);
        }
        OUREF(obj_next);
    }

    hpch = UMAX(10, ch->hit);
    dam = number_range(hpch / 16 + 1, hpch / 8);
    if (saves_spell(level, victim))
        dam /= 2;
    damage(ch, victim, dam, sn);
    return TRUE;
}

bool
spell_frost_breath(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    OBJ_DATA           *obj_lose;
    OBJ_DATA           *obj_next;
    int                 dam;
    int                 hpch;

    if (number_percent() < 2 * level && !saves_spell(level, victim)) {
        OREF(obj_next, OBJ_NEXTCONTENT);
        for (obj_lose = victim->first_carry; obj_lose != NULL; obj_lose = obj_next) {
            char               *msg = NULL;

            obj_next = obj_lose->next_in_carry_list;
            if (number_bits(2) != 0)
                continue;

            if (IS_SET(obj_lose->extra_flags, ITEM_NODESTROY))
                continue;
            switch (obj_lose->item_type) {
                default:
                    continue;
                case ITEM_CONTAINER:
                case ITEM_DRINK_CON:
                case ITEM_POTION:
                    msg = "$p freezes and shatters!";
                    break;
            }
            act(msg, victim, obj_lose, NULL, TO_CHAR);
            extract_obj(obj_lose);
        }
        OUREF(obj_next);
    }

    hpch = UMAX(10, ch->hit);
    dam = number_range(hpch / 16 + 1, hpch / 8);
    if (saves_spell(level, victim))
        dam /= 2;
    damage(ch, victim, dam, sn);
    return TRUE;
}

bool
spell_gas_breath(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *vch;
    CHAR_DATA          *vch_next;
    int                 dam;
    int                 hpch;

    CREF(vch_next, CHAR_NEXTROOM);

    for (vch = ch->in_room->first_person; vch != NULL; vch = vch_next) {
        vch_next = vch->next_in_room;
        if (IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch)) {
            hpch = UMAX(10, ch->hit);
            dam = number_range(hpch / 16 + 1, hpch / 8);
            if (saves_spell(level, vch))
                dam /= 2;
            damage(ch, vch, dam, sn);
        }
    }
    CUREF(vch_next);
    return TRUE;
}

bool
spell_lightning_breath(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    int                 dam;
    int                 hpch;

    hpch = UMAX(10, ch->hit);
    dam = number_range(hpch / 16 + 1, hpch / 8);
    if (saves_spell(level, victim))
        dam /= 2;
    damage(ch, victim, dam, sn);
    return TRUE;
}

bool
spell_hellspawn(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    /* High level mag / psi spell. -S- */
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    int                 dam;

    dam = number_range(get_pseudo_level(ch) / 2, get_pseudo_level(ch) * 2.5);
    if (saves_spell(level, victim))
        dam /= 2;

    act("The Dark Powers of the HellSpawn strike $n!!", victim, NULL, NULL, TO_ROOM);
    send_to_char("The Dark Powers of the HellSpawn strike you!!\n\r", victim);
    damage(ch, victim, dam, -1);
    return TRUE;
}

bool
spell_planergy(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{

    /* Psi spell.  Work out which energy to summon, according to
     * ch's level.  Each 9 levels, the Psi gets the use of a different
     * spell, and more damage! ;)  (planergy = planar energy)
     * --Stephen
     */

    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    int                 dam;
    int                 lvl;
    char               *plane;
    char               *engulf;
    char                msg[255];
    char                msg2[255];
    char                msg3[255];
    char                msg4[255];

    if (level <= 8) {
        lvl = 4;
        plane = "Fire";
        engulf = "Flames";
    }
    else if (level <= 17) {
        lvl = 13;
        plane = "Sun";
        engulf = "a Heat Ray";
    }
    else if (level <= 26) {
        lvl = 22;
        plane = "Magma";
        engulf = "Lava";
    }
    else if (level <= 35) {
        lvl = 31;
        plane = "Radiance";
        engulf = "Light";
    }
    else if (level <= 44) {
        lvl = 40;
        plane = "Lightning";
        engulf = "Lightning";
    }
    else if (level <= 53) {
        lvl = 49;
        plane = "Salt";
        engulf = "Dehydration";
    }
    else if (level <= 62) {
        lvl = 58;
        plane = "Ash";
        engulf = "Cold";
    }
    else if (level <= 71) {
        lvl = 67;
        plane = "Positive";
        engulf = "Energy";
    }
    else {
        lvl = 80;
        plane = "Negative";
        engulf = "Energy";
    }

    dam = number_range(10, get_pseudo_level(ch) * 2);

    if (saves_spell(level, victim))
        dam /= 2;

    if (obj == NULL) {
        sprintf(msg, "You summon %s from the %s plane!!\n\r", engulf, plane);
        sprintf(msg2, "$n summons %s from the %s plane!!", engulf, plane);
        send_to_char(msg, ch);
        act(msg2, ch, NULL, NULL, TO_ROOM);
    }
    else {
        sprintf(msg, "$p calls %s from the %s plane!", engulf, plane);
        act(msg, ch, obj, NULL, TO_ROOM);
        act(msg, ch, obj, NULL, TO_CHAR);
    }

    sprintf(msg3, "You are struck by %s!!!!!  Ouch!\n\r", engulf);
    sprintf(msg4, "$N is struck down by %s!", engulf);

    send_to_char(msg3, victim);
    act(msg4, ch, NULL, victim, TO_NOTVICT);

    damage(ch, victim, dam, -1);

    return TRUE;
}

bool
spell_visit(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{

    /* Psi spell.  Kinda like summon, but almost done in reverse.
     * allows the Psi to try and transfer themself to the 'victim'
     * --Stephen
     */

    CHAR_DATA          *victim = (CHAR_DATA *) vo;

    if ((victim = get_char_world(ch, target_name)) == NULL || victim == ch || IS_NPC(victim)
        || victim->in_room == NULL || IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
        || IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
        || IS_SET(victim->in_room->room_flags, ROOM_SAFE)
        || IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
        || !IS_SET(victim->in_room->area->flags, AREA_TELEPORT)) {
        send_to_char("You failed.\n\r", ch);
        return TRUE;
    }

    if (victim->in_room->vnum == 7699 || victim->in_room->vnum == 7698) {
        send_to_char("You failed.\n\r", ch);
        return TRUE;
    }

    if (IS_SET(victim->act, PLR_NOVISIT)) {
        send_to_char("You seem unable to visit your target!\n\r", ch);
        return TRUE;
    }

    /* Check is ch screws up, and ends up in limbo... <grin> */

    if (number_percent() < 25) {    /* 25% chance */
        send_to_char("You get distracted, and appear in the middle of nowhere!\n\r", ch);
        act("$n disappears suddenly.", ch, NULL, NULL, TO_ROOM);
        char_from_room(ch);
        char_to_room(ch, get_room_index(ROOM_VNUM_LIMBO));
        act("$n arrives suddenly.", ch, NULL, NULL, TO_ROOM);
        do_look(ch, "auto");
        return TRUE;
    }

    act("$n disappears suddenly.", ch, NULL, NULL, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, victim->in_room);
    act("$n arrives suddenly.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You change locations!\n\r", ch);
    do_look(ch, "auto");
    return TRUE;
}

bool
spell_barrier(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{

    /* Psi spell, like shield, but slightly better.  Good idea to make
     * sure Psi's don't have access to shield as well as this... ;)
     * -- Stephen
     */

    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (is_affected(victim, sn))
        return FALSE;

    af.type = sn;
    af.duration = 4 + (level / 20);
    af.location = APPLY_AC;
    af.modifier = -get_pseudo_level(ch) / 5;
    af.bitvector = 0;
    af.save = TRUE;
    affect_to_char(victim, &af);

    act("$n is surrounded by a telekinetic barrier.", victim, NULL, NULL, TO_ROOM);
    send_to_char("You are surrounded by a telekinetic barrier.\n\r", victim);
    return TRUE;
}

bool
spell_static(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    /* Psi Spell.
     * Uses ch's movement, which is transfered to static electricity    
     * which is then chanelled at the poor victim <laugh>
     * --Stephen
     */

    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    int                 dam;
    int                 loss;

    send_to_char("You transfer kinetic to static energy!\n\r", ch);

    loss = dice(0, 1.5 * get_pseudo_level(ch));

    dam = number_range(8, 15) * (get_pseudo_level(ch) / 8);
    if (ch->move == 0) {
        send_to_char("You have no kinetic energy left!!\n\r", ch);
        return FALSE;
    }

    ch->move = UMAX(1, ch->move - loss);

    if (saves_spell(level, victim))
        dam /= 2;

    act("$n transfers kinetic to static energy.", ch, NULL, NULL, TO_ROOM);
    act("$n's static discharge makes $N shake and shudder!", ch, NULL, victim, TO_NOTVICT);
    act("Your static discharge makes $N shake and shudder!", ch, NULL, victim, TO_CHAR);
    act("$N's static discharge makes you shake and shudder!", victim, NULL, ch, TO_CHAR);
    damage(ch, victim, dam, -1);

    return TRUE;
}

bool
spell_phobia(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    /* Psi Spell.
     * Conjures victims's phobia in their mind, does dam (small) or
     * causes them to flee.
     * --Stephen
     */

    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    int                 dam;

    if (IS_SET(victim->act, ACT_SENTINEL) && !IS_NPC(ch) && IS_NPC(victim)) {
        return TRUE;
    }
    if ((victim->level) > (level + 8)) {
        send_to_char("Your spell fails to take affect!\n\r", ch);
        return TRUE;
    }

    /* figure out dam.... not too much as spell is meant to make vo flee */

    dam = dice(2, level / 8);
    if (obj == NULL) {
        act("$n projects nightmare images into $N's mind!", ch, NULL, victim, TO_NOTVICT);
        act("You project nightmare images into $N's mind!", ch, NULL, victim, TO_CHAR);
        act("$N projects nightmare images into your mind!", victim, NULL, ch, TO_CHAR);
    }
    else {
        act("$p projects nightmare images into $n's mind!", victim, obj, NULL, TO_ROOM);
        act("$p projects nightmare images into your mind!", victim, obj, NULL, TO_CHAR);
    }
    send_to_char("Your worst phobia springs to life in your mind. Arrrggghhh!\n\r", victim);
    act("$N suffers a mental phobia attack!", ch, NULL, victim, TO_NOTVICT);

    damage(ch, victim, dam, -1);

    if (number_percent() < 70) {    /* 70% chance */
        act("$N screams at $n in horror!!", ch, NULL, victim, TO_ROOM);
        act("$N screams at you in horror!!", ch, NULL, victim, TO_CHAR);
        send_to_char("You flip, and look for escape!!\n\r", victim);
        do_flee(victim, "");
    }

    return TRUE;
}

bool
spell_mindflame(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    /* Psi Multiple Attack - screws up all those affected */

    CHAR_DATA          *vch;
    CHAR_DATA          *vch_next;

    if (obj == NULL) {
        send_to_char("You initiate a mindflame attack!!\n\r", ch);
        act("$n concentrates, and initiates a mindflame attack!", ch, NULL, NULL, TO_ROOM);
    }
    else {
        act("$p glows, and initiates a mindflame attack!", ch, obj, NULL, TO_ROOM);
        act("$p glows, and initiates a mindflame attack!", ch, obj, NULL, TO_CHAR);
    }
    CREF(vch_next, CHAR_NEXT);
    for (vch = first_char; vch != NULL; vch = vch_next) {
        vch_next = vch->next;
        if (vch->in_room == NULL || ch->in_room == NULL)
            continue;

        /* ninjafix -dave */
        if (!IS_NPC(vch)
            && (IS_IMMORTAL(vch)
                || vch->stance == STANCE_AMBUSH || vch->stance == STANCE_AC_BEST)
            )
            continue;

        if (vch->in_room == ch->in_room) {
            if (vch != ch && (IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch))) {
                act("$n rolls on the floor, clutching $s head in pain!", vch, NULL, NULL, TO_ROOM);
                send_to_char("You roll on the floor, clutching your head in pain!\n\r", vch);
                damage(ch, vch, (get_pseudo_level(ch) / 2) + dice(6, 10), -1);
            }
            continue;

        }

        if (vch->in_room->area == ch->in_room->area)
            send_to_char("You notice a slight burning feeling in your mind.\n\r", vch);
    }
    CUREF(vch_next);
    return TRUE;
}

bool
spell_chain_lightning(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    /* affects all in a room, besides caster */
    CHAR_DATA          *vch;
    CHAR_DATA          *vch_next;
    int                 dam;

    /* Work out starting damage. */
    dam = dice(2, get_pseudo_level(ch));

    if (obj == NULL) {
        send_to_char("Lightning flashes from your fingers!\n\r", ch);
        act("$n unleashes lightning from $s fingers!", ch, NULL, NULL, TO_ROOM);
    }
    else {
        act("A lightning bolt flashes from $p!", ch, obj, NULL, TO_ROOM);
        act("A lightning bolt flashes from $p!", ch, obj, NULL, TO_CHAR);
    }

    CREF(vch_next, CHAR_NEXTROOM);
    for (vch = ch->in_room->first_person; vch != NULL; vch = vch_next) {
        vch_next = vch->next_in_room;

        /* ninjafix -dave */
        if (!IS_NPC(vch)
            && (IS_IMMORTAL(vch)
                || vch->stance == STANCE_AMBUSH || vch->stance == STANCE_AC_BEST)
            )
            continue;

        if (vch != ch && IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch)) {
            send_to_char("The lightning bolt strikes you!\n\r", vch);
            act("The lightning bolt strikes $n!", vch, NULL, NULL, TO_ROOM);
            damage(ch, vch, dam, -1);    /* -1 means no dammage message */
            dam = (4 * dam / 5);
        }
    }
    CUREF(vch_next);

    if (!ch->is_free) {
        act("The lightning bolt hits the ground, and is GONE!", ch, NULL, NULL, TO_ROOM);
        send_to_char("Your lightning bolts hits the ground and is GONE!\n\r", ch);
    }

    return TRUE;
}

bool
spell_suffocate(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    int                 dam;

    dam = dice(level, 2);

    if ((level + 5) > victim->level)
        dam += (((level + 5) - victim->level) * 2);
    else
        dam -= ((victim->level - level) * 2);

    act("$n chokes and gags!", victim, NULL, NULL, TO_ROOM);
    send_to_char("You feel your throat squeezed by an invisible force!\n\r", victim);

    damage(ch, victim, dam, -1);    /* -1 = no dam message */
    return TRUE;
}

bool
spell_enhance_weapon(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    OBJ_DATA           *ob = (OBJ_DATA *) vo;
    AFFECT_DATA        *paf;

    /* Quick way to stop imms (Bash?) enchanting weapons for players */
    if (IS_IMMORTAL(ch) && ch->level != 90) {
        send_to_char("Nothing Happens.\n\r", ch);
        return FALSE;
    }

    if ((ob->item_type != ITEM_WEAPON)
        || IS_OBJ_STAT(ob, ITEM_MAGIC)
        || ob->first_apply != NULL)
        return TRUE;

    GET_FREE(paf, affect_free);
    paf->type = sn;
    paf->duration = 3 + (level / 4);
    paf->location = APPLY_HITROLL;
    paf->modifier = 3;
    paf->bitvector = 0;
    paf->save = TRUE;
    LINK(paf, ob->first_apply, ob->last_apply, next, prev);

    GET_FREE(paf, affect_free);
    paf->type = -1;
    paf->duration = 3 + (level / 4);
    paf->location = APPLY_DAMROLL;
    paf->modifier = 2;
    paf->bitvector = 0;
    paf->save = TRUE;
    LINK(paf, ob->first_apply, ob->last_apply, next, prev);

    act("$p shines brightly.", ch, ob, NULL, TO_CHAR);
    act("$p belonging to $n shines brightly.", ch, ob, NULL, TO_ROOM);

    return TRUE;
}

bool
spell_bloody_tears(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;

    if (IS_AFFECTED(victim, AFF_BLIND) || saves_spell(level, victim))
        return TRUE;

    act("$n's eyes start bleeding!", victim, NULL, NULL, TO_ROOM);
    send_to_char("Your eyes start bleeding!\n\r", victim);

    damage(ch, victim, (get_pseudo_level(ch)), -1);    /* -1 = no dam message */
    spell_blindness(skill_lookup("blindness"), level, ch, vo, obj);

    return TRUE;
}

bool
spell_mind_bolt(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    int                 cnt;
    int                 foo;
    int                 dam;

    if (obj == NULL) {
        act("$n attacks $N with a Mind Bolt!", ch, NULL, victim, TO_NOTVICT);
        act("You attack $N with a Mind Bolt!", ch, NULL, victim, TO_CHAR);
        act("$n attacks you with a Mind Bolt!", ch, NULL, victim, TO_VICT);
    }
    else {
        act("$p strikes $n with a Mind Bolt!", victim, obj, NULL, TO_ROOM);
        act("$p strikes you with a Mind Bolt!", victim, obj, NULL, TO_CHAR);
    }
    cnt = 1 + (level >= 20) + (level >= 35) + (level >= 50) + (ch->level >= 65);
    for (foo = 0; foo < cnt; foo++) {
        dam = (cnt * (level / 3));
        if (saves_spell(level, victim))
            dam /= 2;
        act("$n shakes and shudders!", victim, NULL, NULL, TO_ROOM);
        act("You shake and shudder!", victim, NULL, ch, TO_CHAR);
    }
    return TRUE;
}

bool
spell_nerve_fire(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *vch;
    CHAR_DATA          *vch_next;

    if (obj == NULL) {
        send_to_char("You initiate a Nerve Fire attack!!\n\r", ch);
        act("$n concentrates, and initiates a Nerve Fire attack!", ch, NULL, NULL, TO_ROOM);
    }
    else {
        act("$p glows with the power of Nerve Fire!", ch, obj, NULL, TO_ROOM);
        act("$p glows with the power of Nerve Fire!", ch, obj, NULL, TO_CHAR);
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
            if (vch != ch && (IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch))) {
                act("$n rolls on the floor, twitching in agony!", vch, NULL, NULL, TO_ROOM);
                act("You roll on the floor, twitching in agony!", vch, NULL, NULL, TO_CHAR);
                damage(ch, vch, (get_pseudo_level(ch)) + dice(6, 10), -1);
            }
            continue;
        }

        if (vch->in_room->area == ch->in_room->area)
            send_to_char("You notice a slight burning feeling in your body.\n\r", vch);
    }
    CUREF(vch_next);
    return TRUE;
}

bool
spell_fighting_trance(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (is_affected(victim, sn))
        return FALSE;

    af.type = sn;
    af.duration = 6 + (level / 20);
    af.location = APPLY_HITROLL;
    af.modifier = get_pseudo_level(ch) / 40;
    af.bitvector = 0;
    af.save = TRUE;
    affect_to_char(victim, &af);

    af.location = APPLY_SAVING_SPELL;
    af.modifier = 0 - get_pseudo_level(ch) / 8;
    affect_to_char(victim, &af);

    af.location = APPLY_DAMROLL;
    af.modifier = get_pseudo_level(ch) / 40;
    affect_to_char(victim, &af);

    af.location = APPLY_AC;
    af.modifier = -get_pseudo_level(ch) / 40;
    affect_to_char(victim, &af);
    send_to_char("You feel much stronger.\n\r", victim);
    act("$n looks much stronger.", victim, NULL, NULL, TO_ROOM);
    if (ch != victim)
        send_to_char("Ok.\n\r", ch);
    return TRUE;
}

bool
spell_phase(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (IS_AFFECTED(victim, AFF_PASS_DOOR) || item_has_apply(victim, ITEM_APPLY_PASS_DOOR))
        return FALSE;

    af.type = sn;
    af.duration = 3 + (level / 20);
    af.location = APPLY_AC;
    af.modifier = -get_pseudo_level(ch) / 10;
    af.bitvector = AFF_PASS_DOOR;
    af.save = TRUE;
    affect_to_char(victim, &af);
    send_to_char("Your body switches phase.\n\r", victim);
    act("$n's body switches phase.", victim, NULL, NULL, TO_ROOM);
    return TRUE;
}

bool
spell_dimension_blade(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    OBJ_DATA           *ob = (OBJ_DATA *) vo;
    AFFECT_DATA        *paf;

    /* Quick way to stop imms (Bash?) enchanting weapons for players */
    if (IS_IMMORTAL(ch) && ch->level != 90) {
        send_to_char("Nothing Happens.\n\r", ch);
        return FALSE;
    }

    if (dice(4, 5) == 20) {
        act("$p shatters into pieces!", ch, ob, NULL, TO_CHAR);
        act("$p carried by $n shatters!", ch, ob, NULL, TO_ROOM);
        extract_obj(ob);
        return TRUE;
    }

    if ((ob->item_type != ITEM_WEAPON)
        || IS_OBJ_STAT(ob, ITEM_MAGIC)
        || ob->first_apply != NULL)
        return TRUE;

    GET_FREE(paf, affect_free);
    paf->type = sn;
    paf->duration = -1;
    paf->location = APPLY_HITROLL;
    paf->modifier = 1 + (level >= 50) + (level >= 60) + (level >= 70);
    paf->bitvector = 0;
    paf->save = TRUE;
    LINK(paf, ob->first_apply, ob->last_apply, next, prev);

    GET_FREE(paf, affect_free);
    paf->type = -1;
    paf->duration = -1;
    paf->location = APPLY_DAMROLL;
    paf->modifier = 1 + (level >= 55) + (level >= 70);
    paf->bitvector = 0;
    paf->save = TRUE;
    LINK(paf, ob->first_apply, ob->last_apply, next, prev);

    act("Part of $p switches into a different plane.", ch, ob, NULL, TO_CHAR);
    act("$n makes $p into a dimension blade.", ch, ob, NULL, TO_ROOM);
    return TRUE;
}

bool
spell_produce_food(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{

    OBJ_DATA           *mushroom;

    mushroom = create_object(get_obj_index(OBJ_VNUM_FOOD), 0);
    mushroom->value[0] = 5 + level;
    obj_to_room(mushroom, ch->in_room);
    act("$p suddenly appears.", ch, mushroom, NULL, TO_ROOM);
    act("$p suddenly appears.", ch, mushroom, NULL, TO_CHAR);
    return TRUE;
}

bool
spell_animate(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    OBJ_DATA           *ob;
    OBJ_DATA           *bits;
    CHAR_DATA          *corpse;

    if (IS_NPC(ch))
        return FALSE;

    ob = get_obj_here(ch, target_name);

    if (ob == NULL) {
        send_to_char("Couldn't find it.\n\r", ch);
        return FALSE;
    }

    if (ob->item_type != ITEM_CORPSE_NPC) {
        send_to_char("You can't animate that!\n\r", ch);
        return FALSE;
    }

    if (!can_summon_charmie(ch))
        return FALSE;

    act("$n lays $s hands onto the $p!", ch, ob, NULL, TO_ROOM);
    act("You lay your hands upon the $p.", ch, ob, NULL, TO_CHAR);

    act("Bright bolts of lightning fly out from $p!!", ch, ob, NULL, TO_ROOM);
    act("Bright bolts of lightning fly out from $p!!", ch, ob, NULL, TO_CHAR);

    corpse = create_mobile(get_mob_index(MOB_VNUM_ZOMBIE));
    char_to_room(corpse, ch->in_room);
    act("$n stands up, and stretches slowly.", corpse, NULL, NULL, TO_ROOM);

    if (ob->item_type == ITEM_CORPSE_NPC) {
        SET_BIT(corpse->act, ACT_UNDEAD);
        act("$n's eyes glow black!", corpse, NULL, NULL, TO_ROOM);
    }

    corpse->level = ob->level;    /* Level of (N)PC before death */
    corpse->max_hit = dice(5, level);
    corpse->hit = corpse->max_hit;
    corpse->max_move = dice(10, level);
    corpse->move = corpse->max_move;    /* Set Zombie's stats */

    for (;;) {
        if (ob->first_in_carry_list == NULL)
            break;
        bits = ob->first_in_carry_list;
        obj_from_obj(bits);
        obj_to_char(bits, corpse);

    }

    extract_obj(ob);
    do_wear(corpse, "all");        /* FIXME: better to check items, then wear... */
    SET_BIT(corpse->act, ACT_PET);
    SET_BIT(corpse->affected_by, AFF_CHARM);
    corpse->extract_timer = get_pseudo_level(ch) / 3;

    add_follower(corpse, ch);
    return TRUE;
}

bool
spell_see_magic(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (IS_AFFECTED(victim, AFF_DETECT_MAGIC) || item_has_apply(victim, ITEM_APPLY_HIDE))
        return FALSE;
    af.type = sn;
    af.duration = 6 + (level / 4);
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_DETECT_MAGIC;
    af.save = TRUE;
    affect_to_char(victim, &af);
    send_to_char("Your eyes tingle.\n\r", victim);
    act("$n's eyes glow briefly.", victim, NULL, NULL, TO_ROOM);
    return TRUE;
}

bool
spell_detection(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    char                buf[MAX_INPUT_LENGTH];
    OBJ_DATA           *ob;
    OBJ_DATA           *in_obj;
    bool                found;

    found = FALSE;
    for (ob = first_obj; ob != NULL; ob = ob->next) {

        if (!can_see_obj(ch, ob) || !is_name(target_name, ob->name)
            || IS_SET(ob->extra_flags, ITEM_RARE)
            || (ob->item_type == ITEM_PIECE)
            || (IS_SET(ob->extra_flags, ITEM_UNIQUE))
            || (!str_prefix(target_name, "unique")))
            continue;

        for (in_obj = ob; in_obj->in_obj != NULL; in_obj = in_obj->in_obj);

        if (in_obj->carried_by != NULL && IS_IMMORTAL(in_obj->carried_by))
            break;

        if (in_obj->carried_by != NULL) {
            found = TRUE;
            sprintf(buf, "%s carried by %s.\n\r", ob->short_descr, can_see(ch, in_obj->carried_by) ? PERS(in_obj->carried_by, ch) : "someone");
        }
        else {
            found = TRUE;
            sprintf(buf, "%s in %s.\n\r", ob->short_descr, in_obj->in_room == NULL ? "somewhere" : in_obj->in_room->name);
        }

        buf[0] = UPPER(buf[0]);
        send_to_char(buf, ch);
    }

    if (!found)
        send_to_char("You fail to detect any such object.\n\r", ch);

    return TRUE;
}

bool
spell_fire_blade(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    OBJ_DATA           *blade;

    if (get_obj_wear(ch, "fireblade") != NULL)
        return FALSE;            /* Only have one at a time.. */

    if (remove_obj(ch, WEAR_WIELD, TRUE)) {
        blade = create_object(get_obj_index(OBJ_VNUM_FIREBLADE), level);
        obj_to_char(blade, ch);
        equip_char(ch, blade, WEAR_WIELD);
        blade->timer = 2 + (level / 20);
        act("A blazing FireBlade appears in $n's hand!", ch, NULL, NULL, TO_ROOM);
        send_to_char("A blazing FireBlade appears in your hand!\n\r", ch);
    }
    return TRUE;
}

bool
spell_know_weakness(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    AFFECT_DATA         af;

    if (is_affected(ch, sn))
        return FALSE;

    af.type = sn;
    af.duration = UMAX(3, get_pseudo_level(ch) / 15);
    af.location = APPLY_HITROLL;
    af.modifier = get_pseudo_level(ch) / 30;
    af.bitvector = 0;
    af.save = TRUE;
    affect_to_char(ch, &af);
    send_to_char("You are more aware of your enemy's weaknesses.\n\r", ch);
    return TRUE;
}

bool
spell_know_critical(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    AFFECT_DATA         af;

    if (is_affected(ch, sn))
        return FALSE;

    af.type = sn;
    af.duration = UMAX(3, get_pseudo_level(ch) / 15);
    af.location = APPLY_DAMROLL;
    af.modifier = get_pseudo_level(ch) / 30;
    af.bitvector = 0;
    af.save = TRUE;
    affect_to_char(ch, &af);
    send_to_char("You are more aware of critical damage points.\n\r", ch);
    return TRUE;
}

bool
spell_calm(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *ppl;
    int                 chance;

    for (ppl = ch->in_room->first_person; ppl != NULL; ppl = ppl->next_in_room)
        if (IS_NPC(ppl) && ppl != ch) {
            chance = ((IS_NPC(ch) ? 50 : ch->pcdata->learned[sn] / 2)
                + (5 * (level - ppl->level)));
            if (number_percent() < chance)
                stop_fighting(ppl, TRUE);
        }
    if (obj == NULL) {
        act("$n emits a great feeling of calm around you.", ch, NULL, NULL, TO_ROOM);
        send_to_char("You emit a great feeling of calm around you.\n\r", ch);
    }
    else {
        act("$p glows with a clam light.", ch, obj, NULL, TO_ROOM);
        act("$p glows with a clam light.", ch, obj, NULL, TO_CHAR);
    }
    return TRUE;
}

bool
spell_hypnosis(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    /* quest_mob to stop questors being charmed */
    extern CHAR_DATA   *quest_mob;

    if (victim == ch) {
        send_to_char("You like yourself even better!\n\r", ch);
        return FALSE;
    }

    if (!IS_NPC(victim)) {
        send_to_char("You're not that powerful.\n\r", ch);
        return FALSE;
    }

    if (IS_AFFECTED(victim, AFF_CHARM)
        || IS_AFFECTED(ch, AFF_CHARM)
        || level - 5 < victim->level || saves_spell(level, victim))
        return TRUE;

    /* stop lamers charming questor */
    if (IS_NPC(victim) && (quest_mob == victim)) {
        send_to_char("I think NOT!\n\r", ch);
        return FALSE;
    }

    if (IS_SET(victim->act, ACT_AGGRESSIVE) || IS_SET(victim->act, ACT_ALWAYS_AGGR))
        return FALSE;

    if (!IS_NPC(ch) && max_orders(ch) <= ch->num_followers) {
        send_to_char("You have already charmed too many creatures!\n\r", ch);
        return FALSE;
    }

    if (victim->master)
        stop_follower(victim);
    af.type = gsn_charm_person;

    if (ch->adept_level  > 0)
        af.duration = get_pseudo_level(ch) / 5;
    else
        af.duration = 3 + (level / 8);

    af.location = 0;
    af.modifier = 0;
    af.bitvector = AFF_CHARM;
    af.save = TRUE;
    affect_to_char(victim, &af);
    act("Isn't $n just so nice?", ch, NULL, victim, TO_VICT);
    if (ch != victim)
        send_to_char("Ok.\n\r", ch);
    victim->extract_timer = get_pseudo_level(ch) / 3;
    add_follower(victim, ch);
    return TRUE;
}

bool
spell_mind_flail(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    int                 dam;
    CHAR_DATA          *victim = (CHAR_DATA *) vo;

    if (victim == ch)
        return FALSE;

    if (IS_NPC(victim) && IS_SET(victim->act, ACT_NOMIND))
        return TRUE;

    dam = 5 + (5 * (level > 20) + (level >= 40) + (level >= 60) + (level > -80));

    if (saves_spell(level, victim))
        dam /= 2;

    if (obj == NULL) {
        act("$n mind flails $N!", ch, NULL, victim, TO_NOTVICT);
        act("$N mind flails you!", victim, NULL, ch, TO_CHAR);
        act("You mind flail $N!", ch, NULL, victim, TO_CHAR);
    }
    else {
        act("$p mind flails $n!", victim, obj, NULL, TO_ROOM);
        act("$p mind flails you!", victim, obj, NULL, TO_ROOM);
    }
    damage(ch, victim, dam, -1);
    return TRUE;
}

bool
spell_know_item(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    OBJ_DATA           *ob = (OBJ_DATA *) vo;
    char                buf[MAX_STRING_LENGTH];
    AFFECT_DATA        *paf;

    sprintf(buf,
        "Object '%s' is type %s, extra flags %s.\n\rWeight is %d.\n\r",
        ob->short_descr, item_type_name(ob), extra_bit_name(ob->extra_flags), ob->weight);
    send_to_char(buf, ch);
    switch (ob->item_type) {
        case ITEM_SCROLL:
        case ITEM_POTION:
            sprintf(buf, "Level %d spells of:", ob->value[0]);
            send_to_char(buf, ch);

            if (ob->value[1] >= 0 && ob->value[1] < MAX_SKILL) {
                send_to_char(" '", ch);
                send_to_char(skill_table[ob->value[1]].name, ch);
                send_to_char("'", ch);
            }

            if (ob->value[2] >= 0 && ob->value[2] < MAX_SKILL) {
                send_to_char(" '", ch);
                send_to_char(skill_table[ob->value[2]].name, ch);
                send_to_char("'", ch);
            }

            if (ob->value[3] >= 0 && ob->value[3] < MAX_SKILL) {
                send_to_char(" '", ch);
                send_to_char(skill_table[ob->value[3]].name, ch);
                send_to_char("'", ch);
            }

            send_to_char(".\n\r", ch);
            break;

        case ITEM_WAND:
        case ITEM_STAFF:
            sprintf(buf, "Has %d(%d) charges of level %d", ob->value[1], ob->value[2], ob->value[0]);
            send_to_char(buf, ch);

            if (ob->value[3] >= 0 && ob->value[3] < MAX_SKILL) {
                send_to_char(" '", ch);
                send_to_char(skill_table[ob->value[3]].name, ch);
                send_to_char("'", ch);
            }

            send_to_char(".\n\r", ch);
            break;

        case ITEM_WEAPON:
            sprintf(buf, " Average damage is %d.\n\r", (ob->value[1] + ob->value[2]) / 2);
            send_to_char(buf, ch);
            break;

        case ITEM_ARMOR:
            sprintf(buf, "Armor class is %d.\n\r", ob->value[0]);
            send_to_char(buf, ch);
            break;
    }
    for (paf = ob->first_apply; paf != NULL; paf = paf->next) {
        if (paf->location != APPLY_NONE && paf->modifier != 0) {
            sprintf(buf, "Affects %s by %d.\n\r", affect_loc_name(paf->location), paf->modifier);
            send_to_char(buf, ch);
        }
    }

    return TRUE;
}

bool
spell_physic_thrust(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    int                 dam;
    CHAR_DATA          *victim = (CHAR_DATA *) vo;

    if (victim == ch)
        return FALSE;

    if (IS_NPC(victim) && IS_SET(victim->act, ACT_NOMIND))
        return TRUE;

    dam = 5 + (6 * (level > 20) + (level >= 40) + (level >= 60) + (level > -80));

    if (saves_spell(level, victim))
        dam /= 2;

    act("$n psychic thrusts $N!", ch, NULL, victim, TO_NOTVICT);
    act("$N psychic thrusts you!", victim, NULL, ch, TO_CHAR);
    act("You psychic thrust $N!", ch, NULL, victim, TO_CHAR);
    damage(ch, victim, dam, -1);
    return TRUE;
}

bool
spell_physic_crush(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    int                 dam;
    CHAR_DATA          *victim = (CHAR_DATA *) vo;

    if (victim == ch)
        return FALSE;

    if (IS_NPC(victim) && IS_SET(victim->act, ACT_NOMIND))
        return TRUE;

    dam = 20 + (7 * (level > 20) + (level >= 40) + (level >= 60) + (level > -80));

    if (saves_spell(level, victim))
        dam /= 2;

    act("$n psychic crushes $N!", ch, NULL, victim, TO_NOTVICT);
    act("$N psychic crushes you!", victim, NULL, ch, TO_CHAR);
    act("You psychic crush $N!", ch, NULL, victim, TO_CHAR);
    damage(ch, victim, dam, -1);
    return TRUE;
}

bool
spell_ego_whip(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    int                 dam;
    CHAR_DATA          *victim = (CHAR_DATA *) vo;

    if (victim == ch)
        return FALSE;

    if (IS_NPC(victim) && IS_SET(victim->act, ACT_NOMIND))
        return TRUE;

    dam = 50 + (13 * (level > 20) + (level >= 40) + (level >= 60) + (level > -80));

    if (saves_spell(level, victim))
        dam /= 2;

    act("$n ego whips $N!", ch, NULL, victim, TO_NOTVICT);
    act("$N ego whips you!", victim, NULL, ch, TO_CHAR);
    act("You ego whip $N!", ch, NULL, victim, TO_CHAR);
    damage(ch, victim, dam, -1);
    return TRUE;
}

bool
spell_night_vision(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (IS_AFFECTED(victim, AFF_INFRARED) || item_has_apply(victim, ITEM_APPLY_INFRA))
        return (ch == victim ? FALSE : TRUE);
    act("$n's eyes glow red.", ch, NULL, NULL, TO_ROOM);
    af.type = sn;
    af.duration = 4 + (level / 3);
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_INFRARED;
    af.save = TRUE;
    affect_to_char(victim, &af);
    send_to_char("Your eyes glow red.\n\r", victim);
    if (ch != victim)
        send_to_char("Ok.\n\r", ch);
    return TRUE;
}

bool
spell_stalker(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    return TRUE;
}

bool
spell_mystic_armor(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (ch == victim) {
        send_to_char("You are mystically armoured, but it suddenly fades away!\n\r", ch);
        return FALSE;
    }

    if (is_affected(victim, sn))
        return TRUE;

    af.type = sn;
    af.duration = 4 + (level / 3);
    af.location = APPLY_AC;
    af.modifier = -get_pseudo_level(ch) / 8;
    af.bitvector = 0;
    af.save = TRUE;
    affect_to_char(victim, &af);

    act("$n is surrounded by $N's mystic armour.", victim, NULL, ch, TO_ROOM);
    act("$N is surrounded by your mystic armour.", ch, NULL, victim, TO_CHAR);
    act("You are surrounded by $N's mystic armour.", victim, NULL, ch, TO_CHAR);
    return TRUE;
}

bool
spell_flare(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (IS_AFFECTED(victim, AFF_BLIND) || saves_spell(level, victim))
        return TRUE;

    if (victim == ch)
        return FALSE;

    af.type = sn;
    af.location = APPLY_HITROLL;
    af.modifier = -get_pseudo_level(ch) / 30;
    af.duration = 1 + (level / 4);
    af.bitvector = AFF_BLIND;
    af.save = TRUE;

    affect_to_char(victim, &af);
    act("$n invokes the power of Ra to produce a solar flare which blinds $N!", ch, NULL, victim, TO_NOTVICT);
    act("$N invokes the power of Ra to produce a solar flare.  You are BLINDED!", victim, NULL, ch, TO_CHAR);
    act("You invoke the power of Ra to produce a solar flare, which blinds $N!", ch, NULL, victim, TO_CHAR);

    return TRUE;
}

bool
spell_travel(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    /* Transfer the player to Midgaard recall */

    ROOM_INDEX_DATA    *room;

    if (ch->fighting != NULL) {
        send_to_char("You can't travel when fighting!\n\r", ch);
        return FALSE;
    }

    if ((room = get_room_index(3001)) == NULL) {
        send_to_char("It seems the Midgaard Temple has vanished!\n\r", ch);
        return FALSE;
    }

    act("$n is engulfed by a stream of green light!", ch, NULL, NULL, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, room);
    act("$n arrives, carried by a stream of green light!", ch, NULL, NULL, TO_ROOM);
    do_look(ch, "auto");
    return TRUE;
}

bool
spell_window(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    /* Create a magic window to look into another room */
    OBJ_DATA           *beacon;
    OBJ_DATA           *ob;

    if (target_name[0] == '\0') {
        send_to_char("Form a window to whom?\n\r", ch);
        return FALSE;
    }

    beacon = get_obj_world(ch, target_name);

    if (beacon == NULL || (beacon->item_type != ITEM_BEACON)) {
        send_to_char("Couldn't find target.  Sorry.\n\r", ch);
        return FALSE;
    }

    if (beacon->in_room == NULL) {
        send_to_char("It seems that someone is carrying it.\n\r", ch);
        return FALSE;
    }

    if (str_cmp(beacon->owner, ch->name)) {
        send_to_char("That's not one of YOUR beacons!\n\r", ch);
        return FALSE;
    }

    act("$n creates a magic window with a wave of $s hand.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You create a magic window with a wave of your hand.\n\r", ch);

    ob = create_object(get_obj_index(OBJ_VNUM_WINDOW), level);
    obj_to_room(ob, ch->in_room);
    ob->timer = 1 + (level / 30);
    ob->value[0] = beacon->in_room->vnum;
    ob->value[1] = 1;
    send_to_room("The beacon suddenly vanishes!\n\r", beacon->in_room);
    extract_obj(beacon);
    return TRUE;
}

bool
spell_portal(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    /* Create a magic portal to another room */
    OBJ_DATA           *beacon;
    OBJ_DATA           *ob;

    if (target_name[0] == '\0') {
        send_to_char("Form a portal to what?\n\r", ch);
        return FALSE;
    }

    beacon = get_obj_world(ch, target_name);

    if (beacon == NULL || (beacon->item_type != ITEM_BEACON)) {
        send_to_char("Couldn't find target.  Sorry.\n\r", ch);
        return FALSE;
    }

    if (beacon->in_room != NULL && beacon->in_room->vnum == 1) {
        send_to_char("CHEATER CHEATER PUMPKIN EATER!\n\r", ch);
        return FALSE;
    }
    if (beacon->in_room == NULL) {
        send_to_char("It seems that someone is carrying it.\n\r", ch);
        return FALSE;
    }

    /* bugfix: see spell_beacon */
    if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM) && ch->master && str_cmp(beacon->owner, ch->master->name)) {
        send_to_char("That's not one of YOUR beacons!\n\r", ch);
        return FALSE;
    }

    if (!IS_NPC(ch) && str_cmp(beacon->owner, ch->name)) {
        send_to_char("That's not one of YOUR beacons!\n\r", ch);
        return FALSE;
    }

    /*
       if ( str_cmp( beacon->owner, ch->name ) )
       {
       send_to_char( "That's not one of YOUR beacons!\n\r", ch );
       return FALSE;
       }   
     */

    ob = create_object(get_obj_index(OBJ_VNUM_PORTAL), level);
    obj_to_room(ob, ch->in_room);
    ob->timer = 1 + (level / 30);
    ob->value[0] = (beacon->carried_by == NULL ? beacon->in_room->vnum : beacon->carried_by->in_room->vnum);
    ob->value[1] = 1;
    ob->value[2] = 1;

    act("$n creates $p with a wave of $s hand.", ch, ob, NULL, TO_ROOM);
    act("You create $p with a wave of your hand.", ch, ob, NULL, TO_CHAR);

    ob = create_object(get_obj_index(OBJ_VNUM_PORTAL), level);
    obj_to_room(ob, beacon->in_room);
    ob->timer = 1 + (level / 30);
    ob->value[0] = ch->in_room->vnum;
    ob->value[1] = 1;
    ob->value[2] = 1;
    send_to_room("The beacon suddenly vanishes!\n\r", beacon->in_room);
    send_to_room("A glowing portal suddenly forms before you!\n\r", ob->in_room);
    extract_obj(beacon);
    return TRUE;
}

bool
spell_beacon(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    /* Create a beacon, ready for a portal to 'goto'.
     * I owe someone thanks for this, but forgot the name... D'oh.
     * How this works: (+ for portals, etc )
     * a) Caster makes beacon: ownership set, obj->name = target_name
     * b) Someone goes off with beacon, drops it
     * c) Owner can then cast portal spell to it, beacon extracted
     * -S- */

    char                buf[MAX_STRING_LENGTH];
    char                arg[MAX_STRING_LENGTH];
    OBJ_DATA           *ob;

    one_argument(target_name, arg);

    ob = get_obj_world(ch, arg);
    if (ob != NULL) {
        send_to_char("There is already an object with that keyword.\n\r", ch);
        return FALSE;
    }

    ob = create_object(get_obj_index(OBJ_VNUM_BEACON), level);
    sprintf(buf, "%s", arg);
    free_string(ob->name);
    ob->name = str_dup(arg);

    /* bug fix: when a charmie creates a beacon, it acts as its masters
       beacon. this was being abused as one person made the beacon, and
       another person portaled to it because their charmies had the same
       ch->name -dave */

    if ((IS_NPC(ch))
        && (IS_AFFECTED(ch, AFF_CHARM))
        && (ch->master)
        )
        sprintf(buf, "%s", ch->master->name);
    else
        sprintf(buf, "%s", ch->name);

    free_string(ob->owner);
    ob->owner = str_dup(buf);
    ob->timer = number_range(30, 45);
    obj_to_room(ob, ch->in_room);
    act("$n magically produces $p!", ch, ob, NULL, TO_ROOM);
    act("You magically produce $p!", ch, ob, NULL, TO_CHAR);
    return TRUE;
}
