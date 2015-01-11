
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
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include "merc.h"
#include "tables.h"
#include "duel.h"
#include "auction.h"

IDSTRING(rcsid, "$Id: act_info.c,v 1.180 2005/01/19 23:43:08 dave Exp $");

extern bool         dbl_xp;
extern bool         wizlock;
extern bool         nopk;

extern void bust_a_prompt(DESCRIPTOR_DATA *d, bool preview);
extern int avatar_cost(int level);

char               *const where_name[] = {
    "@@N<@@Wused as light@@N>     ",
    "<@@Wworn on finger@@N>    ",
    "<@@Wworn on finger@@N>    ",
    "<@@Wworn around neck@@N>  ",
    "<@@Wworn around neck@@N>  ",
    "<@@Wworn on body@@N>      ",
    "<@@Wworn on head@@N>      ",
    "<@@Wworn on legs@@N>      ",
    "<@@Wworn on feet@@N>      ",
    "<@@Wworn on hands@@N>     ",
    "<@@Wworn on arms@@N>      ",
    "<@@Wworn as shield@@N>    ",
    "<@@Wworn about body@@N>   ",
    "<@@Wworn about waist@@N>  ",
    "<@@Wworn around wrist@@N> ",
    "<@@Wworn around wrist@@N> ",
    "<@@Wwielded@@N>           ",
    "<@@Wheld@@N>              ",
    "<@@Wworn on face@@N>      ",
    "<@@Wworn as earring@@N>   ",
    "<@@Wclutched@@N>          ",
    "<@@Wdual wielded@@N>      ",
    "<@@Wclan token@@N>        ",
};

char const         *notwhere_name[] = {
    "@@N@@g<@@dused as light@@g>     ",
    "@@g<@@dworn on finger@@g>    ",
    "@@g<@@dworn on finger@@g>    ",
    "@@g<@@dworn around neck@@g>  ",
    "@@g<@@dworn around neck@@g>  ",
    "@@g<@@dworn on body@@g>      ",
    "@@g<@@dworn on head@@g>      ",
    "@@g<@@dworn on legs@@g>      ",
    "@@g<@@dworn on feet@@g>      ",
    "@@g<@@dworn on hands@@g>     ",
    "@@g<@@dworn on arms@@g>      ",
    "@@g<@@dworn as shield@@g>    ",
    "@@g<@@dworn about body@@g>   ",
    "@@g<@@dworn about waist@@g>  ",
    "@@g<@@dworn around wrist@@g> ",
    "@@g<@@dworn around wrist@@g> ",
    "@@g<@@dwielded@@g>           ",
    "@@g<@@dheld@@g>              ",
    "@@g<@@dworn on face@@g>      ",
    "@@g<@@dworn as earring@@g>   ",
    "@@g<@@dclutched@@g>          ",
    "@@g<@@ddual wielded@@g>      ",
    "@@g<@@dclan token@@g>        ",
};

char const         *where_name_short[] = {
    "@@N@@W light@@g ",
    "@@Wfinger@@g ",
    "@@Wfinger@@g ",
    "@@W  neck@@g ",
    "@@W  neck@@g ",
    "@@W  body@@g ",
    "@@W  head@@g ",
    "@@W  legs@@g ",
    "@@W  feet@@g ",
    "@@W hands@@g ",
    "@@W  arms@@g ",
    "@@Wshield@@g ",
    "@@W about@@g ",
    "@@W waist@@g ",
    "@@W wrist@@g ",
    "@@W wrist@@g ",
    "@@W wield@@g ",
    "@@W  held@@g ",
    "@@W  face@@g ",
    "@@W   ear@@g ",
    "@@Wclutch@@g ",
    "@@W  dual@@g ",
    "@@W token@@g ",
};

char const         *notwhere_name_short[] = {
    "@@N@@d light@@g ",
    "@@dfinger@@g ",
    "@@dfinger@@g ",
    "@@d  neck@@g ",
    "@@d  neck@@g ",
    "@@d  body@@g ",
    "@@d  head@@g ",
    "@@d  legs@@g ",
    "@@d  feet@@g ",
    "@@d hands@@g ",
    "@@d  arms@@g ",
    "@@dshield@@g ",
    "@@d about@@g ",
    "@@d waist@@g ",
    "@@d wrist@@g ",
    "@@d wrist@@g ",
    "@@d wield@@g ",
    "@@d  held@@g ",
    "@@d  face@@g ",
    "@@d   ear@@g ",
    "@@dclutch@@g ",
    "@@d  dual@@g ",
    "@@d token@@g ",
};

/*
 * Local functions.
 */
char               *format_obj_to_char args((OBJ_DATA *obj, CHAR_DATA *ch, bool fShort));
void show_list_to_char args((OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing));
void show_char_to_char_0 args((CHAR_DATA *victim, CHAR_DATA *ch));
void show_char_to_char_1 args((CHAR_DATA *victim, CHAR_DATA *ch));
void show_char_to_char args((CHAR_DATA *list, CHAR_DATA *ch, bool automode));
bool check_blind    args((CHAR_DATA *ch));
long int custom_exp_to_level(int lvl[5], int lvl2[5], int class, int index);

extern bool can_save args((CHAR_DATA *ch, OBJ_DATA *obj));
extern char        *percbar args((int curhp, int maxhp, int width));

char               *
format_obj_to_char(OBJ_DATA *obj, CHAR_DATA *ch, bool fShort)
{
    static char         buf[MAX_STRING_LENGTH];

    sprintf(buf, "%s", colour_string(ch, "objects"));
    if (IS_SET(obj->item_apply, ITEM_APPLY_HEATED))
        safe_strcat(MSL, buf, "@@y(HOT)@@N ");

    if (IS_OBJ_STAT(obj, ITEM_INVIS))
        safe_strcat(MAX_STRING_LENGTH, buf, "@@g(Invis) ");

    if ((IS_AFFECTED(ch, AFF_DETECT_EVIL) || item_has_apply(ch, ITEM_APPLY_DET_EVIL))
        && IS_OBJ_STAT(obj, ITEM_EVIL))
        safe_strcat(MAX_STRING_LENGTH, buf, "@@d(@@eR@@ee@@ed @@eA@@Rur@@ea@@d)@@N ");

    if ((IS_AFFECTED(ch, AFF_DETECT_MAGIC) || item_has_apply(ch, ITEM_APPLY_DET_MAG))
        && IS_OBJ_STAT(obj, ITEM_MAGIC))
        safe_strcat(MAX_STRING_LENGTH, buf, "@@d(@@GM@@ra@@Gg@@ri@@Gc@@ra@@Gl@@d)@@N ");

    if (IS_OBJ_STAT(obj, ITEM_GLOW))
        safe_strcat(MAX_STRING_LENGTH, buf, "@@e(@@yG@@Wl@@go@@Ww@@gi@@Wn@@yg@@e)@@N ");

    if (IS_OBJ_STAT(obj, ITEM_HUM))
        safe_strcat(MAX_STRING_LENGTH, buf, "@@a(@@WH@@lu@@Bm@@lm@@Bi@@ln@@Wg@@a)@@N ");

    if (fShort) {
        if (obj->short_descr != NULL)
            safe_strcat(MAX_STRING_LENGTH, buf, obj->short_descr);
    }
    else {
        if (obj->description != NULL)
            safe_strcat(MAX_STRING_LENGTH, buf, obj->description);
    }
    safe_strcat(MAX_STRING_LENGTH, buf, colour_string(ch, "normal"));
    return buf;
}

/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void
show_list_to_char(OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing)
{
    char                buf[MAX_STRING_LENGTH];
    char              **prgpstrShow;
    int                *prgnShow;
    char               *pstrShow;
    OBJ_DATA           *obj;
    int                 nShow;
    int                 iShow;
    int                 count;
    bool                fCombine;

    if (ch->desc == NULL)
        return;
    buf[0] = '\0';

    /*
     * Alloc space for output lines.
     */
    count = 0;
    for (obj = list; obj != NULL; obj = obj->next_in_carry_list)
        count++;
    prgpstrShow = qgetmem(count * sizeof(char *));
    prgnShow = qgetmem(count * sizeof(int));
    nShow = 0;

    /*
     * Format the list of objects.
     */
    for (obj = list; obj != NULL; obj = obj->next_in_carry_list) {
        if (obj->wear_loc == WEAR_NONE && can_see_obj(ch, obj)) {
            pstrShow = format_obj_to_char(obj, ch, fShort);
            fCombine = FALSE;

            if (IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE)) {
                /*
                 * Look for duplicates, case sensitive.
                 * Matches tend to be near end so run loop backwords.
                 */
                for (iShow = nShow - 1; iShow >= 0; iShow--) {
                    if (!strcmp(prgpstrShow[iShow], pstrShow)) {
                        prgnShow[iShow]++;
                        fCombine = TRUE;
                        break;
                    }
                }
            }

            /*
             * Couldn't combine, or didn't want to.
             */
            if (!fCombine) {
                prgpstrShow[nShow] = str_dup(pstrShow);
                prgnShow[nShow] = 1;
                nShow++;
            }
        }
    }

    /*
     * Output the formatted list.
     */
    for (iShow = 0; iShow < nShow; iShow++) {
        if (IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE)) {
            if (prgnShow[iShow] != 1) {
                sendf(ch, "(%2d) ", prgnShow[iShow]);
            }
            else {
                send_to_char("     ", ch);
            }
        }
        send_to_char(prgpstrShow[iShow], ch);
        send_to_char("\n\r", ch);
        free_string(prgpstrShow[iShow]);
    }

    if (fShowNothing && nShow == 0) {
        if (IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE))
            send_to_char("     ", ch);
        send_to_char("Nothing.\n\r", ch);
    }

    /*
     * Clean up.
     */
    qdispose(prgpstrShow);
    qdispose(prgnShow);

    return;
}

void
show_room_list_to_char(OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing)
{
    char                buf[MAX_STRING_LENGTH];
    char              **prgpstrShow;
    int                *prgnShow;
    char               *pstrShow;
    OBJ_DATA           *obj;
    int                 nShow;
    int                 iShow;
    int                 count;
    bool                fCombine;

    if (ch->desc == NULL)
        return;
    buf[0] = '\0';

    /*
     * Alloc space for output lines.
     */
    count = 0;
    for (obj = list; obj != NULL; obj = obj->next_in_room)
        count++;
    prgpstrShow = qgetmem(count * sizeof(char *));
    prgnShow = qgetmem(count * sizeof(int));
    nShow = 0;

    /*
     * Format the list of objects.
     */
    for (obj = list; obj != NULL; obj = obj->next_in_room) {
        if (obj->wear_loc == WEAR_NONE && can_see_obj(ch, obj) && str_cmp(obj->description, "")) {
            pstrShow = format_obj_to_char(obj, ch, fShort);
            fCombine = FALSE;

            if (IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE)) {
                /*
                 * Look for duplicates, case sensitive.
                 * Matches tend to be near end so run loop backwords.
                 */
                for (iShow = nShow - 1; iShow >= 0; iShow--) {
                    if (!strcmp(prgpstrShow[iShow], pstrShow)) {
                        prgnShow[iShow]++;
                        fCombine = TRUE;
                        break;
                    }
                }
            }

            /*
             * Couldn't combine, or didn't want to.
             */
            if (!fCombine) {
                prgpstrShow[nShow] = str_dup(pstrShow);
                prgnShow[nShow] = 1;
                nShow++;
            }
        }
    }

    /*
     * Output the formatted list.
     */
    for (iShow = 0; iShow < nShow; iShow++) {
        if ((IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE)) && str_cmp(prgpstrShow[iShow], "")) {
            if (prgnShow[iShow] != 1) {
                sendf(ch, "(%2d) ", prgnShow[iShow]);
            }
            else {
                send_to_char("     ", ch);
            }
        }
        send_to_char(prgpstrShow[iShow], ch);
        send_to_char("\n\r", ch);

        if (prgpstrShow[iShow])
            free_string(prgpstrShow[iShow]);
    }

    if (fShowNothing && nShow == 0) {
        if (IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE))
            send_to_char("     ", ch);
        send_to_char("Nothing.\n\r", ch);
    }

    /*
     * Clean up.
     */
    qdispose(prgpstrShow);
    qdispose(prgnShow);

    return;
}

void
show_char_to_char_0(CHAR_DATA *victim, CHAR_DATA *ch)
{
    char                buf[MAX_STRING_LENGTH];
    char                buf2[MAX_STRING_LENGTH];
    RULER_DATA         *ruler;

    sprintf(buf, "%s", colour_string(ch, "mobiles"));
    buf2[0] = '\0';

    /* disguise bypasses everything, except in duels and when player is linkdead */
    if (!IS_NPC(victim)
        && victim->position == POS_STANDING && victim->long_descr[0] != '\0' && !is_in_duel(victim, DUEL_STAGE_GO)
        && victim->desc) {
        safe_strcat(MAX_STRING_LENGTH, buf, victim->long_descr);
        safe_strcat(MAX_STRING_LENGTH, buf, colour_string(ch, "normal"));
        safe_strcat(MAX_STRING_LENGTH, buf, "@@N");

        send_to_char(buf, ch);
        return;
    }

    if (!IS_NPC(victim)) {
        sprintf(buf2, "(%s) ", race_table[victim->race].race_name);
        safe_strcat(MAX_STRING_LENGTH, buf, buf2);
    }

    if (IS_NPC(victim)) {
        if (IS_SET(victim->act, ACT_UNDEAD) && (IS_AFFECTED(ch, AFF_DETECT_UNDEAD) || item_has_apply(ch, ITEM_APPLY_DET_UNDEAD)))
            safe_strcat(MAX_STRING_LENGTH, buf, "@@R(@@dUn@@gde@@dad@@R)@@N ");
    }

    if (IS_AFFECTED(victim, AFF_INVISIBLE))
        safe_strcat(MAX_STRING_LENGTH, buf, "(Invis) ");

    if (IS_AFFECTED(victim, AFF_HIDE) || item_has_apply(victim, ITEM_APPLY_HIDE))
        safe_strcat(MAX_STRING_LENGTH, buf, "(Hide) ");

    if (IS_AFFECTED(victim, AFF_CHARM))
        safe_strcat(MAX_STRING_LENGTH, buf, "@@e(@@mCharm@@e)@@N ");

    if (IS_AFFECTED(victim, AFF_PASS_DOOR) || item_has_apply(victim, ITEM_APPLY_PASS_DOOR))
        safe_strcat(MAX_STRING_LENGTH, buf, "(Clear) ");

    if (IS_AFFECTED(victim, AFF_FAERIE_FIRE))
        safe_strcat(MAX_STRING_LENGTH, buf, "@@r(@@pPink@@r)@@N ");

    if (IS_EVIL(victim)
        && (IS_AFFECTED(ch, AFF_DETECT_EVIL) || item_has_apply(ch, ITEM_APPLY_DET_EVIL)))
        safe_strcat(MAX_STRING_LENGTH, buf, "@@R(@@eRed@@R)@@N ");

    if (IS_AFFECTED(victim, AFF_SANCTUARY) || item_has_apply(victim, ITEM_APPLY_SANC))
        safe_strcat(MAX_STRING_LENGTH, buf, "@@g(@@WWhite@@g)@@N ");

    if (!IS_NPC(victim) && IS_SET(victim->act, PLR_KILLER))
        safe_strcat(MAX_STRING_LENGTH, buf, "@@W(@@eK@@di@@Rl@@el@@de@@Rr@@W)@@N ");

    if (!IS_NPC(victim) && IS_SET(victim->act, PLR_THIEF))
        safe_strcat(MAX_STRING_LENGTH, buf, "@@W(@@dT@@gh@@di@@ge@@df@@W)@@N ");

    if (!IS_NPC(victim) && !victim->desc)
        safe_strcat(MAX_STRING_LENGTH, buf, "(Linkdead) ");

    if (!IS_NPC(victim) && victim->adept_level == 20 && (ruler = get_ruler(victim))) {
        if (strcmp(ruler->rank, "@@N")) {
            safe_strcat(MSL, buf, ruler->rank);
            safe_strcat(MSL, buf, " ");
        }
    }

    if (victim->position == POS_STANDING && victim->long_descr[0] != '\0' && IS_NPC(victim)) {
        safe_strcat(MAX_STRING_LENGTH, buf, victim->long_descr);
        safe_strcat(MAX_STRING_LENGTH, buf, colour_string(ch, "normal"));

        if ((IS_AFFECTED(victim, AFF_CLOAK_FLAMING))
            || (IS_AFFECTED(victim, AFF_CLOAK_ABSORPTION))
            || (IS_AFFECTED(victim, AFF_CLOAK_REFLECTION))
            ) {
            safe_strcat(MAX_STRING_LENGTH, buf, "  @@NCLOAK:");
            if (IS_AFFECTED(victim, AFF_CLOAK_FLAMING)) {
                safe_strcat(MAX_STRING_LENGTH, buf, " ");
                safe_strcat(MSL, buf, cloak_table[CLOAK_FLAMING].name);
            }

            if (IS_AFFECTED(victim, AFF_CLOAK_ABSORPTION)) {
                safe_strcat(MAX_STRING_LENGTH, buf, " ");
                safe_strcat(MSL, buf, cloak_table[CLOAK_ABSORB].name);
            }

            if (IS_AFFECTED(victim, AFF_CLOAK_REFLECTION)) {
                safe_strcat(MAX_STRING_LENGTH, buf, " ");
                safe_strcat(MSL, buf, cloak_table[CLOAK_REFLECT].name);
            }

            safe_strcat(MAX_STRING_LENGTH, buf, "\n\r");
        }

        if (victim->first_shield != NULL) {
            SHIELD_DATA       *shield;

            safe_strcat(MSL, buf, "   @@WSHIELD: @@N");
            for (shield = victim->first_shield; shield != NULL; shield = shield->next) {
                safe_strcat(MSL, buf, shield_table[shield->index].name);
                safe_strcat(MSL, buf, " ");
            }

            safe_strcat(MSL, buf, "\n\r");
        }

        send_to_char(buf, ch);
        return;
    }

    safe_strcat(MAX_STRING_LENGTH, buf, PERS(victim, ch));
    if (!IS_NPC(victim) && !IS_SET(ch->act, PLR_BRIEF)) {
        safe_strcat(MAX_STRING_LENGTH, buf, victim->pcdata->title);
        safe_strcat(MAX_STRING_LENGTH, buf, "@@N");
    }

    switch (victim->position) {
        case POS_DEAD:
            safe_strcat(MAX_STRING_LENGTH, buf, " is DEAD!!");
            break;
        case POS_MORTAL:
            safe_strcat(MAX_STRING_LENGTH, buf, " is mortally wounded.");
            break;
        case POS_INCAP:
            safe_strcat(MAX_STRING_LENGTH, buf, " is incapacitated.");
            break;
        case POS_STUNNED:
            safe_strcat(MAX_STRING_LENGTH, buf, " is lying here stunned.");
            break;
        case POS_SLEEPING:
            safe_strcat(MAX_STRING_LENGTH, buf, " is sleeping here.");
            break;
        case POS_RESTING:

            if (victim->sitting != NULL && victim->sitting->in_room == victim->in_room) {
                char                sit[MAX_INPUT_LENGTH];

                sprintf(sit, " is here, resting on %s.", victim->sitting->short_descr);
                safe_strcat(MAX_STRING_LENGTH, buf, sit);
            }
            else
                safe_strcat(MAX_STRING_LENGTH, buf, " is resting here.");
            break;

        case POS_STANDING:
        {
            safe_strcat(MAX_STRING_LENGTH, buf, " is here");
            if (!IS_NPC(victim)
                && (victim->stance > 0)
                && (victim->stance != STANCE_AMBUSH)
                ) {
                char                stance_buf[MSL];

                sprintf(stance_buf, " in the Stance of the %s.", stance_app[victim->stance].name);
                safe_strcat(MSL, buf, stance_buf);
            }
            else
                safe_strcat(MSL, buf, ".");

            break;
        }

        case POS_WRITING:
            safe_strcat(MAX_STRING_LENGTH, buf, " is writing a note.");
            break;
        case POS_BUILDING:
            safe_strcat(MAX_STRING_LENGTH, buf, " is BUILDING!!");
            break;
        case POS_FIGHTING:
            safe_strcat(MAX_STRING_LENGTH, buf, " is here, fighting ");
            if (victim->fighting == NULL)
                safe_strcat(MAX_STRING_LENGTH, buf, "thin air??");
            else if (victim->fighting == ch)
                safe_strcat(MAX_STRING_LENGTH, buf, "YOU!");
            else if (victim->in_room == victim->fighting->in_room) {
                safe_strcat(MAX_STRING_LENGTH, buf, PERS(victim->fighting, ch));
                safe_strcat(MAX_STRING_LENGTH, buf, ".");
            }
            else
                safe_strcat(MAX_STRING_LENGTH, buf, "somone who left??");

            break;
    }

    safe_strcat(MAX_STRING_LENGTH, buf, colour_string(ch, "normal"));
    safe_strcat(MAX_STRING_LENGTH, buf, "@@N");
    safe_strcat(MAX_STRING_LENGTH, buf, "\n\r");

    if ((IS_AFFECTED(victim, AFF_CLOAK_FLAMING))
        || (IS_AFFECTED(victim, AFF_CLOAK_ADEPT))
        || (IS_AFFECTED(victim, AFF_CLOAK_REGEN))
        || (IS_AFFECTED(victim, AFF_CLOAK_ABSORPTION))
        || (IS_AFFECTED(victim, AFF_CLOAK_REFLECTION))
        || (IS_AFFECTED(victim, AFF_CLOAK_MANA))) {
        safe_strcat(MAX_STRING_LENGTH, buf, "  @@N@@WCLOAK:");

        if (IS_AFFECTED(victim, AFF_CLOAK_MANA)) {
            safe_strcat(MSL, buf, " ");
            safe_strcat(MSL, buf, cloak_table[CLOAK_MANA].name);
        }

        if (IS_AFFECTED(victim, AFF_CLOAK_FLAMING)) {
            safe_strcat(MSL, buf, " ");
            safe_strcat(MSL, buf, cloak_table[CLOAK_FLAMING].name);
        }

        if (IS_AFFECTED(victim, AFF_CLOAK_ABSORPTION)) {
            safe_strcat(MSL, buf, " ");
            safe_strcat(MSL, buf, cloak_table[CLOAK_ABSORB].name);
        }

        if (IS_AFFECTED(victim, AFF_CLOAK_REFLECTION)) {
            safe_strcat(MSL, buf, " ");
            safe_strcat(MSL, buf, cloak_table[CLOAK_REFLECT].name);
        }

        if (IS_AFFECTED(victim, AFF_CLOAK_ADEPT)) {
            safe_strcat(MSL, buf, " ");
            safe_strcat(MSL, buf, cloak_table[CLOAK_ADEPT].name);
        }

        if (IS_AFFECTED(victim, AFF_CLOAK_REGEN)) {
            safe_strcat(MSL, buf, " ");
            safe_strcat(MSL, buf, cloak_table[CLOAK_REGEN].name);
        }

        safe_strcat(MAX_STRING_LENGTH, buf, "\n\r");
    }

    if (victim->first_shield != NULL) {
        SHIELD_DATA       *shield;

        safe_strcat(MSL, buf, "   @@WSHIELD:@@N");
        for (shield = victim->first_shield; shield != NULL; shield = shield->next) {
            safe_strcat(MSL, buf, " ");
            safe_strcat(MSL, buf, shield_table[shield->index].name);
        }

        safe_strcat(MSL, buf, "\n\r");
    }

    if (victim->riding != NULL && victim->riding->in_room && victim->in_room && victim->in_room == victim->riding->in_room) {
        sprintf(buf2, "  riding %s.\n\r", victim->riding->short_descr);
        safe_strcat(MSL, buf, buf2);
        safe_strcat(MSL, buf, "@@N");
    }

    send_to_char(buf, ch);
    return;
}

void
show_char_to_char_1(CHAR_DATA *victim, CHAR_DATA *ch)
{
    char                buf[MAX_STRING_LENGTH];
    OBJ_DATA           *obj;
    int                 iWear;
    int                 pct;
    bool                found;
    bool                seen = FALSE;

    buf[0] = '\0';

    if (can_see(victim, ch)) {

        act("$n looks at you.", ch, NULL, victim, TO_VICT);
        act("$n looks at $N.", ch, NULL, victim, TO_NOTVICT);
    }

    if (victim->description[0] != '\0' && !IS_SET(ch->act, PLR_BRIEF)) {
        send_to_char(victim->description, ch);
    }
    else {
        act("You see nothing special about $M.", ch, NULL, victim, TO_CHAR);
    }

    send_to_char("@@N", ch);

    if (victim->max_hit > 0)
        pct = (100 * victim->hit) / victim->max_hit;
    else
        pct = -1;

    strcpy(buf, PERS(victim, ch));

    if (pct >= 100)
        safe_strcat(MAX_STRING_LENGTH, buf, " is in pristine condition.\n\r");
    else if (pct >= 90)
        safe_strcat(MAX_STRING_LENGTH, buf, " is slightly scratched.\n\r");
    else if (pct >= 80)
        safe_strcat(MAX_STRING_LENGTH, buf, " has some light bruising.\n\r");
    else if (pct >= 70)
        safe_strcat(MAX_STRING_LENGTH, buf, " has some shallow cuts.\n\r");
    else if (pct >= 60)
        safe_strcat(MAX_STRING_LENGTH, buf, " has several weeping wounds.\n\r");
    else if (pct >= 50)
        safe_strcat(MAX_STRING_LENGTH, buf, " looks like a traffic accident.\n\r");
    else if (pct >= 40)
        safe_strcat(MAX_STRING_LENGTH, buf, " is bleeding slowly into a puddle.\n\r");
    else if (pct >= 30)
        safe_strcat(MAX_STRING_LENGTH, buf, " is spraying blood all over.\n\r");
    else if (pct >= 20)
        safe_strcat(MAX_STRING_LENGTH, buf, " is having trouble living.\n\r");
    else if (pct >= 10)
        safe_strcat(MAX_STRING_LENGTH, buf, " looks ready to kick the bucket.\n\r");
    else
        safe_strcat(MAX_STRING_LENGTH, buf, " is DYING.\n\r");

    buf[0] = UPPER(buf[0]);
    send_to_char(buf, ch);

    found = FALSE;

    for (iWear = 0; iWear < MAX_WEAR; iWear++) {
        pct = iWear;

        if (iWear == 17 && !seen) {
            pct = 21;
            iWear--;
            seen = TRUE;
        }
        else if (pct == 21)
            continue;

        if ((obj = get_eq_char(victim, pct)) != NULL && can_see_obj(ch, obj)) {
            if (!found) {
                send_to_char("\n\r", ch);
                act("$N is using:", ch, NULL, victim, TO_CHAR);
                found = TRUE;
            }
            send_to_char(where_name[pct], ch);
            send_to_char(format_obj_to_char(obj, ch, TRUE), ch);
            send_to_char("\n\r", ch);
        }
    }

    if (victim != ch && !IS_NPC(ch)
        && number_percent() < ch->pcdata->learned[gsn_peek]) {
        send_to_char("\n\rYou peek at the inventory:\n\r", ch);
        show_list_to_char(victim->first_carry, ch, TRUE, TRUE);
    }

    return;
}

void
show_char_to_char(CHAR_DATA *list, CHAR_DATA *ch, bool automode)
{
    CHAR_DATA          *rch;

    for (rch = list; rch != NULL; rch = rch->next_in_room) {
        if (rch == ch)
            continue;

        if (!IS_NPC(rch)
            && IS_SET(rch->act, PLR_WIZINVIS)
            && get_trust(ch) < rch->invis)
            continue;

        if ((rch->rider != NULL)
            && (rch->rider != ch)
            && rch->in_room != NULL && rch->rider->in_room != NULL && rch->rider->in_room == rch->in_room)
            continue;            /* show under the rider, if rider is in same room */

        if (automode && !IS_NPC(ch) && IS_SET(ch->act, PLR_BRIEF2) && !IS_NPC(rch) && is_same_group(ch, rch))
            continue;

        if (can_see(ch, rch)) {
            show_char_to_char_0(rch, ch);
        }
        else if (room_is_dark(ch->in_room)
            && (IS_AFFECTED(rch, AFF_INFRARED) || item_has_apply(rch, ITEM_APPLY_INFRA))) {
            send_to_char("@@eYou see glowing red eyes watching YOU!@@N\n\r", ch);
        }
    }

    return;
}

bool
check_blind(CHAR_DATA *ch)
{
    if (!IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT))
        return TRUE;

    if (IS_AFFECTED(ch, AFF_BLIND)) {
        send_to_char("@@lYou can't see shit!@@N\n\r", ch);
        return FALSE;
    }

    return TRUE;
}

