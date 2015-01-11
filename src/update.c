
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

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

#define __USE_BSD
#include <stdio.h>
#undef __USE_BSD

#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#define __USE_POSIX
#include <signal.h>
#undef __USE_POSIX

#include "merc.h"
#include "duel.h"
#include "auction.h"
#include "dns.h"

IDSTRING(rcsid, "$Id: update.c,v 1.71 2004/11/12 07:18:22 dave Exp $");

extern bool         dbl_xp;
extern POL_DATA     politics_data;
extern OBJ_DATA    *quest_object;

/*
 * Local functions.
 */
int hit_gain        args((CHAR_DATA *ch));
int mana_gain       args((CHAR_DATA *ch));
int move_gain       args((CHAR_DATA *ch));

void mobile_update  args((void));
void weather_update args((void));
void char_update    args((void));
void gain_update    args((void));
void obj_update     args((void));
void aggr_update    args((void));
void objfun_update  args((void));
void auction_update args((void));
void rooms_update   args((void));
void remember_attack args((CHAR_DATA *ch, CHAR_DATA *victim));
void quest_update   args((void));
void note_update    args((void));
void energy_update  args((void));

/* teleport */
void teleport_update args((void));
bool room_teleport  args((TELEPORT_DATA *tele, CHAR_DATA *ch));

void desert_update  args((void));
void logfile_update args((void));

extern void duel_update args((void));

int                 abort_threshold = BOOT_DB_ABORT_THRESHOLD;
bool                disable_timer_abort = FALSE;
int                 last_checkpoint;

int
get_user_seconds()
{
    struct rusage       rus;

    getrusage(RUSAGE_SELF, &rus);
    return rus.ru_utime.tv_sec;
}

/* Update the checkpoint */
void
alarm_update()
{
    extern int          ssm_dup_count;
    extern int          ssm_loops;

    ssm_dup_count = 0;
    ssm_loops = 0;

    last_checkpoint = get_user_seconds();
    if (abort_threshold == BOOT_DB_ABORT_THRESHOLD) {
        abort_threshold = RUNNING_ABORT_THRESHOLD;
        FPRINTF(stdout, "Used %d user CPU seconds.\n", last_checkpoint);
    }
}

/* Set the virtual (CPU time) timer to the standard setting, ALARM_FREQUENCY */
void
reset_itimer()
{
    struct itimerval    itimer;

    itimer.it_interval.tv_usec = 0;    /* miliseconds */
    itimer.it_interval.tv_sec = ALARM_FREQUENCY;
    itimer.it_value.tv_usec = 0;
    itimer.it_value.tv_sec = ALARM_FREQUENCY;

    /* start the timer - in that many CPU seconds, alarm_handler will be called */
    if (setitimer(ITIMER_VIRTUAL, &itimer, NULL) < 0) {
        perror("reset_itimer:setitimer");
        exit(1);
    }
}

const char         *szFrozenMessage = "Alarm_handler: Not checkpointed recently, aborting!\n";

/* Signal handler for alarm - suggested for use in MUDs by Fusion */
void
alarm_handler(int signo)
{
    int                 usage_now = get_user_seconds();

    /* Has there gone abort_threshold CPU seconds without alarm_update? */
    if (!disable_timer_abort && (usage_now - last_checkpoint > abort_threshold)) {
        /* For the log file */
        char                buf[MAX_STRING_LENGTH];
        extern int          ssm_dup_count;
        extern int          ssm_loops;
        extern int          ssm_recent_loops;

        /* spec: log usage values */
        xlogf("current usage: %d, last checkpoint: %d", usage_now, last_checkpoint);
        xlogf("SSM dups: %d, loops: %d, recent: %d", ssm_dup_count, ssm_loops, ssm_recent_loops);

        sprintf(buf, "%s", szFrozenMessage);
        bug(buf);
        raise(SIGABRT);            /* kill ourselves on return */
    }

    /* The timer resets to the values specified in it_interval 
     * automatically.
     *
     * Spec: additionally, SIGABRT is blocked in this handler, and will
     * only be delivered on return. This should ensure a good core.
     */
}

/* Install signal alarm handler */
void
init_alarm_handler()
{
    struct sigaction    sa;

    sa.sa_handler = alarm_handler;
    sa.sa_flags = 1;            /* Restart interrupted system calls */
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGABRT);    /* block abort() in the handler
                                         * so we can get a good coredump */

    if (sigaction(SIGVTALRM, &sa, NULL) < 0) {    /* setup handler for virtual timer */
        perror("init_alarm_handler:sigaction");
        exit(1);
    }
    last_checkpoint = get_user_seconds();
    reset_itimer();                /* start timer */
}

/*
 * Advancement stuff.
 */
void
advance_level(CHAR_DATA *ch, int class, bool show, bool remort, int levels)
{

    /* class used instead of ch->class.  -S- */
    /* show added to allow no display of gain ( when using setclass ) */
    /* remort indicates remortal class or normal mortal class */

    char                buf[MAX_STRING_LENGTH];
    int                 add_hp = 0;
    int                 add_mana = 0;
    int                 add_move = 0;
    int                 add_prac = 0;
    int                 cnt;

    if ((class == 32)) {
        add_hp = UMAX(1, con_app[get_curr_con(ch)].hitp + number_range(10, 50));
        add_mana = UMAX(0, number_range(10, (3 * get_curr_int(ch) + get_curr_wis(ch)) / 4));
        add_move = UMAX(7, number_range(2, (get_curr_con(ch) + get_curr_dex(ch)) / 5));
        add_prac = (wis_app[get_curr_wis(ch)].practice / 2) + number_range(1, 3);
    }
    else if (remort) {
        for (cnt = 0; cnt < levels; cnt++) {
            add_hp += UMAX(1, con_app[get_curr_con(ch)].hitp + number_range(remort_table[class].hp_min, remort_table[class].hp_max));
            add_mana += UMAX(0, remort_table[class].fMana ? number_range(2, (2 * get_curr_int(ch) + get_curr_wis(ch)) / 16) : 0);
            add_move += UMAX(7, number_range(2, (get_curr_con(ch) + get_curr_dex(ch)) / 5));
            add_prac += (wis_app[get_curr_wis(ch)].practice / 2) + number_range(1, 3);
        }
    }
    else {
        for (cnt = 0; cnt < levels; cnt++) {
            add_hp += UMAX(1, con_app[get_curr_con(ch)].hitp + number_range(class_table[class].hp_min, class_table[class].hp_max));
            add_mana += UMAX(0, class_table[class].fMana ? number_range(2, (2 * get_curr_int(ch) + get_curr_wis(ch)) / 16) : 0);
            add_move += UMAX(7, number_range(2, (get_curr_con(ch) + get_curr_dex(ch)) / 5));
            add_prac += (wis_app[get_curr_wis(ch)].practice / 2) + number_range(1, 3);
        }
    }

    ch->pcdata->mana_from_gain += add_mana;
    ch->pcdata->hp_from_gain += add_hp;
    ch->pcdata->move_from_gain += add_move;

    ch->max_hit += add_hp;
    ch->max_mana += add_mana;
    ch->max_move += add_move;
    ch->practice += add_prac;

    if (!IS_NPC(ch))
        REMOVE_BIT(ch->act, PLR_BOUGHT_PET);

    sprintf(buf, "You gain: %d Hit Points, %d Mana, %d Movement and %d pracs.\n\r", add_hp, add_mana, add_move, add_prac);

    if (show)
        send_to_char(buf, ch);

    if (!IS_NPC(ch) && ch->pcdata->clan > 0)
        update_cinfo(ch, FALSE);

    return;
}

void
gain_exp(CHAR_DATA *ch, long_int gain)
{
    if (IS_NPC(ch) || IS_IMMORTAL(ch))
        return;

    ch->exp += gain;
    return;
}

/*
 * Regeneration stuff.
 */
