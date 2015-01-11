
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
#include <ctype.h>
/* For forks etc. */
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "merc.h"
#include "tables.h"
#include "duel.h"
#include "dns.h"

IDSTRING(rcsid, "$Id: act_wiz.c,v 1.99 2004/11/12 01:12:34 dave Exp $");

DECLARE_DO_FUN(build_set_redit);
extern char        *format_eqindex_stats(OBJ_INDEX_DATA *obj);
extern bool write_to_descriptor args((DESCRIPTOR_DATA *desc, char *txt, int length));
extern int          available_qps(CHAR_DATA *ch);

extern bool wizlock;
extern bool auto_quest;
extern bool dbl_xp;
extern bool nopk;
extern unsigned int objid;
extern char *global_nocmd;
extern char *global_nospell;
extern char *nameserver;
extern int elfbonus;
extern int pixbonus;
extern int arenacharm;
extern int save_max_players;
extern int save_max_players_t;
extern int maxdoncount;
extern int avnuminc;
extern int fighttimer;

/* mudset table. variables that save to disk so they maintain themselves over
 * a reboot or crash */
struct mudset_type mudset_table[] = {
    { "wizlock",     &wizlock,            MUDSET_TYPE_BOOL   },
    { "autoquest",   &auto_quest,         MUDSET_TYPE_BOOL   },
    { "dblxp",       &dbl_xp,             MUDSET_TYPE_BOOL   },
    { "nopk",        &nopk,               MUDSET_TYPE_BOOL   },
    { "objid",       &objid,              MUDSET_TYPE_INT    },
    { "nocmd",       &global_nocmd,       MUDSET_TYPE_STRING },
    { "nospell",     &global_nospell,     MUDSET_TYPE_STRING },
    { "elfbonus",    &elfbonus,           MUDSET_TYPE_INT    },
    { "pixbonus",    &pixbonus,           MUDSET_TYPE_INT    },
    { "arenacharm",  &arenacharm,         MUDSET_TYPE_INT    },
    { "nameserver",  &nameserver,         MUDSET_TYPE_STRING },
    { "maxplayers",  &save_max_players,   MUDSET_TYPE_INT    },
    { "maxplayerst", &save_max_players_t, MUDSET_TYPE_INT    },
    { "maxdoncount", &maxdoncount,        MUDSET_TYPE_INT    },
    { "avnuminc",    &avnuminc,           MUDSET_TYPE_INT    },
    { "fighttimer",  &fighttimer,         MUDSET_TYPE_INT    },
    { "",            &wizlock,            MUDSET_TYPE_BOOL   }
};

void
do_wizhelp(CHAR_DATA *ch, char *argument)
{
    char arg[MIL];

    if (argument[0] == '\0') {
        commands_list(ch, TRUE);
        return;
    }

    one_argument(argument, arg);
    if (!send_help(ch, arg, HELP_IMM, FALSE))
        send_to_char("No help on that word.\n\r", ch);

    return;
}

/*
   void do_wizhelp( CHAR_DATA *ch, char *argument )
   {
   CHAR_DATA *rch;
   char       buf  [ MAX_STRING_LENGTH ];
   char       buf1 [ MAX_STRING_LENGTH ];
   int        cmd;
   int        col;

   rch = get_char( ch );

   if ( !authorized( rch, "wizhelp" ) )
   return;

   buf1[0] = '\0';
   col     = 0;

   for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
   {
   if ( cmd_table[cmd].level < LEVEL_HERO
   || str_infix( cmd_table[cmd].name, rch->pcdata->immskll ) )
   continue;
   sprintf( buf, "%-10s", cmd_table[cmd].name );
   strcat( buf1, buf );
   if ( ++col % 8 == 0 )
   strcat( buf1, "\n\r" );
   }

   if ( col % 8 != 0 )
   strcat( buf1, "\n\r" );
   send_to_char( buf1, ch );
   return;
   }

 */
void
do_bamfin(CHAR_DATA *ch, char *argument)
{
    char buf[MSL];
    char *pbuf = buf;
    bool n = FALSE;

    if (!IS_NPC(ch)) {
        smash_tilde(argument);

        while (*argument) {
            if (*argument != '$')
                *pbuf++ = *argument++;
            else if (*(argument + 1) == 'n' && !n) {
                *pbuf++ = '$'; *pbuf++ = 'n';
                argument += 2;
                n = TRUE;
            }
            else
                argument++;
        }

        *pbuf = '\0';

        free_string(ch->pcdata->bamfin);
        ch->pcdata->bamfin = str_dup(buf);
        send_to_char("Ok.\n\r", ch);
    }
    return;
}

void
do_bamfout(CHAR_DATA *ch, char *argument)
{
    char buf[MSL];
    char *pbuf = buf;
    bool n = FALSE;

    if (!IS_NPC(ch)) {
        smash_tilde(argument);

        while (*argument) {
            if (*argument != '$')
                *pbuf++ = *argument++;
            else if (*(argument + 1) == 'n' && !n) {
                *pbuf++ = '$'; *pbuf++ = 'n';
                argument += 2;
                n = TRUE;
            }
            else
                argument++;
        }

        *pbuf = '\0';

        free_string(ch->pcdata->bamfout);
        ch->pcdata->bamfout = str_dup(buf);
        send_to_char("Ok.\n\r", ch);
    }
    return;
}

void
do_deny(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim;
    DESCRIPTOR_DATA     d;

    one_argument(argument, arg);
    if (arg[0] == '\0') {
        send_to_char("Deny whom?\n\r", ch);
        return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL) {
        bool                found = FALSE;

        found = load_char_obj(&d, arg, TRUE);

        if (!found) {
            sendf(ch, "No pFile found for '%s'.\n\r", capitalize(arg));
            free_char(d.character);
            return;
        }

        victim = d.character;
        d.character = NULL;
        victim->desc = NULL;
        LINK(victim, first_char, last_char, next, prev);
        LINK(victim, first_player, last_player, next_player, prev_player);
    }

    if (IS_NPC(victim)) {
        send_to_char("Not on NPC's.\n\r", ch);
        return;
    }

    if (get_trust(victim) >= get_trust(ch)) {
        send_to_char("You failed.\n\r", ch);
        if (victim->desc == NULL)
            do_quit(victim, "NOSAVECHECK");
        return;
    }
    if (IS_SET(victim->act, PLR_DENY)) {
        REMOVE_BIT(victim->act, PLR_DENY);
    }
    else {
        SET_BIT(victim->act, PLR_DENY);
        send_to_char("You are denied access!\n\r", victim);
    }
    send_to_char("OK.\n\r", ch);
    do_quit(victim, "NOSAVECHECK");

    return;
}

void
do_loadlink(CHAR_DATA *ch, char *argument)
{
    char                arg1[MAX_INPUT_LENGTH];
    char                arg2[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim;
    DESCRIPTOR_DATA     d;
    bool                found = FALSE;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0') {
        send_to_char("Loadlink who?\n\r", ch);
        return;
    }

    if (arg2[0] == '\0') {
        if ((victim = get_char_world(ch, arg1)) != NULL) {
            send_to_char("Don't need to load them linkdead, because they're already here!\n\r", ch);
            return;
        }

        arg1[0] = UPPER(arg1[0]);
        found = load_char_obj(&d, arg1, TRUE);

        if (!found) {
            sendf(ch, "No pFile found for '%s'.\n\r", capitalize(arg1));
            free_char(d.character);
            return;
        }

        victim = d.character;
        d.character = NULL;
        victim->desc = NULL;
        LINK(victim, first_char, last_char, next, prev);
        LINK(victim, first_player, last_player, next_player, prev_player);

        if (victim->in_room != NULL)
            char_to_room(victim, victim->in_room);
        else
            char_to_room(victim, get_room_index(2));

        if (get_trust(victim) >= get_trust(ch)) {
            send_to_char("You failed.\n\r", ch);
            do_quit(victim, "NOSAVECHECK");
            return;
        }

        sendf(ch, "@@NLoaded %s linkdead. Use @@aloadlink %s quit@@N to make them quit.\n\r", victim->name, victim->name);
        return;
    }

    if (!str_cmp(arg2, "quit")) {
        if ((victim = get_char_world(ch, arg1)) == NULL)
            send_to_char("They're not here.\n\r", ch);
        else
            do_quit(victim, "NOSAVECHECK");
        return;
    }
    else {
        send_to_char("Invalid extra argument.\n\r", ch);
        return;
    }

}