void
do_look(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    char                arg1[MAX_INPUT_LENGTH];
    char                arg2[MAX_INPUT_LENGTH];
    char                out[MAX_STRING_LENGTH];
    CHAR_DATA          *ppl;
    ROOM_INDEX_DATA    *room;
    ROOM_AFFECT_DATA   *raf;
    EXIT_DATA          *pexit;
    CHAR_DATA          *victim;
    OBJ_DATA           *obj;
    char                pdesc[MSL];
    int                 door;
    bool                found;
    bool                automode = FALSE;

    buf[0] = '\0';
    out[0] = '\0';

    if (!IS_NPC(ch) && ch->desc == NULL)
        return;

    if (ch->position < POS_SLEEPING) {
        send_to_char("@@yYou can't see anything but stars!@@N\n\r", ch);
        return;
    }

    if (ch->position == POS_SLEEPING) {
        send_to_char("@@gYou can't see anything, you're sleeping!@@N\n\r", ch);
        return;
    }

    if (!check_blind(ch))
        return;

    if (!IS_NPC(ch)
        && !IS_SET(ch->act, PLR_HOLYLIGHT)
        && room_is_dark(ch->in_room)) {
        send_to_char("It is pitch black ... \n\r", ch);
        show_char_to_char(ch->in_room->first_person, ch, !str_cmp(arg1, "auto"));
        return;
    }

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0' || !str_cmp(arg1, "auto")) {
        if (!str_cmp(arg1, "auto"))
            automode = TRUE;

        /* 'look' or 'look auto' */
        sprintf(out, "%s%s%s\n\r", colour_string(ch, "rooms"), ch->in_room->name, colour_string(ch, "normal"));

        send_to_char(out, ch);

        if (IS_SWITCHED(ch) || (!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT)))
            do_exits(ch, "auto");

        if (arg1[0] == '\0' || (IS_SWITCHED(ch) || (!IS_NPC(ch) && !IS_SET(ch->act, PLR_BRIEF)))) {
            sprintf(out, "%s%s%s\n\r", colour_string(ch, "rooms"), ch->in_room->description, colour_string(ch, "normal"));
            send_to_char(out, ch);
        }

        /* Show any room-affects */
        if ((ch->in_room->affected_by != 0)
            && ((is_affected(ch, skill_lookup("detect magic")))
                || (item_has_apply(ch, ITEM_APPLY_DET_MAG)))) {
            sprintf(out, "%s", colour_string(ch, "rooms"));
            if (IS_SET(ch->in_room->affected_by, ROOM_BV_ENCAPS))
                safe_strcat(MAX_STRING_LENGTH, out, "@@NA barely visible @@renergy web@@N is blocking all exits here.\n\r");
            if (IS_SET(ch->in_room->affected_by, ROOM_BV_FIRE_RUNE))
                safe_strcat(MAX_STRING_LENGTH, out, "@@NA mystical @@eFire @@NRune@@N hangs in the air above you.\n\r");
            if (IS_SET(ch->in_room->affected_by, ROOM_BV_SHOCK_RUNE))
                safe_strcat(MAX_STRING_LENGTH, out, "@@NA mystical @@lShock@@N Rune@@N hangs in the air above you.\n\r");
            if (IS_SET(ch->in_room->affected_by, ROOM_BV_POISON_RUNE))
                safe_strcat(MAX_STRING_LENGTH, out, "@@NA mystical @@dPoison@@N Rune hangs in the air above you.\n\r");
            if (IS_SET(ch->in_room->affected_by, ROOM_BV_HEAL_REGEN))
                safe_strcat(MAX_STRING_LENGTH, out, "@@NA majestic @@mHealing Light@@N encompasses the room.\n\r");
            if (IS_SET(ch->in_room->affected_by, ROOM_BV_HEAL_STEAL))
                safe_strcat(MAX_STRING_LENGTH, out, "@@NA menacing @@dWithering shadow@@N enfolds the room.\n\r");
            if (IS_SET(ch->in_room->affected_by, ROOM_BV_MANA_REGEN))
                safe_strcat(MAX_STRING_LENGTH, out, "@@NA powerful @@eMana Flare@@N empowers the room.\n\r");
            if (IS_SET(ch->in_room->affected_by, ROOM_BV_MANA_STEAL))
                safe_strcat(MAX_STRING_LENGTH, out, "@@NA mind sapping @@dMana Drain@@N enfolds the room.\n\r");
            if (IS_SET(ch->in_room->affected_by, ROOM_BV_HOLD))
                safe_strcat(MAX_STRING_LENGTH, out, "@@NThe magical bars of a @@rCage@@N surround the room.\n\r");

            if (IS_SET(ch->in_room->affected_by, ROOM_BV_SOUL_NET))
                safe_strcat(MAX_STRING_LENGTH, out, "@@NA demonic @@dSoul Net@@N enshrouds the room.\n\r");

            if (IS_SET(ch->in_room->affected_by, ROOM_BV_SMOKESCREEN))
                safe_strcat(MAX_STRING_LENGTH, out, "@@dA @@gsmoke screen @@dsurrounds the room.@@N\n\r");

            for (raf = ch->in_room->first_room_affect; raf != NULL; raf = raf->next) {
                if (raf->bitvector == ROOM_BV_WARNING_RUNE)
                    if ((raf->caster == ch) || (IS_IMMORTAL(ch)))
                        safe_strcat(MSL, out, "@@NA @@dstrange @@Nhovering rune appears here.\n\r");
                if (raf->bitvector == ROOM_BV_SENTRY)
                    if ((raf->caster == ch) || (IS_IMMORTAL(ch)))
                        safe_strcat(MSL, out, "@@NA @@dsentry @@Nis hovering here.\n\r");
            }

            safe_strcat(MAX_STRING_LENGTH, out, colour_string(ch, "normal"));
            send_to_char(out, ch);
        }

        show_room_list_to_char(ch->in_room->first_content, ch, FALSE, FALSE);
        show_char_to_char(ch->in_room->first_person, ch, automode);
        return;
    }

    if (!str_cmp(arg1, "i") || !str_cmp(arg1, "in")) {
        /* 'look in' */
        if (arg2[0] == '\0') {
            send_to_char("Look in what?\n\r", ch);
            return;
        }

        if ((obj = get_obj_here_r(ch, arg2)) == NULL) {
            send_to_char("You do not see that here.\n\r", ch);
            return;
        }

        switch (obj->item_type) {
            default:
                send_to_char("That is not a container.\n\r", ch);
                break;

            case ITEM_PORTAL:
                if (obj->value[1] == 0) {
                    act("You don't seem to be able to look in $p.", ch, obj, NULL, TO_CHAR);
                    return;
                }

                if ((room = get_room_index(obj->value[0])) == NULL) {
                    send_to_char("You see nothing but blackness!\n\r", ch);
                    return;
                }

                act("$n looks into $p.", ch, obj, NULL, TO_ROOM);

                if (!IS_NPC(ch)
                    && !IS_SET(ch->act, PLR_HOLYLIGHT)
                    && room_is_dark(ch->in_room)) {
                    act("$p comes out into a dark place.  You see nothing!", ch, obj, NULL, TO_CHAR);
                    return;
                }

                sprintf(buf, "You look in $p and see: %s%s.%s", colour_string(ch, "rooms"), room->name, colour_string(ch, "normal"));
                act(buf, ch, obj, NULL, TO_CHAR);

                found = FALSE;
                if (room->first_person != NULL) {
                    send_to_char("You see the following beings:\n\r", ch);
                    for (ppl = room->first_person; ppl != NULL; ppl = ppl->next_in_room) {
                        if (can_see(ch, ppl)) {
                            found = TRUE;
                            sendf(ch, "%s%s%s\n\r", colour_string(ch, "mobiles"), (IS_NPC(ppl) ? ppl->short_descr : ppl->short_descr), colour_string(ch, "normal"));
                        }
                    }
                }
                if (!found)
                    act("You see no beings through $p.", ch, obj, NULL, TO_CHAR);

                if (room->first_content != NULL) {
                    send_to_char("Some objects are visible:\n\r", ch);
                    show_room_list_to_char(room->first_content, ch, TRUE, FALSE);
                }
                else
                    act("You see no objects through $p.", ch, obj, NULL, TO_CHAR);

                break;

            case ITEM_DRINK_CON:
                if (obj->value[1] <= 0) {
                    send_to_char("It is empty.\n\r", ch);
                    break;
                }

                sendf(ch, "It's %s full of a %s liquid.\n\r",
                    obj->value[1] < obj->value[0] / 4
                    ? "less than" : obj->value[1] < 3 * obj->value[0] / 4 ? "about" : "more than", liq_table[obj->value[2]].liq_color);
                break;
            case ITEM_SPELL_MATRIX:
            case ITEM_CONTAINER:
            case ITEM_CORPSE_NPC:
            case ITEM_CORPSE_PC:
                if (IS_SET(obj->value[1], CONT_CLOSED)) {
                    send_to_char("It is closed.\n\r", ch);
                    break;
                }

                if (obj->carried_by == NULL && (obj->item_type == ITEM_CONTAINER || obj->item_type == ITEM_SPELL_MATRIX))
                    act("$p (on the ground) contains:", ch, obj, NULL, TO_CHAR);
                else
                    act("$p contains:", ch, obj, NULL, TO_CHAR);

                show_list_to_char(obj->first_in_carry_list, ch, TRUE, TRUE);
                break;
        }
        return;
    }

    if (!str_cmp(arg1, "board") || !str_cmp(arg2, "board")) {
        /*int bnum; */

        if ((obj = get_obj_here(ch, "board")) == NULL) {
            send_to_char("You do not see that here.\n\r", ch);
            return;

        }

        do_show_contents(ch, obj);
        return;
    }

    if ((victim = get_char_room(ch, arg1)) != NULL) {
        show_char_to_char_1(victim, ch);
        return;
    }

    for (obj = ch->first_carry; obj != NULL; obj = obj->next_in_carry_list) {
        if (can_see_obj(ch, obj)) {
            sprintf(pdesc, "\n\r%s", get_extra_descr(arg1, obj->first_exdesc));
            if (str_cmp(pdesc, "\n\r(null)")) {
                send_to_char(pdesc, ch);
                act("$L$n closely examines $p.", ch, obj, NULL, TO_ROOM);
                return;
            }

            sprintf(pdesc, "\n\r%s", get_extra_descr(arg1, obj->pIndexData->first_exdesc));

            if (str_cmp(pdesc, "\n\r(null)")) {
                send_to_char(pdesc, ch);
                act("$L$n closely examines $p.", ch, obj, NULL, TO_ROOM);
                return;
            }
        }

        if (is_name(arg1, obj->name)) {
            send_to_char(obj->description, ch);
            return;
        }
    }

    for (obj = ch->in_room->first_content; obj != NULL; obj = obj->next_in_room) {
        if (can_see_obj(ch, obj)) {
            sprintf(pdesc, "\n\r%s", get_extra_descr(arg1, obj->first_exdesc));

            if (str_cmp(pdesc, "\n\r(null)")) {
                send_to_char(pdesc, ch);
                act("$L$n closely examines $p.", ch, obj, NULL, TO_ROOM);
                return;
            }

            sprintf(pdesc, "\n\r%s", get_extra_descr(arg1, obj->pIndexData->first_exdesc));
            if (str_cmp(pdesc, "\n\r(null)")) {
                send_to_char(pdesc, ch);
                act("$L$n closely examines $p.", ch, obj, NULL, TO_ROOM);
                return;
            }
        }

        if (is_name(arg1, obj->name)) {
            send_to_char(obj->description, ch);
            act("$L$n closely examines $p.", ch, obj, NULL, TO_ROOM);
            return;
        }
    }

    sprintf(pdesc, "\n\r%s", get_extra_descr(arg1, ch->in_room->first_exdesc));
    if (str_cmp(pdesc, "\n\r(null)")) {
        send_to_char(pdesc, ch);
        act("$L$n closely examines the $t.", ch, arg1, NULL, TO_ROOM);
        return;
    }

    if (!str_cmp(arg1, "n") || !str_cmp(arg1, "north"))
        door = 0;
    else if (!str_cmp(arg1, "e") || !str_cmp(arg1, "east"))
        door = 1;
    else if (!str_cmp(arg1, "s") || !str_cmp(arg1, "south"))
        door = 2;
    else if (!str_cmp(arg1, "w") || !str_cmp(arg1, "west"))
        door = 3;
    else if (!str_cmp(arg1, "u") || !str_cmp(arg1, "up"))
        door = 4;
    else if (!str_cmp(arg1, "d") || !str_cmp(arg1, "down"))
        door = 5;
    else {
        send_to_char("You do not see that here.\n\r", ch);
        return;
    }

    /* 'look direction' */
    if ((pexit = ch->in_room->exit[door]) == NULL) {
        send_to_char("Nothing special there.\n\r", ch);
        return;
    }

    if ((pexit->description != NULL)
        && (pexit->description[0] != '\0')
        && ((!str_cmp(pexit->keyword, ""))
            || ((str_cmp(pexit->keyword, ""))
                && (!str_cmp(pexit->keyword, arg1)))))
        send_to_char(pexit->description, ch);
    else
        send_to_char("Nothing special there.\n\r", ch);

    if ((pexit->keyword != NULL)
        && (pexit->keyword[0] != '\0')
        && (pexit->keyword[0] != ' ')
        && (!str_cmp(pexit->keyword, arg1))) {
        if (IS_SET(pexit->exit_info, EX_CLOSED)) {
            act("$D is closed.", ch, NULL, pexit->keyword, TO_CHAR);
        }
        else if (IS_SET(pexit->exit_info, EX_ISDOOR)) {
            act("$D is open.", ch, NULL, pexit->keyword, TO_CHAR);
        }
    }

    return;
}

void
do_xlook(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA          *victim;
    OBJ_DATA           *obj;
    int                 iWear;
    char                arg[MAX_INPUT_LENGTH];

    if ((victim = get_char_world(ch, argument)) == NULL) {
        send_to_char("No such player found.\n\r", ch);
        return;
    }

    if (!can_see(ch, victim)) {
        send_to_char("No such player found.\n\r", ch);
        return;
    }
    act("$N is wearing:", ch, NULL, victim, TO_CHAR);
    for (iWear = 0; iWear < MAX_WEAR; iWear++) {
        if ((obj = get_eq_char(victim, iWear)) != NULL && can_see_obj(ch, obj)) {
            send_to_char(where_name[iWear], ch);
            one_argument(obj->name, arg);
            send_to_char("[", ch);
            send_to_char(arg, ch);
            send_to_char("] ", ch);
            send_to_char(format_obj_to_char(obj, ch, TRUE), ch);
            send_to_char("\n\r", ch);
        }
    }

    send_to_char("\n\r", ch);
    send_to_char("Inventory:\n\r", ch);
    /*    show_list_to_char( victim->first_carry, ch, TRUE, FALSE ); */

    {
        OBJ_DATA           *in;
        bool                found = FALSE;

        for (obj = victim->first_carry; obj != NULL; obj = obj->next_in_carry_list) {
            if (obj->wear_loc != WEAR_NONE || !can_see_obj(ch, obj))
                continue;

            found = TRUE;

            one_argument(obj->name, arg);

            sendf(ch, "     [%s] %s\n\r", arg, format_obj_to_char(obj, ch, TRUE));

            if (obj->first_in_carry_list) {
                for (in = obj->first_in_carry_list; in != NULL; in = in->next_in_carry_list) {
                    one_argument(in->name, arg);
                    sendf(ch, "          [%s] %s\n\r", arg, format_obj_to_char(in, ch, TRUE));
                }
            }
        }

        if (!found)
            send_to_char("     Nothing.\n\r", ch);

    }

    return;
}

void
do_examine(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    char                arg[MAX_INPUT_LENGTH];
    OBJ_DATA           *obj;

    buf[0] = '\0';

    one_argument(argument, arg);

    if (arg[0] == '\0') {
        send_to_char("Examine what?\n\r", ch);
        return;
    }

    do_look(ch, arg);

    if ((obj = get_obj_here(ch, arg)) != NULL) {
        switch (obj->item_type) {
            default:
                break;

            case ITEM_DRINK_CON:
            case ITEM_CONTAINER:
            case ITEM_CORPSE_NPC:
            case ITEM_CORPSE_PC:
                send_to_char("When you look inside, you see:\n\r", ch);
                sprintf(buf, "in %s", arg);
                do_look(ch, buf);
        }
        if (((ch->position > POS_RESTING) && (ch->in_room != NULL) && (ch->in_room->vnum != 1))
            || (IS_IMMORTAL(ch))
            )
            trigger_handler(ch, obj, TRIGGER_EXAMINE);
    }

    return;
}

/*
 * Thanks to Zrin for auto-exit part.
 */
void
do_exits(CHAR_DATA *ch, char *argument)
{
    extern char        *const compass_name[];
    char                buf[MAX_STRING_LENGTH];
    char                buf2[MAX_STRING_LENGTH];
    EXIT_DATA          *pexit;
    bool                found;
    bool                fAuto;
    int                 door;

    buf[0] = '\0';
    buf2[0] = '\0';

    fAuto = !str_cmp(argument, "auto");

    if (!check_blind(ch))
        return;

    strcpy(buf, fAuto ? "[Exits:" : "Obvious exits:\n\r");

    found = FALSE;
    for (door = 0; door <= 5; door++) {
        /* check for players that are blind (irl) */
        if (!IS_NPC(ch) && IS_IMMORTAL(ch) && fAuto) {
            if ((pexit = ch->in_room->exit[door]) != NULL && pexit->to_room != NULL) {
                found = TRUE;

                if (IS_SET(pexit->exit_info, EX_CLOSED))
                    sprintf(buf2, " (%s)", (!str_cmp(pexit->keyword, "") ? compass_name[door] : pexit->keyword));
                else
                    sprintf(buf2, " %s", (!str_cmp(pexit->keyword, "") ? compass_name[door] : pexit->keyword));

                safe_strcat(MAX_STRING_LENGTH, buf, buf2);
                continue;
            }
        }

        /* Check for thieves with find_doors... */
        if (!IS_NPC(ch))
            if ((pexit = ch->in_room->exit[door]) != NULL && pexit->to_room != NULL && IS_SET(pexit->exit_info, EX_CLOSED)
                && !IS_SET(pexit->exit_info, EX_NODETECT) && (ch->pcdata->learned[gsn_find_doors] > number_percent())
                && (!str_cmp(pexit->keyword, ""))) {
                found = TRUE;

                if (fAuto)
                    sprintf(buf2, " (%s)", compass_name[door]);
                else
                    sprintf(buf2, "%-5s - Door.\n\r", capitalize(compass_name[door]));

                safe_strcat(MAX_STRING_LENGTH, buf, buf2);
                continue;
            }

        if ((pexit = ch->in_room->exit[door]) != NULL && pexit->to_room != NULL && !IS_SET(pexit->exit_info, EX_CLOSED)
            && (!str_cmp(pexit->keyword, ""))) {
            found = TRUE;

            if (fAuto) {
                safe_strcat(MAX_STRING_LENGTH, buf, " ");
                safe_strcat(MAX_STRING_LENGTH, buf, compass_name[door]);
            }
            else {
                sprintf(buf + strlen(buf), "%-5s - %s\n\r", capitalize(compass_name[door]),
                    room_is_dark(pexit->to_room) ? "Too dark to tell" : pexit->to_room->name);
            }
        }
    }

    if (!found)
        safe_strcat(MAX_STRING_LENGTH, buf, fAuto ? " none" : "None.\n\r");

    if (fAuto)
        safe_strcat(MAX_STRING_LENGTH, buf, "]\n\r");

    send_to_char(buf, ch);
    return;
}

void
do_score(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA           *vch = NULL;
    char                buf[MSL];
    char                lbuf[MSL];
    char                buf2[MSL];
    int                 cnt;
    int                 avail_qps = 0;
    sh_int              charmies = 0;
    int                 circle_perc = 0;

    buf[0] = '\0'; lbuf[0] = '\0'; buf2[0] = '\0';

    if (IS_NPC(ch)) {
        send_to_char("Mobs don't need to see their score.\n\r", ch);
        return;
    }

    if (!IS_IMMORTAL(ch) || argument[0] == '\0')
        vch = ch;
    else {
        vch = get_char_world(ch, argument);

        if (!vch) {
            send_to_char("Can't find that player.\n\r", ch);
            return;
        }

        if (IS_NPC(vch)) {
            send_to_char("Can't score mobs.\n\r", ch);
            return;
        }

        if (IS_IMMORTAL(vch) && get_trust(ch) < get_trust(vch)) {
            send_to_char("Can't score immortals higher than you.\n\r", ch);
            return;
        }
    }

    /* top line */
    send_to_char("@@d.-----------------------------------------------------------------@@g=(@@a Score @@g)=@@d-.\n\r", ch);

    /* name + title */
    sprintf(buf, "%s%s@@N", vch->short_descr, vch->pcdata->title);
    sendf(ch, "@@d| @@W%s @@d|\n\r", my_left(buf, lbuf, 75));
    
    /* age + hours RL */
    my_get_age(vch, buf2);
    sprintf(buf, "@@gAge@@d:  @@W%s (%d hours RL)", buf2, my_get_hours(vch));
    sendf(ch, "@@d| %s @@d|\n\r", my_left(buf, lbuf, 75));

    /* race */
    sprintf(buf, "@@gRace@@d: @@W%s", race_table[vch->race].race_title);
    sendf(ch, "@@d| %s ", my_left(buf, lbuf, 36));

    /* clan */
    sprintf(buf, "@@gClan@@d: @@W%s", clan_table[vch->pcdata->clan].clan_name);
    sendf(ch, "%s @@N@@d|\n\r", my_left(buf, lbuf, 38));

    sendf(ch, "%s", "@@d|-----------------------------------------------------------------@@g=(@@W Stats @@g)=@@d-|\n\r");

    sprintf(buf2, "@@gHp@@d:@@e %d@@R/@@e%d   @@gMana@@d:@@r %d@@G/@@r%d   @@gMoves@@d:@@y %d@@b/@@y%d",
        vch->hit, vch->max_hit, vch->mana, vch->max_mana, vch->move, vch->max_move);

    sendf(ch, "@@d| %s @@d|\n\r", center_text(buf2, 75));

    /* only show energy to avatars */
    if (vch->pcdata->avatar) {
        sprintf(buf2, "@@gEnergy@@d:@@a %d@@c/@@a%d", vch->energy, vch->max_energy);

        sendf(ch, "@@d| %s @@d|\n\r", center_text(buf2, 75));
    }

    sprintf(buf2, "@@gStr@@d[@@W%d@@d/@@W%d@@d]  @@gInt@@d[@@W%d@@d/@@W%d@@d]  @@gWis@@d[@@W%d@@d/@@W%d@@d]  @@gDex@@d[@@W%d@@d/@@W%d@@d]  @@gCon@@d[@@W%d@@d/@@W%d@@d]",
        get_curr_str(vch), vch->pcdata->max_str,
        get_curr_int(vch), vch->pcdata->max_int,
        get_curr_wis(vch), vch->pcdata->max_wis,
        get_curr_dex(vch), vch->pcdata->max_dex,
        get_curr_con(vch), vch->pcdata->max_con);

    sendf(ch, "@@d| %s @@d|\n\r", center_text(buf2, 75));

    /* levels */
    send_to_char("@@d|----------------------------------------------------------------@@g=(@@W Levels @@g)=@@d-|\n\r", ch);

    buf2[0] = '\0';
    for (cnt = 0; cnt < MAX_CLASS; cnt++) {
        sprintf(buf, "@@b%s@@d[@@W", class_table[cnt].who_name);
        safe_strcat(MAX_STRING_LENGTH, buf2, buf);
        if (vch->lvl[cnt] != -1)
            sprintf(buf, "%2d@@d] ", vch->lvl[cnt]);
        else
            sprintf(buf, "%s@@d] ", " 0");
        safe_strcat(MAX_STRING_LENGTH, buf2, buf);
    }

    sendf(ch, "@@d|%s @@d|\n\r", center_text(buf2, 76));
    buf2[0] = '\0';

    if (is_remort(vch)) {
        for (cnt = 0; cnt < MAX_CLASS; cnt++) {
            if (vch->lvl2[cnt] != -1) {
                sprintf(buf, "@@m%s@@d[@@W%2d@@d] ", remort_table[cnt].who_name, vch->lvl2[cnt]);
                safe_strcat(MAX_STRING_LENGTH, buf2, buf);
            }
        }
        sendf(ch, "@@d|%s @@d|\n\r", center_text(buf2, 76));
        sprintf(buf, "@@gPseudo Level@@d[@@W%d@@d]", get_pseudo_level(vch));

        if (vch->adept_level > 0) {
            sprintf(buf2, "            @@gAdept@@d[@@m%s@@d]", get_adept_name(vch));
            safe_strcat(MAX_STRING_LENGTH, buf, buf2);
        }

        sendf(ch, "@@d|%s @@d|\n\r", center_text(buf, 76));
    }
    /* end: levels */

    send_to_char("@@d|---------------------------------------------------------------@@g=(@@W Numbers @@g)=@@d-|\n\r", ch);

    /* hitroll */
    sprintf(buf, "@@gHitroll@@d:@@W %d", GET_HITROLL(vch));
    sendf(ch, "@@d| %s ", my_left(buf, lbuf, 36));

    /* experience */
    sprintf(buf, "@@gExperience@@d: @@W%s", number_comma(vch->exp));
    sendf(ch, "%s @@d|\n\r", my_left(buf, lbuf, 38));

    /* damroll */
    sprintf(buf, "@@gDamroll@@d:@@W %d", GET_DAMROLL(vch));
    sendf(ch, "@@d| %s ", my_left(buf, lbuf, 36));

    /* gold */
    {
        int avail = available_gold(vch);

        if (vch->balance <= 0) {
            /* they have no balance */
            sprintf(buf, "@@gGold@@d: @@y%s", number_comma(vch->gold));
        }
        else if (avail == vch->balance) {
            /* they do not have any gold reserved */
            sprintf(buf, "@@gGold@@d: @@y%s @@d/ @@b%s", number_comma(vch->gold), number_comma_r(vch->balance, buf2));
        }
        else {
            /* they have gold reserved */
            sprintf(buf, "@@gGold@@d: @@y%s @@d/ @@b%s @@d(@@g%s@@d)", number_comma(vch->gold), number_comma_r(avail, buf2), number_comma_r(vch->balance - avail, lbuf));
        }

        sendf(ch, "%s @@d|\n\r", my_left(buf, lbuf, 38));
    }

    /* armor class */
    sprintf(buf, "@@gArmor Class@@d: @@W%d", GET_AC(vch));
    sendf(ch, "@@d| %s ", my_left(buf, lbuf, 36));

    /* practices */
    sprintf(buf, "@@gPractices@@d: @@r%d", vch->practice);
    sendf(ch, "%s @@d|\n\r", my_left(buf, lbuf, 38));

    /* saves */
    sprintf(buf, "@@gSave vs Spell@@d: @@W%d", vch->saving_throw);
    sendf(ch, "@@d| %s ", my_left(buf, lbuf, 36));

    /* quest points */
    avail_qps = available_qps(vch);
    if (vch->quest_points == avail_qps)
        sprintf(buf, "@@gQuest Points@@d: @@a%d", vch->quest_points);
    else
        sprintf(buf, "@@gQuest Points@@d: @@a%d @@d+ @@c%d", avail_qps, vch->quest_points - avail_qps);
    sendf(ch, "%s @@d|\n\r", my_left(buf, lbuf, 38));

    send_to_char("@@d|-----------------------------------------------------------------@@g=(@@W Other @@g)=@@d-|\n\r", ch);

    /* player kills/killed */
    sprintf(buf, "@@gPlayer Kill@@d: @@a%d@@d(@@W%d@@d|@@e%d@@d) / @@c%d",
        vch->pcdata->pkills + vch->pcdata->unpkills,
        vch->pcdata->pkills,
        vch->pcdata->unpkills,
        vch->pcdata->pkilled);

    sendf(ch, "@@d| %s ", my_left(buf, lbuf, 36));

    /* mob kills/killed */
    sprintf(buf, "@@gMob Kill@@d: @@a%d @@d/ @@c%d", vch->pcdata->mkills, vch->pcdata->mkilled);
    sendf(ch, "%s @@d|\n\r", my_left(buf, lbuf, 38));

    /* spacer */
    sendf(ch, "%s", "@@d|                                                                             @@d|\n\r");

    /* inventory amount */
    sprintf(buf, "@@gCarrying@@d: @@W%d@@d/@@W%d @@gitems", vch->carry_number, can_carry_n(vch));
    sendf(ch, "@@d| %s ", my_left(buf, lbuf, 36));

    /* wimpy */
    sprintf(buf, "@@gWimpy@@d: @@W%d", vch->wimpy);
    sendf(ch, "%s @@d|\n\r", my_left(buf, lbuf, 38));

    /* weight */
    sprintf(buf, "@@gWeight@@d: @@W%d@@d/@@W%d @@gkg", vch->carry_weight, can_carry_w(vch));
    sendf(ch, "@@d| %s ", my_left(buf, lbuf, 36));

    /* page length */
    sprintf(buf, "@@gPage Length@@d: @@W%d", vch->pcdata->pagelen);
    sendf(ch, "%s @@d|\n\r", my_left(buf, lbuf, 38));

    /* alignment */
    sprintf(buf, "@@gAlignment@@d: @@W%d @@d[%s@@d]",
        vch->alignment,
        IS_GOOD(vch) ? "@@aG" : IS_EVIL(vch) ? "@@eE" : "@@lN");
    sendf(ch, "@@d| %s ", my_left(buf, lbuf, 36));

    /* class order */
    sprintf(buf, "@@gClass Order@@d: @@b%c%c%c%c%c",
        class_table[vch->pcdata->order[0]].who_name[0],
        class_table[vch->pcdata->order[1]].who_name[0],
        class_table[vch->pcdata->order[2]].who_name[0],
        class_table[vch->pcdata->order[3]].who_name[0],
        class_table[vch->pcdata->order[4]].who_name[0]);
    sendf(ch, "%s @@d|\n\r", my_left(buf, lbuf, 38));

    /* charmies */
    charmies = max_orders(vch);
    sprintf(buf, "@@gCharmed Mobiles: @@a%d @@d/ @@c%d", vch->num_followers, charmies);
    sendf(ch, "@@d| %s ", my_left(buf, lbuf, 36));

    if (vch->pcdata->circles_attempted != 0) {
        circle_perc = ((vch->pcdata->circles_landed * 100) / vch->pcdata->circles_attempted);
    }

    sprintf(buf, "@@gPK Circle Stats@@d: (@@W%d@@d|@@e%d@@d) @@c%d%%",
        vch->pcdata->circles_landed,
        vch->pcdata->circles_attempted,
        circle_perc);
    sendf(ch, "%s @@d|\n\r", my_left(buf, lbuf, 38));

    /* spacer */
    send_to_char("@@d|                                                                             @@d|\n\r", ch);

    /* stance */
    sprintf(buf, "@@gStance@@d: @@a%s", stance_app[vch->stance].name);

    if (vch->pcdata && vch->pcdata->autostance && vch->pcdata->autostance[0] != '\0')
        sprintf(buf + strlen(buf), " @@d(@@c%s@@d)", vch->pcdata->autostance);

    sendf(ch, "@@d| %s ", my_left(buf, lbuf, 36));

    switch (vch->position) {
        case POS_DEAD:     sprintf(buf2, "DEAD!!");           break;
        case POS_MORTAL:   sprintf(buf2, "Mortally Wounded"); break;
        case POS_INCAP:    sprintf(buf2, "Incapaciated");     break;
        case POS_STUNNED:  sprintf(buf2, "Stunned");          break;
        case POS_SLEEPING: sprintf(buf2, "Sleeping");         break;
        case POS_RESTING:  sprintf(buf2, "Resting");          break;
        case POS_STANDING: sprintf(buf2, "Standing");         break;
        case POS_FIGHTING: sprintf(buf2, "Fighting");         break;
        case POS_WRITING:  sprintf(buf2, "Writing");          break;
        case POS_BUILDING: sprintf(buf2, "Building");         break;
        case POS_RIDING:   sprintf(buf2, "Riding");           break;
        default:           sprintf(buf2, "NO IDEA");          break;
    }

    /* special case for someone who's mounted */
    if (vch->riding && vch->position == POS_STANDING)
        sprintf(buf2, "Riding");

    /* status */
    sprintf(buf, "@@gStatus@@d: @@W%s ", buf2);
    if (vch->pcdata->condition[COND_DRUNK] > 10)
        safe_strcat(MSL, buf, "@@y[D] ");
    if (vch->pcdata->condition[COND_THIRST] == 0)
        safe_strcat(MSL, buf, "@@y[T] ");
    if (vch->pcdata->condition[COND_FULL] == 0)
        safe_strcat(MSL, buf, "@@y[H] ");

    sendf(ch, "%s @@d|\n\r", my_left(buf, lbuf, 38));

    /* iscore, won't show in score <immortal> */
    if (IS_IMMORTAL(vch) && ch == vch) {
        send_to_char("@@d|---------------------------------------------------------@@g=(@@l I@@Bmmortal @@lI@@Bnfo @@g)=@@d-|\n\r", ch);

        /* wizinvis */
        sprintf(buf, "@@gWizInvis@@d: @@W%s", IS_SET(vch->act, PLR_WIZINVIS) ? number_comma(vch->invis) : "NO");
        sendf(ch, "@@d| %s ", my_left(buf, lbuf, 36));

        /* holylight */
        sprintf(buf, "@@gHolylight@@d: @@W%s", IS_SET(vch->act, PLR_HOLYLIGHT) ? "YES" : "NO");
        sendf(ch, "%s @@d|\n\r", my_left(buf, lbuf, 38));

        /* wizlock */
        sprintf(buf, "@@gWizlock@@d: %s", wizlock ? "@@eYES" : "@@WNO");
        sendf(ch, "@@d| %s ", my_left(buf, lbuf, 36));

        /* dblxp */
        sprintf(buf, "@@gDouble XP@@d: %s", dbl_xp ? "@@WYES" : "@@gNO");
        sendf(ch, "%s @@d|\n\r", my_left(buf, lbuf, 38));

        /* nopk */
        sprintf(buf, "@@gNo PK: %s", nopk ? "@@eYES" : "@@WNO");
        sendf(ch, "@@d| %s ", my_left(buf, lbuf, 36));

        sendf(ch, "%s @@d|\n\r", my_left("", lbuf, 38));

        /* bamfin */
        sprintf(buf, "@@gBamfin@@d: @@W%s", vch->pcdata->bamfin[0] != '\0' ? vch->pcdata->bamfin : "Not changed.");
        sendf(ch, "@@d| %s @@N@@d|\n\r", my_left(buf, lbuf, 75));

        /* bamfout */
        sprintf(buf, "@@gBamfout@@d: @@W%s", vch->pcdata->bamfout[0] != '\0' ? vch->pcdata->bamfout : "Not changed.");
        sendf(ch, "@@d| %s @@N@@d|\n\r", my_left(buf, lbuf, 75));
    }

    /* the end! */
    send_to_char("@@d'-----------------------------------------------------------------------------'@@N\n\r", ch);
    return;
}

void
do_affected(CHAR_DATA *ch, char *argument)
{

    char                buf[MAX_STRING_LENGTH];
    char                sbuf[MIL], lbuf[MIL], abuf[MIL], dbuf[MIL];
    AFFECT_DATA        *paf;
    char               *a_infra, *a_inv, *a_det_inv, *a_sanc, *a_sneak, *a_hide, *a_prot, *a_enhanced, *a_det_mag,
                       *a_det_hid, *a_det_evil, *a_pass_door, *a_det_poison, *a_fly, *a_know_align, *a_det_undead;
    int                 dr_mod = 0, hr_mod = 0, ac_mod = 0;
    bool                cloak = FALSE;
    CHAR_DATA          *vch = NULL;

    buf[0] = '\0';

    if (!IS_IMMORTAL(ch) || argument[0] == '\0')
        vch = ch;
    else {
        vch = get_char_world(ch, argument);

        if (!vch) {
            send_to_char("Can't find that player.\n\r", ch);
            return;
        }

        if (IS_NPC(vch)) {
            send_to_char("Can't use affected command on mobs.\n\r", ch);
            return;
        }

        if (IS_IMMORTAL(vch) && get_trust(ch) < get_trust(vch)) {
            send_to_char("Can't use affected command on immortals higher than you.\n\r", ch);
            return;
        }
    }

    if (item_has_apply(vch, ITEM_APPLY_INFRA))      a_infra      = "@@gInfra@@g";        else a_infra      = "@@dInfra@@g";
    if (item_has_apply(vch, ITEM_APPLY_INV))        a_inv        = "@@gInvis@@g";        else a_inv        = "@@dInvis@@g";
    if (item_has_apply(vch, ITEM_APPLY_DET_INV))    a_det_inv    = "@@gDet Invis@@g";    else a_det_inv    = "@@dDet Invis@@g";
    if (item_has_apply(vch, ITEM_APPLY_SANC))       a_sanc       = "@@gSanc@@g";         else a_sanc       = "@@dSanc@@g";
    if (item_has_apply(vch, ITEM_APPLY_SNEAK))      a_sneak      = "@@gSneak@@g";        else a_sneak      = "@@dSneak@@g";
    if (item_has_apply(vch, ITEM_APPLY_HIDE))       a_hide       = "@@gHide@@g";         else a_hide       = "@@dHide@@g";
    if (item_has_apply(vch, ITEM_APPLY_PROT))       a_prot       = "@@gProt@@g";         else a_prot       = "@@dProt@@g";
    if (item_has_apply(vch, ITEM_APPLY_ENHANCED))   a_enhanced   = "@@gEnhanced Dam@@g"; else a_enhanced   = "@@dEnhanced Dam@@g";
    if (item_has_apply(vch, ITEM_APPLY_DET_MAG))    a_det_mag    = "@@gDet Magic@@g";    else a_det_mag    = "@@dDet Magic@@g";
    if (item_has_apply(vch, ITEM_APPLY_DET_HID))    a_det_hid    = "@@gDet Hidden@@g";   else a_det_hid    = "@@dDet Hidden@@g";
    if (item_has_apply(vch, ITEM_APPLY_DET_EVIL))   a_det_evil   = "@@gDet Evil@@g";     else a_det_evil   = "@@dDet Evil@@g";
    if (item_has_apply(vch, ITEM_APPLY_PASS_DOOR))  a_pass_door  = "@@gPass Door@@g";    else a_pass_door  = "@@dPass Door@@g";
    if (item_has_apply(vch, ITEM_APPLY_DET_POISON)) a_det_poison = "@@gDet Poison@@g";   else a_det_poison = "@@dDet Poison@@g";
    if (item_has_apply(vch, ITEM_APPLY_KNOW_ALIGN)) a_know_align = "@@gKnow Align@@g";   else a_know_align = "@@dKnow Align@@g";
    if (item_has_apply(vch, ITEM_APPLY_DET_UNDEAD)) a_det_undead = "@@gDet Undead@@g";   else a_det_undead = "@@dDet Undead@@g";
    if (item_has_apply(vch, ITEM_APPLY_FLY)
        || (!IS_NPC(vch) && vch->pcdata->learned[gsn_fly] == 101))
                                                   a_fly        = "@@gFly";          else a_fly        = "@@dFly";


    send_to_char("@@N@@d.----------------------------------------------------------@@g=( @@aItem Applies @@g)=@@d-.\n\r", ch);
    sendf(ch, "@@d| %s, %s, %s, %s, %s, %s, %s, %s, %s   @@d|\n\r@@d|  %s, %s, %s, %s, %s, %s, %s   @@d|\n\r",
        a_infra, a_inv, a_det_inv, a_sanc, a_sneak, a_hide, a_prot, a_enhanced, a_det_mag,
        a_det_hid, a_det_evil, a_pass_door, a_det_poison, a_fly, a_know_align, a_det_undead);

    if (vch->first_affect == NULL) {
        send_to_char("@@d'-----------------------------------------------------------------------------'@@N\n\r", ch);
        return;
    }

    send_to_char("@@d|-----------------------------------------------------------@@g=( @@aAffected By @@g)=@@d-|\n\r", ch);
    send_to_char("@@d| @@gSkill/Spell              @@d| @@gLocation        @@d|       @@gAmount @@d| @@gDuration        @@d|\n\r", ch);
    send_to_char("@@d|--------------------------+-----------------+--------------+-----------------|\n\r", ch);

    for (paf = vch->first_affect; paf != NULL; paf = paf->next) {
        sbuf[0] = 0; lbuf[0] = 0; abuf[0] = 0; dbuf[0] = 0;

        if      (paf->location == APPLY_DAMROLL)
            dr_mod += paf->modifier;
        else if (paf->location == APPLY_HITROLL)
            hr_mod += paf->modifier;
        else if (paf->location == APPLY_AC)
            ac_mod += paf->modifier;


        if (paf->location > APPLY_NONE && paf->duration == -1 && paf->type == gsn_emount)
            sprintf(buf, "while mounted");
        else if (paf->location > APPLY_NONE && paf->duration == -1)
            sprintf(buf, "permanently");
        else
            sprintf(buf, "%d hours", paf->duration);

        sendf(ch, "@@d| @@g%s @@d| @@g%s @@d| @@g%s @@d| @@g%s @@d|\n\r",
            my_left(skill_table[paf->type].name, sbuf, 24),
            my_left((paf->location > APPLY_NONE) ? affect_loc_name(paf->location) : "", lbuf, 15),
            my_right((paf->location > APPLY_NONE) ? number_comma(paf->modifier) : "", abuf, 12),
            my_left(buf, dbuf, 15));
    }

    if (   IS_AFFECTED(vch, AFF_CLOAK_REFLECTION)
        || IS_AFFECTED(vch, AFF_CLOAK_FLAMING)
        || IS_AFFECTED(vch, AFF_CLOAK_ABSORPTION)
        || IS_AFFECTED(vch, AFF_CLOAK_ADEPT)
        || IS_AFFECTED(vch, AFF_CLOAK_REGEN)
        || IS_AFFECTED(vch, AFF_CLOAK_MANA)
       )
        cloak = TRUE;

    if (dr_mod == 0 && hr_mod == 0 && ac_mod == 0 && !cloak && !ch->first_shield) {
        send_to_char("@@d'-----------------------------------------------------------------------------'@@N\n\r", ch);
        return;
    }

    send_to_char("@@d|-----------------------------------------------------------------------------|\n\r", ch);

    if (dr_mod != 0 || hr_mod != 0 || ac_mod != 0) {
        buf[0] = 0;

        if (dr_mod != 0)
            sprintf(buf + strlen(buf), "@@gDamroll: @@W%d   ", dr_mod);
        if (hr_mod != 0)
            sprintf(buf + strlen(buf), "@@gHitroll: @@W%d   ", hr_mod);
        if (ac_mod != 0)
            sprintf(buf + strlen(buf), "@@gArmor Class: @@W%d   ", ac_mod);

        sendf(ch, "@@d| @@WTotals:   %s @@d|\n\r", my_left(buf, sbuf, 65));
    }

    if (cloak) {
        buf[0] = 0;
        if (IS_AFFECTED(vch, AFF_CLOAK_MANA))       { safe_strcat(MSL, buf, " "); safe_strcat(MSL, buf, cloak_table[CLOAK_MANA].name);    }
        if (IS_AFFECTED(vch, AFF_CLOAK_FLAMING))    { safe_strcat(MSL, buf, " "); safe_strcat(MSL, buf, cloak_table[CLOAK_FLAMING].name); }
        if (IS_AFFECTED(vch, AFF_CLOAK_ABSORPTION)) { safe_strcat(MSL, buf, " "); safe_strcat(MSL, buf, cloak_table[CLOAK_ABSORB].name);  }
        if (IS_AFFECTED(vch, AFF_CLOAK_REFLECTION)) { safe_strcat(MSL, buf, " "); safe_strcat(MSL, buf, cloak_table[CLOAK_REFLECT].name); }
        if (IS_AFFECTED(vch, AFF_CLOAK_ADEPT))      { safe_strcat(MSL, buf, " "); safe_strcat(MSL, buf, cloak_table[CLOAK_ADEPT].name);   }
        if (IS_AFFECTED(vch, AFF_CLOAK_REGEN))      { safe_strcat(MSL, buf, " "); safe_strcat(MSL, buf, cloak_table[CLOAK_REGEN].name);   }

        sendf(ch, "@@d| @@WCloaks:  %s @@d|\n\r", my_left(buf, sbuf, 66));
    }

    if (vch->first_shield) {
        SHIELD_DATA *shield;
        buf[0] = 0;

        for (shield = vch->first_shield; shield != NULL; shield = shield->next) {
            safe_strcat(MSL, buf, " ");
            safe_strcat(MSL, buf, shield_table[shield->index].name);
        }

        sendf(ch, "@@d| @@WShield:  @@g%s @@d|\n\r", my_left(buf, sbuf, 66));
    }

    send_to_char("@@d'-----------------------------------------------------------------------------'@@N\n\r", ch);

    return;
}

