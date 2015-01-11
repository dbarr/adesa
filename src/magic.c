
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
#include "tables.h"
#include "duel.h"

IDSTRING(rcsid, "$Id: magic.c,v 1.51 2004/10/25 18:51:34 dave Exp $");

/*
 * Local functions.
 */
void say_spell      args((CHAR_DATA *ch, int sn));
void energy_advance(CHAR_DATA *ch);

/* Calculate mana cost */
int
mana_cost(CHAR_DATA *ch, int sn)
{
    int                 best;
    int                 foo;
    int                 skill_lev;
    int                 cost, mincost;
    int                 class = 0;

    if (skill_table[sn].flag1 == AVATAR)
        return skill_table[sn].min_mana;

    if (IS_NPC(ch)) {
        best = get_pseudo_level(ch);
        for (foo = 0; foo < MAX_CLASS; foo++)
            if (best >= skill_table[sn].skill_level[foo]) {
                class = foo;
            }

        if ((skill_table[sn].flag1 == REMORT)
            && (((IS_SET(ch->act, ACT_PET))
                    || (IS_AFFECTED(ch, AFF_CHARM)))
                && (ch->rider == NULL)))
            best = -1;

        if (skill_table[sn].flag1 == ADEPT)
            best = -1;

    }
    else {
        best = -1;
        for (foo = 0; foo < MAX_CLASS; foo++)
            if (ch->lvl[foo] >= skill_table[sn].skill_level[foo]
                && ch->lvl[foo] > best) {
                best = ch->lvl[foo];
                class = foo;
            }
    }
    if (skill_table[sn].flag1 == ADEPT) {
        best = -1;
        if (ch->adept_level >= skill_table[sn].skill_level[0])
            best = ch->adept_level * 4;
    }

    if (best == -1 && IS_NPC(ch))
        return 1000;
    else if ((best == -1)
        && ((!IS_NPC(ch))
            && (!is_name(skill_table[sn].name, race_table[ch->race].skill1))))
        return (1000);

    mincost = 1000;

    skill_lev = skill_table[sn].skill_level[class];

    if (skill_lev > best)
        cost = 1000;
    else
        cost = UMAX(skill_table[sn].min_mana, 100 / (2 + best - skill_lev));

    if (cost < mincost)
        mincost = cost;

    if (IS_NPC(ch))
        mincost /= 2;

    if (!IS_NPC(ch)
        && (ch->stance == STANCE_CASTER))
        mincost = mincost - (mincost * .3);

    if ((!IS_NPC(ch))
        && (is_name(skill_table[sn].name, race_table[ch->race].skill1)))
        mincost = 10;

    if (!IS_NPC(ch) && (skill_table[sn].flag2 == NORM)) {
        if (IS_SET(race_table[ch->race].race_flags, RACE_MOD_NO_MAGIC))
            mincost *= 1.75;
        else if (IS_SET(race_table[ch->race].race_flags, RACE_MOD_WEAK_MAGIC))
            mincost *= 1.25;
        else if (IS_SET(race_table[ch->race].race_flags, RACE_MOD_STRONG_MAGIC))
            mincost *= .75;
    }
    return mincost;
}

/*
 * Lookup a skill by name.
 */
int
skill_lookup(const char *name)
{
    int                 sn;

    for (sn = 0; sn < MAX_SKILL; sn++) {
        if (skill_table[sn].name == NULL)
            break;
        if (LOWER(name[0]) == LOWER(skill_table[sn].name[0])
            && !str_prefix(name, skill_table[sn].name))
            return sn;
    }

    return -1;
}

/*
 * Lookup a skill by slot number.
 * Used for object loading.
 */
int
slot_lookup(int slot)
{
    extern bool         fBootDb;
    int                 sn;

    if (slot <= 0)
        return -1;

    for (sn = 0; sn < MAX_SKILL; sn++) {
        if (slot == skill_table[sn].slot)
            return sn;
    }

    if (fBootDb) {
        bugf("Slot_lookup: bad slot %d.", slot);
        abort();
    }

    return -1;
}

/*
 * Utter mystical words for an sn.
 */
void
say_spell(CHAR_DATA *ch, int sn)
{
    char                buf[MAX_STRING_LENGTH];
    char                buf2[MAX_STRING_LENGTH];
    char                msg[MAX_STRING_LENGTH];
    char                msg2[MAX_STRING_LENGTH];
    CHAR_DATA          *rch;
    char               *pName;
    int                 iSyl;
    int                 length;

    struct syl_type
    {
        char               *old;
        char               *new;
    };

    static const struct syl_type syl_table[] = {
        {" ", " "},
        {"ar", "abra"},
        {"au", "kada"},
        {"bless", "fido"},
        {"blind", "nose"},
        {"bur", "mosa"},
        {"cu", "judi"},
        {"de", "oculo"},
        {"en", "unso"},
        {"light", "dies"},
        {"lo", "hi"},
        {"mor", "zak"},
        {"move", "sido"},
        {"ness", "lacri"},
        {"ning", "illa"},
        {"per", "duda"},
        {"ra", "gru"},
        {"re", "candus"},
        {"son", "sabru"},
        {"tect", "infra"},
        {"tri", "cula"},
        {"ven", "nofo"},
        {"a", "a"}, {"b", "b"}, {"c", "q"}, {"d", "e"},
        {"e", "z"}, {"f", "y"}, {"g", "o"}, {"h", "p"},
        {"i", "u"}, {"j", "y"}, {"k", "t"}, {"l", "r"},
        {"m", "w"}, {"n", "i"}, {"o", "a"}, {"p", "s"},
        {"q", "d"}, {"r", "f"}, {"s", "g"}, {"t", "h"},
        {"u", "j"}, {"v", "z"}, {"w", "x"}, {"x", "n"},
        {"y", "l"}, {"z", "k"},
        {"", ""}
    };

    buf[0] = '\0';
    for (pName = skill_table[sn].name; *pName != '\0'; pName += length) {
        for (iSyl = 0; (length = strlen(syl_table[iSyl].old)) != 0; iSyl++) {
            if (!str_prefix(syl_table[iSyl].old, pName)) {
                strcat(buf, syl_table[iSyl].new);
                break;
            }
        }

        if (length == 0)
            length = 1;
    }

    /* Give some other message, according to TAR type and if !NPC */
    switch (skill_table[sn].target) {
        case TAR_IGNORE:
            sprintf(msg, "$n closes $s eyes, and prays.");
            sprintf(msg2, "You close your eyes and pray.\n\r");
            break;
        case TAR_CHAR_OFFENSIVE:
            sprintf(msg, "$n's eyes glow red for an instant!");
            sprintf(msg2, "Your eyes glow red for an instant!\n\r");
            break;
        case TAR_CHAR_DEFENSIVE:
            sprintf(msg, "$n falls silent as $e meditates.");
            sprintf(msg2, "You become silent as you meditate.\n\r");
            break;
        case TAR_CHAR_SELF:
            sprintf(msg, "$n motions toward $mself.");
            sprintf(msg2, "You motion towards yourself.\n\r");
            break;
        case TAR_OBJ_INV:
            sprintf(msg, "$n's hands briefly glow magically!");
            sprintf(msg2, "Your hands briefly glow magically!\n\r");
    }
    act(msg, ch, NULL, ch, TO_NOTVICT);
    send_to_char(msg2, ch);

    sprintf(buf2, "$n utters the words, '%s'.", buf);
    sprintf(buf, "$n utters the words, '%s'.", skill_table[sn].name);

    for (rch = ch->in_room->first_person; rch; rch = rch->next_in_room) {
        if (rch != ch)
            act(ch->class == rch->class ? buf : buf2, ch, NULL, rch, TO_VICT);
    }

    {
        char                fbuf[MSL], fbuf2[MSL];
        DUEL_DATA          *duel;
        DUEL_WATCHER_DATA  *watcher;

        sprintf(fbuf, "@@d[@@gSPAR@@d]@@N %s", buf);
        sprintf(fbuf2, "@@d[@@gSPAR@@d]@@N %s", buf2);

        for (duel = first_duel; duel != NULL; duel = duel->next)
            if (duel->stage == DUEL_STAGE_GO && ch->in_room && duel->vnum == ch->in_room->vnum)
                break;

        if (duel) {
            for (watcher = duel->first_watcher; watcher != NULL; watcher = watcher->next)
                act(ch->class == watcher->ch->class ? fbuf : fbuf2, ch, NULL, watcher->ch, TO_VICT);
        }
    }

    return;
}

/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool
saves_spell(int level, CHAR_DATA *victim)
{
    int                 save;

    save = 5 + (get_pseudo_level(victim) - level - victim->saving_throw);
    save += wis_app[get_curr_wis(victim)].spell_save;
    if ((IS_NPC(victim)) && (IS_SET(victim->act, ACT_SOLO)))
        save += 20;
    if (!IS_NPC(victim)
        && (IS_SET(race_table[victim->race].race_flags, RACE_MOD_RESIST_SPELL)))
        save += 20;
    save = URANGE(5, save, 98);

    return number_percent() < save;
}

/*
 * The kludgy global is for spells who want more stuff from command line.
 */
char               *target_name;