void
do_disconnect(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA    *d;
    CHAR_DATA          *victim;

    one_argument(argument, arg);
    if (arg[0] == '\0') {
        send_to_char("Disconnect whom?\n\r", ch);
        return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (victim->desc == NULL) {
        act("$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR);
        return;
    }

    for (d = first_desc; d != NULL; d = d->next) {
        if (d == victim->desc) {
            close_socket(d);
            send_to_char("Ok.\n\r", ch);
            return;
        }
    }

    send_to_char("Descriptor not found!\n\r", ch);
    return;
}

void
do_pardon(CHAR_DATA *ch, char *argument)
{
    char                arg1[MAX_INPUT_LENGTH];
    char                arg2[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if ((victim = get_char_world(ch, arg1)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("Not on NPC's.\n\r", ch);
        return;
    }

    if (IS_SET(victim->act, PLR_KILLER))
        REMOVE_BIT(victim->act, PLR_KILLER);
    if (IS_SET(victim->act, PLR_THIEF))
        REMOVE_BIT(victim->act, PLR_THIEF);
    victim->sentence = 0;

    send_to_char("You are no longer wanted.  Praise the gods!\n\r", victim);
    return;
}

void
do_echo(CHAR_DATA *ch, char *argument)
{
    DESCRIPTOR_DATA    *d;
    char                _buf[MSL];
    char               *buf = _buf;

    _buf[0] = 0;

    if (argument[0] == '\0') {
        send_to_char("Echo what?\n\r", ch);
        return;
    }

    while (*argument) {
        if (*argument != '\\' || (*argument == '\\' && *(argument + 1) != 'n'))
            *buf++ = *argument;
        else {
            *buf++ = '\n';
            *buf++ = '\r';
            argument++;
        }

        argument++;
    }

    *buf = 0;

    for (d = first_desc; d; d = d->next) {
        if (d->connected == CON_PLAYING) {
            send_to_char(_buf, d->character);
            send_to_char("@@N\n\r", d->character);
        }
    }

    return;
}

void
do_recho(CHAR_DATA *ch, char *argument)
{
    DESCRIPTOR_DATA    *d;
    char                _buf[MSL];
    char               *buf = _buf;

    _buf[0] = 0;

    if (argument[0] == '\0') {
        send_to_char("Recho what?\n\r", ch);
        return;
    }

    while (*argument) {
        if (*argument != '\\' || (*argument == '\\' && *(argument + 1) != 'n'))
            *buf++ = *argument;
        else {
            *buf++ = '\n';
            *buf++ = '\r';
            argument++;
        }

        argument++;
    }

    *buf = 0;

    for (d = first_desc; d; d = d->next) {
        if (d->connected == CON_PLAYING && d->character->in_room == ch->in_room) {
            send_to_char(_buf, d->character);
            send_to_char("@@N\n\r", d->character);
        }
    }

    return;
}

void
mecho_finished(char *orig, char **dest, CHAR_DATA *ch, bool saved)
{
    DESCRIPTOR_DATA    *d;
    NOTE_DATA          *note = ch->pnote;
    int                 desc_to = -1;
    bool                fAll = FALSE;
    bool                toNotPlaying = FALSE;

    if (!saved || !note || note->text == '\0' || strcmp(note->text, "\n\r") == 0) {
        send_to_char("Mecho cancelled.\n\r", ch);

        if (!note)
            return;

        if (note->text)
            free_string(note->text);
        if (note->to)
            free_string(note->to);

        PUT_FREE(note, note_free);

        return;
    }

    if (!note->to || note->to[0] == '\0')
        fAll = TRUE;
    else if (is_number(note->to))
        desc_to = atoi(note->to);
    else if (!str_cmp(note->to, "notplaying"))
        toNotPlaying = TRUE;

    for (d = first_desc; d != NULL; d = d->next) {
        if ((fAll && d->connected == CON_PLAYING && d->character)
            || (desc_to > -1 && d->descriptor == desc_to)
            || (toNotPlaying && d->connected != CON_PLAYING)
            || (!fAll && d->connected == CON_PLAYING && d->character && !str_cmp(note->to, d->character->name))) {
            if (fAll || (desc_to == -1 && !toNotPlaying))
                send_to_char(note->text, d->character);
            else
                write_to_descriptor(d, note->text, 0);
        }
    }

    free_string(note->to);
    free_string(note->text);
    PUT_FREE(note, note_free);
    return;
}

void
do_mecho(CHAR_DATA *ch, char *argument)
{
    NOTE_DATA          *note;

    note = NULL;
    GET_FREE(note, note_free);
    note->to = str_dup(argument);
    ch->pnote = note;

    write_start(&note->text, (void *) mecho_finished, (void *) NULL, ch);

    return;
}

ROOM_INDEX_DATA    *
find_location(CHAR_DATA *ch, char *arg)
{
    CHAR_DATA          *victim;
    OBJ_DATA           *obj;

    if (is_number(arg))
        return get_room_index(atoi(arg));

    if ((victim = get_char_world(ch, arg)) != NULL)
        return victim->in_room;

    if ((obj = get_obj_world(ch, arg)) != NULL)
        return obj->in_room;

    return NULL;
}

void
do_transfer(CHAR_DATA *ch, char *argument)
{
    char                arg1[MAX_INPUT_LENGTH];
    char                arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA    *location;
    DESCRIPTOR_DATA    *d;
    CHAR_DATA          *victim;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0') {
        send_to_char("Transfer whom (and where)?\n\r", ch);
        return;
    }

    if (!str_cmp(arg1, "all")) {
        for (d = first_desc; d != NULL; d = d->next) {
            if (d->connected == CON_PLAYING && !IS_IMMORTAL(d->character)
                && d->character != ch && d->character->in_room != NULL && can_see(ch, d->character)) {
                char                buf[MAX_STRING_LENGTH];

                sprintf(buf, "%s %s", d->character->name, arg2);
                do_transfer(ch, buf);
            }
        }
        return;
    }

    /*
     * Thanks to Grodyn for the optional location parameter.
     */
    if (arg2[0] == '\0') {
        location = ch->in_room;
    }
    else {
        if ((location = find_location(ch, arg2)) == NULL) {
            send_to_char("No such location.\n\r", ch);
            return;
        }

        if (room_is_private(location)) {
            send_to_char("That room is private right now.\n\r", ch);
            return;
        }
    }

    if ((victim = get_char_world(ch, arg1)) == NULL) {
        send_to_char("They're not here.\n\r", ch);
        return;
    }

    if (victim->in_room == NULL) {
        send_to_char("They are in limbo.\n\r", ch);
        return;
    }

#ifdef BPORT
    if (!IS_NPC(victim) && !IS_SET(location->area->flags, AREA_BUILDVISIT) && get_trust(ch) < 83 && !build_canread(location->area, victim, 0)) {
        send_to_char("They aren't allowed to go there.\n\r", ch);
        return;
    }
#endif

    if (!IS_NPC(victim) && victim->timer >= 12 && victim->was_in_room && victim->in_room == get_room_index(ROOM_VNUM_LIMBO)) {
        send_to_char("They're idling and are in Limbo. Changed the room they will return to when they stop idling.\n\r", ch);
        victim->was_in_room = location;
        return;
    }

    if (victim->fighting != NULL)
        stop_fighting(victim, TRUE);
    act("$n is snatched by the Gods!", victim, NULL, NULL, TO_ROOM);
    char_from_room(victim);
    char_to_room(victim, location);
    act("$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM);
    if (ch != victim)
        act("$n has transferred you.", ch, NULL, victim, TO_VICT);
    do_look(victim, "auto");
    send_to_char("Ok.\n\r", ch);
}

void
do_at(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA    *location;
    ROOM_INDEX_DATA    *original;
    CHAR_DATA          *wch;

    argument = one_argument(argument, arg);

    if (arg[0] == '\0' || argument[0] == '\0') {
        send_to_char("At where what?\n\r", ch);
        return;
    }

    if ((location = find_location(ch, arg)) == NULL) {
        send_to_char("No such location.\n\r", ch);
        return;
    }

    if (room_is_private(location) && (ch->level != 90)) {
        send_to_char("That room is private right now.\n\r", ch);
        return;
    }

    original = ch->in_room;
    char_from_room(ch);
    char_to_room(ch, location);
    interpret(ch, argument);

    /*
     * See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case.
     */
    for (wch = first_char; wch != NULL; wch = wch->next) {
        if (wch == ch) {
            char_from_room(ch);
            char_to_room(ch, original);
            break;
        }
    }

    return;
}

void
do_immortal(CHAR_DATA *ch, char *argument)
{
    char                arg1[MSL];
    char                arg2[MSL];
    char                buf[MSL];
    CHAR_DATA          *victim;
    int                 count;
    int                 value;
    int                 iClass;
    int                 sn;

    send_to_char("This command is being worked on!\n\r", ch);
    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0' || arg2[0] == '\0') {
        send_to_char("Syntax: makeimm <name> <level>\n\r", ch);
        return;
    }

    if ((victim = get_char_world(ch, arg1)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("Not on mobs.\n\r", ch);
        return;
    }

    value = is_number(arg2) ? atoi(arg2) : -9;

    if (value == -9) {
        send_to_char("Invalid value for value\n\r\n\r", ch);
        return;
    }

    for (count = 0; count < 6; count++) {
        for (iClass = victim->lvl[count]; iClass < value; iClass++) {
            victim->lvl[count] += 1;
            advance_level(victim, count, FALSE, TRUE, FALSE);
            victim->lvl2[count] += 1;
            advance_level(victim, count, FALSE, FALSE, FALSE);
        }
    }

    for (sn = 0; sn < MAX_SKILL; sn++) {
        if (skill_table[sn].name != NULL)
            victim->pcdata->learned[sn] = 99;
    }

    victim->pcdata->perm_str = 25;
    victim->pcdata->max_str = 25;
    victim->pcdata->perm_int = 25;
    victim->pcdata->max_int = 25;
    victim->pcdata->perm_wis = 25;
    victim->pcdata->max_wis = 25;
    victim->pcdata->perm_dex = 25;
    victim->pcdata->max_dex = 25;
    victim->pcdata->perm_con = 25;
    victim->pcdata->max_con = 25;

    victim->adept_level = 20;
    victim->level = atoi(arg2);

    sprintf(buf, " %s %s", victim->name, get_adept_name(victim));
    do_whoname(ch, buf);

    do_togbuild(ch, victim->name);
    do_holylight(victim, "");
    do_wizify(ch, victim->name);
    do_save(victim, "");
    act("$N is now an immortal.", ch, NULL, victim, TO_CHAR);
    act("$n has made you an immortal!", ch, NULL, victim, TO_VICT);
}

void
do_goto(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    char                buf[MSL];
    ROOM_INDEX_DATA    *location;

    one_argument(argument, arg);
    if (arg[0] == '\0') {
        send_to_char("Goto where?\n\r", ch);
        return;
    }

    if ((location = find_location(ch, arg)) == NULL) {
        send_to_char("No such location.\n\r", ch);
        return;
    }

    if (room_is_private(location) && ch->level < MAX_LEVEL) {
        send_to_char("That room is private right now.\n\r", ch);
        return;
    }

#ifdef BPORT
    if (!IS_SET(location->area->flags, AREA_BUILDVISIT) && get_trust(ch) < 83 && !build_canread(location->area, ch, 0)) {
        send_to_char("You aren't allowed to go there.\n\r", ch);
        return;
    }
#endif

    if (ch->fighting != NULL)
        stop_fighting(ch, TRUE);

    if (IS_NPC(ch) || ch->pcdata->bamfout[0] == '\0')
        act("$L$n leaves in a swirling mist", ch, NULL, NULL, TO_ROOM);
    else if (strpos(ch->pcdata->bamfout, "$n") == -1)
        act("$L$n $T", ch, NULL, ch->pcdata->bamfout, TO_ROOM);
    else {
        sprintf(buf, "$L%s", ch->pcdata->bamfout);
        act(buf, ch, NULL, NULL, TO_ROOM);
    }

    char_from_room(ch);
    char_to_room(ch, location);

    if (IS_NPC(ch) || ch->pcdata->bamfin[0] == '\0')
        act("$L$n appears in a swirling mist", ch, NULL, NULL, TO_ROOM);
    else if (strpos(ch->pcdata->bamfin, "$n") == -1)
        act("$L$n $T", ch, NULL, ch->pcdata->bamfin, TO_ROOM);
    else {
        sprintf(buf, "$L%s", ch->pcdata->bamfin);
        act(buf, ch, NULL, NULL, TO_ROOM);
    }

    do_look(ch, "auto");
    if (ch->position == POS_BUILDING && ch->act_build == ACT_BUILD_REDIT)
        build_set_redit(ch, "");

    return;
}

void
do_rstat(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    char                buf1[MAX_STRING_LENGTH * 10];
    char                arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA    *location;
    ROOM_AFFECT_DATA   *raf;
    OBJ_DATA           *obj;
    CHAR_DATA          *rch;
    int                 door;

    one_argument(argument, arg);
    location = (arg[0] == '\0') ? ch->in_room : find_location(ch, arg);
    if (location == NULL) {
        send_to_char("No such location.\n\r", ch);
        return;
    }

    if (ch->in_room != location && room_is_private(location) && (ch->level != 90)) {
        send_to_char("That room is private right now.\n\r", ch);
        return;
    }

    buf1[0] = '\0';

    sprintf(buf, "Name: '%s.'\n\rArea: '%s'.\n\r", location->name, location->area->name);
    strcat(buf1, buf);

    sprintf(buf,
        "Vnum: %d.  Light: %d.  Sector: %s.\n\r", location->vnum, location->light, rev_table_lookup(tab_sector_types, location->sector_type));
    strcat(buf1, buf);

    sprintf(buf, "Room flags: %s.\n\rDescription:\n\r%s", bit_table_lookup(tab_room_flags, location->room_flags), location->description);
    strcat(buf1, buf);

    if (location->first_exdesc != NULL) {
        EXTRA_DESCR_DATA   *ed;

        strcat(buf1, "Extra description keywords: '");
        for (ed = location->first_exdesc; ed; ed = ed->next) {
            strcat(buf1, ed->keyword);
            if (ed->next != NULL)
                strcat(buf1, " ");
        }
        strcat(buf1, "'.\n\r");
    }

    if (location->nocmd && location->nocmd[0] != '\0') {
        sprintf(buf, "@@N@@gDisallowed commands: @@y%s@@N\n\r", location->nocmd);
        strcat(buf1, buf);
    }

    if (location->nospell && location->nospell[0] != '\0') {
        sprintf(buf, "@@N@@gDisallowed spells: @@y%s@@N\n\r", location->nospell);
        strcat(buf1, buf);
    }

    strcat(buf1, "Characters:");
    for (rch = location->first_person; rch; rch = rch->next_in_room) {
        strcat(buf1, " ");
        one_argument(rch->name, buf);
        strcat(buf1, buf);
    }

    strcat(buf1, ".\n\rObjects:   ");
    for (obj = location->first_content; obj; obj = obj->next_in_room) {
        strcat(buf1, " ");
        one_argument(obj->name, buf);
        strcat(buf1, buf);
    }
    strcat(buf1, ".\n\r");

    for (door = 0; door <= 5; door++) {
        EXIT_DATA          *pexit;

        if ((pexit = location->exit[door]) != NULL) {
            sprintf(buf,
                "Door: %d.  To: %d.  Key: %d.  Exit flags: %d.\n\rKeyword: '%s'.  Description: %s",
                door,
                pexit->to_room != NULL ? pexit->to_room->vnum : 0,
                pexit->key, pexit->exit_info, pexit->keyword, pexit->description[0] != '\0' ? pexit->description : "(none).\n\r");
            strcat(buf1, buf);
        }
    }

    if (location->first_room_affect != NULL) {
        for (raf = location->first_room_affect; raf != NULL; raf = raf->next) {
            sprintf(buf, "Room_Affect: '%s', level %d, duration %d\n\r", skill_table[raf->type].name, raf->level, raf->duration);
            strcat(buf1, buf);
        }
    }

    send_to_char(buf1, ch);
    return;
}

void
do_ostat(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH*4];
    char                buf1[MAX_STRING_LENGTH];
    char                arg[MAX_INPUT_LENGTH];
    AFFECT_DATA        *paf;
    OBJ_DATA           *obj;
    int                 cnt;
    int                 fubar;
    char               *foo;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
        send_to_char("Ostat what?\n\r", ch);
        return;
    }

    buf1[0] = '\0';

    if ((obj = get_obj_world(ch, arg)) == NULL) {
        send_to_char("Nothing like that in hell, earth, or heaven.\n\r", ch);
        return;
    }

    sprintf(buf, "Name: %s@@N.\n\r", obj->name);
    strcat(buf1, buf);

    if (obj->id > 0) {
        sprintf(buf, "@@eID: @@y%d@@N.\n\r", obj->id);
        strcat(buf1, buf);
    }

    sprintf(buf, "Vnum: %d.  Type: %s.\n\r", obj->pIndexData->vnum, item_type_name(obj));
    strcat(buf1, buf);

    sprintf(buf, "Short description: %s.\n\rLong description: %s\n\r", obj->short_descr, obj->description);
    strcat(buf1, buf);

    sprintf(buf, "Wear bits: %s.\n\rExtra bits: %s.\n\r", bit_table_lookup(tab_wear_flags, obj->wear_flags), extra_bit_name(obj->extra_flags));
    strcat(buf1, buf);

    sprintf(buf, "ITEM_APPLY: %s.\n\r", bit_table_lookup(tab_item_apply, obj->item_apply));
    strcat(buf1, buf);

    sprintf(buf, "Number: %d/%d.  Weight: %d/%d.\n\r", 1, get_obj_number(obj), obj->weight, get_obj_weight(obj));
    strcat(buf1, buf);

    sprintf(buf, "Cost: %d.  Alternative Cost? %s. Timer: %d.  Level: %d.\n\r",
        obj->cost, obj->newcost == FALSE ? "No" : "Yes", obj->timer, obj->level);
    strcat(buf1, buf);

    sprintf(buf,
        "In room: %d.  In object: %s.  Carried by: %s.  Wear_loc: %d.\n\r",
        obj->in_room == NULL ? 0 : obj->in_room->vnum,
        obj->in_obj == NULL ? "(none)" : obj->in_obj->short_descr, obj->carried_by == NULL ? "(none)" : obj->carried_by->name, obj->wear_loc);
    strcat(buf1, buf);

    if (obj->pIndexData->rarity != 0) {
        sprintf(buf, "@@WCurrent Rarity level: @@y1/%d.@@N\n\r", obj->pIndexData->rarity);
        strcat(buf1, buf);
    }

    strcat(buf1, "Item Values:\n\r");
    for (cnt = 0; cnt < 4; cnt++) {
        sprintf(buf, "@@W[Value%d : @@y%6d@@W] %s", cnt, obj->value[cnt], rev_table_lookup(tab_value_meanings, (obj->item_type * 10) + cnt));
        strcat(buf1, buf);
        if (is_name("Spell", rev_table_lookup(tab_value_meanings, (obj->item_type * 10) + cnt))) {
            fubar = obj->value[cnt];
            if (fubar < 0 || fubar > MAX_SKILL)
                sprintf(buf, "               @@R(????\?)@@g\n\r");
            else
                sprintf(buf, "               @@y(%s)@@g\n\r", skill_table[fubar].name);

        }
        else if (is_name("Liquid", rev_table_lookup(tab_value_meanings, (obj->item_type * 10) + cnt))) {
            foo = str_dup(rev_table_lookup(tab_drink_types, obj->value[cnt]));
            if (foo[0] == '\0')
                sprintf(buf, "                  @@R(INVALID!)@@g\n\r");
            else
                sprintf(buf, "                  @@y(%s)@@g\n\r", foo);
        }
        else if (is_name("Weapon", rev_table_lookup(tab_value_meanings, (obj->item_type * 10) + cnt))) {
            foo = rev_table_lookup(tab_weapon_types, obj->value[cnt]);
            if (foo[0] == '\0')
                sprintf(buf, "                  @@R(INVALID!)@@g\n\r");
            else
                sprintf(buf, "                  @@y(%s)@@g\n\r", foo);
        }
        else
            sprintf(buf, "@@g\n\r");
        strcat(buf1, buf);
    }

    /*    
       sprintf( buf, "Values: %d %d %d %d.\n\r",
       obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
       strcat( buf1, buf );
     */

    if (obj->first_exdesc != NULL || obj->pIndexData->first_exdesc != NULL) {
        EXTRA_DESCR_DATA   *ed;

        strcat(buf1, "Extra description keywords: '");

        for (ed = obj->first_exdesc; ed != NULL; ed = ed->next) {
            strcat(buf1, ed->keyword);
            if (ed->next != NULL)
                strcat(buf1, " ");
        }

        for (ed = obj->pIndexData->first_exdesc; ed != NULL; ed = ed->next) {
            strcat(buf1, ed->keyword);
            if (ed->next != NULL)
                strcat(buf1, " ");
        }

        strcat(buf1, "'.\n\r");
    }

    /* If the object has an obj_fun, print it out */
    if (obj->obj_fun && strcmp(rev_obj_fun_lookup(obj->obj_fun), "")) {
        sprintf(buf, "@@aObj_Fun: @@c%s@@g.@@N\n\r", rev_obj_fun_lookup(obj->obj_fun));
        strcat(buf1, buf);
    }

    for (paf = obj->first_apply; paf != NULL; paf = paf->next) {
        sprintf(buf, "Affects %s by %d.\n\r", affect_loc_name(paf->location), paf->modifier);
        strcat(buf1, buf);
    }

    send_to_char(buf1, ch);
    return;
}

void
do_mstat(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    char                buf1[MAX_STRING_LENGTH];
    char                arg[MAX_INPUT_LENGTH];
    AFFECT_DATA        *paf;
    CHAR_DATA          *victim;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
        send_to_char("Mstat whom?\n\r", ch);
        return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    buf1[0] = '\0';

    sprintf(buf, "@@dName@@y:@@g %s@@y.  @@dRace @@g%i\n\r", victim->name, victim->race);
    strcat(buf1, buf);

    sprintf(buf, "@@dVnum@@y: @@g%d@@y.  @@dSex@@y: @@g%s@@y.  @@dRoom@@y: @@g%d@@.\n\r",
        IS_NPC(victim) ? victim->pIndexData->vnum : 0,
        victim->sex == SEX_MALE ? "male" : victim->sex == SEX_FEMALE ? "female" : "neutral", victim->in_room == NULL ? 0 : victim->in_room->vnum);
    strcat(buf1, buf);

    if (IS_NPC(victim)) {
        sprintf(buf, "Str: %d.  Int: %d.  Wis: %d.  Dex: %d.  Con: %d.\n\r",
            get_curr_str(victim), get_curr_int(victim), get_curr_wis(victim), get_curr_dex(victim), get_curr_con(victim));
        strcat(buf1, buf);
    }
    else {
        sprintf(buf,
            "@@dStr@@y:@@g%d@@y/@@g%d @@d Int@@y:@@g%d@@y/@@g%d@@d  Wis@@y:@@g%d@@y/@@g%d@@d  Dex@@y:@@g%d@@y/@@g%d@@d  Con@@y:@@g%d@@y/@@g%d@@y.\n\r",
            get_curr_str(victim), victim->pcdata->max_str, get_curr_int(victim), victim->pcdata->max_int, get_curr_wis(victim),
            victim->pcdata->max_wis, get_curr_dex(victim), victim->pcdata->max_dex, get_curr_con(victim), victim->pcdata->max_con);
        strcat(buf1, buf);
    }

    if (!IS_NPC(victim)) {
        sprintf(buf,
            "@@dMag@@y:@@g %d @@dCle@@y:@@g %d @@dThi@@y:@@g %d @@dWar@@y: @@g%d @@dPsi@@y:@@g %d\n\r",
            victim->lvl[0], victim->lvl[1], victim->lvl[2], victim->lvl[3], victim->lvl[4]);
        strcat(buf1, buf);
        if (ch->level >= MAX_LEVEL) {
            sprintf(buf,
                "@@dSor@@y: @@g%d@@d Ass@@y: @@g%d@@d Kni@@y: @@g%d@@d Nec@@y: @@g%d@@d Mon@@y: @@g%d@@N\n\r",
                victim->lvl2[0], victim->lvl2[1], victim->lvl2[2], victim->lvl2[3], victim->lvl2[4]);
            strcat(buf1, buf);
        }

        if (ch->level >= MAX_LEVEL) {
            sprintf(buf, "@@dPseudo Level@@y: @@g%d @@N", get_pseudo_level(victim));
            strcat(buf1, buf);
        }
        sprintf(buf, "@@dAdept level@@y: @@g%d@@N\n\r", victim->adept_level);
        strcat(buf1, buf);

        sprintf(buf, "@@dAge@@y:@@g ");
        my_get_age(victim, buf);
        strcat(buf1, buf);
        sprintf(buf, "   @@y(@@g%d @@dHours RL play@@y).\n\r", my_get_hours(victim));
        strcat(buf1, buf);

        sprintf(buf, "@@dClass Order@@y: @@g%s %s %s %s %s@@N\n\r",
            class_table[victim->pcdata->order[0]].who_name,
            class_table[victim->pcdata->order[1]].who_name,
            class_table[victim->pcdata->order[2]].who_name,
            class_table[victim->pcdata->order[3]].who_name, class_table[victim->pcdata->order[4]].who_name);
        strcat(buf1, buf);

    }

    sprintf(buf, "@@dHp@@y: @@g%d@@y/@@g%d@@y.  @@dMana@@y: @@g%d@@y/@@g%d@@y.  @@dMove@@y: @@g%d@@y/@@g%d@@y.  @@dEnergy@@y: @@g%d@@y/@@g%d@@y.  @@dPractices@@y: @@g%d@@y.\n\r",
        victim->hit, victim->max_hit, victim->mana, victim->max_mana, victim->move, victim->max_move, victim->energy, victim->max_energy, victim->practice);
    strcat(buf1, buf);

    sprintf(buf,
        "@@dLv@@y:@@g %d@@y.@@d  Class@@y:@@g %d@@y.  @@dAlign@@y:@@g %d@@y.@@d  AC@@y:@@g %d@@y.  @@dGold@@y:@@g %d@@y.  @@dExp@@y:@@g %d@@y.\n\r",
        victim->level, victim->class, victim->alignment, GET_AC(victim), victim->gold, victim->exp);
    strcat(buf1, buf);

    if (!IS_NPC(victim)) {
        sprintf(buf, "@@dRace@@y: @@g%d @@y(@@d%s@@y).  @@d Clan@@y: @@g%d @@y(@@d%s@@y).  @@dBalance@@y:@@g %d@@y.\n\r",
            victim->race,
            race_table[victim->race].race_name,
            victim->pcdata->clan, clan_table[victim->pcdata->clan].clan_abbr, victim->balance);
        strcat(buf1, buf);
    }

    sprintf(buf, "@@dHitroll@@y: @@g%d@@y.@@d  Damroll@@y: @@g%d@@y.  @@dPosition@@y: @@g%d@@y.  @@dWimpy@@y: @@g%d@@y.\n\r",
        GET_HITROLL(victim), GET_DAMROLL(victim), victim->position, victim->wimpy);
    strcat(buf1, buf);

    if (IS_NPC(victim)) {
        sprintf(buf, "@@dMODIFIERS@@y: @@dAC@@y: @@g%d@@y.  @@dHitroll@@y: @@g%d@@y. @@d Damroll@@y:@@g %d@@y.\n\r",
            victim->ac_mod, victim->hr_mod, victim->dr_mod);
        strcat(buf1, buf);
        sprintf(buf, "@@dTARGET@@y:@@g %s\n\r", victim->target);
        strcat(buf1, buf);
        sprintf(buf, "@@dTIMER@@y:@@g %d\n\r", victim->extract_timer);
        strcat(buf1, buf);
    }

    if (!IS_NPC(victim)) {
        sprintf(buf, "@@dPage Lines@@y:@@g %d@@y. ", victim->pcdata->pagelen);
        strcat(buf1, buf);
        sprintf(buf, "@@dSentence@@y: @@g%d@@y. ", victim->sentence);
        strcat(buf1, buf);
    }
    sprintf(buf, "@@dFighting@@y:@@g %s@@y. \n\r", victim->fighting ? victim->fighting->name : "(none)");
    strcat(buf1, buf);

    if (!IS_NPC(victim) && (ch->level >= MAX_LEVEL)) {
        sprintf(buf,
            "@@dThirst@@y: @@g%d@@y. @@d Full@@y: @@g%d@@y.  @@dDrunk@@y: @@g%d@@y.  @@dSaving throw@@y: @@g%d@@y.\n\r",
            victim->pcdata->condition[COND_THIRST],
            victim->pcdata->condition[COND_FULL], victim->pcdata->condition[COND_DRUNK], victim->saving_throw);
        strcat(buf1, buf);
    }

    sprintf(buf, "@@dCarry number@@y: @@g%d@@y.  @@dCarry weight@@y:@@g %d@@y.   @@aQuest Points@@y: @@g%d + %d@@N\n\r",
        victim->carry_number, victim->carry_weight, available_qps(victim), victim->quest_points - available_qps(victim));
    strcat(buf1, buf);

    if (!IS_NPC(victim)) {
        sprintf(buf, "@@dAge@@y:@@g %d@@y.  @@dPlayed@@y:@@g %d@@y. @@d Timer@@y:@@g %d@@y.@@d  Act@@y:@@g %s@@y.\n\r",
            get_age(victim), (int) victim->played, victim->timer, bit_table_lookup(tab_pc_act_flags, victim->act));
        strcat(buf1, buf);
    }
    else {
        sprintf(buf, "@@dAct@@y:@@g %s@@y.\n\r", bit_table_lookup(tab_mob_flags, victim->act));
        strcat(buf1, buf);
    }

    sprintf(buf, "@@dMaster@@y:@@g %s@@y.  @@dLeader@@y:@@g %s@@y.  @@dAffected by@@y:@@g %s@@y.\n\r",
        victim->master ? victim->master->name : "(none)", victim->leader ? victim->leader->name : "(none)", affect_bit_name(victim->affected_by));
    strcat(buf1, buf);

    sprintf(buf, "@@dShort description@@y:@@g %s@@y.\n\r@@dLong  description@@y:@@g %s",
        victim->short_descr, victim->long_descr[0] != '\0' ? victim->long_descr : "@@y(@@dnone@@y).@@N\n\r");
    strcat(buf1, buf);

    if (IS_NPC(victim) && victim->spec_fun != 0)
        strcat(buf1, "@@dMobile has spec fun@@y.\n\r");

    /*    if ( IS_NPC( victim ) 
       && IS_SET( victim->act_hunt, ACT_HUNT_MOVE )
       && victim->move_to != NO_VNUM )
       {
       sprintf( buf, "@@GMoving to room vnum: (%d) %s.@@g\n\r", victim->move_to,
       victim->movename );
       strcat( buf1, buf );
       } */

    strcpy(buf, "@@dMoving to room vnum@@y: (@@g%d@@y) @@g%s@@y.@@N\n\r");
    if (victim->hunting)
        sprintf(buf1 + strlen(buf1), buf, victim->hunting->in_room->vnum, victim->hunting->in_room->name);
    else if (victim->hunt_obj && victim->hunt_obj->in_room)
        sprintf(buf1 + strlen(buf1), buf, victim->hunt_obj->in_room->vnum, victim->hunt_obj->in_room->name);

    /*    if ( IS_NPC(victim) && victim->hunting != NULL)
       {
       switch( (int) victim->hunting)
       {
       case -1:
       sprintf(buf, "Hunting victim: %s (waiting)\n\r",victim->huntname);
       strcat(buf1,buf);
       break;

       case -2:
       sprintf(buf, "Returning home\n\r");
       strcat(buf1,buf);
       break;

       default:
       sprintf(buf, "Hunting victim: %s (%s)\n\r",
       IS_NPC(victim->hunting) ? victim->hunting->short_descr
       : victim->hunting->name,
       IS_NPC(victim->hunting) ? "MOB" : "PLAYER" );
       strcat(buf1, buf);
       if (victim->huntdirs != NULL)
       {
       sprintf(buf,"Steps to victim: %i\n\r",
       strlen(victim->huntdirs)-victim->huntdirno);
       strcat(buf1,buf);
       }
       }
       } */

    if (victim->hunting || victim->hunt_obj) {
        buf[0] = '\0';
        if (victim->hunting)
            sprintf(buf + strlen(buf), " hunting for (%s) %s", (IS_NPC(victim->hunting) ? "mobile" : "player"), NAME(victim->hunting));
        if (victim->hunt_obj) {
            if (victim->hunting && IS_SET(victim->hunt_flags, HUNT_CR) && victim->hunt_obj->item_type == ITEM_CORPSE_PC)
                strcat(buf, " to return a corpse");
            else
                sprintf(buf + strlen(buf), " looking for (object) %s", victim->hunt_obj->short_descr);
        }
        if (IS_NPC(victim) && IS_SET(victim->hunt_flags, HUNT_MERC | HUNT_CR) && victim->hunt_for)
            sprintf(buf + strlen(buf), ", employed by %s", NAME(victim->hunt_for));
        strcat(buf, ".\n\r");
        buf[1] = UPPER(buf[1]);
        strcat(buf1, buf + 1);
    }
    else if (victim->searching) {
        sprintf(buf, "Searching for %s.\n\r", victim->searching);
        strcat(buf1, buf);
    }

    for (paf = victim->first_affect; paf != NULL; paf = paf->next) {
        sprintf(buf,
            "Spell: '%s' modifies %s by %d for %d hours with bits %s.\n\r",
            skill_table[(int) paf->type].name, affect_loc_name(paf->location), paf->modifier, paf->duration, affect_bit_name(paf->bitvector)
            );
        strcat(buf1, buf);
    }

    send_to_char(buf1, ch);
    return;
}

void
do_olmsg(CHAR_DATA *ch, char *argument)
{
    if (!IS_NPC(ch)) {
        smash_tilde(argument);
        free_string(ch->pcdata->load_msg);
        ch->pcdata->load_msg = str_dup(argument);
        send_to_char("Ok.\n\r", ch);
    }
    return;
}

void
do_ofindlev(CHAR_DATA *ch, char *argument)
{
    extern int          top_obj_index;
    char                buf[MAX_STRING_LENGTH];
    char                buf1[MAX_STRING_LENGTH * 10];
    char                arg[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA     *pObjIndex;
    int                 vnum;
    int                 nMatch;
    bool                fAll;
    bool                found;
    int                 level;
    int                 objlev;

    one_argument(argument, arg);
    if (arg[0] == '\0') {
        send_to_char("Ofindlev what lev.?\n\r", ch);
        return;
    }

    buf1[0] = '\0';
    fAll = !str_cmp(arg, "all");
    found = FALSE;
    nMatch = 0;
    level = is_number(arg) ? atoi(arg) : 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_mob_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for (vnum = 0; nMatch < top_obj_index; vnum++) {
        if ((pObjIndex = get_obj_index(vnum)) != NULL) {
            nMatch++;
            if (fAll || pObjIndex->level == level) {
                found = TRUE;
                objlev = pObjIndex->level;

                if (IS_SET(pObjIndex->extra_flags, ITEM_REMORT)) {
                    sprintf(buf, "@@N(@@mREMORTAL@@N) [%5d] %s\n\r", pObjIndex->vnum, capitalize(pObjIndex->short_descr));
                    safe_strcat(MAX_STRING_LENGTH, buf1, buf);
                }
                else if (IS_SET(pObjIndex->extra_flags, ITEM_ADEPT)) {
                    sprintf(buf, "@@N(@@W   ADEPT@@N) [%5d] %s\n\r", pObjIndex->vnum, capitalize(pObjIndex->short_descr));
                    safe_strcat(MAX_STRING_LENGTH, buf1, buf);
                }
                else {
                    sprintf(buf, "@@N(@@g  MORTAL@@N) [%5d] %s\n\r", pObjIndex->vnum, capitalize(pObjIndex->short_descr));
                    safe_strcat(MAX_STRING_LENGTH, buf1, buf);
                }
            }
        }
    }

    if (!found) {
        send_to_char("Nothing like that in hell, earth, or heaven.\n\r", ch);
        return;
    }

    send_to_char(buf1, ch);
    return;
}

void
do_mfind(CHAR_DATA *ch, char *argument)
{
    extern int          top_mob_index;
    char                buf[MAX_STRING_LENGTH];
    char                buf1[MAX_STRING_LENGTH];
    char                arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA     *pMobIndex;
    int                 vnum;
    int                 nMatch;
    bool                fAll;
    bool                found;

    one_argument(argument, arg);
    if (arg[0] == '\0') {
        send_to_char("Mfind whom?\n\r", ch);
        return;
    }

    buf1[0] = '\0';
    fAll = !str_cmp(arg, "all");
    found = FALSE;
    nMatch = 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_mob_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for (vnum = 0; nMatch < top_mob_index; vnum++) {
        if ((pMobIndex = get_mob_index(vnum)) != NULL) {
            nMatch++;
            if (fAll || is_name(arg, pMobIndex->player_name)) {
                found = TRUE;
                sprintf(buf, "@@N[%5d] %s\n\r", pMobIndex->vnum, capitalize(pMobIndex->short_descr));
                safe_strcat(MAX_STRING_LENGTH, buf1, buf);
            }
        }
    }

    if (!found) {
        send_to_char("Nothing like that in hell, earth, or heaven.\n\r", ch);
        return;
    }

    send_to_char(buf1, ch);
    return;
}

void
do_mfindlev(CHAR_DATA *ch, char *argument)
{
    extern int          top_mob_index;
    char                buf[MAX_STRING_LENGTH];
    char                buf1[MAX_STRING_LENGTH];
    char                arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA     *pMobIndex;
    int                 vnum;
    int                 nMatch;
    bool                fAll;
    bool                found;
    int                 level;
    int                 perkills, moblev;

    one_argument(argument, arg);
    if (arg[0] == '\0') {
        send_to_char("Mfindlev what lev.?\n\r", ch);
        return;
    }

    buf1[0] = '\0';
    fAll = !str_cmp(arg, "all");
    found = FALSE;
    nMatch = 0;
    level = is_number(arg) ? atoi(arg) : 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_mob_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for (vnum = 0; nMatch < top_mob_index; vnum++) {
        if ((pMobIndex = get_mob_index(vnum)) != NULL) {
            nMatch++;
            if (fAll || pMobIndex->level == level) {
                found = TRUE;
                moblev = pMobIndex->level;
                if (kill_table[moblev].killed == 0)
                    perkills = 0;
                else
                    perkills = (pMobIndex->killed * 100) / (kill_table[moblev].killed);

                sprintf(buf, "@@N(%3d) [%5d] %s\n\r", perkills, pMobIndex->vnum, capitalize(pMobIndex->short_descr));
                safe_strcat(MAX_STRING_LENGTH, buf1, buf);
            }
        }
    }

    if (!found) {
        send_to_char("Nothing like that in hell, earth, or heaven.\n\r", ch);
        return;
    }

    send_to_char(buf1, ch);
    return;
}

void
do_ofind(CHAR_DATA *ch, char *argument)
{
    extern int          top_obj_index;
    char                buf[MAX_STRING_LENGTH];
    char                buf1[MAX_STRING_LENGTH];
    char                arg[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA     *pObjIndex;
    int                 vnum;
    int                 nMatch;
    bool                fAll;
    bool                found;

    one_argument(argument, arg);
    if (arg[0] == '\0') {
        send_to_char("Ofind what?\n\r", ch);
        return;
    }

    buf1[0] = '\0';
    fAll = !str_cmp(arg, "all");
    found = FALSE;
    nMatch = 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_obj_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for (vnum = 0; nMatch < top_obj_index; vnum++) {
        if ((pObjIndex = get_obj_index(vnum)) != NULL) {
            nMatch++;
            if (fAll || is_name(arg, pObjIndex->name)) {
                found = TRUE;
                sprintf(buf, "@@N[%5d] %s\n\r", pObjIndex->vnum, pObjIndex->short_descr);
                safe_strcat(MAX_STRING_LENGTH, buf1, buf);
            }
        }
    }

    if (!found) {
        send_to_char("Nothing like that in hell, earth, or heaven.\n\r", ch);
        return;
    }

    send_to_char(buf1, ch);
    return;
}

void
do_mwhere(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim;
    bool                found;
    int                 cnt = 0;

    one_argument(argument, arg);
    if (arg[0] == '\0') {
        send_to_char("Mwhere whom? or int to see the intelligent mobs.\n\r", ch);
        return;
    }

    found = FALSE;
    for (victim = first_char; victim != NULL; victim = victim->next) {
        if (IS_NPC(victim)
            && victim->in_room != NULL && is_name(arg, victim->name)) {
            found = TRUE;
            sendf(ch, "[%5d] [%5d] %-20s [%5d] %-30s\n\r",
                ++cnt, victim->pIndexData->vnum, victim->short_descr, victim->in_room->vnum, victim->in_room->name);
        }
    }

    if (!found) {
        send_to_char("Nope, couldn't find it.\n\r", ch);
        return;
    }

    return;
}

void
do_reboo(CHAR_DATA *ch, char *argument)
{
    send_to_char("If you want to REBOOT, spell it out.\n\r", ch);
    return;
}

void
do_reboot(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    extern bool         merc_down;
    extern int          saving_area;

    build_save_flush();

    if (saving_area) {
        send_to_char("Please wait until area saving complete.\n", ch);
        return;
    }

    sprintf(buf, "Reboot by %s.", ch->name);
    do_echo(ch, buf);
    merc_down = TRUE;
    return;
}

void
do_shutdow(CHAR_DATA *ch, char *argument)
{
    send_to_char("If you want to SHUTDOWN, spell it out.\n\r", ch);
    return;
}

void
do_shutdown(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    extern bool         merc_down;
    extern int          saving_area;

    build_save_flush();

    if (saving_area) {
        send_to_char("Please wait until area saving complete.\n", ch);
        return;
    }

    sprintf(buf, "Shutdown by %s.", ch->name);
    append_file(ch, SHUTDOWN_FILE, buf);
    strcat(buf, "\n\r");
    do_echo(ch, buf);
    merc_down = TRUE;
    return;
}

void
do_snoop(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA    *d;
    CHAR_DATA          *victim;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
        send_to_char("Snoop whom?\n\r", ch);
        return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (victim->desc == NULL) {
        send_to_char("No descriptor to snoop.\n\r", ch);
        return;
    }

    if (victim == ch) {
        send_to_char("Cancelling all snoops.\n\r", ch);
        for (d = first_desc; d != NULL; d = d->next) {
            if (d->snoop_by == ch->desc)
                d->snoop_by = NULL;
        }
        return;
    }

    if (victim->desc->snoop_by != NULL) {
        send_to_char("Busy already.\n\r", ch);
        return;
    }

    if (get_trust(victim) >= get_trust(ch)) {
        send_to_char("You failed.\n\r", ch);
        return;
    }

    if (ch->desc != NULL) {
        for (d = ch->desc->snoop_by; d != NULL; d = d->snoop_by) {
            if (d->character == victim || d->original == victim) {
                send_to_char("No snoop loops.\n\r", ch);
                return;
            }
        }
    }

    victim->desc->snoop_by = ch->desc;
    send_to_char("Ok.\n\r", ch);
    return;
}

void
do_switch(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
        send_to_char("Switch into whom?\n\r", ch);
        return;
    }

    if (ch->desc == NULL)
        return;

    if (ch->desc->original != NULL) {
        send_to_char("You are already switched.\n\r", ch);
        return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (victim == ch) {
        send_to_char("Ok.\n\r", ch);
        return;
    }

    /*
     * Pointed out by Da Pub (What Mud)
     */
    if (!IS_NPC(victim)) {
        send_to_char("You cannot switch into a player!\n\r", ch);
        return;
    }

    if (victim->desc != NULL) {
        send_to_char("Character in use.\n\r", ch);
        return;
    }

    ch->desc->character = victim;
    ch->desc->original = ch;
    victim->desc = ch->desc;
    ch->desc = NULL;
    send_to_char("Ok.\n\r", victim);
    return;
}

void
do_return(CHAR_DATA *ch, char *argument)
{
    if (ch->desc == NULL)
        return;

    if (ch->desc->original == NULL) {
        send_to_char("You aren't switched.\n\r", ch);
        return;
    }

    send_to_char("You return to your original body.\n\r", ch);
    ch->desc->character = ch->desc->original;
    ch->desc->original = NULL;
    ch->desc->character->desc = ch->desc;
    ch->desc = NULL;
    return;
}

void
do_mload(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA     *pMobIndex;
    CHAR_DATA          *victim;

    one_argument(argument, arg);

    if (arg[0] == '\0' || !is_number(arg)) {
        send_to_char("Syntax: mload <vnum>.\n\r", ch);
        return;
    }

    if ((pMobIndex = get_mob_index(atoi(arg))) == NULL) {
        send_to_char("No mob has that vnum.\n\r", ch);
        return;
    }

    victim = create_mobile(pMobIndex);
    char_to_room(victim, ch->in_room);
    act("$n has created $N!", ch, NULL, victim, TO_ROOM);
    send_to_char("Ok.\n\r", ch);
    return;
}

void
do_oload(CHAR_DATA *ch, char *argument)
{
    char                arg1[MAX_INPUT_LENGTH];
    char                arg2[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA     *pObjIndex;
    OBJ_DATA           *obj;
    int                 level;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0' || !is_number(arg1)) {
        send_to_char("Syntax: oload <vnum> <level>.\n\r", ch);
        return;
    }

    if (arg2[0] == '\0') {
        level = get_trust(ch);
    }
    else {
        /*
         * New feature from Alander.
         */
        if (!is_number(arg2)) {
            send_to_char("Syntax: oload <vnum> <level>.\n\r", ch);
            return;
        }
        level = atoi(arg2);
        if (level < 0 || level > get_trust(ch)) {
            send_to_char("Limited to your trust level.\n\r", ch);
            return;
        }
    }

    if ((pObjIndex = get_obj_index(atoi(arg1))) == NULL) {
        send_to_char("No object has that vnum.\n\r", ch);
        return;
    }

    if (IS_SET(pObjIndex->extra_flags, ITEM_CLAN_EQ) && (ch->level != MAX_LEVEL)) {
        send_to_char("Only Creators can OLOAD clan equipment.\n\r", ch);
        return;
    }

    obj = create_object(pObjIndex, level);
    if (CAN_WEAR(obj, ITEM_TAKE)) {
        act("$n @@N@@ggestures majestically, and @@N$p @@N@@gappears with a crash of @@WTHUNDER!!@@N", ch, obj, NULL, TO_ROOM);
        obj_to_char(obj, ch);
    }
    else {
        act("$n @@N@@ggestures majestically, and @@N$p @@N@@gappears with a crash of @@WTHUNDER!!@@N", ch, obj, NULL, TO_ROOM);
        obj_to_room(obj, ch->in_room);
    }

    send_to_char("Ok.\n\r", ch);
    return;
}

void
do_purge(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim;
    OBJ_DATA           *obj;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
        /* 'purge' */
        CHAR_DATA          *vnext;
        OBJ_DATA           *obj_next;

        for (victim = ch->in_room->first_person; victim != NULL; victim = vnext) {
            vnext = victim->next_in_room;
            if (IS_NPC(victim) && victim != ch)
                extract_char(victim, TRUE);
        }

        for (obj = ch->in_room->first_content; obj != NULL; obj = obj_next) {
            obj_next = obj->next_in_room;

            /* don't purge player corpses. avoids accidents! if for some reason you need to purge
             * a players corpse, set its timer to 1 or something */
            if (obj->item_type != ITEM_CORPSE_PC)
                extract_obj(obj);
        }

        act("$n cleanses the room with Holy fire!", ch, NULL, NULL, TO_ROOM);
        send_to_char("Your burst of Holy fire cleanses the room!\n\r", ch);
        return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (!IS_NPC(victim)) {
        send_to_char("Not on PC's.\n\r", ch);
        return;
    }

    act("$n obliterates $N with Holy fire!", ch, NULL, victim, TO_NOTVICT);
    act("You obliterate $N with Holy fire!", ch, NULL, victim, TO_CHAR);
    extract_char(victim, TRUE);
    return;
}

void
do_advance(CHAR_DATA *ch, char *argument)
{
    send_to_char("Use setclass instead.  Advance no longer works here.\n\r", ch);
    return;
}

void
do_trust(CHAR_DATA *ch, char *argument)
{
    char                arg1[MAX_INPUT_LENGTH];
    char                arg2[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim;
    int                 level;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0' || arg2[0] == '\0' || !is_number(arg2)) {
        send_to_char("Syntax: trust <char> <level>.\n\r", ch);
        return;
    }

    if ((victim = get_char_room(ch, arg1)) == NULL) {
        send_to_char("That player is not here.\n\r", ch);
        return;
    }

    if ((level = atoi(arg2)) < 0 || level > MAX_LEVEL) {
        sendf(ch, "Level must be 0 (reset) or 1 to %d.\n\r", MAX_LEVEL);
        return;
    }

    if (level > get_trust(ch)) {
        send_to_char("Limited to your trust.\n\r", ch);
        return;
    }

    victim->trust = level;
    return;
}

void cool_player_objs(CHAR_DATA *imm, CHAR_DATA *ch)
{
    OBJ_DATA *obj;

    if (!imm || !ch || IS_NPC(ch))
        return;

    for (obj = ch->first_carry; obj != NULL; obj = obj->next_in_carry_list)
        if (IS_SET(obj->item_apply, ITEM_APPLY_HEATED)) {
            act("@@N@@g$n@@N@@g cools down @@N$p@@N@@g!@@N", imm, obj, ch, TO_VICT);
            act("@@N@@gYou cool down @@N@@g$N@@N@@g's @@N$p@@N@@g!@@N", imm, obj, ch, TO_CHAR);
            REMOVE_BIT(obj->item_apply, ITEM_APPLY_HEATED);
        }

    return;
}

void
do_restore(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim;

    one_argument(argument, arg);
    if (arg[0] == '\0') {
        send_to_char("Restore whom?\n\r", ch);
        return;
    }

    if (!str_cmp(arg, "all")) {
        /* then loop through all players and restore them */
        CHAR_DATA          *vch;
        CHAR_DATA          *vch_next;

        for (vch = first_char; vch != NULL; vch = vch_next) {
            vch_next = vch->next;
            if (!IS_NPC(vch)) {
                if (IS_IMMORTAL(vch) && (vch != ch)) {
                    act("@@N@@gEveryone has been restored by $n@@N@@g.@@N", ch, NULL, vch, TO_VICT);
                }
                else if (!is_in_duel(vch, DUEL_STAGE_GO)) {
                    vch->hit = vch->max_hit;
                    vch->mana = vch->max_mana;
                    vch->move = vch->max_move;
                    update_pos(vch);
                    act("@@N@@g$n@@N@@g kindly restores you.@@N", ch, NULL, vch, TO_VICT);
                    cool_player_objs(ch, vch);
                }
            }
        }

        send_to_char("@@N@@gEveryone has been restored.@@N\n\r", ch);
        return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL) {
        send_to_char("They aren't here!\n\r", ch);
        return;
    }

    victim->hit = victim->max_hit;
    victim->mana = victim->max_mana;
    victim->move = victim->max_move;
    update_pos(victim);
    act("@@N@@g$n@@N@@g kindly restores you.@@N", ch, NULL, victim, TO_VICT);
    cool_player_objs(ch, victim);
    send_to_char("Ok.\n\r", ch);
    return;
}

void
do_freeze(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    char                buf[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
        send_to_char("Freeze whom?\n\r", ch);
        return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("Not on NPC's.\n\r", ch);
        return;
    }

    if (get_trust(victim) >= get_trust(ch)) {
        send_to_char("You failed.\n\r", ch);
        return;
    }

    if (IS_SET(victim->act, PLR_FREEZE)) {
        REMOVE_BIT(victim->act, PLR_FREEZE);
        send_to_char("You can play again.\n\r", victim);
        send_to_char("FREEZE removed.\n\r", ch);
    }
    else {
        SET_BIT(victim->act, PLR_FREEZE);
        sendf(victim, "You can't do anything!\n\rYou have been FROZEN by %s!!\n\r", ch->short_descr);
        send_to_char("Freeze set.\n\r", ch);

        sprintf(buf, "%s has been FROZEN by %s.\n\r", victim->name, ch->name);
        notify(buf, ch->level + 1);
    }

    save_char_obj(victim);

    return;
}

void
do_log(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim;
    bool                found = FALSE;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
        send_to_char("Log whom?\n\r", ch);
        return;
    }

    if (!str_cmp(arg, "all")) {
        if (fLogAll) {
            fLogAll = FALSE;
            send_to_char("Log ALL off.\n\r", ch);
        }
        else {
            fLogAll = TRUE;
            send_to_char("Log ALL on.\n\r", ch);
        }
        return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL) {
        send_to_char("They're not here.\n\r", ch);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("Not on NPC's.\n\r", ch);
        return;
    }

    /*
     * No level check, gods can log anyone.
     */
    if (IS_SET(victim->act, PLR_LOG)) {
        REMOVE_BIT(victim->act, PLR_LOG);
        send_to_char("LOG removed.\n\r", ch);
    }
    else {
        SET_BIT(victim->act, PLR_LOG);
        send_to_char("LOG set.\n\r", ch);
    }
    if (found) {
        do_quit(victim, "NOSAVECHECK");
    }
    return;
}

void
do_noemote(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
        send_to_char("Noemote whom?\n\r", ch);
        return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("Not on NPC's.\n\r", ch);
        return;
    }

    if (get_trust(victim) >= get_trust(ch)) {
        send_to_char("You failed.\n\r", ch);
        return;
    }

    if (IS_SET(victim->act, PLR_NO_EMOTE)) {
        REMOVE_BIT(victim->act, PLR_NO_EMOTE);
        send_to_char("You can emote again.\n\r", victim);
        send_to_char("NO_EMOTE removed.\n\r", ch);
    }
    else {
        SET_BIT(victim->act, PLR_NO_EMOTE);
        send_to_char("Your ability to emote has been removed!\n\r", victim);
        send_to_char("NO_EMOTE set.\n\r", ch);
    }

    return;
}

void
do_notell(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
        send_to_char("Notell whom?\n\r", ch);
        return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("Not on NPC's.\n\r", ch);
        return;
    }

    if (get_trust(victim) >= get_trust(ch)) {
        send_to_char("You failed.\n\r", ch);
        return;
    }

    if (IS_SET(victim->act, PLR_NO_TELL)) {
        REMOVE_BIT(victim->act, PLR_NO_TELL);
        send_to_char("You can tell again.\n\r", victim);
        send_to_char("NO_TELL removed.\n\r", ch);
    }
    else {
        SET_BIT(victim->act, PLR_NO_TELL);
        send_to_char("You now can not use the tell command!\n\r", victim);
        send_to_char("NO_TELL set.\n\r", ch);
    }

    return;
}

void
do_silence(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
        send_to_char("Silence whom?\n\r", ch);
        return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("Not on NPC's.\n\r", ch);
        return;
    }

    if (get_trust(victim) >= get_trust(ch)) {
        send_to_char("You failed.\n\r", ch);
        return;
    }

    if (IS_SET(victim->act, PLR_SILENCE)) {
        REMOVE_BIT(victim->act, PLR_SILENCE);
        send_to_char("You can use channels again.\n\r", victim);
        send_to_char("SILENCE removed.\n\r", ch);
    }
    else {
        SET_BIT(victim->act, PLR_SILENCE);
        send_to_char("You can't use channels!\n\r", victim);
        send_to_char("SILENCE set.\n\r", ch);
    }

    return;
}

