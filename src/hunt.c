#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include "merc.h"

IDSTRING(rcsid, "$Id: hunt.c,v 1.12 2004/01/23 22:08:09 dave Exp $");

#define NEVER_FREE_HUNT

struct h_queue
{
    struct h_queue     *next;
    ROOM_INDEX_DATA    *room;
    sh_int              dir;
};

struct h_queue     *h_head = NULL;
struct h_queue     *h_tail = NULL;
extern char        *const dir_name[];

#ifdef NEVER_FREE_HUNT
struct h_queue     *h_free = NULL;
void                setup_hunt(void);
#endif

#ifdef DEBUG_HUNT_CODE
static FILE        *h_fp;
#endif

void
h_enqueue(ROOM_INDEX_DATA *room, sh_int dir)
{
    struct h_queue     *hunt;

#ifdef NEVER_FREE_HUNT
    if (h_free) {
        hunt = h_free;
        h_free = h_free->next;
    }
    else
#endif
        hunt = getmem(sizeof(*hunt));

    hunt->next = NULL;
    hunt->room = room;
    hunt->dir = dir;
    SET_BIT(room->room_flags, ROOM_HUNT_MARK);

    if (!h_head)
        h_head = hunt;
    else
        h_tail->next = hunt;
    h_tail = hunt;

#ifdef DEBUG_HUNT_CODE
    FPRINTF(h_fp, "Enqueue: %5d - %d\n", room->vnum, IS_SET(room->room_flags, ROOM_HUNT_MARK));
    fflush(h_fp);
#endif
    return;
}

void
h_dequeue(void)
{
    struct h_queue     *hunt;

    if (!(hunt = h_head))
        return;

    h_head = hunt->next;
    if (h_tail == hunt)
        h_tail = NULL;

    REMOVE_BIT(hunt->room->room_flags, ROOM_HUNT_MARK);
#ifdef DEBUG_HUNT_CODE
    FPRINTF(h_fp, "Dequeue: %5d\n", hunt->room->vnum);
    fflush(h_fp);
#endif
#ifdef NEVER_FREE_HUNT
    hunt->next = h_free;
    h_free = hunt;
#else
    dispose(hunt);
#endif
    return;
}

void
h_clear(void)
{
#ifdef DEBUG_HUNT_CODE
    FPRINTF(h_fp, "h_clear\n");
    fflush(h_fp);
#endif
    while (h_head != NULL)
        h_dequeue();
#ifdef DEBUG_HUNT_CODE
    FPRINTF(h_fp, "Cleared\n");
    fflush(h_fp);
#endif
}

bool
h_is_valid_exit(ROOM_INDEX_DATA *room, sh_int dir, int h_flags)
{
    EXIT_DATA          *exit = room->exit[dir];

    if (!exit)
        return FALSE;

    if (!exit->to_room)
        return FALSE;

#ifdef DEBUG_HUNT_CODE
    FPRINTF(h_fp, "IsValid: %5d - %s\n", exit->to_room->vnum, (IS_SET(exit->to_room->room_flags, ROOM_HUNT_MARK) ? "set" : "unset"));
    fflush(h_fp);
#endif
    if (IS_SET(exit->to_room->room_flags, ROOM_HUNT_MARK))
        return FALSE;

    if (!IS_SET(h_flags, HUNT_WORLD) && room->area != exit->to_room->area)
        return FALSE;

    if (IS_SET(exit->exit_info, EX_CLOSED)) {
        if (!IS_SET(h_flags, HUNT_OPENDOOR))
            return FALSE;

        if (IS_SET(exit->exit_info, EX_LOCKED)) {
            if (!IS_SET(h_flags, HUNT_UNLOCKDOOR | HUNT_PICKDOOR))
                return FALSE;
            if (!IS_SET(h_flags, HUNT_UNLOCKDOOR) && IS_SET(exit->exit_info, EX_PICKPROOF))
                return FALSE;
        }
    }

    return TRUE;
}

void
h_enqueue_room(ROOM_INDEX_DATA *room, sh_int dir, int h_flags)
{
    sh_int              edir;

#ifdef DEBUG_HUNT_CODE
    FPRINTF(h_fp, "h_enqueue_room\n");
    fflush(h_fp);
#endif

    for (edir = 0; edir < 6; edir++)
        if (h_is_valid_exit(room, edir, h_flags))
            h_enqueue(room->exit[edir]->to_room, (dir == -1 ? edir : dir));

    return;
}