void
do_cast(CHAR_DATA *ch, char *argument)
{
    char                arg1[MAX_INPUT_LENGTH];
    char                arg2[MAX_INPUT_LENGTH];
    char                buf[MAX_STRING_LENGTH];
    CHAR_DATA          *victim;
    OBJ_DATA           *obj;
    void               *vo;
    int                 mana;
    int                 sn;
    int                 best;    /* The best class to use for the job :P */
    int                 cnt;
    bool                reflected = FALSE;
    bool                char_login = FALSE;
    bool                avatar_spell = FALSE;

    int                 cast_chance = 0;

    if (ch->is_free == TRUE)
        return;
    /* ZENFIX --ch's are surviving multiple kills per combat round */

    if ((!IS_NPC(ch))
        && (ch->desc != NULL)
        && (ch->desc->connected == CON_SETTING_STATS))
        char_login = TRUE;

    target_name = one_argument(argument, arg1);
    one_argument(target_name, arg2);

    if (arg1[0] == '\0') {
        send_to_char("Cast which what where?\n\r", ch);
        return;
    }

    best = -1;                    /* Default 'no way' value */

    if ((sn = skill_lookup(arg1)) == -1) {
        send_to_char("Wiggle swiggle biggle?\n\r", ch);
        return;
    }

    if (skill_table[sn].flag1 == AVATAR)
        avatar_spell = TRUE;

    /* Check if in a no-magic room */
    if (!char_login && IS_SET(ch->in_room->room_flags, ROOM_NO_MAGIC) && !IS_IMMORTAL(ch)) {
        send_to_char("Some strange force prevents you casting the spell!\n\r", ch);
        return;
    }

    if (!legal_spell(ch, sn))
        return;

    if (sn == gsn_emount) {
        send_to_char("This automatic skill cannot be casted. See 'shelp equest' for details.\n\r", ch);
        return;
    }

    /* Compute best level to use for spell, IF it meets requiements */
    if (IS_NPC(ch)) {
        best = get_pseudo_level(ch);
        if ((skill_table[sn].flag1 == REMORT)
            && (((IS_SET(ch->act, ACT_PET))
                    || (IS_AFFECTED(ch, AFF_CHARM)))
                && (ch->rider == NULL)))
            best = -1;

        if (skill_table[sn].flag1 == ADEPT || avatar_spell)
            best = -1;

        for (cnt = 0; cnt < MAX_CLASS; cnt++)
            if (skill_table[sn].skill_level[cnt] <= 80)
                break;

        /* all levels are over 80, this is either an imm-only skill/spell or something else that
         * mobs shouldn't be casting using do_cast() */
        if (cnt == MAX_CLASS)
            best = -1;
    }
    else
        for (cnt = 0; cnt < MAX_CLASS; cnt++) {
            if (((ch->lvl[cnt] >= skill_table[sn].skill_level[cnt]) && (skill_table[sn].flag1 == MORTAL)) && best < ch->lvl[cnt])
                best = ch->lvl[cnt];
            if (((ch->lvl2[cnt] >= skill_table[sn].skill_level[cnt]) && (skill_table[sn].flag1 == REMORT)) && best < ch->lvl2[cnt])
                best = ch->lvl[cnt];
        }

    if ((ch->adept_level > 0) && (skill_table[sn].flag1 == ADEPT))
        best = ch->adept_level * 4;
    if ((ch->adept_level == 20) && avatar_spell)
        best = 1;
    if ((skill_table[sn].flag1 == ADEPT) && (ch->adept_level < skill_table[sn].skill_level[0]))
        best = -1;
    if (IS_NPC(ch) && (skill_table[sn].flag1 == ADEPT))
        best = -1;
    if (IS_NPC(ch) && (skill_table[sn].flag1 == AVATAR))
        best = -1;

    if (best == 80)
        best = 79;
    if ((!IS_NPC(ch))
        && (is_name(skill_table[sn].name, race_table[ch->race].skill1)))
        best = 60;

    if (best == -1) {
        send_to_char("You can't do that.\n\r", ch);
        return;
    }

    /* if they use the default spell with no mastery, use their "best" learnt mastery */
    if (avatar_spell && skill_table[sn].flag2 == NORM && !IS_NPC(ch)) {
        if (ch->pcdata->learned[sn] >= AV_NOVICE && ch->pcdata->learned[sn] < AV_INTERMEDIATE && ch->pcdata->learned[sn + 1] > 0)
            sn += 1;
        else if (ch->pcdata->learned[sn] >= AV_INTERMEDIATE && ch->pcdata->learned[sn] < AV_ADVANCED && ch->pcdata->learned[sn + 2] > 0)
            sn += 2;
        else if (ch->pcdata->learned[sn] >= AV_ADVANCED && ch->pcdata->learned[sn] < AV_EXPERT && ch->pcdata->learned[sn + 3] > 0)
            sn += 3;
        else if (ch->pcdata->learned[sn] >= AV_EXPERT && ch->pcdata->learned[sn] < AV_MASTER && ch->pcdata->learned[sn + 4] > 0)
            sn += 4;
        else if (ch->pcdata->learned[sn] >= AV_MASTER && ch->pcdata->learned[sn + 5] > 0)
            sn += 5;
    }

    if (ch->position < skill_table[sn].minimum_position) {
        send_to_char("You can't concentrate enough.\n\r", ch);
        return;
    }

    mana = mana_cost(ch, sn);

    if (char_login)
        mana = 0;

    /*
     * Locate targets.
     */
    victim = NULL;
    obj = NULL;
    vo = NULL;

    switch (skill_table[sn].target) {
        default:
            bugf("Do_cast: bad target for sn %d.", sn);
            return;

        case TAR_IGNORE:
            break;

        case TAR_CHAR_OFFENSIVE:
            if (arg2[0] == '\0') {
                if ((victim = ch->fighting) == NULL) {
                    send_to_char("Cast the spell on whom?\n\r", ch);
                    return;
                }
            }
            else {
                if ((victim = get_char_room(ch, arg2)) == NULL) {
                    send_to_char("They aren't here.\n\r", ch);
                    return;
                }
            }

            if (IS_SET(victim->in_room->room_flags, ROOM_SAFE) && ch != victim && sn != gsn_charm_person && sn != gsn_hypnosis) {
                send_to_char("Not a chance!  This is a safe room.\n\r", ch);
                return;
            }

            if (IS_AFFECTED(ch, AFF_CHARM) && !IS_NPC(victim)) {
                check_social(ch, "sulk", "");
                return;
            }

            if (IS_DEBUGGER(ch))
                send_to_char("do_cast: you're attacking victim. doing check_killer\n\r", ch);

            if (IS_DEBUGGER(victim))
                send_to_char("do_cast: ch attacking you. doing check_killer\n\r", victim);

            if (ch && victim && !IS_NPC(ch)
                && !IS_NPC(victim)
                && is_safe(ch, victim, TRUE)
                )
                return;

            if (IS_NGR_CHARMIE(ch, victim))
                return;

            if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM) && ch->master && IS_NGR_CHARMIE(ch->master, victim) && !IS_IMMORTAL(ch->master))
                return;

            check_killer(ch, victim);

            /*      if ( !IS_NPC(ch) )
               {
               if ( !IS_NPC(victim) && ch != victim )
               {
               send_to_char( "You can't do that on a player.\n\r", ch );
               return;
               }

               if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
               {
               send_to_char( "You can't do that on your own follower.\n\r",
               ch );
               return;
               }
               }
             */
            vo = (void *) victim;
            break;

        case TAR_CHAR_DEFENSIVE:
            if (arg2[0] == '\0') {
                victim = ch;
            }
            else {
                if ((victim = get_char_room(ch, arg2)) == NULL) {
                    send_to_char("They aren't here.\n\r", ch);
                    return;
                }
            }

            vo = (void *) victim;
            break;

        case TAR_CHAR_SELF:
            if (arg2[0] != '\0' && !is_name(arg2, ch->name)) {
                send_to_char("You cannot cast this spell on another.\n\r", ch);
                return;
            }

            vo = (void *) ch;
            break;

        case TAR_OBJ_INV:
            if (arg2[0] == '\0') {
                send_to_char("What should the spell be cast upon?\n\r", ch);
                return;
            }

            if ((obj = get_obj_carry(ch, arg2)) == NULL) {
                send_to_char("You are not carrying that.\n\r", ch);
                return;
            }

            vo = (void *) obj;
            break;
    }

    switch (skill_table[sn].target) {
        default:
            break;
        case TAR_CHAR_OFFENSIVE:
        case TAR_CHAR_DEFENSIVE:

            if (ch && victim && victim->fighting && !IS_NPC(ch)
                && !IS_NPC(victim)
                && !IS_NPC(victim->fighting)
                && !can_group(ch, victim->fighting)
                && !ch->pcdata->in_arena
                ) {

                sprintf(buf, "%s doesn't seem to be affected. Maybe %s has something to do with it?\n\r", victim->name, victim->fighting->name);
                send_to_char(buf, ch);
                return;
            }

            if (ch && ch->master && victim && victim->fighting && IS_NPC(ch)
                && IS_AFFECTED(ch, AFF_CHARM)
                && !IS_NPC(victim)
                && !IS_NPC(victim->fighting)
                && !IS_NPC(ch->master)
                && !can_group(ch->master, victim->fighting)
                && !ch->master->pcdata->in_arena
                )
                return;

            break;
    }

    if (ch->mana < mana && skill_table[sn].flag1 != AVATAR) {
        send_to_char("You don't have enough mana.\n\r", ch);
        return;
    }
    else if (avatar_spell && ch->energy < mana) {
        send_to_char("You don't have enough energy.\n\r", ch);
        return;
    }

    if (avatar_spell && ch->pcdata->learned[sn] <= 0) {
        send_to_char("You haven't learned this spell.\n\r", ch);
        return;
    }

    if (str_cmp(skill_table[sn].name, "ventriloquate"))
        say_spell(ch, sn);

    WAIT_STATE(ch, skill_table[sn].beats);
    cast_chance = ((IS_NPC(ch) ? ch->level : ch->pcdata->learned[sn])
        + (int_app[get_curr_int(ch)].spell_mod));

    if (!IS_NPC(ch) && (skill_table[sn].flag2 == NORM)) {
        if (IS_SET(race_table[ch->race].race_flags, RACE_MOD_NO_MAGIC))
            cast_chance += -20;
        else if (IS_SET(race_table[ch->race].race_flags, RACE_MOD_WEAK_MAGIC))
            cast_chance += -10;
        else if (IS_SET(race_table[ch->race].race_flags, RACE_MOD_STRONG_MAGIC))
            cast_chance += 15;
    }
    if (cast_chance < 10)
        cast_chance += number_range(3, 7);

    if (!char_login && !IS_NPC(ch) && !avatar_spell
        && number_percent() > cast_chance) {
        bool                good_cast = FALSE;
        DUEL_DATA           *duel;

        if (!IS_NPC(ch)
            && (ch->stance == STANCE_CASTER)) {
            if (number_percent() > cast_chance) {
                good_cast = FALSE;
            }
            else {
                good_cast = TRUE;
            }
        }

        if ((duel = find_duel(ch)) && IS_SET(duel->flags, DUEL_SUPERCAST))
            good_cast = TRUE;

        if (!good_cast) {
            send_to_char("You lost your concentration.\n\r", ch);

            ch->mana -= mana / 2;
            return;
        }
    }

    else if ((skill_table[sn].target == TAR_CHAR_OFFENSIVE)
        && (IS_AFFECTED(victim, AFF_CLOAK_REFLECTION))
        && (ch != victim)
        && (number_percent() < (get_pseudo_level(victim) - 70))) {

        act(cloak_table[CLOAK_REFLECT].absorb_room,   victim, NULL, ch, TO_NOTVICT);
        act(cloak_table[CLOAK_REFLECT].absorb_self,   victim, NULL, ch, TO_CHAR);
        act(cloak_table[CLOAK_REFLECT].absorb_victim, ch, NULL, victim, TO_CHAR);
        reflected = TRUE;
    }

    else if ((skill_table[sn].target == TAR_CHAR_OFFENSIVE)
        && (IS_AFFECTED(victim, AFF_CLOAK_ABSORPTION))
        && (ch != victim)
        && (number_percent() < (get_pseudo_level(victim) - 55))) {
        victim->mana = UMIN(victim->max_mana, victim->mana + mana);

        act(cloak_table[CLOAK_ABSORB].absorb_room,   victim, NULL, ch, TO_NOTVICT);
        act(cloak_table[CLOAK_ABSORB].absorb_self,   victim, NULL, ch, TO_CHAR);
        act(cloak_table[CLOAK_ABSORB].absorb_victim, ch, NULL, victim, TO_CHAR);
    }

    if (!reflected) {
        if ((*skill_table[sn].spell_fun) (sn, best, ch, vo, NULL)) {
            if (!avatar_spell)
                ch->mana -= mana;    /* Only use mana if spell was called correctly */
            else {
                ch->energy -= mana;
                if (!IS_NPC(ch)) {
                    ch->pcdata->energy_used += mana;
                    energy_advance(ch);
                }
            }
        }
    }
    else {
        if ((*skill_table[sn].spell_fun) (sn, best, ch, ch, NULL)) {
            if (!avatar_spell)
                ch->mana -= mana;    /* Only use mana if spell was called correctly */
            else {
                ch->energy -= mana;
                if (!IS_NPC(ch)) {
                    ch->pcdata->energy_used += mana;
                    energy_advance(ch);
                }
            }
        }
        if (ch->is_free != FALSE)
            return;
    }

    if ((skill_table[sn].target == TAR_CHAR_OFFENSIVE)
        && (ch != NULL)
        && (victim != NULL)
        && (ch != victim)
        && (ch->is_free == FALSE)
        && (victim->is_free == FALSE)) {
        CHAR_DATA          *vch;
        CHAR_DATA          *vch_next;

        if (ch->in_room != NULL) {
            for (vch = ch->in_room->first_person; vch; vch = vch_next) {
                vch_next = vch->next_in_room;
                if ((vch == victim)
                    && (vch->fighting == NULL)
                    && (vch->master != ch)
                    && (vch != ch)) {
                    if (is_safe(ch, victim, TRUE))
                        continue;

                    if (IS_DEBUGGER(ch))
                        send_to_char("do_cast: you are hitting victim.\n\r", ch);

                    if (IS_DEBUGGER(victim))
                        send_to_char("do_cast: ch is hitting you.\n\r", victim);

                    multi_hit(ch, victim, TYPE_UNDEFINED);
                    break;
                }
            }
        }
    }
    return;
}