void
do_nopray(CHAR_DATA *ch, char *argument)
{
    /* Remove victim's ability to use pray channel.. -S- */

    char                arg[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
        send_to_char("NoPray whom?\n\r", ch);
        return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("Not on NPC's.\n\r", ch);
        return;
    }

    if (get_trust(victim) >= get_trust(ch)) {
        send_to_char("You failed.\n\r", ch);
        return;
    }

    if (IS_SET(victim->act, PLR_NO_PRAY)) {
        REMOVE_BIT(victim->act, PLR_NO_PRAY);
        send_to_char("You can use 'PRAY' again.\n\r", victim);
        send_to_char("NOPRAY removed.\n\r", ch);
    }
    else {
        SET_BIT(victim->act, PLR_NO_PRAY);
        send_to_char("You can't use 'PRAY'!\n\r", victim);
        send_to_char("NOPRAY set.\n\r", ch);
    }

    return;
}

void
do_peace(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA          *rch;

    for (rch = ch->in_room->first_person; rch != NULL; rch = rch->next_in_room) {
        if (rch->fighting != NULL)
            stop_fighting(rch, TRUE);
    }

    send_to_char("Ok.\n\r", ch);
    return;
}

void
do_ban(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    char                arg[MAX_INPUT_LENGTH];
    char                arg2[MSL];
    char                buf2[MSL];

    BAN_DATA           *pban;

    buf[0] = '\0';
    buf2[0] = '\0';

    if (IS_NPC(ch))
        return;

    argument = one_argument(argument, arg);
    one_argument(argument, arg2);

    if (arg[0] == '\0') {
        strcpy(buf, "Banned sites:\n\r");
        for (pban = first_ban; pban != NULL; pban = pban->next) {
            strcat(buf, pban->name);
            sprintf(buf2, (pban->newbie ? " Newbies" : " All"));
            safe_strcat(MSL, buf, buf2);
            sprintf(buf2, "  Banned by: %s", pban->banned_by);
            safe_strcat(MSL, buf, buf2);
            strcat(buf, "\n\r");
        }
        send_to_char(buf, ch);
        return;
    }

    for (pban = first_ban; pban != NULL; pban = pban->next) {
        if (!str_cmp(arg, pban->name)) {
            send_to_char("That site is already banned!\n\r", ch);
            return;
        }
    }

    GET_FREE(pban, ban_free);
    if (!str_cmp(arg2, "newbie"))
        pban->newbie = TRUE;
    else
        pban->newbie = FALSE;

    pban->name = str_dup(arg);
    pban->banned_by = str_dup(ch->name);
    LINK(pban, first_ban, last_ban, next, prev);
    save_bans();
    send_to_char("Ok.\n\r", ch);
    return;
}

void
do_allow(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    BAN_DATA           *curr;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
        send_to_char("Remove which site from the ban list?\n\r", ch);
        return;
    }

    for (curr = first_ban; curr != NULL; curr = curr->next) {
        if (!str_cmp(arg, curr->name)) {
            UNLINK(curr, first_ban, last_ban, next, prev);
            free_string(curr->name);
            free_string(curr->banned_by);
            PUT_FREE(curr, ban_free);
            send_to_char("Ok.\n\r", ch);
            save_bans();
            return;
        }
    }

    send_to_char("Site is not banned.\n\r", ch);
    return;
}

void
do_wizlock(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_INPUT_LENGTH];
    extern bool         wizlock;

    wizlock = !wizlock;

    if (wizlock) {
        send_to_char("Game wizlocked.\n\r", ch);
        sprintf(buf, "%s wizlocks ACK! Mud.\n\r", ch->short_descr);
    }
    else {
        send_to_char("Game un-wizlocked.\n\r", ch);
        sprintf(buf, "%s un-wizlocks ACK! Mud.\n\r", ch->short_descr);
    }

    notify(buf, ch->level);
    save_mudsets();

    return;
}

void
do_nopk(CHAR_DATA *ch, char *argument)
{
    extern bool         nopk;

    nopk = !nopk;

    if (nopk)
        send_to_char("Player killing has been disabled (wanted & pkoks have no protection).\n\r", ch);
    else
        send_to_char("Player killing is allowed.\n\r", ch);

    save_mudsets();
    return;
}

void
do_dblxp(CHAR_DATA *ch, char *argument)
{
    extern bool         dbl_xp;

    dbl_xp = !dbl_xp;

    if (dbl_xp) {
        send_to_char("Experience Doubling is Enabled.\n\r", ch);
        info("@@eDouble Experience is now in effect!@@N\n\r", 1);
    }
    else {
        send_to_char("Experience Doubling is no longer in affect.\n\r", ch);
        info("@@lDouble Experience is no longer in effect!@@N\n\r", 1);
    }

    save_mudsets();
    return;
}

void
do_slookup(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    int                 sn;

    one_argument(argument, arg);
    if (arg[0] == '\0') {
        send_to_char("Slookup what?\n\r", ch);
        return;
    }

    if (!str_cmp(arg, "all")) {
        for (sn = 0; sn < MAX_SKILL; sn++) {
            if (skill_table[sn].name == NULL)
                break;
            sendf(ch, "Sn: %4d Slot: %4d Skill/spell: '%s'\n\r", sn, skill_table[sn].slot, skill_table[sn].name);
        }
    }
    else {
        if ((sn = skill_lookup(arg)) < 0) {
            send_to_char("No such skill or spell.\n\r", ch);
            return;
        }

        sendf(ch, "Sn: %4d Slot: %4d Skill/spell: '%s'\n\r", sn, skill_table[sn].slot, skill_table[sn].name);
    }

    return;
}

void
do_sset(CHAR_DATA *ch, char *argument)
{
    char                arg1[MAX_INPUT_LENGTH];
    char                arg2[MAX_INPUT_LENGTH];
    char                arg3[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim;
    int                 value;
    int                 sn;
    bool                fAll;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);

    if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0') {
        send_to_char("Syntax: sset <victim> <skill> <value>\n\r", ch);
        send_to_char("or:     sset <victim> all     <value>\n\r", ch);
        send_to_char("Skill being any skill or spell.\n\r", ch);
        return;
    }

    if ((victim = get_char_world(ch, arg1)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("Not on NPC's.\n\r", ch);
        return;
    }

    fAll = !str_cmp(arg2, "all");

    if (fAll && ch->level != 90) {
        send_to_char("Only Creators may SSET all.\n\r", ch);
        return;
    }

    sn = 0;
    if (!fAll && (sn = skill_lookup(arg2)) < 0) {
        send_to_char("No such skill or spell.\n\r", ch);
        return;
    }

    /*
     * Snarf the value.
     */
    if (!is_number(arg3)) {
        send_to_char("Value must be numeric.\n\r", ch);
        return;
    }

    value = atoi(arg3);
    if (value < 0 || value > 101) {
        send_to_char("Value range is 0 to 101.\n\r", ch);
        return;
    }

    if (fAll) {
        for (sn = 0; sn < MAX_SKILL; sn++) {
            if (skill_table[sn].name != NULL)
                victim->pcdata->learned[sn] = value;
        }
    }
    else {
        victim->pcdata->learned[sn] = value;
    }

    return;
}