char               *const day_name[] = {
    "the Moon", "the Bull", "Deception", "Thunder", "Freedom",
    "the Great Gods", "the Sun"
};

/* 
 * Number of months reduced from 17 (!) to 8
 * This is to bring the time it takes a character to age on mud year
 * down from 200+ rl hrs to 64 rl hrs
 * (Player's age stats were screwed in base merc!)
 */

char               *const month_name[] = {
    "the Frost Giant", "the Old Forces",
    "the Grand Struggle", "Futility",
    "the Dark Shades", "the Long Shadows",
    "the Ancient Darkness", "the Great Evil"
};

void
do_time(CHAR_DATA *ch, char *argument)
{
    extern time_t       boot_time, reboot_time;
    extern char        *last_reboot_by;
    char                buf1[30];
    char                buf2[30];
    char                buf3[30];
    char                buf4[50];
    char                buf5[50];
    char               *_buf1 = buf1;
    char               *_buf2 = buf2;
    char               *_buf3 = buf3;
    char               *_buf4 = buf4;
    char               *_buf5 = buf5;
    char               *suf;
    int                 day;

    strcpy(buf1, (char *) ctime(&boot_time));
    strcpy(buf2, (char *) ctime(&reboot_time));
    strcpy(buf3, (char *) ctime(&current_time));
    buf4[0] = 0;
    buf5[0] = 0;

    while (*++_buf1);
    _buf1--;
    *_buf1 = 0;
    while (*++_buf2);
    _buf2--;
    *_buf2 = 0;
    while (*++_buf3);
    _buf3--;
    *_buf3 = 0;

    day = time_info.day + 1;

    if (day > 4 && day < 20)
        suf = "th";
    else if (day % 10 == 1)
        suf = "st";
    else if (day % 10 == 2)
        suf = "nd";
    else if (day % 10 == 3)
        suf = "rd";
    else
        suf = "th";

    sendf(ch, "It is %d o'clock %s, Day of %s, %d%s the Month of %s.\n\r\n\r",
        (time_info.hour % 12 == 0) ? 12 : time_info.hour % 12,
        time_info.hour >= 12 ? "pm" : "am", day_name[day % 7], day, suf, month_name[time_info.month]
        );

    sendf(ch, "%s started up at %s (%s ago)\n\r", mudnamecolor, buf1, duration(current_time - boot_time, _buf4)
        );

    if (reboot_time > 0)
        sendf(ch, "Last reboot at      %s (%s ago)%s%s\n\r",
            buf2, duration(current_time - reboot_time, _buf5), !strcmp(last_reboot_by, "") ? "" : " by ", last_reboot_by);

    sendf(ch, "The time is now     %s\n\r", buf3);
    return;
}

void
do_weather(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    char                buf2[MSL];

    static char        *const sky_look[4] = {
        "cloudless",
        "cloudy",
        "rainy",
        "lit by flashes of lightning"
    };
    buf[0] = '\0';
    buf2[0] = '\0';

    if (!IS_OUTSIDE(ch)) {
        send_to_char("You can't see the weather indoors.\n\r", ch);
        return;
    }

    sprintf(buf, "The sky is %s and %s.\n\r",
        sky_look[weather_info.sky], weather_info.change >= 0 ? "a warm southerly breeze blows" : "a cold northern gust blows");
    switch (weather_info.moon_loc) {
        case MOON_DOWN:
            safe_strcat(MSL, buf, "The moon is not in the sky.\n\r");
            break;
        case MOON_RISE:
            sprintf(buf2, "A %s @@yMoon@@N is just rising.\n\r", get_moon_phase_name());
            safe_strcat(MSL, buf, buf2);
            break;
        case MOON_LOW:
            sprintf(buf2, "A %s @@yMoon@@N is low on the horizon.\n\r", get_moon_phase_name());
            safe_strcat(MSL, buf, buf2);
            break;
        case MOON_PEAK:
            sprintf(buf2, "A %s @@yMoon@@N is high above you.\n\r", get_moon_phase_name());
            safe_strcat(MSL, buf, buf2);
            break;
        case MOON_FALL:
            sprintf(buf2, "A %s @@yMoon@@N is falling.\n\r", get_moon_phase_name());
            safe_strcat(MSL, buf, buf2);
            break;

        case MOON_SET:
            sprintf(buf2, "A %s @@yMoon@@N has just set.\n\r", get_moon_phase_name());
            safe_strcat(MSL, buf, buf2);
            break;

    }

    send_to_char(buf, ch);
    return;
}

bool
send_help(void *to, char *argument, int type, bool desc)
{
    char helplistname[MSL];
    char help[MSL * 8];
    char file[MAX_INPUT_LENGTH];
    char c;
    FILE *helplistfile, *helpfile;
    CHAR_DATA *ch = NULL;
    DESCRIPTOR_DATA *d = NULL;
    bool found = FALSE;

    if (!desc)
        ch = (CHAR_DATA *)to;
    else
        d = (DESCRIPTOR_DATA *)to;

    sprintf(helplistname, "%s%s", HELP_DIR, HELP_FILE);

    if ((helplistfile = fopen(helplistname, "r")) == NULL) {
        if (!desc)
            send_to_char("Error opening help list file. Contact an Immortal.\n\r", ch);

        return TRUE;
    }

    for (;;) {
        if (feof(helplistfile) || (c = fgetc(helplistfile)) == '0')
            break;
        else
            ungetc(c, helplistfile);

        temp_fread_string(helplistfile, help);
        temp_fread_string(helplistfile, file);

        /* there should always be an \n here! */
        fgetc(helplistfile);

        switch (type) {
            case HELP_IMM:
                if (!str_prefix("imm_", help) && !str_prefix(argument, help + 4))
                    found = TRUE;
                break;
            case HELP_BUILD:
                if (!str_prefix("build_", help) && !str_prefix(argument, help + 6))
                    found = TRUE;
                break;
            case HELP_RULES:
                if (!str_prefix("rules_", help) && !str_prefix(argument, help + 6))
                    found = TRUE;
                break;
            case HELP_MAP:
                if (!str_prefix("map_", help) && !str_prefix(argument, help + 4))
                    found = TRUE;
                break;
            case HELP_NORMAL:
                if (   str_prefix("imm_", help)
                    && str_prefix("build_", help)
                    && str_prefix("rules_", help)
                    && str_prefix("map_", help)
                    && !str_prefix(argument, help))
                    found = TRUE;
                break;
            default:
                break;
        }

        if (found) {
            int cnt;
            char _temp[MSL * 8];

            sprintf(_temp, "%s%s", HELP_DIR, file);

            if ((helpfile = fopen(_temp, "r")) == NULL) {
                if (!desc)
                    send_to_char("Error opening help file.\n\r", ch);

                fclose(helplistfile);
                return FALSE;
            }

            while (!feof(helpfile)) {
                char *temp, *myhelp;

                memset(help, '\0', sizeof(help));
                memset(_temp, '\0', sizeof(_temp));

                if ((cnt = fread(help, MAX_STRING_LENGTH * 5, 1, helpfile)) < 0)
                    break;

                myhelp = help;

                /* skip first . */
                if (*myhelp == '.')
                    myhelp++;

                /* we have \n's in the text files, but our standard is to send \n\r, so
                 * replace \n's with \n\r's */
                for (temp = _temp; *myhelp != '\0'; myhelp++) {
                    if (*myhelp == '\n' && *(myhelp + 1) != '\r') {
                        *temp++ = '\n'; *temp++ = '\r';
                    }
                    else
                        *temp++ = *myhelp;
                }

                *temp = '\0';

                if (!desc)
                    send_to_char(_temp, ch);
                else
                    write_to_buffer(d, _temp, 0);
            }

            fclose(helpfile);
            fclose(helplistfile);
            return TRUE;
        }
    }

    fclose(helplistfile);
    return FALSE;
}

#define MAX_SHOW_HELPS 4096

int
show_helps_cmp(const void *x, const void *y)
{
    char           *dx = *(char **) x;
    char           *dy = *(char **) y;

    return strcmp(dx, dy);
}

void
show_helps(CHAR_DATA *ch, int type)
{
    FILE *helplistfile, *helpfile;
    char helplistname[MSL];
    char buf[MSL], buf2[MSL];
    int cnt = 0, helpcnt = 0;
    bool found = FALSE;
    char _help[MSL], file[MSL];
    char *help;
    void **helps;
    int c;
    int col = 0, cols = 4, row = 0, rows = 0;

    sprintf(helplistname, "%s%s", HELP_DIR, HELP_FILE);

    if ((helplistfile = fopen(helplistname, "r")) == NULL) {
        send_to_char("Error opening help list file. Contact an Immortal.\n\r", ch);
        return;
    }

    helps = (void **)malloc(sizeof(char *) * MAX_SHOW_HELPS);
    memset(helps, 0, sizeof(char *) * MAX_SHOW_HELPS);

    if (helps == NULL) {
        send_to_char("Eek.\n\r", ch);
        return;
    }

    buf[0] = '\0';

    for (;;) {
        if (feof(helplistfile) || (c = fgetc(helplistfile)) == '0')
            break;
        else
            ungetc(c, helplistfile);

        temp_fread_string(helplistfile, _help);
        temp_fread_string(helplistfile, file);

        help = _help;

        /* there should always be an \n here! */
        fgetc(helplistfile);

        switch (type) {
            case HELP_IMM:
                if (!str_prefix("imm_", _help)) {
                    help += 4;
                    found = TRUE;
                }
                else
                    continue;

                break;
            case HELP_BUILD:
                if (!str_prefix("build_", help)) {
                    help += 6;
                    found = TRUE;
                }
                else
                    continue;

                break;
            case HELP_RULES:
                if (!str_prefix("rules_", help)) {
                    help += 6;
                    found = TRUE;
                }
                else
                    continue;

                break;
            case HELP_MAP:
                if (!str_prefix("map_", help)) {
                    help += 4;
                    found = TRUE;
                }
                else
                    continue;

                break;
            case HELP_NORMAL:
                if (   str_prefix("imm_", help)
                    && str_prefix("build_", help)
                    && str_prefix("rules_", help)
                    && str_prefix("map_", help)) {
                    found = TRUE;
                }
                else
                    continue;

                break;
            default:
                break;
        }

        if (found) {
            sprintf(buf2, "%s%s", HELP_DIR, file);

            /* if the corresponding file doesn't exist, don't show that a help exists for it */
            if ((helpfile = fopen(buf2, "r")) == NULL)
                continue;
            else {
                fclose(helpfile);

                helps[helpcnt] = malloc(strlen(help) + 1);
                strcpy(helps[helpcnt], help);

                helpcnt++;

                if (helpcnt >= MAX_SHOW_HELPS - 1)
                    break;
            }
        }
    }

    switch (type) {
        case HELP_IMM:   send_to_char("@@N@@d.------------------------------------------------------@@g=( @@aImmortal Helps @@g)=@@d-.@@N\n\r", ch); break;
        case HELP_BUILD: send_to_char("@@N@@d.-------------------------------------------------------@@g=( @@aBuilder Helps @@g)=@@d-.@@N\n\r", ch); break;
        case HELP_RULES: send_to_char("@@N@@d.----------------------------------------------------------@@g=( @@aRule Helps @@g)=@@d-.@@N\n\r", ch); break;
        case HELP_MAP:   send_to_char("@@N@@d.----------------------------------------------------------------@@g=( @@aMaps @@g)=@@d-.@@N\n\r", ch); break;
        default:         send_to_char("@@N@@d.---------------------------------------------------------------@@g=( @@aHelps @@g)=@@d-.@@N\n\r", ch); break;
    }

    qsort(helps, helpcnt, sizeof(void *), show_helps_cmp);

    rows = helpcnt / cols + ((helpcnt % cols != 0) ? 1 : 0);

    for (row = 0; row < rows; row++) {
        send_to_char("@@d|", ch);

        for (col = 0; col < cols; col++) {
            cnt = col * rows + row;

            if (cnt < helpcnt) {
                if (col % 2 == 0)
                    sendf(ch, " @@W%-16.16s @@d|", (char *)helps[cnt]);
                else
                    sendf(ch, " @@g%-16.16s @@d|", (char *)helps[cnt]);
            }
            else
                sendf(ch, " %-16.16s @@d|", "");
        }

        send_to_char("@@N\n\r", ch);
    }

    send_to_char("@@d'---------------------------------------------------------------------------'@@N\n\r", ch);

    for (cnt = 0; cnt < helpcnt; cnt++)
        free(helps[cnt]);

    free(helps);
    fclose(helplistfile);
    return;
}

void
do_help(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];

    if (argument[0] == '\0')
        argument = "summary";

    one_argument(argument, arg);

    if (send_help(ch, arg, HELP_NORMAL, FALSE))
        return;

    if (skill_lookup(arg) >= 0) {
        do_shelp(ch, arg);
        return;
    }

    send_to_char("No help on that word.\n\r", ch);
    return;
}

void
do_helps(CHAR_DATA *ch, char *argument)
{
    send_to_char("@@N@@gBelow is a list of helps you can look up using @@ahelp <keyword>@@g:\n\r\n\r", ch);
    show_helps(ch, HELP_NORMAL);
    return;
}

void
do_ihelps(CHAR_DATA *ch, char *argument)
{
    send_to_char("@@N@@gBelow is a list of Immortal helps you can look up using @@awizhelp <keyword>@@g:\n\r\n\r", ch);
    show_helps(ch, HELP_IMM);
    return;
}

/*
 * New 'who' command originally by Alander of Rivers of Mud.
 * Changed a lot since then though :P
 * List comes in 4 parts: Imms, Adepts, remorts then morts
 */

#define SHOW_IMMORT     0
#define SHOW_AVATAR     1
#define SHOW_ADEPT      2
#define SHOW_REMORT     3
#define SHOW_MORTAL     4
#define SHOW_FINISH     5

void do_who(CHAR_DATA *ch, char *argument)
{
    DESCRIPTOR_DATA    *d;

    char                buf[MAX_STRING_LENGTH * 10];
    char                buf2[MAX_STRING_LENGTH * 4];
    char                buf3[MAX_STRING_LENGTH * 4];
    char                buf4[MAX_STRING_LENGTH * 4];
    char                buf5[MAX_STRING_LENGTH * 4];
    char                buf6[MAX_STRING_LENGTH * 4];
    char                header[MSL * 4];
    char                arg[MAX_STRING_LENGTH];
    char                fgs[MAX_STRING_LENGTH * 4];
    char                clan[64];
    char                col1[20];
    char                col2[20];
    char                col3[20];
    char                col4[20];
    char                col5[20];
    bool                fImmortalOnly = FALSE;
    bool                fAvatarOnly = FALSE;
    bool                fadeptonly = FALSE;
    bool                fremortonly = FALSE;
    bool                fMCCPOnly = FALSE;
    int                 cnt, slength, nlength, nMatch;
    extern int          max_players;
    extern POL_DATA     politics_data;
    int                 list;
    int                 number[SHOW_FINISH];
    bool                idle = FALSE;
    bool                cangroup = FALSE;
    bool                showally = FALSE;
    bool                showarena = FALSE;
    int                 stop_counter = 0;

    buf[0] = '\0';
    buf2[0] = '\0';
    buf3[0] = '\0';
    buf4[0] = '\0';
    buf5[0] = '\0';
    buf6[0] = '\0';
    header[0] = '\0';
    col1[0] = '\0';
    col2[0] = '\0';
    col3[0] = '\0';
    col4[0] = '\0';
    col5[0] = '\0';

    fImmortalOnly = FALSE;
    nMatch = 0;

    argument = one_argument(argument, arg);

    if (!str_cmp(arg, "group"))
        cangroup = TRUE;
    else if (!str_cmp(arg, "avatar"))
        fAvatarOnly = TRUE;
    else if (!str_cmp(arg, "adept"))
        fadeptonly = TRUE;
    else if (!str_cmp(arg, "remort"))
        fremortonly = TRUE;
    else if (!str_cmp(arg, "imm"))
        fImmortalOnly = TRUE;
    else if (!str_cmp(arg, "ally"))
        showally = TRUE;
    else if (!str_cmp(arg, "arena"))
        showarena = TRUE;
    else if (!str_cmp(arg, "mccp"))
        fMCCPOnly = TRUE;

    for (list = SHOW_IMMORT; list < SHOW_FINISH; list++) {
        number[list] = 0;

        for (d = first_desc; d != NULL; d = d->next) {
            CHAR_DATA          *wch;

            wch = (d->original != NULL) ? d->original : d->character;

            if (d->connected != CON_PLAYING || !can_see(ch, wch))
                continue;

            if (   (list == SHOW_IMMORT && wch->level < LEVEL_HERO)
                || (list == SHOW_AVATAR && wch->pcdata->avatar == FALSE)
                || (list == SHOW_AVATAR && wch->level >= LEVEL_HERO)
                || (list == SHOW_ADEPT  && wch->adept_level < 1)
                || (list == SHOW_ADEPT  && wch->level >= LEVEL_HERO)
                || (list == SHOW_ADEPT  && wch->pcdata->avatar == TRUE)
                || (list == SHOW_REMORT && !is_remort(wch))
                || (list == SHOW_REMORT && wch->level >= LEVEL_HERO)
                || (list == SHOW_REMORT && wch->adept_level > 0)
                || (list == SHOW_MORTAL && is_remort(wch))
                || (list == SHOW_MORTAL && wch->level >= LEVEL_HERO)
               )
                continue;

            if ((fImmortalOnly && wch->level < LEVEL_HERO)
                || (fadeptonly && ((wch->adept_level < 1) || wch->level >= LEVEL_HERO || wch->pcdata->avatar))
                || (fremortonly && (!is_remort(wch)
                        || (wch->level >= LEVEL_HERO)
                        || (wch->adept_level > 0)
                    )
                )
                || (fAvatarOnly && (wch->pcdata->avatar == FALSE || wch->level >= LEVEL_HERO))
                || (fMCCPOnly && (!wch->desc || wch->desc->out_compress == NULL))
                || (cangroup && !can_group(ch, wch))
                || (showarena && wch->pcdata->in_arena == FALSE)
                || ( showally &&
                    (   ch->pcdata->clan == 0
                     || wch->pcdata->clan == 0
                     || (   ch->pcdata->clan != wch->pcdata->clan
                         && politics_data.diplomacy[ch->pcdata->clan][wch->pcdata->clan] < 450
                        )
                    )
                   )
                )
                continue;

            nMatch++;
            number[list]++;
        }
    }

    sprintf(header, "@@d.-----------------------------------------------------------@@g=( @@WWHO Listing @@g)=@@d-.\n\r");
    sprintf(buf4, "%s @@d(@@W%d@@d/@@g%d @@dplayer%s online)", mudnamecolor, nMatch, max_players, max_players == 1 ? "" : "s");
    sprintf(buf2, "@@d| %s @@d|\n\r", center_text(buf4, 75));
    safe_strcat(MSL, header, buf2);

    if (nosave) {
        sprintf(buf4, "@@eWarning: @@gMUD in @@WNOSAVE@@g mode: @@W%s@@N",
            strerror(writeerrno));

        sprintf(buf2, "@@d| %s @@d|\n\r", center_text(buf4, 75));
        safe_strcat(MSL, header, buf2);
    }

    /* added it to startup instead -E */
    /*
       if (dbl_xp == TRUE)
       {
       sprintf( buf5, "@@eDouble Experience is on!\n\r");
       sprintf( buf6, "@@d| %s @@d|\n\r", center_text( buf5, 75) );
       safe_strcat( MSL, header, buf6 );
       }
     */
    safe_strcat(MSL, header, "@@d|-----------------------------------------------------------------------------|\n\r");
    safe_strcat(MSL, header, "@@d| @@mSo Mo An Ki Ne @@d|                 |                                          |\n\r");
    safe_strcat(MSL, header, "@@d| @@bMa Cl Th Wa Ps @@d| @@gRace Clan Flags @@d| @@gPlayer (Title)                           @@d|\n\r");
    send_to_char(header, ch);

    buf2[0] = '\0';
    buf4[0] = '\0';

    for (list = SHOW_IMMORT; list < SHOW_FINISH; list++) {
        stop_counter = 0;

        if (number[list] == 0)
            continue;

        switch (list) {
            case SHOW_IMMORT:
                safe_strcat(MAX_STRING_LENGTH, buf,
                    "@@d|----------------+-----------------+----------------------------@@g=( @@lI@@Bmmorts @@g)=@@d-|\n\r");
                break;
            case SHOW_AVATAR:
                safe_strcat(MAX_STRING_LENGTH, buf,
                    "@@d|----------------+-----------------+----------------------------@@g=( @@rA@@Gvatars @@g)=@@d-|\n\r");
                break;
            case SHOW_ADEPT:
                safe_strcat(MAX_STRING_LENGTH, buf,
                    "@@d|----------------+-----------------+----------------------------@@g=( @@yA@@bdepts  @@g)=@@d-|\n\r");
                break;
            case SHOW_REMORT:
                safe_strcat(MAX_STRING_LENGTH, buf,
                    "@@d|----------------+-----------------+----------------------------@@g=( @@pR@@memorts @@g)=@@d-|\n\r");
                break;
            case SHOW_MORTAL:
                safe_strcat(MAX_STRING_LENGTH, buf,
                    "@@d|----------------+-----------------+----------------------------@@g=( @@aM@@cortals @@g)=@@d-|\n\r");
                break;
        }

        for (d = first_desc; d != NULL; d = d->next) {
            CHAR_DATA          *wch;
            char const         *class;

            wch = (d->original != NULL) ? d->original : d->character;

            if (d->connected != CON_PLAYING || !can_see(ch, wch))
                continue;

            if (   (list == SHOW_IMMORT && wch->level < LEVEL_HERO)
                || (list == SHOW_AVATAR && wch->pcdata->avatar == FALSE)
                || (list == SHOW_AVATAR && wch->level >= LEVEL_HERO)
                || (list == SHOW_ADEPT  && wch->adept_level < 1)
                || (list == SHOW_ADEPT  && wch->level >= LEVEL_HERO)
                || (list == SHOW_ADEPT  && wch->pcdata->avatar == TRUE)
                || (list == SHOW_REMORT && !is_remort(wch))
                || (list == SHOW_REMORT && wch->level >= LEVEL_HERO)
                || (list == SHOW_REMORT && wch->adept_level > 0)
                || (list == SHOW_MORTAL && is_remort(wch))
                || (list == SHOW_MORTAL && wch->level >= LEVEL_HERO)
               )
                continue;

            if ((fImmortalOnly && wch->level < LEVEL_HERO)
                || (fadeptonly && ((wch->adept_level < 1) || wch->level >= LEVEL_HERO || wch->pcdata->avatar))
                || (fremortonly && (!is_remort(wch)
                        || (wch->level >= LEVEL_HERO)
                        || (wch->adept_level > 0)
                    )
                )
                || (fAvatarOnly && (wch->pcdata->avatar == FALSE || wch->level >= LEVEL_HERO))
                || (fMCCPOnly && (!wch->desc || wch->desc->out_compress == NULL))
                || (cangroup && !can_group(ch, wch))
                || (showarena && wch->pcdata->in_arena == FALSE)
                || ( showally &&
                    (   ch->pcdata->clan == 0
                     || wch->pcdata->clan == 0
                     || (   ch->pcdata->clan != wch->pcdata->clan
                         && politics_data.diplomacy[ch->pcdata->clan][wch->pcdata->clan] < 450
                        )
                    )
                   )
                )
                continue;

            if (stop_counter++ > 45)
                continue;

            class = class_table[wch->class].who_name;

            if (str_cmp(wch->pcdata->who_name, "off")) {
                class = wch->pcdata->who_name;

                strcpy(buf3, " ");

                strcpy(buf4, class);
                safe_strcat(MSL, buf3, buf4);
            }
            else {
                switch (wch->level) {
                    default:
                        break;
                    case MAX_LEVEL - 0:
                        class = "@@l-* CREATOR *-@@g ";
                        break;
                    case MAX_LEVEL - 1:
                        class = "@@B-= SUPREME =-@@g ";
                        break;
                    case MAX_LEVEL - 2:
                        class = "@@a--  DEITY  --@@g ";
                        break;
                    case MAX_LEVEL - 3:
                        class = "@@c - IMMORTAL- @@g ";
                        break;
                    case MAX_LEVEL - 4:
                        class = "@@W    ADEPT  @@N   ";
                        break;
                }

                {
                    if (wch->level >= (MAX_LEVEL - 4)
                        || str_cmp(wch->pcdata->who_name, "off")
                        ) {
                        switch (wch->level) {
                            case MAX_LEVEL - 0:
                                sprintf(buf3, "@@l %s@@g", class);
                                break;
                            case MAX_LEVEL - 1:
                                sprintf(buf3, "@@B %s@@g", class);
                                break;
                            case MAX_LEVEL - 2:
                                sprintf(buf3, "@@a %s@@g", class);
                                break;
                            case MAX_LEVEL - 3:
                                sprintf(buf3, "@@c %s@@g", class);
                                break;
                            default:
                                sprintf(buf3, "@@W %s@@g", class);
                                break;
                        }
                    }
                    else {
                        int                 matchup = 0;

                        buf4[0] = '\0';
                        buf3[0] = '\0';

                        for (cnt = 0; cnt < MAX_CLASS; cnt++) {
                            switch (cnt) {
                                case 0:
                                    matchup = 0;
                                    break;
                                case 1:
                                    matchup = 4;
                                    break;
                                case 2:
                                    matchup = 1;
                                    break;
                                case 3:
                                    matchup = 2;
                                    break;
                                case 4:
                                    matchup = 3;
                                    break;
                                default:
                                    matchup = 0;
                                    break;
                            }

                            if (wch->lvl2[matchup] > 0)
                                sprintf(buf4, "@@m%3d@@N", wch->lvl2[matchup]);
                            else {
                                if (wch->lvl[cnt] <= 0)
                                    sprintf(buf4, "@@d%3d@@N", 0);
                                else
                                    sprintf(buf4, "@@b%3d@@N", wch->lvl[cnt]);
                            }

                            safe_strcat(MAX_STRING_LENGTH, buf3, buf4);
                        }
                    }
                }
            }

            col1[0] = '\0';
            col2[0] = '\0';
            col3[0] = '\0';
            col4[0] = '\0';
            col5[0] = '\0';
            idle = FALSE;

            if (wch->timer > 5)
                idle = TRUE;

            if (wch->position == POS_BUILDING && !idle)
                safe_strcat(20, col1, "@@lB");
            else if (wch->position == POS_BUILDING && idle)
                safe_strcat(20, col1, "@@eB");
            else if (wch->position == POS_WRITING && !idle)
                safe_strcat(20, col1, "@@lW");
            else if (wch->position == POS_WRITING && idle)
                safe_strcat(20, col1, "@@eW");
            else if (idle)
                safe_strcat(20, col1, "@@eI");
            else
                safe_strcat(20, col1, " ");
            if (IS_SET(wch->config, PLR_ANSWERING) && (IS_SET(wch->pcdata->pflags, PFLAG_XAFK)
                    || IS_SET(wch->pcdata->pflags, PFLAG_AFK)))
                safe_strcat(20, col2, "@@gM@@N");
            else if (IS_SET(wch->pcdata->pflags, PFLAG_XAFK))
                safe_strcat(20, col2, "@@gX@@N");
            else if (IS_SET(wch->pcdata->pflags, PFLAG_AFK))
                safe_strcat(20, col2, "@@gA@@N");
            else
                safe_strcat(20, col2, " ");

            if (IS_SET(wch->pcdata->pflags, PFLAG_CLAN_BOSS))
                safe_strcat(20, col3, "@@r*");
            else if (IS_SET(wch->pcdata->pflags, PFLAG_CLAN_2LEADER))
                safe_strcat(20, col3, "@@cL");
            else if (IS_SET(wch->pcdata->pflags, PFLAG_CLAN_LEADER))
                safe_strcat(20, col3, "@@GL");
            else if (IS_SET(wch->pcdata->pflags, PFLAG_CLAN_TREASURER))
                safe_strcat(20, col3, "@@GT");
            else if (IS_SET(wch->pcdata->pflags, PFLAG_CLAN_ARMOURER))
                safe_strcat(20, col3, "@@d!");
            else if (IS_SET(wch->pcdata->pflags, PFLAG_CLAN_DESERTER))
                safe_strcat(20, col3, "@@yD");
            else
                safe_strcat(20, col3, " ");

            if (IS_SET(wch->pcdata->pflags, PFLAG_SAFE))
                safe_strcat(20, col4, "@@WH");
            else if (IS_SET(wch->pcdata->pflags, PFLAG_PKOK))
                safe_strcat(20, col4, "@@cP");
            else
                safe_strcat(20, col4, " ");

            if (!IS_SET(wch->act, PLR_WIZINVIS)
                && (IS_SET(wch->act, PLR_KILLER)
                    || IS_SET(wch->act, PLR_THIEF)
                )
                )
                safe_strcat(20, col5, "@@a$");
            else if (IS_SET(wch->act, PLR_WIZINVIS)
                && (IS_SET(wch->act, PLR_KILLER)
                    || IS_SET(wch->act, PLR_THIEF)
                )
                )
                safe_strcat(20, col5, "@@aV");
            else if (IS_SET(wch->act, PLR_WIZINVIS))
                safe_strcat(20, col5, "@@dV");
            else if (IS_IMMORTAL(ch) && wch->stance == STANCE_AMBUSH)
                safe_strcat(20, col5, "@@eN");
            else if (IS_IMMORTAL(ch) && wch->stance == STANCE_AC_BEST)
                safe_strcat(20, col5, "@@eS");
            else
                safe_strcat(20, col5, " ");

            if (is_in_duel(wch, DUEL_STAGE_GO))
                strcpy(col5, "@@a#");

            /* don't show clan "None" for imms on wholist. */
            if (!IS_IMMORTAL(wch) || wch->pcdata->clan != 0)
                strcpy(clan, clan_table[wch->pcdata->clan].clan_abbr);
            else
                strcpy(clan, "");

            sprintf(fgs, "%3s %5s %s%s%s%s%s",
                race_table[wch->race].race_name, clan, col1, col2, col3, col4, col5);

            nlength = my_strlen(wch->short_descr);
            slength = 40 - my_strlen(wch->pcdata->title) - nlength;

            strcpy(buf4, wch->short_descr);
            safe_strcat(MSL, buf4, wch->pcdata->title);
            for (nlength = 0; nlength < slength; nlength++)
                safe_strcat(MSL, buf4, " ");

            sprintf(buf + strlen(buf), "@@d|%s%s @@N@@d| %s%s @@d| %s%s@@N @@d|\n\r",
                colour_string(ch, "stats"), buf3, colour_string(ch, "stats"), fgs, colour_string(ch, "stats"), buf4);
        }

        send_to_char(buf, ch);
        buf[0] = '\0';
    }

    send_to_char(buf, ch);
    send_to_char("@@d'-----------------------------------------------------------------------------'\n\r", ch);
    send_to_char(colour_string(ch, "normal"), ch);

    return;
}

void
do_inventory(CHAR_DATA *ch, char *argument)
{
    send_to_char("You are carrying:\n\r", ch);
    show_list_to_char(ch->first_carry, ch, TRUE, TRUE);
    return;
}