/*
 * Cast spells at targets using a magical object.
 */
void
obj_cast_spell(int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj)
{
    void               *vo;
    char                buf[MAX_STRING_LENGTH];

    if (sn <= 0)
        return;

    if (sn >= MAX_SKILL || skill_table[sn].spell_fun == 0) {
        bugf("Obj_cast_spell: bad sn %d.", sn);
        return;
    }

    if (!legal_spell(ch, sn))
        return;

    switch (skill_table[sn].target) {
        default:
            bugf("Obj_cast_spell: bad target for sn %d.", sn);
            return;

        case TAR_IGNORE:
            vo = NULL;
            break;

        case TAR_CHAR_OFFENSIVE:
            if (victim == NULL)
                if (ch != NULL)
                    victim = ch->fighting;
            if (victim == NULL) {
                if (ch != NULL)
                    send_to_char("You can't do that.\n\r", ch);
                return;
            }

            if (ch && victim && !IS_NPC(ch)
                && !IS_NPC(victim)
                && is_safe(ch, victim, FALSE)
                )
                return;

            if (ch && ch->master && victim && IS_NPC(ch)
                && !IS_NPC(ch->master)
                && !IS_NPC(victim)
                && is_safe(ch->master, victim, FALSE)
                )
                return;

            vo = (void *) victim;
            break;

        case TAR_CHAR_DEFENSIVE:
            if (victim == NULL)
                if (ch != NULL)
                    victim = ch;
            if (victim != NULL)
                vo = (void *) victim;
            else
                return;
            break;

        case TAR_CHAR_SELF:
            if (ch != NULL)
                vo = (void *) ch;
            else
                return;
            break;

        case TAR_OBJ_INV:
            if (obj == NULL) {
                if (ch != NULL)
                    send_to_char("You can't do that.\n\r", ch);
                return;
            }
            vo = (void *) obj;
            break;
    }

    switch (skill_table[sn].target) {
        default:
            break;
        case TAR_CHAR_OFFENSIVE:
        case TAR_CHAR_DEFENSIVE:

            if (ch && victim && victim->fighting && !IS_NPC(ch)
                && !IS_NPC(victim)
                && !IS_NPC(victim->fighting)
                && !IS_SET(victim->fighting->act, PLR_KILLER)
                && !IS_SET(victim->fighting->act, PLR_THIEF)
                && !IS_SET(victim->fighting->pcdata->pflags, PFLAG_PKOK)
                && !can_group(ch, victim->fighting)
                && !victim->pcdata->in_arena
                ) {

                sprintf(buf, "%s doesn't seem to be affected. Maybe %s has something to do with it?\n\r", victim->short_descr, victim->fighting->short_descr);
                send_to_char(buf, ch);
                return;
            }

            if (ch && ch->master && victim && victim->fighting && IS_NPC(ch)
                && IS_AFFECTED(ch, AFF_CHARM)
                && !IS_NPC(victim)
                && !IS_NPC(victim->fighting)
                && !IS_NPC(ch->master)
                && !IS_SET(victim->fighting->act, PLR_KILLER)
                && !IS_SET(victim->fighting->act, PLR_THIEF)
                && !IS_SET(victim->fighting->pcdata->pflags, PFLAG_PKOK)
                && !can_group(ch->master, victim->fighting)
                && !ch->master->pcdata->in_arena
                )
                return;

            break;
    }

    target_name = "";

    (*skill_table[sn].spell_fun) (sn, level, ch, vo, obj);

    if (skill_table[sn].target == TAR_CHAR_OFFENSIVE && (victim != NULL)
        && victim->master != ch) {
        CHAR_DATA          *vch;
        CHAR_DATA          *vch_next;

        if (ch->in_room) {
            for (vch = ch->in_room->first_person; vch; vch = vch_next) {
                vch_next = vch->next_in_room;
                if (victim == vch && victim->fighting == NULL) {
                    multi_hit(ch, victim, TYPE_UNDEFINED);    /* SRZ swapped ch& v */
                    break;
                }
            }
        }
    }

    return;
}

/* Spell functions. */

bool
spell_acid_blast(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    int                 dam;

    dam = number_range(get_pseudo_level(ch), get_pseudo_level(ch) * 3);
    if (saves_spell(level, victim))
        dam /= 2;

/*
    act("$N is struck by $n's blast of acid!!", ch, NULL, victim, TO_NOTVICT);
    act("You are struck by $N's blast of acid!!", victim, NULL, ch, TO_CHAR);
    act("Your blast of acid strikes $N!!", ch, NULL, victim, TO_CHAR);
*/

    damage(ch, victim, dam, sn);
    return TRUE;
}

bool
spell_armor(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (is_affected(victim, sn))
        return FALSE;
    af.type = sn;
    af.duration = 24;
    af.modifier = -(get_pseudo_level(ch) / 4);
    af.location = APPLY_AC;
    af.bitvector = 0;
    af.save = TRUE;
    affect_to_char(victim, &af);
    send_to_char("You feel someone protecting you.\n\r", victim);
    if (ch != victim)
        send_to_char("Ok.\n\r", ch);
    return TRUE;
}

bool
spell_badbreath(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)

/* --Stephen */
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
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
    dam = number_range(dam_each[level] / 2, dam_each[level] * 2) + (level / 4);
    if (saves_spell(level, victim)) {
        dam /= 2;
    }

    damage(ch, victim, dam, sn);
    return TRUE;
}

bool
spell_bark_skin(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (is_affected(ch, sn))
        return FALSE;

    af.type = sn;
    af.duration = 3 + (level / 12);
    af.location = APPLY_AC;
    af.modifier = -(get_pseudo_level(ch) / 4);
    af.bitvector = 0;
    af.save = TRUE;
    affect_to_char(victim, &af);
    act("$n's skin turns to bark.", victim, NULL, NULL, TO_ROOM);
    send_to_char("Your skin turns to bark.\n\r", victim);
    return TRUE;
}

bool
spell_bless(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (victim->position == POS_FIGHTING || is_affected(victim, sn))
        return (ch == victim ? FALSE : TRUE);
    af.type = sn;
    af.duration = 6 + (level / 6);
    af.location = APPLY_HITROLL;
    af.modifier = get_pseudo_level(ch) / 8;
    af.bitvector = 0;
    af.save = TRUE;
    affect_to_char(victim, &af);

    af.location = APPLY_SAVING_SPELL;
    af.modifier = 0 - get_pseudo_level(ch) / 8;
    affect_to_char(victim, &af);
    send_to_char("You feel righteous.\n\r", victim);
    if (ch != victim)
        send_to_char("Ok.\n\r", ch);
    return TRUE;
}

bool
spell_blindness(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (IS_AFFECTED(victim, AFF_BLIND) || saves_spell(level, victim))
        return TRUE;

    af.type = sn;
    af.location = APPLY_HITROLL;
    af.modifier = -4;
    af.duration = 1 + (level / 4);
    af.bitvector = AFF_BLIND;
    af.save = TRUE;
    affect_to_char(victim, &af);
    send_to_char("You are blinded!\n\r", victim);
    if (ch != victim)
        send_to_char("Ok.\n\r", ch);
    return TRUE;
}

