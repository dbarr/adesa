
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
#include <sys/stat.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include "merc.h"
#include "duel.h"

IDSTRING(rcsid, "$Id: act_comm.c,v 1.98 2005/04/13 03:12:18 dave Exp $");

/*
 * Local functions.
 */
void talk_channel   args((CHAR_DATA *ch, char *argument, int channel, const char *verb, bool extra));
void                ask_quest_question(CHAR_DATA *ch, char *argument);
void note_finished  args((char *orig, char **dest, CHAR_DATA *ch, bool saved));
void news_finished  args((char *orig, char **dest, CHAR_DATA *ch, bool saved));
extern bool can_save args((CHAR_DATA *ch, OBJ_DATA *obj));

extern POL_DATA     politics_data;

char *crusade_questions[] = {   "what mob?",
                                "who was the thief?",
                                "who is the thief?",
                                "what item?",
                                "where are you?",
                                "who stole the item?",
                                "where is the thief?",
                                "how long is there left?",
                                NULL
                            };

bool
is_valid_player(char *argument)
{
    char                buf[MSL];
    char                c;
    struct stat         s;

    buf[0] = 0;

    if (*argument == '\0')
        return FALSE;

    capitalize(argument);
    c = LOWER(*argument);
    *argument = UPPER(*argument);

    sprintf(buf, PLAYER_DIR "%c/%s", c, argument);

    return (stat(buf, &s) == -1) ? FALSE : TRUE;
}

void
note_finished(char *orig, char **dest, CHAR_DATA *ch, bool saved)
{
    NOTE_DATA          *note;
    CHAR_DATA          *victim;
    bool                playing = FALSE;
    bool                found = FALSE;

    if ((note = ch->pnote) == NULL)
        return;

    if (!saved) {
        PUT_FREE(note, note_free);
        free_string(note->from);
        free_string(note->to);
        free_string(note->subject);
        free_string(note->text);

        send_to_char("Note creation cancelled.\n\r", ch);
        return;
    }

    smash_tilde(note->subject);
    smash_tilde(note->text);

    if ((victim = get_char_world(ch, note->to)) != NULL)
        playing = TRUE;
    else {
        found = is_valid_player(note->to);

        if (!found) {
            send_to_char("That player doesn't exist.\n\r", ch);
            return;
        }
    }

    LINK(note, first_note, last_note, next, prev);

    if (playing) {
        send_to_char("A new note has arrived.\n\r", victim);
        send_to_char("Note sent to player.\n\r", ch);
    }
    else
        send_to_char("Note sent to player.\n\r", ch);

    save_notes();
    return;
}

void
news_finished(char *orig, char **dest, CHAR_DATA *ch, bool saved)
{
    NOTE_DATA          *note;
    CHAR_DATA          *victim;

    if ((note = ch->pnote) == NULL)
        return;

    if (!saved) {
        PUT_FREE(note, note_free);
        free_string(note->from);
        free_string(note->to);
        free_string(note->subject);
        free_string(note->text);

        send_to_char("News creation cancelled.\n\r", ch);
        return;
    }

    smash_tilde(note->subject);
    smash_tilde(note->text);

    LINK(note, first_note, last_note, next, prev);

    for (victim = first_player; victim != NULL; victim = victim->next_player) {
        send_to_char("@@eAn annoucement has been made. Type @@ynews read@@e to read it.@@N\n\r", victim);
    }

    save_notes();
    return;
}

void
do_note(CHAR_DATA *ch, char *argument)
{
    NOTE_DATA          *note;
    NOTE_DATA          *note_next;
    CHAR_DATA          *victim;
    IGNORE_DATA        *ignore;
    DESCRIPTOR_DATA     d;
    char                arg[MAX_INPUT_LENGTH];
    char                arg2[MAX_INPUT_LENGTH];
    char                buf[MSL];
    char                buf2[MSL];
    char                _buf3[64];
    char               *buf3 = _buf3;
    bool                playing = FALSE;
    bool                found = FALSE;
    int                 cnt = 0;

    buf[0] = 0;

    if (IS_NPC(ch))
        return;

    argument = one_argument(argument, arg);

    if (!str_cmp(arg, "send")) {
        /* write a note to someone */

        if (IS_SET(ch->in_room->room_flags, ROOM_QUIET) && !IS_IMMORTAL(ch)) {
            send_to_char("Ssshhh!  This is a quiet room!\n\r", ch);
            return;
        }

        /* must be standing to send notes */
        if (ch->position < POS_STANDING)
            return;

        argument = one_argument(argument, arg2);

        if (arg2[0] == '\0') {
            send_to_char("syntax: note send <person> [subject]\n\r", ch);
            return;
        }

        if (!str_cmp(arg2, "announcement")) {
            send_to_char("You cannot send an announcement using the note command. Use the news command instead.\n\r", ch);
            return;
        }

        if ((victim = get_char_world(ch, arg2)) != NULL)
            playing = TRUE;
        else {
            found = is_valid_player(arg2);

            if (!found) {
                send_to_char("That player doesn't exist.\n\r", ch);
                return;
            }

            if (load_char_obj(&d, arg2, TRUE) == FALSE) {
                free_char(d.character);
                send_to_char("That player doesn't exist.\n\r", ch);
                return;
            }

            victim = d.character;
            d.character = NULL;
            victim->desc = NULL;
        }

        if (IS_NPC(victim)) {
            send_to_char("You can't send a mob a note!\n\r", ch);
            return;
        }

        /* check ignore list of online/offline player */
        for (ignore = victim->pcdata->first_ignore; ignore; ignore = ignore->next) {
            if (!str_cmp(ch->name, ignore->char_ignored)) {
                act("$N is ignoring you!", ch, NULL, victim, TO_CHAR);

                if (!playing)
                    free_char(victim);

                return;
            }
        }

        for (note = first_note; note; note = note->next)
            if (!str_cmp(note->from, ch->name) && !str_cmp(note->to, arg2))
                cnt++;

        if (!IS_IMMORTAL(victim) && IS_SET(ch->act, PLR_SILENCE)) {
            send_to_char("You may only send notes to Immortals while silenced.\n\r", ch);

            if (!playing)
                free_char(victim);

            return;
        }

        if (cnt >= 10) {
            sendf(ch, "You've sent too many notes to %s.\n\r", arg2);

            if (!playing)
                free_char(victim);

            return;
        }

        if (my_strlen(argument) > 53) {
            send_to_char("Subject too long. Limit is 53 characters (excluding colour codes).\n\r", ch);

            if (!playing)
                free_char(victim);

            return;
        }

        note = NULL;
        GET_FREE(note, note_free);
        note->from = str_dup(ch->name);
        note->to = str_dup(victim->name);
        note->subject = str_dup(argument);
        note->date_stamp = current_time;
        note->unread = TRUE;

        ch->pnote = note;

        if (!playing)
            free_char(victim);

        send_to_char("@@N@@gWriting a note. Use @@y.help@@g for help.@@N\n\r", ch);
        write_start(&note->text, (void *) note_finished, (void *) NULL, ch);
    }
    else if (!str_cmp(arg, "list")) {
        cnt = 0;

        if (IS_SET(ch->in_room->room_flags, ROOM_QUIET) && !IS_IMMORTAL(ch)) {
            send_to_char("Ssshhh!  This is a quiet room!\n\r", ch);
            return;
        }

        if (IS_SET(ch->act, PLR_SILENCE)) {
            send_to_char("You are silenced.\n\r", ch);
            return;
        }

        for (note = first_note; note; note = note->next) {
            int                 space = 0;

            _buf3[0] = 0;

            /* if the note is not to them and they don't have a specialname */
            if (str_cmp(note->to, ch->name) && !str_cmp(ch->name, ch->pcdata->origname))
                continue;

            /* they have a specialname and the note isn't to either of their "names" */
            if (str_cmp(ch->name, ch->pcdata->origname) && str_cmp(note->to, ch->name) && str_cmp(note->to, ch->pcdata->origname))
                continue;

            if (cnt == 0)
                send_to_char("@@d.-----------------------------------------------------------------@@g=( @@WNotes @@g)=@@d-.@@N\n\r", ch);

            send_to_char("@@d| ", ch);

            if (note->unread)
                strcpy(buf, "@@y*");
            else
                strcpy(buf, " ");

            sprintf(buf + strlen(buf), "@@W%3d @@d| @@g%12s @@d| @@g%s", ++cnt, note->from, note->subject);

            space = 54 - my_strlen(note->subject);
            while (--space > 0)
                strcat(buf, " ");

            strcat(buf, "@@N @@d|\n\r");
            send_to_char(buf, ch);

            sprintf(buf2, "%s", ctime(&note->date_stamp));
            buf2[24] = 0;

            sprintf(buf2 + strlen(buf2), " @@W(@@g%s ago@@W)", duration(current_time - note->date_stamp, buf3));

            sprintf(buf, "@@d|      |              | @@a%s", buf2);

            space = 54 - my_strlen(buf2);
            while (--space > 0)
                strcat(buf, " ");

            send_to_char(buf, ch);
            send_to_char("@@N @@d|\n\r", ch);
        }

        if (cnt == 0)
            send_to_char("You have no notes.\n\r", ch);
        else
            send_to_char("@@d'-----------------------------------------------------------------------------'@@N\n\r", ch);

        return;
    }
    else if (!str_cmp(arg, "read")) {
        int                 to = 0;
        int                 space = 0;

        if (IS_SET(ch->in_room->room_flags, ROOM_QUIET) && !IS_IMMORTAL(ch)) {
            send_to_char("Ssshhh!  This is a quiet room!\n\r", ch);
            return;
        }

        if (IS_SET(ch->act, PLR_SILENCE)) {
            send_to_char("You are silenced.\n\r", ch);
            return;
        }

        cnt = 0;
        found = FALSE;
        to = atoi(argument);

        for (note = first_note; note; note = note->next) {
            /* if the note is not to them and they don't have a specialname */
            if (str_cmp(note->to, ch->name) && !str_cmp(ch->name, ch->pcdata->origname))
                continue;

            /* they have a specialname and the note isn't to either of their "names" */
            if (str_cmp(ch->name, ch->pcdata->origname) && str_cmp(note->to, ch->name) && str_cmp(note->to, ch->pcdata->origname))
                continue;

            if (++cnt == to) {
                found = TRUE;
                break;
            }
        }

        if (!found) {
            send_to_char("Unable to find that note.\n\r", ch);
            return;
        }

        send_to_char("@@d.-----------------------------------------------------------------@@g=( @@WNotes @@g)=@@d-.@@N\n\r", ch);
        sendf(ch, "@@d| @@g%12s@@W: @@g%s", note->from, note->subject);

        buf[0] = 0;

        space = 62 - my_strlen(note->subject);
        while (--space > 0)
            strcat(buf, " ");

        strcat(buf, "@@N @@d|\n\r");
        send_to_char(buf, ch);
        send_to_char("@@d'-----------------------------------------------------------------------------'@@N\n\r\n\r@@g", ch);

        strcpy(buf, note->text);
        strcat(buf, "@@N\n\r");
        send_to_char(buf, ch);

        note->unread = FALSE;
        save_notes();
        return;
    }
    else if (!str_cmp(arg, "del")) {
        int                 to = 0;
        bool                fAll = FALSE;

        cnt = 0;
        found = FALSE;

        if (!str_cmp(argument, "all")) {
            fAll = TRUE;
            to = -1;
        }
        else
            to = atoi(argument);

        for (note = first_note; note; note = note_next) {
            note_next = note->next;

            /* if the note is not to them and they don't have a specialname */
            if (str_cmp(note->to, ch->name) && !str_cmp(ch->name, ch->pcdata->origname))
                continue;

            /* they have a specialname and the note isn't to either of their "names" */
            if (str_cmp(ch->name, ch->pcdata->origname) && str_cmp(note->to, ch->name) && str_cmp(note->to, ch->pcdata->origname))
                continue;

            if (++cnt == to || fAll) {
                found = TRUE;

                UNLINK(note, first_note, last_note, next, prev);

                PUT_FREE(note, note_free);
                free_string(note->from);
                free_string(note->to);
                free_string(note->subject);
                free_string(note->text);

                if (!fAll)
                    break;
            }
        }

        if (!found)
            send_to_char("Unable to find that note.\n\r", ch);
        else if (fAll)
            send_to_char("All notes erased.\n\r", ch);
        else
            send_to_char("Note erased.\n\r", ch);
    }
    else {
        send_to_char("syntax: note send <player> [subject]\n\r"
            "        note list\n\r" "        note read <number>\n\r" "        note del <number|all>\n\r", ch);

        return;
    }

    save_notes();
    return;
}