sh_int
h_find_dir(ROOM_INDEX_DATA *room, ROOM_INDEX_DATA *target, int h_flags)
{
    struct h_queue     *hunt;

    if (room == target)
        return -1;

#ifdef DEBUG_HUNT_CODE
    if (!nosave && !h_fp)
        h_fp = fopen("hunt.out", "w");

    FPRINTF(h_fp, "h_find_dir\n");
    fflush(h_fp);
#endif

#ifdef NEVER_FREE_HUNT
    if (!h_free && !h_head && !h_tail)
        setup_hunt();
#endif

    SET_BIT(room->room_flags, ROOM_HUNT_MARK);
    h_enqueue_room(room, -1, h_flags);
    for (hunt = h_head; hunt; hunt = hunt->next) {
        if (hunt->room == target) {
            sh_int              dir = hunt->dir;

#ifdef DEBUG_HUNT_CODE
            FPRINTF(h_fp, "Found dir %d\n", dir);
            fflush(h_fp);
#endif
            h_clear();
            REMOVE_BIT(room->room_flags, ROOM_HUNT_MARK);
            return dir;
        }

        h_enqueue_room(hunt->room, hunt->dir, h_flags);
    }

#ifdef DEBUG_HUNT_CODE
    FPRINTF(h_fp, "Invalid dir\n");
    fflush(h_fp);
#endif

    h_clear();
    REMOVE_BIT(room->room_flags, ROOM_HUNT_MARK);
    return -1;
}

bool
set_hunt(CHAR_DATA *ch, CHAR_DATA *fch, CHAR_DATA *vch, OBJ_DATA *vobj, int set_flags, int rem_flags)
{
    int                 nflags;
    ROOM_INDEX_DATA    *troom;
    char                buf[MAX_STRING_LENGTH];

    troom = (vch ? vch->in_room : vobj ? vobj->in_room : NULL);
    nflags = ((ch ? ch->hunt_flags : 0) | set_flags) & ~rem_flags;

    if (!ch || !troom || h_find_dir(ch->in_room, troom, nflags) < 0)
        return FALSE;

    ch->hunting = vch;
    ch->hunt_obj = vobj;
    ch->hunt_for = fch;

    if (IS_NPC(ch))
        ch->hunt_home = (ch->hunt_home ? ch->hunt_home : ch->in_room);
    else
        ch->hunt_home = NULL;

    if (ch->searching) {
        free_string(ch->searching);
        ch->searching = NULL;
    }

    ch->hunt_flags = nflags;
    sprintf(buf, "%s has started hunting (%s) %s",
        NAME(ch), (vch ? IS_NPC(vch) ? "mobile" : "player" : "object"), (vch ? NAME(vch) : vobj->short_descr));

    if (fch)
        sprintf(buf + strlen(buf), " for %s", NAME(fch));

    monitor_chan(buf, MONITOR_HUNTING);

    if (!IS_NPC(ch)) {
        sprintf(buf,
            "\n\r@@WYou hear a faint rustling in the wind and a small @@bwood nymph@@W suddenly appears.\n\r@@WThe small @@bwood nymph@@W says, '@@pI know where %s @@pis, just follow my footprints\n\r and I shall show you the way@@W'. With a wink and a flash the small @@bwood nymph@@W disappears!@@N.\n\r",
            (vch ? NAME(vch) : vobj->short_descr));
        send_to_char(buf, ch);
    }

    return TRUE;
}

void
end_hunt(CHAR_DATA *ch)
{
    ch->hunting = NULL;
    ch->hunt_obj = NULL;
    ch->hunt_for = NULL;

    if (!IS_NPC(ch)) {
        ch->hunt_home = NULL;
        ch->hunt_flags = 0;
    }
    else
        ch->hunt_flags = ch->pIndexData->hunt_flags;

    if (ch->searching) {
        free_string(ch->searching);
        ch->searching = NULL;
    }

    return;
}

bool has_key        args((CHAR_DATA *ch, int key));

void
hunt_move(CHAR_DATA *mob, sh_int dir)
{
    EXIT_DATA          *exit = mob->in_room->exit[dir];

    if (IS_SET(exit->exit_info, EX_CLOSED)) {
        if (IS_SET(exit->exit_info, EX_LOCKED)) {
            if (IS_SET(mob->hunt_flags, HUNT_UNLOCKDOOR) && has_key(mob, exit->key))
                do_unlock(mob, dir_name[dir]);
            else if (IS_SET(mob->hunt_flags, HUNT_PICKDOOR) && !IS_SET(exit->exit_info, EX_PICKPROOF))
                do_pick(mob, dir_name[dir]);
        }
        else if (IS_SET(mob->hunt_flags, HUNT_OPENDOOR))
            do_open(mob, dir_name[dir]);
    }
    else
        move_char(mob, dir);

    return;
}