bool
spell_burning_hands(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = {
        0,
        0, 0, 0, 0, 14, 17, 20, 23, 26, 29,
        29, 29, 30, 30, 31, 31, 32, 32, 33, 33,
        34, 34, 35, 35, 36, 36, 37, 37, 38, 38,
        39, 39, 40, 40, 41, 41, 42, 42, 43, 43,
        44, 44, 45, 45, 46, 46, 47, 47, 48, 48
    };
    int                 dam;

    level = UMIN(level, sizeof(dam_each) / sizeof(dam_each[0]) - 1);
    level = UMAX(0, level);
    dam = number_range(dam_each[level] / 2, dam_each[level] * 2);
    if (saves_spell(level, victim))
        dam /= 2;
    if (obj == NULL) {
        act("A jet of flame from $n's hand engulfs $N!", ch, NULL, victim, TO_NOTVICT);
        act("A jet of flame form $N's hand engulfs you!", victim, NULL, ch, TO_CHAR);
        act("$N is engulfed by a jet of flame from your hand!", ch, NULL, victim, TO_CHAR);
    }
    else {
        act("A jet of flame from $p engulfs $n!", victim, obj, NULL, TO_ROOM);
        act("A jet of flame from $p engulfs you!", victim, obj, NULL, TO_ROOM);
    }
    damage(ch, victim, dam, -1);
    return TRUE;
}

bool
spell_call_lightning(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *vch;
    CHAR_DATA          *vch_next;
    int                 dam;

    if (!IS_OUTSIDE(ch)) {
        send_to_char("You must be out of doors.\n\r", ch);
        return FALSE;
    }

    if (weather_info.sky < SKY_RAINING) {
        send_to_char("You need bad weather.\n\r", ch);
        return FALSE;
    }

    dam = dice(get_pseudo_level(ch) / 2, level / 2);

    if (obj == NULL) {
        act("$n calls God's lightning to strike $s foes!", ch, NULL, NULL, TO_ROOM);
        send_to_char("God's lightning strikes your foes!\n\r", ch);
    }
    else {
        act("$p summons lightning to strike $n's foes!", ch, obj, NULL, TO_ROOM);
        act("$p summons lightning to strike your foes!", ch, obj, NULL, TO_CHAR);
    }
    CREF(vch_next, CHAR_NEXTROOM);
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
                act("A bolt of lightning strikes down onto $N!", ch, NULL, vch, TO_ROOM);
                send_to_char("A bolt of lightning strikes down onto you!\n\r", vch);
                damage(ch, vch, saves_spell(level, vch) ? dam / 2 : dam, -1);
            }
            continue;
        }

        if (vch->in_room->area == ch->in_room->area && IS_OUTSIDE(vch)
            && IS_AWAKE(vch))
            send_to_char("Lightning flashes in the sky.\n\r", vch);
    }
    CUREF(vch_next);
    return TRUE;
}

bool
spell_cause_light(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    damage(ch, (CHAR_DATA *) vo, dice(1, 8) + level / 3, sn);
    return TRUE;
}

bool
spell_cause_critical(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    damage(ch, (CHAR_DATA *) vo, dice(3, 8) + level - 6, sn);
    return TRUE;
}

bool
spell_cause_serious(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    damage(ch, (CHAR_DATA *) vo, dice(2, 8) + level / 2, sn);
    return TRUE;
}

bool
spell_change_sex(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (is_affected(victim, sn))
        return FALSE;
    af.type = sn;
    af.duration = 5 + (level / 10);
    af.location = APPLY_SEX;
    do {
        af.modifier = number_range(0, 2) - victim->sex;
    }
    while (af.modifier == 0);
    af.bitvector = 0;
    af.save = TRUE;
    affect_to_char(victim, &af);
    send_to_char("You feel different.\n\r", victim);
    if (ch != victim)
        send_to_char("Ok.\n\r", ch);
    return TRUE;
}

