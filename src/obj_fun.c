
/***************************************************************************
 *                                                                         *
 *       _/          _/_/_/     _/    _/     _/    ACK! MUD is modified    *
 *      _/_/        _/          _/  _/       _/    Merc2.0/2.1/2.2 code    *
 *     _/  _/      _/           _/_/         _/    (c)Stephen Dooley 1994  *
 *    _/_/_/_/      _/          _/  _/             "This mud has not been  *
 *   _/      _/      _/_/_/     _/    _/     _/      tested on animals."   *
 *                                                                         *
 *                                                                         *
 * OBJ_FUN: Like special.c, but for objects, basically.               *
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

IDSTRING(rcsid, "$Id: obj_fun.c,v 1.7 2003/11/12 01:57:17 dave Exp $");

DECLARE_OBJ_FUN(objfun_giggle);    /* test obj_fun  */
DECLARE_OBJ_FUN(objfun_cast_fight);    /* Casts in fights   */
DECLARE_OBJ_FUN(objfun_sword_aggro);    /* starts fights */
DECLARE_OBJ_FUN(objfun_soul_moan);    /* moaning souls */
DECLARE_OBJ_FUN(objfun_infused_soul);    /* objs with a soul in them */
DECLARE_OBJ_FUN(objfun_flaming);    /* test obj_fun  */
DECLARE_OBJ_FUN(objfun_healing);    /* test obj_fun  */
DECLARE_OBJ_FUN(objfun_dispeller);    /* test obj_fun  */
DECLARE_OBJ_FUN(objfun_regen);    /* test obj_fun  */

OBJ_FUN            *
obj_fun_lookup(const char *name)
{
    if (!str_cmp(name, "objfun_giggle"))
        return (OBJ_FUN *) objfun_giggle;
    if (!str_cmp(name, "objfun_cast_fight"))
        return (OBJ_FUN *) objfun_cast_fight;
    if (!str_cmp(name, "objfun_sword_aggro"))
        return (OBJ_FUN *) objfun_sword_aggro;
    if (!str_cmp(name, "objfun_soul_moan"))
        return (OBJ_FUN *) objfun_soul_moan;
    if (!str_cmp(name, "objfun_infused_soul"))
        return (OBJ_FUN *) objfun_infused_soul;
    if (!str_cmp(name, "objfun_flaming"))
        return (OBJ_FUN *) objfun_flaming;
    if (!str_cmp(name, "objfun_healing"))
        return (OBJ_FUN *) objfun_healing;
    if (!str_cmp(name, "objfun_dispeller"))
        return (OBJ_FUN *) objfun_dispeller;
    if (!str_cmp(name, "objfun_regen"))
        return (OBJ_FUN *) objfun_regen;

    return 0;
}

char               *
rev_obj_fun_lookup(void *func)
{
    if (func == (void *) objfun_giggle)
        return "objfun_giggle";
    if (func == (void *) objfun_cast_fight)
        return "objfun_cast_fight";
    if (func == (void *) objfun_sword_aggro)
        return "objfun_sword_aggro";
    if (func == (void *) objfun_soul_moan)
        return "objfun_soul_moan";
    if (func == (void *) objfun_infused_soul)
        return "objfun_infused_soul";
    if (func == (void *) objfun_flaming)
        return "objfun_flaming";
    if (func == (void *) objfun_healing)
        return "objfun_healing";
    if (func == (void *) objfun_dispeller)
        return "objfun_dispeller";
    if (func == (void *) objfun_regen)
        return "objfun_regen";

    return 0;
}

char               *
rev_obj_fun_lookup_nice(void *func)
{
    /* This object %s. */
    if (func == (void *) objfun_giggle)
        return "Giggles";
    if (func == (void *) objfun_cast_fight)
        return "Casts in Combat";
    if (func == (void *) objfun_sword_aggro)
        return "is Aggressive";
    if (func == (void *) objfun_soul_moan)
        return "has a Moaning Soul";
    if (func == (void *) objfun_infused_soul)
        return "has an Infused Soul";
    if (func == (void *) objfun_flaming)
        return "is Flaming";
    if (func == (void *) objfun_healing)
        return "is Healing";
    if (func == (void *) objfun_dispeller)
        return "is Dispelling";
    if (func == (void *) objfun_regen)
        return "Regenerates";

    return "";
}

