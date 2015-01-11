
/***************************************************************************
 * Avatar spells                                                           *
 ***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include "merc.h"
#include "duel.h"

IDSTRING(rcsid, "$Id $");

/* basic functions */
void energy_advance(CHAR_DATA *ch)
{
    int needed;

    if (IS_NPC(ch) || ch->pcdata->in_arena || find_duel(ch))
         return;

    needed = (int)(400 * pow(1.1, ch->pcdata->energy_level));

    if (ch->pcdata->energy_used >= needed) {
        ch->pcdata->energy_used -= needed;
        ch->pcdata->energy_level++;
        sendf(ch, "You advanced to energy level %d.\n\r", ch->pcdata->energy_level);
        save_char_obj(ch);
    }

    return;
}

int avatar_cost(int level)
{
    int cost;

    cost = 100 + (int)(100 * pow(1.03560211, level));

    return cost;
}

int avatarlimit_lookup(char *skill)
{
    int c;

    for (c = 0; avatarlimit_table[c].name[0] != '\0'; c++)
        if (!str_cmp(skill, avatarlimit_table[c].name))
            return avatarlimit_table[c].limit;

    return 0;
}

/* spell functions */

bool spell_avatar_default(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    send_to_char("You haven't learned this spell.\n\r", ch);
    return FALSE;
}

bool spell_tranquility_novice(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    if (ch->pcdata->safetimer > 0 || ch->pcdata->fighttimer > 0 || ch->pcdata->in_arena || find_duel(ch))
        return FALSE;

    ch->pcdata->safetimer = current_time + 10;
    send_to_char("You feel calm and tranquility.\n\r", ch);
    return TRUE;
}

bool spell_tranquility_intermediate(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    if (ch->pcdata->safetimer > 0 || ch->pcdata->fighttimer > 0 || ch->pcdata->in_arena || find_duel(ch))
        return FALSE;

    ch->pcdata->safetimer = current_time + 60;
    send_to_char("You feel calm and tranquility.\n\r", ch);
    return TRUE;
}

bool spell_tranquility_advanced(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    if (ch->pcdata->safetimer > 0 || ch->pcdata->fighttimer > 0 || ch->pcdata->in_arena || find_duel(ch))
        return FALSE;

    ch->pcdata->safetimer = current_time + 120;
    send_to_char("You feel calm and tranquility.\n\r", ch);
    return TRUE;
}

bool spell_tranquility_expert(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    if (ch->pcdata->safetimer > 0 || ch->pcdata->fighttimer > 0 || ch->pcdata->in_arena || find_duel(ch))
        return FALSE;

    ch->pcdata->safetimer = current_time + 180;
    send_to_char("You feel calm and tranquility.\n\r", ch);
    return TRUE;
}

bool spell_tranquility_master(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA *gch;

    if (ch->pcdata->safetimer > 0 || ch->pcdata->fighttimer > 0 || ch->pcdata->in_arena || find_duel(ch))
        return FALSE;

    for (gch = ch->in_room->first_person; gch != NULL; gch = gch->next_in_room) {
        if (!is_same_group(ch, gch) || IS_NPC(gch) || gch->pcdata->safetimer > 0 || gch->pcdata->fighttimer > 0 || gch->fighting != NULL)
            continue;

        act("$n suddenly feels great calm and tranquility.", gch, NULL, NULL, TO_ROOM);
        send_to_char("You feel great calm and tranquility.\n\r", gch);
        gch->pcdata->safetimer = current_time + 300;
    }

    return TRUE;
}

