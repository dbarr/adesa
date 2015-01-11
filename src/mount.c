/* Mount, ok ppl lets mount them damn horses, i knew they had to be good
 * for something.
 * 
 * Code written by Celestian on Oct 1st 1996
 *
 * Originally written for ThrynnMUD 2.0, Merc 2.2 based.
 *
 * Copyright (c) 1996 Virtual Research
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "merc.h"

IDSTRING(rcsid, "$Id: mount.c,v 1.6 2003/08/30 04:31:04 dave Exp $");

/* external functions
 */
void                log_string(const char *str);
int                 move_gain(CHAR_DATA *ch);

/* functions for riding
 */
void                stop_riding(CHAR_DATA *ch);
void                do_mount(CHAR_DATA *ch, char *argument);
void                do_dismount(CHAR_DATA *ch, char *argument);
bool                check_valid_ride(CHAR_DATA *ch);

void
do_mount(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    CHAR_DATA          *mount;

    if (IS_NPC(ch)) {
        send_to_char("Mob can't ride anything!\n\r ", ch);
        return;
    }

    one_argument(argument, arg);

    if (*arg) {
        if ((mount = get_char_room(ch, arg)) == NULL) {
            send_to_char("I see no one by that name " "here!\n\r", ch);
            return;
        }
    }
    else {
        send_to_char("Whom do you wish to ride?\n\r", ch);
        return;
    }

    if (ch->position == POS_RIDING) {
        if ((ch->riding == NULL)) {
            ch->position = POS_STANDING;
            send_to_char("Odd, you were mounted on thin air!\n\r", ch);
            return;
        }
    }

    if (mount == ch) {
        send_to_char("Ride on your own back?  How?\n\r", ch);
        return;
    }

    if (ch->position == POS_RIDING) {
        send_to_char("You are already riding on something.\n\r", ch);
        return;
    }

    if (ch->in_room->room_flags & ROOM_SOLITARY) {
        send_to_char("This room is too narrow to ride.\n\r", ch);
        return;
    }

    if (IS_NPC(mount)) {
        if (!IS_SET(mount->act, ACT_MOUNT)) {
            act("It's too difficult to ride on $N.", ch, 0, mount, TO_CHAR);
            return;
        }

        if (IS_AFFECTED(mount, AFF_CHARM) && mount->master != ch) {
            act("$N does not recognize you and refuses to " "let you ride $M.", ch, 0, mount, TO_CHAR);
            return;
        }

        if (IS_NPC(ch) || ch->pcdata->learned[gsn_mount] == 0 || (get_pseudo_level(ch) < get_pseudo_level(mount))) {
            act("You are not skillful enough to ride $N.", ch, NULL, mount, TO_CHAR);
            return;
        }

        if (mount->rider && mount->rider != ch) {
            act("Someone else is riding on $N.", ch, 0, mount, TO_CHAR);
            return;
        }

        if (ch->move < MOUNT_COST) {
            ch->move = 0;
            move_gain(ch);
            act("You are too tired to ride $N.", ch, 0, mount, TO_CHAR);
            return;
        }
        ch->move -= MOUNT_COST;
        move_gain(ch);

        ch->riding = mount;
        mount->rider = ch;
        ch->position = POS_STANDING;
        if (!IS_SET(mount->act, ACT_SENTINEL))
            SET_BIT(mount->act, ACT_SENTINEL);
        act("You climb on and ride $N.", ch, 0, mount, TO_CHAR);
        act("$n climbs on and rides $N.", ch, 0, mount, TO_ROOM);
        act("$n climbs on and rides you.", ch, 0, mount, TO_VICT);
        spell_emount(skill_lookup("Equestrian Proficiency"), 80, ch, NULL, NULL);
    }
}

/* returns true if char is in same room.  Don't want to check ch
 * directly because it may be an invalid pointer already.
 * i.e.  Someone summons ride away and kills it.
 */

static int
valid_ride(int room, CHAR_DATA *ch)
{
    CHAR_DATA          *i;

    if (room == NOWHERE) {
        return FALSE;
    }
    if (ch->in_room == NULL) {
        return FALSE;
    }
    for (i = ch->in_room->first_person; i; i = i->next_in_room) {
        if (IS_NPC(i) && (i == ch))
            return TRUE;
    }
    return FALSE;
}