int
hit_gain(CHAR_DATA *ch)
{
    int                 gain;
    bool                is_troll = FALSE;

    if (ch->is_free != FALSE)
        return 0;

    if (IS_NPC(ch))
        gain = (5 + ch->level / 30);
    else
        gain = (5 + ch->level / 20);

    if (!IS_NPC(ch) && ch->race == RACE_TRL)
        is_troll = TRUE;

    if (is_troll)
        gain += (ch->hit + ch->max_hit) / 50;

    if (IS_SET(ch->in_room->room_flags, ROOM_REGEN) && !is_troll)
        gain *= 2;
    else
        gain *= 1.2;

    switch (ch->position) {
        case POS_SLEEPING:
            gain += get_curr_con(ch) / 2;
            break;
        case POS_RESTING:
            gain += get_curr_con(ch) / 4;
            break;
    }
    if (!IS_NPC(ch)) {
        if (ch->pcdata->condition[COND_FULL] == 0)
            gain /= 2;

        if (ch->pcdata->condition[COND_THIRST] == 0)
            gain /= 2;
    }

    if (IS_AFFECTED(ch, AFF_POISON))
        gain /= 4;

    if (IS_SET(ch->in_room->room_flags, ROOM_COLD)
        || (IS_SET(ch->in_room->room_flags, ROOM_HOT)
            && (!IS_NPC(ch) && is_troll))) {
        gain = 0;
    }
    else if (IS_SET(ch->in_room->room_flags, ROOM_COLD)
        || (IS_SET(ch->in_room->room_flags, ROOM_HOT))) {
        gain = -(ch->max_hit / 50);
    }
    if (IS_SET(ch->in_room->affected_by, ROOM_BV_HEAL_REGEN)) {
        if (gain < 0)
            gain = (ch->max_hit / 25);
        else if (!is_troll)
            gain *= 2;
        else
            gain *= 1.2;
    }

    if (IS_AFFECTED(ch, AFF_CLOAK_REGEN)) {
        if (gain < 0)
            gain *= -2;
        else if (!is_troll)
            gain *= 3;
        else
            gain *= 1.2;
    }

    if (IS_SET(ch->in_room->affected_by, ROOM_BV_HEAL_STEAL) && !IS_SET(ch->in_room->room_flags, ROOM_SAFE))
        if (gain > 0)
            gain = -(ch->max_hit / 25);
    if (!IS_NPC(ch) && (gain > 0)) {
        /*      if ( IS_SET( race_table[ch->race].race_flags, RACE_MOD_FAST_HEAL ) )
           gain = gain * 1.5;
         */
        if (IS_SET(race_table[ch->race].race_flags, RACE_MOD_SLOW_HEAL))
            gain = gain * .75;
    }
    if (!IS_NPC(ch) && (gain > 0)) {
        if ((IS_SET(race_table[ch->race].race_flags, RACE_MOD_WOODLAND))
            && (ch->in_room != NULL)) {
            if ((ch->in_room->sector_type == SECT_FIELD)
                || (ch->in_room->sector_type == SECT_FOREST))
                gain = gain * 1.3;
            else if ((ch->in_room->sector_type == SECT_CITY)
                || (ch->in_room->sector_type == SECT_INSIDE))
                gain = gain * .8;
        }
        else if ((IS_SET(race_table[ch->race].race_flags, RACE_MOD_DARKNESS))
            && (ch->in_room != NULL)) {
            if ((ch->in_room->sector_type == SECT_FIELD)
                || (ch->in_room->sector_type == SECT_HILLS)
                || (ch->in_room->sector_type == SECT_AIR)
                || (ch->in_room->sector_type == SECT_DESERT))
                gain = gain * .8;
            else if ((ch->in_room->sector_type == SECT_CITY)
                || (ch->in_room->sector_type == SECT_INSIDE))
                gain = gain * 1.3;
        }

    }

    if (!IS_NPC(ch) && IS_IMMORTAL(ch) && gain <= 0)
        gain = 10;

    return UMIN(gain, ch->max_hit - ch->hit);
}

int
mana_gain(CHAR_DATA *ch)
{
    int                 gain;

    if (ch->is_free != FALSE)
        return 0;

    if (IS_NPC(ch))
        gain = (1 + ch->level / 30);
    else {
        gain = (5 + ch->level / 20);

        if (IS_SET(ch->in_room->room_flags, ROOM_REGEN))
            gain *= 2;

        switch (ch->position) {
            case POS_SLEEPING:
                gain += get_curr_int(ch);
                break;
            case POS_RESTING:
                gain += get_curr_int(ch) / 2;
                break;
        }
        if (!IS_NPC(ch)) {
            if (ch->pcdata->condition[COND_FULL] == 0)
                gain /= 2;

            if (ch->pcdata->condition[COND_THIRST] == 0)
                gain /= 2;
        }

        if (IS_SET(ch->in_room->affected_by, ROOM_BV_MANA_REGEN)) {
            if (gain < 0)
                gain *= -2;
            else
                gain *= 2;
        }
        if (IS_SET(ch->in_room->affected_by, ROOM_BV_MANA_STEAL))
            if (gain > 0)
                gain *= -1;

    }

    if (IS_AFFECTED(ch, AFF_POISON))
        gain /= 4;

    if (!IS_NPC(ch) && (gain > 0)) {
        if (IS_SET(race_table[ch->race].race_flags, RACE_MOD_NO_MAGIC))
            gain = gain * .5;
        else if (IS_SET(race_table[ch->race].race_flags, RACE_MOD_WEAK_MAGIC))
            gain = gain * .75;
        else if (IS_SET(race_table[ch->race].race_flags, RACE_MOD_STRONG_MAGIC))
            gain = gain * 1.25;

    }
    if (!IS_NPC(ch) && (gain > 0)) {
        if ((IS_SET(race_table[ch->race].race_flags, RACE_MOD_WOODLAND))
            && (ch->in_room != NULL)) {
            if ((ch->in_room->sector_type == SECT_FIELD)
                || (ch->in_room->sector_type == SECT_FOREST))
                gain = gain * 1.3;
            else if ((ch->in_room->sector_type == SECT_CITY)
                || (ch->in_room->sector_type == SECT_INSIDE))
                gain = gain * .8;
        }
        else if ((IS_SET(race_table[ch->race].race_flags, RACE_MOD_DARKNESS))
            && (ch->in_room != NULL)) {
            if ((ch->in_room->sector_type == SECT_FIELD)
                || (ch->in_room->sector_type == SECT_HILLS)
                || (ch->in_room->sector_type == SECT_AIR)
                || (ch->in_room->sector_type == SECT_DESERT))
                gain = gain * .8;
            else if ((ch->in_room->sector_type == SECT_CITY)
                || (ch->in_room->sector_type == SECT_INSIDE))
                gain = gain * 1.3;
        }
    }
    if (gain > 0)
        gain = gain * int_app[get_curr_int(ch)].mana_regen / 10;

    return UMIN(gain, ch->max_mana - ch->mana);
}

int
move_gain(CHAR_DATA *ch)
{
    int                 gain;

    if (IS_NPC(ch)) {
        gain = ch->level;
    }
    else {
        gain = (10 + ch->level / 4);

        if (IS_SET(ch->in_room->room_flags, ROOM_REGEN))
            gain *= 2;

        switch (ch->position) {
            case POS_SLEEPING:
                gain += get_curr_dex(ch) / 2;
                break;
            case POS_RESTING:
                gain += get_curr_dex(ch) / 4;
                break;
        }

        if (ch->pcdata->condition[COND_FULL] == 0)
            gain /= 2;

        if (ch->pcdata->condition[COND_THIRST] == 0)
            gain /= 2;
    }

    if (IS_AFFECTED(ch, AFF_POISON))
        gain /= 4;

    return UMIN(gain, ch->max_move - ch->move);
}

void
gain_condition(CHAR_DATA *ch, int iCond, int value)
{
    int                 condition;

    if (value == 0 || IS_NPC(ch) || ch->level >= LEVEL_HERO)
        return;

    condition = ch->pcdata->condition[iCond];
    ch->pcdata->condition[iCond] = URANGE(0, condition + value, 48);

    if (ch->position == POS_BUILDING || ch->position == POS_WRITING)
        return;

    if (ch->pcdata->condition[iCond] == 0) {
        switch (iCond) {
            case COND_FULL:
                send_to_char("You are hungry.\n\r", ch);
                break;

            case COND_THIRST:
                send_to_char("You are thirsty.\n\r", ch);
                break;

            case COND_DRUNK:
                if (condition != 0)
                    send_to_char("You are sober.\n\r", ch);
                break;
        }
    }

    return;
}

/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Merc cpu time.
 * -- Furey
 */