bool
mob_hunt(CHAR_DATA *mob)
{
    sh_int              dir;
    char                buf[MSL];

    if (!mob || !IS_NPC(mob))
        return FALSE;

    if (!mob->hunting) {
        if (mob->hunt_obj != NULL) {
            if (mob->hunt_obj->in_room == NULL) {
                if (IS_SET(mob->hunt_flags, HUNT_CR) && mob->hunt_for && mob->hunt_obj->item_type == ITEM_CORPSE_PC)
                    act("$N tells you 'Someone else seems to have gotten to your corpse before me.'", mob->hunt_for, NULL, mob, TO_CHAR);

                end_hunt(mob);
                return TRUE;
            }

            if (can_see_obj(mob, mob->hunt_obj) && mob->in_room == mob->hunt_obj->in_room) {
                if (IS_SET(mob->hunt_flags, HUNT_CR) && mob->hunt_for && mob->hunt_obj->item_type == ITEM_CORPSE_PC)
                    act("$N tell you 'I have found your corpse.  I shall return it to you now.", mob->hunt_for, NULL, mob, TO_CHAR);

                obj_from_room(mob->hunt_obj);
                obj_to_char(mob->hunt_obj, mob);
                act("$n gets $o.", mob, mob->hunt_obj, NULL, TO_ROOM);
                set_hunt(mob, NULL, mob->hunt_for, mob->hunt_obj, 0, 0);
                return TRUE;
            }

            if (!can_see_obj(mob, mob->hunt_obj) || (dir = h_find_dir(mob->in_room, mob->hunt_obj->in_room, mob->hunt_flags)) < 0) {
                if (IS_SET(mob->hunt_flags, HUNT_CR) && mob->hunt_for && mob->hunt_obj->item_type == ITEM_CORPSE_PC)
                    act("$N tells you 'I seem to have lost the way to your corpse.'", mob->hunt_for, NULL, mob, TO_CHAR);

                end_hunt(mob);
                return TRUE;
            }

            hunt_move(mob, dir);
            if (mob->in_room == mob->hunt_obj->in_room)
                mob_hunt(mob);
        }
        else if (mob->searching && (mob->hunting = get_char_world(mob, mob->searching)) != NULL) {
            free_string(mob->searching);
            mob->searching = NULL;
        }
        else if (mob->hunt_home) {
            dir = -1;

            if (mob->in_room == mob->hunt_home || (dir = h_find_dir(mob->in_room, mob->hunt_home, mob->hunt_flags)) < 0) {
                mob->hunt_home = NULL;
                mob->hunt_flags = mob->pIndexData->hunt_flags;
            }
            else if (dir >= 0)
                hunt_move(mob, dir);
        }
        else {
            return FALSE;
        }

        return TRUE;
    }

    if (!can_see(mob, mob->hunting)) {
        if (IS_SET(mob->hunt_flags, HUNT_MERC) && mob->hunt_for) {
            /* 6.25% chance of giving up, 18.75% chance of telling employer. */
            switch (number_bits(4)) {
                case 0:
                    sprintf(buf, "$N tells you '%s seems to have disappeared!'", NAME(mob->hunting));
                    act(buf, mob->hunt_for, NULL, mob, TO_CHAR);
                    end_hunt(mob);
                    return TRUE;
                case 1:
                case 2:
                case 3:
                    sprintf(buf, "$N tells you '%s seems to have disappeared!  I shall find %s though!'", NAME(mob->hunting),
                        (mob->hunting->sex == SEX_MALE ? "him" : mob->hunting->sex == SEX_FEMALE ? "her" : "it"));
                    act(buf, mob->hunt_for, NULL, mob, TO_CHAR);
                    return TRUE;
            }
        }

        if (IS_SET(mob->hunt_flags, HUNT_INFORM)) {
            switch (number_bits(5)) {
                case 0:
                    sprintf(buf, "Where are you, %s?", NAME(mob->hunting));
                    break;
                case 1:
                    sprintf(buf, "Why can't I find you, %s?", NAME(mob->hunting));
                    break;
                case 2:
                    sprintf(buf, "I know you're out there, %s!", NAME(mob->hunting));
                    break;
                case 3:
                    sprintf(buf, "I'll find you, %s, just wait!", NAME(mob->hunting));
                    break;
                default:
                    return FALSE;
            }

            do_yell(mob, buf);
            return TRUE;
        }

        return FALSE;
    }

    if (mob->in_room == mob->hunting->in_room) {
        if (IS_SET(mob->hunt_flags, HUNT_CR) && mob->hunt_obj && mob->hunt_obj->item_type == ITEM_CORPSE_PC) {
            act("$N tells you 'I have returned with your corpse!'", mob->hunting, NULL, mob, TO_CHAR);

            if (mob->hunt_obj->carried_by == mob) {
                if (mob->hunt_obj->wear_loc != WEAR_NONE)
                    unequip_char(mob, mob->hunt_obj);
                act("$n drops $o.", mob, mob->hunt_obj, NULL, TO_ROOM);
                obj_from_char(mob->hunt_obj);
                obj_to_room(mob->hunt_obj, mob->in_room);
            }

            end_hunt(mob);
            return TRUE;
        }

        if (IS_SET(mob->hunt_flags, HUNT_MERC) && mob->hunt_for) {
            sprintf(buf, "$N tells you 'I have found %s!  Now %s shall die!'",
                NAME(mob->hunting), (mob->hunting->sex == SEX_FEMALE ? "she" : mob->hunting->sex == SEX_MALE ? "he" : "it"));
            act(buf, mob->hunt_for, NULL, mob, TO_CHAR);
        }

        switch (number_bits(2)) {
            case 0:
                sprintf(buf, "Now I have you, %s!", NAME(mob->hunting));
                break;
            case 1:
                sprintf(buf, "I knew you'd be here, %s!", NAME(mob->hunting));
                break;
            case 2:
                sprintf(buf, "Did you really think you were safe, %s?", NAME(mob->hunting));
                break;
            case 3:
                sprintf(buf, "So here you are, %s!", NAME(mob->hunting));
                break;
        }

        if (IS_SET(mob->hunt_flags, HUNT_INFORM))
            do_yell(mob, buf);
        else
            do_say(mob, buf);

        multi_hit(mob, mob->hunting, TYPE_UNDEFINED);
        end_hunt(mob);
        return TRUE;
    }

    if ((dir = h_find_dir(mob->in_room, mob->hunting->in_room, mob->hunt_flags)) == -1) {
        if (IS_SET(mob->hunt_flags, HUNT_MERC) && mob->hunt_for) {
            /* 6.25% chance of giving up, 18.75% chance of informing employer */
            switch (number_bits(4)) {
                case 0:
                    sprintf(buf, "$N tells you 'I seem to have lost %s's trail.'", NAME(mob->hunting));
                    act(buf, mob->hunt_for, NULL, mob, TO_CHAR);
                    end_hunt(mob);
                    return TRUE;
                case 1:
                case 2:
                case 3:
                    sprintf(buf, "$N tells you 'I seem to have lost %s's trail.  I shall find it again, though!'", NAME(mob->hunting));
                    act(buf, mob->hunt_for, NULL, mob, TO_CHAR);
                    return TRUE;
            }
        }

        if (IS_SET(mob->hunt_flags, HUNT_INFORM)) {
            switch (number_bits(6)) {
                case 0:
                    sprintf(buf, "Where are you hiding, %s?", NAME(mob->hunting));
                    break;
                case 1:
                    sprintf(buf, "You can't run forever, %s!", NAME(mob->hunting));
                    break;
                case 2:
                    sprintf(buf, "Come out, come out, wherever you are, %s!", NAME(mob->hunting));
                    break;
                case 3:
                    sprintf(buf, "I promise I won't hurt you, %s.", NAME(mob->hunting));
                    break;
                default:
                    return FALSE;
            }

            do_yell(mob, buf);
            return TRUE;
        }

        return FALSE;
    }

    hunt_move(mob, dir);
    if (mob->in_room == mob->hunting->in_room)
        mob_hunt(mob);

    return TRUE;
}