void
do_dismount(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch)) {
        send_to_char("You can't dismount.\n\r", ch);
        return;
    }

    if (ch->position == POS_RIDING) {
        if (!IS_RIDING(ch)) {
            ch->position = POS_STANDING;
            send_to_char("Odd, you were mounted on thin air!\n\r", ch);
            affect_strip(ch, gsn_emount);
            return;
        }
    }

    if (ch->position == POS_RIDING) {
        if (valid_ride(ch->in_room->vnum, ch->riding)) {
            act("You dismount $N.", ch, 0, ch->riding, TO_CHAR);
            act("$n dismounts $N.", ch, 0, ch->riding, TO_ROOM);
            if (is_same_group(ch, ch->riding))
                do_group(ch, ch->riding->name);
            affect_strip(ch, gsn_emount);
            ch->position = POS_STANDING;
            ch->riding->rider = 0;
            ch->riding = 0;
        }
        else                    /* mount not in same room??? How did this happen? */
            stop_riding(ch);

        if (ch->riding != NULL)
            stop_follower(ch->riding);
    }
    else if (ch->riding) {
        act("You dismount $N.", ch, 0, ch->riding, TO_CHAR);
        act("$n dismounts $N.", ch, 0, ch->riding, TO_ROOM);

        if (is_same_group(ch, ch->riding))
            do_group(ch, ch->riding->name);

        affect_strip(ch, gsn_emount);

        if (ch->position != POS_WRITING) {
            /* if we dismount while not writing, only change our position to standing
             * if we're not also fighting
             */
            if (ch->position != POS_FIGHTING) {
                ch->position = POS_STANDING;
            }
        }
        else {
            BUF_DATA_STRUCT    *buf;

            for (buf = first_buf; buf != NULL; buf = buf->next)
                if (buf->ch == ch) {
                    buf->old_char_pos = POS_STANDING;
                    break;
                }
        }

        ch->riding->rider = 0;
        ch->riding = 0;
    }
    else
        send_to_char("You are not riding anything!\n\r", ch);

}

/* update riding info when mount/rider no longer in same room
 */

void
stop_riding(CHAR_DATA *ch)
{
    CHAR_DATA          *mount, *t_ch;

    if (!ch)
        return;

    if (IS_NPC(ch)) {
        mount = ch;
        t_ch = mount->rider;
    }
    else {
        t_ch = ch;
        mount = t_ch->riding;
    }

    if (!t_ch) {                /* rider has gone poof */
        if (!mount) {            /* so has the mount! */
            return;
        }
        else {
            if (!IS_NPC(mount)) {
                send_to_char("We would appreciate it if " "you explained how a player" "became a mount?\r\n", mount);
                mount->riding = NULL;
                mount->position = POS_STANDING;
                update_pos(mount);
                return;
            }
            else {
                act("You realize that you no longer have " "a rider!", mount, 0, 0, TO_CHAR);
                mount->rider = 0;
                return;
            }
        }
    }
    else {                        /* well we have a rider of some sort */
        if (IS_NPC(t_ch)) {
            char                buf[MAX_STRING_LENGTH];

            sprintf(buf, "NPC %s got on a mount somehow", t_ch->short_descr);
            log_string(buf);
            t_ch->rider = NULL;
            update_pos(t_ch);
            if (mount) {
                send_to_char("Your analomous rider has " "been forcibly dismounted.\r\n", mount);
                if (IS_NPC(mount))
                    mount->rider = NULL;
                else {
                    send_to_char("We would " "appreciate it if you " "explained how a player " "became a mount?\r\n", mount);
                    mount->riding = NULL;
                    mount->position = POS_STANDING;
                    update_pos(mount);
                }
            }
            return;
        }
        if (t_ch->position != POS_RIDING) {
            if (IS_RIDING(t_ch)) {
                char                buf[MAX_STRING_LENGTH];

                sprintf(buf, "%s on mount, but not " "RIDING", t_ch->name);
                log_string(buf);
                t_ch->position = POS_STANDING;
            }
            else
                return;
        }
        if (!mount) {            /* but mount has vanished! */
            act("You fall to the ground.  You stop riding.", t_ch, 0, 0, TO_CHAR);
            t_ch->riding = NULL;
            t_ch->position = POS_RESTING;
            update_pos(t_ch);
        }
        else {                    /* ok, mount and rider both exist, but t_ch is no longer riding */
            if (t_ch->in_room != mount->in_room) {
                act("You fall to the ground.  You stop " "riding.", ch, 0, 0, TO_CHAR);
                t_ch->riding = NULL;
                ch->position = POS_RESTING;
                update_pos(ch);
                if (!IS_NPC(mount)) {
                    send_to_char("We would " "appreciate it if you " "explained how a player " "became a mount?\r\n", mount);
                    mount->riding = NULL;
                    mount->position = POS_STANDING;
                    update_pos(mount);
                    return;
                }
                else {
                    act("You realize that you no " "longer have a rider!", mount, 0, 0, TO_CHAR);
                    mount->rider = NULL;
                }
            }
            else {
                act("You stop riding $N.", t_ch, 0, mount, TO_CHAR);
                if (is_same_group(ch, mount)) {
                    do_group(ch, mount->name);
                }
                t_ch->riding = NULL;
                mount->rider = NULL;
                t_ch->position = POS_STANDING;

            }
        }
    }
}

/* check to make sure mount and rider both exist and are in same room
 */

bool
check_valid_ride(CHAR_DATA *ch)
{
    if (!ch || IS_NPC(ch))
        return FALSE;

    if (valid_ride(ch->in_room->vnum, ch->riding))
        if ((ch->riding)->rider == ch)
            return TRUE;

    stop_riding(ch);
    return FALSE;
}