bool
spell_charm_person(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
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
    af.type = sn;

    if (ch->adept_level > 0)
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
spell_chill_touch(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = {
        0,
        0, 0, 6, 7, 8, 9, 12, 13, 13, 13,
        14, 14, 14, 15, 15, 15, 16, 16, 16, 17,
        17, 17, 18, 18, 18, 19, 19, 19, 20, 20,
        20, 21, 21, 21, 22, 22, 22, 23, 23, 23,
        24, 24, 24, 25, 25, 25, 26, 26, 26, 27
    };
    AFFECT_DATA         af;
    int                 dam;

    level = UMIN(level, sizeof(dam_each) / sizeof(dam_each[0]) - 1);
    level = UMAX(0, level);
    dam = number_range(dam_each[level] / 2, dam_each[level] * 2);
    if (!saves_spell(level, victim)) {
        af.type = sn;
        af.duration = 3;
        af.location = APPLY_STR;
        af.modifier = -get_pseudo_level(ch) / 16;
        af.bitvector = AFF_POISON;
        af.save = TRUE;
        affect_join(victim, &af);
    }
    else {
        dam /= 2;
    }
    if (obj == NULL) {
        act("Blue energy shoots from $n's hand, striking $N!", ch, NULL, victim, TO_NOTVICT);
        act("Blue energy shoots from your hand, and strikes $N!", ch, NULL, victim, TO_CHAR);
        act("Blue energy shoots from $N's hand, and strikes you!", victim, NULL, ch, TO_CHAR);
    }
    else {
        act("Blue energy shoots from $p, and strikes $n!", ch, obj, NULL, TO_ROOM);
        act("Blue energy shoots from $p, and strikes you!", ch, obj, NULL, TO_ROOM);
    }
    damage(ch, victim, dam, -1);
    return TRUE;
}

bool
spell_colour_spray(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = {
        0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        30, 35, 40, 45, 50, 55, 55, 55, 56, 57,
        58, 58, 59, 60, 61, 61, 62, 63, 64, 64,
        65, 66, 67, 67, 68, 69, 70, 70, 71, 72,
        73, 73, 74, 75, 76, 76, 77, 78, 79, 79
    };
    int                 dam;

    level = UMIN(level, sizeof(dam_each) / sizeof(dam_each[0]) - 1);
    level = UMAX(0, level);
    dam = number_range(dam_each[level] / 2, dam_each[level] * 2);
    if (saves_spell(level, victim))
        dam /= 2;
    if (obj == NULL) {
        act("A cone of vivid colours springs forth from $n's hand.", ch, NULL, NULL, TO_ROOM);
        send_to_char("A cone of vivid colours springs forth from your hand\n\r.", ch);
        act("$n's colour spray hits $N in the face!", ch, NULL, victim, TO_NOTVICT);
        act("$N's colour spray hits you in the face!", victim, NULL, ch, TO_CHAR);
        act("Your colour spray hits $N in the face!", ch, NULL, victim, TO_CHAR);
    }
    else {
        act("A cone of vivid colours springs from $p.", ch, obj, NULL, TO_ROOM);
        act("A cone of vivid colours spring from $p.", ch, obj, NULL, TO_CHAR);
        act("The colour spray strikes $n in the face!", victim, NULL, NULL, TO_ROOM);
        act("You are struck in the face by the colour spray!", victim, NULL, NULL, TO_CHAR);
    }
    damage(ch, victim, dam, -1);
    return TRUE;
}

bool
spell_continual_light(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    OBJ_DATA           *light;

    light = create_object(get_obj_index(OBJ_VNUM_LIGHT_BALL), 0);
    obj_to_room(light, ch->in_room);
    if (obj == NULL) {
        act("$n twiddles $s thumbs and $p appears.", ch, light, NULL, TO_ROOM);
        act("You twiddle your thumbs and $p appears.", ch, light, NULL, TO_CHAR);
    }
    else
        act("$p suddenly appears before you!", NULL, light, NULL, TO_ROOM);
    return TRUE;
}

bool
spell_control_weather(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    if (!str_cmp(target_name, "better"))
        weather_info.change += dice(level / 3, 4);
    else if (!str_cmp(target_name, "worse"))
        weather_info.change -= dice(level / 3, 4);
    else {
        send_to_char("Do you want it to get better or worse?\n\r", ch);
        return FALSE;
    }
    send_to_char("Ok.\n\r", ch);
    return TRUE;
}

bool
spell_create_food(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    OBJ_DATA           *mushroom;
    int                 food_vnums[MAX_RACE] = { 20123, 20124, 20125, 20126, 20127, 20128, 20129, 20130,
        20131, 20132, 20133, 20134, 20135, 20136
    };

    if (!IS_NPC(ch))
        mushroom = create_object(get_obj_index(food_vnums[ch->race]), 0);
    else if (IS_AFFECTED(ch, AFF_CHARM) && ch->master)
        mushroom = create_object(get_obj_index(food_vnums[ch->master->race]), 0);
    else
        mushroom = create_object(get_obj_index(food_vnums[0]), 0);

    if (mushroom) {
        mushroom->value[0] = 5 + level;

        if (ch->carry_number + get_obj_number(mushroom) > can_carry_n(ch)) {
            send_to_char("You have your hands full.\n\r", ch);
            extract_obj(mushroom);
            return FALSE;
        }
        
        if (ch->carry_weight + get_obj_weight(mushroom) > can_carry_w(ch)) {
            send_to_char("You can't carry that much weight.", ch);
            extract_obj(mushroom);
            return FALSE;
        }

        obj_to_char(mushroom, ch);
        act("$p suddenly appears in your inventory.", ch, mushroom, NULL, TO_CHAR);
    }

    return TRUE;
}

bool
spell_create_spring(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    OBJ_DATA           *spring;

    if (ch->in_room == NULL)
        return FALSE;

    for (spring = ch->in_room->first_content; spring != NULL; spring = spring->next_in_room)
        if (spring->pIndexData && spring->pIndexData->vnum == OBJ_VNUM_SPRING)
            break;

    if (spring)
        return FALSE;

    spring = create_object(get_obj_index(OBJ_VNUM_SPRING), 0);
    spring->timer = 5;
    obj_to_room(spring, ch->in_room);
    act("$p flows from the ground.", ch, spring, NULL, TO_CHAR);
    act("$p flows from the ground.", ch, spring, NULL, TO_ROOM);
    return TRUE;
}

bool
spell_create_water(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    OBJ_DATA           *ob = (OBJ_DATA *) vo;
    int                 water;

    if (ob->item_type != ITEM_DRINK_CON) {
        send_to_char("It is unable to hold water.\n\r", ch);
        return FALSE;
    }

    if (ob->value[2] != LIQ_WATER && ob->value[1] != 0) {
        send_to_char("It contains some other liquid.\n\r", ch);
        return FALSE;
    }

    water = UMIN(level * (weather_info.sky >= SKY_RAINING ? 4 : 2), ob->value[0] - ob->value[1]
        );

    if (water > 0) {
        ob->value[2] = LIQ_WATER;
        ob->value[1] += water;
        if (!is_name("water", ob->name)) {
            char                buf[MAX_STRING_LENGTH];

            sprintf(buf, "%s water", ob->name);
            free_string(ob->name);
            ob->name = str_dup(buf);
        }
        act("$p is filled.", ch, ob, NULL, TO_CHAR);
    }

    return TRUE;
}

bool
spell_cure_blindness(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;

    if (!is_affected(victim, gsn_blindness))
        return (ch == victim ? FALSE : TRUE);
    affect_strip(victim, gsn_blindness);
    send_to_char("Your vision returns!\n\r", victim);
    if (ch != victim)
        send_to_char("Ok.\n\r", ch);
    return TRUE;
}

bool
spell_cure_critical(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    DUEL_DATA          *duel;
    int                 heal;

    heal = UMIN(150, dice(12, 8) + level);
    if (!IS_NPC(ch) && (duel = find_duel(ch)) && IS_SET(duel->flags, DUEL_DBLHEAL))
        heal *= 2;

    victim->hit = UMIN(victim->hit + heal, victim->max_hit);
    update_pos(victim);
    send_to_char("You feel better!\n\r", victim);
    if (ch != victim)
        send_to_char("Ok.\n\r", ch);
    return TRUE;
}

bool
spell_cure_light(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    int                 heal;
    DUEL_DATA          *duel;

    heal = UMIN(50, dice(5, 8) + level);

    if (!IS_NPC(ch) && (duel = find_duel(ch)) && IS_SET(duel->flags, DUEL_DBLHEAL))
        heal *= 2;

    victim->hit = UMIN(victim->hit + heal, victim->max_hit);
    update_pos(victim);
    send_to_char("You feel better!\n\r", victim);
    if (ch != victim)
        send_to_char("Ok.\n\r", ch);
    return TRUE;
}

bool
spell_cure_poison(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;

    if (is_affected(victim, gsn_poison)) {
        affect_strip(victim, gsn_poison);
        act("$N looks better.", ch, NULL, victim, TO_NOTVICT);
        send_to_char("A warm feeling runs through your body.\n\r", victim);
        send_to_char("Ok.\n\r", ch);
    }
    return TRUE;
}

bool
spell_cure_serious(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    int                 heal;
    DUEL_DATA          *duel;

    heal = UMIN(100, dice(6, 8) + level);

    if (!IS_NPC(ch) && (duel = find_duel(ch)) && IS_SET(duel->flags, DUEL_DBLHEAL))
        heal *= 2;

    victim->hit = UMIN(victim->hit + heal, victim->max_hit);

    update_pos(victim);
    send_to_char("You feel better!\n\r", victim);
    if (ch != victim)
        send_to_char("Ok.\n\r", ch);
    return TRUE;
}

bool
spell_curse(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (victim->level < 20)
        return FALSE;

    if (IS_AFFECTED(victim, AFF_CURSE) || saves_spell(level, victim) || victim->race == RACE_LAM)
        return TRUE;
    af.type = sn;
    af.duration = 2 * (level / 8);
    af.location = APPLY_HITROLL;
    af.modifier = -get_pseudo_level(ch) / 8;
    af.bitvector = AFF_CURSE;
    af.save = TRUE;
    affect_to_char(victim, &af);

    af.location = APPLY_SAVING_SPELL;
    af.modifier = 1;
    affect_to_char(victim, &af);

    send_to_char("You feel unclean.\n\r", victim);
    if (ch != victim)
        send_to_char("Ok.\n\r", ch);
    return TRUE;
}

bool
spell_detect_evil(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
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
    if (ch != victim)
        send_to_char("Ok.\n\r", ch);
    return TRUE;
}

bool
spell_detect_hidden(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (IS_AFFECTED(victim, AFF_DETECT_HIDDEN))
        return FALSE;
    af.type = sn;
    af.duration = 4 + (level / 8);
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_DETECT_HIDDEN;
    af.save = TRUE;
    affect_to_char(victim, &af);
    send_to_char("Your awareness improves.\n\r", victim);
    if (ch != victim)
        send_to_char("Ok.\n\r", ch);
    return TRUE;
}

bool
spell_detect_invis(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (IS_AFFECTED(victim, AFF_DETECT_INVIS))
        return FALSE;
    af.type = sn;
    af.duration = 6 + (level / 8);
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_DETECT_INVIS;
    af.save = TRUE;
    affect_to_char(victim, &af);
    send_to_char("Your eyes tingle.\n\r", victim);
    if (ch != victim)
        send_to_char("Ok.\n\r", ch);
    return TRUE;
}

bool
spell_detect_magic(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (IS_AFFECTED(victim, AFF_DETECT_MAGIC) || item_has_apply(victim, ITEM_APPLY_DET_MAG))
        return FALSE;
    af.type = sn;
    af.duration = 6 + (level / 4);
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_DETECT_MAGIC;
    af.save = TRUE;
    affect_to_char(victim, &af);
    send_to_char("Your eyes tingle.\n\r", victim);
    if (ch != victim)
        send_to_char("Ok.\n\r", ch);
    return TRUE;
}

bool
spell_detect_undead(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (IS_AFFECTED(victim, AFF_DETECT_UNDEAD) || item_has_apply(victim, ITEM_APPLY_DET_UNDEAD))
        return FALSE;

    af.type = sn;
    af.duration = 5 + (level / 6);
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_DETECT_UNDEAD;
    af.save = TRUE;
    affect_to_char(victim, &af);
    send_to_char("Your eyes tingle.\n\r", victim);
    if (ch != victim)
        send_to_char("Ok.\n\r", ch);
    return TRUE;
}

bool
spell_detect_poison(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    OBJ_DATA           *ob = (OBJ_DATA *) vo;

    if (ob->item_type == ITEM_DRINK_CON || ob->item_type == ITEM_FOOD) {
        if (ob->value[3] != 0)
            send_to_char("You smell poisonous fumes.\n\r", ch);
        else
            send_to_char("It looks very delicious.\n\r", ch);
    }
    else {
        send_to_char("It doesn't look poisoned.\n\r", ch);
    }

    return TRUE;
}

bool
spell_dispel_magic(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{

    /* Remove certain affects from victim.  Chance will be 100% if
     * ch == victim.  Otherwise, variable chance of success, depending
     * on if a) victim is NPC b) Difference in levels.
     * Rewritten yet AGAIN to check chance for each dispel... also works
     * on objects as well :)
     * Stephen
     */
    CHAR_DATA          *vch;
    CHAR_DATA          *vch_next;
    CHAR_DATA          *victim;
    AFFECT_DATA        *paf;
    AFFECT_DATA        *paf_next;
    OBJ_DATA           *ob;
    int                 chance;
    char                buf[MSL];

    if (target_name[0] == '\0' && ch->fighting == NULL) {
        send_to_char("Dispel who or what??\n\r", ch);

        /* Couldn't find a target, get rid of their wait state */
        ch->wait = 0;
        return FALSE;
    }

    if (target_name[0] == '\0' && ch->fighting != NULL)
        target_name = "enemy";

    if ((victim = get_char_room(ch, target_name)) != NULL) {

        if (ch && victim && !IS_NPC(ch)
            && !IS_NPC(victim)
            && ch != victim
            && is_safe(ch, victim, TRUE)
            ) {
            send_to_char("A Mystical Being intervenes and denies your request!\n\r", ch);
            return FALSE;
        }

        if (IS_NGR_CHARMIE(ch, victim)) {
            send_to_char("You can't dispel this charmed mobile, as it belongs to someone you can't group with.\n\r", ch);
            return FALSE;
        }

        if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM) && ch->master && IS_NGR_CHARMIE(ch->master, victim))
            return FALSE;

        /* an attempt to remove "o all cast 'dispel m' player" mountkilling */
        if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM)
            && !IS_NPC(victim) && (victim->fighting == NULL))
            return FALSE;

        if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM)
            && !IS_NPC(victim) && victim->race == RACE_TTN)
            return FALSE;

        if (IS_NPC(victim))
            chance = 100;
        else
            chance = 75;

        if (ch == victim)
            chance = 100;

        if (IS_DEBUGGER(ch)) {
            sprintf(buf, "[1] Chance[%d] Level[%d]\n\r", chance, level);
            send_to_char(buf, ch);
        }

        if (!IS_NPC(ch) && ch->race == RACE_GNO)
            level += 20;

        if (!IS_NPC(ch) && !IS_NPC(victim) && ch->race == RACE_GNO)
            chance += 50;

        if (ch->race != RACE_GNO || (ch->race == RACE_GNO && level > victim->level))
            chance += ((level - victim->level) * 2);

        if (IS_DEBUGGER(ch)) {
            sprintf(buf, "[2] Chance[%d] Level[%d]\n\r", chance, level);
            send_to_char(buf, ch);
        }

        /* Bonus/penalty for difference in levels. */
        if (obj == NULL) {
            if (ch == victim) {
                send_to_char("You gesture towards yourself.\n\r", ch);
                act("$n gestures towards $mself.", ch, NULL, NULL, TO_ROOM);
            }
            else {
                act("You gesture towards $N.", ch, NULL, victim, TO_CHAR);
                act("$n gestures towards $N.", ch, NULL, victim, TO_NOTVICT);
                act("$N gestures towards you.", victim, NULL, ch, TO_CHAR);
            }
        }
        else {
            act("$p glows brightly at $n.", victim, obj, NULL, TO_ROOM);
            act("$p glows brightly towards you.", victim, obj, NULL, TO_CHAR);
        }
        /* So now, player should have 'rolled' less than chance, so check
         * and remove affects, with some messages too.
         */

        if ((IS_AFFECTED(victim, AFF_CLOAK_REFLECTION))
            && (ch != victim)
            && ((ch->race != RACE_GNO && number_percent() < (victim->level - 50))
                || (!IS_NPC(ch) && ch->race == RACE_GNO && 50 < number_percent())
            )
            ) {

            act(cloak_table[CLOAK_REFLECT].absorb_room,   victim, NULL, ch, TO_NOTVICT);
            act(cloak_table[CLOAK_REFLECT].absorb_self,   victim, NULL, ch, TO_CHAR);
            act(cloak_table[CLOAK_REFLECT].absorb_victim, ch, NULL, victim, TO_CHAR);

            CREF(vch_next, CHAR_NEXTROOM);
            for (vch = ch->in_room->first_person; vch; vch = vch_next) {
                vch_next = vch->next_in_room;
                if (victim == vch && victim->fighting == NULL) {
                    check_killer(ch, victim);
                    multi_hit(ch, victim, TYPE_UNDEFINED);
                    break;
                }
            }
            CUREF(vch_next);

            return TRUE;
        }

        if ((IS_AFFECTED(victim, AFF_CLOAK_ABSORPTION))
            && (ch != victim)
            && ((ch->race != RACE_GNO && number_percent() < (victim->level - 40))
                || (!IS_NPC(ch) && ch->race == RACE_GNO && 50 < number_percent())
            )
            ) {

            act(cloak_table[CLOAK_ABSORB].absorb_room,   victim, NULL, ch, TO_NOTVICT);
            act(cloak_table[CLOAK_ABSORB].absorb_self,   victim, NULL, ch, TO_CHAR);
            act(cloak_table[CLOAK_ABSORB].absorb_victim, ch, NULL, victim, TO_CHAR);
            CREF(vch_next, CHAR_NEXTROOM);

            for (vch = ch->in_room->first_person; vch; vch = vch_next) {
                vch_next = vch->next_in_room;
                if (victim == vch && victim->fighting == NULL) {
                    check_killer(ch, victim);
                    multi_hit(ch, victim, TYPE_UNDEFINED);
                    break;
                }
            }
            CUREF(vch_next);

            return TRUE;
        }

        /* NODISPEL flagged mobs */
        if ((IS_NPC(victim)) && (IS_SET(victim->act, ACT_NODISPEL))) {
            return TRUE;
        }

        for (paf = victim->first_affect; paf != NULL; paf = paf_next) {
            paf_next = paf->next;

            if (paf->type == gsn_emount)
                continue;

            if (paf->type == skill_lookup("reserved"))
                continue;

            if (skill_table[paf->type].flag1 == AVATAR && ch != victim)
                continue;

            if (IS_DEBUGGER(ch)) {
                sprintf(buf, "[3] Chance[%d] Level[%d]\n\r", chance, level);
                send_to_char(buf, ch);
            }

            if (number_percent() < chance) {
                if (paf->type == gsn_charm_person && victim->master) {
                    stop_follower(victim);
                    chance = (2 * chance) / 3;
                    continue;
                }

                affect_remove(victim, paf);

                if (IS_CLOAK_SN(paf->type)) {
                    int sn = GET_INDEX_CLOAK(paf->type);

                    act(cloak_table[sn].remove_self, victim, NULL, NULL, TO_CHAR);
                    act(cloak_table[sn].remove_room, victim, NULL, NULL, TO_ROOM);
                }
                else {
                    if (paf->type > 0 && skill_table[paf->type].msg_off) {
                        send_to_char(skill_table[paf->type].msg_off, victim);
                        send_to_char("\n\r", victim);
                    }

                    if (paf->type == gsn_change_sex && paf->location == APPLY_SEX && skill_table[paf->type].room_off) {
                        victim->sex -= paf->modifier;
                        act(skill_table[paf->type].room_off, victim, NULL, NULL, TO_ROOM);
                        victim->sex += paf->modifier;
                    }
                    else if (paf->type > 0 && skill_table[paf->type].room_off)
                        act(skill_table[paf->type].room_off, victim, NULL, NULL, TO_ROOM);
                }

                chance = (2 * chance) / 3;
                if (IS_DEBUGGER(ch)) {
                    sprintf(buf, "[4] Chance[%d] Level[%d]\n\r", chance, level);
                    send_to_char(buf, ch);
                }
            }
            else
                break;
        }

        if (IS_NPC(victim)) {
            if (IS_AFFECTED(victim, AFF_CLOAK_ABSORPTION) && (number_percent() < chance)) {
                chance = (chance) / 3;
                if (IS_DEBUGGER(ch)) {
                    sprintf(buf, "[5] Chance[%d] Level[%d]\n\r", chance, level);
                    send_to_char(buf, ch);
                }

                act(cloak_table[CLOAK_ABSORB].remove_self, victim, NULL, NULL, TO_CHAR);
                act(cloak_table[CLOAK_ABSORB].remove_room, victim, NULL, NULL, TO_ROOM);
                REMOVE_BIT(victim->affected_by, AFF_CLOAK_ABSORPTION);
            }

            if (IS_AFFECTED(victim, AFF_CLOAK_REFLECTION) && (number_percent() < chance)) {
                chance = (chance) / 3;
                if (IS_DEBUGGER(ch)) {
                    sprintf(buf, "[6] Chance[%d] Level[%d]\n\r", chance, level);
                    send_to_char(buf, ch);
                }

                act(cloak_table[CLOAK_REFLECT].remove_self, victim, NULL, NULL, TO_CHAR);
                act(cloak_table[CLOAK_REFLECT].remove_room, victim, NULL, NULL, TO_ROOM);
                REMOVE_BIT(victim->affected_by, AFF_CLOAK_REFLECTION);
            }

            if (IS_AFFECTED(victim, AFF_SANCTUARY) && (number_percent() < chance)) {
                chance = (2 * chance) / 3;
                act("The white aura around $n fades.", victim, NULL, NULL, TO_ROOM);
                send_to_char("The white aura around you fades.\n\r", victim);
                REMOVE_BIT(victim->affected_by, AFF_SANCTUARY);
            }
            if (IS_AFFECTED(victim, AFF_FLYING) && (number_percent() < chance)) {
                act("$n suddenly drops to the ground!", victim, NULL, NULL, TO_ROOM);
                send_to_char("You suddenly drop to the ground!\n\r", victim);
                REMOVE_BIT(victim->affected_by, AFF_FLYING);
            }
            if (IS_AFFECTED(victim, AFF_INVISIBLE) && (number_percent() < chance)) {
                act("$n flickers, and becomes visible.", victim, NULL, NULL, TO_ROOM);
                send_to_char("You flicker, and become visible.\n\r", victim);
                REMOVE_BIT(victim->affected_by, AFF_INVISIBLE);
            }
            if (IS_AFFECTED(victim, AFF_FAERIE_FIRE) && (number_percent() < chance)) {
                act("The pink glow around $n suddenly fades.", victim, NULL, NULL, TO_ROOM);
                send_to_char("The pink glow around you suddenly fades.\n\r", victim);
                REMOVE_BIT(victim->affected_by, AFF_FAERIE_FIRE);
            }
            if (IS_AFFECTED(victim, AFF_DETECT_INVIS) && (number_percent() < chance)) {
                act("$n's eyes briefly shimmer black.", victim, NULL, NULL, TO_ROOM);
                REMOVE_BIT(victim->affected_by, AFF_DETECT_INVIS);
            }

            if (IS_AFFECTED(victim, AFF_DETECT_EVIL) && (number_percent() < chance))
                REMOVE_BIT(victim->affected_by, AFF_DETECT_EVIL);

            if (IS_AFFECTED(victim, AFF_DETECT_MAGIC) && (number_percent() < chance))
                REMOVE_BIT(victim->affected_by, AFF_DETECT_MAGIC);

            if (IS_AFFECTED(victim, AFF_DETECT_HIDDEN) && (number_percent() < chance)) {
                act("$n's eyes briefly shimmer white.", victim, NULL, NULL, TO_ROOM);
                REMOVE_BIT(victim->affected_by, AFF_DETECT_HIDDEN);
            }

            /*   if ( IS_AFFECTED( victim, AFF_INFRARED ) )
               {
               act( "The red glow in $n's eyes fades quickly.", victim, NULL, NULL, TO_ROOM );
               send_to_char( "The red glow in your eyes fade.\n\r", victim );
               REMOVE_BIT( victim->affected_by, AFF_INFRARED );
               }

               if ( IS_AFFECTED( victim, AFF_SNEAK ) )
               REMOVE_BIT( victim->affected_by, AFF_SNEAK );

               if ( IS_AFFECTED( victim, AFF_HIDE ) )
               REMOVE_BIT( victim->affected_by, AFF_HIDE );    */

            if (IS_AFFECTED(victim, AFF_CLOAK_FLAMING) && (number_percent() < chance)) {
                chance = (chance) / 3;

                act(cloak_table[CLOAK_FLAMING].remove_self, victim, NULL, NULL, TO_CHAR);
                act(cloak_table[CLOAK_FLAMING].remove_room, victim, NULL, NULL, TO_ROOM);
                REMOVE_BIT(victim->affected_by, AFF_CLOAK_FLAMING);
            }

        }

        CREF(vch_next, CHAR_NEXTROOM);
        for (vch = ch->in_room->first_person; vch; vch = vch_next) {
            vch_next = vch->next_in_room;
            if (victim == vch && victim->fighting == NULL && ch != victim) {
                check_killer(ch, victim);
                if ((victim->in_room != NULL)
                    && !IS_SET(victim->in_room->room_flags, ROOM_SAFE))
                    multi_hit(ch, victim, TYPE_UNDEFINED);
                break;
            }
        }
        CUREF(vch_next);

        return TRUE;
    }
    /* This won't work in conjunction with identify! */
    if ((ob = get_obj_carry(ch, target_name)) != NULL) {
        /* NOTE: Must also remove ALL affects, otherwise players
         * will be able to enchant, dispel, enchant.... -S-
         * No they wouldnt, enchant has a check for obj->first_apply.
         * -- Alty
         */

        if (IS_SET(ob->extra_flags, ITEM_UNIQUE)) {
            act("$p @@N@@edoesn't want to be dispelled, thankyouverymuch!@@N", ch, ob, NULL, TO_CHAR);
            return FALSE;
        }

        if (IS_SET(ob->extra_flags, ITEM_NODISPEL)) {
            act("$p @@N@@gis flagged nodispel! Use the @@Wnodispel @@gcommand to toggle this.@@N", ch, ob, NULL, TO_CHAR);
            return FALSE;
        }

        if (IS_SET(ob->extra_flags, ITEM_GLOW)) {
            REMOVE_BIT(ob->extra_flags, ITEM_GLOW);
            act("$p stops glowing.", ch, ob, NULL, TO_ROOM);
            act("$p stops glowing.", ch, ob, NULL, TO_CHAR);
        }
        if (IS_SET(ob->extra_flags, ITEM_HUM)) {
            REMOVE_BIT(ob->extra_flags, ITEM_HUM);
            act("The hum surrounding $p fades.", ch, ob, NULL, TO_CHAR);
            act("The hum surrounding $p fades.", ch, ob, NULL, TO_ROOM);
        }
        if (IS_SET(ob->extra_flags, ITEM_EVIL)) {
            REMOVE_BIT(ob->extra_flags, ITEM_EVIL);
            act("$p looks less evil.", ch, ob, NULL, TO_CHAR);
            act("$p looks less evil.", ch, ob, NULL, TO_ROOM);
        }
        if (IS_SET(ob->extra_flags, ITEM_NODROP))
            REMOVE_BIT(ob->extra_flags, ITEM_NODROP);

        if (IS_SET(ob->extra_flags, ITEM_INVIS)) {
            REMOVE_BIT(ob->extra_flags, ITEM_INVIS);
            act("$p fades back into view.", ch, ob, NULL, TO_CHAR);
            act("$p fades back into view.", ch, ob, NULL, TO_ROOM);
        }
        if (IS_SET(ob->extra_flags, ITEM_MAGIC)) {
            REMOVE_BIT(ob->extra_flags, ITEM_MAGIC);
            act("$p looks less magical.", ch, ob, NULL, TO_CHAR);
            act("$p looks less magical.", ch, ob, NULL, TO_ROOM);
        }
        if (IS_SET(ob->extra_flags, ITEM_BLESS)) {
            REMOVE_BIT(ob->extra_flags, ITEM_BLESS);
            act("$p looks less Holy.", ch, ob, NULL, TO_CHAR);
            act("$p looks less Holy.", ch, ob, NULL, TO_ROOM);
        }
        if (IS_SET(ob->extra_flags, ITEM_NOREMOVE))
            REMOVE_BIT(ob->extra_flags, ITEM_NOREMOVE);

        {
            AFFECT_DATA        *paf;

            while (ob->first_apply) {
                paf = ob->first_apply;

                UNLINK(paf, ob->first_apply, ob->last_apply, next, prev);
                PUT_FREE(paf, affect_free);
            }
        }

        return TRUE;
    }

    send_to_char("Dispel who or what??\n\r", ch);
    /* Couldn't find a target, get rid of their wait state */
    ch->wait = 0;
    return FALSE;
}