void
do_news(CHAR_DATA *ch, char *argument)
{
    NOTE_DATA          *note;
    char                arg[MAX_INPUT_LENGTH];
    char                buf[MSL];
    char                buf2[MSL];
    char                _buf3[64];
    char               *buf3 = _buf3;
    bool                found = FALSE;
    int                 cnt = 0;

    buf[0] = 0;

    if (IS_NPC(ch))
        return;

    argument = one_argument(argument, arg);

    if (!str_cmp(arg, "send")) {
        if (!IS_IMMORTAL(ch)) {
            send_to_char("You must be an Immortal to send an announcement.\n\r", ch);
            return;
        }

        if (argument[0] == '\0') {
            send_to_char("syntax: news send <subject>\n\r", ch);
            return;
        }

        if (my_strlen(argument) > 53) {
            send_to_char("Subject too long. Limit is 53 characters (excluding colour codes).\n\r", ch);

            return;
        }

        note = NULL;
        GET_FREE(note, note_free);
        note->from = str_dup(ch->name);
        note->to = str_dup("announcement");
        note->subject = str_dup(argument);
        note->date_stamp = current_time;
        note->unread = TRUE;

        ch->pnote = note;

        write_start(&note->text, (void *) news_finished, (void *) NULL, ch);
    }
    else if (!str_cmp(arg, "list")) {
        cnt = 0;

        for (note = first_note; note; note = note->next) {
            int                 space = 0;

            _buf3[0] = 0;

            /* if the note is not an announcement, continue */
            if (str_cmp(note->to, "announcement"))
                continue;

            /* old note? don't show it */
            if (ch->pcdata->news_last_read >= note->date_stamp)
                continue;

            if (cnt == 0)
                send_to_char("@@d.------------------------------------------------------------------@@g=( @@WNews @@g)=@@d-.@@N\n\r", ch);

            send_to_char("@@d| ", ch);

            if (note->unread)
                strcpy(buf, "@@y*");
            else
                strcpy(buf, " ");

            sprintf(buf + strlen(buf), "@@W    @@d| @@g%12s @@d| @@g%s", note->from, note->subject);

            space = 54 - my_strlen(note->subject);
            while (--space > 0)
                strcat(buf, " ");

            strcat(buf, "@@N @@d|\n\r");
            send_to_char(buf, ch);

            sprintf(buf2, "%s", ctime(&note->date_stamp));
            buf2[24] = 0;

            sprintf(buf2 + strlen(buf2), " @@W(@@g%s ago@@W)", duration(current_time - note->date_stamp, buf3));

            sprintf(buf, "@@d|      |              | @@a%s", buf2);

            space = 54 - my_strlen(buf2);
            while (--space > 0)
                strcat(buf, " ");

            send_to_char(buf, ch);
            send_to_char("@@N @@d|\n\r", ch);
            cnt++;
        }

        if (cnt == 0)
            send_to_char("There are no new announcements.\n\r", ch);
        else
            send_to_char("@@d'-----------------------------------------------------------------------------'@@N\n\r", ch);

        return;
    }
    else if (!str_cmp(arg, "read")) {
        int                 space = 0;
        found = FALSE;

        for (note = first_note; note; note = note->next) {
            /* if the note is not an announcement, continue */
            if (str_cmp(note->to, "announcement"))
                continue;

            /* old note? don't show it */
            if (ch->pcdata->news_last_read >= note->date_stamp)
                continue;

            found = TRUE;
            break;
        }

        if (!found) {
            send_to_char("Unable to find that announcement.\n\r", ch);
            return;
        }

        send_to_char("@@d.------------------------------------------------------------------@@g=( @@WNews @@g)=@@d-.@@N\n\r", ch);
        sendf(ch, "@@d| @@g%12s@@W: @@g%s", note->from, note->subject);

        buf[0] = 0;

        space = 62 - my_strlen(note->subject);
        while (--space > 0)
            strcat(buf, " ");

        strcat(buf, "@@N @@d|\n\r");
        send_to_char(buf, ch);
        send_to_char("@@d'-----------------------------------------------------------------------------'@@N\n\r\n\r@@g", ch);

        strcpy(buf, note->text);
        strcat(buf, "@@N\n\r");
        send_to_char(buf, ch);

        ch->pcdata->news_last_read = note->date_stamp;
        save_char_obj(ch);
        save_notes();

        cnt = 0;
        for (note = first_note; note; note = note->next) {
            /* if the note is not an announcement, continue */
            if (str_cmp(note->to, "announcement"))
                continue;

            /* old note? don't show it */
            if (ch->pcdata->news_last_read >= note->date_stamp)
                continue;

            cnt++;
        }

        if (cnt ==  1) {
            send_to_char("@@gThere is still @@y1@@g more announcement. Type @@enews read@@g to read it.@@N\n\r", ch);
        }
        else if (cnt > 1) {
            sendf(ch, "@@gThere are still @@y%d@@g announcements. Type @@enews read@@g to read the next one.@@N\n\r", cnt);
        }

        return;
    }
    else {
        send_to_char("syntax: news send <subject>\n\r"
            "        news list\n\r" "        news read\n\r", ch);

        return;
    }

    save_notes();
    return;
}

/*
 * Generic channel function.
 */

#define TALK_STANDALONE                    \
do {                                       \
    position     = ch->position;           \
    ch->position = POS_STANDING;           \
    act(buf, ch, argument, NULL, TO_CHAR); \
    ch->position = position;               \
} while (0)