void
mobile_update(void)
{
    CHAR_DATA          *ch;
    CHAR_DATA          *ch_next;
    CHAR_DATA          *target;
    EXIT_DATA          *pexit;
    extern OBJ_DATA    *quest_object;
    extern CHAR_DATA   *quest_mob, *quest_target;
    int                 door;

    /* Examine all mobs. */
    CREF(ch_next, CHAR_NEXT);

    for (ch = first_char; ch != NULL; ch = ch_next) {
        ch_next = ch->next;

        if (!IS_NPC(ch) || ch->in_room == NULL || IS_AFFECTED(ch, AFF_CHARM))
            continue;

        /* Examine call for special procedure */
        if (ch->spec_fun != 0) {
            if ((*ch->spec_fun) (ch))
                continue;
        }

        /* That's all for sleeping / busy monster */
        if (ch->position < POS_STANDING)
            continue;

        /* Check for rewield, and re-equip (specials not used anymore) */
        if (IS_SET(ch->act, ACT_REWIELD))
            if (check_rewield(ch))
                continue;

        if (IS_SET(ch->act, ACT_RE_EQUIP))
            if (check_re_equip(ch))
                continue;

        /* Check for remember victims */
        if (ch->target != NULL && (target = get_char_room(ch, ch->target)) != NULL && ch != quest_mob && ch != quest_target) {
            remember_attack(ch, target);
            continue;
        }

        /* Check to see if mob is moving somewhere */
        /*  if ( mob_hunt(ch) )
           continue;
           if ( IS_SET( ch->act_hunt, ACT_HUNT_MOVE ) 
           && ch->move_to != NO_VNUM )
           {
           hunt_move( ch );
           continue;
           }
         */

        /* MOBprogram random trigger */
        if (ch->in_room->area->nplayer > 0) {
            mprog_random_trigger(ch);
            /* If ch dies or changes
               position due to its random
               trigger continue - Kahn */
            if (ch->position < POS_STANDING)
                continue;
        }

        /* Scavenge */
        if (IS_SET(ch->act, ACT_SCAVENGER) && ch != quest_mob
            && ch->in_room->first_content != NULL && number_bits(2) == 0) {
            OBJ_DATA           *obj;
            OBJ_DATA           *obj_best;
            int                 max;

            max = 1;
            obj_best = 0;
            for (obj = ch->in_room->first_content; obj; obj = obj->next_in_room) {
                if (CAN_WEAR(obj, ITEM_TAKE) && obj->cost > max) {
                    obj_best = obj;
                    max = obj->cost;
                }
            }

            if (obj_best && obj_best != quest_object) {
                obj_from_room(obj_best);
                obj_to_char(obj_best, ch);
                act("$n gets $p.", ch, obj_best, NULL, TO_ROOM);
            }
        }

        /* Wander */
        if (!IS_SET(ch->act, ACT_SENTINEL)
            && ch->leader == NULL
            && (door = number_bits(5)) <= 5
            && (pexit = ch->in_room->exit[door]) != NULL && pexit->to_room != NULL && !IS_SET(pexit->exit_info, EX_CLOSED)
            && !IS_SET(pexit->to_room->room_flags, ROOM_NO_MOB)
            && !IS_SET(pexit->to_room->room_flags, ROOM_SAFE)
            && (!IS_SET(ch->act, ACT_STAY_AREA)
                || pexit->to_room->area == ch->in_room->area)) {
            move_char(ch, door);
            /* If ch changes position due
               to its or someother mob's
               movement via MOBProgs,
               continue - Kahn */
            if (ch->position < POS_STANDING)
                continue;
        }

        /* Flee */
        if (ch->hit < (ch->max_hit / 2)
            && (door = number_bits(3)) <= 5
            && (pexit = ch->in_room->exit[door]) != NULL && pexit->to_room != NULL && !IS_SET(pexit->exit_info, EX_CLOSED)
            && !IS_SET(pexit->to_room->room_flags, ROOM_NO_MOB)
            /* weedermod: sentinel mobs do NOT flee!
               they stand their ground no
               matter how much theyre
               suffering :P
             */
            && !IS_SET(ch->act, ACT_SENTINEL)) {
            CHAR_DATA          *rch;
            bool                found;

            found = FALSE;
            for (rch = pexit->to_room->first_person; rch != NULL; rch = rch->next_in_room) {
                if (!IS_NPC(rch)) {
                    found = TRUE;
                    break;
                }
            }
            if (!found)
                move_char(ch, door);
        }

        if (ch->pIndexData && ch->pIndexData->path != NULL && ch->pIndexData->path[0] != '\0') {
            if (ch->in_room && ch->fighting == NULL) {
                char               *current_step;
                bool                bad_first = FALSE;
                ROOM_INDEX_DATA    *room;

                current_step = get_path_step(ch->pIndexData->path, ch->path_steps);

                if (current_step == NULL) {
                    if (ch->path_steps > 0) {
                        ch->path_steps = 0;
                        current_step = get_path_step(ch->pIndexData->path, ch->path_steps);
                        if (current_step == NULL)
                            bad_first = TRUE;
                    }
                    else
                        bad_first = TRUE;
                }

                if (!bad_first && ((strlen(current_step) == 1 && current_step[0] != '.') || current_step[0] == '!')) {
                    /* it's either a direction, or we've noted that we want to watch for movement (!) */
                    if (current_step[0] == '!')
                        current_step++;

                    room = ch->in_room;

                    if (strpos(current_step, "|") == -1)
                        interpret(ch, current_step);
                    else {
                        /* parse out separate commands */
                        char                buf[MSL];
                        char               *p_buf = buf;
                        char               *go;

                        go = current_step;

                        while (*go) {
                            if (*go == '|') {
                                *p_buf = '\0';
                                interpret(ch, buf);
                                buf[0] = '\0';
                                p_buf = buf;
                            }
                            else
                                *p_buf++ = *go;

                            go++;
                        }

                        *p_buf = '\0';
                        interpret(ch, buf);
                    }

                    if (ch && ch->is_free == FALSE && ch->in_room && ch->in_room != room) {
                        /* successfully changed rooms */
                        ch->path_steps++;
                    }
                }
                else if (!bad_first) {
                    if (current_step[0] != '.') {
                        if (strpos(current_step, "|") == -1)
                            interpret(ch, current_step);
                        else {
                            /* parse out separate commands */
                            char                buf[MSL];
                            char               *p_buf = buf;
                            char               *go;

                            go = current_step;

                            while (*go) {
                                if (*go == '|') {
                                    *p_buf = '\0';
                                    interpret(ch, buf);
                                    buf[0] = '\0';
                                    p_buf = buf;
                                }
                                else
                                    *p_buf++ = *go;

                                go++;
                            }

                            *p_buf = '\0';
                            interpret(ch, buf);
                        }
                    }

                    ch->path_steps++;
                }
            }
        }
    }

    CUREF(ch_next);
    return;
}

/*
 * Update the weather.
 */
void
weather_update(void)
{
    char                buf[MAX_STRING_LENGTH];
    char                buf2[MSL];
    DESCRIPTOR_DATA    *d;
    int                 diff;
    sh_int              x, y;

    buf[0] = '\0';
    buf2[0] = '\0';

    switch (++time_info.hour) {
        case 5:
            weather_info.sunlight = SUN_LIGHT;
            strcat(buf, "The sky shows signs of daybreak.\n\r");
            break;

        case 6:
            weather_info.sunlight = SUN_RISE;
            strcat(buf, "The sun rises in the east.\n\r");
            for (x = 1; x < MAX_CLAN; x++)
                for (y = 1; y < MAX_CLAN; y++)
                    politics_data.daily_negotiate_table[x][y] = FALSE;

            break;
        case 12:

            for (x = 1; x < MAX_CLAN; x++)
                for (y = 1; y < MAX_CLAN; y++)
                    politics_data.daily_negotiate_table[x][y] = FALSE;
            break;

        case 19:
            weather_info.sunlight = SUN_SET;
            strcat(buf, "The sun slowly disappears in the west.\n\r");
            for (x = 1; x < MAX_CLAN; x++)
                for (y = 1; y < MAX_CLAN; y++)
                    politics_data.daily_negotiate_table[x][y] = FALSE;
            break;

        case 20:
            weather_info.sunlight = SUN_DARK;
            strcat(buf, "The night has begun.\n\r");
            break;

        case 24:
            time_info.hour = 0;
            time_info.day++;
            for (x = 1; x < MAX_CLAN; x++)
                for (y = 1; y < MAX_CLAN; y++)
                    politics_data.daily_negotiate_table[x][y] = FALSE;
            break;
    }
    switch (time_info.moon++) {
        case 5:
            weather_info.moon_loc = MOON_RISE;
            sprintf(buf2, "@@NA %s @@yMoon @@Nhas risen.\n\r", get_moon_phase_name());
            safe_strcat(MSL, buf, buf2);
            break;
        case 10:
            weather_info.moon_loc = MOON_LOW;
            sprintf(buf2, "@@NThe %s @@yMoon @@Nrides low on the horizon.\n\r", get_moon_phase_name());
            safe_strcat(MSL, buf, buf2);
            break;
        case 15:
            weather_info.moon_loc = MOON_PEAK;
            sprintf(buf2, "@@NThe %s @@yMoon @@Nreaches its zenith.\n\r", get_moon_phase_name());
            safe_strcat(MSL, buf, buf2);
            break;
        case 20:
            weather_info.moon_loc = MOON_FALL;
            sprintf(buf2, "@@NThe %s @@yMoon @@Nfalls.\n\r", get_moon_phase_name());
            safe_strcat(MSL, buf, buf2);
            break;
        case 25:
            weather_info.moon_loc = MOON_SET;
            sprintf(buf2, "@@NThe %s @@yMoon @@Nis setting.\n\r", get_moon_phase_name());
            safe_strcat(MSL, buf, buf2);
            break;
        case 30:
            weather_info.moon_loc = MOON_DOWN;
            sprintf(buf2, "@@NThe %s @@yMoon @@Nhas left the sky.\n\r", get_moon_phase_name());
            safe_strcat(MSL, buf, buf2);
            break;

        default:
            break;
    }

    if (time_info.moon >= 50) {
        time_info.moon = 0;
        weather_info.moon_loc = MOON_DOWN;
    }

    if (time_info.day >= 20) {    /* now 20 days = 1 month */
        time_info.day = 0;
        time_info.month++;
    }

    if (time_info.month >= 8) {    /* 8 months a year */
        time_info.month = 0;
        time_info.year++;
    }
    if (((time_info.day) % 4) == 0) {
        if (!weather_info.phase_changed)
            weather_info.moon_phase++;
        if (weather_info.moon_phase > MOON_WAN_CRE)
            weather_info.moon_phase = MOON_NEW;
        weather_info.phase_changed = TRUE;
    }
    else
        weather_info.phase_changed = FALSE;

    /*
     * Weather change.
     */
    if (time_info.month >= 9 && time_info.month <= 16)
        diff = weather_info.mmhg > 985 ? -2 : 2;
    else
        diff = weather_info.mmhg > 1015 ? -2 : 2;

    weather_info.change += diff * dice(1, 4) + dice(2, 6) - dice(2, 6);
    weather_info.change = UMAX(weather_info.change, -12);
    weather_info.change = UMIN(weather_info.change, 12);

    weather_info.mmhg += weather_info.change;
    weather_info.mmhg = UMAX(weather_info.mmhg, 960);
    weather_info.mmhg = UMIN(weather_info.mmhg, 1040);

    switch (weather_info.sky) {
        default:
            bugf("Weather_update: bad sky %d.", weather_info.sky);
            weather_info.sky = SKY_CLOUDLESS;
            break;

        case SKY_CLOUDLESS:
            if (weather_info.mmhg < 990 || (weather_info.mmhg < 1010 && number_bits(2) == 0)) {
                strcat(buf, "The sky is getting cloudy.\n\r");
                weather_info.sky = SKY_CLOUDY;
            }
            break;

        case SKY_CLOUDY:
            if (weather_info.mmhg < 970 || (weather_info.mmhg < 990 && number_bits(2) == 0)) {
                strcat(buf, "It starts to rain.\n\r");
                weather_info.sky = SKY_RAINING;
            }

            if (weather_info.mmhg > 1030 && number_bits(2) == 0) {
                strcat(buf, "The clouds disappear.\n\r");
                weather_info.sky = SKY_CLOUDLESS;
            }
            break;

        case SKY_RAINING:
            if (weather_info.mmhg < 970 && number_bits(2) == 0) {
                strcat(buf, "Lightning flashes in the sky.\n\r");
                weather_info.sky = SKY_LIGHTNING;
            }

            if (weather_info.mmhg > 1030 || (weather_info.mmhg > 1010 && number_bits(2) == 0)) {
                strcat(buf, "The rain stopped.\n\r");
                weather_info.sky = SKY_CLOUDY;
            }
            break;

        case SKY_LIGHTNING:
            if (weather_info.mmhg > 1010 || (weather_info.mmhg > 990 && number_bits(2) == 0)) {
                strcat(buf, "The lightning has stopped.\n\r");
                weather_info.sky = SKY_RAINING;
                break;
            }
            break;
    }

    if (buf[0] != '\0') {
        for (d = first_desc; d != NULL; d = d->next) {
            if (d->connected == CON_PLAYING && IS_OUTSIDE(d->character)
                && (d->character->position != POS_WRITING)
                && IS_AWAKE(d->character))
                send_to_char(buf, d->character);
        }
    }

    return;
}