bool spell_smokescreen_novice(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    ROOM_INDEX_DATA    *room, *arearoom;
    BUILD_DATA_LIST    *pList = NULL;
    ROOM_AFFECT_DATA    raf;

    room = ch->in_room;

    if (room == NULL)
        return FALSE;

    if (IS_SET(room->affected_by, ROOM_BV_SMOKESCREEN)) {
        send_to_char("@@NThere is already a @@dSmokescreen@@N operating here!\n\r", ch);
        return FALSE;
    }

    pList = ch->in_room->area->first_area_room;

    if (pList == NULL || pList->data == NULL)
        return FALSE;

    arearoom = pList->data;

    if (IS_SET(arearoom->affected_by, ROOM_BV_SMOKESCREEN_AREA)) {
        send_to_char("@@NThere is already an @@dArea Smokescreen@@N operating in this area!\n\r", ch);
        return FALSE;
    }

    act("@@dA @@gSmokescreen@@d encompasses the room.@@N", ch, NULL, NULL, TO_ROOM);
    send_to_char("@@dYou fill the room with @@gSmoke@@d.@@N\n\r", ch);

    raf.type      = sn;
    raf.duration  = 1;
    raf.level     = level;
    raf.bitvector = ROOM_BV_SMOKESCREEN;
    raf.caster    = ch;
    raf.modifier  = 0;
    raf.name      = str_dup("");
    affect_to_room(room, &raf);

    return TRUE;
}

bool spell_smokescreen_intermediate(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    ROOM_INDEX_DATA    *room, *arearoom;
    BUILD_DATA_LIST    *pList = NULL;
    ROOM_AFFECT_DATA    raf;

    room = ch->in_room;

    if (room == NULL)
        return FALSE;

    if (IS_SET(room->affected_by, ROOM_BV_SMOKESCREEN)) {
        send_to_char("@@NThere is already a @@dSmokescreen@@N operating here!\n\r", ch);
        return FALSE;
    }

    pList = ch->in_room->area->first_area_room;

    if (pList == NULL || pList->data == NULL)
        return FALSE;

    arearoom = pList->data;

    if (IS_SET(arearoom->affected_by, ROOM_BV_SMOKESCREEN_AREA)) {
        send_to_char("@@NThere is already an @@dArea Smokescreen@@N operating in this area!\n\r", ch);
        return FALSE;
    }

    act("@@dA @@gSmokescreen@@d encompasses the room.@@N", ch, NULL, NULL, TO_ROOM);
    send_to_char("@@dYou fill the room with @@gSmoke@@d.@@N\n\r", ch);

    raf.type      = sn;
    raf.duration  = 4;
    raf.level     = level;
    raf.bitvector = ROOM_BV_SMOKESCREEN;
    raf.caster    = ch;
    raf.modifier  = 0;
    raf.name      = str_dup("");
    affect_to_room(room, &raf);

    return TRUE;
}

bool spell_smokescreen_advanced(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    ROOM_INDEX_DATA    *room, *arearoom;
    BUILD_DATA_LIST    *pList = NULL;
    ROOM_AFFECT_DATA    raf;

    room = ch->in_room;

    if (room == NULL)
        return FALSE;

    pList = ch->in_room->area->first_area_room;

    if (pList == NULL || pList->data == NULL)
        return FALSE;

    arearoom = pList->data;

    if (IS_SET(arearoom->affected_by, ROOM_BV_SMOKESCREEN_AREA)) {
        send_to_char("@@NThere is already an @@dArea Smokescreen@@N operating in this area!\n\r", ch);
        return FALSE;
    }

    if (IS_SET(room->affected_by, ROOM_BV_SMOKESCREEN)) {
        send_to_char("@@NThere is already a @@dSmokescreen@@N operating here!\n\r", ch);
        return FALSE;
    }

    act("@@dA @@gSmokescreen@@d encompasses the room.@@N", ch, NULL, NULL, TO_ROOM);
    send_to_char("@@dYou fill the room with @@gSmoke@@d.@@N\n\r", ch);

    raf.type      = sn;
    raf.duration  = 8;
    raf.level     = level;
    raf.bitvector = ROOM_BV_SMOKESCREEN;
    raf.caster    = ch;
    raf.modifier  = 0;
    raf.name      = str_dup("");
    affect_to_room(room, &raf);

    return TRUE;
}