bool
spell_earthquake(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *vch;
    CHAR_DATA          *vch_next;

    if (obj == NULL) {
        send_to_char("The earth trembles beneath your feet!\n\r", ch);
        act("$n makes the earth tremble and shiver.", ch, NULL, NULL, TO_ROOM);
    }
    else {
        act("$p vibrates violently, making the earth tremble!", ch, obj, NULL, TO_CHAR);
        act("$p vibrates violenty, making the earth around $n tremble!", ch, obj, NULL, TO_ROOM);
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
                act("$n loses $s footing, and falls to the ground!", vch, NULL, NULL, TO_ROOM);
                send_to_char("You lose your footing, and fall to the ground!\n\r", vch);
                damage(ch, vch, level + dice(2, 8), -1);
            }
            else {
                act("$n keeps $s footing, and stays where $e is.", vch, NULL, NULL, TO_ROOM);
                send_to_char("You keep your footing.\n\r", vch);
            }
            continue;
        }

        if (vch->in_room->area == ch->in_room->area)
            send_to_char("The earth trembles and shivers.\n\r", vch);
    }
    CUREF(vch_next);
    return TRUE;
}

bool
spell_enchant_weapon(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    OBJ_DATA           *ob = (OBJ_DATA *) vo;
    AFFECT_DATA        *paf;

    /* Quick way to stop imms (Bash?) enchanting weapons for players */
    if (IS_IMMORTAL(ch) && ch->level != 90) {
        send_to_char("Nothing Happens.\n\r", ch);
        return FALSE;
    }

    /* Should change the first_apply thing to something else..
       right now you can't enchant ANYTHING with ANY affect -- Alty */
    if (ob->item_type != ITEM_WEAPON || IS_OBJ_STAT(ob, ITEM_MAGIC)
        || ob->first_apply != NULL) {
        send_to_char("Nothing happens.\n\r", ch);
        return TRUE;
    }

    GET_FREE(paf, affect_free);
    paf->type = sn;
    paf->duration = -1;
    paf->location = APPLY_HITROLL;
    paf->modifier = UMIN((level / 30) + 1, ob->level);
    paf->bitvector = 0;
    paf->save = TRUE;
    LINK(paf, ob->first_apply, ob->last_apply, next, prev);

    GET_FREE(paf, affect_free);
    paf->type = -1;
    paf->duration = -1;
    paf->location = APPLY_DAMROLL;
    paf->modifier = UMIN((level / 40) + 1, ob->level);
    paf->bitvector = 0;
    paf->save = TRUE;
    LINK(paf, ob->first_apply, ob->last_apply, next, prev);

    if (IS_GOOD(ch)) {
        SET_BIT(ob->extra_flags, ITEM_ANTI_EVIL);
        act("$p glows white.", ch, ob, NULL, TO_CHAR);
    }
    else if (IS_EVIL(ch)) {
        SET_BIT(ob->extra_flags, ITEM_ANTI_GOOD);
        act("$p glows black.", ch, ob, NULL, TO_CHAR);
    }
    else {
        SET_BIT(ob->extra_flags, ITEM_ANTI_EVIL);
        SET_BIT(ob->extra_flags, ITEM_ANTI_GOOD);
        act("$p glows yellow.", ch, ob, NULL, TO_CHAR);
    }

    return TRUE;
}