void
char_hunt(CHAR_DATA *ch)
{
    sh_int              dir;

    if (IS_NPC(ch))
        return;

    if (!ch->hunting) {
        if (ch->hunt_obj) {
            if (!can_see_obj(ch, ch->hunt_obj) || !ch->hunt_obj->in_room) {
                sendf(ch, "%sYou seem to have lost the trail to %s.@@N\n\r", colour_string(ch, "hunt"), ch->hunt_obj->short_descr);
                end_hunt(ch);
            }
            else if (ch->hunt_obj->in_room == ch->in_room) {
                sendf(ch, "%sAhhh.  You have found %s!@@N\n\r", colour_string(ch, "hunt"), ch->hunt_obj->short_descr);
                end_hunt(ch);
            }
            else if ((dir = h_find_dir(ch->in_room, ch->hunt_obj->in_room, ch->hunt_flags)) < 0) {
                sendf(ch, "%sYou seem to have lost the trail to %s.@@N\n\r", colour_string(ch, "hunt"), ch->hunt_obj->short_descr);
                end_hunt(ch);
            }
            else {
                sendf(ch, "%sYou sense that %s is %s of here.@@N\n\r", colour_string(ch, "hunt"), ch->hunt_obj->short_descr, dir_name[dir]);
            }
        }

        return;
    }

    if (!can_see(ch, ch->hunting)) {
        sendf(ch, "%sThe footprints seem to have faded.@@N\n\r", colour_string(ch, "hunt"));
        end_hunt(ch);
    }
    else if (ch->in_room == ch->hunting->in_room) {
        sendf(ch, "%sAhhh.  You have found your prey!@@N\n\r", colour_string(ch, "hunt"));
        end_hunt(ch);
    }
    else if ((dir = h_find_dir(ch->in_room, ch->hunting->in_room, ch->hunt_flags)) < 0) {
        sendf(ch, "%sThe footprints seem to have faded.@@N\n\r", colour_string(ch, "hunt"));
        end_hunt(ch);
    }
    else {
        sendf(ch, "%sYou see glowing footprints leading %s of here.@@N\n\r", colour_string(ch, "hunt"), dir_name[dir]);
    }

    return;
}