void
print_obj_fun_lookup(char *buf)
{
    strcat(buf, " objfun_giggle \n\r");
    strcat(buf, " objfun_cast_fight    \n\r");
    strcat(buf, " objfun_sword_aggro \n\r");
    strcat(buf, " objfun_soul_moan \n\r  ");
    strcat(buf, " objfun_infused_soul \n\r  ");
    strcat(buf, " objfun_flaming \n\r");
    strcat(buf, " objfun_healing \n\r  ");
    strcat(buf, " objfun_dispeller \n\r  ");
    strcat(buf, " objfun_regen \n\r  ");

    return;
}

/********************* OBJFUN FUNCTIONS ********************************/

void
objfun_giggle(OBJ_DATA *obj, CHAR_DATA *keeper)
{
    if (keeper == NULL || keeper->in_room == NULL)
        return;

    if (!IS_NPC(keeper)
        && (keeper->stance == STANCE_AMBUSH || keeper->stance == STANCE_AC_BEST)
        )
        return;

    /* Come on... it was SO annoying! */

    if (number_percent() < 5) {
        act("$O$p carried by $n starts giggling to itself!", keeper, obj, NULL, TO_ROOM);
        act("$O$p carried by you starts giggling to itself!", keeper, obj, NULL, TO_CHAR);
        return;
    }
    return;
}

void
objfun_soul_moan(OBJ_DATA *obj, CHAR_DATA *keeper)
{
    if (keeper == NULL || keeper->in_room == NULL)
        return;

    if (!IS_NPC(keeper)
        && (keeper->stance == STANCE_AMBUSH || keeper->stance == STANCE_AC_BEST)
        )
        return;

    /* Come on... it was SO annoying! */
    if (number_percent() < 2) {
        act("$O@@NThe @@eSoul@@N in $p@@N carried by @@a$n@@N moans in agony.", keeper, obj, NULL, TO_ROOM);
        act("$O@@NThe @@eSoul@@N in $p @@Ncarried by you moans to be set free!", keeper, obj, NULL, TO_CHAR);
        return;
    }

    if (number_percent() < 2) {
        act("$O@@NThe @@eSoul@@N in $p@@N carried by @@a$n@@N tries to break free of its inprisonment!", keeper, obj, NULL, TO_ROOM);
        act("$O@@NThe @@eSoul@@N in $p @@Ncarried by you starts writhing--look out!!", keeper, obj, NULL, TO_CHAR);
        return;
    }

    return;
}

void
objfun_infused_soul(OBJ_DATA *obj, CHAR_DATA *keeper)
{

    int                 sn;
    CHAR_DATA          *victim;
    int                 min_lev;
    char               *spell = "";

    if (keeper == NULL || keeper->in_room == NULL)
        return;
    if ((keeper == NULL)
        || (obj->item_type != ITEM_WEAPON))
        return;

    if ((get_eq_char(keeper, WEAR_WIELD) != obj)
        && (get_eq_char(keeper, WEAR_WIELD_2) != obj))
        return;                    /* Must be wielded to work */

    if (keeper->fighting != NULL) {

        for (victim = keeper->in_room->first_person; victim != NULL; victim = victim->next_in_room)
            if (victim->fighting == keeper && number_bits(1) == 0)
                break;

        if ((victim == NULL)
            || (victim->is_free != FALSE))
            return;

        switch (number_range(0, 3)) {
            case 0:
                min_lev = 43;
                spell = "throwing star";
                break;
            case 1:
                min_lev = 55;
                spell = "acid blast";
                break;
            case 2:
                min_lev = 68;
                spell = "dispel magic";
                break;
            case 3:
                min_lev = 55;
                spell = "flamestrike";
                break;

        }
        if ((sn = skill_lookup(spell)) < 0)
            return;
        act("$p glows brightly at $n!", victim, obj, NULL, TO_ROOM);
        act("$p glows brightly at you!", victim, obj, NULL, TO_CHAR);

        if ((!IS_NPC(keeper))
            && (!IS_NPC(victim))
            && (keeper->fighting == victim)
            )
            obj_cast_spell(sn, obj->level, keeper, victim, obj);
        return;
    }
    else {
        if (!IS_NPC(keeper)
            && (keeper->stance == STANCE_AMBUSH || keeper->stance == STANCE_AC_BEST)
            )
            return;

        /* Come on... it was SO annoying! */
        if (number_percent() < 2) {
            act("$O@@NThe @@eSoul@@N in $p@@N carried by @@a$n@@N moans in agony.", keeper, obj, NULL, TO_ROOM);
            act("$O@@NThe @@eSoul@@N in $p @@Ncarried by you moans to be set free!", keeper, obj, NULL, TO_CHAR);
            return;
        }

        if (number_percent() < 2) {
            act("$O@@NThe @@eSoul@@N in $p@@N carried by @@a$n@@N tries to break free of its inprisonment!", keeper, obj, NULL, TO_ROOM);
            act("$O@@NThe @@eSoul@@N in $p @@Ncarried by you starts writhing--look out!!", keeper, obj, NULL, TO_CHAR);
            return;
        }
    }
    return;
}