void
do_equipment(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA           *obj;
    int                 iWear;
    int                 pct = 0;
    bool                found;
    bool                seen = FALSE;
    bool                show_held = FALSE;
    bool                show_shield = FALSE;
    bool                is_dualwielding = FALSE;

    send_to_char("You are using:\n\r", ch);

    if (get_eq_char(ch, WEAR_WIELD_2) != NULL)
        is_dualwielding = TRUE;

    if (!is_dualwielding) {
        show_held = TRUE;
        show_shield = TRUE;
    }
    else {
        if (ch->race != RACE_DWF) {
            show_held = FALSE;
            show_shield = FALSE;
        }
        else {
            show_held = (get_eq_char(ch, WEAR_SHIELD)) ? FALSE : TRUE;
            show_shield = (get_eq_char(ch, WEAR_HOLD)) ? FALSE : TRUE;
        }
    }

    found = FALSE;

    for (iWear = 0; iWear < MAX_WEAR; iWear++) {
        pct = iWear;

        if (iWear == 17 && !seen) {
            pct = 21;
            iWear--;
            seen = TRUE;
        }
        else if (pct == 21)
            continue;

        if ((obj = get_eq_char(ch, pct)) == NULL) {
            if (pct == WEAR_HOLD && !show_held)
                continue;

            if (pct == WEAR_SHIELD && !show_shield)
                continue;

            if (pct == WEAR_WIELD_2)
                continue;

            send_to_char(notwhere_name[pct], ch);
            send_to_char("@@N\n\r", ch);

            continue;
        }

        send_to_char(where_name[pct], ch);
        if (can_see_obj(ch, obj)) {
            send_to_char(format_obj_to_char(obj, ch, TRUE), ch);
            send_to_char("@@N\n\r", ch);
        }
        else {
            send_to_char("something.\n\r", ch);
        }
        found = TRUE;
    }

    return;
}

char               *
format_eq_stats(OBJ_DATA *obj)
{
    static char         buf[512];
    AFFECT_DATA        *af;
    bool                found = FALSE;
    bool                foundperm = FALSE;
    bool                foundhit = FALSE;

    int                 apply_str = 0, apply_dex = 0, apply_int = 0, apply_wis = 0, apply_con = 0;
    int                 apply_hit = 0, apply_mana = 0, apply_move = 0;
    int                 apply_damroll = 0, apply_hitroll = 0, apply_ac = 0;
    int                 apply_saves = 0;

    for (af = obj->first_apply; af; af = af->next) {
        switch (af->location) {
            default:
            case APPLY_NONE:
            case APPLY_SEX:
            case APPLY_CLASS:
            case APPLY_LEVEL:
            case APPLY_AGE:
            case APPLY_HEIGHT:
            case APPLY_WEIGHT:
            case APPLY_GOLD:
            case APPLY_EXP:
                break;

            case APPLY_STR:
                apply_str += af->modifier;
                foundperm = TRUE;
                break;
            case APPLY_INT:
                apply_int += af->modifier;
                foundperm = TRUE;
                break;
            case APPLY_WIS:
                apply_wis += af->modifier;
                foundperm = TRUE;
                break;
            case APPLY_DEX:
                apply_dex += af->modifier;
                foundperm = TRUE;
                break;
            case APPLY_CON:
                apply_con += af->modifier;
                foundperm = TRUE;
                break;

            case APPLY_HIT:
                apply_hit += af->modifier;
                foundhit = TRUE;
                break;
            case APPLY_MANA:
                apply_mana += af->modifier;
                foundhit = TRUE;
                break;
            case APPLY_MOVE:
                apply_move += af->modifier;
                foundhit = TRUE;
                break;

            case APPLY_DAMROLL:
                apply_damroll += af->modifier;
                found = TRUE;
                break;
            case APPLY_HITROLL:
                apply_hitroll += af->modifier;
                found = TRUE;
                break;
            case APPLY_AC:
                apply_ac += af->modifier;
                found = TRUE;
                break;

            case APPLY_SAVING_PARA:
            case APPLY_SAVING_ROD:
            case APPLY_SAVING_PETRI:
            case APPLY_SAVING_BREATH:
            case APPLY_SAVING_SPELL:
                apply_saves += af->modifier;
                found = TRUE;
                break;
        }
    }

    found = found || foundhit || foundperm;

    if (!found) {
        strcpy(buf, "[none]");
        return buf;
    }

    sprintf(buf, "[%d/%d/%d", apply_damroll, apply_hitroll, apply_ac);
    if (foundhit || foundperm)
        sprintf(buf + strlen(buf), " %d/%d/%d", apply_hit, apply_mana, apply_move);
    if (apply_saves != 0)
        sprintf(buf + strlen(buf), " %d", apply_saves);
    if (foundperm)
        sprintf(buf + strlen(buf), " %d/%d/%d/%d/%d", apply_str, apply_int, apply_wis, apply_dex, apply_con);

    strcat(buf, "]");

    return buf;
}

char               *
format_eqindex_stats(OBJ_INDEX_DATA *obj)
{
    static char         buf[512];
    AFFECT_DATA        *af;
    bool                found = FALSE;
    bool                foundperm = FALSE;
    bool                foundhit = FALSE;

    int                 apply_str = 0, apply_dex = 0, apply_int = 0, apply_wis = 0, apply_con = 0;
    int                 apply_hit = 0, apply_mana = 0, apply_move = 0;
    int                 apply_damroll = 0, apply_hitroll = 0, apply_ac = 0;
    int                 apply_saves = 0;

    for (af = obj->first_apply; af; af = af->next) {
        switch (af->location) {
            default:
            case APPLY_NONE:
            case APPLY_SEX:
            case APPLY_CLASS:
            case APPLY_LEVEL:
            case APPLY_AGE:
            case APPLY_HEIGHT:
            case APPLY_WEIGHT:
            case APPLY_GOLD:
            case APPLY_EXP:
                break;

            case APPLY_STR:
                apply_str += af->modifier;
                foundperm = TRUE;
                break;
            case APPLY_INT:
                apply_int += af->modifier;
                foundperm = TRUE;
                break;
            case APPLY_WIS:
                apply_wis += af->modifier;
                foundperm = TRUE;
                break;
            case APPLY_DEX:
                apply_dex += af->modifier;
                foundperm = TRUE;
                break;
            case APPLY_CON:
                apply_con += af->modifier;
                foundperm = TRUE;
                break;

            case APPLY_HIT:
                apply_hit += af->modifier;
                foundhit = TRUE;
                break;
            case APPLY_MANA:
                apply_mana += af->modifier;
                foundhit = TRUE;
                break;
            case APPLY_MOVE:
                apply_move += af->modifier;
                foundhit = TRUE;
                break;

            case APPLY_DAMROLL:
                apply_damroll += af->modifier;
                found = TRUE;
                break;
            case APPLY_HITROLL:
                apply_hitroll += af->modifier;
                found = TRUE;
                break;
            case APPLY_AC:
                apply_ac += af->modifier;
                found = TRUE;
                break;

            case APPLY_SAVING_PARA:
            case APPLY_SAVING_ROD:
            case APPLY_SAVING_PETRI:
            case APPLY_SAVING_BREATH:
            case APPLY_SAVING_SPELL:
                apply_saves += af->modifier;
                found = TRUE;
                break;
        }
    }

    found = found || foundhit || foundperm;

    if (!found) {
        strcpy(buf, "");
        return buf;
    }

    sprintf(buf, "[%d/%d/%d", apply_damroll, apply_hitroll, apply_ac);
    if (foundhit || foundperm)
        sprintf(buf + strlen(buf), " %d/%d/%d", apply_hit, apply_mana, apply_move);
    if (apply_saves != 0)
        sprintf(buf + strlen(buf), " %d", apply_saves);
    if (foundperm)
        sprintf(buf + strlen(buf), " %d/%d/%d/%d/%d", apply_str, apply_int, apply_wis, apply_dex, apply_con);

    strcat(buf, "]");

    return buf;
}

void
do_eqaffects(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA           *obj;
    int                 iWear;
    int                 pct = 0;
    bool                found;
    bool                seen = FALSE;
    bool                show_held = FALSE;
    bool                show_shield = FALSE;
    bool                is_dualwielding = FALSE;
    char                buf[MSL];

    if (IS_NPC(ch) || !ch->pcdata)
        return;

    if (skill_lookup("identify") && ch->pcdata->learned[skill_lookup("identify")] <= 0) {
        send_to_char("You must practice the identify spell to use this command.\n\r", ch);
        return;
    }

    send_to_char("You are using:\n\r", ch);

    if (get_eq_char(ch, WEAR_WIELD_2) != NULL)
        is_dualwielding = TRUE;

    if (!is_dualwielding) {
        show_held = TRUE;
        show_shield = TRUE;
    }
    else {
        if (ch->race != RACE_DWF) {
            show_held = FALSE;
            show_shield = FALSE;
        }
        else {
            show_held = (get_eq_char(ch, WEAR_SHIELD)) ? FALSE : TRUE;
            show_shield = (get_eq_char(ch, WEAR_HOLD)) ? FALSE : TRUE;
        }
    }

    found = FALSE;

    for (iWear = 0; iWear < MAX_WEAR; iWear++) {
        pct = iWear;

        if (iWear == 17 && !seen) {
            pct = 21;
            iWear--;
            seen = TRUE;
        }
        else if (pct == 21)
            continue;

        if ((obj = get_eq_char(ch, pct)) == NULL) {
            if (pct == WEAR_HOLD && !show_held)
                continue;

            if (pct == WEAR_SHIELD && !show_shield)
                continue;

            if (pct == WEAR_WIELD_2)
                continue;

            send_to_char(notwhere_name_short[pct], ch);
            send_to_char("@@N\n\r", ch);

            continue;
        }

        send_to_char(where_name_short[pct], ch);
        if (can_see_obj(ch, obj)) {
            buf[0] = '\0';
            send_to_char(my_left(obj->short_descr, buf, 23), ch);

            /* show eq stats in yellow if item is unique */
            if (!IS_SET(obj->extra_flags, ITEM_UNIQUE))
                send_to_char("@@N ", ch);
            else
                send_to_char("@@y ", ch);

            send_to_char(format_eq_stats(obj), ch);
            send_to_char("@@N\n\r", ch);
        }
        else {
            send_to_char("something.             @@N ", ch);
            send_to_char(format_eq_stats(obj), ch);
            send_to_char("\n\r", ch);
        }
        found = TRUE;
    }

    return;
}

void
do_xlookaff(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA          *victim;
    OBJ_DATA           *obj;
    int                 iWear;
    char                arg[MAX_INPUT_LENGTH];
    char                buf[MSL];

    if ((victim = get_char_world(ch, argument)) == NULL) {
        send_to_char("No such player found.\n\r", ch);
        return;
    }

    if (!can_see(ch, victim)) {
        send_to_char("No such player found.\n\r", ch);
        return;
    }
    act("$N is wearing:", ch, NULL, victim, TO_CHAR);
    for (iWear = 0; iWear < MAX_WEAR; iWear++) {
        if ((obj = get_eq_char(victim, iWear)) != NULL && can_see_obj(ch, obj)) {
            one_argument(obj->name, arg);
            if (!IS_SET(obj->extra_flags, ITEM_UNIQUE))
                sendf(ch, "%s %s @@N<%s> %s@@N\n\r", where_name_short[iWear], my_left(obj->short_descr, buf, 23), arg, format_eq_stats(obj));
            else
                sendf(ch, "%s %s @@N<%s> @@y%s@@N\n\r", where_name_short[iWear], my_left(obj->short_descr, buf, 23), arg, format_eq_stats(obj));
        }
        else
            sendf(ch, "%s\n\r", notwhere_name_short[iWear]);
    }

    send_to_char("@@N\n\r", ch);
    send_to_char("Inventory:\n\r", ch);

    {
        OBJ_DATA           *in;
        bool                found = FALSE;

        for (obj = victim->first_carry; obj != NULL; obj = obj->next_in_carry_list) {
            if (obj->wear_loc != WEAR_NONE || !can_see_obj(ch, obj))
                continue;

            found = TRUE;

            one_argument(obj->name, arg);

            sendf(ch, "     [%s] %s\n\r", arg, format_obj_to_char(obj, ch, TRUE));

            if (obj->first_in_carry_list) {
                for (in = obj->first_in_carry_list; in != NULL; in = in->next_in_carry_list) {
                    one_argument(in->name, arg);
                    sendf(ch, "          [%s] %s\n\r", arg, format_obj_to_char(in, ch, TRUE));
                }
            }
        }

        if (!found)
            send_to_char("     Nothing.\n\r", ch);

    }

    return;
}

void
do_compare(CHAR_DATA *ch, char *argument)
{
    char                arg1[MAX_INPUT_LENGTH];
    char                arg2[MAX_INPUT_LENGTH];
    OBJ_DATA           *obj1;
    OBJ_DATA           *obj2;
    int                 value1;
    int                 value2;
    char               *msg;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    if (arg1[0] == '\0') {
        send_to_char("Compare what to what?\n\r", ch);
        return;
    }

    if ((obj1 = get_obj_carry(ch, arg1)) == NULL) {
        send_to_char("You do not have that item.\n\r", ch);
        return;
    }

    if (arg2[0] == '\0') {
        for (obj2 = ch->first_carry; obj2 != NULL; obj2 = obj2->next_in_carry_list) {
            if (obj2->wear_loc != WEAR_NONE && can_see_obj(ch, obj2)
                && obj1->item_type == obj2->item_type && (obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0)
                break;
        }

        if (obj2 == NULL) {
            send_to_char("You aren't wearing anything comparable.\n\r", ch);
            return;
        }
    }
    else {
        if ((obj2 = get_obj_carry(ch, arg2)) == NULL) {
            send_to_char("You do not have that item.\n\r", ch);
            return;
        }
    }

    msg = NULL;
    value1 = 0;
    value2 = 0;

    if (obj1 == obj2) {
        msg = "You compare $p to itself.  It looks about the same.";
    }
    else if (obj1->item_type != obj2->item_type) {
        msg = "You can't compare $p and $P.";
    }
    else {
        switch (obj1->item_type) {
            default:
                msg = "You can't compare $p and $P.";
                break;

            case ITEM_ARMOR:
                value1 = obj1->value[0];
                value2 = obj2->value[0];
                break;

            case ITEM_WEAPON:
                value1 = obj1->value[1] + obj1->value[2];
                value2 = obj2->value[1] + obj2->value[2];
                break;
        }
    }

    if (msg == NULL) {
        if (value1 == value2)
            msg = "$p and $P look about the same.";
        else if (value1 > value2)
            msg = "$p looks better than $P.";
        else
            msg = "$p looks worse than $P.";
    }

    act(msg, ch, obj1, obj2, TO_CHAR);
    return;
}

void
do_credits(CHAR_DATA *ch, char *argument)
{
    do_help(ch, "diku");
    return;
}

void
do_where(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    char                arg[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim;
    DESCRIPTOR_DATA    *d;
    BUILD_DATA_LIST    *pList = NULL;
    ROOM_INDEX_DATA    *room = NULL;
    bool                found;
    int                 space;
    int                 cnt;

    buf[0] = '\0';

    one_argument(argument, arg);

    if (arg[0] == '\0') {
        send_to_char("Players near you:\n\r", ch);
        sendf(ch, "In %s %s@@N:\n\r", ch->in_room->area->level_label, ch->in_room->area->name);
        pList = ch->in_room->area->first_area_room;
        room = pList->data;
        found = FALSE;

        if (!IS_SET(room->affected_by, ROOM_BV_SMOKESCREEN_AREA)) {
            for (d = first_desc; d; d = d->next) {
                if (d->connected == CON_PLAYING && (victim = d->character) != NULL && !IS_NPC(victim)
                    && victim->in_room != NULL && victim->in_room->area == ch->in_room->area && can_see(ch, victim)) {

                    if ((!IS_NPC(victim) || IS_AFFECTED(victim, AFF_CHARM)) && victim->in_room && IS_SET(victim->in_room->affected_by, ROOM_BV_SMOKESCREEN)) {
                        ROOM_AFFECT_DATA *paf;

                        /* smokescreen found, find out what type it is */
                        for (paf = victim->in_room->first_room_affect; paf; paf = paf->next)
                            if (paf->bitvector == ROOM_BV_SMOKESCREEN)
                                break;

                        if (paf) {
                            if (   paf->type == gsn_smokescreen_expert
                                || paf->type == gsn_smokescreen_master)
                                continue;
                        }
                    }

                    found = TRUE;
                    space = 28 - my_strlen(victim->short_descr);
                    strcpy(buf, victim->short_descr);
                    for (cnt = 0; cnt < space; cnt++)
                        safe_strcat(MSL, buf, " ");

                    safe_strcat(MSL, buf, victim->in_room->name);
                    safe_strcat(MSL, buf, "\n\r");

                    send_to_char(buf, ch);
                }
            }
        }

        if (!found)
            send_to_char("None\n\r", ch);
    }

    return;
}

#define GET_PERCENT(a, b) ((int)(((a) == 0 ? 1.0 : (float)(a)) * 100 / ((b) == 0 ? 1 : (b))))

void
do_consider(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim;
    char               *msg = '\0';
    char               *buf = '\0';
    char               *buf2 = '\0';
    char               *buf3 = '\0';
    char               *buf4 = '\0';
    float               diff;
    int                 hpdiff;
    int                 hrdiff;
    int                 drdiff;
    int                 acdiff;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
        send_to_char("Consider killing whom?\n\r", ch);
        return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL) {
        send_to_char("They're not here.\n\r", ch);
        return;
    }

    if (!IS_NPC(victim))
        send_to_char("@@lRemember there are downfalls to PKilling!@@N\n\r", ch);

    /* Stephen - bypass class adjustment if victim == NPC */
    /* Also, only look at modifiers if victim == NPC */

    /*    diff = victim->level - ch->level; */

    diff = (get_pseudo_level(victim) - get_pseudo_level(ch));

    /* Mod rolls. */
    if (IS_NPC(victim)) {
        diff += victim->hr_mod / 4;
        diff += victim->dr_mod / 4;
        diff -= victim->ac_mod / 30;
    }

    if (diff >= 10)
        msg = "Death will thank you for your gift.";
    if (diff <= 9)
        msg = "$N laughs at you mercilessly.";
    if (diff <= 4)
        msg = "$N says 'Do you feel lucky, punk?'.";
    if (diff <= 1)
        msg = "The perfect match!";
    if (diff <= -2)
        msg = "$N looks like an easy kill.";
    if (diff <= -5)
        msg = "$N is no match for you.";
    if (diff <= -10)
        msg = "You can kill $N naked and weaponless.";

    act(msg, ch, NULL, victim, TO_CHAR);
    msg = "";

    /* additions by king@tinuviel.cs.wcu.edu */
    hpdiff = GET_PERCENT(ch->hit, victim->hit);

    if (hpdiff > 190)
        buf = "You are *MUCH* healthier than $M.";
    else if (hpdiff > 175)
        buf = "You are MUCH healthier than $M.";
    else if (hpdiff > 150)
        buf = "You are much healthier than $M.";
    else if (hpdiff > 115)
        buf = "You are healthier than $M.";
    else if (hpdiff > 102)
        buf = "You are slightly healthier than $M.";
    else if (hpdiff > 97)
        buf = "You have roughly the same amount of health as $M.";
    else if (hpdiff > 85)
        buf = "$E is slightly healthier than you.";
    else if (hpdiff > 50)
        buf = "$E is healthier than you.";
    else if (hpdiff > 25)
        buf = "$E is much healthier than you.";
    else if (hpdiff > 10)
        buf = "$E is MUCH healthier than you.";
    else
        buf = "$E is *MUCH* healthier than you.";

    act(buf, ch, NULL, victim, TO_CHAR);
    buf = "";

    drdiff = GET_PERCENT(GET_DAMROLL(ch), GET_DAMROLL(victim));
    if (drdiff > 190)
        buf2 = "You hit *A LOT* harder than $M.";
    else if (drdiff > 175)
        buf2 = "You hit A LOT harder than $M.";
    else if (drdiff > 150)
        buf2 = "You hit a lot harder than $M.";
    else if (drdiff > 115)
        buf2 = "You hit harder than $M.";
    else if (drdiff > 102)
        buf2 = "You hit slightly harder than $M.";
    else if (drdiff > 97)
        buf2 = "$E hits about the same as you.";
    else if (drdiff > 85)
        buf2 = "$E hits slightly harder than you.";
    else if (drdiff > 50)
        buf2 = "$E hits harder than you.";
    else if (drdiff > 25)
        buf2 = "$E hits a lot harder than you.";
    else if (drdiff > 10)
        buf2 = "$E hits A LOT harder than you.";
    else
        buf2 = "$E hits *A LOT* harder than you.";

    act(buf2, ch, NULL, victim, TO_CHAR);
    buf = "";

    hrdiff = GET_PERCENT(GET_HITROLL(ch), GET_HITROLL(victim));
    if (hrdiff > 190)
        buf3 = "You hit *A LOT* more often than $M.";
    else if (hrdiff > 175)
        buf3 = "You hit A LOT more often than $M.";
    else if (hrdiff > 150)
        buf3 = "You hit a lot more often than $M.";
    else if (hrdiff > 115)
        buf3 = "You hit more often than $M.";
    else if (hrdiff > 102)
        buf3 = "You hit slightly more often than $M.";
    else if (hrdiff > 97)
        buf3 = "$E hits about equally as often as you.";
    else if (hrdiff > 85)
        buf3 = "$E hits slightly more often than you.";
    else if (hrdiff > 50)
        buf3 = "$E hits more often than you.";
    else if (hrdiff > 25)
        buf3 = "$E hits a lot more often than you.";
    else if (hrdiff > 10)
        buf3 = "$E hits A LOT more often than you.";
    else
        buf3 = "$E hits *A LOT* more often than you.";

    act(buf3, ch, NULL, victim, TO_CHAR);
    buf3 = "";

    acdiff = abs(GET_PERCENT(GET_AC(ch), GET_AC(victim)));
    if (acdiff > 190)
        buf4 = "You are armored like a tank compared to $M.";
    else if (acdiff > 175)
        buf4 = "You are armored a lot better than $M.";
    else if (acdiff > 150)
        buf4 = "You are armored quite a bit better than $M.";
    else if (acdiff > 115)
        buf4 = "You are more heavily armored than $M.";
    else if (acdiff > 102)
        buf4 = "You are slightly more armored than $M.";
    else if (acdiff > 97)
        buf4 = "You and $M are armored pretty equally.";
    else if (acdiff > 85)
        buf4 = "$E is slightly more armored than you.";
    else if (acdiff > 50)
        buf4 = "$E is more heavily armored than you.";
    else if (acdiff > 25)
        buf4 = "$E is quite a bit better armored than you.";
    else if (acdiff > 10)
        buf4 = "$E is armored a lot better than you.";
    else
        buf4 = "$E is armored like a tank compared to you.";

    act(buf4, ch, NULL, victim, TO_CHAR);

    buf = "";
    return;
}

void
set_title(CHAR_DATA *ch, char *title)
{
    char                buf[MAX_STRING_LENGTH];

    buf[0] = '\0';

    if (IS_NPC(ch))
        return;

    if (isalpha(title[0]) || isdigit(title[0])) {
        buf[0] = ' ';
        strcpy(buf + 1, title);
    }
    else {
        strcpy(buf, title);
    }

    free_string(ch->pcdata->title);
    ch->pcdata->title = str_dup(buf);
    return;
}

void
do_title(CHAR_DATA *ch, char *argument)
{
    /* Changed this to limit title length, and to remove and brackets. -S- */

    char                buf[MAX_STRING_LENGTH];
    int                 cnt;
    bool                changed;

    buf[0] = '\0';

    if (IS_NPC(ch))
        return;

    if (argument[0] == '\0') {
        set_title(ch, argument);
        send_to_char("Title turned off.\n\r", ch);
        return;
    }
    changed = FALSE;

    for (cnt = 0; cnt < strlen(argument); cnt++) {
        if (argument[cnt] == '[' || argument[cnt] == ']') {
            changed = TRUE;
            argument[cnt] = (argument[cnt] == ']') ? '>' : '<';
        }
    }

    if (changed)
        send_to_char("You used either [ or ] in your title.  They have been removed!\n\r", ch);

    /* my_strlen handles colour codes as zero length */
    cnt = my_strlen(ch->short_descr);
    cnt += my_strlen(argument);

    if (isalpha(argument[0]) || isdigit(argument[0]))
        cnt++;

    if (cnt > 40) {
        send_to_char("Title too long.  Please try again.\n\r", ch);
        return;
    }

    smash_tilde(argument);
    set_title(ch, argument);
    sendf(ch, "You are now: %s%s.\n\r", ch->short_descr, ch->pcdata->title);
}

void
do_description(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    int                 lines = 0;
    char               *chk;
    char               *arg;
    buf[0] = '\0';

    arg = argument;

    if (arg[0] == '\0') {
        send_to_char("Your description is:\n\r", ch);
        send_to_char(ch->description ? ch->description : "(None).\n\r", ch);
        return;
    }

    if (!str_cmp("reset", arg)) {
        free_string(ch->description);
        ch->description = str_dup("");
        send_to_char("Description reset.\n\r", ch);
        return;
    }

    buf[0] = '\0';
    smash_tilde(arg);
    if (arg[0] == '+') {
        if (ch->description != NULL) {
            chk = ch->description;

            while (*chk++ != '\0')
                if (*chk == '\n')
                    lines++;

            if (lines > 150) {
                send_to_char("Description too long.\n\r", ch);
                return;
            }

            safe_strcat(MAX_STRING_LENGTH - 200, buf, ch->description);
        }

        arg++;
        while (isspace(*arg))
            arg++;
    }

    if (strlen(buf) + strlen(arg) >= MAX_STRING_LENGTH - 400) {
        send_to_char("Description too long.\n\r", ch);
        return;
    }

    safe_strcat(MAX_STRING_LENGTH, buf, arg);
    safe_strcat(MAX_STRING_LENGTH, buf, "\n\r");
    free_string(ch->description);
    ch->description = str_dup(buf);

    if (argument[0] == '+') {
        send_to_char("Line added to your description.\n\r", ch);
        return;
    }
}

void
do_report(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_INPUT_LENGTH];

    sendf(ch, "You report: %d/%d hp %d/%d mana %d/%d mv %d xp.\n\r", ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move, ch->exp);
    sprintf(buf, "$n reports: %d/%d hp %d/%d mana %d/%d mv %d xp.", ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move, ch->exp);
    act(buf, ch, NULL, NULL, TO_ROOM);

    return;
}

void
do_preport(CHAR_DATA *ch, char *argument)
{
    char                buf[MSL];
    char                buf2[MSL];
    char                buf3[MSL];
    char                buf4[MSL];
    CHAR_DATA          *victim;
    OBJ_DATA           *obj;

    if (!IS_IMMORTAL(ch) || (IS_IMMORTAL(ch) && str_cmp(argument, "all"))) {
        sprintf(buf2, "%s", percbar(ch->hit, ch->max_hit, 10));
        sprintf(buf3, "%s", percbar(ch->mana, ch->max_mana, 10));
        sprintf(buf4, "%s", percbar(ch->move, ch->max_move, 10));

        sendf(ch, "@@NYou report: hp(%s@@N) mana(%s@@N) move(%s@@N).\n\r", buf2, buf3, buf4);

        if (ch->in_room == NULL)
            return;

        for (victim = ch->in_room->first_person; victim != NULL; victim = victim->next_in_room) {
            if (ch == victim)
                continue;

            sendf(victim, "%s reports: hp(%s@@N) mana(%s@@N) move(%s@@N).\n\r", PERS(ch, victim), buf2, buf3, buf4);
        }
    }
    else {
        for (victim = first_player; victim != NULL; victim = victim->next_player) {
            if (!victim->desc)
                send_to_char("@@y* @@N", ch);
            else
                send_to_char("@@N  ", ch);

            sprintf(buf2, "%s", percbar(victim->hit, victim->max_hit, 10));
            sprintf(buf3, "%s", percbar(victim->mana, victim->max_mana, 10));
            sprintf(buf4, "%s", percbar(victim->move, victim->max_move, 10));
            sendf(ch, "%s hp(%s@@N) mana(%s@@N) move(%s@@N).\n\r", my_left(PERS(victim, ch), buf, 20), buf2, buf3, buf4);

            if (!IS_IMMORTAL(victim))
                for (obj = victim->first_carry; obj != NULL; obj = obj->next_in_carry_list)
                    if (IS_SET(obj->item_apply, ITEM_APPLY_HEATED))
                        sendf(ch, "    @@N@@gburning: %s@@N\n\r", obj->short_descr);
        }
    }

    return;
}

void
do_sreport(CHAR_DATA *ch, char *argument)
{
    char                buf[MSL];

    if (IS_NPC(ch))
        return;

    sendf(ch, "You report: @@gStr@@d[@@g%2d@@d/@@g%2d@@d]  @@gInt@@d[@@g%2d@@d/@@g%2d@@d]  @@gWis@@d[@@g%2d@@d/@@g%2d@@d]  @@gDex@@d[@@g%2d@@d/@@g%2d@@d]  @@gCon@@d[@@g%2d@@d/@@g%2d@@d]  @@gTotal@@d[@@g%2d@@d/@@g%2d@@d]@@N.\n\r",
        get_curr_str(ch), ch->pcdata->max_str, get_curr_int(ch), ch->pcdata->max_int, get_curr_wis(ch), ch->pcdata->max_wis, get_curr_dex(ch),
        ch->pcdata->max_dex, get_curr_con(ch), ch->pcdata->max_con,
        get_curr_str(ch) + get_curr_int(ch) + get_curr_wis(ch) + get_curr_dex(ch) + get_curr_con(ch),
        ch->pcdata->max_str + ch->pcdata->max_int + ch->pcdata->max_wis + ch->pcdata->max_dex + ch->pcdata->max_con);

    sprintf(buf, "$n reports: @@gStr@@d[@@g%2d@@d/@@g%2d@@d]  @@gInt@@d[@@g%2d@@d/@@g%2d@@d]  @@gWis@@d[@@g%2d@@d/@@g%2d@@d]  @@gDex@@d[@@g%2d@@d/@@g%2d@@d]  @@gCon@@d[@@g%2d@@d/@@g%2d@@d]  @@gTotal@@d[@@g%2d@@d/@@g%2d@@d]@@N.",
        get_curr_str(ch), ch->pcdata->max_str, get_curr_int(ch), ch->pcdata->max_int, get_curr_wis(ch), ch->pcdata->max_wis, get_curr_dex(ch),
        ch->pcdata->max_dex, get_curr_con(ch), ch->pcdata->max_con,
        get_curr_str(ch) + get_curr_int(ch) + get_curr_wis(ch) + get_curr_dex(ch) + get_curr_con(ch),
        ch->pcdata->max_str + ch->pcdata->max_int + ch->pcdata->max_wis + ch->pcdata->max_dex + ch->pcdata->max_con);
    act(buf, ch, NULL, NULL, TO_ROOM);

}

struct praclist {
    int sn;
    int learned;
};

typedef struct praclist PRACLIST;

int practice_list_cmp(const void *x, const void *y)
{
    PRACLIST       *dx = *(PRACLIST **) x;
    PRACLIST       *dy = *(PRACLIST **) y;

/* people said that finding skills/spells in practice was hard, and
 * sorting by % learnt then alphabetically doesn't necessarily help this. so
 * sort alphabetically only */

/*
    if (dx->learned < dy->learned)
        return -1;
    else if (dx->learned == dy->learned)
*/
        return strcmp(skill_table[dx->sn].name, skill_table[dy->sn].name);
/*
    else
        return 1;
*/
}

void do_practice_list(CHAR_DATA *ch)
{
    char buf[MSL], buf2[MSL];
    char perc[16];
    char skillbuf[32];
    int col = 0;
    int cols = 3;
    int row = 0;
    int rows = 0;
    int sn;
    int x;
    char *namecol = NULL;

    PRACLIST **skills;
    int cnt, skillcnt = 0;

    for (cnt = 0; cnt < MAX_SKILL; cnt++)
        if (ch->pcdata->learned[cnt] > 0)
            skillcnt++;

    if (skillcnt == 0) {
        send_to_char("You have no skills/spells practiced.\n\r", ch);
        return;
    }

    skills = (PRACLIST **)malloc(sizeof(PRACLIST *) * skillcnt);
    memset(skills, 0, sizeof(PRACLIST *) * skillcnt);

    for (cnt = 0; cnt < skillcnt; cnt++)
        skills[cnt] = (PRACLIST *)malloc(sizeof(PRACLIST));

    skillcnt = 0;

    for (cnt = 0; cnt < MAX_SKILL; cnt++)
        if (ch->pcdata->learned[cnt] > 0) {
            skills[skillcnt]->sn = cnt;
            skills[skillcnt]->learned = ch->pcdata->learned[cnt];
            skillcnt++;
        }

    qsort(skills, skillcnt, sizeof(PRACLIST *), practice_list_cmp);

    send_to_char("@@N@@d.-----------------------------------------------@@g=( @@WPracticed Skills/Spells @@g)=-@@d.\n\r", ch);
    sprintf(buf, "@@gYou have @@W%d @@gpractice %s remaining.", ch->practice, ch->practice == 1 ? "session" : "sessions");
    sendf(ch, "@@d| %s @@d|\n\r", my_left(buf, buf2, 75));
    send_to_char("@@d|-----------------------------------------------------------------------------|\n\r", ch);

    cols = 3;

    rows = skillcnt / cols + ((skillcnt % cols != 0) ? 1 : 0);
    x = 0;

    for (row = 0; row < rows; row++) {
        send_to_char("@@d|", ch);

        for (col = 0; col < cols; col++) {
            cnt = col * rows + row;
            sn = cnt < skillcnt ? skills[cnt]->sn : -1;

            if (sn > -1) {
                if (skill_table[sn].flag1 == AVATAR && skill_table[sn].flag2 == NORM)
                    sprintf(perc, "@@y%d%%", skills[cnt]->learned);
                else
                    sprintf(perc, "@@W%d%%", skills[cnt]->learned);

                if (skill_table[sn].flag1 == AVATAR && skill_table[sn].flag2 == SUB)
                    namecol = "@@a";
                else if (skills[cnt]->learned >= best_learnt(ch, sn))
                    namecol = "@@g";
                else
                    namecol = "@@e";

                sprintf(skillbuf, "%s%s", namecol, skill_table[sn].name);
                sprintf(buf, " %s %s @@d|", my_left(skillbuf, buf2, 22 - my_strlen(perc)), perc);
            }
            else
                sprintf(buf, "                         @@d|");

            send_to_char(buf, ch);
        }

        send_to_char("@@N\n\r", ch);
    }

    send_to_char("@@d'-----------------------------------------------------------------------------'@@N\n\r", ch);

    for (cnt = 0; cnt < skillcnt; cnt++)
        free(skills[cnt]);

    free(skills);
    return;
}