/* New update loop to handle gains for players => smaller 'ticks' for
   hp/mana/move gain, normal 'ticks' for objects, affects, weather, etc */
void
gain_update(void)
{
    CHAR_DATA          *ch;

    /* send wholist to web page :) Zen  !-!-! ONLY RUN THIS ON ONE PORT OF YOUR SERVER !-! */
#if defined(SOE) && !defined(SOETEST) && !defined(SOEBLD)
    list_who_to_output();
#endif

    for (ch = first_char; ch != NULL; ch = ch->next) {
        if (ch->is_free != FALSE)
            continue;
        if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
            continue;

        if (ch->position >= POS_STUNNED) {
            if ((ch->hit < ch->max_hit)
                && (!IS_SET(ch->in_room->affected_by, ROOM_BV_NONE)))
                ch->hit += hit_gain(ch);
            ch->hit = UMAX(25, ch->hit);

            if ((ch->mana < ch->max_mana)
                && (!IS_SET(ch->in_room->affected_by, ROOM_BV_NONE)))
                ch->mana += mana_gain(ch);

            if ((ch->move < ch->max_move)
                && (ch->carry_weight < can_carry_w(ch)))
                ch->move += move_gain(ch);
            else if (ch->carry_weight >= can_carry_w(ch)) {
                send_to_char("You are carrying so much weight that you are @@eEXHAUSTED@@N!!\n\r", ch);
                ch->move = 0;
            }

        }

        if (ch->position == POS_STUNNED)
            update_pos(ch);
    }
    return;
}

/*
 * Update all chars, including mobs.
 * This function is performance sensitive.
 */
void
char_update(void)
{
    CHAR_DATA          *ch;
    CHAR_DATA          *ch_next;
    CHAR_DATA          *ch_save;
    CHAR_DATA          *ch_quit;
    time_t              save_time;

    save_time = current_time;
    ch_save = NULL;
    ch_quit = NULL;

    CREF(ch_next, CHAR_NEXT);
    for (ch = first_char; ch != NULL; ch = ch_next) {
        AFFECT_DATA        *paf;
        AFFECT_DATA        *paf_next;

        ch_next = ch->next;
        if (ch->is_free != FALSE)
            continue;

        /*
         * Find dude with oldest save time.
         */
        if (!IS_NPC(ch)
            && (ch->desc == NULL || ch->desc->connected == CON_PLAYING)
            && ch->level >= 2 && ch->save_time < save_time) {
            ch_save = ch;
            save_time = ch->save_time;
        }

        if ((IS_NPC(ch))
            && (ch->hit < -15))
            raw_kill(ch, "");

        if (ch->sitting != NULL && ch->sitting->in_room != ch->in_room) {
            ch->sitting->value[1]--;
            ch->sitting = NULL;
        }

        if (ch->position >= POS_STUNNED) {
            /* -S- mod. */
            if (ch->position != POS_WRITING && ch->position != POS_BUILDING) {
                if (IS_SET(ch->in_room->room_flags, ROOM_HOT))
                    send_to_char("You feel your skin burning.\n\r", ch);
                else if (IS_SET(ch->in_room->room_flags, ROOM_COLD))
                    send_to_char("You feel your skin freezing.\n\r", ch);
            }

        }

        /*    if ( ch->stunTimer > 0 )
           {
           ch->position = POS_STUNNED;
           ch->stunTimer -= 1;
           }
           else
           {
           ch->stunTimer = 0;
           ch->position = POS_STANDING;

           }  */

        if ((!IS_NPC(ch) && ch->level < LEVEL_IMMORTAL)) {

            OBJ_DATA           *obj;

            if ((obj = get_eq_char(ch, WEAR_LIGHT)) != NULL && obj->item_type == ITEM_LIGHT && obj->value[2] > 0) {
                if (--obj->value[2] == 0 && ch->in_room != NULL) {
                    --ch->in_room->light;
                    act("$p goes out.", ch, obj, NULL, TO_ROOM);
                    act("$p goes out.", ch, obj, NULL, TO_CHAR);
                    extract_obj(obj);
                }
            }

            if (++ch->timer >= 12) {
                if (ch->was_in_room == NULL && ch->in_room != NULL && ch->in_room->vnum != 1) {
                    ch->was_in_room = ch->in_room;
                    if (ch->fighting != NULL)
                        stop_fighting(ch, TRUE);
                    act("$n disappears into the void.", ch, NULL, NULL, TO_ROOM);
                    send_to_char("You disappear into the void.\n\r", ch);
                    save_char_obj(ch);
                    char_from_room(ch);
                    char_to_room(ch, get_room_index(ROOM_VNUM_LIMBO));
                }
            }

            if (ch->timer >= DUEL_TIMEOUT_IDLE && !IS_NPC(ch) && is_in_duel(ch, DUEL_STAGE_GO)) {
                cancel_duel(find_duel(ch), ch, DUEL_END_IDLE);
            }

            if (ch->timer > 30)
                ch_quit = ch;

            gain_condition(ch, COND_THIRST, 0 - number_range(1, 2));
            if (ch->pcdata->condition[COND_THIRST] <= 10)
                ch->pcdata->condition[COND_THIRST] = 10;
            gain_condition(ch, COND_DRUNK, 0 - number_range(1, 2));

            if (IS_NPC(ch) || (!IS_NPC(ch) && ch->pcdata->learned[gsn_nutrition_novice] == 0))
                gain_condition(ch, COND_FULL, 0 - number_range(1, 2));
        }

        for (paf = ch->first_affect; paf != NULL; paf = paf_next) {
            paf_next = paf->next;
            if (paf->duration > 0) {
                paf->duration--;

                if (paf->type == gsn_black_hand) {
                    if (paf->caster != NULL && !IS_NPC(paf->caster)) {
                        send_to_char("You feel the Black Hand choking you.\n\r", ch);
                        ch->hit -= paf->modifier;
                    }
                }
                if ((paf->type == gsn_adrenaline)
                    && (ch->fighting == NULL)
                    && (ch->hit > 10)) {
                    ch->hit = UMAX(10, (ch->hit - (paf->duration * 30)));
                    ch->move = UMAX(10, (ch->move - (paf->duration * 80)));
                    send_to_char("@@NYou feel the affects of your @@eadrenaline rush@@N wear off, leaving you exhausted.\n\r", ch);
                    affect_remove(ch, paf);
                }
            }
            else if (paf->duration < 0);
            else {
                if (paf_next == NULL || paf_next->type != paf->type || paf_next->duration > 0) {

                    if (IS_CLOAK_SN(paf->type)) {
                        int sn = GET_INDEX_CLOAK(paf->type);

                        act(cloak_table[sn].remove_self, ch, NULL, NULL, TO_CHAR);
                        act(cloak_table[sn].remove_room, ch, NULL, NULL, TO_ROOM);
                    }
                    else {
                        if (paf->type > 0 && skill_table[paf->type].msg_off) {
                            send_to_char(skill_table[paf->type].msg_off, ch);
                            send_to_char("\n\r", ch);
                        }

                        if (paf->type == gsn_change_sex && paf->location == APPLY_SEX && skill_table[paf->type].room_off) {
                            ch->sex -= paf->modifier;
                            act(skill_table[paf->type].room_off, ch, NULL, NULL, TO_ROOM);
                            ch->sex += paf->modifier;
                        }
                        else if (paf->type > 0 && skill_table[paf->type].room_off)
                            act(skill_table[paf->type].room_off, ch, NULL, NULL, TO_ROOM);

                        if (paf->type == gsn_charm_person && ch->master != NULL) {
                            stop_follower(ch);
                            continue;
                        }
                    }
                }

                affect_remove(ch, paf);
            }
        }

        /*
         * Careful with the damages here,
         *   MUST NOT refer to ch after damage taken,
         *   as it may be lethal damage (on NPC).
         */
        if (IS_NPC(ch)) {
            if (ch->target != NULL && number_bits(4) == 0) {
                free_string(ch->target);
                ch->target = NULL;
            }

            /* charmies without a charm flag are extracted sooner */
            if (ch->extract_timer > 2 && !IS_AFFECTED(ch, AFF_CHARM)) {
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
                    case MOB_VNUM_ZOMBIE:
                        ch->extract_timer = 2;
                        break;
                    default:
                        break;
                }
            }

            if (ch->extract_timer > 0) {
                ch->extract_timer--;
            }
            else if (ch->extract_timer == 0) {
                /*            if ( IS_SET( ch->affected_by, AFF_CHARM ) )
                   {  */
                if ((ch->master == NULL)
                    || (ch->master->in_room == NULL)
                    || (ch->in_room != ch->master->in_room)) {
                    if (ch->in_room != NULL) {
                        do_say(ch, "Whaa?? Where am I? How did I get here?");
                        do_say(ch, "AHHH!!! Help me!!!! I'm MELTING......");
                    }
                    extract_char(ch, TRUE);
                    continue;
                }
                else {
                    if (number_range(0, 99) < get_pseudo_level(ch->master) - 25) {
                        CHAR_DATA          *this_master;

                        this_master = ch->master;
                        do_say(ch, "Whaa?? Where am I? How did I get here?");
                        do_scan(ch, "");
                        check_social(ch, "growl", ch->master->name);
                        do_say(ch, "How dare you order me around!!!");
                        stop_follower(ch);
                        affect_strip(ch, gsn_emount);
                        multi_hit(ch, this_master, TYPE_UNDEFINED);
                        continue;
                    }
                }
                /* }  */
            }
        }

        if (IS_AFFECTED(ch, AFF_POISON)) {
            act("$n shivers and suffers.", ch, NULL, NULL, TO_ROOM);
            send_to_char("You shiver and suffer.\n\r", ch);
            damage(ch, ch, number_range(2, 8), TYPE_UNDEFINED);
        }
        else if (ch->position == POS_INCAP) {
            damage(ch, ch, number_range(1, 4), TYPE_UNDEFINED);
        }
        else if (ch->position == POS_MORTAL) {
            damage(ch, ch, number_range(2, 3), TYPE_UNDEFINED);
        }
        else if (ch->position == POS_DEAD) {
            damage(ch, ch, number_range(5, 10), TYPE_UNDEFINED);
        }
        else if (ch->hit < -10) {
            damage(ch, ch, number_range(5, 10), TYPE_UNDEFINED);
        }

    }
    CUREF(ch_next);

    /*
     * Autosave and autoquit.
     * Check that these chars still exist.
     */
    if (ch_save != NULL || ch_quit != NULL) {
        CREF(ch_next, CHAR_NEXT);

        for (ch = first_player; ch != NULL; ch = ch_next) {
            ch_next = ch->next_player;
            if (ch == ch_save)
                save_char_obj(ch);
            if (ch == ch_quit) {
                send_to_char("Idle for too long.  Bye Bye!\n\r", ch);
                if ((ch->was_in_room) && IS_SET(ch->was_in_room->room_flags, ROOM_NO_QUIT)) {
                    ch->was_in_room = NULL;
                }
                do_quit(ch, "NOSAVECHECK");
            }
        }
        CUREF(ch_next);

    }

    return;
}