void
talk_channel(CHAR_DATA *ch, char *argument, int channel, const char *verb, bool extra)
{
    char                buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA    *d;
    int                 position;
    char                ansi[MAX_STRING_LENGTH];
    IGNORE_DATA        *ignore_list = NULL;
    bool                ignore_check = FALSE;
    extern CHAR_DATA   *quest_mob;

    buf[0] = '\0';
    ansi[0] = '\0';

    /* Allows immortals to communicate in silent rooms */
    if (IS_SET(ch->in_room->room_flags, ROOM_QUIET)
        && ((!IS_NPC(ch) && !IS_IMMORTAL(ch))
            || (IS_NPC(ch) && ch != quest_mob)
        )) {
        send_to_char("Ssshhh!  This is a quiet room!\n\r", ch);
        return;
    }

    if (argument[0] == '\0')
        return;

    if (!IS_NPC(ch) && IS_SET(ch->act, PLR_SILENCE)) {
        sendf(ch, "You can't %s.\n\r", verb);
        return;
    }

    if (!extra)
        REMOVE_BIT(ch->deaf, channel);
    else
        REMOVE_BIT(ch->deaf2, channel);

    if (IS_SET(ch->deaf, CHANNEL_HERMIT))
        send_to_char("You are hermit right now, and will not hear the response.\n\r ", ch);

    if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
        argument = slur_text(argument);

    if (!extra) {
        switch (channel) {
            default:                 sprintf(buf, "@@gYou %s '%s@@N@@g'.@@N\n\r", verb, argument); send_to_char(buf, ch); break;

            case CHANNEL_GOSSIP:     sprintf(buf, "%sYou %s '%s@@N%s'.@@N\n\r", colour_string(ch, "gossip"), verb, argument, colour_string(ch, "gossip"));                  send_to_char(buf, ch); break;
            case CHANNEL_MUSIC:      sprintf(buf, "%sYou %s '%s@@N%s'.@@N\n\r", colour_string(ch, "music"),  verb, argument, colour_string(ch, "music"));                   send_to_char(buf, ch); break;
            case CHANNEL_YELL:       sprintf(buf, "%sYou %s '%s@@N%s'.@@N\n\r", colour_string(ch, "yell"),   verb, argument, colour_string(ch, "yell"));                    send_to_char(buf, ch); break;
            case CHANNEL_FLAME:      sprintf(buf, "%sYou %s '%s@@N%s'.@@N\n\r", colour_string(ch, "flame"),  verb, argument, colour_string(ch, "flame"));                   send_to_char(buf, ch); break;
            case CHANNEL_SHOUT:      sprintf(buf, "%sYou %s '%s@@N%s'.@@N\n\r", colour_string(ch, "shout"),  verb, argument, colour_string(ch, "shout"));                   send_to_char(buf, ch); break;
            case CHANNEL_FAMILY:     sprintf(buf, "%s%s %s: '%s@@N%s'.@@N\n\r", colour_string(ch, "clan"),   verb, ch->short_descr, argument, colour_string(ch, "clan"));   send_to_char(buf, ch); break;
            case CHANNEL_RACE:       sprintf(buf, "%s%s %s: '%s@@N%s'.@@N\n\r", colour_string(ch, "race"),   ch->short_descr, verb, argument, colour_string(ch, "race"));   send_to_char(buf, ch); break;

            case CHANNEL_CRUSADE:    sprintf(buf, "@@gYou @@l%s@@N@@l '%s@@N@@l'.@@N\n\r", verb, argument); send_to_char(buf, ch); break;

            case CHANNEL_CLAN:       sprintf(buf, "%s$n@@N%s [%s@@N%s]: '$t@@N%s'.@@N", colour_string(ch, "clan"), colour_string(ch, "clan"), verb, colour_string(ch, "clan"), colour_string(ch, "clan")); TALK_STANDALONE; break;
            case CHANNEL_OOC:        sprintf(buf, "%s%s $n@@N%s: $t@@N%s.@@N",          colour_string(ch, "ooc"),  verb,  colour_string(ch, "ooc"), colour_string(ch, "ooc"));                             TALK_STANDALONE; break;

            case CHANNEL_CREATOR:    sprintf(buf, "%s @@c$n: @@N$t@@N.", verb);               TALK_STANDALONE; break;
            case CHANNEL_IMMTALK:    sprintf(buf, "@@b<@@y$n@@b>@@N $t@@N.");                 TALK_STANDALONE; break;
            case CHANNEL_ADEPT:      sprintf(buf, "@@N@@W$n@@W %s@@g: '$t@@N@@g'.@@N", verb); TALK_STANDALONE; break;
            case CHANNEL_REMORTTALK: sprintf(buf, "%s $n: @@g$t@@N@@g.@@N", verb);            TALK_STANDALONE; break;
            case CHANNEL_DIPLOMAT:   sprintf(buf, "%s @@g$n@@N@@g: $t@@N@@g.@@N", verb);      TALK_STANDALONE; break;
            case CHANNEL_AVATAR:     sprintf(buf, "@@N@@c$n @@N@@W[@@a%s@@W]@@g: $t@@N@@g.@@N", verb); TALK_STANDALONE; break;
            case CHANNEL_PKOK:       sprintf(buf, "@@e$n @@d[@@R%s@@d]: $t@@N@@d.@@N", verb); TALK_STANDALONE; break;
            case CHANNEL_QUEST:      sprintf(buf, "%s $n@@g: $t@@N@@g.@@N", verb);                     TALK_STANDALONE; break;
            case CHANNEL_TRIVIA:     sprintf(buf, "%s $n@@g: $t@@N@@g.@@N", verb);                     TALK_STANDALONE; break;
        }
    }
    else {
        /* channel2 */
        switch (channel) {
            default:            sprintf(buf, "@@gYou %s '%s@@N@@g'.@@N\n\r", verb, argument); send_to_char(buf, ch); break;
            case CHANNEL2_ALLY: sprintf(buf, "%s @@N@@W$n@@N@@d:@@N $t@@N.", verb); TALK_STANDALONE; break;
        }
    }

    for (d = first_desc; d != NULL; d = d->next) {
        CHAR_DATA          *och;
        CHAR_DATA          *vch;
        CHAR_DATA          *cch;

        och = (d->original) ? (d->original) : (d->character);
        vch = d->character;
        cch = (och) ? och : ch;

        if (!extra) {
            switch (channel) {
                default:              sprintf(buf, "@@N@@g$n@@g %ss '$t@@N@@g'.@@N", verb); break;
                case CHANNEL_CRUSADE: sprintf(buf, "@@g$n@@N@@l %ss '$t@@N@@l'.@@N", verb); break;

                case CHANNEL_CLAN:    sprintf(buf, "%s$n@@N%s [%s@@N%s]: '$t@@N%s'.@@N", colour_string(cch, "clan"), colour_string(cch, "clan"),   verb, colour_string(cch, "clan"), colour_string(cch, "clan")); break;
                case CHANNEL_OOC:     sprintf(buf, "%s%s $n@@N%s: $t@@N%s.@@N",          colour_string(cch, "ooc"),  verb,  colour_string(cch, "ooc"), colour_string(cch, "ooc"));                                break;

                case CHANNEL_GOSSIP:  sprintf(buf, "%s$n@@N%s %ss '$t@@N%s'.@@N",      colour_string(cch, "gossip"), colour_string(cch, "gossip"), verb, colour_string(cch, "gossip")); break;
                case CHANNEL_MUSIC:   sprintf(buf, "%s$n@@N%s %ss '$t@@N%s'.@@N",      colour_string(cch, "music"),  colour_string(cch, "music"),  verb, colour_string(cch, "music"));  break;
                case CHANNEL_YELL:    sprintf(buf, "%s$n@@N%s %ss '$t@@N%s'.@@N",      colour_string(cch, "yell"),   colour_string(cch, "yell"),   verb, colour_string(cch, "yell"));   break;
                case CHANNEL_FLAME:   sprintf(buf, "%s$n@@N%s %ss '$t@@N%s'.@@N",      colour_string(cch, "flame"),  colour_string(cch, "flame"),  verb, colour_string(cch, "flame"));  break;
                case CHANNEL_SHOUT:   sprintf(buf, "%s$n@@N%s %ss '$t@@N%s'.@@N",      colour_string(cch, "shout"),  colour_string(cch, "shout"),  verb, colour_string(cch, "shout"));  break;
                case CHANNEL_FAMILY:  sprintf(buf, "%s$n@@N%s %s: '$t@@N%s'.@@N",      colour_string(cch, "clan"),   colour_string(cch, "clan"),   verb, colour_string(cch, "clan"));   break;
                case CHANNEL_RACE:    sprintf(buf, "%s$n@@N%s %s: '$t@@N%s'.@@N",      colour_string(cch, "race"),   colour_string(cch, "race"),   verb, colour_string(cch, "race"));   break;

                case CHANNEL_CREATOR:
                case CHANNEL_IMMTALK:
                case CHANNEL_ADEPT:
                case CHANNEL_REMORTTALK:
                case CHANNEL_DIPLOMAT:
                case CHANNEL_AVATAR:
                case CHANNEL_PKOK:
                case CHANNEL_QUEST:
                case CHANNEL_TRIVIA:
                    /* these are standalone channels in which you can't configure the colours and the message looks the same to all involved */
                    break;
            }
        }
        else {
            /* channel2 */
            switch (channel) {
                default:              sprintf(buf, "@@N@@g$n@@g %ss '$t@@N@@g'.@@N", verb); break;
                case CHANNEL2_ALLY:
                    /* standalone channels */
                    break;
            }
        }

        if (d->connected == CON_PLAYING && vch != ch && ((!extra && !IS_SET(och->deaf, channel)) || (extra && !IS_SET(och->deaf2, channel)))
            && !IS_SET(och->deaf, CHANNEL_HERMIT)) {
            if (IS_SET(vch->in_room->room_flags, ROOM_QUIET) && !IS_IMMORTAL(ch))
                continue;

            if (channel == CHANNEL_CRUSADE && IS_SET(och->deaf, CHANNEL_AUTOCRUSADE)) {
                int cnt;
                bool found = FALSE;

                for (cnt = 0; crusade_questions[cnt] != NULL; cnt++)
                    if (!strcmp(argument, crusade_questions[cnt])) {
                        found = TRUE;
                        break;
                    }

                if (found || IS_NPC(ch))
                    continue;
            }

            ignore_check = FALSE;

            if (!IS_NPC(ch) && !IS_NPC(och))
                for (ignore_list = och->pcdata->first_ignore; ignore_list != NULL; ignore_list = ignore_list->next)
                    if (!str_cmp(ch->name, ignore_list->char_ignored))
                        ignore_check = TRUE;

            if (ignore_check)
                continue;

            if (!extra) {
                if (channel == CHANNEL_CREATOR && get_trust(och) < MAX_LEVEL)
                    continue;
                if (channel == CHANNEL_IMMTALK && !IS_HERO(och))
                    continue;
                if (channel == CHANNEL_DIPLOMAT && !IS_SET(och->pcdata->pflags, PFLAG_CLAN_DIPLOMAT) && !IS_SET(och->pcdata->pflags, PFLAG_CLAN_BOSS) && !IS_SET(och->pcdata->pflags, PFLAG_CLAN_2LEADER))
                    continue;
                if (channel == CHANNEL_REMORTTALK && !is_remort(och))
                    continue;
                if (channel == CHANNEL_YELL && vch->in_room->area != ch->in_room->area)
                    continue;
                if (channel == CHANNEL_ZZZ && vch->position != POS_SLEEPING && och->level != 90)
                    continue;
                if (channel == CHANNEL_AVATAR && !och->pcdata->avatar && !IS_IMMORTAL(och))
                    continue;
                if (channel == CHANNEL_PKOK && !IS_SET(och->pcdata->pflags, PFLAG_PKOK) && !IS_IMMORTAL(och))
                    continue;
                if (channel == CHANNEL_RACE && vch->race != ch->race && (och->level != 90 || IS_SET(och->deaf, CHANNEL_ALLRACE)))
                    continue;
                if (channel == CHANNEL_CLAN && och->pcdata->clan != ch->pcdata->clan && (IS_SET(och->deaf, CHANNEL_ALLCLAN)
                        || get_trust(och) != MAX_LEVEL))
                    continue;
                if (channel == CHANNEL_ADEPT && vch->adept_level < 1)
                    continue;
            }
            else {
                /* channel2 */
                if (channel == CHANNEL2_ALLY && ((!IS_IMMORTAL(och)
                            && ch->pcdata->clan > 0 && och->pcdata->clan > 0 && (politics_data.diplomacy[ch->pcdata->clan][och->pcdata->clan] < 450)
                            && ch->pcdata->clan != och->pcdata->clan)
                        || (och->pcdata->clan == 0 && !IS_IMMORTAL(och))))
                    continue;
            }

            position = vch->position;
            if (extra || (channel != CHANNEL_SHOUT && channel != CHANNEL_YELL))
                vch->position = POS_STANDING;

            act(buf, ch, argument, vch, TO_VICT);
            vch->position = position;
        }
    }

    return;
}

void
do_creator(CHAR_DATA *ch, char *argument)
{
    talk_channel(ch, argument, CHANNEL_CREATOR, "@@a[@@cC-Net@@a]@@N", FALSE);
    return;
}

void
do_gossip(CHAR_DATA *ch, char *argument)
{
    talk_channel(ch, argument, CHANNEL_GOSSIP, "gossip", FALSE);
    return;
}

void
do_crusade(CHAR_DATA *ch, char *argument)
{
    int cnt;

    for (cnt = 0; crusade_questions[cnt] != NULL; cnt++)
        if (!str_cmp(argument, crusade_questions[cnt])) {
            /* if they ask the question, assume they want the response */
            REMOVE_BIT(ch->deaf, CHANNEL_AUTOCRUSADE);
            talk_channel(ch, crusade_questions[cnt], CHANNEL_CRUSADE, "@@lcrusade", FALSE);
            ask_quest_question(ch, crusade_questions[cnt]);
            return;
        }

    talk_channel(ch, argument, CHANNEL_CRUSADE, "@@lcrusade", FALSE);

    return;
}

void
do_music(CHAR_DATA *ch, char *argument)
{
    talk_channel(ch, argument, CHANNEL_MUSIC, "music", FALSE);
    return;
}

void
do_quest2(CHAR_DATA *ch, char *argument)
{
    talk_channel(ch, argument, CHANNEL_QUEST, "@@a[@@cQUEST@@a]@@W", FALSE);
    return;
}

void
do_trivia(CHAR_DATA *ch, char *argument)
{
    talk_channel(ch, argument, CHANNEL_TRIVIA, "@@y[@@rTRIVIA@@y]@@W", FALSE);
    return;
}

void
do_race(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch)) {
        send_to_char("NPCs cannot use this channel.\n\r", ch);
        return;
    }

    sprintf(buf, "[  %s  ]", race_table[ch->race].race_name);
    talk_channel(ch, argument, CHANNEL_RACE, buf, FALSE);
    return;
}

void
do_clan(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch)) {
        send_to_char("NPCs cannot use this channel.\n\r", ch);
        return;
    }

    if (ch->pcdata->clan == 0) {
        send_to_char("Only players in clans may use this channel.\n\r", ch);
        return;
    }

    sprintf(buf, "%s", clan_table[ch->pcdata->clan].clan_abbr);
    talk_channel(ch, argument, CHANNEL_CLAN, buf, FALSE);
    return;
}

void
do_newbie(CHAR_DATA *ch, char *argument)
{
    talk_channel(ch, argument, CHANNEL_NEWBIE, "newbie", FALSE);
    return;
}

void
do_question(CHAR_DATA *ch, char *argument)
{
    talk_channel(ch, argument, CHANNEL_QUESTION, "question", FALSE);
    return;
}

void
do_answer(CHAR_DATA *ch, char *argument)
{
    talk_channel(ch, argument, CHANNEL_QUESTION, "answer", FALSE);
    return;
}

void
do_shout(CHAR_DATA *ch, char *argument)
{
    talk_channel(ch, argument, CHANNEL_SHOUT, "shout", FALSE);
    WAIT_STATE(ch, 12);
    return;
}

void
do_flame(CHAR_DATA *ch, char *argument)
{
    if (ch->level < 3) {
        send_to_char("You must be level 3 to use this channel.\n\r", ch);
        return;
    }

    talk_channel(ch, argument, CHANNEL_FLAME, "flame", FALSE);
    return;
}

void
do_zzz(CHAR_DATA *ch, char *argument)
{
    if (ch->level < 3) {
        send_to_char("You must be level 3 to use this channel.\n\r", ch);

        if (ch->position != POS_SLEEPING)
            send_to_char("... and you have to be asleep anyway!\n\r", ch);

        return;
    }

    if (ch->position != POS_SLEEPING) {
        send_to_char("You can only use this channel when asleep!\n\r", ch);
        return;
    }

    talk_channel(ch, argument, CHANNEL_ZZZ, "zzz", FALSE);
    return;
}