void
do_mset(CHAR_DATA *ch, char *argument)
{
    char                arg1[MAX_INPUT_LENGTH];
    char                arg2[MAX_INPUT_LENGTH];
    char                arg3[MAX_INPUT_LENGTH];
    char                buf[MAX_STRING_LENGTH];
    CHAR_DATA          *victim;
    int                 value, max;

    smash_tilde(argument);
    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (str_cmp(arg1, "order"))
        strcpy(arg3, argument);
    else
        argument = one_argument(argument, arg3);

    if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0') {
        send_to_char("\n\r", ch);
        send_to_char("        @@gmset <@@atarget@@g> <@@afield@@g> <@@avalue@@g>@@N\n\r", ch);
        send_to_char("\n\r", ch);
        send_to_char("@@c+----------------@@dNormal Fields@@c----------------+@@N\n\r", ch);
        send_to_char("@@c|                                             @@c|@@N\n\r", ch);
        send_to_char("@@c| @@gstr, int, wis, dex, con, hp, mana, move     @@c|@@N\n\r", ch);
        send_to_char("@@c| @@gmaxstr, maxint, maxwis, maxdex, maxcon      @@c|\n\r", ch);
        send_to_char("@@c| @@gpractice, align, thirst, drunk, full,       @@c|@@N\n\r", ch);
        send_to_char("@@c| @@gflags, aff, recall, order, title, entry,    @@c|@@N\n\r", ch);
        send_to_char("@@c| @@gexit, rulerrank, short, long, name, sex     @@c|@@N\n\r", ch);
        send_to_char("@@c| @@gadept, gold, pkills, mkills, mkilled, level @@c|@@N\n\r", ch);
        send_to_char("@@c| @@gpkilled, unpkills, sentence, weight, class  @@c|@@N\n\r", ch);
        send_to_char("@@c| @@ghunt, race, keep, kdon, balance             @@c|@@N\n\r", ch);
        send_to_char("@@c| @@ghpgain, managain, movegain                  @@c|@@N\n\r", ch);
        send_to_char("@@c+---------------------------------------------+@@N\n\r", ch);
        return;
    }

    if ((victim = get_char_world(ch, arg1)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = is_number(arg3) ? atoi(arg3) : -1;

    /*
     * Set something.
     */

    if (!str_cmp(arg2, "order")) {

        int                 cnt;
        int                 class[MAX_CLASS];
        int                 parity[MAX_CLASS];
        int                 index[MAX_CLASS];
        int                 foo;
        bool                ok = TRUE;
        char                arg[MAX_STRING_LENGTH];

        if (IS_NPC(victim)) {
            send_to_char("Not on NPCs!\n\r", ch);
            return;
        }

        if (get_trust(ch) < 89) {
            send_to_char("Only a Supreme or above may use this option.\n\r", ch);
            return;
        }
        for (cnt = 0; cnt < MAX_CLASS; cnt++)
            parity[cnt] = -1;

        for (cnt = 0; cnt < MAX_CLASS; cnt++) {
            argument = one_argument(argument, arg);
            if (arg[0] == '\0') {
                ok = FALSE;
                break;
            }
            for (foo = 0; foo < MAX_CLASS; foo++)
                if (!str_cmp(arg, class_table[foo].who_name)) {
                    class[cnt] = foo;
                    index[foo] = (cnt);
                    parity[foo] = 1;
                    break;
                }
            if (foo == MAX_CLASS) {
                ok = FALSE;
                break;
            }
        }

        for (cnt = 0; cnt < MAX_CLASS; cnt++)
            if (parity[cnt] == -1)
                ok = FALSE;

        if (!ok) {
            send_to_char("Must be 5 3-letter abbrev for different classes.\n\r", ch);
            return;
        }

        /* Copy classes to pcdata */
        for (cnt = 0; cnt < MAX_CLASS; cnt++) {
            victim->pcdata->order[cnt] = class[cnt];
            victim->pcdata->index[cnt] = index[cnt];
        }

        send_to_char("Your classes have been re-ordered.\n\r", victim);
        send_to_char("Done.\n\r", ch);
        return;
    }

    if (!str_cmp(arg2, "sentence")) {
        if (ch->level < MAX_LEVEL) {
            send_to_char("This option only available to Creators.\n\r", ch);
            return;
        }

        if (IS_NPC(victim)) {
            send_to_char("Not on NPCs!\n\r", ch);
            return;
        }
        victim->sentence = value;
        return;
    }

    if (!str_cmp(arg2, "weight")) {
        if (IS_NPC(victim)) {
            send_to_char("Not on NPCs!\n\r", ch);
            return;
        }
        victim->carry_weight = value;
        return;
    }

    if (!str_cmp(arg2, "str")) {
        if (IS_NPC(victim)) {
            send_to_char("Not on NPC's.\n\r", ch);
            return;
        }

        max = victim->pcdata->max_str;

        if (value < 3 || value > max) {
            sendf(ch, "Strength range is 3 to %d.\n\r", max);
            return;
        }

        victim->pcdata->perm_str = value;
        return;
    }

    if (!str_cmp(arg2, "int")) {
        if (IS_NPC(victim)) {
            send_to_char("Not on NPC's.\n\r", ch);
            return;
        }

        max = victim->pcdata->max_int;

        if (value < 3 || value > max) {
            sendf(ch, "Intelligence range is 3 to %d.\n\r", max);
            return;
        }

        victim->pcdata->perm_int = value;
        return;
    }

    if (!str_cmp(arg2, "wis")) {
        if (IS_NPC(victim)) {
            send_to_char("Not on NPC's.\n\r", ch);
            return;
        }

        max = victim->pcdata->max_wis;

        if (value < 3 || value > max) {
            sendf(ch, "Wisdom range is 3 to %d.\n\r", max);
            return;
        }

        victim->pcdata->perm_wis = value;
        return;
    }

    if (!str_cmp(arg2, "dex")) {
        if (IS_NPC(victim)) {
            send_to_char("Not on NPC's.\n\r", ch);
            return;
        }

        max = victim->pcdata->max_dex;

        if (value < 3 || value > max) {
            sendf(ch, "Dexterity range is 3 to %d.\n\r", max);
            return;
        }

        victim->pcdata->perm_dex = value;
        return;
    }

    if (!str_cmp(arg2, "con")) {
        if (IS_NPC(victim)) {
            send_to_char("Not on NPC's.\n\r", ch);
            return;
        }

        max = victim->pcdata->max_con;

        if (value < 3 || value > max) {
            sendf(ch, "Constitution range is 3 to %d.\n\r", max);
            return;
        }

        victim->pcdata->perm_con = value;
        return;
    }

    if (!str_cmp(arg2, "maxstr")) {
        if (IS_NPC(victim)) {
            send_to_char("Not on NPC's.\n\r", ch);
            return;
        }

        if (value < 0 || value > 25) {
            send_to_char("Strength range is 0 to 25.\n\r", ch);
            return;
        }

        victim->pcdata->max_str = value;
        return;
    }

    if (!str_cmp(arg2, "maxint")) {
        if (IS_NPC(victim)) {
            send_to_char("Not on NPC's.\n\r", ch);
            return;
        }

        if (value < 0 || value > 25) {
            send_to_char("Intelligence range is 0 to 25.\n\r", ch);
            return;
        }

        victim->pcdata->max_int = value;
        return;
    }

    if (!str_cmp(arg2, "maxwis")) {
        if (IS_NPC(victim)) {
            send_to_char("Not on NPC's.\n\r", ch);
            return;
        }

        if (value < 0 || value > 25) {
            send_to_char("Wisdom range is 0 to 25.\n\r", ch);
            return;
        }

        victim->pcdata->max_wis = value;
        return;
    }

    if (!str_cmp(arg2, "maxdex")) {
        if (IS_NPC(victim)) {
            send_to_char("Not on NPC's.\n\r", ch);
            return;
        }

        if (value < 0 || value > 25) {
            send_to_char("Dexterity range is 0 to 25.\n\r", ch);
            return;
        }

        victim->pcdata->max_dex = value;
        return;
    }

    if (!str_cmp(arg2, "maxcon")) {
        if (IS_NPC(victim)) {
            send_to_char("Not on NPC's.\n\r", ch);
            return;
        }

        if (value < 0 || value > 25) {
            send_to_char("Constitution range is 0 to 25.\n\r", ch);
            return;
        }

        victim->pcdata->max_con = value;
        return;
    }

    if (!str_cmp(arg2, "adept")) {
        if (IS_NPC(victim)) {
            send_to_char("Not on NPC's.\n\r", ch);
            return;
        }

        if (value < 0 || value > 20) {
            send_to_char("Adept range is 0 to 20.\n\r", ch);
            return;
        }

        victim->adept_level = value;
        return;
    }

    if (!str_cmp(arg2, "sex")) {
        RULER_DATA *ruler;

        if (value < 0 || value > 2) {
            send_to_char("Sex range is 0 to 2.\n\r", ch);
            return;
        }
        victim->sex = value;
        victim->login_sex = value;

        if (victim->adept_level == 20 && (ruler = get_ruler(victim))) {
            if (is_name(ruler->rank, "@@WLord@@N @@WLady@@N @@WMonitor@@N")) {
                char *rank;

                if (value == SEX_NEUTRAL)
                    rank = "@@WMonitor@@N";
                else if (value == SEX_MALE)
                    rank = "@@WLord@@N";
                else
                    rank = "@@WLady@@N";

                free_string(ruler->rank);
                ruler->rank = str_dup(rank);
                save_rulers();
            }
        }

        return;
    }

    if (!str_cmp(arg2, "class")) {
        if (value < 0 || value >= MAX_CLASS) {
            sendf(ch, "Class range is 0 to %d.\n", MAX_CLASS - 1);
            return;
        }

        victim->class = value;
        return;
    }

    if (!str_cmp(arg2, "race")) {
        int                 cnt = 0;

        for (cnt = 0; cnt < MAX_RACE; cnt++) {
            if (!str_cmp(arg3, race_table[cnt].race_name))
                break;
        }

        if (cnt < MAX_RACE)
            victim->race = cnt;
        else
            send_to_char("Unknown race.\n\r", ch);

        return;
    }

    if (!str_cmp(arg2, "rulerrank")) {
        RULER_DATA *ruler = NULL;

        if (IS_NPC(victim)) {
            send_to_char("Not on Npcs.\n\r", ch);
            return;
        }

        if ((ruler = get_ruler(victim))) {
            free_string(ruler->rank);
            ruler->rank = str_dup(arg3);

            send_to_char("Done.\n\r", ch);
        }
        else
            send_to_char("Unable to find their ruler entry. Are you sure they're a Realm Lord?\n\r", ch);

        save_rulers();
        return;
    }

    if (!str_cmp(arg2, "hunt")) {

        CHAR_DATA          *hunted = 0;

        if (ch->level < 89) {
            send_to_char("Currently restricted to reduce abuses.\n\r", ch);
            return;
        }

        if (!IS_NPC(victim)) {
            send_to_char("Not on PC's.\n\r", ch);
            return;
        }

        if (str_cmp(arg3, ".")) {
            if ((hunted = get_char_world(victim, arg3)) == NULL) {
                send_to_char("Mob couldn't locate the victim to hunt.\n\r", ch);
                return;
            }
            if (!set_hunt(victim, NULL, hunted, NULL, 0, 0))
                /*           if ( ! make_hunt(victim,hunted) ) */
                send_to_char("Mob could not hunt victim.\n\r", ch);
        }
        else {
            victim->hunting = NULL;
            victim->hunt_obj = NULL;
            victim->hunt_for = NULL;
            if (victim->searching) {
                free_string(victim->searching);
                victim->searching = NULL;
            }
            victim->hunt_flags = victim->pIndexData->hunt_flags;
        }

        return;
    }

    if (!str_cmp(arg2, "level")) {
        if (!IS_NPC(victim)) {
            send_to_char("Not on PC's.\n\r", ch);
            return;
        }

        if (value < 0 || value > 100) {
            send_to_char("Level range is 0 to 100.\n\r", ch);
            return;
        }
        victim->level = value;
        return;
    }
    if (!str_cmp(arg2, "timer")) {
        if (!IS_NPC(victim)) {
            send_to_char("Not on PC's.\n\r", ch);
            return;
        }

        victim->extract_timer = value;
        return;
    }

    if (!str_cmp(arg2, "gold")) {
        victim->gold = value;
        return;
    }

    if (!str_cmp(arg2, "balance")) {
        victim->balance = value;
        return;
    }

    if (!str_cmp(arg2, "hp")) {
        if (value < 1) {
            send_to_char("Hp minimum is 1.\n\r", ch);
            return;
        }
        victim->max_hit = value;

        return;
    }
    if (!str_cmp(arg2, "curhp")) {
        if (value < 1) {
            send_to_char("CurHp minimum is 1.\n\r", ch);
            return;
        }
        victim->hit = value;

        return;
    }

    if (!str_cmp(arg2, "hpgain")) {
        if (IS_NPC(victim))
            return;

        if (value < 1) {
            send_to_char("Hpgain minimum is 1.\n\r", ch);
            return;
        }
        victim->pcdata->hp_from_gain = value;

        return;
    }

    if (!str_cmp(arg2, "mana")) {
        if (value < 0) {
            send_to_char("Mana minimum is 0.\n\r", ch);
            return;
        }
        victim->max_mana = value;
        return;
    }

    if (!str_cmp(arg2, "managain")) {
        if (IS_NPC(victim))
            return;

        if (value < 1) {
            send_to_char("Managain minimum is 1.\n\r", ch);
            return;
        }
        victim->pcdata->mana_from_gain = value;

        return;
    }

    if (!str_cmp(arg2, "energy")) {
        if (value < 0) {
            send_to_char("Energy minimum is 0.\n\r", ch);
            return;
        }
        victim->energy = value;
        return;
    }

    if (!str_cmp(arg2, "maxenergy")) {
        if (IS_NPC(victim))
            return;

        if (value < 1) {
            send_to_char("Maxenergy minimum is 1.\n\r", ch);
            return;
        }
        victim->max_energy = value;

        return;
    }


    if (!str_cmp(arg2, "move")) {
        if (value < 0) {
            send_to_char("Move minimum is 0.\n\r", ch);
            return;
        }
        victim->max_move = value;
        return;
    }

    if (!str_cmp(arg2, "movegain")) {
        if (IS_NPC(victim))
            return;

        if (value < 1) {
            send_to_char("Movegain minimum is 1.\n\r", ch);
            return;
        }
        victim->pcdata->move_from_gain = value;

        return;
    }

    if (!str_cmp(arg2, "practice")) {
        if (value < 0) {
            send_to_char("Practice minimum is 0.\n\r", ch);
            return;
        }
        victim->practice = value;
        return;
    }

    if (!str_cmp(arg2, "align")) {
        if (value < -1000 || value > 1000) {
            send_to_char("Alignment range is -1000 to 1000.\n\r", ch);
            return;
        }
        victim->alignment = value;
        return;
    }

    if (!str_cmp(arg2, "thirst")) {
        if (IS_NPC(victim)) {
            send_to_char("Not on NPCs.\n\r", ch);
            return;
        }

        if (value < 0 || value > 100) {
            send_to_char("Thirst range is 0 to 100.\n\r", ch);
            return;
        }

        victim->pcdata->condition[COND_THIRST] = value;
        return;
    }

    if (!str_cmp(arg2, "drunk")) {
        if (IS_NPC(victim)) {
            send_to_char("Not on NPC's.\n\r", ch);
            return;
        }

        if (value < 0 || value > 100) {
            send_to_char("Drunk range is 0 to 100.\n\r", ch);
            return;
        }

        victim->pcdata->condition[COND_DRUNK] = value;
        return;
    }

    if (!str_cmp(arg2, "full")) {
        if (IS_NPC(victim)) {
            send_to_char("Not on NPC's.\n\r", ch);
            return;
        }

        if (value < 0 || value > 100) {
            send_to_char("Full range is 0 to 100.\n\r", ch);
            return;
        }

        victim->pcdata->condition[COND_FULL] = value;
        return;
    }

    if (!str_cmp(arg2, "flags")) {
        int                 neg = 0;
        char               *lookupstr = arg3;

        if (get_trust(ch) < MAX_LEVEL - 1) {
            send_to_char("Only supreme or creator level immortals may use this.\n\r", ch);
            return;
        }

        if (lookupstr[0] == '-') {
            neg = 1;
            lookupstr++;
        }
        if (lookupstr[0] == '+')
            lookupstr++;

        value = table_lookup(tab_player_flags, lookupstr);
        if (value < 1) {
            sprintf(buf, "Valid player flags are :\n\r");
            table_printout(tab_player_flags, buf + strlen(buf));
            send_to_char(buf, ch);
            return;
        }

        if (neg)
            REMOVE_BIT(victim->pcdata->pflags, value);
        else
            SET_BIT(victim->pcdata->pflags, value);

        if (victim->pcdata->clan > 0)
            update_cinfo(victim, FALSE);

        return;
    }
    if (!str_cmp(arg2, "aff")) {
        if (IS_NPC(victim)) {
            send_to_char("Not on NPCs.\n\r", ch);
            return;
        }

        victim->affected_by = 0;
        return;
    }

    /* whois changing shit -dave */
    if (!str_cmp(arg2, "mkills")) {
        if (IS_NPC(victim)) {
            send_to_char("Not on Npcs.\n\r", ch);
            return;
        }

        if (get_trust(ch) < MAX_LEVEL) {
            send_to_char("Only creators may use this feature.\n\r", ch);
            return;
        }

        victim->pcdata->mkills = value;
        return;
    }
    if (!str_cmp(arg2, "mkilled")) {
        if (IS_NPC(victim)) {
            send_to_char("Not on Npcs.\n\r", ch);
            return;
        }

        if (get_trust(ch) < MAX_LEVEL) {
            send_to_char("Only creators may use this feature.\n\r", ch);
            return;
        }

        victim->pcdata->mkilled = value;
        return;
    }
    if (!str_cmp(arg2, "pkills")) {
        if (IS_NPC(victim)) {
            send_to_char("Not on Npcs.\n\r", ch);
            return;
        }

        if (get_trust(ch) < MAX_LEVEL) {
            send_to_char("Only creators may use this feature.\n\r", ch);
            return;
        }

        victim->pcdata->pkills = value;
        return;
    }
    if (!str_cmp(arg2, "unpkills")) {
        if (IS_NPC(victim)) {
            send_to_char("Not on Npcs.\n\r", ch);
            return;
        }

        if (get_trust(ch) < MAX_LEVEL) {
            send_to_char("Only creators may use this feature.\n\r", ch);
            return;
        }

        victim->pcdata->unpkills = value;
        return;
    }
    if (!str_cmp(arg2, "pkilled")) {
        if (IS_NPC(victim)) {
            send_to_char("Not on Npcs.\n\r", ch);
            return;
        }

        if (get_trust(ch) < MAX_LEVEL) {
            send_to_char("Only creators may use this feature.\n\r", ch);
            return;
        }

        victim->pcdata->pkilled = value;
        return;
    }

    if (!str_cmp(arg2, "name")) {
        if (!IS_NPC(victim) && !IS_SET(victim->pcdata->pflags, PFLAG_SPECIALNAME)) {
            send_to_char("Can only mset name on players who are flagged specialname.\n\r", ch);
            return;
        }

        free_string(victim->name);
        victim->name = str_dup(arg3);
        return;
    }

    if (!str_cmp(arg2, "short")) {
        free_string(victim->short_descr);
        sprintf(buf, "%s", arg3);
        victim->short_descr = str_dup(buf);
        return;
    }

    if (!str_cmp(arg2, "long")) {
        free_string(victim->long_descr);

        sprintf(buf, "%s\n\r", arg3);
        victim->long_descr = str_dup(buf);
        return;
    }

    if (!str_cmp(arg2, "title")) {
        if (IS_NPC(victim)) {
            send_to_char("Not on NPC's.\n\r", ch);
            return;
        }

        if (ch->level < 90) {
            send_to_char("This option only available to Creators.\n\r", ch);
            return;
        }

        set_title(victim, arg3);
        return;
    }

    if (!str_cmp(arg2, "entry")) {
        if (IS_NPC(victim)) {
            send_to_char("Not on NPC's.\n\r", ch);
            return;
        }

        /*  if ( ch->level < 90 )
           {
           send_to_char( "This option only available to Creators.\n\r", ch );
           return;
           }
         */
        free_string(victim->pcdata->room_enter);

        sprintf(buf, "%s", arg3);
        victim->pcdata->room_enter = str_dup(buf);

        return;
    }

    if (!str_cmp(arg2, "exit")) {
        if (IS_NPC(victim)) {
            send_to_char("Not on NPC's.\n\r", ch);
            return;
        }

        /*  if ( ch->level < 90 )
           {
           send_to_char( "This option only available to Creators.\n\r", ch );
           return;
           }
         */
        free_string(victim->pcdata->room_exit);

        sprintf(buf, "%s", arg3);
        victim->pcdata->room_exit = str_dup(buf);

        return;
    }

    if (!str_cmp(arg2, "keep")) {
        if (IS_NPC(victim)) {
            send_to_char("Not on NPCs.\n\r", ch);
            return;
        }
        else {
            victim->pcdata->keep_vnum = victim->in_room->vnum;
            send_to_char("Done!\n\r", ch);
            send_to_char("This is your new KEEP!!!\n\r", victim);
        }
        return;
    }

    if (!str_cmp(arg2, "kdon")) {
        if (IS_NPC(victim)) {
            send_to_char("Not on NPCs.\n\r", ch);
            return;
        }
        else {
            victim->pcdata->kdon_vnum = victim->in_room->vnum;
            send_to_char("Done!\n\r", ch);
            send_to_char("This is now your Keep-Donation room.\n\r", victim);
            do_save(victim, "");
        }
        return;
    }

    if (!str_cmp(arg2, "recall")) {
        if (IS_NPC(victim)) {
            send_to_char("Not on NPCs.\n\r", ch);
            return;
        }
        else {
            victim->pcdata->recall_vnum = victim->in_room->vnum;
            send_to_char("Done!\n\r", ch);
            send_to_char("You will now recall to....HERE!\n\r", victim);
        }
        return;
    }

    if (!str_cmp(arg2, "spec")) {
        if (!IS_NPC(victim)) {
            send_to_char("Not on PC's.\n\r", ch);
            return;
        }

        if ((victim->spec_fun = spec_lookup(arg3)) == 0) {
            send_to_char("No such spec fun.\n\r", ch);
            return;
        }

        return;
    }

    /*
     * Generate usage message.
     */
    do_mset(ch, "");
    return;
}

void
do_oset(CHAR_DATA *ch, char *argument)
{
    char                arg1[MAX_INPUT_LENGTH];
    char                arg2[MAX_INPUT_LENGTH];
    char                arg3[MAX_INPUT_LENGTH];
    char                buf[MAX_STRING_LENGTH];
    OBJ_DATA           *obj;
    int                 value;
    int                 num;
    char               *argn;

    smash_tilde(argument);
    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    strcpy(arg3, argument);

    if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0') {
        send_to_char("Syntax: oset <object> <field>  <value>\n\r", ch);
        send_to_char("or:     oset <object> <string> <value>\n\r", ch);
        send_to_char("\n\r", ch);
        send_to_char("Field being one of:\n\r", ch);
        send_to_char("  value0 value1 value2 value3 [v0,v1,v2,v3]\n\r", ch);
        send_to_char("  extra wear level weight cost timer\n\r", ch);
        send_to_char("  apply\n\r", ch);
        send_to_char("String being one of:\n\r", ch);
        send_to_char("  name short long ed\n\r", ch);
        return;
    }

    if ((obj = get_obj_world(ch, arg1)) == NULL) {
        send_to_char("Nothing like that in hell, earth, or heaven.\n\r", ch);
        return;
    }

    if (IS_SET(obj->extra_flags, ITEM_CLAN_EQ) && ch->level != MAX_LEVEL) {
        send_to_char("Only creators can OSET Clan equipment!\n\r", ch);
        return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = atoi(arg3);

    /*
     * Set something.
     */
    if (!str_cmp(arg2, "value0") || !str_cmp(arg2, "v0")) {
        if (ch->level < 90)
            return;
        obj->value[0] = value;
        return;
    }

    if (!str_cmp(arg2, "value1") || !str_cmp(arg2, "v1")) {
        if (ch->level < 90)
            return;

        obj->value[1] = value;
        return;
    }

    if (!str_cmp(arg2, "value2") || !str_cmp(arg2, "v2")) {
        if (ch->level < 90)
            return;

        obj->value[2] = value;
        return;
    }

    if (!str_cmp(arg2, "value3") || !str_cmp(arg2, "v3")) {
        if (ch->level < 90)
            return;

        obj->value[3] = value;
        return;
    }

    if (!str_cmp(arg2, "extra")) {
        num = 1;
        argn = arg3;
        if (argn[0] == '+') {
            num = 1;
            argn++;
        }
        if (argn[0] == '-') {
            num = 0;
            argn++;
        }
        value = table_lookup(tab_obj_flags, argn);
        if (value == 0) {
            sprintf(buf, "Values for extra flags are +/- :\n\r");
            wide_table_printout(tab_obj_flags, buf + strlen(buf));
            send_to_char(buf, ch);
            return;
        }
        if (!ok_to_use(ch, tab_obj_flags, value))
            return;

        if (num == 1)
            SET_BIT(obj->extra_flags, value);
        else
            REMOVE_BIT(obj->extra_flags, value);
        return;
    }

    /* allow "item apply" changes */
    if (!str_cmp(arg2, "apply")) {
        num = 1;
        argn = arg3;
        if (argn[0] == '+') {
            num = 1;
            argn++;
        }
        if (argn[0] == '-') {
            num = 0;
            argn++;
        }
        value = table_lookup(tab_item_apply, argn);
        if (value == 0) {
            sprintf(buf, "Values for apply flags are +/- :\n\r");
            wide_table_printout(tab_item_apply, buf + strlen(buf));
            send_to_char(buf, ch);
            return;
        }
        if (!ok_to_use(ch, tab_item_apply, value))
            return;

        if (num == 1)
            SET_BIT(obj->item_apply, value);
        else
            REMOVE_BIT(obj->item_apply, value);
        return;
    }

    if (!str_cmp(arg2, "wear")) {
        num = 1;
        argn = arg3;
        if (argn[0] == '+') {
            num = 1;
            argn++;
        }
        if (argn[0] == '-') {
            num = 0;
            argn++;
        }
        value = table_lookup(tab_wear_flags, argn);
        if (value == 0) {
            sprintf(buf, "Values for wear flags are +/- :\n\r");
            wide_table_printout(tab_wear_flags, buf + strlen(buf));
            send_to_char(buf, ch);
            return;
        }
        if (!ok_to_use(ch, tab_wear_flags, value))
            return;

        if (num == 1)
            SET_BIT(obj->wear_flags, value);
        else
            REMOVE_BIT(obj->wear_flags, value);
        return;
    }

    if (!str_cmp(arg2, "level")) {
        if (ch->level < 90)
            return;

        obj->level = value;
        return;
    }

    if (!str_cmp(arg2, "weight")) {
        obj->weight = value;
        return;
    }

    if (!str_cmp(arg2, "cost")) {
        obj->cost = value;
        return;
    }

    if (!str_cmp(arg2, "timer")) {
        obj->timer = value;
        return;
    }

    if (!str_cmp(arg2, "name")) {
        free_string(obj->name);
        obj->name = str_dup(arg3);
        return;
    }

    if (!str_cmp(arg2, "short")) {
        free_string(obj->short_descr);
        obj->short_descr = str_dup(arg3);
        return;
    }

    if (!str_cmp(arg2, "long")) {
        sprintf(buf, "%s\n\r", arg3);
        free_string(obj->description);
        obj->description = str_dup(buf);
        return;
    }

    if (!str_cmp(arg2, "ed")) {
        EXTRA_DESCR_DATA   *ed;

        argument = one_argument(argument, arg3);
        if (argument == NULL) {
            send_to_char("Syntax: oset <object> ed <keyword> <string>\n\r", ch);
            return;
        }

        GET_FREE(ed, exdesc_free);
        ed->keyword = str_dup(arg3);
        ed->description = str_dup(argument);
        LINK(ed, obj->first_exdesc, obj->last_exdesc, next, prev);
        return;
    }

    /*
     * Generate usage message.
     */
    do_oset(ch, "");
    return;
}

void
do_rset(CHAR_DATA *ch, char *argument)
{
    char                arg1[MAX_INPUT_LENGTH];
    char                arg2[MAX_INPUT_LENGTH];
    char                arg3[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA    *location;
    int                 value;

    smash_tilde(argument);
    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    strcpy(arg3, argument);

    if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0') {
        send_to_char("Syntax: rset <location> <field> value\n\r", ch);
        send_to_char("\n\r", ch);
        send_to_char("Field being one of:\n\r", ch);
        send_to_char("  flags sector\n\r", ch);
        return;
    }

    if ((location = find_location(ch, arg1)) == NULL) {
        send_to_char("No such location.\n\r", ch);
        return;
    }

    /*
     * Snarf the value.
     */
    if (!is_number(arg3)) {
        send_to_char("Value must be numeric.\n\r", ch);
        return;
    }
    value = atoi(arg3);

    /*
     * Set something.
     */
    if (!str_cmp(arg2, "flags")) {
        location->room_flags = value;
        return;
    }

    if (!str_cmp(arg2, "sector")) {
        location->sector_type = value;
        return;
    }

    /*
     * Generate usage message.
     */
    do_rset(ch, "");
    return;
}

void
subusers(CHAR_DATA *ch, DESCRIPTOR_DATA *d, int count, bool showaddr)
{
    char                state[MSL];
    char                nbuf[MIL];
    char                ibuf[MIL];

    if (d->character == NULL)
        return;

    switch (d->connected) {
        case CON_PLAYING:
            strcpy(state, "Playing   ");
            break;
        case CON_GET_NAME:
            strcpy(state, "@@yGetName   @@N");
            break;
        case CON_GET_OLD_PASSWORD:
            strcpy(state, "@@yGetOldPass@@N");
            break;
        case CON_CONFIRM_NEW_NAME:
            strcpy(state, "@@yCfmNewName@@N");
            break;
        case CON_GET_NEW_PASSWORD:
            strcpy(state, "@@yGetNewPass@@N");
            break;
        case CON_CONFIRM_NEW_PASSWORD:
            strcpy(state, "@@yCfmNewPass@@N");
            break;
        case CON_GET_NEW_SEX:
            strcpy(state, "@@yGetNewSex @@N");
            break;
        case CON_GET_NEW_CLASS:
            strcpy(state, "@@yGetNewCls @@N");
            break;
        case CON_GET_RACE:
            strcpy(state, "@@yGetNewRace@@N");
            break;
        case CON_READ_MOTD:
            strcpy(state, "@@yReadngMOTD@@N");
            break;
        case CON_MENU:
            strcpy(state, "@@yMenu      @@N");
            break;
        case CON_GET_STATS:
            strcpy(state, "@@yRolling   @@N");
            break;
        default:
            strcpy(state, "@@yUnknown   @@N");
            break;
    }

    if (showaddr)
        sendf(ch, "@@d| @@g%s @@d| @@g%s @@d|\n\r",
            my_right(d->original ? d->original->name : d->character ? d->character->name : "(none)", nbuf, 12),
            my_left(d->host, ibuf, 60));
    else
        sendf(ch, "@@d| @@g%3d @@d| @@g%s @@d| @@g%s @@N@@d| @@g%s @@d| @@g%5d @@d|\n\r",
            d->descriptor,
            state,
            my_right(d->original ? d->original->name : d->character ? d->character->name : "(none)", nbuf, 12),
            my_left(d->ip, ibuf, 15),
            d->remote_port);

    return;
}

int
users_cmp(const void *x, const void *y)
{
    DESCRIPTOR_DATA    *dx = *(DESCRIPTOR_DATA **) x;
    DESCRIPTOR_DATA    *dy = *(DESCRIPTOR_DATA **) y;
    unsigned long       dxip, dyip;

    dxip = ntohl(inet_addr(dx->ip));
    dyip = ntohl(inet_addr(dy->ip));

    if (dxip < dyip)
        return -1;

    if (dxip == dyip)
        return 0;

    return 1;
}

void
do_users(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    char                buf2[MAX_STRING_LENGTH];
    char                arg[MIL];
    DESCRIPTOR_DATA    *d;
    DNS_DATA           *dd;
    int                 count, count2, c;
    bool                sorted = FALSE;
    bool                showaddr = FALSE;
    extern struct dns_setup dns;

    count = 0;
    buf[0] = '\0';
    buf2[0] = '\0';

    argument = one_argument(argument, arg);

    if (!str_cmp(arg, "dns")) {
        send_to_char("lookups:\n\r\n\r", ch);
        for (dd = dns.first_lookup; dd != NULL; dd = dd->next)
            sendf(ch, "flags[%d] host[%s] ip[%s] expire[%s]\n\r", dd->flags, dd->host, dd->ip, duration(dd->expire - current_time, buf2));

        send_to_char("\n\rcached:\n\r\n\r", ch);
        for (dd = dns.first_cache; dd != NULL; dd = dd->next)
            sendf(ch, "flags[%d] host[%s] ip[%s] expire[%s]\n\r", dd->flags, dd->host, dd->ip, duration(dd->expire - current_time, buf2));

        return;
    }

    if (!str_cmp(arg, "host"))
        showaddr = TRUE;

    if (!str_cmp(arg, "sort") || !str_cmp(argument, "sort"))
        sorted = TRUE;

    if (showaddr) {
        send_to_char("@@N@@d.-----------------------------------------------------------------@@g=( @@WUsers @@g)=-@@d.\n\r", ch);
        send_to_char("@@d|         @@gName @@d| @@gHostname                                                     @@d|\n\r", ch);
        send_to_char("@@d|--------------+--------------------------------------------------------------|\n\r", ch);
    }
    else {
        send_to_char("@@N@@d.-----------------------------------------------@@g=( @@WUsers @@g)=@@d-.\n\r", ch);
        send_to_char("@@d|   @@g# @@d| @@gState      @@d|         @@gName @@d| @@gIP Address      @@d| @@g Port @@d|\n\r", ch);
        send_to_char("@@d|-----+------------+--------------+-----------------+-------|\n\r", ch);
    }

    for (d = first_desc; d != NULL; d = d->next)
        if (!sorted && d->character != NULL)
            subusers(ch, d, ++count, showaddr);
        else if (sorted && d->character != NULL)
            ++count;

    if (sorted) {
        void              **dsort = (void **) malloc(sizeof(DESCRIPTOR_DATA *) * count);
        memset(dsort, 0, sizeof(DESCRIPTOR_DATA *) * count);
        c = 0;

        for (d = first_desc; d != NULL; d = d->next)
            if (d->character != NULL)
                dsort[c++] = (void *) d;

        qsort(dsort, c, sizeof(void *), users_cmp);

        count = 0;
        for (count2 = 0; count2 < c; count2++) {
            d = dsort[count2];

            subusers(ch, d, ++count, showaddr);
        }

        free(dsort);
    }

    if (showaddr)
        send_to_char("@@d'-----------------------------------------------------------------------------'@@N\n\r", ch);
    else
        send_to_char("@@d'-----------------------------------------------------------'@@N\n\r", ch);

    return;
}

/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void
do_force(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    int                 trust;
    int                 cmd;

    argument = one_argument(argument, arg);

    if (arg[0] == '\0' || argument[0] == '\0') {
        send_to_char("Force whom to do what?\n\r", ch);
        return;
    }

    /*
     * Look for command in command table.
     */
    trust = get_trust(ch);
    for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++) {
        if (argument[0] == cmd_table[cmd].name[0]
            && !str_prefix(argument, cmd_table[cmd].name)
            && (cmd_table[cmd].level > trust && cmd_table[cmd].level != 41)) {
            send_to_char("You can't even do that yourself!\n\r", ch);
            return;
        }
    }

    /*
     * Allow force to be used on ALL mobs....
     * Only highest level players to use this... it can cause trouble!!!
     * Good for mob "invasions"
     * This could get interesting ;)
     * -- Stephen
     */

    if (!str_cmp(arg, "everymob")) {

        CHAR_DATA          *vch;
        CHAR_DATA          *vch_next;

        if (ch->level < MAX_LEVEL) {
            send_to_char("This option is only available to true Gods.\n\r", ch);
            return;
        }

        for (vch = first_char; vch != NULL; vch = vch = vch_next) {

            vch_next = vch->next;

            if (IS_NPC(vch)) {
                interpret(vch, argument);
            }
        }
        return;
    }

    /* Like above but for mobs in same area as ch */

    if (!str_cmp(arg, "localmobs"))
    {
        CHAR_DATA          *vim;
        CHAR_DATA          *vim_next;

        for (vim = first_char; vim != NULL; vim = vim = vim_next) {

            vim_next = vim->next;

            if (IS_NPC(vim)
                && (vim->in_room->area == ch->in_room->area)) {
                interpret(vim, argument);
            }
        }
        return;
    }

    if (!str_cmp(arg, "all")) {
        CHAR_DATA          *vch;
        CHAR_DATA          *vch_next;

        for (vch = first_char; vch != NULL; vch = vch_next) {
            vch_next = vch->next;

            if (!IS_NPC(vch) && !IS_IMMORTAL(vch)) {
                MOBtrigger = FALSE;
                act("$n forces you to '$t'.", ch, argument, vch, TO_VICT);
                interpret(vch, argument);
            }
        }
    }
    else {
        CHAR_DATA          *victim;

        if ((victim = get_char_world(ch, arg)) == NULL) {
            send_to_char("They aren't here.\n\r", ch);
            return;
        }

        if (victim == ch) {
            send_to_char("Aye aye, right away!\n\r", ch);
            return;
        }

        if (get_trust(victim) >= get_trust(ch)) {
            send_to_char("Do it yourself!\n\r", ch);
            return;
        }

        MOBtrigger = FALSE;
        act("$n forces you to '$t'.", ch, argument, victim, TO_VICT);
        interpret(victim, argument);
    }

    send_to_char("Ok.\n\r", ch);
    return;
}