/* Check for objfuns.... this is probably performance sensitive too. */
void
objfun_update(void)
{
    OBJ_DATA           *obj;

    for (obj = first_obj; obj != NULL; obj = obj->next)
        if (obj->obj_fun != NULL) {
            /* if no one is carrying it and it's not in a room, then it doesn't really exist */
            if (obj->carried_by == NULL && obj->in_room == NULL)
                continue;

            (*obj->obj_fun) (obj, obj->carried_by);
        }

    return;
}

/*
 * Update all objs.
 * This function is performance sensitive.
 */
void
obj_update(void)
{

    OBJ_DATA           *marker;
    OBJ_DATA           *obj;
    extern OBJ_DATA    *auction_item;
    DUEL_DATA          *duel;
    DUEL_PLAYER_DATA   *player;
    DUEL_OBJ_DATA      *dobj;
    DUEL_OBJ_DATA      *dobj_next;

    /* cool off duel objects */
    for (duel = first_duel; duel != NULL; duel = duel->next)
        for (player = duel->first_player; player != NULL; player = player->next)
            for (dobj = player->first_obj; dobj != NULL; dobj = dobj_next) {
                dobj_next = dobj->next;

                if (dobj->obj && number_percent() < 25) {
                    act("Your $p @@acools off@@N!!", player->ch, dobj->obj, NULL, TO_CHAR);
                    DUNLINK(dobj, player->first_obj, player->last_obj, next, prev);
                    DESTROY_MEMBER(dobj);
                }
            }

    /*
     * Create dummy object and add to end of list.  This object is
     * only a marker, and will not actually be processed by this
     * routine.
     */
    GET_FREE(marker, obj_free);
    LINK(marker, first_obj, last_obj, next, prev);

    /*
     * Repeatedly remove object from front of list, add to tail, and process
     * until the marker is at the head of the list.  That will indicate all
     * objects have been processed.
     */
    disable_timer_abort = FALSE;
    while ((obj = first_obj) != marker) {

        CHAR_DATA          *rch;
        char               *message;

        UNLINK(obj, first_obj, last_obj, next, prev);
        LINK(obj, first_obj, last_obj, next, prev);

        if (obj == auction_item)
            continue;
        if ((IS_SET(obj->item_apply, ITEM_APPLY_HEATED) || IS_SET(obj->item_apply, ITEM_APPLY_ARENAHEATED))
            && number_range(0, 100) < 25) {
            if (IS_SET(obj->item_apply, ITEM_APPLY_HEATED))
                REMOVE_BIT(obj->item_apply, ITEM_APPLY_HEATED);
            if (IS_SET(obj->item_apply, ITEM_APPLY_ARENAHEATED))
                REMOVE_BIT(obj->item_apply, ITEM_APPLY_ARENAHEATED);

            if (obj->carried_by != NULL) {
                act("Your $p @@acools off@@N!!", obj->carried_by, obj, NULL, TO_CHAR);
            }
            else if (obj->in_room != NULL && (rch = obj->in_room->first_person) != NULL) {
                act("$p @@acools off@@N!!", rch, obj, NULL, TO_ROOM);
                act("$p @@acools off@@N!!", rch, obj, NULL, TO_CHAR);
            }

        }

        /* empty player corpses disappear sooner */
        if (obj->item_type == ITEM_CORPSE_PC && obj->first_in_carry_list == NULL && obj->timer > 1)
            obj->timer = 1;

        if (obj->timer <= 0 || --obj->timer > 0)
            continue;
        switch (obj->item_type) {
            default:
                message = "$p vanishes.";
                break;
            case ITEM_FOUNTAIN:
                message = "$p dries up.";
                break;
            case ITEM_CORPSE_NPC:
                message = "$p decays into dust.";
                break;
            case ITEM_CORPSE_PC:
                message = "$p vapourises, and goes to heaven.";
                break;
            case ITEM_PORTAL:
                message = "$p implodes suddenly.";
                break;
            case ITEM_FOOD:
                message = "$p decomposes.";
                break;
        }

        if (obj->carried_by != NULL) {
            act(message, obj->carried_by, obj, NULL, TO_CHAR);
        }
        else if (obj->in_room != NULL && (rch = obj->in_room->first_person) != NULL) {
            act(message, rch, obj, NULL, TO_ROOM);
            act(message, rch, obj, NULL, TO_CHAR);
        }
        /*
           if ( obj->in_room == NULL )
           continue; 
           if ( obj->item_type == ITEM_CORPSE_NPC )
           continue;
         */
        extract_obj(obj);
    }

    /*
     * All objects have been processed.  Remove the marker object and
     * put it back on the free list.
     */
    UNLINK(marker, first_obj, last_obj, next, prev);
    PUT_FREE(marker, obj_free);

    disable_timer_abort = FALSE;
    return;
}

/*
 * Aggress.
 *
 * for each mortal PC
 *     for each mob in room
 *         aggress on some random PC
 *
 * This function takes 25% to 35% of ALL Merc cpu time.
 * Unfortunately, checking on each PC move is too tricky,
 *   because we don't the mob to just attack the first PC
 *   who leads the party into the room.
 *
 * -- Furey
 */