bool
spell_encumber(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)

/* Increases victim's ac.  Tweaked copy of armor function
 * --Stephen
 */
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (is_affected(victim, sn)) {
        if (victim == ch)
            send_to_char("You are already encumbered!\n\r", ch);
        else
            act("$N is already encumbered!", ch, NULL, victim, TO_CHAR);
        return TRUE;
    }
    af.type = sn;
    af.duration = 5 + (level / 16);
    af.modifier = +40;
    af.location = APPLY_AC;
    af.bitvector = 0;
    af.save = TRUE;
    affect_to_char(victim, &af);
    send_to_char("You feel less protected.\n\r", victim);
    if (ch != victim)
        act("$N looks more protected.", ch, NULL, victim, TO_CHAR);
    return TRUE;
}

bool
spell_fireball(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = {
        0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 30, 35, 40, 45, 50, 55,
        60, 65, 70, 75, 80, 82, 84, 86, 88, 90,
        92, 94, 96, 98, 100, 102, 104, 106, 108, 110,
        112, 114, 116, 118, 120, 122, 124, 126, 128, 130
    };
    int                 dam;

    level = UMIN(level, sizeof(dam_each) / sizeof(dam_each[0]) - 1);
    level = UMAX(0, level);
    dam = number_range(dam_each[level] / 2, dam_each[level] * 2);
    if (saves_spell(level, victim))
        dam /= 2;
    if (obj == NULL) {
        act("$n points at $N.  A fireball springs out and strikes $M!", ch, NULL, victim, TO_NOTVICT);
        act("You point at $N.  A fireball springs out and strikes $M!", ch, NULL, victim, TO_CHAR);
        act("$N points at you.  A fireball springs out and strikes you!", victim, NULL, ch, TO_CHAR);
    }
    else {
        act("A fireball shoots from $p and hits $n!", victim, obj, NULL, TO_NOTVICT);
        act("A fireball shoots from $p and hits you!", victim, obj, NULL, TO_CHAR);
    }
    damage(ch, victim, dam, -1);
    return TRUE;
}

bool
spell_flamestrike(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    int                 dam;

    dam = dice(6, level);
    if (saves_spell(level, victim))
        dam /= 2;
    act("A jet of flame roars down onto $n!", victim, NULL, NULL, TO_ROOM);
    send_to_char("A jet of flame roars down onto you!\n\r", victim);

    damage(ch, victim, dam, -1);
    return TRUE;
}

bool
spell_faerie_fire(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (IS_AFFECTED(victim, AFF_FAERIE_FIRE))
        return (ch == victim ? FALSE : TRUE);
    af.type = sn;
    af.duration = 5 + (level / 5);
    af.location = APPLY_AC;
    af.modifier = 2 * get_pseudo_level(ch);
    af.bitvector = AFF_FAERIE_FIRE;
    af.save = TRUE;
    affect_to_char(victim, &af);
    send_to_char("You are surrounded by a pink outline.\n\r", victim);
    act("$n is surrounded by a pink outline.", victim, NULL, NULL, TO_ROOM);
    return TRUE;
}

bool
spell_faerie_fog(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *ich;

    if (obj == NULL) {
        act("$n conjures a cloud of purple smoke.", ch, NULL, NULL, TO_ROOM);
        send_to_char("You conjure a cloud of purple smoke.\n\r", ch);
    }
    else {
        act("A cloud of purple smoke flows from $p.", ch, obj, NULL, TO_ROOM);
        act("A cloud of purple smoke flows from $p.", ch, obj, NULL, TO_ROOM);
    }
    for (ich = ch->in_room->first_person; ich != NULL; ich = ich->next_in_room) {
        if (!IS_NPC(ich) && IS_SET(ich->act, PLR_WIZINVIS))
            continue;

        if (ich == ch || saves_spell(level, ich))
            continue;

        /* ninjafix -dave */
        if (!IS_NPC(ich)
            && (IS_IMMORTAL(ich)
                || ich->stance == STANCE_AMBUSH || ich->stance == STANCE_AC_BEST)
            )
            continue;

        affect_strip(ich, gsn_invis);
        affect_strip(ich, gsn_mass_invis);
        affect_strip(ich, gsn_sneak);
        REMOVE_BIT(ich->affected_by, AFF_HIDE);
        REMOVE_BIT(ich->affected_by, AFF_INVISIBLE);
        REMOVE_BIT(ich->affected_by, AFF_SNEAK);
        act("$n is revealed!", ich, NULL, NULL, TO_ROOM);
        send_to_char("You are revealed!\n\r", ich);
    }

    return TRUE;
}