bool spell_smokescreen_expert(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    ROOM_INDEX_DATA    *room, *arearoom;
    BUILD_DATA_LIST    *pList = NULL;
    ROOM_AFFECT_DATA    raf;

    room = ch->in_room;

    if (room == NULL)
        return FALSE;

    pList = ch->in_room->area->first_area_room;

    if (pList == NULL || pList->data == NULL)
        return FALSE;

    arearoom = pList->data;

    if (IS_SET(arearoom->affected_by, ROOM_BV_SMOKESCREEN_AREA)) {
        send_to_char("@@NThere is already an @@dArea Smokescreen@@N operating in this area!\n\r", ch);
        return FALSE;
    }

    if (IS_SET(room->affected_by, ROOM_BV_SMOKESCREEN)) {
        send_to_char("@@NThere is already a @@dSmokescreen@@N operating here!\n\r", ch);
        return FALSE;
    }

    act("@@dA @@gSmokescreen@@d encompasses the room.@@N", ch, NULL, NULL, TO_ROOM);
    send_to_char("@@dYou fill the room with @@gSmoke@@d.@@N\n\r", ch);

    raf.type      = sn;
    raf.duration  = 12;
    raf.level     = level;
    raf.bitvector = ROOM_BV_SMOKESCREEN;
    raf.caster    = ch;
    raf.modifier  = 0;
    raf.name      = str_dup("");
    affect_to_room(room, &raf);

    return TRUE;
}

bool spell_smokescreen_master(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    ROOM_INDEX_DATA    *arearoom;
    BUILD_DATA_LIST    *pList = NULL;
    ROOM_AFFECT_DATA    raf;

    pList = ch->in_room->area->first_area_room;

    if (pList == NULL || pList->data == NULL)
        return FALSE;

    arearoom = pList->data;

    if (IS_SET(arearoom->affected_by, ROOM_BV_SMOKESCREEN_AREA)) {
        send_to_char("@@NThere is already an @@dArea Smokescreen@@N operating in this area!\n\r", ch);
        return FALSE;
    }

    act("@@dA @@gSmokescreen@@d encompasses the area.@@N", ch, NULL, NULL, TO_ROOM);
    send_to_char("@@dYou fill the area with @@gSmoke@@d.@@N\n\r", ch);

    raf.type      = sn;
    raf.duration  = 20;
    raf.level     = level;
    raf.bitvector = ROOM_BV_SMOKESCREEN_AREA;
    raf.caster    = ch;
    raf.modifier  = 0;
    raf.name      = str_dup("");
    affect_to_room(arearoom, &raf);

    return TRUE;
}

bool spell_sentry_novice(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    ROOM_INDEX_DATA    *room;
    ROOM_AFFECT_DATA    raf;
    ROOM_AFFECT_DATA   *paf;

    room = ch->in_room;

    if (room == NULL)
        return FALSE;

    if (IS_SET(room->affected_by, ROOM_BV_SENTRY)) {
        for (paf = room->first_room_affect; paf; paf = paf->next) {
            if (paf->bitvector == ROOM_BV_SENTRY && paf->caster == ch) {
                send_to_char("@@NYou are already operating a @@dSentry@@N here!\n\r", ch);
                return FALSE;
            }
        }
    }

    if (ch->pcdata->runes >= 10) {
        send_to_char("You already have the maximum amount of rune/sentries.\n\r", ch);
        return FALSE;
    }

    if (*target_name == '\0') {
        send_to_char("You must give your sentry a name.\n\r", ch);
        return FALSE;
    }

    send_to_char("@@gYou set up a @@dSentry@@g.@@N\n\r", ch);

    raf.type      = sn;
    raf.duration  = 2;
    raf.level     = level;
    raf.bitvector = ROOM_BV_SENTRY;
    raf.caster    = ch;
    raf.modifier  = 0;
    raf.name      = str_dup(target_name);
    affect_to_room(room, &raf);

    return TRUE;
}