void
aggr_update(void)
{

    /* Check to see if ch has encountered a mob with ACT_REMEMBER set,
     * and with victim->target == ch->name...    tbc ;)
     * -- Stephen
     */

    CHAR_DATA          *wch;
    CHAR_DATA          *wch_next;
    CHAR_DATA          *ch;
    CHAR_DATA          *ch_next;
    CHAR_DATA          *vch;
    CHAR_DATA          *vch_next;
    CHAR_DATA          *victim;
    OBJ_DATA           *wield;
    MPROG_ACT_LIST     *mpact, *mpact_next;
    extern CHAR_DATA   *quest_mob;

    for (mpact = first_mpact; mpact != NULL; mpact = mpact_next) {
        mpact_next = mpact->next;
        wch = mpact->mob;

        if (wch == NULL || wch->is_free != FALSE || wch->mpactnum == 0)
            continue;

        mprog_wordlist_check(mpact->buf, wch, mpact->ch, mpact->obj, mpact->vo, ACT_PROG);
        UNLINK(mpact, first_mpact, last_mpact, next, prev);
        free_string(mpact->buf);
        PUT_FREE(mpact, mpact_free);
    }

    for (wch = first_player; wch != NULL; wch = wch_next) {
        wch_next = wch->next_player;

        if (wch->is_free != FALSE)
            continue;

        if (wch->in_room == NULL)
            continue;

        for (ch = wch->in_room->first_person; ch != NULL; ch = ch_next) {
            int                 count;

            ch_next = ch->next_in_room;

            if (!IS_NPC(ch)
                || ((!IS_SET(ch->act, ACT_AGGRESSIVE) && !can_see(ch, wch))
                    && (!IS_SET(ch->act, ACT_ALWAYS_AGGR))
                )
                || ch->fighting != NULL || ch->hunting != NULL || IS_AFFECTED(ch, AFF_CHARM)
                || !IS_AWAKE(ch)
                || ch == quest_mob || (IS_SET(ch->act, ACT_WIMPY) && IS_AWAKE(wch)))
                continue;

            if ((IS_AFFECTED(wch, AFF_SNEAK) || item_has_apply(wch, ITEM_APPLY_SNEAK))
                && (number_percent() < 50 + (2 * (get_pseudo_level(wch) - get_pseudo_level(ch)))))
                continue;

            if (!IS_NPC(wch) && wch->stance == STANCE_AMBUSH && wch->pcdata->stealth == gsn_stealth_master)
                continue;

            /*
             * Ok we have a 'wch' player character and a 'ch' npc aggressor.
             * MAG - wch can be an intelligent NPC.
             * Now make the aggressor fight a RANDOM pc victim in the room,
             *   giving each 'vch' an equal chance of selection.
             */
            count = 0;
            victim = NULL;
            CREF(vch_next, CHAR_NEXTROOM);
            for (vch = wch->in_room->first_person; vch != NULL; vch = vch_next) {
                vch_next = vch->next_in_room;

                if (   !IS_NPC(vch)
                    && (vch->level < LEVEL_IMMORTAL)
                    && (!IS_SET(ch->act, ACT_WIMPY) || !IS_AWAKE(vch))
                    && ((IS_SET(ch->act, ACT_AGGRESSIVE)
                            && !IS_SET(ch->act, ACT_ALWAYS_AGGR)
                            && can_see(ch, vch))
                        || (IS_SET(ch->act, ACT_ALWAYS_AGGR))
                    )
                    ) {
                    if (number_range(0, count) == 0)
                        victim = vch;
                    count++;
                }
            }
            CUREF(vch_next);
            if (victim == NULL) {
                continue;
            }
            if (IS_SET(victim->in_room->room_flags, ROOM_SAFE))
                continue;

            act("$n growls at $N!", victim, NULL, ch, TO_NOTVICT);
            act("$N growls at you!  Uh-oh!!", victim, NULL, ch, TO_CHAR);
            act("You growl at $N.  Get $M!!", ch, NULL, victim, TO_CHAR);

            wield = get_eq_char(ch, WEAR_WIELD);
            if (wield != NULL && wield->item_type == ITEM_WEAPON && wield->value[3] == 11 && victim->fighting == NULL && can_see(ch, victim))
                do_backstab(ch, victim->name);
            else
                multi_hit(ch, victim, TYPE_UNDEFINED);
        }
    }

    return;
}

/*
 * Check ALL rooms for affects... the ratio of affects to rooms should
 * be relatively low, so this shouldn't hit performance too much.
 * -S-
 */

void
rooms_update(void)
{
    ROOM_INDEX_DATA    *room;
    AREA_DATA          *area;
    BUILD_DATA_LIST    *thing;
    ROOM_AFFECT_DATA   *raf;
    ROOM_AFFECT_DATA   *raf_next;

    for (area = first_area; area != NULL; area = area->next) {
        for (thing = area->first_area_room; thing != NULL; thing = thing->next) {
            room = thing->data;

            if (room == NULL)
                continue;

            for (raf = room->first_room_affect; raf != NULL; raf = raf_next) {
                raf_next = raf->next;

                if ((raf->bitvector == ROOM_BV_WARNING_RUNE || raf->bitvector == ROOM_BV_SENTRY) && raf->caster == NULL) {
                    r_affect_remove(room, raf);
                    continue;
                }

                if (raf->duration > 0)
                    raf->duration--;
                else if (raf->duration < 0);
                else {
                    if (raf->bitvector == ROOM_BV_WARNING_RUNE || raf->bitvector == ROOM_BV_SENTRY) {
                        char                buf[MSL];

                        sprintf(buf, skill_table[raf->type].msg_off, raf->name);
                        send_to_char(buf, raf->caster);

                        r_affect_remove(room, raf);
                    }
                    else if (raf_next == NULL || raf_next->type != raf->type || raf_next->duration > 0) {
                        if (raf->type > 0 && skill_table[raf->type].msg_off)
                            send_to_room(skill_table[raf->type].msg_off, room);

                        r_affect_remove(room, raf);
                    }
                }
            }
        }
    }
    return;
}

/* teleport */
void
teleport_update(void)
{
    char                buf[MAX_STRING_LENGTH];
    TELEPORT_DATA      *tele;
    ROOM_INDEX_DATA    *room = NULL;
    CHAR_DATA          *ch;
    CHAR_DATA          *chnext;
    bool                reset = FALSE;

    buf[0] = '\0';

    for (tele = last_tele; tele; tele = tele->prev) {
        if ((room = get_room_index(tele->room->vnum)) == NULL)
            continue;

        reset = FALSE;

        for (ch = room->first_person; ch != NULL; ch = chnext) {
            chnext = ch->next_in_room;

            if (reset) {
                ch->tele_timer = tele->wait - 1;
                continue;
            }

            if (ch->tele_timer-- <= 0) {
                if ((room_teleport(tele, ch)) && (IS_SET(tele->flags, TEL_SOLITARY))) {
                    reset = TRUE;
                }
            }
        }
    }

    return;
}