void
do_yell(CHAR_DATA *ch, char *argument)
{
    talk_channel(ch, argument, CHANNEL_YELL, "yell", FALSE);
    return;
}

void
do_immtalk(CHAR_DATA *ch, char *argument)
{
    talk_channel(ch, argument, CHANNEL_IMMTALK, "immtalk", FALSE);
    return;
}

void
do_diptalk(CHAR_DATA *ch, char *argument)
{
    if (!IS_IMMORTAL(ch) && !IS_SET(ch->pcdata->pflags, PFLAG_CLAN_DIPLOMAT) && !IS_SET(ch->pcdata->pflags, PFLAG_CLAN_BOSS) && !IS_SET(ch->pcdata->pflags, PFLAG_CLAN_2LEADER)) {
        send_to_char("You are not a clan diplomat or a clan boss.\n\r", ch);
        return;
    }

    talk_channel(ch, argument, CHANNEL_DIPLOMAT, "@@lNEGOTIATE@@N", FALSE);
    return;
}

void
do_remorttalk(CHAR_DATA *ch, char *argument)
{
    if (!is_remort(ch)) {
        send_to_char("You are not a @@mREMORT@@N!!!\n\r", ch);
        return;
    }

    talk_channel(ch, argument, CHANNEL_REMORTTALK, "@@W[@@mREMORT@@W]@@N", FALSE);
    return;
}

void
do_ooc(CHAR_DATA *ch, char *argument)
{
    talk_channel(ch, argument, CHANNEL_OOC, "[OOC]", FALSE);
    return;
}

void
do_adepttalk(CHAR_DATA *ch, char *argument)
{
    if (!IS_IMMORTAL(ch) && ch->adept_level < 1) {
        send_to_char("You are not @@cADEPT!!@@N\n\r", ch);
        return;
    }

    talk_channel(ch, argument, CHANNEL_ADEPT, "@@N@@W[@@lADEPT@@W]@@N", FALSE);
    return;
}

void
do_avatar(CHAR_DATA *ch, char *argument)
{
    if (!IS_IMMORTAL(ch) && !ch->pcdata->avatar) {
        send_to_char("@@gYou must be an @@aAvatar@@g to use this channel.@@N\n\r", ch);
        return;
    }

    talk_channel(ch, argument, CHANNEL_AVATAR, "AVATAR", FALSE);
    return;
}

void
do_pkok(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch) || (!IS_IMMORTAL(ch) && !IS_SET(ch->pcdata->pflags, PFLAG_PKOK))) {
        send_to_char("@@gYou must be @@ePKOK@@g to use this channel.@@N\n\r", ch);
        return;
    }

    talk_channel(ch, argument, CHANNEL_PKOK, "PKOK", FALSE);
    return;
}