bool spell_sentry_intermediate(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    ROOM_INDEX_DATA    *room;
    ROOM_AFFECT_DATA    raf;
    ROOM_AFFECT_DATA   *paf;

    room = ch->in_room;

    if (room == NULL)
        return FALSE;

    if (IS_SET(room->affected_by, ROOM_BV_SENTRY)) {
        for (paf = room->first_room_affect; paf; paf = paf->next) {
            if (paf->bitvector == ROOM_BV_SENTRY && paf->caster == ch) {
                send_to_char("@@NYou are already operating a @@dSentry@@N here!\n\r", ch);
                return FALSE;
            }
        }
    }

    if (ch->pcdata->runes >= 10) {
        send_to_char("You already have the maximum amount of rune/sentries.\n\r", ch);
        return FALSE;
    }

    if (*target_name == '\0') {
        send_to_char("You must give your sentry a name.\n\r", ch);
        return FALSE;
    }

    send_to_char("@@gYou set up a @@dSentry@@g.@@N\n\r", ch);

    raf.type      = sn;
    raf.duration  = 5;
    raf.level     = level;
    raf.bitvector = ROOM_BV_SENTRY;
    raf.caster    = ch;
    raf.modifier  = 0;
    raf.name      = str_dup(target_name);
    affect_to_room(room, &raf);

    return TRUE;
}

bool spell_sentry_advanced(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    ROOM_INDEX_DATA    *room;
    ROOM_AFFECT_DATA    raf;
    ROOM_AFFECT_DATA   *paf;

    room = ch->in_room;

    if (room == NULL)
        return FALSE;

    if (IS_SET(room->affected_by, ROOM_BV_SENTRY)) {
        for (paf = room->first_room_affect; paf; paf = paf->next) {
            if (paf->bitvector == ROOM_BV_SENTRY && paf->caster == ch) {
                send_to_char("@@NYou are already operating a @@dSentry@@N here!\n\r", ch);
                return FALSE;
            }
        }
    }

    if (ch->pcdata->runes >= 10) {
        send_to_char("You already have the maximum amount of rune/sentries.\n\r", ch);
        return FALSE;
    }

    if (*target_name == '\0') {
        send_to_char("You must give your sentry a name.\n\r", ch);
        return FALSE;
    }

    send_to_char("@@gYou set up a @@dSentry@@g.@@N\n\r", ch);

    raf.type      = sn;
    raf.duration  = 10;
    raf.level     = level;
    raf.bitvector = ROOM_BV_SENTRY;
    raf.caster    = ch;
    raf.modifier  = 0;
    raf.name      = str_dup(target_name);
    affect_to_room(room, &raf);

    return TRUE;
}

bool spell_sentry_expert(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    ROOM_INDEX_DATA    *room;
    ROOM_AFFECT_DATA    raf;
    ROOM_AFFECT_DATA   *paf;

    room = ch->in_room;

    if (room == NULL)
        return FALSE;

    if (IS_SET(room->affected_by, ROOM_BV_SENTRY)) {
        for (paf = room->first_room_affect; paf; paf = paf->next) {
            if (paf->bitvector == ROOM_BV_SENTRY && paf->caster == ch) {
                send_to_char("@@NYou are already operating a @@dSentry@@N here!\n\r", ch);
                return FALSE;
            }
        }
    }

    if (ch->pcdata->runes >= 10) {
        send_to_char("You already have the maximum amount of rune/sentries.\n\r", ch);
        return FALSE;
    }

    if (*target_name == '\0') {
        send_to_char("You must give your sentry a name.\n\r", ch);
        return FALSE;
    }

    send_to_char("@@gYou set up a @@dSentry@@g.@@N\n\r", ch);

    raf.type      = sn;
    raf.duration  = 15;
    raf.level     = level;
    raf.bitvector = ROOM_BV_SENTRY;
    raf.caster    = ch;
    raf.modifier  = 0;
    raf.name      = str_dup(target_name);
    affect_to_room(room, &raf);

    return TRUE;
}