void
do_practice(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    CHAR_DATA          *mob;
    int                 sn;
    bool                avatar = FALSE;

    /* Now need to check through ch->lvl[] to see if player's level in
     * the required class is enough for him/her to be able to prac the
     * skill/spell.  Eg if char is cle:10 and war:50, we don't want the
     * player to be getting level 50 cleric spells, which would happen
     * if ch->class was used here! -S-
     */
    buf[0] = '\0';

    if (IS_NPC(ch))
        return;

    if (argument[0] == '\0') {
        do_practice_list(ch);
    }
    else {
        int                 adept;

        if (ch->level < 3) {
            send_to_char("You must be third level to practice.  Go train instead!\n\r", ch);
            return;
        }

        if (!IS_AWAKE(ch)) {
            send_to_char("@@WIn your dreams, or what?@@N\n\r", ch);
            return;
        }

        for (mob = ch->in_room->first_person; mob != NULL; mob = mob->next_in_room) {
            if (IS_NPC(mob) && IS_SET(mob->act, ACT_PRACTICE))
                break;
        }

        if (mob == NULL) {
            send_to_char("You can't do that here.\n\r", ch);
            return;
        }

        if ((sn = skill_lookup(argument)) < 0) {
            send_to_char("You can't practice that.\n\r", ch);
            return;
        }

        adept = best_learnt(ch, sn);

        if (skill_table[sn].flag1 == AVATAR && skill_table[sn].flag2 == SUB)
            adept = 0;
        else if (skill_table[sn].flag1 == AVATAR && skill_table[sn].flag2 == NORM)
            avatar = TRUE;

        if (ch->practice <= 0 && !avatar) {
            send_to_char("You have no practice sessions left.\n\r", ch);
            return;
        }

        if (adept == 0) {
            send_to_char("You can't practice that.\n\r", ch);
            return;
        }

        if (ch->pcdata->learned[sn] >= adept) {
            sendf(ch, "@@N@@gYou already know @@W%s@@g as well as is currently possible.@@N\n\r", skill_table[sn].name);
        }
        else if (!avatar) {
            ch->practice--;
            ch->pcdata->learned[sn] += int_app[get_curr_int(ch)].learn;
            if (ch->pcdata->learned[sn] < adept) {
                act("You practice $T.", ch, NULL, skill_table[sn].name, TO_CHAR);
                act("$n practices $T.", ch, NULL, skill_table[sn].name, TO_ROOM);
            }
            else {
                ch->pcdata->learned[sn] = adept;
                act("You are now a master of $T.", ch, NULL, skill_table[sn].name, TO_CHAR);
                act("$n is now a master of $T.", ch, NULL, skill_table[sn].name, TO_ROOM);
            }
        }
        else {
            /* AVATAR SKILL PRACTICING HERE! */
            int cost, hpgain, managain, energygain;

            if (!ch->pcdata->avatar) {
                send_to_char("You must be an avatar to practice avatar spells/skills.\n\r", ch);
                return;
            }

            cost = avatar_cost(ch->pcdata->learned[sn] > 0 ? ch->pcdata->learned[sn] : 0) * 1000;
            hpgain = cost / 50000;
            managain = cost / 50000;
            energygain = 10;

            if (ch->exp < cost) {
                sendf(ch, "You don't have enough experience points to practice that. You need %s.\n\r", number_comma(cost));
                return;
            }

            if (ch->pcdata->learned[sn] + 1 == AV_MASTER) {
                /* we're praccing to master, give them extra hp to make up the 2k */
                hpgain += 50;
                managain += 50;
            }

            ch->exp -= cost;
            ch->pcdata->hp_from_gain += hpgain;
            ch->max_hit += hpgain;
            ch->hit += hpgain;
            ch->pcdata->mana_from_gain += managain;
            ch->max_mana += managain;
            ch->mana += managain;
            ch->max_energy += energygain;
            ch->energy += energygain;

            ch->pcdata->learned[sn]++;

            sendf(ch, "You advance in %s. You receive %d HP, %d Mana, %d Energy!\n\r", skill_table[sn].name, hpgain, managain, energygain);
            gainf("%s advanced in %s.", ch->short_descr, skill_table[sn].name);
            sprintf(log_buf, "%s advanced in %s, now %d. Got %d hp %d mana. Cost %d xp.", ch->short_descr, skill_table[sn].name, ch->pcdata->learned[sn], hpgain, managain, cost);
            monitor_chan(log_buf, MONITOR_GEN_MORT);

            if (ch->pcdata->learned[sn] == AV_NOVICE) {
                sendf(ch, "@@yYou are now a NOVICE at %s.\n\r", skill_table[sn].name);
                gainf("%s is now a novice at %s.", ch->short_descr, skill_table[sn].name);
                sn += 1;
                ch->pcdata->learned[sn] = 99;
            }
            else if (ch->pcdata->learned[sn] == AV_INTERMEDIATE) {
                sendf(ch, "@@yYou are now an INTERMEDIATE at %s.@@N\n\r", skill_table[sn].name);
                gainf("%s is now intermediate at %s.", ch->short_descr, skill_table[sn].name);
                sn += 2;
                ch->pcdata->learned[sn] = 99;
            }
            else if (ch->pcdata->learned[sn] == AV_ADVANCED) {
                sendf(ch, "@@yYou are now ADVANCED at %s.@@N\n\r", skill_table[sn].name);
                gainf("%s is now advanced at %s.", ch->short_descr, skill_table[sn].name);
                sn += 3;
                ch->pcdata->learned[sn] = 99;
            }
            else if (ch->pcdata->learned[sn] == AV_EXPERT) {
                sendf(ch, "@@yYou are now an EXPERT at %s.@@N\n\r", skill_table[sn].name);
                gainf("%s is now an expert at %s.", ch->short_descr, skill_table[sn].name);
                sn += 4;
                ch->pcdata->learned[sn] = 99;
            }
            else if (ch->pcdata->learned[sn] == AV_MASTER) {
                sendf(ch, "@@y@@fYou are now a MASTER at %s.@@N\n\r", skill_table[sn].name);
                gainf("%s is now a MASTER at %s!", ch->short_descr, skill_table[sn].name);
                sn += 5;
                ch->pcdata->learned[sn] = 99;
            }

            save_char_obj(ch);
        }
    }
    return;
}

void do_unpractice(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *mob;
    int sn;

    if (!IS_AWAKE(ch)) {
        send_to_char("@@WIn your dreams, or what?@@N\n\r", ch);
        return;
    }
 
    for (mob = ch->in_room->first_person; mob != NULL; mob = mob->next_in_room) {
        if (IS_NPC(mob) && IS_SET(mob->act, ACT_PRACTICE))
            break;
    }

    if (mob == NULL) {
        send_to_char("You can't do that here.\n\r", ch);
        return;
    }

    if ((sn = skill_lookup(argument)) < 0) {
        send_to_char("You can't unpractice that.\n\r", ch);
        return;
    }

    if (ch->pcdata->learned[sn] == 0) {
        sendf(ch, "You haven't practiced %s.\n\r", skill_table[sn].name);
        return;
    }

    if (ch->pcdata->learned[sn] == 101) {
       send_to_char("You can't unpractice racial skills.\n\r", ch);
       return;
    }

    if (skill_table[sn].flag1 == AVATAR) {
        send_to_char("Unpractice an avatar skill? That's unthinkable!\n\r", ch);
        return;
    }

    xlogf("%s unpractices %s, was at %d%%.", ch->name, skill_table[sn].name, ch->pcdata->learned[sn]);
    sendf(ch, "You unpractice %s.\n\r", skill_table[sn].name);

    ch->pcdata->learned[sn] = 0;
    save_char_obj(ch);
    return;
}


/*
 * 'Wimpy' originally by Dionysos.
 */
void
do_wimpy(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    char                arg[MAX_INPUT_LENGTH];
    int                 wimpy;

    buf[0] = '\0';

    one_argument(argument, arg);

    if (arg[0] == '\0')
        wimpy = ch->max_hit / 5;
    else
        wimpy = atoi(arg);

    if (wimpy < 0) {
        send_to_char("Your courage exceeds your wisdom.\n\r", ch);
        return;
    }

    if (wimpy > ch->max_hit) {
        send_to_char("Such cowardice ill becomes you.\n\r", ch);
        return;
    }

    ch->wimpy = wimpy;
    sendf(ch, "Wimpy set to %d hit points.\n\r", wimpy);
    return;
}

void
do_password(CHAR_DATA *ch, char *argument)
{
    char                arg1[MAX_INPUT_LENGTH];
    char                arg2[MAX_INPUT_LENGTH];
    char                buf1[MSL];
    char                buf2[MSL];
    char                md5buf[33];
    char               *pArg;
    char               *pwdnew;
    char                cEnd;

    if (IS_NPC(ch))
        return;
    arg1[0] = '\0';
    arg2[0] = '\0';

    /*
     * Can't use one_argument here because it smashes case.
     * So we just steal all its code.  Bleagh.
     */
    pArg = arg1;
    while (isspace(*argument))
        argument++;

    cEnd = ' ';
    if (*argument == '\'' || *argument == '"')
        cEnd = *argument++;

    while (*argument != '\0') {
        if (*argument == cEnd) {
            argument++;
            break;
        }
        *pArg++ = *argument++;
    }
    *pArg = '\0';

    pArg = arg2;
    while (isspace(*argument))
        argument++;

    cEnd = ' ';
    if (*argument == '\'' || *argument == '"')
        cEnd = *argument++;

    while (*argument != '\0') {
        if (*argument == cEnd) {
            argument++;
            break;
        }
        *pArg++ = *argument++;
    }
    *pArg = '\0';

    if ((ch->pcdata->pwd != '\0')
        && (arg1[0] == '\0' || arg2[0] == '\0')) {
        send_to_char("Syntax: password <old> <new>.\n\r", ch);
        return;
    }

    if (ch->pcdata->pwd != '\0') {
        if (strlen(ch->pcdata->pwd) < 32) {
            strcpy(buf1, crypt(arg1, ch->pcdata->pwd));
            strcpy(buf2, ch->pcdata->pwd);
        }
        else {
            strcpy(buf1, md5string(arg1, md5buf));
            strcpy(buf2, ch->pcdata->pwd);
        }

        if (strcmp(buf1, buf2)) {
            WAIT_STATE(ch, 40);
            send_to_char("Wrong password.  Wait 10 seconds.\n\r", ch);
            return;
        }
    }

    if (strlen(arg2) < 5) {
        send_to_char("New password must be at least five characters long.\n\r", ch);
        return;
    }

    pwdnew = md5string(arg2, md5buf);

    free_string(ch->pcdata->pwd);
    ch->pcdata->pwd = str_dup(pwdnew);
    save_char_obj(ch);
    send_to_char("Ok.\n\r", ch);
    return;
}

void
do_socials(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    char                out[MAX_STRING_LENGTH * 2];
    int                 iSocial;
    int                 col;

    buf[0] = '\0';

    col = 0;
    out[0] = '\0';

    for (iSocial = 0; social_table[iSocial].name[0] != '\0'; iSocial++) {
        sprintf(buf, "%-12s", social_table[iSocial].name);
        safe_strcat(MAX_STRING_LENGTH, out, buf);
        if (++col % 6 == 0)
            safe_strcat(MAX_STRING_LENGTH, out, "\n\r");
    }

    if (col % 6 != 0)
        safe_strcat(MAX_STRING_LENGTH, out, "\n\r");
    send_to_char(out, ch);
    return;
}

int commands_cmp(const void *x, const void *y)
{   
    int       *dx = (int *)x;
    int       *dy = (int *)y;
            
    return strcmp(cmd_table[*dx].name, cmd_table[*dy].name);   
}

void commands_list(CHAR_DATA *ch, int imm)
{
    int                 cnt, cmdcnt = 0;
    int                *cmds;
    int                 row = 0, rows = 0, col = 0, cols = 5;

    if (IS_NPC(ch))
        return;

    for (cnt = 0; cmd_table[cnt].name[0] != '\0'; cnt++) {
        if (   (!imm && cmd_table[cnt].level < LEVEL_HERO && cmd_table[cnt].level <= get_trust(ch))
            || (imm && cmd_table[cnt].level >= LEVEL_HERO && cmd_table[cnt].level <= get_trust(ch))) {
            if (cmd_table[cnt].level == CLAN_ONLY && ch->pcdata->clan == 0)
                continue;

            if (cmd_table[cnt].level == BOSS_ONLY && !IS_SET(ch->pcdata->pflags, PFLAG_CLAN_BOSS))
                continue;

            cmdcnt++;
        }
    }

    cmds = (int *)malloc(sizeof(int) * cmdcnt);
    memset(cmds, 0, sizeof(int) * cmdcnt);

    cmdcnt = 0;
    for (cnt = 0; cmd_table[cnt].name[0] != '\0'; cnt++) {
        if (   (!imm && cmd_table[cnt].level < LEVEL_HERO && cmd_table[cnt].level <= get_trust(ch))
            || (imm && cmd_table[cnt].level >= LEVEL_HERO && cmd_table[cnt].level <= get_trust(ch))) {
            if (cmd_table[cnt].level == CLAN_ONLY && ch->pcdata->clan == 0)
                continue;

            if (cmd_table[cnt].level == BOSS_ONLY && !IS_SET(ch->pcdata->pflags, PFLAG_CLAN_BOSS))
                continue;

            cmds[cmdcnt] = cnt;
            cmdcnt++;
        }
    }

    qsort(cmds, cmdcnt, sizeof(int), commands_cmp);

    if (!imm)
        send_to_char("@@N@@d.-------------------------------------------------------@@g=( @@aCommand List @@g)=@@d-.@@N\n\r", ch);
    else
        send_to_char("@@N@@d.----------------------------------------------@@g=( @@aImmortal Command List @@g)=@@d-.@@N\n\r", ch);

    rows = cmdcnt / cols + ((cmdcnt % cols != 0) ? 1 : 0);

    for (row = 0; row < rows; row++) {
        send_to_char("@@d|", ch);

        for (col = 0; col < cols; col++) {
            cnt = col * rows + row;

            if (cnt < cmdcnt)
                sendf(ch, " @@%c%-12.12s @@d|", col % 2 ? 'W' : 'g', cmd_table[cmds[cnt]].name);
            else
                sendf(ch, " %-12.12s @@d|", "");
        }

        send_to_char("@@N\n\r", ch);
    }

    send_to_char("@@d'--------------------------------------------------------------------------'@@N\n\r", ch);
    free(cmds);
    return;
}

void do_commands(CHAR_DATA *ch, char *argument)
{
    commands_list(ch, FALSE);
    return;
}

struct chan_type
{
    int                 bit;
    int                 min_level;
    bool                extra;
    char               *name;
    char               *uppername;
    char               *on_string;
    char               *off_string;
};

struct chan_type    channels[] = {
    /* Immortal channels */
    {CHANNEL_IMMTALK,    81, FALSE, "immtalk",    "IMMTALK",
     "You hear what other immortals have to say",
     "You don't hear what other immortals have to say"},
    {CHANNEL_NOTIFY,     81, FALSE, "notify",     "NOTIFY",
     "You hear player information",
     "You don't hear player information"},
    {CHANNEL_LOG,        81, FALSE, "log",        "LOG",
     "You see players who are logged",
     "You don't see players who are logged"},
    {CHANNEL_CREATOR,    90, FALSE, "creator",    "CREATOR",
     "You hear creators discussing MUD matters",
     "You don't hear creators discussing MUD matters"},
    {CHANNEL_ALLCLAN,    90, FALSE, "allclan",    "ALLCLAN",
     "You hear all clan channels",
     "You don't hear all clan channels"},
    {CHANNEL_ALLRACE,    90, FALSE, "allrace",    "ALLRACE",
     "You hear all race channels",
     "You don't hear all race channels"},

    /* Mortal Channels */
    {CHANNEL_HERMIT,      0, FALSE, "hermit",     "HERMIT",
     "@@eYOU ARE IGNORING ALL CHANNELS",
     "You are NOT ignoring all channels"},
    {CHANNEL_AUCTION,     0, FALSE, "auction",    "AUCTION",
     "You hear auctions",
     "You don't hear auctions"},
    {CHANNEL_GOSSIP,      0, FALSE, "gossip",     "GOSSIP",
     "You hear general gossip",
     "You don't hear general gossip"},
    {CHANNEL_MUSIC,       0, FALSE, "music",      "MUSIC",
     "You hear people singing badly",
     "You don't hear people singing badly"},
    {CHANNEL_QUEST,       0, FALSE, "quest",      "QUEST",
     "You hear roleplaying quests",
     "You ignore roleplaying quests"},
    {CHANNEL_NEWBIE,      0, FALSE, "newbie",     "NEWBIE",
     "You hear newbie chit-chat",
     "You don't hear newbie chit-chat"},
    {CHANNEL_QUESTION,    0, FALSE, "question",   "QUESTION",
     "You hear player questions and answers",
     "You don't hear player questions and answers"},
    {CHANNEL_SHOUT,       0, FALSE, "shout",      "SHOUT",
     "You hear people shouting",
     "You don't hear people shouting"},
    {CHANNEL_YELL,        0, FALSE, "yell",       "YELL",
     "You hear people yelling",
     "You don't hear people yelling"},
    {CHANNEL_CLAN,        0, FALSE, "clan",       "CLAN",
     "You hear clan chit-chat",
     "You don't hear clan chit-chat"},
    {CHANNEL_TRIVIA,        0, FALSE, "trivia",       "TRIVIA",
     "You hear players participating in Trivia games",
     "You don't hear people playing Trivia games"},
    {CHANNEL_RACE,        0, FALSE, "race",       "RACE",
     "You hear your race chit-chat",
     "You don't hear your race chit-chat"},
    {CHANNEL_FLAME,       0, FALSE, "flame",      "FLAME",
     "You hear players flaming each other",
     "You don't hear players flaming each other"},
    {CHANNEL_ZZZ,         0, FALSE, "zzz",        "ZZZ",
     "You hear sleeping players chatting",
     "You don't hear sleeping players chatting"},
    {CHANNEL_INFO,        0, FALSE, "info",       "INFO",
     "You hear information about deaths, etc",
     "You don't hear information about deaths, etc"},
    {CHANNEL_BEEP,        0, FALSE, "beep",       "BEEP",
     "You accept beeps from other players",
     "You are ignoring beeps from other players"},
    {CHANNEL_DIPLOMAT,    0, FALSE, "diplomat",   "DIPLOMAT",
     "You hear diplomatic negotiations",
     "You don't hear diplomatic negotiations"},
    {CHANNEL_REMORTTALK,  0, FALSE, "{",          "{",
     "You hear remorts gossiping amongst themselves",
     "You are ignoring idle remort chatter"},
    {CHANNEL_CRUSADE,     0, FALSE, "crusade",    "CRUSADE",
     "You see players crusading",
     "You are ignoring player crusades"},
    {CHANNEL_AUTOCRUSADE, 0, FALSE, "autocrusade", "AUTOCRUSADE",
     "You see mobiles crusading",
     "You are ignoring mobile crusades"},
    {CHANNEL_ADEPT,       0, FALSE, "adept",      "ADEPT",
     "You hear adept discussions",
     "You don't hear adept discussions"},
    {CHANNEL_OOC,         0, FALSE, "ooc",        "OOC",
     "You hear non-roleplaying chit-chat",
     "You don't hear non-roleplaying chit-chat"},
    {CHANNEL_AVATAR,   0, FALSE, "avatar",      "AVATAR",
     "You hear Avatars plotting deaths",
     "You don't hear Avatars plotting deaths"},
    {CHANNEL_PKOK,        0, FALSE, "pkok",       "PKOK",
     "You hear PKOK players planning mortal combat",
     "You are ignoring PKOK discussion"},
    {CHANNEL2_CHALLENGE,  0,  TRUE, "challenge",  "CHALLENGE",
     "You see spar/duel messages",
     "You don't see spar/duel messages"},
    {CHANNEL2_GAIN,       0,  TRUE, "gain",       "GAIN",
     "You see people gaining levels",
     "You don't see people gaining levels"},
    {CHANNEL2_ALLY,       0,  TRUE, "ally",       "ALLY",
     "You hear alliance conversations",
     "You don't hear alliance conversations"},
    {CHANNEL2_ARENA,      0,  TRUE, "arena",      "ARENA",
     "You hear people battling in the arena",
     "You don't hear people battling in the arena"},

    {0, 0, FALSE, NULL, NULL, NULL}
};

void
do_channels(CHAR_DATA *ch, char *argument)
{
    char                arg[MIL];
    char                buffer[MSL];
    char                buf[MSL], lbuf1[MSL], lbuf2[MSL];
    int                 a, trust, *two;
    bool                on = FALSE;

    two = NULL;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
        if (!IS_NPC(ch) && IS_SET(ch->act, PLR_SILENCE)) {
            send_to_char("You are SILENCED.\n\r", ch);
            return;
        }

        trust = get_trust(ch);
        buffer[0] = '\0';
        safe_strcat(MSL, buffer, "@@d.--------------------------------------------------------------@@g=( @@yChannels @@g)=@@d-.\n\r");

        for (a = 0; channels[a].bit != 0; a++) {
            if (trust >= channels[a].min_level && strcmp(channels[a].off_string, "")) {
                if (   (!channels[a].extra && IS_SET(ch->deaf, channels[a].bit))
                    || (channels[a].extra  && IS_SET(ch->deaf2, channels[a].bit)))
                    on = FALSE;
                else
                    on = TRUE;

                /* hermit is a special case, if hermit is "deaf" then its enabled
                 * which is the opposite of normal, so we switch the flags */
                if (!channels[a].extra && channels[a].bit == CHANNEL_HERMIT)
                    on = !on;

                if (!on) {
                    sprintf(buf, "@@d| @@b-%s @@d: @@g%s @@N@@d|\n\r",
                        my_left(channels[a].name, lbuf1, 13),
                        my_left(channels[a].off_string, lbuf2, 58)
                    );
    
                    safe_strcat(MSL, buffer, buf);
                }
                else {
                    sprintf(buf, "@@d| @@y+%s @@d: @@g%s @@N@@d|\n\r",
                        my_left(channels[a].uppername, lbuf1, 13),
                        my_left(channels[a].on_string, lbuf2, 58)
                    );
    
                    safe_strcat(MSL, buffer, buf);
                }
            }

            safe_strcat(MSL, buffer, "");
        }

        send_to_char(buffer, ch);
        send_to_char("@@d'-----------------------------------------------------------------------------'@@N\n\r", ch);
    }
    else {
        bool                fClear;
        int                 bit;
        char               *on = NULL, *off = NULL;

        if (arg[0] == '+')
            fClear = TRUE;
        else if (arg[0] == '-')
            fClear = FALSE;
        else {
            send_to_char("Channels -channel or +channel?\n\r", ch);
            return;
        }

        /* Now check through table to set/unset channel... */
        bit = 0;
        for (a = 0; channels[a].bit != 0; a++) {
            if (channels[a].min_level > get_trust(ch))
                continue;
            if (!str_prefix(arg + 1, channels[a].name)) {
                bit = channels[a].bit;
                two = (channels[a].extra) ? &ch->deaf2 : &ch->deaf;
                on = channels[a].on_string;
                off = channels[a].off_string;
                break;
            }
        }

        if (bit == 0) {
            send_to_char("Set or clear which channel?\n\r", ch);
            return;
        }

        /* again, hermit is special.. make everything reversed */
        if (!channels[a].extra && channels[a].bit == CHANNEL_HERMIT) {
            fClear = !fClear;
            on = channels[a].off_string;
            off = channels[a].on_string;
        }

        if (fClear)
            REMOVE_BIT(*two, bit);
        else
            SET_BIT(*two, bit);

        sendf(ch, "@@N@@g%s@@N@@g.@@N\n\r",
            !fClear ? off : on);
    }

    return;
}

/*
 * Contributed by Grodyn.
 */

struct config_type {
    int bit;
    bool config;
    char *off;
    char *on;
    char *onlong;
    char *offlong;
};

void
do_config(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    int                 cnt = 0;
    int                *bit = NULL;

    struct config_type config_table[] = {
        { PLR_NOSUMMON,   FALSE, "nosummon  ", "NOSUMMON  ",
            "You may not be summoned.",
            "You may be summoned." },
        { PLR_NOVISIT,    FALSE, "novisit   ", "NOVISIT   ",
            "You may not be 'visited'.",
            "You may be 'visited'." },
        { PLR_COLOUR,     FALSE, "colour    ", "COLOUR    ",
            "You receive ANSI colour.",
            "You don't receive ANSI colour." },
        { PLR_AUTOEXIT,   FALSE, "autoexit  ", "AUTOEXIT  ",
            "You automatically see exits.",
            "You don't automatically see exits." },
        { PLR_AUTOGOLD,   TRUE,  "autogold  ", "AUTOGOLD  ",
            "You automatically retrieve gold from corpses.",
            "You leave gold in corpses." },
        { PLR_AUTOSPLIT,  TRUE,  "autosplit ", "AUTOSPLIT ",
            "You automatically split gold amongst group members.",
            "You don't automatically split gold amongst group members." },
        { PLR_MASKQP,     TRUE,  "maskqp    ", "MASKQP    ",
            "You mask qps used by the qinfo command.",
            "You don't mask qps used by the qinfo command." },
        { PLR_ANSWERING,  TRUE,  "answering ", "ANSWERING ",
            "You have your AFK answering machine enabled.",
            "You have your AFK answering machine disabled." },
        { PLR_AUTOLOOT,   FALSE, "autoloot  ", "AUTOLOOT  ",
            "You automatically loot mob corpses.",
            "You don't automatically loot mob corpses." },
        { PLR_AUTOSAC,    FALSE, "autosac   ", "AUTOSAC   ",
            "You automatically sacrifice corpses.",
            "You don't automatically sacrifice corpses." },
        { PLR_BLANK,      FALSE, "blank     ", "BLANK     ",
            "You have a blank line before your prompt.",
            "You have no blank line before your prompt." },
        { PLR_BRIEF,      FALSE, "brief     ", "BRIEF     ",
            "You see brief descriptions.",
            "You see long descriptions." },
        { PLR_BRIEF2,     FALSE, "brief2    ", "BRIEF2    ",
            "You do not see group members on look auto.",
            "You see group members on look auto." },
        { PLR_COMBINE,    FALSE, "combine   ", "COMBINE   ",
            "You see object lists in combined format.",
            "You see object lists in singular format." },
        { PLR_PROMPT,     FALSE, "prompt    ", "PROMPT    ",
            "You have a prompt.",
            "You don't have a prompt." },
        { PLR_SHOWBLACK,  TRUE,  "showblack ", "SHOWBLACK ",
            "You are viewing black as dark grey.",
            "You are viewing black normally." },
        { PLR_SHOWDAMAGE, TRUE,  "showdamage", "SHOWDAMAGE",
            "You are viewing damage amount.",
            "You are not viewing damage amount." },
        { PLR_NOGIVE,     TRUE,  "nogive    ", "NOGIVE    ",
            "You can't be given items.",
            "You can be given items." },
        { PLR_SAVECHECK,  TRUE,  "savecheck ", "SAVECHECK ",
            "You don't manually quit out with objects you can't save.",
            "You quit out, even with objects you can't save." },
        { PLR_NORESCUE,   TRUE,  "norescue  ", "NORESCUE  ",
            "You can't be rescued.",
            "You can be rescued." },
        { PLR_PACIFIST,   TRUE,  "pacifist  ", "PACIFIST  ",
            "You don't land normal attacks on charmed mobiles.",
            "You land normal attacks on charmed mobiles." },
        { PLR_NOBOND,     TRUE,  "nobond    ", "NOBOND    ",
            "Non-group-range players can't creature bond your charmed mobiles.",
            "Non-group-range players can creature bond your charmed mobiles." },
        { PLR_NOFOLLOW,   TRUE,  "nofollow  ", "NOFOLLOW  ",
            "You can't be followed by players.",
            "You can be followed by players." },
        { PLR_NOOBJFUN,   TRUE,  "noobjspam ", "NOOBJSPAM ",
            "Objects are noiseless.",
            "Objects are noisy.", },
        { 0, FALSE, "", "", "", "" }
    };

    if (IS_NPC(ch))
        return;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
        send_to_char("@@d.----------------------------------------------@@g=( @@WConfig @@g)=@@d-\n\r", ch);

        for (cnt = 0; config_table[cnt].bit != 0; cnt++) {
            bit = (config_table[cnt].config) ? &ch->config : &ch->act;

            if (IS_SET(*bit, config_table[cnt].bit))
                sendf(ch, "@@d| @@a+%s @@d: @@g%s@@N\n\r", config_table[cnt].on, config_table[cnt].onlong);
            else
                sendf(ch, "@@d| @@c-%s @@d: @@g%s@@N\n\r", config_table[cnt].off, config_table[cnt].offlong);
        }

        send_to_char("@@d'-----------------------------------------------------------@@N\n\r", ch);
    }
    else {
        bool                fSet = FALSE, fToggle = FALSE;
        int                *bit;
        int                 set;
        char               *a = arg;

        if (*a == '+')
            fSet = TRUE;
        else if (*a == '-')
            fSet = FALSE;
        else if (*a == '?')
            fToggle = TRUE;
        else {
            send_to_char("@@N@@gsyntax: config <-|+|?>option. See @@Whelp config@@g for further details.@@N\n\r", ch);
            return;
        }

        a++;

        if (*a == '\0') {
            send_to_char("@@N@@gsyntax: config <-|+|?>option. See @@Whelp config@@g for further details.@@N\n\r", ch);
            return;
        }

        if (!str_prefix(a, "color"))
            strcpy(a, "colour");

        bit = NULL; set = 0;

        for (cnt = 0; config_table[cnt].bit != 0; cnt++) {
            bit = (config_table[cnt].config) ? &ch->config : &ch->act;
            set = config_table[cnt].bit;

            if (!str_prefix(a, config_table[cnt].on))
                break;
        }

        if (config_table[cnt].bit == 0) {
            send_to_char("Config which option? Type config for a list of options.\n\r", ch);
            return;
        }

        if (fSet) {
            sendf(ch, "%s\n\r", config_table[cnt].onlong);
            SET_BIT(*bit, set);
        }
        else if (fToggle) {
            if (IS_SET(*bit, set)) {
                sendf(ch, "%s\n\r", config_table[cnt].offlong);
                REMOVE_BIT(*bit, set);
            }
            else {
                sendf(ch, "%s\n\r", config_table[cnt].onlong);
                SET_BIT(*bit, set);
            }
        }
        else {
            sendf(ch, "%s\n\r", config_table[cnt].offlong);
            REMOVE_BIT(*bit, set);
        }
    }

    return;
}

void
do_wizlist(CHAR_DATA *ch, char *argument)
{

    do_help(ch, "wizlist");
    return;

}

void
do_race_list(CHAR_DATA *ch, char *argument)
{
    int  iRace;
    char buf1[80];

    if (!IS_IMMORTAL(ch)) {
        send_to_char("@@N@@d.------------------------------------@@g=( @@WRaces @@g)=@@d-.\n\r", ch);
        send_to_char("@@d| @@gRace @@d| @@gFull Name @@d| @@gStr @@d| @@gInt @@d| @@gWis @@d| @@gDex @@d| @@gCon @@d|\n\r", ch);
        send_to_char("@@d|------+-----------+-----+-----+-----+-----+-----|\n\r", ch);
    }
    else {
        send_to_char("@@N@@d.-------------------------------------------------@@g=( @@WRaces @@g)=@@d-.\n\r", ch);
        send_to_char("@@d|  @@g# @@d|  @@gWord @@d| @@gRace @@d| @@gFull Name @@d| @@gStr @@d| @@gInt @@d| @@gWis @@d| @@gDex @@d| @@gCon @@d|\n\r", ch);
        send_to_char("@@d|----+-------+------+-----------+-----+-----+-----+-----+-----|\n\r", ch);
    }

    for (iRace = 0; iRace < MAX_RACE; iRace++) {
        if (!IS_IMMORTAL(ch))
            sendf(ch, "@@d| @@g%-4s @@d| @@g%s @@d| @@g%3d @@d| @@g%3d @@d| @@g%3d @@d| @@g%3d @@d| @@g%3d @@d|\n\r",
                race_table[iRace].race_name,
                my_left(race_table[iRace].race_title, buf1, 9),
                race_table[iRace].race_str,
                race_table[iRace].race_int,
                race_table[iRace].race_wis,
                race_table[iRace].race_dex,
                race_table[iRace].race_con
            );
        else
            sendf(ch, "@@d| @@g%2d @@d| @@g%5d @@d| @@g%-4s @@d| @@g%s @@d| @@g%3d @@d| @@g%3d @@d| @@g%3d @@d| @@g%3d @@d| @@g%3d @@d|\n\r",
                iRace,
                race_table[iRace].recall,
                race_table[iRace].race_name,
                my_left(race_table[iRace].race_title, buf1, 9),
                race_table[iRace].race_str,
                race_table[iRace].race_int,
                race_table[iRace].race_wis,
                race_table[iRace].race_dex,
                race_table[iRace].race_con
            );
    }

    if (!IS_IMMORTAL(ch))
        send_to_char("@@d'------------------------------------------------'@@N\n\r", ch);
    else
        send_to_char("@@d'-------------------------------------------------------------'@@N\n\r", ch);

    send_to_char("\n\r@@gNote: The attributes listed are the base attribute values. You can add or\n\rremove a maximum of 3 to each one, with a grand total not exceeding 90.@@N\n\r", ch);
    return;
}

void
do_clan_list(CHAR_DATA *ch, char *argument)
{
    int                 iClan;
    char                lbuf[MIL], nbuf[MIL];

    if (!IS_IMMORTAL(ch)) {
        send_to_char("@@N@@d.-------------------------------------------------------@@g=( @@WClan List @@g)=@@d-.\n\r", ch);
        send_to_char("@@d|  @@a# @@d| @@gAbbr  @@d| @@gLeader(s)                 @@d| @@gName                         @@d|\n\r", ch);
        send_to_char("@@d|----+-------+---------------------------+------------------------------|\n\r", ch);
    }
    else {
        send_to_char("@@N@@d.-------------------------------------------------------------@@g=( @@WClan List @@g)=@@d-.\n\r", ch);
        send_to_char("@@d|  @@a# @@d|  @@WRoom @@d|  @@WCdon @@d| @@gAbbr  @@d| @@gLeader(s)       @@d| @@gName                         @@d|\n\r", ch);
        send_to_char("@@d|----+-------+-------+-------+-----------------+------------------------------|\n\r", ch);
    }

    for (iClan = 0; iClan < MAX_CLAN; iClan++) {
        if (!IS_IMMORTAL(ch) && !str_cmp(clan_table[iClan].clan_abbr, "None "))
            continue;            /* Don't list 'none' as a clan for players :) */

        if (!IS_IMMORTAL(ch))
            sendf(ch, "@@d| @@a%2d @@d| @@g%s @@d| @@g%s @@d| @@g%s @@d|\n\r", iClan, clan_table[iClan].clan_abbr, my_left(clan_table[iClan].leader, lbuf, 25), my_left(clan_table[iClan].clan_name, nbuf, 28));
        else
            sendf(ch, "@@d| @@a%2d @@d| @@W%5d @@d| @@W%5d @@d| @@g%s @@d| @@g%s @@d| @@g%s @@d|\n\r",
                iClan, clan_table[iClan].clan_room, clan_table[iClan].donat_room, clan_table[iClan].clan_abbr, my_left(clan_table[iClan].leader, lbuf, 15), my_left(clan_table[iClan].clan_name, nbuf, 28));
    }

    if (!IS_IMMORTAL(ch))
        send_to_char("@@d'-----------------------------------------------------------------------'@@N\n\r", ch);
    else
        send_to_char("@@d'-----------------------------------------------------------------------------'@@N\n\r", ch);

    return;
}

int spells_list_cmp(const void *x, const void *y)
{
    int       *dx = (int *)x;
    int       *dy = (int *)y;

    return strcmp(skill_table[*dx].name, skill_table[*dy].name);
}