void
objfun_cast_fight(OBJ_DATA *obj, CHAR_DATA *keeper)
{
    int                 sn;
    CHAR_DATA          *victim;
    int                 min_lev;
    char               *spell = "";

    if (keeper == NULL || (keeper->fighting == NULL)
        || (obj->item_type != ITEM_WEAPON))
        return;
    if ((get_eq_char(keeper, WEAR_WIELD) != obj)
        && (get_eq_char(keeper, WEAR_WIELD_2) != obj))
        return;                    /* Must be wielded to work */
    for (victim = keeper->in_room->first_person; victim != NULL; victim = victim->next_in_room)
        if (victim->fighting == keeper && number_bits(2) == 0)
            break;

    if ((victim == NULL)
        || (victim->is_free != FALSE))
        return;
    switch (number_range(0, 5)) {
        case 0:
            min_lev = 5;
            spell = "magic missile";
            break;
        case 1:
            min_lev = 7;
            spell = "colour spray";
            break;
        case 2:
            min_lev = 8;
            spell = "chill touch";
            break;
        case 3:
            min_lev = 30;
            spell = "fireball";
            break;
        case 4:
            min_lev = 55;
            spell = "flamestrike";
            break;
        case 5:
            min_lev = 30;
            spell = "lightning bolt";
            break;
    }
    if ((sn = skill_lookup(spell)) < 0)
        return;
    act("$p glows brightly at $n!", victim, obj, NULL, TO_ROOM);
    act("$p glows brightly at you!", victim, obj, NULL, TO_CHAR);
    obj_cast_spell(sn, obj->level, keeper, victim, obj);
    return;
}

void
objfun_sword_aggro(OBJ_DATA *obj, CHAR_DATA *keeper)
{
    /* Weapon 'draws' an aggro mob's attention to the player */
    /* If fighting,  make cast spells? */
    CHAR_DATA          *vch;

    if (obj->item_type != ITEM_WEAPON)
        return;
    if ((keeper == NULL) || (obj == NULL))
        return;

    if ((get_eq_char(keeper, WEAR_WIELD) != obj)
        && (get_eq_char(keeper, WEAR_WIELD_2) != obj))
        return;
    if (keeper == NULL || keeper->fighting == NULL)
        return;

    for (vch = keeper->in_room->first_person; vch != NULL; vch = vch->next_in_room) {
        if (IS_NPC(vch)
            && (vch->level > keeper->level)
            && IS_SET(vch->act, ACT_AGGRESSIVE)
            && vch->fighting == NULL && number_bits(4) == 0) {
            act("$p carried by $n glows in $s hands.", keeper, obj, NULL, TO_ROOM);
            act("$p carried by you glows in your hands.", keeper, obj, NULL, TO_CHAR);
            act("$p says 'LOOK! LOOK!  $n is here!!'", keeper, obj, NULL, TO_ROOM);
            act("$p says 'LOOK! LOOK!  $n is here!!'", keeper, obj, NULL, TO_CHAR);
            multi_hit(vch, keeper, TYPE_UNDEFINED);
            break;
        }
    }
    return;
}