/*
 * New routines by Dionysos.
 */
void
do_invis(CHAR_DATA *ch, char *argument)
{

    sh_int              level;

    level = -1;

    if (argument[0] != '\0')
        /* Then we have a level argument */
    {
        if (!is_number(argument)) {
            level = get_trust(ch);
        }
        level = UMAX(1, atoi(argument));
        level = UMIN(ch->level, level);

        if (IS_SET(ch->act, PLR_WIZINVIS)) {
            ch->invis = level;
            sendf(ch, "Wizinvis changed to level: %d\n\r", level);
            return;
        }

    }

    if (level == -1)
        level = get_trust(ch);

    ch->invis = level;

    if (IS_NPC(ch))
        return;

    if (IS_SET(ch->act, PLR_WIZINVIS)) {
        REMOVE_BIT(ch->act, PLR_WIZINVIS);
        act("Small, dazzling spots of light focus into the shape of $n!", ch, NULL, NULL, TO_ROOM);

        send_to_char("Your body becomes solid again.\n\r", ch);
    }
    else {
        SET_BIT(ch->act, PLR_WIZINVIS);
        act("$n dissolves into a storm of dazzling points of light!", ch, NULL, NULL, TO_ROOM);
        send_to_char("You slowly vanish into thin air.\n\r", ch);
        sendf(ch, "Setting Wizinvis to level: %d.\n\r", level);
    }

    return;
}

void
do_holylight(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
        return;

    if (IS_SET(ch->act, PLR_HOLYLIGHT)) {
        REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
        send_to_char("Holy light mode off.\n\r", ch);
    }
    else {
        SET_BIT(ch->act, PLR_HOLYLIGHT);
        send_to_char("Holy light mode on.\n\r", ch);
    }

    return;
}

/* Wizify and Wizbit sent in by M. B. King */