void do_spells(CHAR_DATA *ch, char *argument)
{
    char buf[MSL], buf2[MSL];
    char perc[16];
    char skillbuf[32];
    int row, rows, col, cols = 3;
    int *skills;
    int cnt, sn, skillcnt = 0;

    for (cnt = 0; cnt < MAX_SKILL; cnt++)
        if (ch->pcdata->learned[cnt] > 0)
            skillcnt++;

    if (skillcnt == 0) {
        send_to_char("You have no spells practiced.\n\r", ch);
        return;
    }

    skills = (int *)malloc(sizeof(int) * skillcnt);
    memset(skills, 0, sizeof(int) * skillcnt);

    skillcnt = 0;

    for (cnt = 0; cnt < MAX_SKILL; cnt++)
        if (ch->pcdata->learned[cnt] > 0 && skill_table[cnt].slot != 0) {
            if (skill_table[cnt].flag1 == AVATAR && skill_table[cnt].flag2 == NORM)
                continue;

            skills[skillcnt] = cnt;
            skillcnt++;
        }

    qsort(skills, skillcnt, sizeof(int), spells_list_cmp);

    send_to_char("@@N@@d.-----------------------------------------------------------@@g=( @@WSpell Costs @@g)=-@@d.\n\r", ch);

    rows = skillcnt / cols + ((skillcnt % cols != 0) ? 1 : 0);

    for (row = 0; row < rows; row++) {
        send_to_char("@@d|", ch);

        for (col = 0; col < cols; col++) {
            cnt = col * rows + row;
            sn = cnt < skillcnt ? skills[cnt] : -1;

            if (sn > -1) {
                sprintf(perc, "%4d", mana_cost(ch, sn));
                sprintf(skillbuf, "%s%s", ch->pcdata->learned[sn] >= best_learnt(ch, sn) ? "@@g" : "@@e", skill_table[sn].name);
                sprintf(buf, " %s %s%s @@d|", my_left(skillbuf, buf2, 22 - strlen(perc)), skill_table[sn].flag1 == AVATAR ? "@@a" : "@@W", perc);
            }
            else
                sprintf(buf, "                         @@d|");

            send_to_char(buf, ch);
        }

        send_to_char("@@N\n\r", ch);
    }

    send_to_char("@@d'-----------------------------------------------------------------------------'@@N\n\r", ch);

    free(skills);
    return;
}

/*
void
do_spells(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    char                buf1[MAX_STRING_LENGTH];
    int                 sn;
    int                 col;
    int                 cnt;
    bool                ok;

    buf[0] = '\0';

    if (IS_NPC(ch)) {
        send_to_char("You do not know how to cast spells!\n\r", ch);
        return;
    }

    buf1[0] = '\0';

    col = 0;
    for (sn = 0; sn < MAX_SKILL; sn++) {
        ok = FALSE;

        if (skill_table[sn].name == NULL)
            break;
        for (cnt = 0; cnt < MAX_CLASS; cnt++)
            if ((ch->lvl[cnt] >= skill_table[sn].skill_level[cnt])
                && (skill_table[sn].skill_level[cnt] < LEVEL_HERO))
                ok = TRUE;

        if (ch->pcdata->learned[sn] == 0)
            continue;

        if (skill_table[sn].slot == 0)
            continue;

        sprintf(buf, "%18s %3dpts ", skill_table[sn].name, mana_cost(ch, sn));
        safe_strcat(MAX_STRING_LENGTH, buf1, buf);
        if (++col % 3 == 0)
            safe_strcat(MAX_STRING_LENGTH, buf1, "\n\r");
    }

    if (col % 3 != 0)
        safe_strcat(MAX_STRING_LENGTH, buf1, "\n\r");

    send_to_char(buf1, ch);
    return;

}
*/

void
do_slist(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    char                buf1[MAX_STRING_LENGTH];
    char                skillname[64];
    char               *pskillname;
    int                 class;
    int                 foo;
    int                 sn;
    int                 col;
    int                 level;
    int                 best;
    bool                any;
    bool                mortal_class;
    bool                remort_class;
    bool                adept_class;
    bool                avatar_class;
    int                 maxcolumns = 0;
    int                 columns = 0;
    char                *classname = NULL;
    char                *str = NULL;

    buf[0] = '\0';
    buf1[0] = '\0';

    if (IS_NPC(ch)) {
        send_to_char("You do not need any stinking spells!\n\r", ch);
        return;
    }

    if (argument[0] == '\0') {
        send_to_char("syntax: slist <mag|cle|thi|war|psi|sor|mon|ass|kni|nec|adept|avatar>\n\r", ch);
        return;
    }

    any = FALSE;
    class = -1;
    mortal_class = FALSE;
    remort_class = FALSE;
    adept_class = FALSE;
    avatar_class = FALSE;

    for (foo = 0; foo < MAX_CLASS; foo++)
        if (!str_cmp(class_table[foo].who_name, argument)) {
            any = TRUE;
            class = foo;
            mortal_class = TRUE;
            classname = class_table[foo].class_name;
        }
        else if (!str_cmp(remort_table[foo].who_name, argument)) {
            any = TRUE;
            class = foo;
            remort_class = TRUE;
            classname = remort_table[foo].class_name;
        }
        else if (!str_cmp("adept", argument)) {
            any = TRUE;
            adept_class = TRUE;
            class = 0;
            classname = "Adept";
        }
        else if (!str_cmp("avatar", argument)) {
            any = TRUE;
            avatar_class = TRUE;
            class = 0;
            classname = "Avatar";
        }

    if (!any) {
        send_to_char("That abbreviation not recognised! Recognised abbreviations are:\n\r"
                     "  mag, cle, thi, war, psi, sor, mon, ass, kni, nec, adept, or avatar.\n\r", ch);
        return;
    }

    /* before anything happens, determine how many columns we have (where columns span
     * because skills are at the same level). max of 3 columns.
     */
    for (level = 1; level < LEVEL_IMMORTAL; level++) {
        columns = 0;

        for (sn = 0; sn < MAX_SKILL; sn++) {
            if (skill_table[sn].name == NULL)
                break;

            if (   (skill_table[sn].skill_level[class] != level && !avatar_class)
                || (   (mortal_class && skill_table[sn].flag1 != MORTAL)
                    || (remort_class && skill_table[sn].flag1 != REMORT)
                    || (adept_class  && skill_table[sn].flag1 != ADEPT)
                   )
                || (avatar_class && level == 1 && skill_table[sn].flag1 != AVATAR)
                || (avatar_class && level == 1 && skill_table[sn].flag2 != NORM)
                || (avatar_class && level > 1)
               )
                continue;

            columns = UMIN(3, columns + 1);
        }

        if (columns > maxcolumns)
            maxcolumns = columns;
    }

    buf[0] = '\0';

    strcpy(buf, "@@d.");
    for (foo = ((maxcolumns - 1) * 24) + 6; foo > 0; foo--)
       strcat(buf, "-");

    strcat(buf, "@@g=( @@WSkill/Spell List @@g)=@@d-.@@N\n\r");
    send_to_char(buf, ch);

    sendf(ch, "@@d| @@rLv @@d| @@g%s @@d|@@N\n\r", my_left(classname, buf1, (maxcolumns * 24 - 2)));

    strcpy(buf, "@@d|----+");
    for (foo = maxcolumns * 24; foo > 0; foo--)
       strcat(buf, "-");

    strcat(buf, "@@d|@@N\n\r");
    send_to_char(buf, ch);

    buf[0] = '\0';
    buf1[0] = '\0';

    for (level = 1; level < LEVEL_IMMORTAL; level++) {
        col = 0;
        buf[0] = '\0';

        for (sn = 0; sn < MAX_SKILL; sn++) {
            if (skill_table[sn].name == NULL)
                break;

            if (   (skill_table[sn].skill_level[class] != level && !avatar_class)
                || (   (mortal_class && skill_table[sn].flag1 != MORTAL)
                    || (remort_class && skill_table[sn].flag1 != REMORT)
                    || (adept_class  && skill_table[sn].flag1 != ADEPT)
                   )
                || (avatar_class && level == 1 && skill_table[sn].flag1 != AVATAR)
                || (avatar_class && level == 1 && skill_table[sn].flag2 != NORM)
                || (avatar_class && level > 1)
               )
                continue;

            best = best_learnt(ch, sn);

            pskillname = skillname;
            str = skill_table[sn].name;
            while (*str != '\0') {
                *pskillname++ = LOWER(*str);
                str++;
            }

            *pskillname = '\0';

            if (ch->pcdata->learned[sn] == 0 && best == 0)
                str = "@@d";
            else if (ch->pcdata->learned[sn] == 0 && best > 0)
                str = "@@a";
            else if (ch->pcdata->learned[sn] < best)
                str = "@@e";
            else
                str = "@@m";

            if (avatar_class && col == 0)
                sprintf(buf, "@@d|    |@@g ");
            else if (col == 0)
                sprintf(buf, "@@d| @@r%2d @@d|@@g ", level);
            else if (col % maxcolumns == 0)
                sprintf(buf, "@@d|    |@@g ");

            col++;

            sprintf(buf + strlen(buf), "%s%s ", str, my_left(skillname, buf1, 23));

            if ((col - 1) % maxcolumns == maxcolumns - 1) {
                sprintf(buf + strlen(buf) - 1, "@@d|@@N\n\r");
                send_to_char(buf, ch);
                buf[0] = '\0';
            }
        }

        if (buf[0] != '\0') {
            send_to_char(buf, ch);
            buf[0] = '\0';
/*            sendf(ch, "Leftover columns: %d\n\r", maxcolumns - (col % maxcolumns)); */
            for (foo = (maxcolumns - (col % maxcolumns)) * 24 - 1; foo > 0; foo--)
                strcat(buf, " ");

            strcat(buf, "@@d|@@N\n\r");
            send_to_char(buf, ch);
        }
    }

    strcpy(buf, "@@d'-----");
    for (foo = maxcolumns * 24; foo > 0; foo--)
       strcat(buf, "-");

    strcat(buf, "@@d'@@N\n\r");
    send_to_char(buf, ch);

    return;

}

/* by passing the conf command - Kahn */

void
do_autoexit(CHAR_DATA *ch, char *argument)
{
    do_config(ch, "?autoexit");
    return;
}

void
do_autoloot(CHAR_DATA *ch, char *argument)
{
    do_config(ch, "?autoloot");
    return;
}

void
do_autosac(CHAR_DATA *ch, char *argument)
{
    do_config(ch, "?autosac");
    return;
}

void
do_autogold(CHAR_DATA *ch, char *argument)
{
    do_config(ch, "?autogold");
    return;
}

void
do_autosplit(CHAR_DATA *ch, char *argument)
{
    do_config(ch, "?autosplit");
    return;
}

void
do_blank(CHAR_DATA *ch, char *argument)
{
    do_config(ch, "?blank");
    return;
}

void
do_brief(CHAR_DATA *ch, char *argument)
{
    do_config(ch, "?brief");
    return;
}

void
do_brief2(CHAR_DATA *ch, char *argument)
{
    do_config(ch, "?brief2");
    return;
}

void
do_combine(CHAR_DATA *ch, char *argument)
{
    do_config(ch, "?combine");
    return;
}

void
do_showdamage(CHAR_DATA *ch, char *argument)
{
    do_config(ch, "?showdamage");
    return;
}


void
do_nosummon(CHAR_DATA *ch, char *argument)
{
    do_config(ch, "?nosummon");
    return;
}

void
do_novisit(CHAR_DATA *ch, char *argument)
{
    do_config(ch, "?novisit");
    return;
}

void
do_nogive(CHAR_DATA *ch, char *argument)
{
    do_config(ch, "?nogive");
    return;
}

void
do_norescue(CHAR_DATA *ch, char *argument)
{
    do_config(ch, "?norescue");
    return;
}

void
do_nobond(CHAR_DATA *ch, char *argument)
{
    do_config(ch, "?nobond");
    return;
}

void
do_nofollow(CHAR_DATA *ch, char *argument)
{
    do_config(ch, "?nofollow");
    return;
}

void
do_noobjspam(CHAR_DATA *ch, char *argument)
{
    do_config(ch, "?noobjspam");
    return;
}

void
do_pacifist(CHAR_DATA *ch, char *argument)
{
    do_config(ch, "?pacifist");
    return;
}

void
do_showblack(CHAR_DATA *ch, char *argument)
{
    do_config(ch, "?showblack");
    return;
}

void
do_answering(CHAR_DATA *ch, char *argument)
{
    do_config(ch, "?answering");
    return;
}

void
do_maskqp(CHAR_DATA *ch, char *argument)
{
    do_config(ch, "?maskqp");
    return;
}

void
do_pagelen(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    int                 lines;

    one_argument(argument, arg);

    /* bugfix, NPCs dont have pcdata ;) */
    if (IS_NPC(ch)) {
        send_to_char("Not on NPCs!\n\r", ch);
        return;
    }

    if (arg[0] == '\0')
        lines = 20;
    else
        lines = atoi(arg);

    if (lines < 1) {
        send_to_char("Negative or Zero values for a page pause is not legal.\n\r", ch);
        return;
    }

    ch->pcdata->pagelen = lines;
    sendf(ch, "Page pause set to %d lines.\n\r", lines);
    return;
}

void
do_bprompt(CHAR_DATA *ch, char *argument)
{
    char                buf[MSL];

    if (IS_NPC(ch) || !ch->pcdata)
        return;

    if (argument[0] == '\0') {
        char               *a, *b;

        sendf(ch, "@@N@@gYour current battle prompt is: %s@@N.\n\r", ch->pcdata->battleprompt);

        a = buf;
        b = ch->pcdata->battleprompt;

        while (*b != '\0') {
            if (*b == '@' && *(b + 1) == '@') {
                b += 2;
                *a++ = '@';
                *a++ = '@';
                *a++ = '_';
            }
            else
                *a++ = *b++;
        }

        *a = '\0';

        sendf(ch, "@@N@@gYour current battle prompt (raw) is: %s@@N\n\r", buf);
        return;
    }

    if (!str_cmp("standard", argument) || !str_cmp("1", argument)) {
        send_to_char("Your battle prompt has been set to the standard: @@N@@a[@@W%d@@a:%o@@a] [@@WVictim:%O@@a]@@N%c\n\r", ch);

        if (ch->pcdata->battleprompt)
            free_string(ch->pcdata->battleprompt);

        ch->pcdata->battleprompt = str_dup("@@N@@a[@@W%d@@a:%o@@a] [@@WVictim:%O@@a]@@N%c");
        do_save(ch, "");
        return;
    }

    if (!str_cmp("extended", argument) || !str_cmp("2", argument)) {
        send_to_char("Your battle prompt has been set to the extended: @@N@@g[%n @@d(%b@@d)@@g] [%N @@d(%B@@d)@@g]@@N%c\n\r", ch);

        if (ch->pcdata->battleprompt)
            free_string(ch->pcdata->battleprompt);

        ch->pcdata->battleprompt = str_dup("@@N@@g[%n @@d(%b@@d)@@g] [%N @@d(%B@@d)@@g]@@N%c");
        do_save(ch, "");
        return;
    }

    smash_tilde(argument);

    if (ch->pcdata->battleprompt)
        free_string(ch->pcdata->battleprompt);

    ch->pcdata->battleprompt = str_dup(argument);

    sendf(ch, "@@N@@gYour battle prompt has been set to: %s@@N\n\r", argument);
    do_save(ch, "");

    return;
}

void
do_nprompt(CHAR_DATA *ch, char *argument)
{
    char                buf[MSL];

    if (IS_NPC(ch) || !ch->pcdata)
        return;

    if (argument[0] == '\0') {
        char               *a, *b;

        sendf(ch, "@@N@@gYour current note prompt is: %s@@N.\n\r", ch->pcdata->noteprompt);

        a = buf;
        b = ch->pcdata->noteprompt;

        while (*b != '\0') {
            if (*b == '@' && *(b + 1) == '@') {
                b += 2;
                *a++ = '@';
                *a++ = '@';
                *a++ = '_';
            }
            else
                *a++ = *b++;
        }

        *a = '\0';

        sendf(ch, "@@N@@gYour current note prompt (raw) is: %s@@N\n\r", buf);
        return;
    }

    if (!str_cmp("standard", argument) || !str_cmp("1", argument)) {
        send_to_char("Your note prompt has been set to the standard: @@N@@R[@@e[@@W[@@g You have @@y%a @@gnew note%s @@W]@@e]@@R]@@N%c\n\r", ch);

        if (ch->pcdata->noteprompt)
            free_string(ch->pcdata->noteprompt);

        ch->pcdata->noteprompt = str_dup("@@N@@R[@@e[@@W[@@g You have @@y%a @@gnew note%s @@W]@@e]@@R]@@N%c");
        do_save(ch, "");
        return;
    }

    smash_tilde(argument);

    if (ch->pcdata->noteprompt)
        free_string(ch->pcdata->noteprompt);

    ch->pcdata->noteprompt = str_dup(argument);

    sendf(ch, "@@N@@gYour note prompt has been set to: %s@@N\n\r", argument);
    do_save(ch, "");

    return;
}

/* Do_prompt from Morgenes from Aldara Mud */
void
do_prompt(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    const char *prompts[] = {
        "@@W<@@e%h@@g/@@R%H @@Whp @@a%m@@g/@@c%M @@Wmp @@y%x @@Wxp %t %s> @@N",
        "@@d(@@y%h@@g/@@b%H@@d[@@g%j@@d]|@@a%m@@g/@@c%M@@d[@@g%J@@d]|@@g%x@@d|@@g%g@@d|@@g%s@@d|@@g%t@@d [@@g%!@@Whr @@g%+@@Wdr @@g%*@@Wac@@d])@@N%c",
        "<@@e%h@@N/@@R%H @@a%m@@d/@@c%M @@r%v @@d[@@R%s@@d] @@p%x @@y%g @@g%! @@d%+ @@a%* @@W%t@@N>",
        "@@d[ @@e%h@@d/@@R%H @@dhp ] [ @@l%m@@d/@@B%M @@dmana ] [ %x @@GE@@rx@@GP @@d]@@N%c",
        "@@R%h@@g/@@d%H %m@@g/@@R%M @@d%x@@gxp @@R%s@@N",
        "@@c|@@g%h@@c/@@d%H@@d(@@R%j@@d)@@c|@@g%m@@c/@@d%M@@d(@@R%J@@d)@@c|@@d%g@@c|@@g%s@@c|@@d%t@@c|@@e[@@R[@@d%x@@R]@@e]@@N"
    };

    buf[0] = '\0';

    if (IS_NPC(ch))
        return;

    if (argument[0] == '\0') {
        char               *a;
        char               *b;

        sendf(ch, "Your current prompt is: %s@@N\n\r", ch->pcdata->prompt);

        a = buf;
        b = ch->pcdata->prompt;

        while (*b != '\0') {
            if (*b == '@' && *(b + 1) == '@') {
                b += 2;
                *a++ = '@';
                *a++ = '@';
                *a++ = '_';
            }
            else
                *a++ = *b++;
        }

        *a = '\0';

        sendf(ch, "@@N@@gYour current prompt (raw) is: %s@@N\n\r", buf);
        return;
    }

    if (!strcmp(argument, "all"))
        strcpy(buf, "%N%n<%hhp %mm %vmv> ");
    else if (is_number(argument)) {
        int                 num = atoi(argument) - 1;
        char               *pr;

        if (num < 0 || num >= sizeof(prompts) / sizeof(prompts[0])) {
            send_to_char("Unable to find that predefined prompt.\n\r", ch);
            return;
        }

        pr = (char *)prompts[num];

        free_string(ch->pcdata->prompt);
        ch->pcdata->prompt = str_dup(pr);
        send_to_char("Ok.\n\r", ch);
        return;
    }
    else if (!str_cmp("list", argument)) {
        int num;

        if (!ch->pcdata->prompt || !ch->desc)
            return;

        strcpy(buf, ch->pcdata->prompt);
        free_string(ch->pcdata->prompt);

        send_to_char("@@N@@gYou may use the following predefined prompts by doing @@Wprompt <number>@@g:@@N\n\r\n\r", ch);

        for (num = 0; num < sizeof(prompts) / sizeof(prompts[0]); num++) {
            sendf(ch, "@@N@@g%d@@d:@@N\n\r\n\r", num + 1);
            ch->pcdata->prompt = str_dup(prompts[num]);
            bust_a_prompt(ch->desc, TRUE);
            send_to_char("@@N\n\r\n\r", ch);
            free_string(ch->pcdata->prompt);
        }

        ch->pcdata->prompt = str_dup(buf);
        return;
    }
    else {
        strcpy(buf, argument);
        smash_tilde(buf);
    }

    free_string(ch->pcdata->prompt);
    ch->pcdata->prompt = str_dup(buf);
    send_to_char("Ok.\n\r", ch);
    return;
}

void
do_players(CHAR_DATA *ch, char *argument)
{

    send_to_char("This command is no longer needed, as 'WHO' carries all details.\n\r", ch);
    return;
}

void
do_diagnose(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    char                arg[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim;
    int                 pct;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
        send_to_char("Diagnose whom?\n\r", ch);
        return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL) {
        send_to_char("They're not here.\n\r", ch);
        return;
    }

    if (can_see(ch, victim)) {
        act("$n gives you the once-over.", ch, NULL, victim, TO_VICT);
        act("$n gives $N the once-over.", ch, NULL, victim, TO_NOTVICT);
    }
    else {
        send_to_char("They're not here.\n\r", ch);
        return;
    }

    if (victim->max_hit > 0)
        pct = (100 * victim->hit) / victim->max_hit;
    else
        pct = -1;

    strcpy(buf, PERS(victim, ch));

    if      (pct >=  100) safe_strcat(MAX_STRING_LENGTH, buf, " --  [100] 90  80  70  60  50  40  30  20  10   0\n\r");
    else if (pct >=   90) safe_strcat(MAX_STRING_LENGTH, buf, " --   100 [90] 80  70  60  50  40  30  20  10   0\n\r");
    else if (pct >=   80) safe_strcat(MAX_STRING_LENGTH, buf, " --   100  90 [80] 70  60  50  40  30  20  10   0\n\r");
    else if (pct >=   70) safe_strcat(MAX_STRING_LENGTH, buf, " --   100  90  80 [70] 60  50  40  30  20  10   0\n\r");
    else if (pct >=   60) safe_strcat(MAX_STRING_LENGTH, buf, " --   100  90  80  70 [60] 50  40  30  20  10   0\n\r");
    else if (pct >=   50) safe_strcat(MAX_STRING_LENGTH, buf, " --   100  90  80  70  60 [50] 40  30  20  10   0\n\r");
    else if (pct >=   40) safe_strcat(MAX_STRING_LENGTH, buf, " --   100  90  80  70  60  50 [40] 30  20  10   0\n\r");
    else if (pct >=   30) safe_strcat(MAX_STRING_LENGTH, buf, " --   100  90  80  70  60  50  40 [30] 20  10   0\n\r");
    else if (pct >=   20) safe_strcat(MAX_STRING_LENGTH, buf, " --   100  90  80  70  60  50  40  30 [20] 10   0\n\r");
    else if (pct >=   10) safe_strcat(MAX_STRING_LENGTH, buf, " --   100  90  80  70  60  50  40  30  20 [10]  0\n\r");
    else                  safe_strcat(MAX_STRING_LENGTH, buf, " --   100  90  80  70  60  50  40  30  20  10 [ 0]\n\r");

    buf[0] = UPPER(buf[0]);
    send_to_char(buf, ch);

    return;
}

void
do_heal(CHAR_DATA *ch, char *argument)
{
    /* This function used when a player types heal when in a room with
     * a mob with ACT_HEAL set.  Cost is based on the ch's level.
     * -- Stephen
     */

    CHAR_DATA          *mob;
    char                buf[MAX_STRING_LENGTH];
    int                 mult;    /* Multiplier used to calculate costs. */

    buf[0] = '\0';

    /* Check for mob with act->heal */
    for (mob = ch->in_room->first_person; mob; mob = mob->next_in_room) {
        if (IS_NPC(mob) && IS_SET(mob->act, ACT_HEAL))
            break;
    }

    if (mob == NULL) {
        send_to_char("You can't do that here.\n\r", ch);
        return;
    }

    mult = UMAX(10, ch->level / 2);

    if (argument[0] == '\0') {
        /* Work out costs of different spells. */
        send_to_char("Costs for spells:\n\r", ch);
        sendf(ch,    "Sanctuary:          %7d GP.\n\r", (mult * 100));
        sendf(ch,    "Heal:               %7d GP.\n\r", (mult * 90));
        sendf(ch,    "Invisibilty:        %7d GP.\n\r", (mult * 20));
        sendf(ch,    "Detect Invisibilty: %7d GP.\n\r", (mult * 10));
        sendf(ch,    "Refresh:            %7d GP.\n\r", (mult * 10));
        sendf(ch,    "Night Vision:       %7d GP.\n\r", (mult * 20));
        sendf(ch,    "Magical Dispel:     %7d GP.\n\r", (mult * 200));
        send_to_char("Power (150 Mana)      10000 GP.\n\r", ch);
        send_to_char("Type HEAL [S|H|I|D|R|N|M|P]\n\r", ch);
        send_to_char("Eg: 'HEAL H' will result in the heal spell being cast.\n\r", ch);
        send_to_char("\n\r**ALL** Spells will be cast on the buyer ONLY.\n\r", ch);
        return;
    }

    switch (UPPER(argument[0])) {
        case 'S':                /* Sanc */
            if (ch->gold < (mult * 100)) {
                send_to_char("You don't have that much gold right now...\n\r", ch);
                return;
            }
            act("$N gestures towards $n.", ch, NULL, mob, TO_NOTVICT);
            act("$N gestures towards you.", ch, NULL, mob, TO_CHAR);
            spell_sanctuary(skill_lookup("sanc"), mult, ch, ch, NULL);
            ch->gold -= (mult * 100);
            break;
        case 'P':                /* mana */
            if (ch->gold < 3000) {
                send_to_char("You don't have that much gold right now...\n\r", ch);
                return;
            }
            act("$N gestures towards $n.", ch, NULL, mob, TO_NOTVICT);
            act("$N gestures towards you.", ch, NULL, mob, TO_CHAR);
            ch->gold -= 3000;
            ch->mana = UMIN(ch->max_mana, ch->mana + 150);
            break;

        case 'H':                /* Heal */
            if (ch->gold < (mult * 90)) {
                send_to_char("You don't have that much gold right now...\n\r", ch);
                return;
            }
            act("$N gestures towards $n.", ch, NULL, mob, TO_NOTVICT);
            act("$N gestures towards you.", ch, NULL, mob, TO_CHAR);
            spell_heal(skill_lookup("heal"), mult, mob, ch, NULL);
            ch->gold -= (mult * 90);
            break;
        case 'I':                /* invis */
            if (ch->gold < (mult * 20)) {
                send_to_char("You don't have that much gold right now...\n\r", ch);
                return;
            }
            act("$N gestures towards $n.", ch, NULL, mob, TO_NOTVICT);
            act("$N gestures towards you.", ch, NULL, mob, TO_CHAR);
            spell_invis(skill_lookup("invis"), mult, mob, ch, NULL);
            ch->gold -= (mult * 20);
            break;
        case 'D':                /* detect invis */
            if (ch->gold < (mult * 10)) {
                send_to_char("You don't have that much gold right now...\n\r", ch);
                return;
            }
            act("$N gestures towards $n.", ch, NULL, mob, TO_NOTVICT);
            act("$N gestures towards you.", ch, NULL, mob, TO_CHAR);
            spell_detect_invis(skill_lookup("detect invis"), mult, mob, ch, NULL);
            ch->gold -= (mult * 10);
            break;
        case 'R':                /* refresh */
            if (ch->gold < (mult * 10)) {
                send_to_char("You don't have that much gold right now...\n\r", ch);
                return;
            }
            act("$N gestures towards $n.", ch, NULL, mob, TO_NOTVICT);
            act("$N gestures towards you.", ch, NULL, mob, TO_CHAR);
            spell_refresh(skill_lookup("refresh"), mult, mob, ch, NULL);
            ch->gold -= (mult * 10);
            break;
        case 'N':                /* Infra */
            if (ch->gold < (mult * 20)) {
                send_to_char("You don't have that much gold right now...\n\r", ch);
                return;
            }
            act("$N gestures towards $n.", ch, NULL, mob, TO_NOTVICT);
            act("$N gestures towards you.", ch, NULL, mob, TO_CHAR);
            spell_infravision(skill_lookup("infra"), mult, ch, ch, NULL);
            ch->gold -= (mult * 20);
            break;
        case 'M':                /* dispel */
            if (ch->gold < (mult * 200)) {
                send_to_char("You don't have that much gold right now...\n\r", ch);
                return;
            }
            /* No acts, as they are in spell_dispel_magic.  Doh.  Josh */
            act("$N gestures towards $n.", ch, NULL, mob, TO_NOTVICT);
            act("$N gestures towards you.", ch, NULL, mob, TO_CHAR);
            spell_dispel_magic(skill_lookup("dispel magic"), mult * 5, ch, ch, NULL);
            ch->gold -= (mult * 200);
            break;
        default:
            send_to_char("Are you sure you're reading the instructions right??\n\r", ch);
            return;
    }
    return;
}