void
objfun_flaming(OBJ_DATA *obj, CHAR_DATA *keeper)
{
    int                 sn;
    CHAR_DATA          *victim;
    int                 min_lev;
    char               *spell = "";

    if (keeper == NULL || keeper->in_room == NULL)
        return;
    if ((keeper == NULL)
        || (obj->item_type != ITEM_WEAPON))
        return;

    for (victim = keeper->in_room->first_person; victim != NULL; victim = victim->next_in_room)
        if (victim->fighting == keeper && number_bits(2) == 0)
            break;

    if (victim == NULL) {
        if (!IS_NPC(keeper)
            && (keeper->stance == STANCE_AMBUSH || keeper->stance == STANCE_AC_BEST)
            )
            return;

        if (number_percent() < 2) {
            act("$O@@N$p@@N carried by @@a$n@@e flames @@ybrightly@@N!", keeper, obj, NULL, TO_ROOM);
            act("$O@@N$p @@Ncarried by you@@e flames @@ybrightly@@N!", keeper, obj, NULL, TO_CHAR);
            return;
        }

        if (number_percent() < 2) {
            act("$O@@N$p@@N carried by @@a$n@@N goes @@ddim@@N.", keeper, obj, NULL, TO_ROOM);
            act("$O@@N$p @@Ncarried by you@@N goes @@ddim@@N.", keeper, obj, NULL, TO_CHAR);
            return;
        }
        return;
    }
    if ((victim == NULL)
        || (victim->is_free != FALSE))
        return;

    if ((get_eq_char(keeper, WEAR_WIELD) != obj)
        && (get_eq_char(keeper, WEAR_WIELD_2) != obj))
        return;                    /* Must be wielded to work */

    switch (number_range(0, 10)) {
        case 0:
        case 1:
        case 2:
        case 3:
            min_lev = 5;
            spell = "fireball";
            break;
        case 4:
        case 5:
        case 6:
        case 7:
            min_lev = 7;
            spell = "flamestrike";
            break;
        case 8:
        case 9:
            min_lev = 8;
            spell = "heat armor";
            break;
        case 10:
            min_lev = 25;
            spell = "heat armor";
            break;

    }
    if ((sn = skill_lookup(spell)) < 0)
        return;
    act("$p flames at $n!", victim, obj, NULL, TO_ROOM);
    act("$p flames brightly at you!", victim, obj, NULL, TO_CHAR);
    obj_cast_spell(sn, obj->level, keeper, victim, obj);
    return;
}

void
objfun_healing(OBJ_DATA *obj, CHAR_DATA *keeper)
{
    int                 sn;
    CHAR_DATA          *victim;
    int                 min_lev;
    char               *spell = "";

    if (keeper == NULL || keeper->in_room == NULL)
        return;
    if ((keeper == NULL)
        || (obj->item_type != ITEM_WEAPON))
        return;

    for (victim = keeper->in_room->first_person; victim != NULL; victim = victim->next_in_room)
        if (victim->fighting == keeper && number_bits(1) == 0)
            break;

    if (victim == NULL) {
        if (!IS_NPC(keeper)
            && (keeper->stance == STANCE_AMBUSH || keeper->stance == STANCE_AC_BEST)
            )
            return;

        if (number_percent() < 2) {
            act("$O@@N$p@@N carried by @@a$n@@y glows@@N with a @@mHoly @@Wlight@@N!", keeper, obj, NULL, TO_ROOM);
            act("$O@@N$p @@Ncarried by you@@y glows@@N with a @@mHoly @@Wlight@@N!", keeper, obj, NULL, TO_CHAR);
            return;
        }

        if (number_percent() < 2) {
            act("$O@@N$p@@N carried by @@a$n@@N sings a @@mhymn@@N.", keeper, obj, NULL, TO_ROOM);
            act("$O@@N$p @@Ncarried by you@@N sings a hymn@@N.", keeper, obj, NULL, TO_CHAR);
            return;
        }
        return;
    }
    if ((get_eq_char(keeper, WEAR_WIELD) != obj)
        && (get_eq_char(keeper, WEAR_WIELD_2) != obj))
        return;                    /* Must be wielded to work */
    switch (number_range(0, 11)) {
        case 0:
        case 1:
            min_lev = 5;
            spell = "cure light";
            break;
        case 2:
        case 3:
        case 4:
            min_lev = 7;
            spell = "cure serious";
            break;
        case 5:
        case 6:
        case 7:
            min_lev = 8;
            spell = "cure critical";
            break;
        case 8:
        case 9:
        case 10:
        case 11:
            min_lev = 30;
            spell = "heal";
            break;

    }
    if ((sn = skill_lookup(spell)) < 0)
        return;
    act("$p blankets $n @@Win a @@mhealing @@Waura@@N!", keeper, obj, NULL, TO_ROOM);
    act("$p blankets you in a @@mhealing @@Waura@@N!", keeper, obj, NULL, TO_CHAR);
    obj_cast_spell(sn, obj->level, keeper, keeper, obj);
    return;
}