void
do_ally(CHAR_DATA *ch, char *argument)
{
    int                 y;
    char                buf[MSL];
    bool                found = FALSE;

    strcpy(buf, "@@B[@@lALLY@@B]@@N");

    if (IS_NPC(ch))
        return;

    if (ch->pcdata->clan == 0) {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    for (y = 0; y < MAX_CLAN; y++)
        if (politics_data.diplomacy[ch->pcdata->clan][y] >= 450 && ch->pcdata->clan != y) {
            found = TRUE;
            /*            sprintf(buf + strlen(buf), "%s ", clan_table[y].clan_abbr); */
        }

    if (!found) {
        send_to_char("You have no allies at the moment.\n\r", ch);
        return;
    }

    /*    strcat(buf, "@@N]"); */

    talk_channel(ch, argument, CHANNEL2_ALLY, buf, TRUE);
    return;
}

void
do_say(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    CHAR_DATA          *ppl;
    char               *verb = NULL, *verbs = NULL;
    char                last;

    if (!ch->in_room)
        return;

    if (IS_SET(ch->in_room->room_flags, ROOM_QUIET) && !IS_IMMORTAL(ch)) {
        send_to_char("Sssshhhh! This is a quiet room!\n\r", ch);
        return;
    }

    if (argument[0] == '\0') {
        send_to_char("Say what?\n\r", ch);
        return;
    }

    last = argument[strlen(argument) - 1];

    if      (last == '!') { verb = "exclaim"; verbs = "exclaims"; }
    else if (last == '?') { verb = "ask";     verbs = "asks";     }
    else if (last == '.') { verb = "state";   verbs = "states";   }
    else                  { verb = "say";     verbs = "says";     }

    if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
        argument = slur_text(argument);

    sprintf(buf, "You %s '%s$T%s'.", verb, colour_string(ch, "say"), colour_string(ch, "normal"));
    act(buf, ch, NULL, argument, TO_CHAR);

    for (ppl = ch->in_room->first_person; ppl != NULL; ppl = ppl->next_in_room) {
        sprintf(buf, "$n %s '%s$t%s'.", verbs, colour_string(ppl, "say"), colour_string(ppl, "normal"));
        act(buf, ch, argument, ppl, TO_VICT);
    }

    mprog_speech_trigger(argument, ch);
    return;
}

void
do_ignore(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim;
    IGNORE_DATA        *ignore_list;
    IGNORE_DATA        *ignore_find = NULL;
    int                 num = 1;
    int                 cnt = 0;
    bool                found = FALSE;

    arg[0] = 0;
    argument = one_argument(argument, arg);

    if (IS_NPC(ch)) {
        send_to_char("NOT for NPCs!\n\r", ch);
        return;
    }

    ignore_list = ch->pcdata->first_ignore;
    if (ignore_list == NULL && arg[0] == '\0') {
        send_to_char("You have no one on ignore!\n\r", ch);
        return;
    }

    if (arg[0] == '\0') {
        send_to_char("@@ysyntax@@g: ignore <victim>\n\r\n\r", ch);
        send_to_char("Current people to be ignored:\n\r", ch);

        for (ignore_list = ch->pcdata->first_ignore; ignore_list != NULL; ignore_list = ignore_list->next) {
            sendf(ch, "@@W%2d@@d) @@g%s@@N\n\r", num, ignore_list->char_ignored);
            num++;
        }

        return;
    }

    if (arg[0] != '\0' && !IS_NPC(ch))
        for (ignore_find = ch->pcdata->first_ignore; ignore_find != NULL; ignore_find = ignore_find->next) {
            cnt++;

            if (!found && !str_cmp(arg, ignore_find->char_ignored)) {
                found = TRUE;
                break;
            }
        }

    if ((victim = get_char_world(ch, arg)) == NULL && !found) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (!found) {
        if (IS_NPC(victim)) {
            send_to_char("You can only ignore players!!\n\r", ch);
            return;
        }

        if (IS_IMMORTAL(victim)) {
            send_to_char("You cannot ignore Immortals! (as much as you want to sometimes)\n\r", ch);
            return;
        }
    }
    else {
        UNLINK(ignore_find, ch->pcdata->first_ignore, ch->pcdata->last_ignore, next, prev);
        free_string(ignore_find->char_ignored);
        PUT_FREE(ignore_find, ignore_free);
        send_to_char("Ignore removed.\n\r", ch);
        return;
    }

    if (cnt >= 99) {
        send_to_char("You're ignoring too many people!\n\r", ch);
        return;
    }

    GET_FREE(ignore_list, ignore_free);
    ignore_list->char_ignored = str_dup(arg);
    LINK(ignore_list, ch->pcdata->first_ignore, ch->pcdata->last_ignore, next, prev);

    send_to_char("Ignore added.\n\r", ch);
    return;
}

void
do_tell(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    char                arg[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim;
    int                 position;
    ANSWERING_DATA     *answering = NULL;
    ANSWERING_DATA     *answeringcheck = NULL;
    int                 messagecount = 0;
    IGNORE_DATA        *ignore_list = NULL;

    if (!IS_NPC(ch) && IS_SET(ch->pcdata->pflags, PFLAG_AFK) && !IS_IMMORTAL(ch)) {
        REMOVE_BIT(ch->pcdata->pflags, PFLAG_AFK);
        act("You are no longer AFK.", ch, NULL, NULL, TO_CHAR);
        display_messages(ch, argument);
    }

    if (!IS_NPC(ch) && IS_SET(ch->pcdata->pflags, PFLAG_XAFK) && !IS_IMMORTAL(ch)) {
        REMOVE_BIT(ch->pcdata->pflags, PFLAG_XAFK);
        act("You are no longer AFK.", ch, NULL, NULL, TO_CHAR);
        display_messages(ch, argument);
    }

    if (!IS_NPC(ch) && IS_SET(ch->act, PLR_SILENCE)) {
        send_to_char("Your message didn't get through.\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg);

    if (arg[0] == '\0' || argument[0] == '\0') {
        send_to_char("Tell whom what?\n\r", ch);
        return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (!IS_NPC(victim) && !victim->desc) {
        act("Sorry, but $N is currently link dead.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (!IS_NPC(victim))
        for (ignore_list = victim->pcdata->first_ignore; ignore_list != NULL; ignore_list = ignore_list->next)
            if (!IS_NPC(ch) && !str_cmp(ch->name, ignore_list->char_ignored)) {
                act("$N @@N@@eis IGNORING you!@@N", ch, NULL, victim, TO_CHAR);
                return;
            }

    if (!IS_NPC(ch) && !IS_NPC(victim) && !IS_IMMORTAL(ch) && (IS_SET(victim->pcdata->pflags, PFLAG_AFK) || IS_SET(victim->pcdata->pflags, PFLAG_XAFK))) {
        if (IS_SET(victim->config, PLR_ANSWERING) && victim->afk_msg == NULL)
            act("Your message has been added to $N's answering machine.", ch, NULL, victim, TO_CHAR);
        else if (IS_SET(victim->config, PLR_ANSWERING))
            act("$N is afk with $S answering machine on: $t@@N", ch, victim->afk_msg, victim, TO_CHAR);
        else if (victim->afk_msg == NULL || victim->afk_msg[0] == '\0') {
            act("$N is currently away from the keybord.", ch, NULL, victim, TO_CHAR);
        }
        else
            act("$N is afk: $t@@N", ch, victim->afk_msg, victim, TO_CHAR);

        /* answering maching stuff by -ogma- */
        if (!IS_NPC(victim) && IS_SET(victim->config, PLR_ANSWERING)) {
            for (answeringcheck = victim->first_message; answeringcheck != NULL; answeringcheck = answeringcheck->next)
                messagecount++;

            if (messagecount >= MAX_MESSAGES) {
                act("$N's answering machine is full, sorry.", ch, NULL, victim, TO_CHAR);
                return;
            }

            GET_FREE(answering, answering_free);
            answering->message = str_dup(argument);
            answering->name = str_dup(ch->name);
            answering->time = current_time;
            LINK(answering, victim->first_message, victim->last_message, next, prev);
            victim->num_messages++;
        }
        /* end answering machine stuff by -ogma- */

        return;
    }

    if (IS_SET(victim->in_room->room_flags, ROOM_QUIET) && !IS_IMMORTAL(ch)) {
        act("$N is in a quiet room, $E can't hear you.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (!IS_IMMORTAL(ch) && !IS_AWAKE(victim)) {
        act("$E can't hear you.", ch, 0, victim, TO_CHAR);
        return;
    }

    if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
        argument = slur_text(argument);

    sprintf(buf, "You tell $N '%s$t%s'.", colour_string(ch, "tell"), colour_string(ch, "normal"));
    act(buf, ch, argument, victim, TO_CHAR);

    position = victim->position;
    victim->position = POS_STANDING;
    sprintf(buf, "$n tells you '%s$t%s'.", colour_string(victim, "tell"), colour_string(victim, "normal"));
    act(buf, ch, argument, victim, TO_VICT);
    victim->position = position;

    if (!IS_NPC(ch))
        victim->reply = ch;

    if (!IS_NPC(ch) && IS_IMMORTAL(ch))
        victim->ireply = ch;

    return;
}

void
reply(CHAR_DATA *ch, char *argument, bool imm)
{
    CHAR_DATA          *victim;
    char                buf[MSL];
    int                 position;
    IGNORE_DATA        *ignore_list = NULL;

    if (!IS_NPC(ch) && IS_SET(ch->act, PLR_SILENCE)) {
        send_to_char("Your message didn't get through.\n\r", ch);
        return;
    }

    if (!imm && (victim = ch->reply) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (imm && (victim = ch->ireply) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (IS_SET(victim->in_room->room_flags, ROOM_QUIET) && !IS_IMMORTAL(ch)) {
        act("$N is in a quiet room. $E can't hear you.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (!IS_IMMORTAL(ch) && !IS_AWAKE(victim)) {
        act("$E can't hear you.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (!IS_NPC(victim) && !victim->desc) {
        act("Sorry, but $N is currently link dead.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (!IS_NPC(victim))
        for (ignore_list = victim->pcdata->first_ignore; ignore_list != NULL; ignore_list = ignore_list->next)
            if (!IS_NPC(ch) && !str_cmp(ch->name, ignore_list->char_ignored)) {
                act("$N @@eis IGNORING you!@@N", ch, NULL, victim, TO_CHAR);
                return;
            }

    if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
        argument = slur_text(argument);

    sprintf(buf, "You tell $N '%s$t%s'.", colour_string(ch, "tell"), colour_string(ch, "normal"));
    act(buf, ch, argument, victim, TO_CHAR);

    position = victim->position;
    victim->position = POS_STANDING;
    sprintf(buf, "$n tells you '%s$t%s'.", colour_string(victim, "tell"), colour_string(victim, "normal"));
    act(buf, ch, argument, victim, TO_VICT);
    victim->position = position;

    if (!IS_NPC(ch))
        victim->reply = ch;

    if (!IS_NPC(ch) && IS_IMMORTAL(ch) && imm)
        victim->ireply = ch;

    return;
}

void do_reply(CHAR_DATA *ch, char *argument)
{
    reply(ch, argument, FALSE);
    return;
}

void do_ireply(CHAR_DATA *ch, char *argument)
{
    reply(ch, argument, TRUE);
    return;
}

void
do_emote(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    char               *plast;

    buf[0] = '\0';

    if (!IS_NPC(ch) && IS_SET(ch->act, PLR_NO_EMOTE)) {
        send_to_char("You can't show your emotions.\n\r", ch);
        return;
    }

    if (IS_SET(ch->in_room->room_flags, ROOM_QUIET)) {
        send_to_char("You can't show your emotions in a quiet room!\n\r", ch);
        return;
    }

    if (argument[0] == '\0') {
        send_to_char("Emote what?\n\r", ch);
        return;
    }

    for (plast = argument; *plast != '\0'; plast++);

    strcpy(buf, argument);

    if (isalpha(plast[-1]))
        strcat(buf, ".");

    act("* $n $T", ch, NULL, buf, TO_ROOM);
    act("* $n $T", ch, NULL, buf, TO_CHAR);
    return;
}

void
do_bug(CHAR_DATA *ch, char *argument)
{
    append_file(ch, BUG_FILE, argument);
    send_to_char("Ok.  Thanks.\n\r", ch);
    return;
}

void
do_idea(CHAR_DATA *ch, char *argument)
{
    append_file(ch, IDEA_FILE, argument);
    send_to_char("Ok.  Thanks.\n\r", ch);
    return;
}

void
do_typo(CHAR_DATA *ch, char *argument)
{
    append_file(ch, TYPO_FILE, argument);
    send_to_char("Ok.  Thanks.\n\r", ch);
    return;
}

void
do_rent(CHAR_DATA *ch, char *argument)
{
    send_to_char("There is no rent here.  Just save and quit.\n\r", ch);
    return;
}

void
do_qui(CHAR_DATA *ch, char *argument)
{
    send_to_char("If you want to QUIT, you have to spell it out.\n\r", ch);
    return;
}

void
do_quit(CHAR_DATA *ch, char *argument)
{
    DESCRIPTOR_DATA    *d;
    DESCRIPTOR_DATA    *other_logins;
    DESCRIPTOR_DATA    *other_logins_next;
    OBJ_DATA           *obj;
    OBJ_DATA           *in_obj;
    char                buf[MSL];
    bool                found = FALSE;
    int                 cnt = 0;
    int                 rnd = 0;
    extern int          cur_players;

    buf[0] = 0;

    if (IS_NPC(ch))
        return;

    if (ch->position == POS_FIGHTING) {
        send_to_char("No way! You are fighting.\n\r", ch);
        return;
    }

    if (ch->position < POS_STUNNED) {
        send_to_char("You're not DEAD yet.\n\r", ch);
        return;
    }

    if (ch->in_room && IS_SET(ch->in_room->room_flags, ROOM_NO_QUIT)) {
        send_to_char("You can't quit in this room.\n\r", ch);
        return;
    }

    if (IS_SET(ch->config, PLR_SAVECHECK) && strcmp("NOSAVECHECK", argument) != 0) {
        for (obj = first_obj; obj != NULL; obj = obj->next) {
            for (in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj);

            if (in_obj->carried_by != ch)
                continue;

            cnt++;

            if (!can_save(ch, obj)) {
                if (!found)
                    send_to_char("Not quitting because you cannot save the following object(s):\n\r\n\r", ch);

                sendf(ch, "     %s\n\r", obj->short_descr);
                found = TRUE;
            }
        }

        if (cnt > 650) {
            send_to_char("\n\r@@eWarning: You have over 650 items on you, pfiles can only save a maximum of 650, some will be LOST!@@N", ch);
            return;
        }

        if (found)
            return;
    }

    send_to_char("\n\r", ch);
    act("      @@WFarewell, $n.  Return safe to these realms.", ch, NULL, NULL, TO_CHAR);
    send_to_char("\n\r", ch);

    rnd = number_range(1, 7);
    switch (rnd) {
        case 1:
            send_to_char("@@y'Man who walks through airport door sideways is going to Bangkok.'\n\r", ch);
            send_to_char("@@W-- Confucius (551 BCE - 479) @@N\n\r", ch);
            break;
        case 2:
            send_to_char("@@y'If you enjoy what you do, you'll never work another day in your life.'\n\r", ch);
            send_to_char("@@W-- Confucius (551 BCE - 479) @@N\n\r", ch);
            break;
        case 3:
            send_to_char("@@y'War not determine who's right, war determines who's left.'\n\r", ch);
            send_to_char("@@W-- Confucius (551 BCE - 479) @@N\n\r", ch);
            break;
        case 4:
            send_to_char("@@y'It is only the wisest and the very stupidest who cannot change.'\n\r", ch);
            send_to_char("@@W-- Confucius (551 BCE - 479) @@N\n\r", ch);
            break;
        default:
            send_to_char("@@y'Real knowledge is to know the extent of one's ignorance.'\n\r", ch);
            send_to_char("@@W-- Confucius (551 BCE - 479) @@N\n\r", ch);
            break;
    }

    act("$n waves, and leaves the game.", ch, NULL, NULL, TO_ROOM);
    sprintf(log_buf, "%s quits. (%s)", ch->name, ch->pcdata->host);

    notify(log_buf, MAX_LEVEL);
    log_string(log_buf);

    /*
     * After extract_char the ch is no longer valid!
     */

    d = ch->desc;

    for (other_logins = first_desc; other_logins != NULL; other_logins = other_logins_next) {
        other_logins_next = other_logins->next;

        if (other_logins != d && other_logins->character != NULL && other_logins->connected != CON_RECONNECTING
            && !str_cmp(other_logins->character->name, ch->name)) {
            if (other_logins->connected == CON_GET_OLD_PASSWORD) {
                char                logbuf[MSL];

                sprintf(logbuf, "CHEATER!!! Possible attempt to utilize eq dup bug, %s", other_logins->character->name);
                log_string(logbuf);
            }

            close_socket(other_logins);
        }
    }

    save_char_obj(ch);
    ch->is_quitting = TRUE;
    extract_char(ch, TRUE);

    if (d != NULL) {
        cur_players--;
        close_socket(d);
    }

    return;
}

void
do_save(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch)) {
        send_to_char("NPCs are not able to save.\n\r", ch);
        return;
    }

    if (ch->level < 2) {
        send_to_char("You must be at least second level to save.\n\r", ch);
        return;
    }

    if (nosave) {
        send_to_char("@@eWARNING: @@gThe MUD is currently in @@WNOSAVE @@gmode.@@N\n\r", ch);
        return;
    }

    save_char_obj(ch);
    sendf(ch, "Saving %s.\n\r", ch->name);
    return;
}

void
do_savecheck(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA           *obj;
    OBJ_DATA           *in_obj;
    char                buf[MSL];
    bool                found = FALSE;
    int                 cnt = 0;

    buf[0] = 0;

    for (obj = first_obj; obj != NULL; obj = obj->next) {
        for (in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj);

        if (in_obj->carried_by != ch)
            continue;

        cnt++;

        if (!can_save(ch, obj)) {
            if (!found)
                send_to_char("You cannot save the following object(s):\n\r\n\r", ch);

            sendf(ch, "     %s\n\r", obj->short_descr);
            found = TRUE;
        }
    }

    if (cnt > 650) {
        send_to_char("\n\r@@eWarning: You have over 650 items on you, pfiles can only save a maximum of 650, some will be LOST!@@N\n\r", ch);
        return;
    }

    if (!found)
        send_to_char("You can save all objects!\n\r", ch);

    return;
}

void
do_follow(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
        send_to_char("Follow whom?\n\r", ch);
        return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL) {
        act("But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR);
        return;
    }

    if (victim == ch) {
        if (ch->master == NULL) {
            send_to_char("You already follow yourself.\n\r", ch);
            return;
        }

        stop_follower(ch);
        return;
    }

    if ((ch->level - victim->level < -20 || ch->level - victim->level > 20) && !IS_HERO(ch)) {
        send_to_char("You are not of the right caliber to follow.\n\r", ch);
        return;
    }

    if (!IS_NPC(ch) && !IS_NPC(victim) && !IS_IMMORTAL(ch) && IS_SET(victim->config, PLR_NOFOLLOW)) {
        act("$N does not want to be followed.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (IS_RIDING(ch) && is_same_group(ch, ch->riding))
        do_group(ch, ch->riding->name);

    if (ch->master != NULL)
        stop_follower(ch);

    add_follower(ch, victim);
    return;
}

void
add_follower(CHAR_DATA *ch, CHAR_DATA *master)
{
    bool noshow = FALSE;

    if (ch->master != NULL) {
        bug("add_follower: non-null master.");
        return;
    }

    if (IS_NPC(ch) && !IS_NPC(master)) {
        sh_int              charmies;

        charmies = max_orders(master);
        if (charmies <= master->num_followers) {
            send_to_char("You cannot control anymore followers.\n\r", master);
            do_say(ch, "Whaa?? Where am I? How did I get here?");
            do_say(ch, "AHHH!!! Help me!!!! I'm MELTING......");
            extract_char(ch, TRUE);
            return;
        }
        else {
            master->num_followers++;
        }
    }

    ch->master = master;
    ch->leader = NULL;

    if (!IS_NPC(ch) && ch->stance == STANCE_AMBUSH && ch->pcdata->stealth >= gsn_stealth_advanced)
        noshow = TRUE;

    if (can_see(master, ch) && !noshow)
        act("$n starts following you.", ch, NULL, master, TO_VICT);

    if (!noshow)
        act("You start following $N.", ch, NULL, master, TO_CHAR);
    else
        act("You start covertly following $N.", ch, NULL, master, TO_CHAR);

    return;
}

void
stop_follower(CHAR_DATA *ch)
{
    bool noshow = FALSE;

    if (ch->master == NULL) {
        bug("stop_follower: null master.");
        return;
    }

    if (IS_AFFECTED(ch, AFF_CHARM)) {
        REMOVE_BIT(ch->affected_by, AFF_CHARM);
        affect_strip(ch, gsn_charm_person);
    }

    if (is_affected(ch->master, gsn_emount) && ch->rider && ch->master && ch->rider == ch->master)
        affect_strip(ch->master, gsn_emount);

    if (!IS_NPC(ch) && ch->stance == STANCE_AMBUSH && ch->pcdata->stealth >= gsn_stealth_advanced)
        noshow = TRUE;

    if (can_see(ch->master, ch) && !noshow)
        act("$n stops following you.", ch, NULL, ch->master, TO_VICT);

    if (!noshow)
        act("You stop following $N.", ch, NULL, ch->master, TO_CHAR);
    else
        act("You stop covertly following $N.", ch, NULL, ch->master, TO_CHAR);

    if (IS_NPC(ch))
        ch->master->num_followers--;

    ch->master = NULL;
    ch->leader = NULL;
    return;
}

void
die_follower(CHAR_DATA *ch)
{
    CHAR_DATA          *fch;

    if (ch->master != NULL)
        stop_follower(ch);

    ch->leader = NULL;

    for (fch = first_char; fch != NULL; fch = fch->next) {
        if (fch->master == ch)
            stop_follower(fch);
        if (fch->leader == ch)
            fch->leader = fch;
    }

    return;
}

void
do_order(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    char                arg2[MAX_INPUT_LENGTH];
    char                arg3[MAX_INPUT_LENGTH];
    char                argbf[MSL];
    char               *argbuf = argbf;
    CHAR_DATA          *victim;
    CHAR_DATA          *och;
    CHAR_DATA          *och_next;
    bool                found = FALSE;
    bool                fAll = FALSE;
    bool                allowed = FALSE;
    sh_int              num_followers = 0, charmies = 0, cmd = 0;

    argument = one_argument(argument, arg);

    if (arg[0] == '\0' || argument[0] == '\0') {
        send_to_char("Order whom to do what?\n\r", ch);
        return;
    }

    if (IS_AFFECTED(ch, AFF_CHARM)) {
        send_to_char("You feel like taking, not giving, orders.\n\r", ch);
        return;
    }

    if (!str_cmp(arg, "all")) {
        fAll = TRUE;
        victim = NULL;
    }
    else {
        fAll = FALSE;

        if ((victim = get_char_room(ch, arg)) == NULL) {
            send_to_char("They aren't here.\n\r", ch);
            return;
        }

        if (victim == ch) {
            send_to_char("Aye aye, right away!\n\r", ch);
            return;
        }

        if (!IS_AFFECTED(victim, AFF_CHARM) || victim->master != ch) {
            send_to_char("Do it yourself!\n\r", ch);
            return;
        }
    }

    one_argument(argument, arg);
    strcpy(argbuf, argument);
    argbuf = one_argument(argbuf, arg2);
    argbuf = one_argument(argbuf, arg3);

    for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++) {
        if (!str_prefix(arg2, cmd_table[cmd].name)) {
            found = TRUE;

            if (!str_cmp(cmd_table[cmd].name, "cast")) {
                int                 sn = -1;

                if ((sn = skill_lookup(arg3)) >= 0) {
                    char               *skill = skill_table[sn].name;

                    if (   !str_cmp(skill, "fire breath")
                        || !str_cmp(skill, "frost breath")
                        || !str_cmp(skill, "acid breath")
                        || !str_cmp(skill, "lightning breath")
                        || !str_cmp(skill, "gas breath")
                        || !str_cmp(skill, "high explosive")
/*                      || !str_cmp(skill, "fireshield") */
/*                      || !str_cmp(skill, "iceshield") */
                        || !str_cmp(skill, "demonshield")
/*                      || !str_cmp(skill, "shockshield") */
                        || !str_cmp(skill, "rune:poison")
                        || !str_cmp(skill, "rune:fire")
                        || !str_cmp(skill, "rune:shock")
/*                      || !str_cmp(skill, "morale") */
/*                      || !str_cmp(skill, "leadership") */
/*                      || !str_cmp(skill, "holy armor") */
/*                      || !str_cmp(skill, "deflect weapon") */
                        || !str_cmp(skill, "poison weapon")
/*                      || !str_cmp(skill, "healing light") */
/*                      || !str_cmp(skill, "mana flare") */
                        || !str_cmp(skill, "thoughtshield")
                        || !str_cmp(skill, "shadowshield")
                        || !str_cmp(skill, "gate")
                        || !str_cmp(skill, "stalker")
                        || !str_cmp(skill, "call lightning")
                        || !str_cmp(skill, "earthquake")
                        || !str_cmp(skill, "mindflame")
                        || !str_cmp(skill, "chain lightning")
                        || !str_cmp(skill, "nerve fire")
                        || !str_cmp(skill, "retributive strike")
                        || !str_cmp(skill, "hellfire")
                        || !str_cmp(skill, "creature bond")
                        || !str_cmp(skill, "corrupt bond")
                        || !str_cmp(skill, "charm person")
                        || !str_cmp(skill, "hypnosis")
                       ) {
                        allowed = FALSE;
                        break;
                    }
                }
            }

            if (cmd_table[cmd].can_order)
                allowed = TRUE;
            else
                allowed = FALSE;

            break;
        }
    }

    if (!found) {
        for (cmd = 0; social_table[cmd].name[0] != '\0'; cmd++) {
            if (arg2[0] == social_table[cmd].name[0]
                && !str_prefix(arg2, social_table[cmd].name)) {
                allowed = TRUE;
                break;
            }
        }
    }

    if (!allowed) {
        send_to_char("I'm not allowed to do that!\n\r", ch);
        return;
    }

    if ((fAll != TRUE) && (victim->master == ch) && (victim->is_free == FALSE)) {
        act("$n orders you to '$t'.", ch, argument, victim, TO_VICT);
        interpret(victim, argument);
        send_to_char("Ok.\n\r", ch);
        return;
    }

    found = FALSE;
    charmies = max_orders(ch);

    for (och = ch->in_room->first_person; och != NULL; och = och_next) {
        och_next = och->next_in_room;

        /* players don't overlap, neither do non-charmed mobs */
        if (!IS_NPC(och) || (IS_NPC(och) && !IS_AFFECTED(och, AFF_CHARM)))
            continue;

        /* charmed mobs who don't belong to a group-member don't overlap */
        if (IS_NPC(och) && IS_AFFECTED(och, AFF_CHARM) && och->master && ch != och->master && is_same_group(ch, och->master))
            continue;

        /* mounts, even group-member mounts, don't overlap */
        if (IS_NPC(och) && IS_AFFECTED(och, AFF_CHARM) && och->master && ch != och->master && och->rider)
            continue;

        if (och->is_free == FALSE) {
            num_followers++;

            if (num_followers <= charmies) {
                if (IS_AFFECTED(och, AFF_CHARM) && och->master == ch && fAll) {
                    found = TRUE;
                    act("$n orders you to '$t'.", ch, argument, och, TO_VICT);
                    interpret(och, argument);
                }
            }
            else {
                send_to_char("@@eYou can't control anymore followers!!@@N\n\r", ch);
                break;
            }
        }
    }

    if (found)
        send_to_char("Ok.\n\r", ch);
    else
        send_to_char("You have no followers here.\n\r", ch);

    return;
}

void
group_all(CHAR_DATA *ch)
{
    CHAR_DATA          *gch;
    int                 new_members = 0;

    for (gch = ch->in_room->first_person; gch != NULL; gch = gch->next_in_room) {
        if ((gch->master == ch) && (gch->leader != ch)) {
            if (!can_group(ch, gch)) {
                act("$N cannot join $n's group.", ch, NULL, gch, TO_NOTVICT);
                act("You cannot join $n's group.", ch, NULL, gch, TO_VICT);
                act("$N cannot join your group.", ch, NULL, gch, TO_CHAR);
            }
            else {
                gch->leader = ch;
                act("$N joins $n's group.", ch, NULL, gch, TO_NOTVICT);
                act("You join $n's group.", ch, NULL, gch, TO_VICT);
                act("$N joins your group.", ch, NULL, gch, TO_CHAR);
                new_members++;
            }
        }
    }

    if (new_members == 0)
        send_to_char("No one else wants to join your group.\n\r", ch);

    return;
}

bool
can_group(CHAR_DATA *ch, CHAR_DATA *victim)
{
    bool                ch_adept = FALSE, victim_adept = FALSE, ch_dremort = FALSE, victim_dremort = FALSE, ch_sremort = FALSE, victim_sremort =
        FALSE;
    bool                legal_group = FALSE;
    DUEL_DATA          *duel;

    /* note that you can't actually group anyone in a duel. this it to bypass
     * group range pk checks
     */
    if ((duel = find_duel(ch)) && duel->stage == DUEL_STAGE_GO)
        return TRUE;

    if (ch->adept_level > 0)
        ch_adept = TRUE;
    if (victim->adept_level > 0)
        victim_adept = TRUE;
    if (get_pseudo_level(ch) > 97)
        ch_dremort = TRUE;
    if (get_pseudo_level(victim) > 97)
        victim_dremort = TRUE;
    if (get_pseudo_level(ch) > 80)
        ch_sremort = TRUE;
    if (get_pseudo_level(victim) > 80)
        victim_sremort = TRUE;

    if (ch_adept && victim_adept)
        legal_group = TRUE;
    else if ((ch_adept && victim_dremort) || (victim_adept && ch_dremort)) {
        if (abs(get_pseudo_level(ch) - get_pseudo_level(victim)) < 9)
            legal_group = TRUE;
    }
    else if (ch_dremort || victim_dremort || ch_sremort || victim_sremort) {
        if (abs(get_pseudo_level(ch) - get_pseudo_level(victim)) < 8)
            legal_group = TRUE;
        else
            legal_group = FALSE;
    }
    else {
        if (abs(get_pseudo_level(ch) - get_pseudo_level(victim)) < 21)
            legal_group = TRUE;
    }

    if (ch->riding == victim)
        legal_group = TRUE;

    if (legal_group)
        return TRUE;
    else
        return FALSE;
}

bool
can_group_level(CHAR_DATA *ch, int level)
{
    bool                ch_adept = FALSE, victim_adept = FALSE, ch_dremort = FALSE, victim_dremort = FALSE, ch_sremort = FALSE, victim_sremort =
        FALSE;
    bool                legal_group = FALSE;

    if (ch->adept_level > 0)
        ch_adept = TRUE;
    if (level == 120)
        victim_adept = TRUE;
    if (get_pseudo_level(ch) > 97)
        ch_dremort = TRUE;
    if (level > 97)
        victim_dremort = TRUE;
    if (get_pseudo_level(ch) > 80)
        ch_sremort = TRUE;
    if (level > 80)
        victim_sremort = TRUE;

    if (ch_adept && victim_adept)
        legal_group = TRUE;
    else if ((ch_adept && victim_dremort) || (victim_adept && ch_dremort)) {
        if (abs(get_pseudo_level(ch) - level) < 9)
            legal_group = TRUE;
        else
            legal_group = FALSE;
    }
    else if (ch_dremort || victim_dremort || ch_sremort || victim_sremort) {
        if (abs(get_pseudo_level(ch) - level) < 8)
            legal_group = TRUE;
        else
            legal_group = FALSE;
    }
    else {
        if (abs(get_pseudo_level(ch) - level) < 21)
            legal_group = TRUE;
    }

    if (legal_group)
        return TRUE;
    else
        return FALSE;
}

void
do_pgroup(CHAR_DATA *ch, char *argument)
{
    char               buf[MSL];
    int                cnt = 0;
    CHAR_DATA          *gch;
    CHAR_DATA          *leader;

    leader = (ch->leader != NULL) ? ch->leader : ch;

    strcpy(buf, "@@N@@d.");
    cnt = abs(62 - my_strlen(PERS(leader, ch)));
    for (; cnt > 0; cnt--)
        safe_strcat(MSL, buf, "-");

    sprintf(buf + strlen(buf), "@@g=( @@W%s's group @@g)=@@d-.@@N\n\r", PERS(leader, ch));
    send_to_char(buf, ch);

    send_to_char("@@d| @@cLvl @@aCls @@gName         @@d|         @@eHp @@d|       @@rMana @@d|      @@yMoves @@d|            @@WXP @@d|@@N\n\r", ch);
    send_to_char("@@d|----------------------+------------+------------+------------+---------------|@@N\n\r", ch);

    for (gch = first_char; gch; gch = gch->next) {
        if (is_same_group(gch, ch)) {
            char                nbuf[MIL]; 
            char                hbuf[MIL];
            char                mbuf[MIL];
            char                vbuf[MIL];
            char                xbuf[MIL];

            sprintf(hbuf, "%s", percbar(gch->hit, gch->max_hit, 10));
            sprintf(mbuf, "%s", percbar(gch->mana, gch->max_mana, 10));
            sprintf(vbuf, "%s", percbar(gch->move, gch->max_move, 10));

            sendf(ch, "@@d| @@c%3d @@a%s @@N@@g%s @@N@@d| @@N@@g%s @@N@@d| @@N@@g%s @@N@@d| @@N@@g%s @@N@@d| @@W%s @@d|@@N\n\r",
                !IS_IMMORTAL(gch) ? gch->level : 0,
                IS_NPC(gch) ? "Mob" : class_table[gch->class].who_name,
                my_left(PERS(gch, ch), nbuf, 12), hbuf, mbuf, vbuf, my_right(number_comma(gch->exp), xbuf, 13));

        }
    }

    send_to_char("@@d'-----------------------------------------------------------------------------'@@N\n\r", ch);
    return;
}

void
do_group(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    char                buf[MAX_INPUT_LENGTH]; 
    char                buf2[MAX_INPUT_LENGTH];
    char                buf3[MAX_INPUT_LENGTH];
    char                buf4[MAX_INPUT_LENGTH];
    char                hbuf[MIL];
    char                mbuf[MIL];
    char                vbuf[MIL];
    char                xbuf[MIL];
    int                 cnt = 0;

    CHAR_DATA          *victim;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
        CHAR_DATA          *gch;
        CHAR_DATA          *leader;

        leader = (ch->leader != NULL) ? ch->leader : ch;

        strcpy(buf, "@@N@@d.");
        cnt = abs(62 - my_strlen(PERS(leader, ch)));
        for (; cnt > 0; cnt--)
            safe_strcat(MSL, buf, "-");

        sprintf(buf + strlen(buf), "@@g=( @@W%s's group @@g)=@@d-.@@N\n\r", PERS(leader, ch));
        send_to_char(buf, ch);
        buf[0] = '\0';

        send_to_char("@@d| @@cLvl @@aCls @@gName      @@d|          @@eHp @@d|        @@rMana @@d|       @@yMoves @@d|            @@WXP @@d|@@N\n\r", ch);
        send_to_char("@@d|-------------------+-------------+-------------+-------------+---------------|@@N\n\r", ch);

        for (gch = first_char; gch != NULL; gch = gch->next) {
            if (is_same_group(gch, ch)) {
                sprintf(buf2, "@@e%d@@R/@@e%d", gch->hit,  gch->max_hit);
                sprintf(buf3, "@@r%d@@G/@@r%d", gch->mana, gch->max_mana);
                sprintf(buf4, "@@y%d@@b/@@y%d", gch->move, gch->max_move);

                sendf(ch, "@@d| @@c%3d @@a%s @@N@@g%s @@N@@d| %s @@d| %s @@d| %s @@d| @@W%s @@d|@@N\n\r",
                    !IS_IMMORTAL(gch) ? gch->level : 0,
                    IS_NPC(gch) ? "Mob" : class_table[gch->class].who_name,
                    my_left(PERS(gch, ch), buf, 9), my_right(buf2, hbuf, 11), my_right(buf3, mbuf, 11), my_right(buf4, vbuf, 11), my_right(number_comma(gch->exp), xbuf, 13));

            }
        }

        sendf(ch, "@@d'-----------------------------------------------------------------------------'@@N\n\r");
        return;
    }

    if (!IS_NPC(ch) && is_in_duel(ch, DUEL_STAGE_GO)) {
        send_to_char("Not while you're sparring.\n\r", ch);
        return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (ch->master != NULL || (ch->leader != NULL && ch->leader != ch)) {
        send_to_char("But you are following someone else!\n\r", ch);
        return;
    }

    if (victim->master != ch && ch != victim) {
        act("$N isn't following you.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (is_same_group(victim, ch) && ch != victim) {
        victim->leader = NULL;
        act("$n removes $N from $s group.", ch, NULL, victim, TO_NOTVICT);
        act("$n removes you from $s group.", ch, NULL, victim, TO_VICT);
        act("You remove $N from your group.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (!can_group(ch, victim)) {
        act("$N cannot join $n's group.", ch, NULL, victim, TO_NOTVICT);
        act("You cannot join $n's group.", ch, NULL, victim, TO_VICT);
        act("$N cannot join your group.", ch, NULL, victim, TO_CHAR);
        return;
    }

    victim->leader = ch;
    act("$N joins $n's group.", ch, NULL, victim, TO_NOTVICT);
    act("You join $n's group.", ch, NULL, victim, TO_VICT);
    act("$N joins your group.", ch, NULL, victim, TO_CHAR);
    return;
}

void
do_split(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
	char                buf2[MAX_STRING_LENGTH];
    char                arg[MAX_INPUT_LENGTH];

    CHAR_DATA          *gch;
    int                 members;
    int                 amount;
    int                 share;
    int                 extra;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
        send_to_char("Split how much?\n\r", ch);
        return;
    }

    amount = atoi(arg);

    if (amount < 0) {
        send_to_char("Your group wouldn't like that.\n\r", ch);
        return;
    }

    if (amount == 0) {
        send_to_char("You hand out zero coins, but no one notices.\n\r", ch);
        return;
    }

    if (ch->gold < amount) {
        send_to_char("You don't have that much gold.\n\r", ch);
        return;
    }

    members = 0;

    for (gch = ch->in_room->first_person; gch != NULL; gch = gch->next_in_room)
        if (!IS_NPC(gch) && is_same_group(gch, ch))
            members++;

    if (members < 2) {
        send_to_char("Just keep it all.\n\r", ch);
        return;
    }

    share = amount / members;
    extra = amount % members;

    if (share == 0) {
        send_to_char("Don't even bother, cheapskate.\n\r", ch);
        return;
    }

    ch->gold -= amount;
    ch->gold += share + extra;

    sendf(ch, "You split @@y%s@@N gold coins.  Your share is @@y%s@@N gold coins.\n\r", number_comma(amount), number_comma_r(share + extra, buf2));

    sprintf(buf, "$n splits @@y%s@@N gold coins.  Your share is @@y%s@@N gold coins.", number_comma(amount), number_comma_r(share, buf2));

    for (gch = ch->in_room->first_person; gch != NULL; gch = gch->next_in_room)
        if (gch != ch && !IS_NPC(gch) && is_same_group(gch, ch)) {
            act(buf, ch, NULL, gch, TO_VICT);
            gch->gold += share;
        }

    return;
}

void
do_gtell(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA          *gch;

    if (argument[0] == '\0') {
        send_to_char("Tell your group what?\n\r", ch);
        return;
    }

    if (IS_SET(ch->act, PLR_NO_TELL)) {
        send_to_char("Your message didn't get through!\n\r", ch);
        return;
    }

    /*
     * Note use of send_to_char, so gtell works on sleepers.
     */

    if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
        argument = slur_text(argument);

    for (gch = first_player; gch != NULL; gch = gch->next_player)
        if (is_same_group(gch, ch))
            sendf(gch, "@@g%s @@N@@gtells the group '%s%s@@N@@g'.@@N\n\r", ch->short_descr, colour_string(gch, "gtell"), argument);

    return;
}

bool
is_same_group(CHAR_DATA *ach, CHAR_DATA *bch)
{
    if (ach->leader != NULL)
        ach = ach->leader;
    if (bch->leader != NULL)
        bch = bch->leader;
    return (ach == bch);
}

bool
is_group_leader(CHAR_DATA *ch)
{
    CHAR_DATA          *vch;
    bool                rvalue = FALSE;

    for (vch = ch->in_room->first_person; vch != NULL; vch = vch->next_in_room)
        if (vch != ch && vch->leader == ch) {
            rvalue = TRUE;
            break;
        }

    return rvalue;
}

void
do_pemote(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    char               *plast;

    buf[0] = '\0';

    if (!IS_NPC(ch) && IS_SET(ch->act, PLR_NO_EMOTE)) {
        send_to_char("You can't pemote.\n\r", ch);
        return;
    }

    if (IS_SET(ch->in_room->room_flags, ROOM_QUIET)) {
        send_to_char("You can't show your emotions in a quiet room!\n\r", ch);
        return;
    }

    if (argument[0] == '\0') {
        send_to_char("Pemote what?\n\r", ch);
        return;
    }

    for (plast = argument; *plast != '\0'; plast++);

    strcpy(buf, argument);
    if (isalpha(plast[-1]))
        strcat(buf, ".");

    act("* $n's $T", ch, NULL, buf, TO_ROOM);
    act("* $n's $T", ch, NULL, buf, TO_CHAR);
    return;
}

void
do_pray(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];

    if (ch->level > LEVEL_HERO) {
        send_to_char("Hey, try immtalk why don't you?\n\r", ch);
        return;
    }

    if (IS_NPC(ch)) {
        send_to_char("You pray hard... who knows if it got heard?\n\r", ch);
        return;
    }

    if (argument[0] == '\0') {
        send_to_char("What do you wish to pray?\n\r", ch);
        return;
    }

    if (IS_SET(ch->act, PLR_NO_PRAY)) {
        send_to_char("The Gods are not listening to you today.\n\r", ch);
        return;
    }

    if (ch->pcdata->idlecheck == 0) {
        send_to_char("You pray, concentrating on your message.\n\r", ch);
        sprintf(buf, "%s sends the following message via prayer:\n\r`%s'\n\r", ch->name, argument);
    }
    else {
        char dbuf[64];

        send_to_char("You pray, concentrating on your message. @@eAn Immortal wanted your attention, please be attentative until you receive a reply from an Immortal.@@N\n\r", ch);
        if (current_time <= ch->pcdata->idlecheck)
            sprintf(buf, "%s sends the following message via prayer (replied with %s to go):\n\r`%s'\n\r", ch->short_descr, duration(ch->pcdata->idlecheck - current_time, dbuf), argument);
        else
            sprintf(buf, "%s sends the following message via prayer (replied %s AFTER):\n\r`%s'\n\r", ch->short_descr, duration(current_time - ch->pcdata->idlecheck, dbuf), argument);

        ch->pcdata->idlecheck = 0;
    }

    notify(buf, LEVEL_HERO);
    return;
}

void
do_tongue(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    char                buf2[MAX_STRING_LENGTH];
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
        {"fuck", "love"},
        {"go", "swim"},
        {"the", "woi"},
        {"hi", "yibba"},
        {"hello", "smeg"},
        {"me", "ug"},
        {"follow", "grep"},
        {"kill", "banzai"},
        {"ing", "pft"},
        {"er", "sf"},
        {"you", "lpt"},
        {"ast", "pal"},
        {"nd", "ja"},
        {"re", "qa"},
        {"tell", "argh"},
        {"who", "wib"},
        {"which", "fvl"},
        {"ts", "glup"},
        {"st", "plop"},
        {"ck", "fim"},
        {"ord", "xio"},
        {"sw", "cow"},
        {"ic", "er"},
        {"ea", "ox"},
        {"?", "?"}, {"!", "!"}, {":", ":"}, {")", ")"},
        {"(", "("}, {";", ";"}, {"*", "*"}, {"-", "-"},
        {".", ","}, {",", ","}, {"a", "y"}, {"b", "d"},
        {"c", "g"}, {"d", "b"}, {"e", "q"}, {"f", "i"},
        {"g", "u"}, {"h", "r"}, {"i", "t"}, {"j", "m"},
        {"k", "j"}, {"l", "v"}, {"m", "o"}, {"n", "t"},
        {"o", "s"}, {"p", "f"}, {"q", "k"}, {"r", "x"},
        {"s", "z"}, {"t", "e"}, {"u", "p"}, {"v", "c"},
        {"w", "n"}, {"x", "h"}, {"y", "a"}, {"z", "l"},
        {"", ""}
    };

    buf[0] = '\0';
    buf2[0] = '\0';

    if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
        argument = slur_text(argument);

    for (pName = argument; *pName != '\0'; pName += length) {
        for (iSyl = 0; (length = strlen(syl_table[iSyl].old)) != 0; iSyl++) {
            if (!str_prefix(syl_table[iSyl].old, pName)) {
                strcat(buf, syl_table[iSyl].new);
                break;
            }
        }

        if (length == 0)
            length = 1;
    }

    for (rch = ch->in_room->first_person; rch; rch = rch->next_in_room) {
        if (IS_NPC(rch))
            continue;

        if (rch != ch)
            act("$n tongues, '$t'", ch, (ch->race == rch->race || rch->level > LEVEL_IMMORTAL) ? argument : buf, rch, TO_VICT);
    }

    sendf(ch, "You tongue, '%s'.\n\r", buf);
    sendf(ch, "Translation: %s.\n\r", argument);
    return;
}

char               *
slur_text(char *argument)
{
    /* Used to slur text, if a player is DRUNK. */

    static char         buf[MAX_STRING_LENGTH];
    char               *pName;
    int                 iSyl;
    int                 length;

    struct syl_type
    {
        char               *old;
        char               *new;
    };

    static const struct syl_type syl_table[] = {
        {" ", " "}, {"th", "f"}, {"ck", "k"}, {"?", "?"}, {"!", "!"},
        {":", ":"}, {")", ")"}, {"(", "("}, {";", ";"}, {"*", "*"},
        {"-", "-"}, {".", ","}, {",", ","}, {"a", "a"}, {"b", "b"},
        {"c", "see"}, {"d", "d"}, {"e", "e"}, {"f", "f"}, {"g", "g"},
        {"h", "h"}, {"i", "i"}, {"j", "j"}, {"k", "g"}, {"l", "l"},
        {"m", "m"}, {"n", "n"}, {"o", "o"}, {"p", "p"}, {"q", "q"},
        {"r", "r"}, {"s", "ss"}, {"t", "t"}, {"u", "u"}, {"v", "s"},
        {"w", "w"}, {"x", "x"}, {"y", "y"}, {"z", "s"}, {NULL, NULL}
    };

    buf[0] = '\0';

    for (pName = argument; *pName != '\0'; pName += length) {
        length = 1;

        if (*pName == '$') {    /* do not modify args to act. */
            if (*(pName + 1) != '\0')
                length++;

            continue;
        }

        for (iSyl = 0; syl_table[iSyl].old != NULL; iSyl++) {
            if (!str_prefix(syl_table[iSyl].old, pName)) {
                strcat(buf, syl_table[iSyl].new);
                length = strlen(syl_table[iSyl].old);
                break;
            }
        }
    }

    return buf;
}

void
do_whisper(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim;
    int                 position;

    if (!IS_NPC(ch) && IS_SET(ch->act, PLR_SILENCE)) {
        send_to_char("Your whispering skills seem rusty today.\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg);

    if (arg[0] == '\0' || argument[0] == '\0') {
        send_to_char("Whisper what to whom?\n\r", ch);
        return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (!IS_IMMORTAL(ch) && !IS_AWAKE(victim)) {
        act("$E can't hear you.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
        argument = slur_text(argument);

    act("You whisper to $N '$t'.", ch, argument, victim, TO_CHAR);
    position = victim->position;
    victim->position = POS_STANDING;
    act("$n whispers to you '$t'.", ch, argument, victim, TO_VICT);
    act("$n whispers something secret to $N.", ch, NULL, victim, TO_NOTVICT);
    victim->position = position;
    return;
}

void
do_ask(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim;
    int                 position;

    if (!IS_NPC(ch) && IS_SET(ch->act, PLR_SILENCE)) {
        send_to_char("You seem to have problems speaking!\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg);

    if (arg[0] == '\0' || !argument || argument[0] == '\0') {
        send_to_char("Ask whom what?\n\r", ch);
        return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (!IS_IMMORTAL(ch) && !IS_AWAKE(victim)) {
        act("$E can't hear you.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
        argument = slur_text(argument);

    act("You ask $N '$t'.", ch, argument, victim, TO_CHAR);
    position = victim->position;
    victim->position = POS_STANDING;
    act("$n asks you '$t'.", ch, argument, victim, TO_VICT);
    act("$n asks $N a question.", ch, NULL, victim, TO_NOTVICT);
    victim->position = position;
    return;
}

void
send_to_room(char *message, ROOM_INDEX_DATA *room)
{
    CHAR_DATA          *vch;
    DUEL_DATA          *duel;

    for (vch = first_char; vch != NULL; vch = vch->next)
        if (IS_AWAKE(vch) && vch->in_room != NULL && vch->in_room == room) {
            send_to_char(message, vch);
            send_to_char("\n\r", vch);
        }

    /* send_to_room doesn't send stuff to people watching spars, so do it */
    if (room->vnum >= DUEL_MIN_ROOM && room->vnum <= DUEL_MAX_ROOM) {
        for (duel = first_duel; duel != NULL; duel = duel->next)
            if (room->vnum == duel->vnum)
                break;

        if (duel) {
            DUEL_WATCHER_DATA *watcher;

            for (watcher = duel->first_watcher; watcher != NULL; watcher = watcher->next)
                if (watcher->ch) {
                    send_to_char("@@d[@@gSPAR@@d] @@N", watcher->ch);
                    send_to_char(message, watcher->ch);
                    send_to_char("\n\r", watcher->ch);
                }
        }
    }

    return;
}

void
do_beep(CHAR_DATA *ch, char *argument)
{
    int                 pos;
    char                arg1[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim;

    one_argument(argument, arg1);

    if (!IS_NPC(ch) && IS_SET(ch->act, PLR_SILENCE)) {
        send_to_char("You are silenced.\n\r", ch);
        return;
    }

    if (arg1[0] == '\0') {
        send_to_char("Usage: BEEP <victim>\n\r", ch);
        return;
    }

    if ((victim = get_char_world(ch, arg1)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("Can only beep players...\n\r", ch);
        return;
    }

    if (ch == victim) {
        send_to_char("Beep yourself?  Sure, ok... BEEP!\a\n\r", ch);
        return;
    }

    if (IS_SET(victim->deaf, CHANNEL_BEEP)) {
        send_to_char("Your victim is ignoring beeps.  Sorry!\n\r", ch);
        return;
    }

    if (!IS_NPC(victim)) {
        IGNORE_DATA        *ignore_list;

        for (ignore_list = victim->pcdata->first_ignore; ignore_list != NULL; ignore_list = ignore_list->next)
            if (!IS_NPC(ch) && !str_cmp(ch->name, ignore_list->char_ignored)) {
                act("$N @@eis IGNORING you!@@N", ch, NULL, victim, TO_CHAR);
                return;
            }
    }

    pos = victim->position;
    victim->position = POS_STANDING;
    act("You beep $N...", ch, NULL, victim, TO_CHAR);
    act("\a$n is beeping you...", ch, NULL, victim, TO_VICT);
    victim->position = pos;
    return;
}

void
ask_quest_question(CHAR_DATA *ch, char *argument)
{
    extern CHAR_DATA   *quest_mob;
    extern CHAR_DATA   *quest_target;
    extern OBJ_DATA    *quest_object;
    extern sh_int       quest_timer;
    extern bool         quest;
    char                buf[MAX_STRING_LENGTH];

    buf[0] = '\0';

    if (!quest || IS_NPC(ch))
        return;

    if ((!strcmp(argument, "who is the thief?"))
        || (!strcmp(argument, "who was the thief?"))
        || (!strcmp(argument, "what mob?"))
        || (!strcmp(argument, "who stole the item?"))) {
        if (quest_mob) {
            if (quest_timer < 7)
                sprintf(buf, "@@eI don't even know who stole it yet!@@N");
            else if (quest_object && quest_target)
                sprintf(buf, "@@lIt was @@N%s@@N@@l who stole my %s@@l", quest_target->short_descr, quest_object->short_descr);
        }
        else if (quest_object)
            sprintf(buf, "@@lDon't worry about who stole my @@N%s@@N@@l, he has recieved his just reward!", quest_object->short_descr);

        if (quest_mob != NULL)
            do_crusade(quest_mob, buf);

        return;
    }

    if (!strcmp(argument, "what item?")) {
        if (quest_mob && quest_object) {
            sprintf(buf, "@@lMy @@N%s @@N@@lwas stolen from me", quest_object->short_descr);
            do_crusade(quest_mob, buf);
            return;
        }
    }

    if (!strcmp(argument, "where are you?"))
        if (quest_mob) {
            sprintf(buf, "@@lYou can find me in %s@@N@@l, please hurry!", quest_mob->in_room->area->name);
            do_crusade(quest_mob, buf);
            return;
        }

    if (!strcmp(argument, "where is the thief?")) {
        if (quest_mob) {
            if (quest_target && quest_timer > 7) {
                if (quest_timer < 10)
                    sprintf(buf, "@@lI don't really know where @@N%s@@N@@l is, let me try and find out", quest_target->short_descr);
                else if (quest_target)
                    sprintf(buf, "@@lI'm not really sure, but I THINK @@N%s@@N@@l is in %s@@N@@l", quest_target->short_descr,
                        quest_target->in_room->area->name);
            }
            else if (quest_target && quest_timer <= 7)
                sprintf(buf, "@@eI don't even know who stole it yet!@@N");
            else
                sprintf(buf, "@@lDon't worry about where the thief who stole my @@N%s@@N@@l is, he has recieved his just reward",
                    quest_object->short_descr);

            do_crusade(quest_mob, buf);
        }
    }

    if (!strcmp(argument, "how long is there left?")) {
        if (quest_mob) {
            sprintf(buf, "@@lI will abandon my quest in @@y%d@@l %s!", 16 - quest_timer, (16 - quest_timer == 1) ? "hour" : "hours");
            do_crusade(quest_mob, buf);
        }
    }

    return;
}

void
list_who_to_output(void)
{
    return;
}

void do_charmpurge(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *vch;
    bool found = FALSE;

    for (vch = first_char; vch != NULL; vch = vch->next)
        if (ch != vch && IS_NPC(vch) && vch->master && vch->master == ch) {
            stop_follower(vch);
            found = TRUE;
        }

    if (!found)
         send_to_char("You do not currently have any charmed mobiles.\n\r", ch);
    
    return;
}