bool spell_sentry_master(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    ROOM_INDEX_DATA    *room;
    ROOM_AFFECT_DATA    raf;
    ROOM_AFFECT_DATA   *paf;

    room = ch->in_room;

    if (room == NULL)
        return FALSE;

    if (IS_SET(room->affected_by, ROOM_BV_SENTRY)) {
        for (paf = room->first_room_affect; paf; paf = paf->next) {
            if (paf->bitvector == ROOM_BV_SENTRY && paf->caster == ch) {
                send_to_char("@@NYou are already operating a @@dSentry@@N here!\n\r", ch);
                return FALSE;
            }
        }
    }

    if (ch->pcdata->runes >= 10) {
        send_to_char("You already have the maximum amount of rune/sentries.\n\r", ch);
        return FALSE;
    }

    if (*target_name == '\0') {
        send_to_char("You must give your sentry a name.\n\r", ch);
        return FALSE;
    }

    send_to_char("@@gYou set up a @@dSentry@@g.@@N\n\r", ch);

    raf.type      = sn;
    raf.duration  = 30;
    raf.level     = level;
    raf.bitvector = ROOM_BV_SENTRY;
    raf.caster    = ch;
    raf.modifier  = 0;
    raf.name      = str_dup(target_name);
    affect_to_room(room, &raf);

    return TRUE;
}

bool coldwave_cooldown(CHAR_DATA *ch, CHAR_DATA *victim, int max)
{
    OBJ_DATA *heated_item;
    DUEL_PLAYER_DATA   *player;
    DUEL_OBJ_DATA      *dobj = NULL, *dobj_next = NULL;
    DUEL_DATA          *duel;
    int coolcnt = 0;
    int coolmax;

    coolmax = max;
    duel = find_duel(victim);

    if (!duel) {
        for (heated_item = victim->first_carry; heated_item != NULL; heated_item = heated_item->next_in_carry_list) {
            if (heated_item->wear_loc == WEAR_NONE)
                continue;
            else {
                if (   IS_SET(heated_item->item_apply, ITEM_APPLY_HEATED)
                    || (!IS_NPC(victim) && victim->pcdata->in_arena && IS_SET(heated_item->item_apply, ITEM_APPLY_ARENAHEATED))) {
                    if (coolcnt >= coolmax)
                        break;

                    if (IS_SET(heated_item->item_apply, ITEM_APPLY_HEATED) && !victim->pcdata->in_arena) {
                        act("@@N@@g$n@@N@@g cools down @@N$p@@N@@g!@@N", ch, heated_item, victim, TO_NOTVICT);
                        if (ch != victim) {
                            act("@@N@@gYou cool down @@N$N@@N's @@N$p@@N@@g!@@N", ch, heated_item, victim, TO_CHAR);
                            act("@@N@@g$n cools down your @@N$p@@N@@g!@@N", ch, heated_item, victim, TO_VICT);
                        }
                        else
                            act("@@N@@gYou cool down @@N$p@@N@@g!@@N", ch, heated_item, NULL, TO_CHAR);

                        REMOVE_BIT(heated_item->item_apply, ITEM_APPLY_HEATED);
                        coolcnt++;
                    }
                    else if (IS_SET(heated_item->item_apply, ITEM_APPLY_ARENAHEATED) && victim->pcdata->in_arena) {
                        act("@@N@@g$n@@N@@g cools down @@N$p@@N@@g!@@N", ch, heated_item, victim, TO_NOTVICT);
                        if (ch != victim) {
                            act("@@N@@gYou cool down @@N$N@@N's @@N$p@@N@@g!@@N", ch, heated_item, victim, TO_CHAR);
                            act("@@N@@g$n cools down your @@N$p@@N@@g!@@N", ch, heated_item, victim, TO_VICT);
                        }
                        else
                            act("@@N@@gYou cool down @@N$p@@N@@g!@@N", ch, heated_item, NULL, TO_CHAR);

                        REMOVE_BIT(heated_item->item_apply, ITEM_APPLY_ARENAHEATED);
                        coolcnt++;
                    }
                }
            }
        }
    }

    if (   duel
        && (player = find_duel_player(victim))
        && duel->stage == DUEL_STAGE_GO) {
        for (dobj = player->first_obj; dobj != NULL; dobj = dobj_next) {
            dobj_next = dobj->next;

            if (dobj->obj && dobj->obj->wear_loc != WEAR_NONE && coolcnt < coolmax) {
                act("@@N@@gYour @@N$p@@N @@acools off@@N!!", player->ch, dobj->obj, NULL, TO_CHAR);
                DUNLINK(dobj, player->first_obj, player->last_obj, next, prev);
                DESTROY_MEMBER(dobj);
                coolcnt++;
            }
        }
    }

    if (coolcnt > 0)
        return TRUE;
    else
        return FALSE;
}