/* teleport */
bool
room_teleport(TELEPORT_DATA *tele, CHAR_DATA *ch)
{
    ROOM_INDEX_DATA    *roomto = NULL;
    BUF_DATA_STRUCT    *buf_data = NULL;

    if ((roomto = get_room_index(tele->vnum)) == NULL)
        return FALSE;

    /* private room, EEK! */
    if (room_is_private(roomto))
        return FALSE;

    /* teleporter flagged nomob, dont allow mobs! (charmies allowed) */
    if (IS_SET(tele->flags, TEL_NOMOB) && IS_NPC(ch) && !IS_AFFECTED(ch, AFF_CHARM))
        return FALSE;

    /* teleporter flagged nopc, dont allow people! */
    if (IS_SET(tele->flags, TEL_NOPC) && !IS_NPC(ch))
        return FALSE;

    /* teleporter flagged nocharm, charmies will stay there! */
    if (IS_SET(tele->flags, TEL_NOCHARM) && IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
        return FALSE;

    /* charmies wait for master if room is flagged for not fighting, and charmwait is set */
    if ((!IS_SET(tele->flags, TEL_FIGHTING))
        && (IS_SET(tele->flags, TEL_CHARMWAIT))
        && (IS_AFFECTED(ch, AFF_CHARM))
        && (ch->master != NULL)
        && (ch->master->in_room == ch->in_room)
        && (ch->master->position == POS_FIGHTING))
        return FALSE;

    /* person is building/fighting and fighting isnt set, they can stay */
    /* took out writing and assume theres no teleporters with boards in them */

    if (ch->position == POS_BUILDING || (ch->position == POS_FIGHTING && (!IS_SET(tele->flags, TEL_FIGHTING)))
        )
        return FALSE;

    /* check to see if person is writing, and if they are, check to see if they're writing
     * in relation to a builder command */
    for (buf_data = first_buf; buf_data != NULL; buf_data = buf_data->next)
        if (buf_data->ch == ch && buf_data->old_char_pos == POS_BUILDING)
            return FALSE;

    /* person is fighting and are allowed through, so stop em fighting! */
    if (ch->position == POS_FIGHTING && (IS_SET(tele->flags, TEL_FIGHTING)))
        stop_fighting(ch, FALSE);

    if (tele->in != NULL && tele->in[0] != '\0')
        send_to_char(tele->in, ch);

    char_from_room(ch);
    char_to_room(ch, roomto);

    if (tele->out != NULL && tele->out[0] != '\0')
        act(tele->out, ch, NULL, NULL, TO_NOTVICT);

    do_look(ch, "auto");

    return TRUE;
}

extern void         build_save_flush(void);

/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */
void
update_handler(void)
{
#if 0
    static int          pulse_message;
#endif

    static int          objfun_check;
    static int          pulse_area;
    static int          pulse_rooms;
    static int          pulse_mobile;
    static int          pulse_gain;
    static int          pulse_violence;

    /* teleport */
    static int          pulse_teleport;

    static int          pulse_point;
    static int          pulse_auction;
    extern int          saving_area;
    extern bool         auction_flop;

    if (saving_area)
        build_save();            /* For incremental area saving */

    if (--pulse_area <= 0) {
        pulse_area = number_range(PULSE_AREA / 2, 3 * PULSE_AREA / 2);
#ifndef BPORT
        area_update();
#endif
        build_save_flush();
    }

    if (--pulse_rooms <= 0) {
        pulse_rooms = PULSE_ROOMS;
        rooms_update();
    }

    /* teleport */
    if (--pulse_teleport <= 0) {
        pulse_teleport = PULSE_TELEPORT;
        teleport_update();
        energy_update();
        duel_update();            /* in duel.c */
        note_update();            /* teleport happens once a sec, so we'll put it here */
        desert_update();        /* teleport happens once a sec, so we'll put it here */
        dns_update();
        logfile_update();
        auc_update();            /* gauction/qauction updates */
    }

#if 0
    if (--pulse_message <= 0) {
        pulse_message = PULSE_MESSAGE;
        message_update();
    }
#endif

    if (auction_flop) {
        pulse_auction = PULSE_AUCTION;
        auction_flop = FALSE;
    }

    if (--pulse_auction <= 0) {
        pulse_auction = PULSE_AUCTION;
        auction_update();
    }

    if (--objfun_check <= 0) {
        objfun_check = PULSE_OBJFUN;
        objfun_update();
    }

    if (--pulse_violence <= 0) {
        alarm_update();
        pulse_violence = PULSE_VIOLENCE;
        violence_update();
    }

    if (--pulse_mobile <= 0) {
        pulse_mobile = PULSE_MOBILE;
        mobile_update();
    }

    if (--pulse_gain <= 0) {
        gain_update();
        pulse_gain = PULSE_PER_SECOND * number_range(5, 8);
    }

    if (--pulse_point <= 0) {
        pulse_point = PULSE_TICK;
        weather_update();
        char_update();
        obj_update();
        quest_update();

        /* This will log the number of perms being used...
         * fgrep the log file to get results...
         */

        /*   perm_update( ); */
    }

    aggr_update();
    tail_chain();
    return;
}

bool
check_rewield(CHAR_DATA *ch)
{
    OBJ_DATA           *obj;
    OBJ_DATA           *weapon = NULL;
    int                 dam;
    int                 chance;
    bool                pickup;
    char                buf[MAX_STRING_LENGTH];
    extern bool         quest;
    extern bool         auto_quest;

    pickup = TRUE;
    dam = 0;

    chance = (ch->fighting == NULL ? 35 : 60);

    if (number_percent() < chance) {
        for (obj = ch->first_carry; obj != NULL; obj = obj->next_in_carry_list)
            if (obj->item_type == ITEM_WEAPON && dam < obj->value[2]) {
                dam = obj->value[2];
                pickup = FALSE;
                weapon = obj;
            }

        /* Then check inventory and room for any weapons */
        for (obj = ch->in_room->first_content; obj != NULL; obj = obj->next_in_room) {
            if (obj->item_type == ITEM_WEAPON) {
                if (obj->value[2] > dam) {
                    dam = obj->value[2];
                    weapon = obj;
                    pickup = TRUE;
                }
            }
        }

        if (weapon == NULL)
            return FALSE;

        if (weapon->wear_loc == WEAR_WIELD)
            return FALSE;

        if ((quest || auto_quest) && weapon == quest_object)
            return FALSE;

        if (pickup) {
            sprintf(buf, "Great!  %s!  Just what i've always wanted!", weapon->short_descr);
            do_say(ch, buf);
        }

        if (weapon != NULL) {
            /* Now make the mob get the weapon */
            if (pickup)
                get_obj(ch, weapon, NULL);

            do_wear(ch, weapon->name);

            /* Check is mob wielded weapon ok... */
            if (weapon->wear_loc == WEAR_NONE && weapon != quest_object) {
                act("$n sniffs sadly.  'Baah!  It's no good to me!'", ch, NULL, NULL, TO_ROOM);
                extract_obj(weapon);
                act("$n sacrifices $p.", ch, weapon, NULL, TO_ROOM);
            }
            return TRUE;
        }

    }

    return FALSE;
}

bool
check_re_equip(CHAR_DATA *ch)
{
    OBJ_DATA           *obj;
    OBJ_DATA           *obj2;
    OBJ_DATA           *armor = NULL;
    OBJ_DATA           *light = NULL;
    int                 ac;
    int                 chance;
    bool                pickup;
    bool                ident;
    int                 best;
    char                buf[MAX_STRING_LENGTH];
    int                 objnum;
    extern bool         quest;
    extern bool         auto_quest;

    best = -1;
    pickup = TRUE;
    ac = 0;

    chance = (ch->fighting == NULL ? 35 : 60);
    if (number_percent() < chance) {
        /* Check each armor in room against ch's equipment */

        ident = FALSE;
        for (obj = ch->in_room->first_content; obj != NULL; obj = obj->next_in_room) {
            if (!can_see_obj(ch, obj))
                continue;

            if ((obj->wear_flags & ~ITEM_TAKE) == 0)
                continue;        /* Check to see if item cannot be worn */
            if (obj->item_type == ITEM_PIECE)
                continue;

            if ((quest || auto_quest) && obj == quest_object)
                continue;

            if (obj->item_type == ITEM_ARMOR) {
                /* Check this object against our equiped objects */
                ident = FALSE;
                for (obj2 = ch->first_carry; obj2 != NULL; obj2 = obj2->next_in_carry_list) {
                    /* Only scan against worn objects. 
                     * If obj2 is being worn in a position that obj can be worn in,
                     * and obj2->value[0] is better, then choose it.
                     */

                    if (obj2->wear_loc != WEAR_NONE && obj2->item_type == ITEM_ARMOR && can_wear_at(ch, obj, obj2->wear_loc)
                        && obj->value[0] > obj2->value[0]) {
                        ident = TRUE;    /* identical wear_loc */
                        armor = obj;
                        break;
                    }
                }

                /* Found no match for locations, so get and wear. */
                if (!ident) {
                    armor = obj;
                    break;
                }
            }

            if (obj->item_type == ITEM_LIGHT && (get_eq_char(ch, WEAR_LIGHT) == NULL)) {
                light = obj;
                break;
            }
        }

        /* MAG Modification. Only check one item each time, against currently
           worn object. */

        /* Check one inv item against worn eq, incase we've picked up some nicer
           stuff */

        objnum = number_percent() * ch->carry_number / 100;
        for (obj = ch->first_carry; obj != NULL && objnum > 0; obj = obj->next_in_carry_list)
            objnum--;

        if (obj != NULL && obj->wear_loc == WEAR_NONE && obj->item_type == ITEM_ARMOR) {
            ident = FALSE;
            for (obj2 = ch->first_carry; obj2 != NULL; obj2 = obj2->next_in_carry_list) {
                if (obj2->wear_loc != WEAR_NONE && can_wear_at(ch, obj, obj2->wear_loc)
                    && obj->value[0] > obj2->value[0]) {
                    ident = TRUE;
                    armor = obj;
                    break;
                }
            }
            if (!ident) {
                armor = obj;
            }
        }

        if (obj != NULL && obj->item_type == ITEM_LIGHT && (get_eq_char(ch, WEAR_LIGHT) == NULL)) {
            light = obj;
        }
    }

    if (armor != NULL) {
        if (armor->carried_by != ch) {
            /* Pick up off ground */
            if (pickup) {
                sprintf(buf, "Great!  %s!  Just what i've always wanted!", armor->short_descr);
                do_say(ch, buf);
            }

            /* Now make the mob get the armor */
            if (pickup)
                get_obj(ch, armor, NULL);
        }

        do_wear(ch, armor->name);

        /* Check is mob wielded weapon ok... */
        if (armor->wear_loc == WEAR_NONE && armor != quest_object) {
            act("$n sniffs sadly.  'Baah!  It's no good to me!'", ch, NULL, NULL, TO_ROOM);
            extract_obj(armor);
            act("$n sacrifices $p.", ch, armor, NULL, TO_ROOM);
        }
        return TRUE;
    }

    if (light != NULL) {
        if (light->carried_by != ch) {
            /* Pick up off ground */
            if (pickup) {
                sprintf(buf, "Great!  %s!  Just what i've always wanted!", light->short_descr);
                do_say(ch, buf);
            }

            /* Now make the mob get the light */
            if (pickup)
                get_obj(ch, light, NULL);
        }

        do_wear(ch, light->name);

        /* Check is mob wielded weapon ok... */
        if (light->wear_loc == WEAR_NONE && light != quest_object) {
            act("$n sniffs sadly.  'Baah!  It's no good to me!'", ch, NULL, NULL, TO_ROOM);
            extract_obj(light);
            act("$n sacrifices $p.", ch, light, NULL, TO_ROOM);
        }
        return TRUE;
    }

    return FALSE;
}

void
auction_update(void)
{
    extern OBJ_DATA    *auction_item;
    extern CHAR_DATA   *auction_owner;
    extern CHAR_DATA   *auction_bidder;
    extern int          auction_bid;
    extern int          auction_reserve;
    extern int          auction_stage;
    char                buf[MAX_STRING_LENGTH];

    CHAR_DATA          *ach;
    bool                good_seller = FALSE, good_buyer = FALSE;

    /* Stages: 0) No/New bid.  
       1) Waiting.  (If no bid here, then give up next time)
       2) Going once.  
       3) Going Twice.  
       4) GONE! 
     */

    if (auction_item == NULL)
        return;

    switch (auction_stage) {
        case 0:
            if (auction_bidder == NULL) {
                sprintf(buf,
                    "@@N%s (level: %d, valued at %s GP) has been offered for auction.  A @@e10%% fee@@N will be charged, the higher of the reserve price or highest bid.",
                    auction_item->short_descr, auction_item->level, number_comma(auction_item->cost));
            }
            else {
                sprintf(buf, "%s has bid %s for %s.", auction_bidder->name, number_comma(auction_bid), auction_item->short_descr);
            }
            break;
        case 1:
            if (auction_bidder == NULL)
                sprintf(buf, "Last chance to bid for %s.", auction_item->short_descr);
            else
                sprintf(buf, "Last bid for %s was @@y%s@@N by %s.  Any more offers?",
                    auction_item->short_descr, number_comma(auction_bid), auction_bidder->name);
            break;
        case 2:
            if (auction_bidder == NULL) {
                auction("No bidders.  Auction Ended.");

                for (ach = first_char; ach != NULL; ach = ach->next) {
                    if (auction_owner == ach)
                        good_seller = TRUE;
                    if (auction_bidder == ach)
                        good_buyer = TRUE;
                }

                if (good_seller) {
                    auction_owner->gold = UMAX(0, auction_owner->gold - abs(auction_reserve * .1));

                    obj_to_char(auction_item, auction_owner);
                }
                else {
                    auction("Oh, well..guess they didn't want it anymore, since they LEFT!!  Well, it's mine now! ");
                    extract_obj(auction_item);
                }
                auction_item = NULL;
                return;
            }
            sprintf(buf, "%s - Going Once for @@y%s@@N to %s!", auction_item->short_descr, number_comma(auction_bid), auction_bidder->name);
            break;
        case 3:
            if (auction_bidder) {
                sprintf(buf, "%s - Going Twice for @@y%s@@N to %s!", auction_item->short_descr, number_comma(auction_bid), auction_bidder->name);
            }
            break;
        case 4:
            if (auction_bid < auction_reserve) {
                for (ach = first_char; ach != NULL; ach = ach->next) {
                    if (auction_owner == ach)
                        good_seller = TRUE;
                    if (auction_bidder == ach)
                        good_buyer = TRUE;
                }

                sprintf(buf, "%s - CANCELLED.  Reserve price not matched.", auction_item->short_descr);
                if (good_seller) {
                    auction_owner->gold = UMAX(0, auction_owner->gold - abs(auction_reserve * .1));

                    obj_to_char(auction_item, auction_owner);
                }
                else
                    extract_obj(auction_item);
                if (good_buyer)
                    auction_bidder->gold += auction_bid;

            }
            else {

                for (ach = first_char; ach != NULL; ach = ach->next) {
                    if (auction_owner == ach)
                        good_seller = TRUE;
                    if (auction_bidder == ach)
                        good_buyer = TRUE;
                }

                if (good_buyer) {
                    sprintf(buf, "%s - SOLD! to %s for @@y%s@@N GP.", auction_item->short_descr, auction_bidder->name, number_comma(auction_bid));

                    obj_to_char(auction_item, auction_bidder);
                }
                else {
                    sprintf(buf, "%s - SOLD!, but the buyer has left us.  Oh Well!!!", auction_item->short_descr);
                    extract_obj(auction_item);
                }
                if (good_seller)
                    auction_owner->gold += (auction_bid - (auction_bid * .1));

            }

            auction_stage = 0;
            auction_bidder = NULL;
            auction_owner = NULL;
            auction_item = NULL;
            auction_reserve = 0;
            auction_bid = 0;
            break;
    }
    auction(buf);
    auction_stage++;
    return;
}

void
remember_attack(CHAR_DATA *ch, CHAR_DATA *victim)
{
    /* Called when an NPC ch encounters a PC victim, that tried to
     * kill it previously.
     * --Stephen
     */

    char                buf[MAX_STRING_LENGTH];

    /* Pick a random response for ch to give, before attacking */

    switch (number_range(0, 7)) {
        case 0:
            sprintf(buf, "%s returns!  I shall have my revenge at last!", victim->name);
            do_yell(ch, buf);
            break;
        case 1:
            sprintf(buf, "%s You should never have returned.  Ye shall DIE!", victim->name);
            do_whisper(ch, buf);
            break;
        case 2:
            act("$n looks at $N, remembering $S attack", ch, NULL, victim, TO_ROOM);
            act("$n looks at you, remembering your attack", ch, NULL, victim, TO_VICT);
            act("You look at $N, remembering $S attack.", ch, NULL, victim, TO_CHAR);
            do_say(ch, "I SHALL HAVE MY REVENGE!!!");
            break;
        case 3:
            sprintf(buf, "%s has wronged me, and now I will seek my revenge!", victim->name);
            do_gossip(ch, buf);
            sprintf(buf, "Prepare to die, %s.", victim->name);
            do_say(ch, buf);
            break;
        case 4:
            sprintf(buf, "So, %s.  You have returned.  Let us finish our fight this time!", victim->name);
            do_say(ch, buf);
            break;
        case 5:
            sprintf(buf, "Only cowards flee from me, %s!", victim->name);
            do_say(ch, buf);
            break;
        case 6:
            act("$n looks at $N, and recognizes $M!!", ch, NULL, victim, TO_NOTVICT);
            act("$n looks at you, and recognizes you!!", ch, NULL, victim, TO_VICT);
            act("You look at $N, and recognize $M!", ch, NULL, victim, TO_CHAR);
            sprintf(buf, "There can only be one winner, %s.", victim->name);
            do_say(ch, buf);
            break;
    }

    /* Check if has intelligence, and call correct attack? */

    multi_hit(ch, victim, TYPE_UNDEFINED);
    /* spec- plug leak here */
    if (ch->target) {
        free_string(ch->target);
        ch->target = NULL;
    }
    return;
}

void
quest_update()
{
    extern bool         auto_quest;
    extern bool         quest;
    extern CHAR_DATA   *quest_mob;
    extern OBJ_DATA    *quest_object;
    extern int          quest_timer;
    extern int          quest_wait;

    if (!quest && !auto_quest)
        return;

    if (quest) {
        /* Make sure the mobile and obj still exist! */
        if (quest_mob == NULL || quest_object == NULL) {
            quest_cancel();
            return;
        }

        quest_inform();
        if (quest_timer > 15)
            quest_cancel();
        return;
    }

    if (!quest) {
        if (quest_wait > 0) {
            quest_wait--;
            return;
        }

        if (auto_quest)
            generate_auto_quest();
    }
}

void energy_advance(CHAR_DATA *ch);

void energy_update(void)
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA       *ch;

    for (d = first_desc; d; d = d->next) {
        ch = d->original ? d->original : d->character;

        if (d->connected != CON_PLAYING)
            continue;

        if (ch->max_energy == 0)
            continue;

        if (ch->in_room && ch->in_room->area && IS_SET(ch->in_room->area->flags, AREA_NOENERGY))
            continue;

        if (ch->in_room && IS_SET(ch->in_room->room_flags, ROOM_SAFE))
            continue;

        if (ch->stance == STANCE_AMBUSH && ch->pcdata->stealth > 0 && ch->energy > 0) {
            ch->energy--;
            ch->pcdata->energy_used++;
            energy_advance(ch);
        }

        if (ch->energy_wait_count > 0 && (ch->stance != STANCE_AMBUSH || ch->pcdata->stealth == 0))
            ch->energy_wait_count--;

        if (ch->energy_wait_count == 0 && ch->energy < ch->max_energy) {
            ch->energy++;
            ch->energy_wait_count = ch->energy_wait;
        }

        if (ch->stance == STANCE_AMBUSH && ch->pcdata->stealth > 0 && ch->energy <= 0) {
            send_to_char("You step out of the shadows.\n\r", ch);
            ch->stance = STANCE_WARRIOR;
            ch->stance_ac_mod = 0;
            ch->stance_dr_mod = 0;
            ch->stance_hr_mod = 0;
            act("$n steps out of the Shadows!", ch, NULL, NULL, TO_ROOM);
        }
    }

    return;
}