bool
spell_fly(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (IS_AFFECTED(victim, AFF_FLYING))
        return (ch == victim ? FALSE : TRUE);
    af.type = sn;
    af.duration = 3 + (level / 6);
    af.location = 0;
    af.modifier = 0;
    af.bitvector = AFF_FLYING;
    af.save = TRUE;
    affect_to_char(victim, &af);
    send_to_char("Your feet rise off the ground.\n\r", victim);
    act("$n's feet rise off the ground.", victim, NULL, NULL, TO_ROOM);
    return TRUE;
}

bool
spell_gate(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    return TRUE;
}

/*
 * Spell for mega1.are from Glop/Erkenbrand.
 */
bool
spell_general_purpose(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    int                 dam;

    dam = number_range(25, 100);
    if (saves_spell(level, victim))
        dam /= 2;
    if (obj != NULL) {
        act("A round of general purpose ammo from $p strikes $n!", victim, obj, NULL, TO_ROOM);
        act("A round of general purpose ammo from $p strikes you!", victim, obj, NULL, TO_CHAR);
    }
    else {
        act("$n's general purpose ammo strikes $N!", ch, NULL, victim, TO_NOTVICT);
        act("$n's general purpose ammo strikes you!", ch, NULL, victim, TO_VICT);
        act("You strike $N with your general purpose ammo!", ch, NULL, victim, TO_CHAR);
    }

    damage(ch, victim, dam, -1);
    return TRUE;
}

bool
spell_giant_strength(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (is_affected(victim, sn))
        return FALSE;
    af.type = sn;
    af.duration = 2 + (level / 4);
    af.location = APPLY_STR;
    af.modifier = 1 + (level >= 50) + (level >= 65);
    af.bitvector = 0;
    af.save = TRUE;
    affect_to_char(victim, &af);
    send_to_char("You feel stronger.\n\r", victim);
    if (ch != victim)
        send_to_char("Ok.\n\r", ch);
    return TRUE;
}

bool
spell_harm(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    int                 dam;
    AFFECT_DATA         af;

    dam = UMAX(250, victim->hit - dice(1, 4));
    if (saves_spell(level, victim))
        dam = UMIN(50, dam / 4);
    dam = UMIN(100, dam);
    damage(ch, victim, dam, sn);

    if (victim->race != RACE_LAM) {
        af.type = skill_lookup("poison");
        af.duration = 12 + (level / 10);
        af.location = APPLY_STR;
        af.modifier = -2;
        af.bitvector = AFF_POISON;
        af.save = TRUE;
        affect_join(victim, &af);
        send_to_char("You feel very sick.\n\r", victim);
    }

    if (ch != victim)
        send_to_char("Ok.\n\r", ch);
    return TRUE;
}

bool
spell_heal(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    DUEL_DATA          *duel;
    int                 heal = 300;

    if (!IS_NPC(ch) && (duel = find_duel(ch)) && IS_SET(duel->flags, DUEL_DBLHEAL))
        heal *= 2;

    victim->hit = UMIN(victim->hit + heal, victim->max_hit);
    update_pos(victim);
    send_to_char("A warm feeling fills your body.\n\r", victim);
    if (ch != victim)
        send_to_char("Ok.\n\r", ch);
    spell_cure_poison(sn, level, ch, vo, obj);

    return TRUE;
}

bool
spell_influx(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)

/* -- Stephen */
{
    CHAR_DATA          *vch;

    if (obj == NULL) {
        send_to_char("You fill the room with healing energy!\n\r", ch);
        act("$n spreads $s arms and heals the room!", ch, NULL, NULL, TO_ROOM);
    }
    else {
        act("$p glows blindingly bright in $n's hand!", ch, obj, NULL, TO_ROOM);
        act("$p glows blindingly in your hand!", ch, obj, NULL, TO_CHAR);
    }
    /* need to do this on everyone! */
    /* for loop taken from spell_earthquake */
    /* Dont need to do on everyone.. earthquake does because it needs to
       send messages to people in other rooms as well.. (whole area sees
       "the earth trembles and shivers").. -- Alty */

    for (vch = ch->in_room->first_person; vch; vch = vch->next_in_room) {
        vch->hit = UMIN(vch->hit + 40, vch->max_hit);
        update_pos(vch);
    }

    /*    for ( vch=first_char; vch != NULL; vch = vch->next )
       {

       if ( vch->in_room == NULL )
       continue;
       if ( vch->in_room == ch->in_room )
       {
       CHAR_DATA *victim = (CHAR_DATA *) vo;
       victim->hit = UMIN( victim->hit + 40, victim->max_hit );
       update_pos( victim );
       }
       } */
    return TRUE;
}

/*
 * Spell for mega1.are from Glop/Erkenbrand.
 */
bool
spell_high_explosive(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    int                 dam;

    if (obj != NULL) {
        act("A high explosive charge from $p engulfs $n!", victim, obj, NULL, TO_ROOM);
        act("A high explosive charge from $p engulfs you!", victim, obj, NULL, TO_CHAR);
    }
    else {
        act("$n's high explosive charge engulfs $N!", ch, NULL, victim, TO_NOTVICT);
        act("Your high explosive charge engulfs $N!", ch, NULL, victim, TO_CHAR);
        act("$n's high explosive charge engulfs you!", ch, NULL, victim, TO_VICT);
    }

    dam = number_range(30, 120);
    if (saves_spell(level, victim))
        dam /= 2;
    damage(ch, victim, dam, sn);
    return TRUE;
}

bool
spell_laserbolt(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    int                 dam;

    dam = number_range(level / 2, level * 1.5);
    if (saves_spell(level, victim))
        dam /= 2;
    if (obj == NULL) {
        act("$n's laser bolt hits $N in the chest!!", ch, NULL, victim, TO_NOTVICT);
        act("Your laser bolt hits $N in the chest!!", ch, NULL, victim, TO_CHAR);
        act("$n's laser bolt hits you in the chest!!", ch, NULL, victim, TO_VICT);
    }
    else {
        act("A laser bolt shoots from $p, and hits $n!", victim, obj, NULL, TO_ROOM);
        act("A laser bolt shoots from $p, and hits you!", victim, obj, NULL, TO_CHAR);
    }
    damage(ch, victim, dam, -1);
    return TRUE;
}

bool
spell_identify(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    OBJ_DATA           *ob = (OBJ_DATA *) vo;
    char                buf[MAX_STRING_LENGTH];
    AFFECT_DATA        *paf, *paf2;

    sprintf(buf,
        "@@NObject '%s' is @@etype@@N %s, @@aextra flags@@N %s.\n\r@@mWorn@@N: %s, @@cWeight@@N: %d, @@yvalue@@N: %d @@yGP@@N, @@rlevel@@N: %d.\n\r",
        ob->short_descr,
        item_type_name(ob), extra_bit_name(ob->extra_flags), bit_table_lookup(tab_wear_flags, ob->wear_flags), ob->weight, ob->cost, ob->level);
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
            sprintf(buf, "Damage is %d to %d (average %d).\n\r", ob->value[1], ob->value[2], (ob->value[1] + ob->value[2]) / 2);
            send_to_char(buf, ch);
            break;

        case ITEM_ARMOR:
            sprintf(buf, "Armor class is %d.\n\r", ob->value[0]);
            send_to_char(buf, ch);
            break;
        case ITEM_QUEST:
            if (ob->value[0] > 0) {
                sprintf(buf, "Worth %d qps.\n\r", ob->value[0]);
                send_to_char(buf, ch);
            }
            if (ob->value[3] > 0) {
                sprintf(buf, "Must be level %d and lower to deposit.\n\r", ob->value[3]);
                send_to_char(buf, ch);
            }

            break;
    }

    /* If the object has an obj_fun, print it out */
    if (ob->obj_fun && strcmp(rev_obj_fun_lookup_nice(ob->obj_fun), ""))
        sendf(ch, "@@gThis object @@W%s@@g.@@N\n\r", rev_obj_fun_lookup_nice(ob->obj_fun));

    for (paf = ob->first_apply; paf != NULL; paf = paf->next) {
        if (paf->location != APPLY_NONE && paf->modifier != 0) {
            if (paf->type == -1) {
                if (   !IS_SET(ob->extra_flags, ITEM_UNIQUE)
                    || !ob->pIndexData
                    || ob->pIndexData->vnum == 3090
                   )
                    sprintf(buf, "Affects %s by %d.\n\r", affect_loc_name(paf->location), paf->modifier);
                else {
                    int diff;

                    paf2 = find_apply_location(ob->pIndexData, paf->location, TRUE);

                    if (!paf2)
                        diff = paf->modifier;
                    else
                        diff = paf->modifier - paf2->modifier;

                    if (diff == 0)
                        sprintf(buf, "Affects %s by %d.\n\r", affect_loc_name(paf->location), paf->modifier);
                    else
                        sprintf(buf, "Affects %s by %d. (%s%d)\n\r", affect_loc_name(paf->location), paf->modifier, (diff > 0) ? "+" : "", diff);
                }
            }
            else {
                int slotsn = slot_lookup(paf->type);

                if (slotsn == -1)
                    sprintf(buf, "Affects %s by %d.\n\r", affect_loc_name(paf->location), paf->modifier);
                else
                    sprintf(buf, "Affects %s by %d (%s).\n\r", affect_loc_name(paf->location), paf->modifier, skill_table[slotsn].name);
            }

            send_to_char(buf, ch);
        }
    }

    return TRUE;
}

bool
spell_infravision(int sn, int level, CHAR_DATA *ch, void *vo, OBJ_DATA *obj)
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    AFFECT_DATA         af;

    if (IS_AFFECTED(victim, AFF_INFRARED) || item_has_apply(victim, ITEM_APPLY_INFRA))
        return (ch == victim ? FALSE : TRUE);
    af.type = sn;
    af.duration = 4 + (level / 3);
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_INFRARED;
    af.save = TRUE;
    affect_to_char(victim, &af);
    send_to_char("Your eyes glow red.\n\r", victim);
    act("$n's eyes glow red.", victim, NULL, NULL, TO_ROOM);
    return TRUE;
}

bool can_summon_charmie(CHAR_DATA *master)
{
    DUEL_ALLOWED_CHARMIE(master);

    if (master->in_room && master->in_room->area && (IS_SET(master->in_room->room_flags, ROOM_NO_CHARM) || IS_SET(master->in_room->area->flags, AREA_NOCHARM)))
        return FALSE;

    return TRUE;
}