bool spell_coldwave_novice(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    return coldwave_cooldown(ch, ch, 1);
}

bool spell_coldwave_intermediate(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    return coldwave_cooldown(ch, ch, 2);
}

bool spell_coldwave_advanced(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    return coldwave_cooldown(ch, ch, 3);
}

bool spell_coldwave_expert(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    return coldwave_cooldown(ch, ch, 4);
}

bool spell_coldwave_master(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    bool success = FALSE, success2 = FALSE;
    CHAR_DATA *gch;

    success = coldwave_cooldown(ch, ch, 5);

    for (gch = ch->in_room->first_person; gch != NULL; gch = gch->next_in_room) {
        if (!is_same_group(ch, gch))
            continue;

        success2 = coldwave_cooldown(ch, gch, 2);
        if (success2)
            success = success2;
    }

    return success;
}

void do_stealth(CHAR_DATA *ch, char *argument)
{
    bool best = FALSE;
    int sn = gsn_stealth;
    char *type = NULL;

    if (IS_NPC(ch))
        return;

    if (*argument == '\0')
        best = TRUE;
    else if (!str_prefix("novice", argument)) {
        sn = gsn_stealth_novice;
        type = "novice";
    }
    else if (!str_prefix("intermediate", argument)) {
        sn = gsn_stealth_intermediate;
        type = "intermediate";
    }
    else if (!str_prefix("advanced", argument)) {
        sn = gsn_stealth_advanced;
        type = "advanced";
    }
    else if (!str_prefix("expert", argument)) {
        sn = gsn_stealth_expert;
        type = "expert";
    }
    else if (!str_prefix("master", argument)) {
        sn = gsn_stealth_master;
        type = "master";
    }
    else if (!str_prefix("off", argument) && ch->pcdata->stealth > 0) {
        send_to_char("Stealth disabled. Reverting to normal stance ninja.\n\r", ch);
        ch->pcdata->stealth = 0;
        return;
    }
    else {
        send_to_char("Syntax: stealth [novice|intermediate|advanced|expert|master|off]\n\r", ch);
        return;
    }

    if (best) {
        if (ch->pcdata->learned[sn] >= AV_NOVICE && ch->pcdata->learned[sn] < AV_INTERMEDIATE) {
            sn += 1; type = "novice";
        }
        else if (ch->pcdata->learned[sn] >= AV_INTERMEDIATE && ch->pcdata->learned[sn] < AV_ADVANCED) {
            sn += 2; type = "intermediate";
        }
        else if (ch->pcdata->learned[sn] >= AV_ADVANCED && ch->pcdata->learned[sn] < AV_EXPERT) {
            sn += 3; type = "advanced";
        }
        else if (ch->pcdata->learned[sn] >= AV_EXPERT && ch->pcdata->learned[sn] < AV_MASTER) {
            sn += 4; type = "expert";
        }
        else if (ch->pcdata->learned[sn] >= AV_MASTER) {
            sn += 5; type = "master";
        }
        else {
           sn += 1; type = "novice";
        }
    }

    if (ch->pcdata->learned[sn] == 0) {
        sendf(ch, "You haven't learned %s stealth.\n\r", type);
        return;
    }

    sendf(ch, "Using %s stealth. Use the Ninja stance to engage!\n\r", type);
    ch->pcdata->stealth = sn;
    return;
}