void
objfun_dispeller(OBJ_DATA *obj, CHAR_DATA *keeper)
{
    int                 sn;
    CHAR_DATA          *victim;
    int                 min_lev;
    char               *spell = "";

    if (keeper == NULL || keeper->in_room == NULL)
        return;
    if ((keeper == NULL)
        || (obj->item_type != ITEM_WEAPON))
        return;

    for (victim = keeper->in_room->first_person; victim != NULL; victim = victim->next_in_room)
        if (victim->fighting == keeper && number_bits(2) == 0)
            break;

    if (victim == NULL) {
        if (!IS_NPC(keeper)
            && (keeper->stance == STANCE_AMBUSH || keeper->stance == STANCE_AC_BEST)
            )
            return;

        if (number_percent() < 2) {
            act("$O@@N$p@@N carried by @@a$n@@a vibrates @@Nsuddenly@@N!", keeper, obj, NULL, TO_ROOM);
            act("$O@@N$p @@Ncarried by you@@a vibrates @@Nsuddenly@@N!", keeper, obj, NULL, TO_CHAR);
            return;
        }

        if (number_percent() < 2) {
            act("$O@@N$p@@N carried by @@a$n@@a peers@@N at your inventory@@N.", keeper, obj, NULL, TO_ROOM);
            act("$O@@N$p @@Ncarried by you@@a peers@@N around the room@@N.", keeper, obj, NULL, TO_CHAR);
            return;
        }
        return;
    }
    if ((victim == NULL)
        || (victim->is_free != FALSE))
        return;

    if ((get_eq_char(keeper, WEAR_WIELD) != obj)
        && (get_eq_char(keeper, WEAR_WIELD_2) != obj))
        return;                    /* Must be wielded to work */

    switch (number_range(0, 11)) {
        case 0:
        case 1:
        case 2:
        case 3:
            min_lev = 5;
            spell = "ego whip";
            break;
        case 4:
        case 5:
        case 6:
            min_lev = 7;
            spell = "bloody tears";
            break;
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
            min_lev = 30;
            spell = "dispel magic";
            break;

    }
    if ((sn = skill_lookup(spell)) < 0)
        return;
    act("$p @@apeers@@N deeply into the mind of $n!", victim, obj, NULL, TO_ROOM);
    act("$p @@apeers @@Ninto your thoughts!", victim, obj, NULL, TO_CHAR);
    obj_cast_spell(sn, obj->level, keeper, victim, obj);
    return;
}

void
objfun_regen(OBJ_DATA *obj, CHAR_DATA *keeper)
{
    if (keeper == NULL || keeper->in_room == NULL)
        return;

    if (obj->wear_loc < 0)
        return;
    keeper->hit = UMIN(keeper->max_hit, keeper->hit + (number_range(obj->level / 20, obj->level / 5)));
    return;
}