void
do_hunt(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA          *victim;
    char                arg[MAX_INPUT_LENGTH];

    if (IS_NPC(ch) && (ch->hunting || ch->hunt_obj))
        return;

    argument = one_argument(argument, arg);
    if (!*arg) {
        send_to_char("Hunt for whom?\n\r", ch);
        return;
    }

    if (!IS_NPC(ch) && ch->pcdata->learned[gsn_hunt] == 0) {
        send_to_char("You don't know how to hunt.\n\r", ch);
        return;
    }

    if (!IS_NPC(ch) && IS_IMMORTAL(ch) && ch->level == MAX_LEVEL && !str_cmp("world", argument))
        victim = get_char_world(ch, arg);
    else
        victim = get_char_area(ch, arg);

    if (victim == ch) {
        if (ch->hunting) {
            sprintf(arg, "You stop hunting %s.\n\r", NAME(ch->hunting));
            send_to_char(arg, ch);
        }
        else if (ch->hunt_obj) {
            sprintf(arg, "You stop looking for %s.\n\r", ch->hunt_obj->short_descr);
            send_to_char(arg, ch);
        }
        else
            send_to_char("You find yourself right where you're standing!\n\r", ch);

        end_hunt(ch);
        return;
    }
    else if (!IS_IMMORTAL(ch) && (victim != NULL) && !IS_NPC(victim) && IS_IMMORTAL(victim)) {
        act("You can't hunt Immortal $N!", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (victim != NULL && victim->in_room == ch->in_room) {
        send_to_char("You're already there!\n\r", ch);
        return;
    }

    if (!IS_NPC(ch))
        ch->hunt_flags = HUNT_OPENDOOR | HUNT_UNLOCKDOOR | HUNT_PICKDOOR;

    if (!IS_NPC(ch) && IS_IMMORTAL(ch) && ch->level == MAX_LEVEL && !str_cmp("world", argument))
        SET_BIT(ch->hunt_flags, HUNT_WORLD);

    if (!victim || !set_hunt(ch, NULL, victim, NULL, 0, HUNT_CR | HUNT_MERC)) {
        send_to_char("You couldn't find a trail.\n\r", ch);
        return;
    }

    return;
}

#ifdef NEVER_FREE_HUNT
#define MAX_BUCKET_SIZE 4096
#define TOP_BUCKET_LIST 4095
void
setup_hunt(void)
{
    struct h_queue     *bucket;
    int                 bcnt;

    bucket = getmem(MAX_BUCKET_SIZE * sizeof(*bucket));

    for (bcnt = 0; bcnt < MAX_BUCKET_SIZE; bcnt++)
        bucket[bcnt].next = (bcnt < TOP_BUCKET_LIST ? &bucket[bcnt + 1] : h_free);

    h_free = &bucket[0];
    return;
}

#undef TOP_BUCKET_SIZE
#undef MAX_BUCKET_SIZE
#endif