void
note_update()
{
    DESCRIPTOR_DATA    *d;
    CHAR_DATA          *ch;

    for (d = first_desc; d; d = d->next) {
        ch = d->original ? d->original : d->character;

        if (d->connected != CON_PLAYING)
            continue;

        /* pop safe timer checking here */
        if (!IS_NPC(ch) && ch->pcdata->safetimer > 0 && current_time >= ch->pcdata->safetimer) {
            act("Your protection from player fighting wears off.", ch, NULL, NULL, TO_CHAR);
            act("$n's protection from player fighting wears off.", ch, NULL, NULL, TO_ROOM);
            ch->pcdata->safetimer = 0;
        }
    }

    for (ch = first_player; ch; ch = ch->next_player) {
        /* pop fight timer decrease here */
        if (!IS_NPC(ch) && ch->pcdata->fighttimer > 0 && ch->fighting == NULL)
            ch->pcdata->fighttimer--;
    }

    return;
}

void
desert_update(void)
{
    DESCRIPTOR_DATA    *d;
    CHAR_DATA          *ch;
    char                buf[MSL];
    char                durbuf[64];

    for (d = first_desc; d; d = d->next) {
        ch = d->original ? d->original : d->character;

        if (d->connected != CON_PLAYING)
            continue;

        durbuf[0] = 0;

        if (!IS_NPC(ch) && ch->pcdata->desert_time > 0) {
            if (current_time >= ch->pcdata->desert_time
                || (!IS_SET(ch->pcdata->pflags, PFLAG_CLAN_DESERTER) && !IS_SET(ch->pcdata->pflags, PFLAG_CLAN_LEAVER))
                ) {
                sprintf(buf, "You are no longer a %s, you may join another clan!\n\r", (IS_SET(ch->pcdata->pflags, PFLAG_CLAN_DESERTER))
                    ? "Deserter" : "Leaver");

                send_to_char(buf, ch);
                REMOVE_BIT(ch->pcdata->pflags, PFLAG_CLAN_DESERTER);
                REMOVE_BIT(ch->pcdata->pflags, PFLAG_CLAN_LEAVER);
                ch->pcdata->desert_time = (time_t) 0;
                save_char_obj(ch);
            }
            else if ((ch->pcdata->desert_time - current_time) % 3600 == 0) {
                sprintf(buf, "You have %s left of being a %s.\n\r",
                    duration(ch->pcdata->desert_time - current_time, durbuf), (IS_SET(ch->pcdata->pflags, PFLAG_CLAN_DESERTER))
                    ? "Deserter" : "Leaver");

                send_to_char(buf, ch);
            }
        }

        if (!IS_NPC(ch) && ch->pcdata->accept_time > 0) {
            if (current_time >= ch->pcdata->accept_time) {
                ch->pcdata->accept_time = (time_t) 0;
                send_to_char("Clan invitation expired..\n\r", ch);
                free_string(ch->pcdata->accept_name);
                ch->pcdata->accept_name = str_dup("");
            }
        }
    }

    return;
}

void
logfile_update(void)
{
    char                logfilename[MSL];

    if (current_time - 86401 > today_time) {
        today_time = get_today_ctime();

        sprintf(logfilename, "../log/acklog.%s", get_today_string(today_time));
        if (freopen(logfilename, "a", stdout) == NULL)
            FPRINTF(stderr, "ERROR: reopening %s in logfile_update()\n", logfilename);

        setlinebuf(stdout);
    }

    return;
}