bool spell_innerflame_novice(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    AFFECT_DATA         af;

    if (   is_affected(ch, gsn_innerflame_novice)
        || is_affected(ch, gsn_innerflame_intermediate)
        || is_affected(ch, gsn_innerflame_advanced)
        || is_affected(ch, gsn_innerflame_expert)
        || is_affected(ch, gsn_innerflame_master))
        return FALSE;

    af.type = sn;
    af.duration = 2;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = 0;
    af.save = TRUE;
    affect_to_char(ch, &af);

    act("You concentrate on your inner flame.", ch, NULL, NULL, TO_CHAR);
    act("$n concentrates on $s inner flame.", ch, NULL, NULL, TO_ROOM);

    return TRUE;

}

bool spell_innerflame_intermediate(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    AFFECT_DATA         af;

    if (   is_affected(ch, gsn_innerflame_novice)
        || is_affected(ch, gsn_innerflame_intermediate)
        || is_affected(ch, gsn_innerflame_advanced)
        || is_affected(ch, gsn_innerflame_expert)
        || is_affected(ch, gsn_innerflame_master))
        return FALSE;

    af.type = sn;
    af.duration = 4;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = 0;
    af.save = TRUE;
    affect_to_char(ch, &af);

    act("You concentrate on your inner flame.", ch, NULL, NULL, TO_CHAR);
    act("$n concentrates on $s inner flame.", ch, NULL, NULL, TO_ROOM);

    return TRUE;

}

bool spell_innerflame_advanced(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    AFFECT_DATA         af;

    if (   is_affected(ch, gsn_innerflame_novice)
        || is_affected(ch, gsn_innerflame_intermediate)
        || is_affected(ch, gsn_innerflame_advanced)
        || is_affected(ch, gsn_innerflame_expert)
        || is_affected(ch, gsn_innerflame_master))
        return FALSE;

    af.type = sn;
    af.duration = 6;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = 0;
    af.save = TRUE;
    affect_to_char(ch, &af);

    act("You concentrate on your inner flame.", ch, NULL, NULL, TO_CHAR);
    act("$n concentrates on $s inner flame.", ch, NULL, NULL, TO_ROOM);

    return TRUE;

}

bool spell_innerflame_expert(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    AFFECT_DATA         af;

    if (   is_affected(ch, gsn_innerflame_novice)
        || is_affected(ch, gsn_innerflame_intermediate)
        || is_affected(ch, gsn_innerflame_advanced)
        || is_affected(ch, gsn_innerflame_expert)
        || is_affected(ch, gsn_innerflame_master))
        return FALSE;

    af.type = sn;
    af.duration = 10;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = 0;
    af.save = TRUE;
    affect_to_char(ch, &af);

    act("You concentrate on your inner flame.", ch, NULL, NULL, TO_CHAR);
    act("$n concentrates on $s inner flame.", ch, NULL, NULL, TO_ROOM);

    return TRUE;

}

bool spell_innerflame_master(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    AFFECT_DATA         af;

    if (   is_affected(ch, gsn_innerflame_novice)
        || is_affected(ch, gsn_innerflame_intermediate)
        || is_affected(ch, gsn_innerflame_advanced)
        || is_affected(ch, gsn_innerflame_expert)
        || is_affected(ch, gsn_innerflame_master))
        return FALSE;

    af.type = sn;
    af.duration = 15;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = 0;
    af.save = TRUE;
    affect_to_char(ch, &af);

    act("You concentrate on your inner flame.", ch, NULL, NULL, TO_CHAR);
    act("$n concentrates on $s inner flame.", ch, NULL, NULL, TO_ROOM);

    return TRUE;

}