void
do_bank(CHAR_DATA *ch, char *argument)
{
    /* Simple banking system.  Allow ch to check balance, make a 
     * deposit or withdrawl some money.
     * -- Stephen
     *
     * added support for ITEM_QUEST deposits -dave
     * added Commas to everything in here -josh, with the comma thingie dave did
     */

    CHAR_DATA          *mob;
    OBJ_DATA           *obj;
    char                buf[MAX_STRING_LENGTH];
    char                arg1[MAX_INPUT_LENGTH];
    char                arg2[MAX_INPUT_LENGTH];

    buf[0] = '\0';

    if (IS_NPC(ch)) {
        send_to_char("Banking Services are only available to players!\n\r", ch);
        return;
    }

    /* Check for mob with act->heal */
    for (mob = ch->in_room->first_person; mob; mob = mob->next_in_room) {
        if (IS_NPC(mob) && IS_SET(mob->act, ACT_BANKER))
            break;
    }

    if (mob == NULL) {
        send_to_char("You can't do that here.\n\r", ch);
        return;
    }

    if (argument[0] == '\0') {
        send_to_char("                  BANK : Options:\n\r\n\r", ch);
        send_to_char("          BANK BALANCE : Displays your balance.\n\r", ch);
        send_to_char(" BANK DEPOSIT <amount> : Deposit gold into your account.\n\r", ch);
        send_to_char(" BANK DEPOSIT   <item> : Cash in quest point items.\n\r", ch);
        send_to_char("BANK WITHDRAW <amount> : Withdrawl gold from your account.\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    /* Now work out what to do... */

    if (!str_cmp(arg1, "balance")) {
        int                 aucamt = available_gold(ch);

        if (aucamt == ch->balance) {
            sprintf(buf, "Your current balance is: @@y%s @@NGP.\n\r", number_comma(ch->balance));
        }
        else {
            sprintf(buf, "Your current balance is: @@y%s @@NGP, ", number_comma(ch->balance));
            sprintf(buf + strlen(buf), "of which @@y%s @@NGP is reserved for auctions you are currently winning.\n\r",
                number_comma(ch->balance - aucamt));
        }

        send_to_char(buf, ch);

        return;

    }

    if (!str_cmp(arg1, "deposit")) {
        int                 amount;

        if (!str_cmp("all", arg2) || is_number(arg2)) {
            if (!str_cmp("all", arg2))
                amount = ch->gold;
            else
                amount = atoi(arg2);
            if (amount < 0)
                return;
            if (amount > ch->gold) {
                sendf(ch, "How can you deposit @@y%s @@NGP ", number_comma(amount));
                sendf(ch, "when you only have @@y%s@@N?\n\r", number_comma(ch->gold));
                return;
            }

            ch->gold -= amount;
            ch->balance += amount;
            sendf(ch, "You deposit @@y%s @@NGP.  ", number_comma(amount));
            sendf(ch, "Your new balance is @@y%s @@NGP. \n\r", number_comma(ch->balance));
            do_save(ch, "");
            return;
        }
        else {
            if ((obj = get_obj_carry(ch, arg2)) == NULL) {
                send_to_char("You do not have that item.\n\r", ch);
                return;
            }

            if (obj->item_type != ITEM_QUEST) {
                send_to_char("That's not a quest point item.\n\r", ch);
                return;
            }

            if (obj->value[0] < 1) {
                send_to_char("Item isn't worth anything.\n\r", ch);
                return;
            }

            if (obj->value[3] != 0 && get_pseudo_level(ch) > obj->value[3]) {
                send_to_char("Your level is too high to deposit this item.\n\r", ch);
                return;
            }

            sprintf(buf, "$N @@Ngives you @@y%d @@a%s@@N for $p@@N.", obj->value[0], (obj->value[0] == 1) ? "QP" : "QPS");
            act(buf, ch, obj, mob, TO_CHAR);
            sprintf(buf, "%s receives %dqps for %s.", ch->name, obj->value[0], obj->short_descr);
            log_string(buf);

            ch->quest_points += obj->value[0];
            extract_obj(obj);
            do_save(ch, "");
            return;
        }

    }

    if (!str_cmp(arg1, "withdraw")) {
        int                 amount;
        int                 aucamt = 0;

        if (is_number(arg2) || !str_cmp("all", arg2)) {
            if (!str_cmp("all", arg2))
                amount = ch->balance;
            else
                amount = atoi(arg2);

            if (amount < 0)
                return;

            if (amount > ch->balance) {
                sendf(ch, "How can you withdraw @@y%s @@NGP when your balance is @@y%d@@N?\n\r", number_comma(amount), ch->balance);
                return;
            }

            aucamt = available_gold(ch);

            if (amount > aucamt) {
                sendf(ch, "Withdrawing that much gold would not leave you with enough for the auctions you are currently winning. Try withdrawing %s GP instead.\n\r", number_comma(aucamt));
                return;
            }

            if (ch->gold + amount > 100000000) {
                send_to_char("You can't carry that much gold.\n\r", ch);
                return;
            }
            ch->balance -= amount;
            ch->gold += amount;
            sprintf(buf, "You withdraw @@y%s @@NGP.", number_comma(amount));
            sprintf(buf + strlen(buf), "  Your new balance is @@y%s @@NGP.\n\r", number_comma(ch->balance));
            send_to_char(buf, ch);
            do_save(ch, "");
            return;
        }
    }

    send_to_char("That option not recognised!\n\r", ch);
    return;
}

void
do_gain(CHAR_DATA *ch, char *argument)
{
    /* Allow ch to gain a level in a chosen class.     
     * Only can be done at prac/train mob. -S-
     * Now handles remort chars
     */

    CHAR_DATA          *mob;
    char                buf[MAX_STRING_LENGTH];
    char                arg[MIL];
    char                *msg = NULL;
    long_int            cost = 0;
    long_int            custxp = 0;
    int                 cnt;
    int                 subpop;
    bool                any;
    int                 c;        /* The class to gain in */
    int                 numclasses;    /* Current number of classes person has */
    int                 a;        /* Looping var */
    bool                remort = FALSE;
    bool                adept = FALSE;
    bool                avatar = FALSE;
    int                 lvl[5] = {0, 0, 0, 0, 0};
    int                 lvl2[5] = {0, 0, 0, 0, 0};
    int                 levels = 1;
    bool                max = FALSE;
    sh_int              morts_at_seventy = 0;
    sh_int              remorts_at_seventy = 0;
    sh_int              morts_at_eighty = 0;
    sh_int              remorts_at_eighty = 0;
    sh_int              num_remorts = 0;
    bool                allow_remort = FALSE;
    bool                allow_adept = FALSE;
    bool                allow_avatar = FALSE;

    buf[0] = '\0';

    if (IS_NPC(ch)) {
        send_to_char("Hahaha, not for NPCs.\n\r", ch);
        return;
    }

    lvl[0] = ch->lvl[0] < 1 ? 0 : ch->lvl[0];
    lvl[1] = ch->lvl[1] < 1 ? 0 : ch->lvl[1];
    lvl[2] = ch->lvl[2] < 1 ? 0 : ch->lvl[2];
    lvl[3] = ch->lvl[3] < 1 ? 0 : ch->lvl[3];
    lvl[4] = ch->lvl[4] < 1 ? 0 : ch->lvl[4];
    lvl2[0] = ch->lvl2[0] < 1 ? 0 : ch->lvl2[0];
    lvl2[1] = ch->lvl2[1] < 1 ? 0 : ch->lvl2[1];
    lvl2[2] = ch->lvl2[2] < 1 ? 0 : ch->lvl2[2];
    lvl2[3] = ch->lvl2[3] < 1 ? 0 : ch->lvl2[3];
    lvl2[4] = ch->lvl2[4] < 1 ? 0 : ch->lvl2[4];

    argument = one_argument(argument, arg);

    /* Check for mob with act->pac/train */
    for (mob = ch->in_room->first_person; mob; mob = mob->next_in_room) {
        if (IS_NPC(mob) && (IS_SET(mob->act, ACT_TRAIN) || IS_SET(mob->act, ACT_PRACTICE)))
            break;
    }

    if (mob == NULL) {
        send_to_char("You can't do that here.\n\r", ch);
        return;
    }
    for (cnt = 0; cnt < MAX_CLASS; cnt++) {
        if (ch->lvl[cnt] >= 70)
            morts_at_seventy++;
        if (ch->lvl[cnt] == 80)
            morts_at_eighty++;
        if (ch->lvl2[cnt] >= 70)
            remorts_at_seventy++;
        if (ch->lvl2[cnt] == 80)
            remorts_at_eighty++;
        if (ch->lvl2[cnt] > -1)
            num_remorts++;
    }
    /* first case.. remort  */
    if (((morts_at_seventy >= 2)
            && (is_remort(ch) == FALSE))
        || ((morts_at_eighty == 5)
            && (remorts_at_seventy == 1)
            && (num_remorts == 1))) {
        allow_remort = TRUE;
    }

    /* second case..can adept */

    if ((morts_at_eighty == 5)
        && (remorts_at_eighty == 2)
        && (ch->adept_level < 1)) {
        allow_adept = TRUE;
    }

    if (ch->adept_level == 20 && ch->pcdata->avatar == FALSE)
        allow_avatar = TRUE;

    if (arg[0] == '\0') {

        /* Display details... */
        send_to_char("You can gain levels in:\n\r", ch);
        any = FALSE;
        numclasses = 0;
        for (a = 0; a < MAX_CLASS; a++)
            if (ch->lvl[a] >= 0)
                numclasses++;

        for (cnt = 0; cnt < MAX_CLASS; cnt++)
            if ((ch->lvl[cnt] != -1 || numclasses < race_table[ch->race].classes)
                && ch->lvl[cnt] < (LEVEL_HERO - 1)) {
                any = TRUE;
                cost = exp_to_level(ch, cnt, (ch->pcdata)->index[cnt]);

                sendf(ch, "%s : %d Exp.\n\r", class_table[cnt].who_name, cost);
            }

        for (cnt = 0; cnt < MAX_CLASS; cnt++)
            if (ch->lvl2[cnt] != -1 && ch->lvl2[cnt] < (LEVEL_HERO - 1)) {
                any = TRUE;
                cost = exp_to_level(ch, cnt, 5);    /* 5 means remort */
                sendf(ch, "%s : %d Exp.\n\r", remort_table[cnt].who_name, cost);
            }

        if ((ch->adept_level > 0) && (ch->adept_level < 20)) {
            any = TRUE;
            cost = exp_to_level_adept(ch);
            sendf(ch, "@@WAdept@@N: %d Exp.\n\r", cost);
        }

        if (allow_remort) {
            any = TRUE;
            send_to_char("You can @@mREMORT@@N!!! Type gain <first three letters of the class> you want.\n\r", ch);
        }

        if (allow_adept) {
            any = TRUE;
            send_to_char("You can @@WADEPT@@N!!! Type gain adept!!\n\r", ch);
        }

        if (allow_avatar) {
            any = TRUE;
            send_to_char("You can become an @@rAVATAR@@N!!! Type gain avatar!!\n\r", ch);
        }

        if (!any)
            send_to_char("None.\n\r", ch);

        return;
    }

    /* If an argument supplied, make sure it's valid :P */

    any = FALSE;
    c = -1;
    for (cnt = 0; cnt < MAX_CLASS; cnt++)
        if (!str_cmp(class_table[cnt].who_name, arg)) {
            any = TRUE;
            c = cnt;
        }

    for (cnt = 0; cnt < MAX_CLASS; cnt++)
        if ((!str_cmp(remort_table[cnt].who_name, arg))
            && ((ch->lvl2[cnt] > 0) || (allow_remort))) {
            any = TRUE;
            remort = TRUE;
            c = cnt;
        }

    if (!str_prefix("ADEPT", arg)) {
        if ((ch->adept_level < 0) && !allow_adept)
            return;

        any = TRUE;
        adept = TRUE;
    }

    if (!str_prefix("AVATAR", arg)) {
        if (ch->adept_level < 20 || ch->pcdata->avatar == TRUE)
            return;

        any = TRUE;
        avatar = TRUE;
    }

    if (!any) {
        send_to_char("That's either not a class, or you are not currently allowed to gain in that class.\n\r", ch);
        return;
    }

    if (argument[0] != '\0') {
        if (!str_cmp("max", argument)) {
            levels = -1;
            max = TRUE;
        }
        else if (is_number(argument)) {
            levels = atoi(argument);

            if (levels < 1 || levels > 80) {
                send_to_char("Level parameter must be between 1 and 80.\n\r", ch);
                return;
            }
        }
    }
    /* Ok, so now class should be valid.  Check if enough exp */
    if (remort) {
        if (levels == -1)
            levels = 80 - (ch->lvl2[c] > 0 ? ch->lvl2[c] : 0);
        else if (ch->lvl2[c] + levels > 80)
            levels = 80 - (ch->lvl2[c] > 0 ? ch->lvl2[c] : 0);

        for (cost = 0, cnt = 0; cnt < levels; cnt++) {
            custxp = custom_exp_to_level(lvl, lvl2, c, 5);

            if (max && cost + custxp > ch->exp) {
                levels = cnt;

                if (levels == 0) {
                    levels = 1;
                    cost = custxp;
                }

                break;
            }

            cost += custxp;
            lvl2[c]++;
        }
    }
    else if (adept) {
        if (ch->adept_level < 1)
            cost = 0;
        else
            cost = exp_to_level_adept(ch);
    }
    else if (avatar) {
        cost = 0;
    }
    else {
        if (levels == -1)
            levels = 80 - (ch->lvl[c] > 0 ? ch->lvl[c] : 0);
        else if (ch->lvl[c] + levels > 80)
            levels = 80 - (ch->lvl[c] > 0 ? ch->lvl[c] : 0);

        for (cost = 0, cnt = 0; cnt < levels; cnt++) {
            custxp = custom_exp_to_level(lvl, lvl2, c, ch->pcdata->index[c]);

            if (max && cost + custxp > ch->exp) {
                levels = cnt;

                if (levels == 0) {
                    levels = 1;
                    cost = custxp;
                }

                break;
            }

            cost += custxp;
            lvl[c]++;
        }
    }

    if (levels == 0) {
        send_to_char("You cannot gain further in this class.\n\r", ch);
        return;
    }

    if (ch->exp < cost) {
        sendf(ch, "Cost is %d Exp.  You only have %d (%d short).\n\r", cost, ch->exp, (cost - ch->exp));
        return;
    }

    if ((adept) && (ch->adept_level < 20)) {
        c = ADVANCE_ADEPT;
        send_to_char("@@WYou have reached another step on the stairway to Wisdom!!!@@N\n\r", ch);
        ch->exp -= cost;
        advance_level(ch, c, TRUE, FALSE, 1);
        ch->adept_level = UMAX(1, ch->adept_level + 1);
        update_cinfo(ch, FALSE);

        free_string(ch->pcdata->who_name);
        ch->pcdata->who_name = str_dup(get_adept_name(ch));

        if (ch->adept_level == 20) {
            RULER_DATA *ruler;
            char       *rank;

            if      (ch->login_sex == SEX_NEUTRAL)
                rank = "@@WMonitor@@N";
            else if (ch->login_sex == SEX_MALE)
                rank = "@@WLord@@N";
            else
                rank = "@@WLady@@N";

            GET_FREE(ruler, ruler_free);
            ruler->name      = str_dup(capitalize(ch->name));
            ruler->whoname   = str_dup(ch->pcdata->who_name);
            ruler->rank      = str_dup(rank);
            ruler->clan      = ch->pcdata->clan;
            ruler->realmtime = current_time;

            LINK(ruler, first_ruler, last_ruler, next, prev);
            save_rulers();
        }

        gainf("%s @@N@@Wadvances in the way of the Adept!!", ch->short_descr);

        if (ch->adept_level == 1)
            ch->exp /= 1000;

        do_save(ch, "");
        return;
    }
    else if (adept) {
        send_to_char("@@aYou peer down upon all the hapless mortals, knowing that you have reached the final step upon the stairway of Wisdom.@@N\n\r", ch);
        return;
    }
    else if (avatar) {
        send_to_char("You become an avatar.\n\r", ch);
        gainf("%s joins the ranks of the Avatars.", ch->short_descr);
        ch->pcdata->avatar = TRUE;
        save_char_obj(ch);
        return;
    }

    /* Don't bother adapting for remort... dropped num classes yrs ago! */
    if (ch->lvl[c] < 0 && !remort)
        ch->lvl[c] = 0;
    else if (ch->lvl2[c] < 0 && remort)
        ch->lvl2[c] = 0;

    /* Check to see if able to reach new level */
    if (remort)
        sendf(ch, "You gain %d %s level%s!\n\r", levels, remort_table[c].class_name, levels == 1 ? "" : "s");
    else
        sendf(ch, "You gain %d %s level%s!\n\r", levels, class_table[c].class_name, levels == 1 ? "" : "s");

    /* Use info channel to inform of level gained! */

    if (levels == 1)
        msg = "@@N@@g%s@@N@@g advances in the way of the %s.";
    else if (levels <= 30)
        msg = "@@N@@g%s@@N@@g just gained @@W%d@@g %s levels! @@aL@@co@@dv@@ge@@Wl@@ay@@c!@@d!@@N";
    else if (levels <= 50)
        msg = "@@N@@g%s@@N@@g just gained @@W%d@@g %s levels! @@cR@@ge@@Wm@@aa@@cr@@dk@@ga@@Wb@@al@@ce@@d!@@g!@@N";
    else if (levels <= 60)
        msg = "@@N@@g%s@@N@@g just gained @@W%d@@g %s levels! @@WM@@aa@@cr@@dv@@ge@@Wl@@ao@@cu@@ds@@g!@@W!@@N";
    else if (levels <= 70)
        msg = "@@N@@g%s@@N@@g just gained @@W%d@@g %s levels! @@gO@@Wu@@at@@cs@@dt@@ga@@Wn@@ad@@ci@@dn@@gg@@W!@@a!@@N";
    else if (levels <= 79)
        msg = "@@N@@g%s@@N@@g just gained @@W%d@@g %s levels! @@dI@@gn@@Wc@@ar@@ce@@dd@@gi@@Wb@@al@@ce@@d!@@g!@@N";
    else if (levels == 80)
        msg = "@@aW@@ch@@do@@ga@@W!@@a!@@g! @@N@@g%s@@N@@g just gained all @@W%d@@g %s levels! @@aA@@cb@@ds@@go@@Wl@@au@@ct@@de@@gl@@Wy @@Rp@@ehe@@ynome@@ena@@Rl@@e!!@@N";

    if (levels == 1)
        gainf(msg, ch->short_descr, remort ? remort_table[c].class_name : class_table[c].class_name);
    else
        gainf(msg, ch->short_descr, levels, remort ? remort_table[c].class_name : class_table[c].class_name);

    ch->exp -= cost;

    advance_level(ch, c, TRUE, remort, levels);

    if (remort)
        ch->lvl2[c] = URANGE(1, ch->lvl2[c] + levels, 80);
    else
        ch->lvl[c] = UMIN(80, ch->lvl[c] + levels);        /* Incr. the right class */

    /* Maintain ch->level as max level of the lot */
    for (subpop = 0; subpop < MAX_CLASS; subpop++) {
        if (ch->lvl[subpop] > ch->level)
            ch->level = ch->lvl[subpop];
        if (ch->lvl2[subpop] > ch->level)
            ch->level = ch->lvl2[subpop];
    }
    do_save(ch, "");

    if (levels == 1)
        WAIT_STATE(ch, PULSE_PER_SECOND * 2);

    return;
}

void
do_alias(CHAR_DATA *ch, char *argument)
{
    /* Handle aliases - setting and clearing, as well as listing. */

    int                 cnt;
    int                 alias_no;
    char                buf[MAX_STRING_LENGTH];
    char                arg1[MAX_STRING_LENGTH];
    char                arg2[MAX_STRING_LENGTH];
    char                arg3[MAX_STRING_LENGTH];
    char                argbuf[MSL];

    buf[0] = '\0';

    if (IS_NPC(ch)) {
        send_to_char("Not a chance!\n\r", ch);
        return;
    }
    smash_tilde(argument);
    if (argument[0] == '\0') {
        send_to_char("Defined Aliases:\n\r", ch);

        for (cnt = 0; cnt < MAX_ALIASES; cnt++)
            sendf(ch, "(%2d) [Name:] %12s  [Aliases:] %s\n\r", cnt, ch->pcdata->alias_name[cnt], ch->pcdata->alias[cnt]);

        send_to_char("\n\rTo Set an Alias:\n\r", ch);
        send_to_char("ALIAS <num> <name> <alias>\n\r", ch);
        send_to_char("-enter 'CLEAR' as name to clear an alias.\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg1);    /* Number */
    argument = one_argument(argument, arg2);    /* name   */
    strcpy(arg3, argument);        /* alias  */
    one_argument(arg3, argbuf);
    if (arg3[0] == '\0' && str_cmp(arg2, "clear")) {
        send_to_char("\n\rTo Set an Alias:\n\r", ch);
        send_to_char("ALIAS <num> <name> <alias>\n\r", ch);
        send_to_char("-enter 'CLEAR' as name to clear an alias.\n\r", ch);
        return;
    }

    if (!is_number(arg1)) {
        send_to_char("First argument must be an integer.\n\r", ch);
        return;
    }
    else
        alias_no = atoi(arg1);

    if (alias_no < 0 || alias_no > (MAX_ALIASES - 1)) {
        sendf(ch, "Valid alias numbers are 0 to %d.\n\r", MAX_ALIASES);
        return;
    }

    if (!str_cmp("clear", arg2)) {
        free_string(ch->pcdata->alias_name[alias_no]);
        free_string(ch->pcdata->alias[alias_no]);

        ch->pcdata->alias_name[alias_no] = str_dup("<none>");
        ch->pcdata->alias[alias_no] = str_dup("<none>");

        /* Clear the alias (enter <none> for name and alias */
        return;
    }

    if (str_cmp("a", argbuf) && !str_prefix(argbuf, "alias")) {
        send_to_char("Aliasing an alias command is a Bad Thing(tm), and has been logged due to possible abuse.\n\r", ch);
        sprintf(buf, "%s used alias bug with parameters: %s\n", ch->name, arg3);
        log_string(buf);
        return;
    }

    /* Hopefully, now just set the (new) alias... */

    free_string(ch->pcdata->alias_name[alias_no]);
    free_string(ch->pcdata->alias[alias_no]);

    ch->pcdata->alias_name[alias_no] = str_dup(arg2);
    ch->pcdata->alias[alias_no] = str_dup(arg3);
    return;
}

void
do_colour(CHAR_DATA *ch, char *argument)
{
    /* Allow users to set which colour they get certain texts in. -S- */

    char                buf[MAX_STRING_LENGTH];
    char                arg1[MAX_STRING_LENGTH];
    char                arg2[MAX_STRING_LENGTH];
    int                 col;
    int                 cnt;
    int                 ansi_number;
    int                 colour_number;

    buf[0] = '\0';

    if (IS_NPC(ch))
        return;

    /* First check to see if there is NO argument.  If so, display
     * the current settings for players colour. 
     * In this context, 'colour' means the type of text, eg "say" or "shout"
     *                  'ansi'   means the actual colour
     *                  ->Confusing, right?
     * -S-
     */

    if (IS_NPC(ch))
        return;
    col = 0;

    if (argument[0] == '\0') {
        send_to_char("@@yPresent Colour Configuration:@@g\n\r\n\r", ch);

        for (cnt = 0; cnt < MAX_COLOUR; cnt++) {
            if (!str_cmp(colour_table[cnt].name, "unused"))
                continue;

            sendf(ch, "@@W%8s: %s%-12s@@N   ", colour_table[cnt].name, ansi_table[ch->pcdata->colour[cnt]].value, ansi_table[ch->pcdata->colour[cnt]].name);

            if (++col % 3 == 0)
                send_to_char("\n\r", ch);
        }
        if (col % 3 != 0)
            send_to_char("\n\r", ch);

        send_to_char("\n\r@@yAvailable Colours:@@g\n\r", ch);

        col = 0;

        for (cnt = 0; cnt < MAX_ANSI; cnt++) {
            sendf(ch, "%s%-12s@@N  ", IS_SET(ch->act, PLR_COLOUR) ? ansi_table[cnt].value : "", ansi_table[cnt].name);
            if (++col % 5 == 0)
                send_to_char("\n\r", ch);
        }
        if (col % 5 != 0)
            send_to_char("\n\r", ch);

        send_to_char("\n\rUSAGE: COLOUR <name> <colour>\n\r", ch);
        send_to_char("Eg:     COLOUR say red\n\r", ch);
        send_to_char("OR: COLOUR highlighted/dimmed <colour> for emphasized or dimmed text.\n\r", ch);

        send_to_char("NOTE: The 'stats' info covers who, score, etc.\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg1);    /* The name, eg 'say'   */
    argument = one_argument(argument, arg2);    /* The colour, eg 'red' */

    if (arg2[0] == '\0') {
        do_colour(ch, "");        /* Generate message */
        return;
    }
    if (!str_prefix(arg1, "highlighted"))
        colour_number = -2;
    else if (!str_prefix(arg1, "dimmed"))
        colour_number = -3;
    else {
        /* Check to see if the name is valid */
        colour_number = -1;
        for (cnt = 0; cnt < MAX_COLOUR; cnt++) {
            if (!str_cmp("unused", colour_table[cnt].name))
                continue;

            if (!str_cmp(arg1, colour_table[cnt].name))
                colour_number = colour_table[cnt].index;
        }
    }

    if (colour_number == -1) {
        /* list possible choices */
        /* do_colour( ch, "help" ); */
        return;
    }

    /* colour (the name) is ok.  Now find the ansi (the colour) */
    ansi_number = -1;
    for (cnt = 0; cnt < MAX_ANSI; cnt++)
        if (!str_cmp(arg2, ansi_table[cnt].name))
            ansi_number = ansi_table[cnt].index;

    if (ansi_number == -1) {
        /* list possible choice */
        /* do_colour( ch, "help" ); */
        return;
    }

    /* Ok now, we have colour_number, which is the index to pcdata->colour[]
     * so we need to set the value of it to the colour.  
     * -S-
     */

    if (colour_number == -2) {
        ch->pcdata->hicol = ansi_table[ansi_number].letter;
        return;
    }
    else if (colour_number == -3) {
        ch->pcdata->dimcol = ansi_table[ansi_number].letter;
        return;
    }

    ch->pcdata->colour[colour_number] = ansi_number;
    send_to_char("OK.\n\r", ch);
    return;
}

/* A simple, return the char sequence, function -S- */

char               *
colour_string(CHAR_DATA *ch, char *argument)
{
    int                 cnt;
    int                 num;

    /* if we don't want to send the string, return "" */
    /* argument should be the string to find, eg "say" */

    if (ch == NULL || !ch->pcdata || IS_NPC(ch) || argument[0] == '\0')
        return ("");

    if (!IS_SET(ch->act, PLR_COLOUR))
        return ("");

    if (!str_cmp(argument, "normal"))
        return ("\x1b[0m");

    /* By here, ch is a PC and wants colour */

    num = -1;
    for (cnt = 0; cnt < MAX_COLOUR; cnt++)
        if (!str_cmp(argument, colour_table[cnt].name))
            num = cnt;

    if (num == -1)
        /* bug report? */
        return ("");

    return (ansi_table[ch->pcdata->colour[num]].value);

}
void
do_worth(CHAR_DATA *ch, char *argument)
{
    /* Show details regarding cost to level each class, etc 
    Themeized the worth command -Josh
    I wonder if this is going to work:)*/

    bool                any;
    char                buf[MAX_STRING_LENGTH];
    char                rbuf[MSL];
    int                 numclasses;
    int                 a;
    long_int            cost;
    int                 cnt;
    int                 sn;
    bool                found = FALSE;
    CHAR_DATA          *vch, *fch;

    if (IS_NPC(ch)) {
        send_to_char("Only for PCs.\n\r", ch);
        return;
    }

    vch = ch;

    if (IS_IMMORTAL(ch) && *argument && (fch = get_char_world(ch, argument)) && !IS_NPC(fch)) {
        vch = fch;
    }

    send_to_char("@@d.-------------------------------------------@@g=( @@aWorth @@g)=@@d-.\n\r", ch);
    sprintf(buf, "@@gCosts in Exp for you to level:");
    sendf(ch, "@@d| %s @@d|\n\r", center_text(buf, 53));
    send_to_char("@@d|-------------------------------------------------------|\n\r", ch);
    send_to_char("@@d| @@aClass name          @@d|        @@eCost  @@d|      @@gDifference  @@d|\n\r", ch);
    send_to_char("@@d|-------------------------------------------------------|\n\r", ch);


    if (vch->adept_level == 20 && !vch->pcdata->avatar)
    {
        sprintf(buf, "@@mPsst. You need to gain avatar!");
        sendf(ch, "@@d| %s @@d|\n\r", center_text(buf, 53));
        send_to_char("@@d'-------------------------------------------------------@@N'\n\r", ch);
        return;
    }

    if (vch->pcdata->avatar) {
        for (sn = 0; sn < MAX_SKILL; sn++) {
            if (skill_table[sn].flag1 == AVATAR && skill_table[sn].flag2 == NORM) {
                char skill[MIL];

                rbuf[0] = 0; buf[0] = 0;

                if (vch->pcdata->learned[sn] >= best_learnt(ch, sn))
                    continue;

                strncpy(skill, skill_table[sn].name, sizeof(skill) - 1);
                skill[0] = UPPER(skill[0]);

                cost = avatar_cost(vch->pcdata->learned[sn] > 0 ? vch->pcdata->learned[sn] : 0) * 1000;
                sprintf(buf, "@@g%s @@W%d%%", skill, vch->pcdata->learned[sn]);
                sendf(ch, "@@d| %s ", my_left(buf, rbuf, 19));
                sprintf(buf, "@@e%9s ", number_comma(cost));
                sendf(ch, "@@d| %9s ", my_right(buf, rbuf, 12));
                sprintf(buf, "%11s @@d", number_comma(UMAX(0, cost - vch->exp)));
                sendf(ch, "@@d| %11s @@d|\n\r", my_right(buf, rbuf, 16));
                found = TRUE;
            }
        }

        if (!found) {
            sprintf(buf, "@@mYou've mastered every avatar skill/spell! WOW.");
            sendf(ch, "@@d| %s @@d|\n\r", center_text(buf, 53));
        }

        send_to_char("@@d'-------------------------------------------------------'@@N\n\r", ch);
        return;
    }

    if (vch->adept_level > 0)
    {

        cost = exp_to_level_adept(vch);
        sprintf(buf, "%s", get_adept_name2(vch));
        sendf(ch, "@@d| %s ", my_left(buf, rbuf, 19));
        sprintf(buf, "@@e%9s ", number_comma(cost));
        sendf(ch, "@@d| %9s ", my_right(buf, rbuf, 12));
        sprintf(buf, "%11s @@d", number_comma(UMAX(0, cost - vch->exp)));
        sendf(ch, "@@d| %11s @@d|\n\r", my_right(buf, rbuf, 16));
        send_to_char("@@d'-------------------------------------------------------'@@N\n\r", ch);
        return;
    }

    any = FALSE;
    numclasses = 0;
    for (a = 0; a < MAX_CLASS; a++)
        if (vch->lvl[a] >= 0)
            numclasses++;

    for (cnt = 0; cnt < MAX_CLASS; cnt++)
        if ((vch->lvl[cnt] != -1 || numclasses < race_table[vch->race].classes)
            && vch->lvl[cnt] < LEVEL_HERO - 1) 
        {
            any = TRUE;
            cost = exp_to_level(vch, cnt, (vch->pcdata)->index[cnt]);

            sprintf(buf, "@@a%s", class_table[cnt].class_name);
            sendf(ch, "@@d| @@a%s ", my_left(buf, rbuf, 19));
            sprintf(buf, "@@e%s ", number_comma(cost));
            sendf(ch, "@@d| %s ", my_right(buf, rbuf, 12));
            sprintf(buf, " @@g%s ", number_comma(UMAX(0, cost - vch->exp)));
            sendf(ch, "@@d| @@g%s @@d|\n\r", my_right(buf, rbuf, 16));
        }

    /* Check for remort classes */
    for (cnt = 0; cnt < MAX_CLASS; cnt++)
        if (vch->lvl2[cnt] != -1 && vch->lvl2[cnt] < LEVEL_HERO - 1) 
        {
            any = TRUE;
            cost = exp_to_level(vch, cnt, 5);    /* Pass 5 for remort */
            sprintf(buf, "@@c%s", remort_table[cnt].class_name);
            sendf(ch, "@@d| @@c%s ", my_left(buf, rbuf, 19));
            sprintf(buf, "@@e%s ", number_comma(cost));
            sendf(ch, "@@d| %s ", my_right(buf, rbuf, 12));
            sprintf(buf, " @@g%s ", number_comma(UMAX(0, cost - vch->exp)));
            sendf(ch, "@@d| @@g%s @@d|\n\r", my_right(buf, rbuf, 16));
        }

    if (any)
    {
        send_to_char("@@d|-------------------------------------------------------|\n\r", ch);
        send_to_char("@@d|  @@gSee also: @@cxpcalc                                     @@d|\n\r", ch);
        send_to_char("@@d'-------------------------------------------------------'@@N\n\r", ch);
    }
    else
    {
        send_to_char("@@d|  @@gNone to show!!                                       @@d|\n\r", ch);
        send_to_char("@@d'-------------------------------------------------------'@@N\n\r", ch);
    }
    return;
}

#if 0
void
do_whois(CHAR_DATA *ch, char *argument)
{
    /* Show ch some details about the 'victim'
     * Make sure ch can see victim!
     * -S-
     */

    CHAR_DATA          *victim;
    char                buf[MAX_STRING_LENGTH];

    if ((victim = get_char_world(ch, argument)) == NULL) {
        send_to_char("No such player found.\n\r", ch);
        return;
    }

    if (!can_see(ch, victim)) {
        send_to_char("No such player found.\n\r", ch);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("Not on NPCs.\n\r", ch);
        return;
    }

    /* Ok, so now show the details! */
    sprintf(buf, "@@y-=-=-=-=-=-=-=-=-=-=-@@R%9s@@y -=-=-=-=-=-=-=-=-=-=-@@N\n\r", victim->name);
    if (IS_IMMORTAL(victim)) {
        sprintf(buf + strlen(buf), " [ %3s ]\n\r", victim->pcdata->who_name);
    }
    else if (victim->adept_level > 0) {
        sprintf(buf + strlen(buf), " %s \n\r", get_adept_name(victim));
    }
    else {
        sprintf(buf + strlen(buf), "@@pLevels@@N: [ @@mMag@@N:%2d  @@aCle@@N:%2d  @@dThi@@N:%2d  @@RWar@@N:%2d  @@gPsi@@N:%2d ]\n\r",
            victim->lvl[0] > 0 ? victim->lvl[0] : 0,
            victim->lvl[1] > 0 ? victim->lvl[1] : 0,
            victim->lvl[2] > 0 ? victim->lvl[2] : 0, victim->lvl[3] > 0 ? victim->lvl[3] : 0, victim->lvl[4] > 0 ? victim->lvl[4] : 0);

        if (is_remort(victim))

            sprintf(buf + strlen(buf), "Levels: [ @@ySor@@N:%2d  @@eAss@@N:%2d  @@gKni@@N:%2d  @@dNec@@N:%2d  @@mMon@@N:%2d ]\n\r",
                victim->lvl2[0] > 0 ? victim->lvl2[0] : 0,
                victim->lvl2[1] > 0 ? victim->lvl2[1] : 0,
                victim->lvl2[2] > 0 ? victim->lvl2[2] : 0, victim->lvl2[3] > 0 ? victim->lvl2[3] : 0, victim->lvl2[4] > 0 ? victim->lvl2[4] : 0);
    }
    sprintf(buf + strlen(buf), "@@cSex@@N: %s.  @@rRace@@N: %s.  @@bClan@@N: %s.\n\r",
        (victim->sex == SEX_MALE) ? "Male" :
        (victim->sex == SEX_FEMALE) ? "Female" : "None", race_table[victim->race].race_name, clan_table[victim->pcdata->clan].clan_name);

    if (IS_SET(victim->pcdata->pflags, PFLAG_PKOK))
        sprintf(buf + strlen(buf), "Player is @@ePKOK@@N\n\r");
    sprintf(buf + strlen(buf), "@@ePlayers Killed@@N: %d.  @@RTimes killed by players@@N: %d.\n\r", victim->pcdata->pkills, victim->pcdata->pkilled);

    sprintf(buf + strlen(buf), "@@BMobs killed@@N: %d.  @@lTimes killed by mobs@@N: %d.\n\r", victim->pcdata->mkills, victim->pcdata->mkilled);

    if (IS_IMMORTAL(victim)) {
        sprintf(buf + strlen(buf), "%s @@wis an Immortal@@N.\r\n", victim->name);
    }

    /* show wizinvis level */
    if (IS_IMMORTAL(ch) && IS_SET(victim->act, PLR_WIZINVIS)) {
        sprintf(buf + strlen(buf), "%s @@wis WIZI at level: %d.\r\n", victim->name, victim->invis);
    }

    /* Description here, or email address? */

    sprintf(buf + strlen(buf), "@@y-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-@@N\n\r");
    send_to_char(buf, ch);
    return;
}
#endif

void
do_whois(CHAR_DATA *ch, char *argument)
{
    /* Show ch some details about the 'victim'
     * Make sure ch can see victim!
     * -S-
     */

    CHAR_DATA          *victim;
    char                buf[MSL];
    char                buf2[MSL];
    char                flags[17][512];
    int                 lastflag = 16;
    int                 cnt = 0;
    int                 space = 0;
    int                 other = 0;
    int                 other2 = 0;
    bool                hasflags = FALSE;

    if ((victim = get_char_world(ch, argument)) == NULL) {
        send_to_char("No such player found.\n\r", ch);
        return;
    }

    if (!can_see(ch, victim)) {
        send_to_char("No such player found.\n\r", ch);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("Not on NPCs.\n\r", ch);
        return;
    }

    strcpy(flags[cnt], !victim->desc ? "@@yLINKDEAD" : "");
    cnt++;
    strcpy(flags[cnt], victim->timer > 5 ? "@@WIdle (%d ticks)" : "");
    cnt++;
    if (IS_SET(victim->config, PLR_ANSWERING)) {
        strcpy(flags[cnt], (IS_SET(victim->pcdata->pflags, PFLAG_AFK)
                || IS_SET(victim->pcdata->pflags, PFLAG_XAFK))
            ? "@@rAFK Machine" : "");
        cnt++;
    }
    else {
        strcpy(flags[cnt], IS_SET(victim->pcdata->pflags, PFLAG_AFK)
            ? "@@rAFK" : "");
        cnt++;
        strcpy(flags[cnt], IS_SET(victim->pcdata->pflags, PFLAG_XAFK)
            ? "@@rXAFK" : "");
        cnt++;
    }
    strcpy(flags[cnt], victim->position == POS_WRITING ? "@@WWriting" : "");
    cnt++;
    strcpy(flags[cnt], IS_SET(victim->act, PLR_KILLER) || IS_SET(victim->act, PLR_THIEF)
        ? "@@yWanted" : "");
    cnt++;
    strcpy(flags[cnt], IS_SET(victim->pcdata->pflags, PFLAG_PKOK)
        ? "@@WPKOK" : "");
    cnt++;
    strcpy(flags[cnt], IS_SET(victim->pcdata->pflags, PFLAG_CLAN_ARMOURER)
        ? "@@WClan Armourer" : "");
    cnt++;
    strcpy(flags[cnt], IS_SET(victim->pcdata->pflags, PFLAG_CLAN_DIPLOMAT)
        ? "@@WClan Diplomat" : "");
    cnt++;
    strcpy(flags[cnt], IS_SET(victim->pcdata->pflags, PFLAG_CLAN_TREASURER)
        ? "@@WClan Treasurer" : "");
    cnt++;
    strcpy(flags[cnt], IS_SET(victim->pcdata->pflags, PFLAG_CLAN_2LEADER)
        ? "@@WClan Main Leader" : "");
    cnt++;
    strcpy(flags[cnt], IS_SET(victim->pcdata->pflags, PFLAG_CLAN_LEADER)
        ? "@@WClan Leader" : "");
    cnt++;
    strcpy(flags[cnt], IS_SET(victim->pcdata->pflags, PFLAG_CLAN_BOSS)
        ? "@@WClan Boss" : "");
    cnt++;
    strcpy(flags[cnt], IS_SET(victim->pcdata->pflags, PFLAG_CLAN_DESERTER)
        ? "@@yClan Deserter" : "");
    cnt++;

    strcpy(flags[cnt], victim->position == POS_BUILDING ? "@@aBuilding" : "");
    cnt++;
    strcpy(flags[cnt], IS_IMMORTAL(victim)
        ? "@@aImmortal" : "");
    cnt++;
    strcpy(flags[cnt], IS_IMMORTAL(ch) && IS_SET(victim->act, PLR_WIZINVIS)
        ? "@@aWIZI: %d" : "");
    cnt++;

    lastflag = cnt - 1;

    if (strcmp(flags[1], ""))
        sprintf(flags[1], flags[1], victim->timer);
    if (strcmp(flags[lastflag], ""))
        sprintf(flags[lastflag], flags[lastflag], victim->invis);

    send_to_char("@@d.-----------------------------------------------------------------@@g=( @@WWhois @@g)=@@d-.\n\r", ch);
    send_to_char("@@d| @@W", ch);

    space = my_strlen(victim->short_descr);
    space += my_strlen(victim->pcdata->title);
    space = 75 - space;

    sprintf(buf, "%s%s@@N", victim->short_descr, victim->pcdata->title);

    for (cnt = 0; cnt < space; cnt++)
        safe_strcat(MSL, buf, " ");

    safe_strcat(MSL, buf, " @@d|\n\r@@d|-----------------------------------------------------------------------------|\n\r@@d| @@gClan@@d: @@W");
    send_to_char(buf, ch);

    sprintf(buf, "%s", clan_table[victim->pcdata->clan].clan_name);
    space = 69 - my_strlen(buf);
    for (cnt = 0; cnt < space; cnt++)
        safe_strcat(MSL, buf, " ");

    safe_strcat(MSL, buf, " @@d|\n\r@@d| @@gSex@@d: @@W");
    send_to_char(buf, ch);

    sprintf(buf, "%s", (victim->sex == SEX_MALE) ? "Male" : (victim->sex == SEX_FEMALE) ? "Female" : "None");
    space = 32 - my_strlen(buf);
    for (cnt = 0; cnt < space; cnt++)
        safe_strcat(MSL, buf, " ");

    safe_strcat(MSL, buf, "@@gRace@@d: @@W");
    send_to_char(buf, ch);

    sprintf(buf, "%s", IS_NPC(ch) ? "n/a" : race_table[victim->race].race_title);
    space = 32 - my_strlen(buf);
    for (cnt = 0; cnt < space; cnt++)
        safe_strcat(MSL, buf, " ");

    safe_strcat(MSL, buf, " @@d|\n\r");
    send_to_char(buf, ch);

    send_to_char("@@d| @@gPlayer Kill@@d: @@a", ch);
    sprintf(buf, "%d@@d(@@W%d@@d|@@e%d@@d)", victim->pcdata->pkills + victim->pcdata->unpkills, victim->pcdata->pkills, victim->pcdata->unpkills);
    space = my_strlen(buf);
    send_to_char(buf, ch);
    send_to_char(" @@d/ @@c", ch);
    sprintf(buf, "%d", victim->pcdata->pkilled);
    space += my_strlen(buf);
    space = 21 - space;
    for (cnt = 0; cnt < space; cnt++)
        safe_strcat(MSL, buf, " ");

    send_to_char(buf, ch);

    send_to_char("@@gMob Kill@@d: @@a", ch);
    sprintf(buf, "%d", victim->pcdata->mkills);
    space = my_strlen(buf);
    send_to_char(buf, ch);
    send_to_char(" @@d/ @@c", ch);
    sprintf(buf, "%d", victim->pcdata->mkilled);
    space += my_strlen(buf);
    space = 25 - space;
    for (cnt = 0; cnt < space; cnt++)
        safe_strcat(MSL, buf, " ");

    safe_strcat(MSL, buf, " @@d|\n\r");
    send_to_char(buf, ch);

    send_to_char("@@d|                                                                             |\n\r", ch);

    if ((!IS_IMMORTAL(victim) && !victim->pcdata->avatar)
        || (IS_IMMORTAL(victim) && ch->level == MAX_LEVEL)
        || (ch == victim)
        || (ch->level == MAX_LEVEL)
        ) {
        buf2[0] = '\0';
        for (cnt = 0; cnt < MAX_CLASS; cnt++) {
            sprintf(buf, "@@b%s@@d[@@W", class_table[cnt].who_name);
            safe_strcat(MAX_STRING_LENGTH, buf2, buf);
            if (victim->lvl[cnt] != -1)
                sprintf(buf, "%2d@@d] ", victim->lvl[cnt]);
            else
                sprintf(buf, "%s@@d] ", " 0");
            safe_strcat(MAX_STRING_LENGTH, buf2, buf);
        }

        sprintf(buf, "@@d|%s @@d|\n\r", center_text(buf2, 76));
        send_to_char(buf, ch);
        buf2[0] = '\0';

        if (is_remort(victim)) {
            for (cnt = 0; cnt < MAX_CLASS; cnt++) {
                if (victim->lvl2[cnt] != -1) {
                    sprintf(buf, "@@m%s@@d[@@W%2d@@d] ", remort_table[cnt].who_name, victim->lvl2[cnt]);
                    safe_strcat(MAX_STRING_LENGTH, buf2, buf);
                }
            }
            sprintf(buf, "@@d|%s @@d|\n\r", center_text(buf2, 76));
            send_to_char(buf, ch);
            sprintf(buf, "@@gPseudo Level@@d[@@W%d@@d]", get_pseudo_level(victim));

            if (victim->adept_level > 0) {
                sprintf(buf2, "            @@gAdept@@d[@@m%s@@d]", IS_IMMORTAL(victim) ? victim->pcdata->who_name : get_adept_name(victim)
                    );
                safe_strcat(MAX_STRING_LENGTH, buf, buf2);
            }

            sprintf(buf, "@@d|%s @@d|\n\r", center_text(buf, 76));
            send_to_char(buf, ch);
        }

    }

    for (cnt = 0, space = 0; cnt <= lastflag; cnt++) {
        if (strcmp(flags[cnt], "")) {
            hasflags = TRUE;
            break;
        }
    }

    if (hasflags) {
        send_to_char("@@d|                                                                             |\n\r", ch);
        strcpy(buf, "@@d|");
        space = 67 - my_strlen(victim->short_descr);
        for (cnt = 0; cnt < space; cnt++)
            safe_strcat(MSL, buf, "-");

        safe_strcat(MSL, buf, "@@g=( @@W");
        sprintf(buf + strlen(buf), "%s", victim->short_descr);
        safe_strcat(MSL, buf, " is @@g)=@@d-|\n\r");
        send_to_char(buf, ch);

        buf[0] = '\0';

        for (cnt = 0, space = 0; cnt <= lastflag; cnt++) {
            if (!strcmp(flags[cnt], ""))
                continue;

            space++;
            if (space % 4 == 1)
                safe_strcat(MSL, buf, "@@d|");

            sprintf(buf2, " %s ", flags[cnt]);
            safe_strcat(MSL, buf, buf2);
            other2 = (space % 4 == 1 || space % 4 == 0) ? 19 : 18;
            other2 = other2 - my_strlen(buf2);
            for (other = 0; other < other2; other++)
                safe_strcat(MSL, buf, " ");
            safe_strcat(MSL, buf, "@@d|");

            if (space % 4 == 0)
                safe_strcat(MSL, buf, "\n\r");
        }
        if (space < 4) {
            switch (space) {
                default:
                    space = 19 + 18 + 18 + 19 + 4 - space;
                    break;
                case 1:
                    space = 18 + 18 + 19 + 3 - space;
                    break;
                case 2:
                    space = 18 + 19 + 3 - space;
                    break;
                case 3:
                    space = 19;
                    break;
            }

            for (cnt = 0; cnt < space; cnt++)
                safe_strcat(MSL, buf, " ");
            safe_strcat(MSL, buf, "@@d|\n\r");
        }
        else if (space % 4) {
            while (space % 4) {
                space++;
                other2 = (space % 4 == 1 || space % 4 == 0) ? 19 : 18;
                for (other = 0; other < other2; other++)
                    safe_strcat(MSL, buf, " ");
                safe_strcat(MSL, buf, "@@d|");
            }

            safe_strcat(MSL, buf, "\n\r");
        }

        send_to_char(buf, ch);
    }
    send_to_char("@@d'-----------------------------------------------------------------------------'@@N\n\r", ch);
    return;
}

/*
 * START: shelp addition
 */

void
do_shelp(CHAR_DATA *ch, char *argument)
{
    /* Like help, except for spells and skills. */
    int                 sn, cnt;
    char                buf[MAX_STRING_LENGTH];
    char                buf2[MAX_STRING_LENGTH * 8];
    char               *buf3;
    char                buf4[MAX_STRING_LENGTH];
    char                buf5[MAX_STRING_LENGTH];
    SHELP_DATA         *sHelp;
    bool                found = FALSE;

    buf[0] = '\0';
    buf2[0] = '\0';
    buf4[0] = '\0';
    buf5[0] = '\0';

    if (argument[0] == '\0') {
        do_help(ch, "shelp_summary");
        return;
    }

    if ((sn = skill_lookup(argument)) < 0) {
        sendf(ch, "No sHelp found for %s.\n\r", argument);
        return;
    }

    for (sHelp = first_shelp; sHelp != NULL; sHelp = sHelp->next) {
        if (!str_cmp(skill_table[sn].name, sHelp->name)) {
            found = TRUE;
            send_to_char("@@d.------------------------------------------------------@@g=( @@WSpell/Skill Help @@g)=@@d-.@@N\n\r", ch);

            sprintf(buf, "@@d| @@a%s @@d(%s@@d) ", sHelp->name, (skill_table[sn].spell_fun == spell_null ? "Skill" : "Spell")
                );

            for (cnt = 0; cnt < (66 - strlen(skill_table[sn].name)); cnt++)
                safe_strcat(MSL, buf, " ");

            safe_strcat(MSL, buf, " @@d|\n\r");

            safe_strcat(MSL, buf2, buf);
            safe_strcat(MSL, buf2, "@@d|-----------------------------------------------------------------------------|\n\r");

            for (cnt = 0; cnt < MAX_CLASS; cnt++) {
                if (skill_table[sn].skill_level[cnt] > 0 && skill_table[sn].skill_level[cnt] <= 80 && skill_table[sn].flag2 == NORM && skill_table[sn].flag1 != AVATAR) {
                    if (skill_table[sn].flag1 == MORTAL) {
                        sprintf(buf5, "%s(%d), ", class_table[cnt].class_name, skill_table[sn].skill_level[cnt]);
                    }
                    else if (skill_table[sn].flag1 == REMORT) {
                        sprintf(buf5, "%s(%d), ", remort_table[cnt].class_name, skill_table[sn].skill_level[cnt]);
                    }
                    else if (skill_table[sn].flag1 == ADEPT) {
                        sprintf(buf5, "Adept(%d), ", skill_table[sn].skill_level[cnt]);
                    }
                    safe_strcat(MSL, buf4, buf5);
                }
            }

            if (skill_table[sn].flag1 == AVATAR && skill_table[sn].flag2 == NORM)
                safe_strcat(MSL, buf4, "Avatar");
            else if (skill_table[sn].flag1 == AVATAR && skill_table[sn].flag2 == SUB) {
                switch (skill_table[sn].skill_level[0]) {
                    /* TODO: use AV_* macros for these percentages */
                    case 1: safe_strcat(MSL, buf4, "Avatar (9% practiced)");   break;
                    case 2: safe_strcat(MSL, buf4, "Avatar (31% practiced)");  break;
                    case 3: safe_strcat(MSL, buf4, "Avatar (54% practiced)");  break;
                    case 4: safe_strcat(MSL, buf4, "Avatar (77% practiced)");  break;
                    case 5: safe_strcat(MSL, buf4, "Avatar (100% practiced)"); break;
                    default:safe_strcat(MSL, buf4, "Avatar (?)");     break;
                }
            }
            else
                buf4[strlen(buf4) - 2] = '\0';

            buf3 = wordwrap(buf4, "@@d| @@WClasses     @@d: @@g%s @@d|\n\r", "@@d|               @@g%s @@d|\n\r", 61);

            safe_strcat(MSL, buf2, buf3);

            if (str_cmp(sHelp->duration, "n/a")) {
                sprintf(buf, "@@d| @@WDuration    @@d: @@g%-61s @@d|\n\r", sHelp->duration);
                safe_strcat(MSL, buf2, buf);
            }
            if (str_cmp(sHelp->modify, "n/a")) {
                sprintf(buf, "@@d| @@WModifies    @@d: @@g%-61s @@d|\n\r", sHelp->modify);
                safe_strcat(MSL, buf2, buf);
            }
            if (str_cmp(sHelp->type, "n/a")) {
                sprintf(buf, "@@d| @@WType        @@d: @@g%-61s @@d|\n\r", sHelp->type);
                safe_strcat(MSL, buf2, buf);
            }
            if (str_cmp(sHelp->target, "n/a")) {
                sprintf(buf, "@@d| @@WTarget      @@d: @@g%-61s @@d|\n\r", sHelp->target);
                safe_strcat(MSL, buf2, buf);
            }

            if (str_cmp(sHelp->desc, "n/a")) {
                buf3 = wordwrap(sHelp->desc, "@@d| @@WDescription @@d: @@g%s @@d|\n\r", "@@d|               @@g%s @@d|\n\r", 61);

                safe_strcat(MSL, buf2, buf3);
            }
            safe_strcat(MSL, buf2, "@@d'-----------------------------------------------------------------------------'@@N\n\r");
            send_to_char(buf2, ch);

            break;
        }
    }

    if (!found)
        send_to_char("Couldn't find a sHelp for that skill/spell.\n\r", ch);

    return;
}

/*
 * FINISH: shelp addition
 */

void
do_rules(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_STRING_LENGTH];

    if (argument[0] == '\0')
        argument = "rules";

    one_argument(argument, arg);

    if (!send_help(ch, arg, HELP_RULES, FALSE))
        send_to_char("Couldn't find that rules file.\n\r", ch);

    return;
}

void
do_map(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_STRING_LENGTH];

    if (argument[0] == '\0') {
        do_maps(ch, "");
        return;
    }

    one_argument(argument, arg);

    if (!send_help(ch, arg, HELP_MAP, FALSE))
        send_to_char("Couldn't find that map.\n\r", ch);

    return;
}

void
do_maps(CHAR_DATA *ch, char *argument)
{
    send_to_char("@@N@@gBelow is a list of maps you can look up using @@amap <keyword>@@g:\n\r\n\r", ch);
    show_helps(ch, HELP_MAP);
    return;
}

void
do_afk_msg(CHAR_DATA *ch, char *argument)
{
    char                msgbuf[MSL];
    char                afk_msg[MSL];

    sprintf(afk_msg, "%s is currently Away From Keyboard", ch->short_descr);

    if (ch->afk_msg == NULL)
        ch->afk_msg = str_dup(afk_msg);
    sprintf(msgbuf, "Your AFK message is: '%s'", ch->afk_msg);
    send_to_char(msgbuf, ch);
    return;
}

void
do_afk(CHAR_DATA *ch, char *argument)
{
    int                 value;
    int                 value2;
    char                afk_arg[MAX_STRING_LENGTH];

    smash_tilde(argument);

    strcpy(afk_arg, argument);

    if (argument[0] != '\0')
        ch->afk_msg = str_dup(afk_arg);

    if (IS_NPC(ch))
        return;
    if (argument[0] == '\0') {
        value = table_lookup(tab_player_flags, "AFK");
        value2 = table_lookup(tab_player_flags, "XAFK");
    }
    else {
        value = table_lookup(tab_player_flags, "XAFK");
        value2 = table_lookup(tab_player_flags, "AFK");
    }
    if (IS_SET(ch->pcdata->pflags, value) || IS_SET(ch->pcdata->pflags, value2)) {
        REMOVE_BIT(ch->pcdata->pflags, value);
        REMOVE_BIT(ch->pcdata->pflags, value2);
        ch->afk_msg = NULL;
        send_to_char("AFK flag turned off.\n\r", ch);

        display_messages(ch, argument);
    }
    else {
        SET_BIT(ch->pcdata->pflags, value);
        send_to_char("AFK flag turned on.\n\r", ch);
    }
    return;
}

/* answering machine stuff by -ogma- */
void
display_messages(CHAR_DATA *ch, char *argument)
{
    ANSWERING_DATA     *answering, *answering_next;
    char                buf[MAX_STRING_LENGTH];
    char        _buf2[MSL];
    char                *buf2 = _buf2;
    int                 count = 1;
    char                _buf3[64];
    char                *buf3 = _buf3;


    if (!IS_NPC(ch) && IS_SET(ch->config, PLR_ANSWERING)) {
        answering = ch->first_message;

        if (answering == NULL)
            return;

        send_to_char("@@N@@d.-------------------------------------------------------------@@g=( @@eM@@Ressages @@g)=@@d-.\n\r", ch);

        for (answering = ch->first_message; answering; answering = answering_next) {
            answering_next = answering->next;

            _buf3[0] = 0;
            sprintf(buf, "@@d| @@y[@@b%d@@y] @@N", count);
            sprintf(buf2, " @@c%s ", ctime(&answering->time));
            buf2[24] = 0;
            sprintf(buf + strlen(buf), "%s", buf2);
            sprintf(buf2,"@@W( @@g%s ago@@W)",duration(current_time - answering->time, buf3));
            sprintf(buf + strlen(buf), " %s@@R(@@e%s@@R) ", my_left(buf2, buf2, 45 - UMAX(1, my_strlen(answering->name))), answering->name);
            sprintf(buf + strlen(buf), "@@d |@@N\n\r");
            send_to_char(buf, ch);

            sprintf(buf, "%s", answering->message);

            buf2 = wordwrap(buf, "@@d| @@g%s @@d|\n\r", "@@d| @@g%s @@d|\n\r", 74);

            send_to_char(buf2, ch);
            send_to_char("@@d'----------------------------------------------------------------------------'@@N\n\r", ch);

            /* clean up */
            free_string(answering->name);
            free_string(answering->message);
            UNLINK(answering, ch->first_message, ch->last_message, next, prev);
            PUT_FREE(answering, answering_free);
            count++;
        }
    }

    ch->num_messages = 0;
    return;
}

void
do_Xafk(CHAR_DATA *ch, char *argument)
{
    int                 value;

    if (IS_NPC(ch))
        return;

    value = table_lookup(tab_player_flags, "XAFK");

    if (IS_SET(ch->pcdata->pflags, value)) {
        REMOVE_BIT(ch->pcdata->pflags, value);
        send_to_char("Extended AFK flag turned off.\n\r", ch);
    }
    else {
        SET_BIT(ch->pcdata->pflags, value);
        send_to_char("Extended AFK flag turned on.\n\r", ch);
    }
    return;
}

void
do_gold(CHAR_DATA *ch, char *argument)
{
    sendf(ch, "You are carrying @@y%s @@Ngold coin%s.\n\r", number_comma(ch->gold), ch->gold == 1 ? "" : "s");
    return;
}

void
do_colist(CHAR_DATA *ch, char *argument)
{
    int                 col, n;

    if IS_NPC
        (ch)
            return;

    send_to_char("@@WColour Codes: @@yTo use colour codes within a string, use the following\n\r", ch);
    send_to_char("characters in sequence: @@_<letter>.@@N\n\r\n\r", ch);

    n = 0;

    for (col = 0; col < MAX_ANSI; col++) {
        sendf(ch, "%c - %s%-14s@@N    ", ansi_table[col].letter, IS_SET(ch->act, PLR_COLOUR) ? ansi_table[col].value : "", ansi_table[col].name);

        if (++n % 3 == 0)
            send_to_char("\n\r", ch);
    }

    if (n % 3 != 0)
        send_to_char("\n\r", ch);

    send_to_char("\n\r", ch);
    return;
}

void
do_loot(CHAR_DATA *ch, char *argument)
{

    char                arg[MAX_INPUT_LENGTH];
    OBJ_DATA           *corpse;
    OBJ_DATA           *obj = NULL;
    int                 counter, num;

    one_argument(argument, arg);

    if (IS_NPC(ch)) {
        send_to_char("NPC's cannot loot corpses.\n\r", ch);
        return;
    }

    if (arg[0] == '\0') {
        send_to_char("Loot what?\n\r", ch);
        return;
    }

    if (!str_cmp(arg, "all") || !str_prefix("all.", arg)) {
        send_to_char("You can't do that.\n\r", ch);
        return;
    }

    corpse = get_obj_room(ch, arg, ch->in_room->first_content);
    if (corpse == NULL) {
        act("I see no $T here.", ch, NULL, arg, TO_CHAR);
        return;
    }

    if (corpse->item_type == ITEM_CORPSE_NPC) {
        send_to_char("Just go ahead and take it.\n\r", ch);
        return;
    }

    if (corpse->item_type != ITEM_CORPSE_PC) {
        send_to_char("You cannot loot that.\n\r", ch);
        return;
    }

    /* begin checking for lootability */

    if (ch->pcdata->clan == 0 && !IS_SET(ch->pcdata->pflags, PFLAG_PKOK)) {
        send_to_char("You cannot loot corpses.\n\r", ch);
        return;
    }

    if (corpse->value[3] == 0) {
        send_to_char("You cannot loot this corpse.\n\r", ch);
        return;
    }

    if (   ch->pcdata->clan == corpse->value[2]
        || (IS_SET(ch->pcdata->pflags, PFLAG_PKOK) && corpse->value[0] == 1)
       ) {
        counter = number_range(1, 100);

        if (counter >= 40) {
            if (corpse->first_in_carry_list == NULL) {
                send_to_char("There isn't anything in the corpse.\n\r", ch);
                return;
            }

            num = 0;
            for (obj = corpse->first_in_carry_list; obj != NULL; obj = obj->next_in_carry_list) {
                ++num;
            }

            counter = number_range(1, num);

            obj = corpse->first_in_carry_list;
            for (num = 1; num < counter; ++num) {
                obj = obj->next_in_carry_list;
            }

            if (lootable_item(obj)) {
                get_obj(ch, obj, corpse);

                /* for Q! */
                {
                    char                buf[MSL];

                    sprintf(buf, "%s loots %s from %s", ch->short_descr, obj->short_descr, corpse->short_descr);
                    monitor_chan(buf, MONITOR_OBJ);
                    log_string(buf);
                }

                /* just incase... */
                if (ch->level > 1) {
                    do_save(ch, "");
                }
            }
            else {
                send_to_char("You failed to loot the corpse.\n\r", ch);
                return;
            }

            corpse->value[3] = corpse->value[3] - 1;
            return;
        }
        else {
            send_to_char("You failed to loot the corpse.\n\r", ch);
            return;
        }
    }

    send_to_char("You cannot loot this corpse.\n\r", ch);
    return;
}

void
do_version(CHAR_DATA *ch, char *argument)
{
    extern int          build_no;
    extern time_t       creation_time;

    char                buf2[64];
    char               *_buf2 = buf2;

    buf2[0] = 0;

    sendf(ch, "%s is running a modified version of ACK!Mud 4.2. Build #%d. Last compiled %s ago.\n\r",
        mudnamecolor, build_no, duration(current_time - creation_time, _buf2));

    return;
}

int
dur_to_secs(char *argument)
{
    char                _buf[8192];
    char               *buf = _buf;
    char                c;
    int                 secs = 0;
    int                 d;

    _buf[0] = 0;

    while ((c = *argument++) != '\0') {
        if (isdigit(c))
            *buf++ = c;
        else {
            if (_buf[0] == 0)
                continue;

            *buf = 0;

            d = atoi(_buf);
            memset(_buf, 0, sizeof(_buf));
            buf = _buf;

            if (d == 0)
                continue;

            switch (LOWER(c)) {
                case 'w':
                    secs += d * 60 * 60 * 24 * 7;
                    break;
                case 'd':
                    secs += d * 60 * 60 * 24;
                    break;
                case 'h':
                    secs += d * 60 * 60;
                    break;
                case 'm':
                    secs += d * 60;
                    break;
                case 's':
                default:
                    secs += d;
                    break;
            }
        }
    }

    return UMAX(0, secs);
}

long int
custom_exp_to_level(int lvl[5], int lvl2[5], int class, int index)
{
    int                 max_level = 0;
    int                 mult;
    int                 level;
    int                 next_level_index;
    int                 totlevels = 0, diff;
    long int            cost;
    int                 a;

    if ((index == 5) && (lvl2[class] <= 0))
        return 0;

    for (a = 0; a < MAX_CLASS; a++)
        if (lvl[a] > max_level)
            max_level = lvl[a];

    switch (index) {
        case 0:
            mult = 3;
            break;
        case 1:
            mult = 4;
            break;
        case 2:
            mult = 5;
            break;
        case 3:
            mult = 6;
            break;
        case 4:
            mult = 7;
            break;
        default:
            mult = 23;
            break;
    }

    if (index == 5)
        level = UMAX(0, lvl2[class]);
    else
        level = UMAX(0, lvl[class]);

    for (a = 0; a < MAX_CLASS; a++) {
        totlevels += lvl[a];

        if (lvl2[a] > 0)
            totlevels += lvl2[a];
    }

    if (index != 5)
        next_level_index = lvl[class];
    else
        next_level_index = UMIN(lvl2[class] + 20, 79);

    if (next_level_index < 0)
        next_level_index = 0;

    cost = exp_table[next_level_index].exp_base[class];

    diff = (totlevels / MAX_CLASS) - (level + 20);

    if (index == 5)
        diff -= 30;
    if (diff < 10)
        diff = 10;

    cost *= (diff / 10);

    if ((index != 5) && ((max_level - lvl[class]) > 25))
        cost *= (diff / 7);

    cost *= mult;
    cost /= 5.4;

    return (cost);
}

struct exp_holder
{
    int                 class;
    int                 remort;
    int                 level;
} exphold[1024];

void
do_xpcalc(CHAR_DATA *ch, char *argument)
{
    int                 lvl[5] = { 10, 10, 80, 10, 10 };
    int                 lvl2[5] = { 0, 0, 0, 0, 0 };
    int                 order[5] = { 0, 0, 0, 0, 0 };
    int                 cnt = 0;
    int                 a, b;
    int                 xp = 0;
    int                 totxp = 0;
    int                 thisclass = -1;
    int                 remort = 0;
    int                 level = 0;
    int                 class = 0;
    char                buf[MSL];
    char                buf2[MSL];
    char                _arg[MAX_INPUT_LENGTH];

    if (IS_NPC(ch))
        return;

    if (*argument == '\0') { do_help(ch, "xpcalc"); return; }

    for (a = 0; a < 1024; a++) {
        exphold[a].class = -1;
        exphold[a].remort = 0;
        exphold[a].level = -1;
    }

    for (a = 0; a < MAX_CLASS; a++) {
        order[a] = ch->pcdata->index[a];
        lvl[a] = (ch->lvl[a] > 0) ? ch->lvl[a] : 0;
        lvl2[a] = ch->lvl2[a];
    }

    while (*argument) {
        char               *arg = _arg;

        remort = 0;
        thisclass = -1;

        argument = one_argument(argument, _arg);

        a = atoi(_arg);
        if (isdigit(*arg))
            while (isdigit(*++arg));

        if (strcmp(arg, "mag") == 0)
            thisclass = 0;
        else if (strcmp(arg, "cle") == 0)
            thisclass = 1;
        else if (strcmp(arg, "thi") == 0)
            thisclass = 2;
        else if (strcmp(arg, "war") == 0)
            thisclass = 3;
        else if (strcmp(arg, "psi") == 0)
            thisclass = 4;
        else if (strcmp(arg, "sor") == 0) {
            thisclass = 0;
            remort = 1;
        }
        else if (strcmp(arg, "ass") == 0) {
            thisclass = 1;
            remort = 1;
        }
        else if (strcmp(arg, "kni") == 0) {
            thisclass = 2;
            remort = 1;
        }
        else if (strcmp(arg, "nec") == 0) {
            thisclass = 3;
            remort = 1;
        }
        else if (strcmp(arg, "mon") == 0) {
            thisclass = 4;
            remort = 1;
        }
        else if (strcmp(arg, "xp") == 0) {
            xp = a;
        }
        else if (strcmp(arg, "myxp") == 0) {
            xp = ch->exp;
        }

        if (thisclass > -1) {
            exphold[cnt].class = thisclass;
            exphold[cnt].remort = remort;
            exphold[cnt].level = URANGE(1, a, 80);
            cnt++;
        }
    }

    if (xp == 0)
        xp = 2147483647;

    for (a = 0; exphold[a].level != -1 && a < 1024; a++) {
        class = exphold[a].class;
        remort = exphold[a].remort;
        level = exphold[a].level;

        for (b = (!remort) ? lvl[class] : lvl2[class]; b < level; b++) {
            if ((!remort && lvl[class] > level) || (remort && lvl2[class] > level))
                break;

            if (xp > 0)
                xp = UMAX(0, xp - custom_exp_to_level(lvl, lvl2, class, (!remort) ? order[class] : 5));

            if (xp == 0)
                break;

            if (totxp + custom_exp_to_level(lvl, lvl2, class, (!remort) ? order[class] : 5) < 0)
                break;

            totxp += custom_exp_to_level(lvl, lvl2, class, (!remort) ? order[class] : 5);
            if (!remort)
                lvl[class]++;
            else
                lvl2[class]++;

        }
    }

    if (totxp <= ch->exp)
        sprintf(buf, "@@gGaining @@y%s @@gXP worth, your levels would look like this:\n\r\n\r", number_comma(totxp));
    else {
        sprintf(buf, "@@gFor these levels, you need @@y%s @@gXP. ", number_comma(totxp));
        sprintf(buf + strlen(buf), "You have @@y%s @@gXP (%s@@N@@g):\n\r\n\r", number_comma(ch->exp), percbar(ch->exp, totxp, 10));
    }

    send_to_char(buf, ch);

    sendf(ch, "@@bMag@@d[@@W%2d@@d] @@bCle@@d[@@W%2d@@d] @@bThi@@d[@@W%2d@@d] @@bWar@@d[@@W%2d@@d] @@bPsi@@d[@@W%2d@@d]@@N\n\r", lvl[0], lvl[1], lvl[2], lvl[3], lvl[4]);

    buf2[0] = 0;

    for (cnt = 0; cnt < MAX_CLASS; cnt++) {
        if (lvl2[cnt] != -1) {
            sprintf(buf, "@@m%s@@d[@@W%2d@@d] ", remort_table[cnt].who_name, lvl2[cnt]);
            safe_strcat(MAX_STRING_LENGTH, buf2, buf);
            safe_strcat(MAX_STRING_LENGTH, buf2, "@@N");
        }
    }

    if (buf2[0] != 0) {
        safe_strcat(MSL, buf2, "\n\r");
        send_to_char(buf2, ch);
    }

    return;
}

void
do_token(CHAR_DATA *ch, char *argument)
{
    OBJ_INDEX_DATA     *pObjIndex;
    OBJ_DATA           *obj;
    char                buf[MSL];

    int                 amt = 0;

    if (IS_NPC(ch))
        return;

    if (*argument == '\0') {
        send_to_char("syntax: token <amount>\n\r", ch);
        return;
    }

    amt = atoi(argument);

    if (amt <= 0) {
        send_to_char("1 or above, love!\n\r", ch);
        return;
    }

    if (amt > available_qps(ch)) {
        send_to_char("You don't have that many quest points.\n\r", ch);
        return;
    }

    if (ch->carry_number >= can_carry_n(ch)) {
        send_to_char("Your inventory is full.\n\r", ch);
        return;
    }

    if (ch->carry_weight + 5 >= can_carry_w(ch)) {
        send_to_char("You are overweight. (Go on a diet or do some exercize!)\n\r", ch);
        return;
    }

    pObjIndex = get_obj_index(OBJ_VNUM_TOKEN);

    if (pObjIndex == NULL) {
        send_to_char("Problem loading the token.\n\r", ch);
        return;
    }

    obj = create_object(pObjIndex, 0);

    if (obj == NULL) {
        send_to_char("Problem creating the token.\n\r", ch);
        return;
    }

    sprintf(buf, obj->short_descr, amt);
    if (obj->short_descr)
        free_string(obj->short_descr);
    obj->short_descr = str_dup(buf);

    obj->value[0] = amt;
    obj->value[3] = get_pseudo_level(ch);

    obj_to_char(obj, ch);
    ch->quest_points -= amt;
    save_char_obj(ch);

    send_to_char("Done!\n\r", ch);

    sprintf(buf, "%s makes a %d qp token, leaving them with %d qps.", ch->name, amt, ch->quest_points);
    log_string(buf);
    return;
}

void
do_cleft(CHAR_DATA *ch, char *argument)
{
    AFFECT_DATA        *af;
    int                 amt = -1;

    if (!IS_NPC(ch) || !IS_AFFECTED(ch, AFF_CHARM) || !ch->master) {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    for (af = ch->first_affect; af != NULL; af = af->next) {
        if (IS_SET(af->bitvector, AFF_CHARM)) {
            amt = af->duration;
            break;
        }
    }

    if (amt == -1 && ch->extract_timer > 0)
        amt = ch->extract_timer;

    if (amt != -1)
        sendf(ch->master, "%s has %d %s remaining.\n\r", ch->short_descr, amt, (amt == 1) ? "hour" : "hours");

    return;
}

void do_autostance(CHAR_DATA *ch, char *argument)
{
    int cnt;

    if (argument[0] == '\0') {
        send_to_char("syntax: autostance <stance|none>\n\r", ch);
        return;
    }

    if (!str_cmp("none", argument)) {
        free_string(ch->pcdata->autostance);
        ch->pcdata->autostance = str_dup("");
        send_to_char("Your autostance has been removed.\n\r", ch);
        save_char_obj(ch);
        return;
    }

    for (cnt = 0; cnt < MAX_STANCE; cnt++)
        if (!str_cmp(stance_app[cnt].name, argument))
            break;

    if (cnt == MAX_STANCE) {
        send_to_char("Invalid stance.\n\r", ch);
        return;
    }

    free_string(ch->pcdata->autostance);
    ch->pcdata->autostance = str_dup(stance_app[cnt].name);
    sendf(ch, "Your autostance has been set to: %s.\n\r", ch->pcdata->autostance);
    WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
    save_char_obj(ch);
    return;
}

/* Max players Time by Ogma */
void 
do_max_players(CHAR_DATA *ch, char *argument)
{
    extern int save_max_players_t;
    extern int save_max_players;
    char       buf[MAX_STRING_LENGTH];
    char       buf2[MSL];
    char       buf3[MSL];

    buf[0] = 0;
    buf2[0] = 0;
    buf3[0] = 0;


    send_to_char("@@N@@d.----------------------------------------------------------@@g=( @@eM@@Rax @@eP@@Rlayers @@g)=@@d-.\n\r", ch);
    send_to_char("@@d| @@N", ch);
    sprintf(buf3, "@@cMax players of %d occured on", save_max_players);
    sprintf(buf2, "%s", ctime((time_t *)&save_max_players_t));
    buf2[24] = 0;
    sprintf(buf + strlen(buf), "%s %s", buf3, buf2);
    sprintf(buf2,"@@W (@@g%s ago@@W)",duration(current_time - save_max_players_t, buf3));
    sprintf(buf + strlen(buf), "%s", buf2);
    sprintf(buf, "%s%s", my_left(buf, buf, 78 - UMAX(1, my_strlen("@@d|@@N\n\r"))), "@@d|@@N\n\r");
    send_to_char(buf, ch);

    send_to_char("@@d'----------------------------------------------------------------------------'@@N\n\r", ch);

    return;
}