void
do_wizify(CHAR_DATA *ch, char *argument)
{
    char                arg1[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim;

    argument = one_argument(argument, arg1);
    if (arg1[0] == '\0') {
        send_to_char("Syntax: wizify <name>\n\r", ch);
        return;
    }
    if ((victim = get_char_world(ch, arg1)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }
    if (IS_NPC(victim)) {
        send_to_char("Not on mobs.\n\r", ch);
        return;
    }
    victim->wizbit = !victim->wizbit;
    if (victim->wizbit) {
        act("$N wizified.", ch, NULL, victim, TO_CHAR);
        act("$n has wizified you!", ch, NULL, victim, TO_VICT);
    }
    else {
        act("$N dewizzed.", ch, NULL, victim, TO_CHAR);
        act("$n has dewizzed you!", ch, NULL, victim, TO_VICT);
    }

    do_save(victim, "");
    return;
}

/* Idea from Talen of Vego's do_where command */

void
do_owhere(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    char                arg[MAX_INPUT_LENGTH];
    bool                found = FALSE;
    OBJ_DATA           *obj;
    OBJ_DATA           *in_obj;
    int                 obj_counter = 1;
    extern OBJ_DATA    *auction_item;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
        send_to_char("Syntax:  owhere <object>.\n\r", ch);
        return;
    }
    else {
        for (obj = first_obj; obj != NULL; obj = obj->next) {
            if (!can_see_obj(ch, obj) || !is_name(arg, obj->name))
                continue;
            if (obj == auction_item)
                continue;

            found = TRUE;

            for (in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj);

            if (in_obj->carried_by != NULL) {
                sprintf(buf, "[%2d] %s carried by %s [Room:%d].\n\r",
                    obj_counter, obj->short_descr, PERS(in_obj->carried_by, ch), in_obj->carried_by->in_room->vnum);
            }
            else if (in_obj->in_room != NULL) {
                sprintf(buf, "[%2d] %s in %s [Room:%d].\n\r", obj_counter, obj->short_descr, in_obj->in_room->name, in_obj->in_room->vnum);
            }
            else
                continue;

            obj_counter++;
            send_to_char(buf, ch);
        }
    }

    if (!found)
        send_to_char("Nothing like that in hell, earth, or heaven.\n\r", ch);

    return;

}

void
do_oflag(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    bool                found = FALSE;
    AREA_DATA          *area;
    BUILD_DATA_LIST    *obj;
    OBJ_INDEX_DATA     *pobj;
    int                 value = -1;

    if (*argument == '\0') {
        send_to_char("Syntax:  oflag <extra flag type>.\n\r", ch);
        return;
    }
    else {
        value = table_lookup(tab_obj_flags, argument);
        if (value < 1) {
            sprintf(buf, "Valid extra flags are:\n\r");
            table_printout(tab_obj_flags, buf + strlen(buf));
            send_to_char(buf, ch);
            return;
        }

        for (area = first_area; area != NULL; area = area->next)
            for (obj = area->first_area_object; obj != NULL; obj = obj->next) {
                pobj = obj->data;

                if (IS_SET(pobj->extra_flags, value)) {
                    if (!found) {
                        found = TRUE;
                        sendf(ch, "@@N@@gObjects with extra flag '%s':\n\r\n\r",
                            rev_table_lookup(tab_obj_flags, value)
                        );
                    }

                    sendf(ch, "@@N@@d[@@g%5d@@d] @@N%s@@N\n\r", pobj->vnum, pobj->short_descr);
                }
            }

    }

    if (!found)
        send_to_char("No objects have that extra flag.\n\r", ch);

    return;

}

void
do_mflag(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    bool                found = FALSE;
    AREA_DATA          *area;
    BUILD_DATA_LIST    *mob;
    MOB_INDEX_DATA     *pmob;
    int                 value = -1;

    if (*argument == '\0') {
        send_to_char("Syntax:  mflag <mob flag type>.\n\r", ch);
        return;
    }
    else {
        value = table_lookup(tab_mob_flags, argument);
        if (value < 1) {
            sprintf(buf, "Valid mob flags are:\n\r");
            table_printout(tab_mob_flags, buf + strlen(buf));
            send_to_char(buf, ch);
            return;
        }

        for (area = first_area; area != NULL; area = area->next)
            for (mob = area->first_area_mobile; mob != NULL; mob = mob->next) {
                pmob = mob->data;

                if (IS_SET(pmob->act, value)) {
                    if (!found) {
                        found = TRUE;
                        sendf(ch, "@@N@@gMobiles with flag '%s':\n\r\n\r",
                            rev_table_lookup(tab_mob_flags, value)
                        );
                    }

                    sendf(ch, "@@N@@d[@@g%5d@@d] @@N%s@@N\n\r", pmob->vnum, pmob->short_descr);
                }
            }

    }

    if (!found)
        send_to_char("No mobiles have that flag set.\n\r", ch);

    return;

}

void
do_mpcr(CHAR_DATA *ch, char *victim)
{
    /* A Function to perform Corpse Retrivals (CRs) 
     * Gets first corpse (if any) matching argument.
     * Will NOT get corpses from the room ch is in, allowing the
     * (N)PC to keep calling this function, and to 'pile up' all the
     * corpses.
     * -- Stephen
     */

    OBJ_DATA           *obj;
    bool                found = FALSE;
    char                arg[MAX_INPUT_LENGTH];

    one_argument(victim, arg);

    if (arg[0] == '\0') {
        send_to_char("Retrive WHICH corpse??\n\r", ch);
        return;
    }

    for (obj = first_obj; obj != NULL; obj = obj->next) {
        if (((obj->pIndexData->vnum) == OBJ_VNUM_CORPSE_PC)
            && (!str_cmp(arg, obj->owner))
            && (!(obj->in_room == ch->in_room))) {    /*don't work! */
            found = TRUE;
            obj_from_room(obj);
            obj_to_room(obj, ch->in_room);
            act("Got the blighter!", ch, NULL, NULL, TO_CHAR);

        }

    }

    /* act used to enable mobiles to check for CR triggers... */

    if (!found) {
        act("Couldn't find it.", ch, NULL, NULL, TO_CHAR);
    }
    return;
}

void
do_resetpassword(CHAR_DATA *ch, char *argument)
{
    char                arg1[MAX_INPUT_LENGTH];
    char                md5buf[33];
    CHAR_DATA          *victim;
    char               *pwdnew;

    if (IS_NPC(ch))
        return;

    argument = one_argument(argument, arg1);

    victim = get_char_world(ch, arg1);

    if (victim == '\0') {
        send_to_char("This character is not playing at this time\n\r", ch);
        return;
    }
    if ((ch->level != MAX_LEVEL) && ch->level < victim->level) {
        send_to_char("You cannot change the password of immortals!\n\r", ch);
        return;
    }
    if (IS_NPC(victim)) {
        send_to_char("You cannot change the password of NPCs!\n\r", ch);
        return;
    }

    if ((ch->pcdata->pwd != '\0')
        && (arg1[0] == '\0' || *argument == '\0')) {
        send_to_char("Syntax: password <char> <new>.\n\r", ch);
        return;
    }

    if (strlen(argument) < 5) {
        send_to_char("New password must be at least five characters long.\n\r", ch);
        return;
    }

    pwdnew = md5string(argument, md5buf);

    free_string(victim->pcdata->pwd);
    victim->pcdata->pwd = str_dup(pwdnew);
    save_char_obj(victim);
    send_to_char("Ok.\n\r", ch);
    return;
}

void
do_fights(CHAR_DATA *ch, char *argument)
{
    /* Displays list of any PCs currently fighting.  Also shows 'victim'
     * --Stephen
     */

    CHAR_DATA          *vch;
    char                buf[MAX_STRING_LENGTH];
    char                buf2[MAX_STRING_LENGTH];
    int                 count;

    count = 0;
    buf2[0] = '\0';

    for (vch = first_char; vch != NULL; vch = vch->next) {
        if (!IS_NPC(vch) && (vch->fighting != NULL) && can_see(ch, vch)) {
            count++;
            sprintf(buf, "%s Vs. %s  [Room:%5d]\n\r", vch->name, IS_NPC(vch->fighting) ?
                vch->fighting->short_descr : vch->fighting->name, vch->in_room->vnum);
            strcat(buf2, buf);
        }
    }

    if (count == 0)
        strcat(buf2, "No Players are currently fighting!\n\r");
    else {
        sprintf(buf, "%d fight%s\n\r", count, (count > 1) ? "s." : ".");
        strcat(buf2, buf);
    }

    strcat(buf2, "\n\r");
    send_to_char(buf2, ch);
    return;
}

void
do_iwhere(CHAR_DATA *ch, char *argument)
{
    /* Like WHERE, except is global, and shows area & room.
     * --Stephen
     */

    CHAR_DATA          *vch;
    char                buf[MAX_STRING_LENGTH];
    char                buf2[MAX_STRING_LENGTH];
    int                 count = 0;

    buf2[0] = '\0';

    for (vch = first_player; vch != NULL; vch = vch->next_player) {
        if (!IS_NPC(vch) && can_see(ch, vch)) {
            char                nbuf[MSL];
            char                abuf[MSL];
            char                rbuf[MSL];

            count++;
            sprintf(buf, "@@d| %s @@N@@g%s @@N@@d| @@g%5d @@d| @@g%s @@N@@d| @@g%s @@N@@d|\n\r",
                vch->desc ? " " : "@@y*",
                my_left(vch->short_descr, nbuf, 12),
                vch->in_room == NULL ? 0 : vch->in_room->vnum, my_left(vch->in_room->area->name, abuf, 27), my_left(vch->in_room->name, rbuf, 20));
            safe_strcat(MSL, buf2, buf);
        }
    }

    if (count == 0) {
        send_to_char("No people found.\n\r", ch);
        return;
    }

    sendf(ch, "@@d.-------------------------------------@@g=@@W( @@aImmortal Where @@d[@@gfound @@a%3d @@g%s@@d] @@W)@@g=@@d-.\n\r", count, (count == 1) ? "person" : "people");
    send_to_char("@@d| @@y* @@gName         @@d|  @@gVnum @@d| @@gArea                        @@d| @@gRoom                 @@d|\n\r", ch);
    send_to_char("@@d|----------------+-------+-----------------------------+----------------------|\n\r", ch);
    send_to_char(buf2, ch);
    send_to_char("@@d'-----------------------------------------------------------------------------'@@N\n\r", ch);
    return;
}

void
do_setclass(CHAR_DATA *ch, char *argument)
{
    /*
     * New version of advance, using some of old code.
     * and a slight change to what was left.  -S-
     * Added support for setting remort class levels.
     */

    char                arg1[MAX_INPUT_LENGTH];
    char                arg2[MAX_INPUT_LENGTH];
    char                arg3[MAX_INPUT_LENGTH];
    char                buf[MAX_STRING_LENGTH];
    CHAR_DATA          *victim;
    int                 value;
    int                 iClass;
    bool                cok, remort;
    int                 class = 0;
    int                 cnt;
    int                 lose;

    argument = one_argument(argument, arg1);    /* Player */
    argument = one_argument(argument, arg2);    /* class */
    strcpy(arg3, argument);        /* arg3 = value */

    if (arg1[0] == '\0' || arg2[0] == '\0') {
        send_to_char("Syntax: SetClass <player> <class> <value>\n\r", ch);
        send_to_char("if value = -1 then player will NOT be able to level in that class.\n\r", ch);
        return;
    }

    if ((victim = get_char_world(ch, arg1)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }
    cok = FALSE;
    remort = FALSE;

    for (iClass = 0; iClass < MAX_CLASS; iClass++) {
        if (!str_cmp(arg2, class_table[iClass].who_name)) {
            class = iClass;
            cok = TRUE;
        }
        if (!str_cmp(arg2, remort_table[iClass].who_name)) {
            class = iClass;
            cok = TRUE;
            remort = TRUE;
        }

    }

    if (!str_prefix(arg2, "ADEPT")) {
        if (victim->adept_level > 0) {
            send_to_char("They are already an adept.\n\r", ch);
            return;
        }
        else {
            class = ADVANCE_ADEPT;
            advance_level(victim, class, TRUE, FALSE, FALSE);
            victim->adept_level = 1;
            sprintf(buf, " %s %s", victim->name, get_adept_name(victim));
            do_whoname(ch, buf);
            victim->exp = 0;
            do_save(victim, "");
            return;
        }
    }

    if (!cok) {
        send_to_char("That's not a class!!\n\r", ch);
        return;
    }

    value = is_number(arg3) ? atoi(arg3) : -9;

    if (value == -9) {
        send_to_char("Invalid value for value\n\r\n\r", ch);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("Not on NPC's.\n\r", ch);
        return;
    }

    if (value < -1 || value > MAX_LEVEL) {
        sendf(ch, "%d is not a valid value.\n\r", value);
        sendf(ch, "Use a value between -1 and %d.\n\r\n\r", MAX_LEVEL - 1);
        return;
    }

    if (value > get_trust(ch)) {
        send_to_char("Limited to your trust level.\n\r", ch);
        return;
    }

    /*
     * Lower level:
     *   Reset to level 1.
     *   Then raise again.
     *   Currently, an imp can lower another imp.
     *   -- Swiftest
     */
    if (value <= (remort ? victim->lvl2[class] : victim->lvl[class])) {
        int                 sn;

        lose = (remort ? victim->lvl2[class] - 1 : victim->lvl[class] - 1);

        send_to_char("Lowering a player's level!\n\r", ch);
        send_to_char("**** OOOOHHHHHHHHHH  NNNNOOOO ****\n\r", victim);

        if (remort) {
            if (value != -1)
                victim->lvl2[class] = 1;
            else
                victim->lvl2[class] = -1;
        }
        else
            victim->lvl[class] = 1;

        victim->exp = 0;

        if (remort)
            victim->max_hit -= UMIN(victim->max_hit, lose * remort_table[class].hp_min);
        else
            victim->max_hit -= UMIN(victim->max_hit, lose * class_table[class].hp_min);

        victim->max_mana = 100;
        victim->max_move = 100;
        for (sn = 0; sn < MAX_SKILL; sn++)
            victim->pcdata->learned[sn] = 0;
        victim->practice = 0;
        victim->hit = victim->max_hit;
        victim->mana = victim->max_mana;
        victim->move = victim->max_move;

        advance_level(victim, class, FALSE, remort, FALSE);
    }
    else {
        send_to_char("Raising a player's level!\n\r", ch);
        send_to_char("**** OOOOHHHHHHHHHH  YYYYEEEESSS ****\n\r", victim);
    }

    if (value != -1 && !remort) {
        sendf(victim, "You are now level %d in your %s class.\n\r", value, class_table[class].class_name);

        for (iClass = victim->lvl[class]; iClass < value; iClass++) {
            victim->lvl[class] += 1;
            advance_level(victim, class, FALSE, remort, FALSE);
        }
    }
    if (remort) {
        sendf(victim, "You are now level %d in your %s class.\n\r", value, remort_table[class].class_name);
        for (iClass = victim->lvl2[class]; iClass < value; iClass++) {
            victim->lvl2[class] += 1;
            advance_level(victim, class, FALSE, remort, FALSE);
        }
    }

    victim->exp = 0;
    victim->trust = 0;

    /* Make sure that ch->level holds vicitm's max level */
    victim->level = 0;
    for (cnt = 0; cnt < MAX_CLASS; cnt++)
        if (victim->lvl[cnt] > victim->level)
            victim->level = victim->lvl[cnt];

    /* check for remort levels too... */
    for (cnt = 0; cnt < MAX_CLASS; cnt++)
        if (victim->lvl2[cnt] > victim->level)
            victim->level = victim->lvl2[cnt];

    send_to_char("Ok.\n\r", ch);
    return;
}

void
do_isnoop(CHAR_DATA *ch, char *argument)
{
    /* Creator-only command.  Lists who (if anyone) is being snooped.
     * -S- */

    DESCRIPTOR_DATA    *d;
    int                 count = 0;

    send_to_char("Snoop List:\n\r-=-=-=-=-=-\n\r", ch);

    for (d = first_desc; d != NULL; d = d->next) {
        if (d->snoop_by != NULL) {
            count++;
            sendf(ch, "%s by %s.\n\r", d->character->name, d->snoop_by->character->name);
        }
    }

    if (count != 0)
        sendf(ch, "%d snoops found.\n\r", count);
    else
        sendf(ch, "No snoops found.\n\r");

    return;
}

void
do_dog(CHAR_DATA *ch, char *argument)
{
    /* A real silly command which switches the (mortal) victim into
     * a mob.  As the victim is mortal, they won't be able to use
     * return ;P  So will have to be released by someone...
     * -S-
     */

    ROOM_INDEX_DATA    *location;
    MOB_INDEX_DATA     *pMobIndex;
    CHAR_DATA          *mob;
    CHAR_DATA          *victim;

    if (ch->level < MAX_LEVEL) {
        send_to_char("Only for creators.\n\r", ch);
        return;
    }

    if (argument[0] == '\0') {
        send_to_char("Turn WHO into a little doggy?\n\r", ch);
        return;
    }

    if ((victim = get_char_world(ch, argument)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("Cannot do this to mobs, only pcs.\n\r", ch);
        return;
    }

    if ((pMobIndex = get_mob_index(MOB_VNUM_DOGGY)) == NULL) {
        send_to_char("Couldn't find the doggy's vnum!!\n\r", ch);
        return;
    }

    if (victim->desc == NULL) {
        send_to_char("Already switched, like.\n\r", ch);
        return;
    }

    mob = create_mobile(pMobIndex);
    location = victim->in_room;    /* Remember where to load doggy! */
    char_from_room(victim);

    char_to_room(victim, get_room_index(ROOM_VNUM_LIMBO));
    char_to_room(mob, location);

    /*
       ch->desc->character = victim;
       ch->desc->original  = ch;
       victim->desc        = ch->desc;
       ch->desc            = NULL;
     */

    /* Instead of calling do switch, just do the relevant bit here */
    victim->desc->character = mob;
    victim->desc->original = victim;
    mob->desc = victim->desc;
    victim->desc = NULL;

    act("$n is suddenly turned into a small doggy!!", victim, NULL, NULL, TO_NOTVICT);
    send_to_char("You suddenly turn into a small doggy!\n\r", victim);
    send_to_char("Ok.\n\r", ch);
    return;
}

void
do_togbuild(CHAR_DATA *ch, char *argument)
{
    /* Toggles PC's ch->act PLR_BUILDER value 
     * -S-
     */

    CHAR_DATA          *victim;

    if (argument[0] == '\0') {
        send_to_char("Toggle who as a builder??\n\r", ch);
        return;
    }

    if ((victim = get_char_world(ch, argument)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("Not on NPCs!\n\r", ch);
        return;
    }

    if (!IS_SET(victim->act, PLR_BUILDER)) {
        SET_BIT(victim->act, PLR_BUILDER);
        send_to_char("Bit set to ALLOW building.\n\r", ch);
        send_to_char("You have been authorized to use the builder.\n\r", victim);
    }
    else {
        REMOVE_BIT(victim->act, PLR_BUILDER);
        send_to_char("Bit set to DISALLOW building.\n\r", ch);
        send_to_char("You authorization to build has been revoked.\n\r", victim);
    }

    return;
}

void
do_togleader(CHAR_DATA *ch, char *argument)
{
    /* Toggles PC's ch->pcdata->pfalgs PLR_CLAN_BOSS value 
     * -S-
     */

    CHAR_DATA          *victim;

    if (argument[0] == '\0') {
        send_to_char("Toggle who as a clan boss??\n\r", ch);
        return;
    }

    if ((victim = get_char_world(ch, argument)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("Not on NPCs!\n\r", ch);
        return;
    }

    if (!IS_SET(victim->pcdata->pflags, PFLAG_CLAN_BOSS)) {
        SET_BIT(victim->pcdata->pflags, PFLAG_CLAN_BOSS);
        send_to_char("Bit set for CLAN_BOSS.\n\r", ch);
        send_to_char("You have been set as a clan boss.\n\r", victim);
    }
    else {
        REMOVE_BIT(victim->pcdata->pflags, PFLAG_CLAN_BOSS);
        send_to_char("Bit removed for CLAN_BOSS.\n\r", ch);
        send_to_char("You are no longer a clan boss.\n\r", victim);
    }

    update_cinfo(victim, FALSE);

    return;
}

void
do_whoname(CHAR_DATA *ch, char *argument)
{
    /* Set victim's who name - 
     * what appears on who list in place of their levels
     * --Stephen
     */

    CHAR_DATA          *victim;
    RULER_DATA         *ruler;
    char                arg[MAX_INPUT_LENGTH];
    char                foo[MAX_STRING_LENGTH];
    int                 side;    /* -1 = left, +1 = right side */

    if (IS_NPC(ch))
        return;

    argument = one_argument(argument, arg);

    if (argument[0] == '\0' || arg[0] == '\0') {
        send_to_char("Usage: whoname <victim> <string>\n\r\n\r", ch);
        send_to_char("Where string is no more than 14 letters long.\n\r", ch);
        send_to_char("Use 'off' as name to use default who name.\n\r", ch);
        return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL) {
        send_to_char("Couldn't find target.\n\r", ch);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("Mobiles don't have a whoname!\n\r", ch);
        return;
    }

    if ((get_trust(ch) < (MAX_LEVEL - 1)) && ch != victim) {
        send_to_char("Only Supremes and Creators can set the whoname of others.\n\r", ch);
        return;
    }

    if (!str_cmp(argument, "off")) {
        free_string(victim->pcdata->who_name);
        victim->pcdata->who_name = str_dup("off");
        send_to_char("Who name set to default value.\n\r", ch);
        return;
    }

    if (my_strlen(argument) > 14) {
        send_to_char("Name too long.\n\r", ch);
        do_whoname(ch, "");        /* Usage message */
        return;
    }

    smash_tilde(argument);

    /* Now for the fun part -
     * CENTER the string if less than 14 chars <g>
     * Add spaces to alternate sides - UGLY
     */
    /*foo = str_dup( argument ); */
    side = -1;

    while (my_strlen(argument) < 14) {
        switch (side) {
            case -1:            /* left side */
                sprintf(foo, " %s", argument);
                break;
            case 1:
                sprintf(foo, "%s ", argument);
                break;
        }
        side *= -1;                /* Switch sides for next loop */
        sprintf(argument, "%s", foo);
    }

    free_string(victim->pcdata->who_name);
    victim->pcdata->who_name = str_dup(argument);

    if (victim->adept_level == 20 && (ruler = get_ruler(victim))) {
        free_string(ruler->whoname);
        ruler->whoname = str_dup(argument);
        save_rulers();
    }

    send_to_char("Ok, done.\n\r", ch);
    return;
}

void
do_lhunt(CHAR_DATA *ch, char *argument)
{
    /* Simple function for Imms... loops through all mobs, and
     * shows details of any currently hunting someone. -S-
     */
    /* Rewritten to suit new hunt functions.. :) -- Alty */
    CHAR_DATA          *lch;
    char                buf[MAX_STRING_LENGTH];
    bool                found = FALSE;

    for (lch = first_char; lch; lch = lch->next) {
        if (!lch->hunting && !lch->hunt_obj) {
            if (lch->searching) {
                sendf(ch, "%s searching for %s.\n\r", NAME(lch), lch->searching);
                found = TRUE;
            }
            continue;
        }
        found = TRUE;
        sprintf(buf, "%s (%s)", NAME(lch), (IS_NPC(lch) ? "mobile" : "player"));
        if (lch->hunting)
            sprintf(buf + strlen(buf), " hunting for (%s) %s", (IS_NPC(lch->hunting) ? "mobile" : "player"), NAME(lch->hunting));
        if (lch->hunt_obj) {
            if (lch->hunting && IS_SET(lch->hunt_flags, HUNT_CR) && lch->hunt_obj->item_type == ITEM_CORPSE_PC)
                strcat(buf, " to return a corpse");
            else
                sprintf(buf + strlen(buf), " looking for (object) %s", lch->hunt_obj->short_descr);
        }
        if (IS_NPC(lch) && IS_SET(lch->hunt_flags, HUNT_MERC | HUNT_CR) && lch->hunt_for)
            sprintf(buf + strlen(buf), ", employed by %s", NAME(lch->hunt_for));
        strcat(buf, ".\n\r");
        send_to_char(buf, ch);
    }
    if (!found)
        send_to_char("No one is currently hunting.\n\r", ch);
    return;
}

/*   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *victim;

   for ( victim = first_char; victim != NULL; victim = victim->next )
   {
   if ( IS_NPC( victim ) 
   && IS_SET( victim->act_hunt, ACT_HUNT_MOVE )
   && victim->move_to != NO_VNUM )
   {
   sprintf( buf, "[%s] Moving to (%d) %s.g\n\r", 
   victim->short_descr,
   victim->move_to,
   victim->movename );
   strcat( buf1, buf );
   }

   if ( IS_NPC(victim) && victim->hunting != NULL)
   {
   switch( (int) victim->hunting)
   {
   case -1:
   sprintf(buf, "[%s] Hunting: %s (waiting)\n\r",
   victim->short_descr,
   victim->huntname);
   strcat(buf1,buf);
   break;

   case -2:
   sprintf(buf, "[%s] Returning home\n\r", victim->short_descr);
   strcat(buf1,buf);
   break;

   default:
   sprintf(buf, "[%s] Hunting: %s",
   victim->short_descr,
   IS_NPC(victim->hunting) ? victim->hunting->short_descr
   : victim->hunting->name );
   strcat(buf1, buf);
   if (victim->huntdirs != NULL)
   {
   sprintf(buf," (%i steps)",
   strlen(victim->huntdirs)-victim->huntdirno);
   strcat(buf1,buf);
   }
   strcat( buf1, "\n\r" );
   }
   }
   } 
   send_to_char( buf1, ch );
   return;
   } */

void
do_sstat(CHAR_DATA *ch, char *argument)
{
    /* Lists the % for a player's skill(s)
     * Either shows all, or value for just a given skill
     * -S-
     */

    char                buf[MAX_STRING_LENGTH];
    char                buf1[MAX_STRING_LENGTH];
    char                arg[MAX_INPUT_LENGTH];
    int                 skill = -1;
    int                 sn;
    int                 col;
    CHAR_DATA          *victim;

    argument = one_argument(argument, arg);

    if (arg[0] == '\0') {
        send_to_char("Usage: sstat <victim> [skill]\n\r", ch);
        send_to_char("Where skill is an optional argument.\n\r", ch);
        return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL) {
        send_to_char("Couldn't find target.\n\r", ch);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("Mobiles don't have skills!\n\r", ch);
        return;
    }

    if (argument[0] != '\0') {
        skill = skill_lookup(argument);
        if (skill <= 0) {
            send_to_char("No such skill/spell!\n\r", ch);
            return;
        }

        sendf(ch, "%17s - %3d%%\n\r", skill_table[skill].name, victim->pcdata->learned[skill]);
        return;
    }

    col = 0;
    buf1[0] = '\0';

    for (sn = 0; sn < MAX_SKILL; sn++) {
        if (skill_table[sn].name == NULL)
            break;

        sprintf(buf, "%16s - %3d%%  ", skill_table[sn].name, victim->pcdata->learned[sn]);
        strcat(buf1, buf);

        if (++col % 3 == 0)
            strcat(buf1, "\n\r");
    }
    if (col % 3 != 0)
        strcat(buf1, "\n\r");

    send_to_char(buf1, ch);
    return;
}

void
do_test(CHAR_DATA *ch, char *argument)
{
    send_to_char("html who test!!\n\r", ch);
    list_who_to_output();
    return;
}

struct monitor_type
{
    char               *name;
    int                 channel;
    int                 min_level;
    char               *col;
    char               *id;
    char               *on_name;
    char               *off_name;
};

struct monitor_type monitor_table[] = {
    {"connection", MONITOR_CONNECT, 89, "@@l", "CON",
            "[ CONNECTION   ] Shows details of players connecting to the mud.\n\r",
        "[ connection   ] Not showing details of players connecting.\n\r"},

    {"area_update", MONITOR_AREA_UPDATE, 82, "@@p", "A_UPD",
            "[ AREA_UPDATE  ] Informs you of ALL area updates.\n\r",
        "[ area_update  ] You are not informed of area updates.\n\r"},

    {"area_bugs", MONITOR_AREA_BUGS, 87, "@@p", "A_BUG",
            "[ AREA_BUGS    ] Notifies you of any errors within areas.\n\r",
        "[ area_bugs    ] You are not told of errors within areas.\n\r"},

    {"area_save", MONITOR_AREA_SAVING, 82, "@@p", "A_SAVE",
            "[ AREA_SAVE    ] You get told of all area saving.\n\r",
        "[ area_save    ] You don't get told of all area saves.\n\r"},

    {"objects", MONITOR_OBJ, 89, "@@r", "OBJ",
            "[ OBJECTS      ] You are told of problems relating to objects.\n\r",
        "[ objects      ] You are not told of object-related problems.\n\r"},

    {"mobile", MONITOR_MOB, 85, "@@a", "MOB",
            "[ MOBILE       ] Watching mobile/player problems.\n\r",
        "[ mobile       ] Not watching problems with mobiles/players\n\r"},

    {"room", MONITOR_ROOM, 85, "@@e", "ROOM",
            "[ ROOM         ] You are informed of problems involved with rooms.\n\r",
        "[ room         ] Not informed of problems with rooms.\n\r"},

    {"imm_general", MONITOR_GEN_IMM, 90, "@@y", "IMM_GEN",
            "[ IMM_GENERAL  ] You are notified of use of logged immortal commands.\n\r",
        "[ imm_general  ] You are not told of the use of logged immortal commands.\n\r"},

    {"mort_general", MONITOR_GEN_MORT, 89, "@@y", "MORT_GEN",
            "[ MORT_GENERAL ] You are notified of use of logged mortal commands.\n\r",
        "[ mort_general ] You are not told of the use of logged mortal commands.\n\r"},

    {"combat", MONITOR_COMBAT, 83, "@@R", "COMBAT",
            "[ COMBAT       ] You are monitoring problems in combat.\n\r",
        "[ combat       ] Not monitoring any combat problems.\n\r"},

    {"hunting", MONITOR_HUNTING, 82, "@@B", "HUNT",
            "[ HUNTING      ] You are told of all mobile hunting.\n\r",
        "[ hunting      ] Not told about mobiles hunting players.\n\r"},

    {"build", MONITOR_BUILD, 90, "@@y", "BUILD",
            "[ BUILD        ] You receive logged building commands.\n\r",
        "[ build        ] You don't monitor logged building commands.\n\r"},

    {"clan", MONITOR_CLAN, 89, "@@b", "CLAN",
            "[ CLAN         ] You are informed of use of certain clan commands.\n\r",
        "[ clan         ] You are not told of use of certain clan commands.\n\r"},

    {"bad", MONITOR_BAD, 88, "@@W", "BAD",
            "[ BAD          ] You are told of 'bad' things players (try to) do!\n\r",
        "[ bad          ] Not told of 'bad' things players do.\n\r"},

    {"rawcol", MONITOR_RAWCOL, 81, "@@W", "RAWCOL",
            "[ RAWCOL       ] You see colours in their raw format.\n\r",
        "[ rawcol       ] You don't see colours in their raw format.\n\r"},

    {NULL, 0, 0, NULL, NULL}
};

void
do_monitor(CHAR_DATA *ch, char *argument)
{
    int                 a;
    bool                found = FALSE;

    if (IS_NPC(ch)) {
        send_to_char("Not for NPCs.\n\r", ch);
        return;
    }

    if (argument[0] == '\0') {
        send_to_char("@@yMonitor Channel Details:@@g\n\r\n\r", ch);
        for (a = 0; monitor_table[a].min_level != 0; a++) {

            if (monitor_table[a].min_level > get_trust(ch))
                continue;

            if (IS_SET(ch->pcdata->monitor, monitor_table[a].channel))
                sendf(ch, "@@W%s@@g", monitor_table[a].on_name);
            else
                sendf(ch, "@@d%s@@g", monitor_table[a].off_name);
        }
        send_to_char("\n\r@@yMONITOR <name> toggles the monitor channels.@@N\n\r", ch);
        return;
    }
    /* Search for monitor channel to turn on/off */
    for (a = 0; monitor_table[a].min_level != 0; a++) {
        if (!strcmp(argument, monitor_table[a].name)) {
            found = TRUE;
            if (IS_SET(ch->pcdata->monitor, monitor_table[a].channel))
                REMOVE_BIT(ch->pcdata->monitor, monitor_table[a].channel);
            else
                SET_BIT(ch->pcdata->monitor, monitor_table[a].channel);
            break;
        }
    }
    if (!found) {
        do_monitor(ch, "");
        return;
    }
    send_to_char("Ok, monitor channel toggled.\n\r", ch);
    return;
}

void
monitor_chan(const char *message, int channel)
{
    char                buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA    *d;
    int                 a;
    int                 level = 90;

    for (a = 0; monitor_table[a].min_level != 0; a++)
        if (monitor_table[a].channel == channel) {
            level = monitor_table[a].min_level;
            break;
        }

    sprintf(buf, "%s[MON:%s]@@N %s@@N\n\r", monitor_table[a].col, monitor_table[a].id, message);

    for (d = first_desc; d; d = d->next) {
        if (d->connected == CON_PLAYING && !IS_NPC(d->character)
            && IS_SET(d->character->pcdata->monitor, channel)
            && level <= get_trust(d->character))
            send_to_char(buf, d->character);
    }
    return;
}

void
do_reward(CHAR_DATA *ch, char *argument)
{
    char                arg1[MAX_INPUT_LENGTH];
    char                arg2[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim = NULL;
    int                 value = 0;
    bool                fAll = FALSE;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0' || arg2[0] == '\0') {
        send_to_char("Syntax: reward <victim> <value>\n\r", ch);
        send_to_char("Value being pos to give points, or neg to take points.\n\r", ch);
        return;
    }

    if (!is_number(arg2)) {
        send_to_char("Value must be numeric.\n\r", ch);
        return;
    }

    value = atoi(arg2);
    if (ch->level < MAX_LEVEL && (value < -100 || value > 100)) {
        send_to_char("Value range is -100 to 100.\n\r", ch);
        return;
    }

    if (!str_cmp(arg1, "all"))
        fAll = TRUE;

    if (!fAll && (victim = get_char_world(ch, arg1)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (!fAll && IS_NPC(victim)) {
        send_to_char("Not on NPC's.\n\r", ch);
        return;
    }

    if (fAll) {
        /* then loop through all players and reward or charge  them */
        CHAR_DATA          *vch;
        CHAR_DATA          *vch_next;

        for (vch = first_player; vch != NULL; vch = vch_next) {
            vch_next = vch->next_player;

            if (!IS_NPC(vch)) {
                if (ch == vch)
                    continue;

                if (IS_IMMORTAL(vch)) {
                    sendf(vch, "Everyone has been rewarded @@y%d @@aQuest Points by %s@@N.\n\r", value, ch->short_descr);
                    continue;
                }

                if (value > 0) {
                    sendf(vch, "@@NYou have been rewarded @@y%d @@aQuest Points@@N by @@m%s@@N!!!\n\r", value, ch->short_descr);
                    sendf(ch, "@@NYou have rewarded @@r%s @@y%d @@aQuest Points@@N!!!\n\r", vch->short_descr, value);
                }
                else {
                    sendf(vch, "@@NYou have been charged @@y%d @@aQuest Points@@N by @@m%s@@N!!!\n\r", abs(value), ch->short_descr);
                    sendf(ch, "@@NYou have charged @@r%s @@y%d @@aQuest Points@@N!!!\n\r", vch->short_descr, abs(value));
                }

                vch->quest_points += value;
                do_save(vch, "");
            }
        }

        send_to_char("Everyone has been rewarded or charged.\n\r", ch);
        return;
    }

    /*
     * Snarf the value.
     */

    if (value > 0) {
        sendf(victim, "@@NYou have been rewarded @@y%d @@aQuest Points@@N by @@m%s@@N!!!\n\r", value, ch->short_descr);
        sendf(ch, "@@NYou have rewarded @@r%s @@y%d @@aQuest Points@@N!!!\n\r", victim->short_descr, value);
    }
    else {
        sendf(victim, "@@NYou have been charged @@y%d @@aQuest Points@@N by @@m%s@@N!!!\n\r", abs(value), ch->short_descr);
        sendf(ch, "@@NYou have charged @@r%s @@y%d @@aQuest Points@@N!!!\n\r", victim->short_descr, abs(value));
    }

    victim->quest_points += value;
    do_save(victim, "");

    return;
}

void
do_xpreward(CHAR_DATA *ch, char *argument)
{
    char                arg1[MAX_INPUT_LENGTH];
    char                arg2[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim;
    int                 value;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0' || arg2[0] == '\0') {
        send_to_char("Syntax: xpreward <victim> <value>\n\r", ch);
        send_to_char("Value being pos to give xp, or neg to take xp.\n\r", ch);
        return;
    }

    if ((victim = get_char_world(ch, arg1)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("Not on NPC's.\n\r", ch);
        return;
    }

    /*
     * Snarf the value.
     */
    if (!is_number(arg2)) {
        send_to_char("Value must be numeric.\n\r", ch);
        return;
    }

    value = atoi(arg2);

    sendf(victim, "@@NYou have been rewarded @@y%d @@aExperience Points@@N by @@m%s@@N!!!\n\r", value, ch->short_descr);
    sendf(ch, "@@NYou have rewarded @@r%s @@y%d @@aExperience Points@@N!!!\n\r", victim->short_descr, value);

    victim->exp += value;
    do_save(victim, "");

    return;
}

void
do_fhunt(CHAR_DATA *ch, char *argument)
{
    char                arg1[MAX_INPUT_LENGTH];
    char                arg2[MAX_INPUT_LENGTH];
    CHAR_DATA          *target;
    CHAR_DATA          *victim;

    one_argument(argument, arg1);
    one_argument(argument, arg2);

    if (arg1[0] == '\0' || arg2[0] == '\0') {
        send_to_char("Syntax: fhunt <victim> <target/stop>.\n\r", ch);
        return;
    }

    if ((victim = get_char_world(ch, arg1)) == NULL) {
        send_to_char("Your victim is not here.\n\r", ch);
        return;
    }

    /* Do not force players to hunt. Can only force mobs */
    if (!IS_NPC(victim)) {
        send_to_char("You can't force a player character to hunt.\n\r", ch);
        return;
    }

    /* force a mob to stop hunting */
    if (str_cmp(arg2, "stop")) {
        if (victim->hunting != NULL) {
            sendf(ch, "%s stops hunting %s.\n\r", victim->short_descr, victim->hunting->short_descr);
            end_hunt(victim);
            return;
        }
        else {
            send_to_char("They aren't hunting anyone.\n\r", ch);
            return;
        }
    }

    if ((target = get_char_world(victim, arg2)) == NULL) {
        send_to_char("The new target to be hunted is not here.\n\r", ch);
        return;
    }

    /* By Now:
     * You can only force mobs to hunt.
     */

    /* if victim is currently in a group, leave group */
    /*   if (  ( victim->leader != NULL )
       || ( victim->master != NULL )  )
       do_follow ( victim, victim );  */

    /* once i put this skill in, remember to take out the brackets
     * -- do_abandon will kick everyone out of the victim's group if the victim
     * is the group leader -- or i guess the whole group can go hunting *shrug*
     *          - Uni */
    /* do_abandon ( victim, "all" ); */

    if (victim->hunting != NULL) {
        sendf(ch, "%s stops hunting %s.\n\r", victim->short_descr, victim->hunting->short_descr);
        end_hunt(victim);
    }

    victim->hunting = target;
    sendf(ch, "%s starts hunting %s.\n\r", victim->short_descr, victim->hunting->short_descr);

    return;

}

void
do_alink(CHAR_DATA *ch, char *argument)
{

    AREA_DATA          *this_area;
    ROOM_INDEX_DATA    *this_room;

    BUILD_DATA_LIST    *pointer;
    ROOM_INDEX_DATA    *current_room;
    int                 area_top, area_bottom;
    sh_int              doorway;

    this_room = ch->in_room;
    this_area = ch->in_room->area;
    area_top = this_area->max_vnum;
    area_bottom = this_area->min_vnum;
    sendf(ch, "External room links for %s.\n\r  THIS DOES NOT INCLUDE ONE WAY DOORS INTO THIS AREA.\n\r", this_area->name);

    for (pointer = this_area->first_area_room; pointer != NULL; pointer = pointer->next) {
        current_room = pointer->data;

        for (doorway = 0; doorway < 6; doorway++) {
            EXIT_DATA          *pexit;

            if (((pexit = current_room->exit[doorway]) == NULL)
                || (pexit->to_room == NULL)
                || ((pexit->to_room->vnum >= area_bottom)
                    && (pexit->to_room->vnum <= area_top)))
                continue;
            sendf(ch, "Room: %d linked to room: %d.\n\r", current_room->vnum, pexit->to_room->vnum);
        }
    }

    return;
}

void
do_imtlset(CHAR_DATA *ch, char *argument)
{

    CHAR_DATA          *rch;
    CHAR_DATA          *victim;
    char                arg1[MAX_INPUT_LENGTH];
    char                buf[MAX_STRING_LENGTH];
    char                buf1[MAX_STRING_LENGTH];
    char               *buf2;
    char               *buf3 = NULL;
    char               *skill;
    int                 cmd;
    int                 col = 0;
    int                 i = 0;

    rch = get_char(ch);

    if (!authorized(rch, "imtlset"))
        return;

    argument = one_argument(argument, arg1);

    if (arg1[0] == '\0') {
        send_to_char("Syntax: imtlset <victim> +|- <immortal skill>\n\r", ch);
        send_to_char("or:     imtlset <victim> +|- all\n\r", ch);
        send_to_char("or:     imtlset <victim>\n\r", ch);
        return;
    }

    if (!(victim = get_char_world(rch, arg1))) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("Not on NPC's.\n\r", ch);
        return;
    }

    if (get_trust(rch) <= get_trust(victim) && rch != victim) {
        send_to_char("You may not imtlset your peer nor your superior.\n\r", ch);
        return;
    }

    if ((rch == victim) && (rch->level != MAX_LEVEL)) {
        send_to_char("You may not set your own immortal skills.\n\r", ch);
        return;
    }

    if (argument[0] == '+' || argument[0] == '-') {
        buf[0] = '\0';
        smash_tilde(argument);
        if (argument[0] == '+') {
            argument++;
            if (!str_cmp("all", argument))
            {
                for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++) {
                    if (cmd_table[cmd].level > get_trust(rch))
                        continue;
                    if (cmd_table[cmd].level <= victim->level && cmd_table[cmd].level >= LEVEL_HERO) {
                        strcat(buf, cmd_table[cmd].name);
                        strcat(buf, " ");
                    }
                }
            }
            else {
                if (victim->pcdata->immskll)
                    strcat(buf, victim->pcdata->immskll);
                while (isspace(*argument))
                    argument++;
                for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++) {
                    if (cmd_table[cmd].level > get_trust(rch))
                        continue;
                    if (!str_cmp(argument, cmd_table[cmd].name))
                        break;
                }
                if (cmd_table[cmd].name[0] == '\0') {
                    send_to_char("That is not an immskill.\n\r", ch);
                    return;
                }
                if (!str_infix(argument, victim->pcdata->immskll)) {
                    send_to_char("That skill has already been set.\n\r", ch);
                    return;
                }
                strcat(buf, argument);
                strcat(buf, " ");
            }
        }

        if (argument[0] == '-') {
            argument++;
            one_argument(argument, arg1);
            if (!str_cmp("all", arg1)) {
                free_string(victim->pcdata->immskll);
                victim->pcdata->immskll = str_dup("");
                send_to_char("All immskills have been deleted.\n\r", ch);
                return;
            }
            else if (arg1) {
                /*
                 * Cool great imtlset <victim> - <skill> code...
                 * Idea from Canth (phule@xs4all.nl)
                 * Code by Vego (v942429@si.hhs.nl)
                 * Still needs memory improvements.... (I think)
                 */
                buf2 = str_dup(victim->pcdata->immskll);
                buf3 = buf2;
                if ((skill = strstr(buf2, arg1)) == NULL) {
                    send_to_char("That person doesn't have that immskill.\n\r", ch);
                    return;
                }
                else {
                    while (buf2 != skill)
                        buf[i++] = *(buf2++);
                    while (!isspace(*(buf2++)));
                    buf[i] = '\0';
                    strcat(buf, buf2);
                }
            }
            else {
                send_to_char("That's not an immskill\n\r", ch);
                return;
            }
        }

        free_string(buf3);
        skill = buf2 = buf3 = NULL;
        free_string(victim->pcdata->immskll);
        victim->pcdata->immskll = str_dup(buf);
    }

    sendf(ch, "Immortal skills set for %s:\n\r", victim->name);

    buf1[0] = '\0';
    for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++) {
        if (cmd_table[cmd].level < LEVEL_HERO || str_infix(cmd_table[cmd].name, victim->pcdata->immskll))
            continue;

        sprintf(buf, "%-10s", cmd_table[cmd].name);
        strcat(buf1, buf);
        if (++col % 8 == 0)
            strcat(buf1, "\n\r");
    }

    if (col % 8 != 0)
        strcat(buf1, "\n\r");
    send_to_char(buf1, ch);

    return;

}

void
do_gain_stat_reset(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA          *victim;
    OBJ_DATA           *wear_object;
    CHAR_DATA          *rch;

    rch = get_char(ch);

    if (!authorized(rch, "resetgain"))
        return;

    if (argument[0] == '\0') {
        send_to_char("Reset who's gain stats??\n\r", ch);
        return;
    }

    if ((victim = get_char_world(ch, argument)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("Not on NPCs!\n\r", ch);
        return;
    }

    reset_gain_stats(victim);

    victim->desc->connected = CON_SETTING_STATS;
    victim->hitroll = 0;
    victim->damroll = 0;
    victim->armor = 100;

    victim->max_mana = victim->pcdata->mana_from_gain;
    victim->max_hit = victim->pcdata->hp_from_gain;
    victim->max_move = victim->pcdata->move_from_gain;

    for (wear_object = victim->first_carry; wear_object != NULL; wear_object = wear_object->next_in_carry_list) {
        if (wear_object->wear_loc > WEAR_NONE)
            equip_char(victim, wear_object, wear_object->wear_loc);
    }

    victim->desc->connected = CON_PLAYING;

    send_to_char("Done!\n\r", ch);
    send_to_char("Your stats have been reset.\n\r", victim);

}

/* Expand the name of a character into a string that identifies THAT
   character within a room. E.g. the second 'guard' -> 2. guard */
const char         *
name_expand(CHAR_DATA *ch)
{
    int                 count = 1;
    CHAR_DATA          *rch;
    char                name[MAX_INPUT_LENGTH];    /*  HOPEFULLY no mob has a name longer than THAT */

    static char         outbuf[MAX_INPUT_LENGTH];

    if (!IS_NPC(ch))
        return ch->name;

    one_argument(ch->name, name);    /* copy the first word into name */

    if (!name[0]) {                /* weird mob .. no keywords */
        strcpy(outbuf, "");        /* Do not return NULL, just an empty buffer */
        return outbuf;
    }

    for (rch = ch->in_room->first_person; rch && (rch != ch); rch = rch->next_in_room)
        if (is_name(name, rch->name))
            count++;

    sprintf(outbuf, "%d.%s", count, name);
    return outbuf;
}

/*
 * For by Erwin S. Andreasen (4u2@aabc.dk)
 */
void
do_for(CHAR_DATA *ch, char *argument)
{
    char                range[MAX_INPUT_LENGTH];
    char                buf[MAX_STRING_LENGTH];
    bool                fGods = FALSE, fMortals = FALSE, fMobs = FALSE, fEverywhere = FALSE, found;
    ROOM_INDEX_DATA    *room, *old_room;
    CHAR_DATA          *p, *p_next;
    int                 i;

    extern bool         disable_timer_abort;

    if (!authorized(ch, "for"))
        return;

    disable_timer_abort = TRUE;

    argument = one_argument(argument, range);

    if (!range[0] || !argument[0]) {    /* invalid usage? */
        do_help(ch, "for");
        disable_timer_abort = FALSE;
        return;
    }

    if (!str_prefix("quit", argument)) {
        send_to_char("Are you trying to crash the MUD or something?\n\r", ch);
        disable_timer_abort = FALSE;
        return;
    }

    if (!str_cmp(range, "all")) {
        fMortals = TRUE;
        fGods = TRUE;
    }
    else if (!str_cmp(range, "gods"))
        fGods = TRUE;
    else if (!str_cmp(range, "mortals"))
        fMortals = TRUE;
    else if (!str_cmp(range, "mobs"))
        fMobs = TRUE;
    else if (!str_cmp(range, "everywhere"))
        fEverywhere = TRUE;
    else
        do_help(ch, "for");        /* show syntax */

    /* do not allow # to make it easier */
    if (fEverywhere && strchr(argument, '#')) {
        send_to_char("Cannot use FOR EVERYWHERE with the # thingie.\n\r", ch);
        disable_timer_abort = FALSE;
        return;
    }

    if (fMobs && strchr(argument, '#')) {
        send_to_char("Cannot use FOR MOBS with the # thingie.\n\r", ch);
        disable_timer_abort = FALSE;
        return;
    }

    if (strchr(argument, '#')) {    /* replace # ? */
        for (p = first_char; p; p = p_next) {
            p_next = p->next;    /* In case someone DOES try to AT MOBS SLAY # */
            found = FALSE;

            if (!(p->in_room) || room_is_private(p->in_room) || (p == ch))
                continue;

            if (IS_NPC(p) && fMobs)
                found = TRUE;
            else if (!IS_NPC(p) && p->level >= LEVEL_IMMORTAL && fGods)
                found = TRUE;
            else if (!IS_NPC(p) && p->level < LEVEL_IMMORTAL && fMortals)
                found = TRUE;

            /* It looks ugly to me.. but it works :) */
            if (found) {        /* p is 'appropriate' */
                char               *pSource = argument;    /* head of buffer to be parsed */
                char               *pDest = buf;    /* parse into this */

                while (*pSource) {
                    if (*pSource == '#') {    /* Replace # with name of target */
                        const char         *namebuf = name_expand(p);

                        if (namebuf)    /* in case there is no mob name ?? */
                            while (*namebuf)    /* copy name over */
                                *(pDest++) = *(namebuf++);

                        pSource++;
                    }
                    else
                        *(pDest++) = *(pSource++);
                }                /* while */
                *pDest = '\0';    /* Terminate */

                /* Execute */
                old_room = ch->in_room;
                char_from_room(ch);
                char_to_room(ch, p->in_room);
                interpret(ch, buf);
                char_from_room(ch);
                char_to_room(ch, old_room);

            }                    /* if found */
        }                        /* for every char */
    }
    else {                        /* just for every room with the appropriate people in it */

        for (i = 0; i < MAX_KEY_HASH; i++)    /* run through all the buckets */
            for (room = room_index_hash[i]; room; room = room->next) {
                found = FALSE;

                /* Anyone in here at all? */
                if (fEverywhere)    /* Everywhere executes always */
                    found = TRUE;
                else if (!room->first_person)    /* Skip it if room is empty */
                    continue;

                /* Check if there is anyone here of the requried type */
                /* Stop as soon as a match is found or there are no more ppl in room */
                for (p = room->first_person; p && !found; p = p->next_in_room) {

                    if (p == ch)    /* do not execute on oneself */
                        continue;

                    if (IS_NPC(p) && fMobs)
                        found = TRUE;
                    else if (!IS_NPC(p) && (p->level >= LEVEL_IMMORTAL) && fGods)
                        found = TRUE;
                    else if (!IS_NPC(p) && (p->level <= LEVEL_IMMORTAL) && fMortals)
                        found = TRUE;
                }                /* for everyone inside the room */

                if (found && !room_is_private(room)) {    /* Any of the required type here AND room not private? */
                    /* This may be ineffective. Consider moving character out of old_room
                       once at beginning of command then moving back at the end.
                       This however, is more safe?
                     */

                    old_room = ch->in_room;
                    char_from_room(ch);
                    char_to_room(ch, room);
                    interpret(ch, argument);
                    char_from_room(ch);
                    char_to_room(ch, old_room);
                }                /* if found */
            }                    /* for every room in a bucket */
    }                            /* if strchr */
    disable_timer_abort = FALSE;
}                                /* do_for */

void
do_otype(CHAR_DATA *ch, char *argument)
{
    extern int          top_obj_index;
    char                arg[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA     *pObjIndex;
    int                 vnum;
    int                 nMatch;
    bool                fAll;
    bool                found;

    one_argument(argument, arg);
    if (arg[0] == '\0') {
        send_to_char
            ("Otype <light scroll wand staff weapon beacon portal treasure armor potion clutch furniture trash trigger container quest drink_con key food money ",
            ch);
        send_to_char("boat corpse_npc corpse_pc fountain pill board soul piece matrix enchantment>\n\r", ch);
        return;
    }

    fAll = !str_cmp(arg, "all");
    found = FALSE;
    nMatch = 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_obj_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for (vnum = 0; nMatch < top_obj_index; vnum++) {
        if ((pObjIndex = get_obj_index(vnum)) != NULL) {
            nMatch++;
            if (fAll || is_name(arg, tab_item_types[(pObjIndex->item_type) - 1].text)) {
                found = TRUE;
                sendf(ch, "<%s> [%5d] %s\n\r", tab_item_types[(pObjIndex->item_type) - 1].text, pObjIndex->vnum, pObjIndex->short_descr);
            }
        }
    }

    if (!found) {
        send_to_char("Nothing like that in hell, earth, or heaven.\n\r", ch);
        return;
    }

    return;
}

int
owear_cmp(const void *x, const void *y)
{
    OBJ_INDEX_DATA     *objx = *(OBJ_INDEX_DATA **) x;
    OBJ_INDEX_DATA     *objy = *(OBJ_INDEX_DATA **) y;
    int                 lvlx, lvly;

    if (objx == NULL || objy == NULL)
        return 0;

    lvlx = objx->level;
    lvly = objy->level;

    if (IS_SET(objx->extra_flags, ITEM_ADEPT))
        lvlx += 120;

    if (IS_SET(objy->extra_flags, ITEM_ADEPT))
        lvly += 120;

    if (IS_SET(objx->extra_flags, ITEM_REMORT)) {
        lvlx /= 4;
        lvlx += 80;
    }

    if (IS_SET(objy->extra_flags, ITEM_REMORT)) {
        lvly /= 4;
        lvly += 80;
    }

    if (lvlx < lvly)
        return -1;

    if (lvlx == lvly)
        return 0;

    return 1;
}

void
do_owear(CHAR_DATA *ch, char *argument)
{
    extern int          top_obj_index;
    char                buf[MAX_STRING_LENGTH];
    char                arg[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA     *pObjIndex;
    void              **objsort;
    int                 vnum;
    int                 nMatch;
    int                 c = 0;
    int                 b = 0;
    bool                fAll;
    bool                found;
    bool                light = FALSE;

    one_argument(argument, arg);
    if (arg[0] == '\0') {
        send_to_char
            ("Owear <take light finger neck body head legs feet hands arms shield about waist wrist wield hold face ear clutch wield_2 clan_eq>\n\r", ch);
        return;
    }

    buf[0] = '\0';
    fAll = !str_cmp(arg, "all");
    found = FALSE;
    nMatch = 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_obj_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */

    if (!str_cmp(arg, "light")) {
        strcpy(arg, "hold");
        light = TRUE;
    }

    for (vnum = 0; nMatch < top_obj_index; vnum++) {
        if ((pObjIndex = get_obj_index(vnum)) != NULL) {
            nMatch++;
            if (fAll || !str_infix(arg, bit_table_lookup(tab_wear_flags, pObjIndex->wear_flags))) {
                if (!light || pObjIndex->item_type == ITEM_LIGHT) {
                    c++;

                    found = TRUE;
                }
            }
        }
    }

    if (!found) {
        send_to_char("Nothing like that in hell, earth, or heaven.\n\r", ch);
        return;
    }

    objsort = (void **) malloc(sizeof(OBJ_INDEX_DATA *) * c);
    memset(objsort, 0, sizeof(OBJ_INDEX_DATA *) * c);

    c = 0;
    nMatch = 0;
    for (vnum = 0; nMatch < top_obj_index; vnum++) {
        if ((pObjIndex = get_obj_index(vnum)) != NULL) {
            nMatch++;
            if (fAll || !str_infix(arg, bit_table_lookup(tab_wear_flags, pObjIndex->wear_flags))) {
                if (!light || pObjIndex->item_type == ITEM_LIGHT)
                    objsort[c++] = (void *) pObjIndex;
            }
        }
    }

    qsort(objsort, c, sizeof(void *), owear_cmp);

    for (b = 0; b < c; b++) {
        pObjIndex = objsort[b];

        sendf(ch, "[%5d] @@N[%s%3d @@N%s@@N] %s @@N@@g%s@@N\n\r",
            pObjIndex->vnum,
            (pObjIndex->level <= 80) ? "@@y" : "@@p",
            pObjIndex->level,
            ((IS_SET(pObjIndex->extra_flags, ITEM_ADEPT)) ? "@@W ADEPT@@N" :
                (IS_SET(pObjIndex->extra_flags, ITEM_REMORT) ? "@@mREMORT@@N" : "@@gMORTAL@@N")), my_left(pObjIndex->short_descr, buf, 25), format_eqindex_stats(pObjIndex));
    }

    free(objsort);
    return;
}

void
do_mlist(CHAR_DATA *ch, char *argument)
{
    int                 curvnum;
    char                buf[MAX_STRING_LENGTH];
    char                buf1[MAX_STRING_LENGTH * 10];
    MOB_INDEX_DATA     *pMobIndex;
    AREA_DATA          *area;
    bool                found = FALSE;

    if (IS_NPC(ch)) {
        send_to_char("You're a mob, go away!\n\r", ch);
        return;
    }
    if (ch->in_room == NULL) {
        send_to_char("Where the hell are you?\n\r", ch);
        return;
    }

    area = ch->in_room->area;

    if (!build_canread(area, ch, 1))
        return;

    sendf(ch, "Mobile list for area: %s [%d to %d], owned by %s\n\r\n\r", area->name, area->min_vnum, area->max_vnum, area->owner);

    sprintf(buf1, "%s", "");

    for (curvnum = area->min_vnum; curvnum <= area->max_vnum; curvnum++) {
        if ((pMobIndex = get_mob_index(curvnum)) != NULL) {
            found = TRUE;
            sprintf(buf, "[%5d] %s\n\r", pMobIndex->vnum, pMobIndex->short_descr);
            safe_strcat(MAX_STRING_LENGTH * 10, buf1, buf);
        }
    }

    if (!found) {
        send_to_char("This area doesn't have any mobiles.\n\r", ch);
        return;
    }

    send_to_char(buf1, ch);
    return;
}

void
do_olist(CHAR_DATA *ch, char *argument)
{
    int                 curvnum;
    char                buf[MAX_STRING_LENGTH];
    char                buf1[MAX_STRING_LENGTH * 10];
    char                lbuf[MSL];

    OBJ_INDEX_DATA     *pObjIndex;
    AREA_DATA          *area;
    bool                found = FALSE;

    if (IS_NPC(ch)) {
        send_to_char("You're a mob, go away!\n\r", ch);
        return;
    }
    if (ch->in_room == NULL) {
        send_to_char("Where the hell are you?\n\r", ch);
        return;
    }

    area = ch->in_room->area;

    if (!build_canread(area, ch, 1))
        return;

    sendf(ch, "Object list for area: %s [%d to %d], owned by %s\n\r\n\r", area->name, area->min_vnum, area->max_vnum, area->owner);

    sprintf(buf1, "%s", "");

    for (curvnum = area->min_vnum; curvnum <= area->max_vnum; curvnum++) {
        if ((pObjIndex = get_obj_index(curvnum)) != NULL) {
            found = TRUE;

            if (pObjIndex->item_type != ITEM_PIECE)
                sprintf(buf, "@@d[@@g%5d@@d]@@N %s @@N@@g%s@@N\n\r",
                    pObjIndex->vnum, my_left(pObjIndex->short_descr, lbuf, 30), format_eqindex_stats(pObjIndex));
            else {
                OBJ_INDEX_DATA *pObjIndex1, *pObjIndex2, *pObjIndex3;

                pObjIndex1 = get_obj_index(pObjIndex->value[0]);
                pObjIndex2 = get_obj_index(pObjIndex->value[1]);
                pObjIndex3 = get_obj_index(pObjIndex->value[2]);

                if      (pObjIndex1 && pObjIndex2 && pObjIndex3 && pObjIndex1 != pObjIndex2)
                    sprintf(buf, "@@d[@@g%5d@@d]@@N %s @@N@@g%s @@N(%s@@N(%d) or %s@@N(%d) -> %s@@N(%d))\n\r", pObjIndex->vnum, my_left(pObjIndex->short_descr, lbuf, 30), format_eqindex_stats(pObjIndex), pObjIndex1->short_descr, pObjIndex1->vnum, pObjIndex2->short_descr, pObjIndex2->vnum, pObjIndex3->short_descr, pObjIndex3->vnum);
                else if (pObjIndex1 && pObjIndex3)
                    sprintf(buf, "@@d[@@g%5d@@d]@@N %s @@N@@g%s @@N(%s@@N(%d) -> %s@@N(%d))\n\r", pObjIndex->vnum, my_left(pObjIndex->short_descr, lbuf, 30), format_eqindex_stats(pObjIndex), pObjIndex1->short_descr, pObjIndex1->vnum, pObjIndex3->short_descr, pObjIndex3->vnum);
                else if (pObjIndex2 && pObjIndex3)
                    sprintf(buf, "@@d[@@g%5d@@d]@@N %s @@N@@g%s @@N(%s@@N(%d) -> %s@@N(%d))\n\r", pObjIndex->vnum, my_left(pObjIndex->short_descr, lbuf, 30), format_eqindex_stats(pObjIndex), pObjIndex2->short_descr, pObjIndex2->vnum, pObjIndex3->short_descr, pObjIndex3->vnum);
                else if (pObjIndex3)
                    sprintf(buf, "@@d[@@g%5d@@d]@@N %s @@N@@g%s @@N(@@eINVALID@@N -> %s@@N(%d))\n\r", pObjIndex->vnum, my_left(pObjIndex->short_descr, lbuf, 30), format_eqindex_stats(pObjIndex), pObjIndex3->short_descr, pObjIndex3->vnum);
                else
                    sprintf(buf, "@@d[@@g%5d@@d]@@N %s @@N@@g%s @@N(@@eINVALID@@N -> @@eINVALID@@N)\n\r", pObjIndex->vnum, my_left(pObjIndex->short_descr, lbuf, 30), format_eqindex_stats(pObjIndex));
            }

            safe_strcat(MAX_STRING_LENGTH * 10, buf1, buf);
        }
    }

    if (!found) {
        send_to_char("This area doesn't have any objects.\n\r", ch);
        return;
    }

    send_to_char(buf1, ch);
    return;
}

void
do_oreset(CHAR_DATA *ch, char *argument)
{
    char                buf[MSL], buf2[MSL], buf3[MSL];
    int                 vnum = -1, mobvnum = 0, roomvnum = 0;
    AREA_DATA          *area, *myarea;
    RESET_DATA         *reset;
    MOB_INDEX_DATA     *mob;
    OBJ_INDEX_DATA     *obj;
    ROOM_INDEX_DATA    *room;
    bool                corpse = FALSE, sold = FALSE;

    if (*argument == '\0') {
        send_to_char("syntax: oreset <vnum|keyword|thisarea|corpse|sold>\n\r", ch);
        return;
    }

    myarea = NULL;

    if (is_number(argument))
        if ((vnum = atoi(argument)) < 1 || vnum > MAX_VNUM) {
            send_to_char("Invalid vnum.\n\r", ch);
            return;
        }

    if (!str_cmp(argument, "thisarea")) {
        if (ch->in_room == NULL)
            return;

        myarea = ch->in_room->area;
        vnum = -1;
    }
    else if (!str_cmp(argument, "corpse"))
        corpse = TRUE;
    else if (!str_cmp(argument, "sold"))
        sold = TRUE;

    for (area = first_area; area != NULL; area = area->next) {
        mobvnum = 0;
        roomvnum = 0;

        if (myarea && area != myarea)
            continue;

        for (reset = area->first_reset; reset != NULL; reset = reset->next) {
            switch (reset->command) {
                case 'M':
                    mobvnum = reset->arg1;
                    roomvnum = reset->arg3;
                    break;
                case 'E':
                case 'G':
                case 'C':
                    if (!mobvnum)
                        break;

                    if (!(mob = get_mob_index(mobvnum))) {
                        mobvnum = 0;
                        break;
                    }

                    if (vnum != -1 && reset->arg1 != vnum)
                        break;

                    if (!(room = get_room_index(roomvnum)))
                        break;

                    if (!(obj = get_obj_index(reset->arg1)))
                        break;

                    if (corpse && reset->command != 'C')
                        break;

                    if (sold && mob->pShop == NULL)
                        break;

                    if (vnum == -1 && !myarea && !corpse && !sold && !is_name(argument, obj->name))
                        break;

                    if (reset->command == 'E')
                        sprintf(buf, "[%5d] @@y  WORN@@g %s @@N@@g[%5d]\n\r", room->vnum, my_left(mob->short_descr, buf2, 20), mob->vnum);
                    else if (reset->command == 'C')
                        sprintf(buf, "[%5d] @@eCORPSE@@g %s @@N@@g[%5d] [%5d]\n\r",
                            room->vnum, my_left(mob->short_descr, buf2, 20), mob->vnum, reset->arg3);
                    else if (mob->pShop == NULL)
                        sprintf(buf, "[%5d] @@a GIVEN@@g %s @@N@@g[%5d]\n\r", room->vnum, my_left(mob->short_descr, buf2, 20), mob->vnum);
                    else
                        sprintf(buf, "[%5d] @@p  SOLD@@g %s @@N@@g[%5d]\n\r", room->vnum, my_left(mob->short_descr, buf2, 20), mob->vnum);

                    if (vnum == -1) {
                        sendf(ch, "[%5d] %s @@N@@g", obj->vnum, my_left(obj->short_descr, buf3, 20));
                    }

                    send_to_char(buf, ch);
                    break;
                case 'O':
                    if (vnum != -1 && reset->arg1 != vnum)
                        break;

                    if (!(obj = get_obj_index(reset->arg1)))
                        break;

                    if (corpse)
                        break;

                    if (vnum == -1 && !myarea && !is_name(argument, obj->name))
                        break;

                    if (!(room = get_room_index(reset->arg3)))
                        break;

                    sprintf(buf, "[%5d] @@c FLOOR@@g                      [%5d]\n\r", room->vnum, reset->arg2);

                    if (vnum == -1) {
                        sendf(ch, "[%5d] %s @@N@@g", obj->vnum, my_left(obj->short_descr, buf3, 20));
                    }

                    send_to_char(buf, ch);
                    break;
            }
        }
    }

    return;
}

/*
 * START: shelp addition
 */

void
do_shedit(CHAR_DATA *ch, char *argument)
{
    char                cmd[MAX_INPUT_LENGTH], shelp[MAX_INPUT_LENGTH];
    bool                found = FALSE;
    int                 sn;
    SHELP_DATA         *findit;

    smash_tilde(argument);
    argument = one_argument(argument, shelp);
    argument = one_argument(argument, cmd);

    if (IS_NPC(ch)) {
        send_to_char("Not for NPCs.\n\r", ch);
        return;
    }

    if (!shelp[0]) {
        send_to_char("What shelp do you want to operate on?\n\r", ch);
        return;
    }

    if (!cmd[0]) {
        send_to_char("SHEDIT syntax:\n\rshedit <skill/spell> <new|delete|show|duration|modify|type|target|desc> [parms]\n\r", ch);
        return;
    }

    if ((sn = skill_lookup(shelp)) < 0) {
        send_to_char("No such spell/skill exists.\n\r", ch);
        return;
    }

    for (findit = first_shelp; findit != NULL; findit = findit->next)
        if (!str_cmp(skill_table[sn].name, findit->name)) {
            found = TRUE;
            break;
        }

    if (str_cmp(cmd, "new") && !found) {
        send_to_char("No such spell/skill exists.\n\r", ch);
        return;
    }

    if (!str_cmp(cmd, "delete")) {
        UNLINK(findit, first_shelp, last_shelp, next, prev);
        free_string(findit->name);
        free_string(findit->duration);
        free_string(findit->modify);
        free_string(findit->type);
        free_string(findit->target);
        free_string(findit->desc);
        PUT_FREE(findit, shelp_free);

        send_to_char("SHELP deleted.\n\r", ch);
        save_shelps();
        return;
    }

    else if (!str_cmp(cmd, "new")) {
        SHELP_DATA         *myshelp;

        if (found) {
            send_to_char("An shelp with that name already exists.\n\r", ch);
            return;
        }

        if (str_cmp(argument, skill_table[sn].name)) {
            send_to_char("Argument does not match skill/spell.\n\r", ch);
            return;
        }

        GET_FREE(myshelp, shelp_free);
        myshelp->name = str_dup(argument);
        myshelp->duration = str_dup("n/a");
        myshelp->type = str_dup("n/a");
        myshelp->modify = str_dup("n/a");
        myshelp->target = str_dup("n/a");
        myshelp->desc = str_dup("n/a");
        LINK(myshelp, first_shelp, last_shelp, next, prev);
        send_to_char("SHELP added.\n\r", ch);
        save_shelps();
        return;
    }

    else if (!str_cmp(cmd, "show")) {    /* Allow it, but refer it to do_shelp */
        do_shelp(ch, shelp);
        return;
    }

    else if (!str_cmp(cmd, "duration")) {
        free_string(findit->duration);
        findit->duration = str_dup(argument);
    }
    else if (!str_cmp(cmd, "modify")) {
        free_string(findit->modify);
        findit->modify = str_dup(argument);
    }
    else if (!str_cmp(cmd, "type")) {
        free_string(findit->type);
        findit->type = str_dup(argument);
    }
    else if (!str_cmp(cmd, "target")) {
        free_string(findit->target);
        findit->target = str_dup(argument);
    }
    else if (!str_cmp(cmd, "desc")) {
        free_string(findit->desc);
        findit->desc = str_dup(argument);
    }
    else {
        do_shedit(ch, "");
        return;
    }

    send_to_char("Done.\n\r", ch);
    save_shelps();
}

/*
 * FINISH: shelp addition
 */

void
do_areasave(CHAR_DATA *ch, char *argument)
{
    AREA_DATA          *pArea;
    sh_int              start = 1;
    sh_int              cnt = 0;
    sh_int              found = 0;

    if (argument[0] != '\0' && is_number(argument))
        start = atoi(argument);

    for (pArea = first_area; pArea != NULL; pArea = pArea->next) {
        if (++cnt >= start && cnt <= start + 50) {
            found++;
            area_modified(pArea);
        }
    }

    if (!found)
        send_to_char("Out of range.\n\r", ch);
    else {
        sendf(ch, "Saving %d areas, starting at area #%d.\n\r", found, start);
    }

    return;
}

void
do_ctalk(CHAR_DATA *ch, char *argument)
{
    char                arg[MSL];
    int                 clan = 0;
    int                 oldclan;

    argument = one_argument(argument, arg);

    if (arg[0] == '\0' || *argument == '\0' || !is_number(arg) || IS_NPC(ch)) {
        send_to_char("syntax: ctalk <clan number> <text>\n\r", ch);
        return;
    }

    clan = atoi(arg);
    oldclan = ch->pcdata->clan;

    if (clan <= 0 || clan >= MAX_CLAN) {
        send_to_char("No such clan exists.\n\r", ch);
        return;
    }

    ch->pcdata->clan = clan;
    do_clan(ch, argument);
    ch->pcdata->clan = oldclan;

    return;
}

void
do_atalk(CHAR_DATA *ch, char *argument)
{   
    char                arg[MSL];
    int                 clan = 0;
    int                 oldclan;

    argument = one_argument(argument, arg);

    if (arg[0] == '\0' || *argument == '\0' || !is_number(arg) || IS_NPC(ch)) {
        send_to_char("syntax: atalk <clan number> <text>\n\r", ch);
        return;
    }

    clan = atoi(arg);
    oldclan = ch->pcdata->clan;

    if (clan <= 0 || clan >= MAX_CLAN) {
        send_to_char("No such clan exists.\n\r", ch);
        return;
    }

    ch->pcdata->clan = clan;
    do_ally(ch, argument);
    ch->pcdata->clan = oldclan;

    return;
}

void
do_maffect(CHAR_DATA *ch, char *argument)
{
    char                arg1[MAX_INPUT_LENGTH];
    char                arg2[MAX_INPUT_LENGTH];
    char                arg3[MAX_INPUT_LENGTH];
    char                arg4[MAX_INPUT_LENGTH];
    char                arg5[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim = NULL;
    int                 sn = -1;
    int                 position = 0;
    int                 amount = 0;
    int                 duration = 0;
    bool                savable = FALSE;
    bool                fAll = FALSE;
    AFFECT_DATA         af;

    arg1[0] = 0;
    arg2[0] = 0;
    arg3[0] = 0;
    arg4[0] = 0;
    arg5[0] = 0;
    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);
    argument = one_argument(argument, arg4);
    argument = one_argument(argument, arg5);

    if (argument == NULL || *argument == '\0') {
        send_to_char("syntax: maffect <victim|all> <spell> <position> <amount> <duration> <savable>\n\r", ch);
        return;
    }

    if (!str_cmp("all", arg1))
        fAll = TRUE;
    else if ((victim = get_char_world(ch, arg1)) == NULL) {
        send_to_char("maffect Who?\n\r", ch);
        return;
    }

    if ((sn = skill_lookup(arg2)) == -1) {
        send_to_char("Invalid spell/skill.\n\r", ch);
        return;
    }

    if ((position = table_lookup(tab_obj_aff, arg3)) == 0) {
        char                buf[MSL];

        send_to_char("Invalid position type, valid positions are:\n\r\n\r", ch);

        table_printout(tab_obj_aff, buf);
        send_to_char(buf, ch);

        return;
    }

    amount = atoi(arg4);
    duration = atoi(arg5);
    savable = atoi(argument);
    savable = (savable) ? TRUE : FALSE;

    af.type = sn;
    af.duration = duration;
    af.modifier = amount;
    af.location = position;
    af.bitvector = 0;
    af.save = savable;

    if (!fAll)
        affect_to_char(victim, &af);
    else
        for (victim = first_player; victim != NULL; victim = victim->next_player)
            if (!IS_IMMORTAL(victim))
                affect_to_char(victim, &af);

    return;
}

void
do_nocmd(CHAR_DATA *ch, char *argument)
{
    extern char    *global_nocmd;

    if (!argument || argument[0] == '\0') {
        if (!global_nocmd || global_nocmd[0] == '\0')
            send_to_char("syntax: nocmd <list of commands>\n\r", ch);
        else
            sendf(ch, "@@gDisallowed commands: @@y%s@@N\n\r", global_nocmd);

        return;
    }

    if (!str_cmp("clear", argument)) {
        if (global_nocmd) free_string(global_nocmd);
        global_nocmd = str_dup("");

        send_to_char("Disallowed commands cleared.\n\r", ch);
        save_mudsets();
        return;
    }

    if (global_nocmd) free_string(global_nocmd);
    global_nocmd = str_dup(argument);
    save_mudsets();
    return;
}

void
do_nospell(CHAR_DATA *ch, char *argument)
{
    extern char    *global_nospell;

    if (!argument || argument[0] == '\0') {
        if (!global_nospell || global_nospell[0] == '\0')
            send_to_char("syntax: nospell <list of commands>\n\r", ch);
        else
            sendf(ch, "@@gDisallowed spells: @@y%s@@N\n\r", global_nospell);

        return;
    }

    if (!str_cmp("clear", argument)) {
        if (global_nospell) free_string(global_nospell);
        global_nospell = str_dup("");

        send_to_char("Disallowed spells cleared.\n\r", ch);
        save_mudsets();
        return;
    }

    if (global_nospell) free_string(global_nospell);
    global_nospell = str_dup(argument);
    save_mudsets();
    return;
}

#define RFLAG(type, key) do { if (b & type) safe_strcat(MSL, buf, key); else if (spaced) safe_strcat(MSL, buf, " "); } while (0)

char *get_room_flags_short(ROOM_INDEX_DATA *room, bool spaced)
{
    static char buf[MSL];
    int b = room->room_flags;

    buf[0] = '[';
    buf[1] = '\0';

    RFLAG(ROOM_DARK,        "D");
    RFLAG(ROOM_REGEN,       "R");
    RFLAG(ROOM_NO_MOB,      "M");
    RFLAG(ROOM_INDOORS,     "I");
    RFLAG(ROOM_NO_MAGIC,    "A");
    RFLAG(ROOM_HOT,         "H");
    RFLAG(ROOM_COLD,        "C");
    RFLAG(ROOM_PK,          "P");
    RFLAG(ROOM_QUIET,       "Q");
    RFLAG(ROOM_PRIVATE,     "V");
    RFLAG(ROOM_SAFE,        "S");
    RFLAG(ROOM_SOLITARY,    "O");
    RFLAG(ROOM_PET_SHOP,    "E");
    RFLAG(ROOM_NO_RECALL,   "L");
    RFLAG(ROOM_NO_TELEPORT, "T");
    RFLAG(ROOM_HUNT_MARK,   "U");
    RFLAG(ROOM_NO_PORTAL,   "p");
    RFLAG(ROOM_NO_REPOP,    "r");
    RFLAG(ROOM_NO_QUIT,     "q");
    RFLAG(ROOM_ANTI_PORTAL, "a");

    if (buf[1] == '\0')
        buf[0] = '\0';
    else
        safe_strcat(MSL, buf, "]");

    return buf;
}

#undef RFLAG

char
get_room_sector_short(ROOM_INDEX_DATA *room)
{
    switch (room->sector_type) {
        case SECT_INSIDE:       return 'I';
        case SECT_CITY:         return 'C';
        case SECT_FIELD:        return 'F';
        case SECT_FOREST:       return 'O';
        case SECT_HILLS:        return 'H';
        case SECT_MOUNTAIN:     return 'M';
        case SECT_WATER_SWIM:   return 'w';
        case SECT_WATER_NOSWIM: return 'W';
        case SECT_RECALL_OK:    return 'R';
        case SECT_AIR:          return 'A';
        case SECT_DESERT:       return 'D';
        case SECT_MAX:          return 'X';
        default: return '?';
    }

    return '?';
}

void
do_roomlist(CHAR_DATA *ch, char *argument)
{
    int                 curvnum;
    char                buf[MAX_STRING_LENGTH];
    char                buf1[MAX_STRING_LENGTH * 10];
    char                lbuf[MSL];
    bool                spaced = FALSE;
    ROOM_INDEX_DATA    *pRoomIndex;
    AREA_DATA          *area;
    bool                found = FALSE;

    if (IS_NPC(ch)) {
        send_to_char("You're a mob, go away!\n\r", ch);
        return;
    }

    if (ch->in_room == NULL) {
        send_to_char("Where the hell are you?\n\r", ch);
        return;
    }

    area = ch->in_room->area;

    if (!build_canread(area, ch, 1))
        return;

    if (argument[0] != '\0')
        spaced = TRUE;

    sendf(ch, "Room list for area: %s [%d to %d], owned by %s\n\r\n\r", area->name, area->min_vnum, area->max_vnum, area->owner);

    sprintf(buf1, "%s", "");

    for (curvnum = area->min_vnum; curvnum <= area->max_vnum; curvnum++) {
        if ((pRoomIndex = get_room_index(curvnum)) != NULL) {
            found = TRUE;
            sprintf(buf, "@@d[@@g%5d@@d]@@N %s @@N@@g(%c) %s@@N\n\r",
                curvnum,
                my_left(pRoomIndex->name, lbuf, 30),
                get_room_sector_short(pRoomIndex),
                get_room_flags_short(pRoomIndex, spaced));

            safe_strcat(MAX_STRING_LENGTH * 10, buf1, buf);
        }
    }

    if (!found) {
        send_to_char("This area doesn't have any rooms.\n\r", ch);
        return;
    }

    send_to_char(buf1, ch);
    return;
}

void
do_mudset(CHAR_DATA *ch, char *argument)
{
    char    arg[MIL];
    int     cnt;
    bool    found = FALSE;
    bool           *type_bool;
    unsigned int   *type_uint;
    char          **type_string;
    
    argument = one_argument(argument, arg);

    for (cnt = 0; mudset_table[cnt].name[0] != '\0'; cnt++) {
        if (arg[0] != '\0' && !str_cmp(mudset_table[cnt].name, arg)) {
            found = TRUE;
            break;
        }
    }

    if (!found || !argument) {
        send_to_char("syntax: mudset <name> <value>\n\r\n\r", ch);

        for (cnt = 0; mudset_table[cnt].name[0] != '\0'; cnt++) {
            char buf[MSL];

            switch (mudset_table[cnt].type) {
                case MUDSET_TYPE_BOOL:
                    type_bool = (bool *)mudset_table[cnt].var;
                    sprintf(buf, "%12s %s\n\r", mudset_table[cnt].name, (*type_bool) ? "enabled" : "disabled");
                    send_to_char(buf, ch);
                    break;
                case MUDSET_TYPE_INT:
                    type_uint = (unsigned int *)mudset_table[cnt].var;
                    sprintf(buf, "%12s %u\n\r", mudset_table[cnt].name, *type_uint);
                    send_to_char(buf, ch);
                    break;
                case MUDSET_TYPE_STRING:
                    type_string = (char **)mudset_table[cnt].var;
                    sprintf(buf, "%12s %s\n\r", mudset_table[cnt].name, *type_string ? *type_string : "");
                    send_to_char(buf, ch);
                    break;
                default:
                    break;
            }
        }

        return;
    }

    switch (mudset_table[cnt].type) {
        case MUDSET_TYPE_BOOL:
            type_bool = (bool *)mudset_table[cnt].var;

            if (is_name(argument, "true 1 yes enable on"))
                *type_bool = TRUE;
            else if (is_name(argument, "false 0 no disable off"))
                *type_bool = FALSE;
            else {
                send_to_char("Invalid value.\n\r", ch);
                return;
            }

            break;

        case MUDSET_TYPE_INT:
            type_uint = (unsigned int *)mudset_table[cnt].var;

            *type_uint = (unsigned int)strtol(argument, (char **)NULL, 10);
            break;

        case MUDSET_TYPE_STRING:
            type_string = (char **)mudset_table[cnt].var;

            free_string(*type_string);
            *type_string = str_dup(argument);
            break;
        default:
            break;
    }

    save_mudsets();
    send_to_char("Mudset data saved.\n\r", ch);
    return;
}

void do_irename(CHAR_DATA *ch, char *argument)
{
    RENAME_DATA *rename;
    char arg[MIL], arg2[MIL];
    int cnt = 0;
    int num = 0;
    bool found = FALSE;

    argument = one_argument(argument, arg);

    if (arg[0] == '\0') {
        do_wizhelp(ch, "irename");
        return;
    }

    if (!str_cmp(arg, "list")) {
        for (rename = first_rename; rename != NULL; rename = rename->next) {
            found = TRUE;
            sendf(ch, "%2d: %s wants to rename %s to %s (long: %s) (keywords: %s)\n\r",
                ++cnt, rename->playername, rename->oldshort, rename->newshort, rename->newlong, rename->newkeyword);
        }

        if (!found)
            send_to_char("No renames are awaiting acceptance.\n\r", ch);

        return;
    }

    if (!str_cmp(arg, "accept")) {
        CHAR_DATA *player, *victim;
        OBJ_DATA *obj;
        bool online = FALSE;
        DESCRIPTOR_DATA d;
        char buf[MIL];

        if (*argument == '\0') {
            send_to_char("syntax: irename accept <number>\n\r", ch);
            return;
        }

        num = abs(atoi(argument));

        for (rename = first_rename; rename != NULL; rename = rename->next)
            if (++cnt == num)
                break;

        if (!rename) {
            send_to_char("Can't find rename.\n\r", ch);
            return;
        }

        for (player = first_player; player != NULL; player = player->next_player)
            if (!str_cmp(player->pcdata->origname, rename->playername)) {
                online = TRUE;
                break;
            }

        if (!online) {
            found = load_char_obj(&d, rename->playername, TRUE);

            if (!found) {
                sendf(ch, "No pFile found for '%s'.\n\r", rename->playername);
                free_char(d.character);
                return;
            }

            victim = d.character;
            d.character = NULL;
            victim->desc = NULL;
            LINK(victim, first_char, last_char, next, prev);
            LINK(victim, first_player, last_player, next_player, prev_player);

            if (victim->in_room != NULL)
                char_to_room(victim, victim->in_room);
            else
                char_to_room(victim, get_room_index(2));
        }
        else
            victim = player;

        for (obj = victim->first_carry; obj != NULL; obj = obj->next_in_carry_list)
            if (obj->id == rename->id)
                break;

        if (!obj) {
            send_to_char("Unable to find object on character.\n\r", ch);

            if (!online)
                do_quit(victim, "NOSAVECHECK");
            else
                sendf(victim, "@@N%s@@N@@g was going to be renamed, except you don't have it any more!@@N\n\r", rename->oldshort);

            UNLINK(rename, first_rename, last_rename, next, prev);
            free_string(rename->playername);
            free_string(rename->oldshort);
            free_string(rename->oldlong);
            free_string(rename->oldkeyword);
            free_string(rename->newshort);
            free_string(rename->newlong);
            free_string(rename->newkeyword);
            PUT_FREE(rename, rename_free);

            save_renames();
            return;
        }

        free_string(obj->short_descr);
        free_string(obj->description);
        free_string(obj->name);

        obj->short_descr = str_dup(rename->newshort);

        sprintf(buf, "%s\n\r", rename->newlong);
        obj->description = str_dup(buf);
        obj->name        = str_dup(rename->newkeyword);

        victim->quest_points -= 15;

        /* possibly add note notification here for offline players */
        if (!online)
            do_quit(victim, "NOSAVECHECK");
        else {
            sendf(victim, "Your object @@N%s@@N@@g has been renamed to @@N%s@@N@@g! (keyword: %s)@@N\n\r",
                rename->oldshort, rename->newshort, rename->newkeyword);
            save_char_obj(victim);
        }

        UNLINK(rename, first_rename, last_rename, next, prev);
        free_string(rename->playername);
        free_string(rename->oldshort);
        free_string(rename->oldlong);
        free_string(rename->oldkeyword);
        free_string(rename->newshort);
        free_string(rename->newlong);
        free_string(rename->newkeyword);
        PUT_FREE(rename, rename_free);

        save_renames();
        return;
    }

    if (!str_cmp(arg, "reject")) {
        CHAR_DATA *player;
        bool online = FALSE;

        argument = one_argument(argument, arg2);

        if (arg2[0] == '\0') {
            send_to_char("syntax: irename reject <number> [reason]\n\r", ch);
            return;
        }

        num = abs(atoi(arg2));

        for (rename = first_rename; rename != NULL; rename = rename->next)
            if (++cnt == num)
                break;

        if (!rename) {
            send_to_char("Can't find rename.\n\r", ch);
            return;
        }

        for (player = first_player; player != NULL; player = player->next_player)
            if (!str_cmp(player->pcdata->origname, rename->playername)) {
                online = TRUE;
                break;
            }

        if (online) {
            sendf(player, "@@N@@gYour rename of @@N%s@@N@@g to @@N%s@@N@@g was rejected.", rename->oldshort, rename->newshort);

            if (*argument != '\0')
                sendf(player, " Reason: @@y%s@@N@@g.@@N\n\r", argument);
            else
                sendf(player, "@@N\n\r");
        }
        /* TODO: add offline rename rejection notification */

        UNLINK(rename, first_rename, last_rename, next, prev);
        free_string(rename->playername);
        free_string(rename->oldshort);
        free_string(rename->oldlong);
        free_string(rename->oldkeyword);
        free_string(rename->newshort);
        free_string(rename->newlong);
        free_string(rename->newkeyword);
        PUT_FREE(rename, rename_free);

        save_renames();

        return;
    }

    do_wizhelp(ch, "irename");
    return;
}

void do_deimm(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
        return;

    ch->pcdata->deimm = TRUE;
    interpret(ch, argument);
    ch->pcdata->deimm = FALSE;
    return;
}

int
ocount_cmp(const void *x, const void *y)
{
    OBJ_INDEX_DATA     *objx = *(OBJ_INDEX_DATA **) x;
    OBJ_INDEX_DATA     *objy = *(OBJ_INDEX_DATA **) y;

    if (objx == NULL || objy == NULL)
        return 0;

    if (objx->count < objy->count)
        return 1;

    if (objx->count == objy->count)
        return 0;

    return -1;
}

void
do_ocount(CHAR_DATA *ch, char *argument)
{
    extern int          top_obj_index;
    char                buf[MAX_STRING_LENGTH];
    OBJ_INDEX_DATA     *pObjIndex;
    void              **objsort;
    int                 vnum;
    int                 nMatch;
    int                 c = 0;
    int                 b = 0;
    int                 limit = 0;
    bool                found;

    buf[0] = '\0';
    found = FALSE;
    nMatch = 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_obj_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */

    for (vnum = 0; nMatch < top_obj_index; vnum++) {
        if ((pObjIndex = get_obj_index(vnum)) != NULL) {
            nMatch++;
            c++;
            found = TRUE;
        }
    }

    if (!found) {
        send_to_char("Nothing like that in hell, earth, or heaven.\n\r", ch);
        return;
    }

    objsort = (void **) malloc(sizeof(OBJ_INDEX_DATA *) * c);
    memset(objsort, 0, sizeof(OBJ_INDEX_DATA *) * c);

    c = 0;
    nMatch = 0;
    for (vnum = 0; nMatch < top_obj_index; vnum++) {
        if ((pObjIndex = get_obj_index(vnum)) != NULL) {
            nMatch++;
            objsort[c++] = (void *) pObjIndex;
        }
    }

    qsort(objsort, c, sizeof(void *), ocount_cmp);

    for (b = 0; b < c; b++) {
        pObjIndex = objsort[b];
        limit++;

        if (limit > 50)
            break;

        sendf(ch, "[%5d] [%5d] @@N%s@@N\n\r", pObjIndex->vnum, pObjIndex->count, pObjIndex->short_descr);
    }

    free(objsort);
    return;
}

void
do_orare(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    bool                found = FALSE;
    OBJ_DATA           *obj;
    OBJ_DATA           *in_obj;
    int                 obj_counter = 1;
    extern OBJ_DATA    *auction_item;
    extern OBJ_DATA    *quest_object;

    for (obj = first_obj; obj != NULL; obj = obj->next) {
        if (obj == auction_item || obj == quest_object)
            continue;

        if (!IS_SET(obj->extra_flags, ITEM_RARE))
            continue;

        for (in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj);

        if (in_obj->carried_by != NULL && IS_NPC(in_obj->carried_by) && in_obj->carried_by->master == NULL) {
            sprintf(buf, "[%2d] %s carried by %s [Room:%d].\n\r",
                obj_counter, obj->short_descr, PERS(in_obj->carried_by, ch), in_obj->carried_by->in_room->vnum);
        }
        else if (in_obj->in_room != NULL) {
            sprintf(buf, "[%2d] %s in %s [Room:%d].\n\r", obj_counter, obj->short_descr, in_obj->in_room->name, in_obj->in_room->vnum);
        }
        else
            continue;

        if (obj->pIndexData && (obj->pIndexData->rarity == 0 || obj->pIndexData->rarity > 250)) {
            found = TRUE;
            obj_counter++;
            send_to_char(buf, ch);
        }
    }

    if (!found)
        send_to_char("Nothing like that in hell, earth, or heaven.\n\r", ch);

    return;

}

void
do_opotion(CHAR_DATA *ch, char *argument)
{
    bool                found = FALSE, ofound = FALSE;
    AREA_DATA          *area;
    BUILD_DATA_LIST    *obj;
    OBJ_INDEX_DATA     *pobj;
    int                 lvl, sn1, sn2, sn3;
    char                *sn1name, *sn2name, *sn3name;
    char                buf[MSL];

    for (area = first_area; area != NULL; area = area->next) {
        for (obj = area->first_area_object; obj != NULL; obj = obj->next) {
            pobj = obj->data;

            ofound = FALSE;

            switch (pobj->item_type) {
                case ITEM_SCROLL:
                case ITEM_WAND:
                case ITEM_STAFF:
                case ITEM_POTION:
                case ITEM_PILL:
                    ofound = TRUE;
                    found = TRUE;
                    break;
                default:
                    break;
            }

            if (!ofound)
                continue;

            if (pobj->item_type == ITEM_SCROLL || pobj->item_type == ITEM_POTION || pobj->item_type == ITEM_PILL) {
                lvl = pobj->value[0];
                sn1 = pobj->value[1];
                sn2 = pobj->value[2];
                sn3 = pobj->value[3];

                if (sn1 < 0 || sn1 > MAX_SKILL)
                    sn1name = "";
                else
                    sn1name = skill_table[sn1].name;

                if (sn2 < 0 || sn2 > MAX_SKILL)
                    sn2name = "";
                else
                    sn2name = skill_table[sn2].name;

                if (sn3 < 0 || sn3 > MAX_SKILL)
                    sn3name = "";
                else
                    sn3name = skill_table[sn3].name;

                sendf(ch, "@@N@@d[@@g%5d@@d] @@N%s@@N '%15.15s' '%15.15s' '%15.15s'\n\r", pobj->vnum, my_left(pobj->short_descr, buf, 20), sn1name, sn2name, sn3name);
            }
            else if (pobj->item_type == ITEM_WAND || pobj->item_type == ITEM_STAFF) {
                lvl = pobj->value[0];
                sn3 = pobj->value[3];

                if (sn3 < 0 || sn3 > MAX_SKILL)
                    sn3name = "";
                else
                    sn3name = skill_table[sn3].name;

                sendf(ch, "@@N@@d[@@g%5d@@d] @@N%s@@N '%15.15s'\n\r", pobj->vnum, my_left(pobj->short_descr, buf, 20), sn3name);
            }
        }
    }

    if (!found)
        send_to_char("No objects found.\n\r", ch);

    return;

}

void do_idlecheck(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    char dbuf[64];
    bool found = FALSE;

    if (*argument == '\0') {
        for (victim = first_player; victim; victim = victim->next_player) {
            if (victim->pcdata->idlecheck > 0) {
                found = TRUE;

                if (current_time > victim->pcdata->idlecheck)
                    sendf(ch, "%s's idle check @@eran out@@g %s ago.\n\r", victim->short_descr, duration(current_time - victim->pcdata->idlecheck, dbuf));
                else
                    sendf(ch, "%s's idle check has %s to go.\n\r", victim->short_descr, duration(victim->pcdata->idlecheck - current_time, dbuf));
            }
        }

        if (!found) {
            send_to_char("No players have an idle check set.\n\r", ch);
        }

        return;
    }

    if ((victim = get_char_world(ch, argument)) == NULL) {
        send_to_char("No such player found.\n\r", ch);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("You can't idlecheck NPCs.\n\r", ch);
        return;
    }

    if (IS_IMMORTAL(victim)) {
        send_to_char("You can't idlecheck Immortals.\n\r", ch);
        return;
    }

    if (victim->pcdata->idlecheck > 0) {
        if (current_time > victim->pcdata->idlecheck)
            sendf(ch, "%s's idle check ran out %s ago.\n\r", victim->short_descr, duration(current_time - victim->pcdata->idlecheck, dbuf));
        else
            sendf(ch, "%s's idle check has %s to go.\n\r", victim->short_descr, duration(victim->pcdata->idlecheck - current_time, dbuf));

        return;
    }

    victim->pcdata->idlecheck = current_time + (60 * 10);
    sendf(ch, "Idlecheck set on %s.\n\r", victim->short_descr);
    return;
}
