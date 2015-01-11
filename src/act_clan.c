
/***************************************************************************
 *                                                                         *
 *       _/          _/_/_/     _/    _/     _/    ACK! MUD is modified    *
 *      _/_/        _/          _/  _/       _/    Merc2.0/2.1/2.2 code    *
 *     _/  _/      _/           _/_/         _/    (c)Stephen Dooley 1994  *
 *    _/_/_/_/      _/          _/  _/             "This mud has not been  *
 *   _/      _/      _/_/_/     _/    _/     _/      tested on animals."   *
 *                                                                         *
 ***************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include "merc.h"

IDSTRING(rcsid, "$Id: act_clan.c,v 1.34 2004/10/16 05:47:52 dave Exp $");

extern POL_DATA     politics_data;

void
save_clan_table(void)
{
    FILE               *fp;
    char                clan_file_name[MAX_STRING_LENGTH];
    sh_int              x, y;

    if (nosave)
        return;

    fclose(fpReserve);
    sprintf(clan_file_name, "%s", CLAN_FILE);

    if ((fp = fopen(clan_file_name, "w")) == NULL) {
        bug("Save Clan Table: fopen");
        perror("failed open of clan_table.dat in save_clan_table");
    }
    else {
        for (x = 1; x < MAX_CLAN; x++) {
            for (y = 1; y < MAX_CLAN; y++)
                FPRINTF(fp, "  %5d  ", politics_data.diplomacy[x][y]);

            FPRINTF(fp, "\n");
        }

        for (x = 1; x < MAX_CLAN; x++)
            FPRINTF(fp, "%d\n", politics_data.treasury[x]);

        for (x = 1; x < MAX_CLAN; x++) {
            for (y = 1; y < MAX_CLAN; y++)
                FPRINTF(fp, "  %5d  ", politics_data.end_current_state[x][y]);

            FPRINTF(fp, "\n");
        }

        fflush(fp);
        fclose(fp);
    }

    fpReserve = fopen(NULL_FILE, "r");
    return;
}

bool
is_at_war(CHAR_DATA *ch)
{
    int                 clan, y;

    if (IS_NPC(ch))
        return FALSE;

    clan = ch->pcdata->clan;

    for (y = 0; y < MAX_CLAN; y++)
        if (politics_data.diplomacy[clan][y] < -450)
            return TRUE;

    return FALSE;
}

void
do_ctoggle(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA          *victim = NULL;
    DESCRIPTOR_DATA     d;
    CINFO_DATA         *ci = NULL;
    int                 job;
    char                arg1[MAX_STRING_LENGTH];
    char                arg2[MAX_STRING_LENGTH];

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (IS_NPC(ch) || (!IS_SET(ch->pcdata->pflags, PFLAG_CLAN_BOSS) && !IS_SET(ch->pcdata->pflags, PFLAG_CLAN_2LEADER))) {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    if (arg1[0] == '\0' || arg2[0] == '\0') {
        if (IS_SET(ch->pcdata->pflags, PFLAG_CLAN_BOSS))
            send_to_char("Syntax: ctoggle <player> <armourer/diplomat/treasurer/leader/2leader/trusted>\n\r", ch);
        else
            send_to_char("Syntax: ctoggle <player> <armourer/trusted>\n\r", ch);

        return;
    }

    if ((victim = get_char_world(ch, arg1)) == NULL) {
        if (!IS_SET(ch->pcdata->pflags, PFLAG_CLAN_BOSS)) {
            send_to_char("No such person found.\n\r", ch);
            return;
        }
    }

    if (victim == NULL && IS_SET(ch->pcdata->pflags, PFLAG_CLAN_BOSS)) {
        for (ci = first_cinfo; ci != NULL; ci = ci->next)
             if (ci->clan == ch->pcdata->clan && !str_cmp(arg1, ci->name))
                  break;

        if (ci != NULL) {
            /* load in victim */
            if (load_char_obj(&d, ci->name, TRUE) == FALSE) {
                free_char(d.character);
                send_to_char("No such person found.\n\r", ch);
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
        else {
            send_to_char("No such person found.\n\r", ch);
            return;
        }
    }

    if (victim == NULL) {
        send_to_char("No such person found.\n\r", ch);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("Not on NPCs!\n\r", ch);

        if (ci != NULL) {
            /* they were offline, remove them */
            victim->is_quitting = TRUE;
            extract_char(victim, TRUE);
        }

        return;
    }

    if (ch->pcdata->clan != victim->pcdata->clan) {
        send_to_char("Only on members of YOUR clan!\n\r", ch);

        if (ci != NULL) {
            /* they were offline, remove them */
            victim->is_quitting = TRUE;
            extract_char(victim, TRUE);
        }

        return;
    }

    if (!str_prefix(arg2, "armourer"))
        job = PFLAG_CLAN_ARMOURER;
    else if (!str_prefix(arg2, "diplomat"))
        job = PFLAG_CLAN_DIPLOMAT;
    else if (!str_prefix(arg2, "treasurer"))
        job = PFLAG_CLAN_TREASURER;
    else if (!str_prefix(arg2, "leader"))
        job = PFLAG_CLAN_LEADER;
    else if (!str_prefix(arg2, "2leader"))
        job = PFLAG_CLAN_2LEADER;
    else if (!str_prefix(arg2, "trusted"))
        job = PFLAG_CLAN_TRUSTED;
    else {
        send_to_char("That's not a legal clan job.\n\r", ch);

        if (ci != NULL) {
            /* they were offline, remove them */
            victim->is_quitting = TRUE;
            extract_char(victim, TRUE);
        }

        return;
    }

    if (!IS_SET(ch->pcdata->pflags, PFLAG_CLAN_BOSS) && job != PFLAG_CLAN_ARMOURER && job != PFLAG_CLAN_TRUSTED) {
        send_to_char("Clan Main Leaders may only ctoggle the armourer and trusted flags.\n\r", ch);

        if (ci != NULL) {
            /* they were offline, remove them */
            victim->is_quitting = TRUE;
            extract_char(victim, TRUE);
        }

        return;
    }

    if (!IS_SET(ch->pcdata->pflags, PFLAG_CLAN_BOSS)
        && (IS_SET(victim->pcdata->pflags, PFLAG_CLAN_BOSS) || IS_SET(victim->pcdata->pflags, PFLAG_CLAN_2LEADER))
        && ch != victim) {
        send_to_char("Clan Main Leaders may only ctoggle themselves and leaders or below.\n\r", ch);

        if (ci != NULL) {
            /* they were offline, remove them */
            victim->is_quitting = TRUE;
            extract_char(victim, TRUE);
        }

        return;
    }

    switch (job) {
        case PFLAG_CLAN_TREASURER:
            if (!IS_SET(victim->pcdata->pflags, PFLAG_CLAN_TREASURER)) {
                SET_BIT(victim->pcdata->pflags, PFLAG_CLAN_TREASURER);
                act("$N is now a clan treasurer.", ch, NULL, victim, TO_CHAR);
                act("$n sets you as a clan treasurer.", ch, NULL, victim, TO_VICT);
            }
            else {
                REMOVE_BIT(victim->pcdata->pflags, PFLAG_CLAN_TREASURER);
                act("$N is no longer a clan treasurer.", ch, NULL, victim, TO_CHAR);
                act("$n removes you as a clan treasurer.", ch, NULL, victim, TO_VICT);
            }

            break;

        case PFLAG_CLAN_DIPLOMAT:
            if (!IS_SET(victim->pcdata->pflags, PFLAG_CLAN_DIPLOMAT)) {
                SET_BIT(victim->pcdata->pflags, PFLAG_CLAN_DIPLOMAT);
                act("$N is now a clan diplomat.", ch, NULL, victim, TO_CHAR);
                act("$n sets you as a clan diplomat.", ch, NULL, victim, TO_VICT);
            }
            else {
                REMOVE_BIT(victim->pcdata->pflags, PFLAG_CLAN_DIPLOMAT);
                act("$N is no longer a clan diplomat.", ch, NULL, victim, TO_CHAR);
                act("$n removes you as a clan diplomat.", ch, NULL, victim, TO_VICT);
            }

            break;
        case PFLAG_CLAN_ARMOURER:
            if (!IS_SET(victim->pcdata->pflags, PFLAG_CLAN_ARMOURER)) {
                SET_BIT(victim->pcdata->pflags, PFLAG_CLAN_ARMOURER);
                act("$N is now a clan armourer.", ch, NULL, victim, TO_CHAR);
                act("$n sets you as a clan armourer.", ch, NULL, victim, TO_VICT);
            }
            else {
                REMOVE_BIT(victim->pcdata->pflags, PFLAG_CLAN_ARMOURER);
                act("$N is no longer a clan armourer.", ch, NULL, victim, TO_CHAR);
                act("$n removes you as a clan armourer.", ch, NULL, victim, TO_VICT);
            }

            break;
        case PFLAG_CLAN_LEADER:
            if (!IS_SET(victim->pcdata->pflags, PFLAG_CLAN_LEADER)) {
                SET_BIT(victim->pcdata->pflags, PFLAG_CLAN_LEADER);
                act("$N is now a clan leader.", ch, NULL, victim, TO_CHAR);
                act("$n sets you as a clan leader.", ch, NULL, victim, TO_VICT);
            }
            else {
                REMOVE_BIT(victim->pcdata->pflags, PFLAG_CLAN_LEADER);
                act("$N is no longer a clan leader.", ch, NULL, victim, TO_CHAR);
                act("$n removes you as a clan leader.", ch, NULL, victim, TO_VICT);
            }

            break;
        case PFLAG_CLAN_2LEADER:
            if (!IS_SET(victim->pcdata->pflags, PFLAG_CLAN_2LEADER)) {
                CINFO_DATA *ci2;
                int amt = 0;

                /* Check for too many main leaders in the clan */
                for (ci2 = first_cinfo; ci2 != NULL; ci2 = ci2->next)
                    if (ci2->clan == victim->pcdata->clan && ci2->position == 3)
                        amt++;

                if (amt >= 10) {
                    send_to_char("Main Leaders are currently limited to 10 per clan.\n\r", ch);

                    if (ci != NULL) {
                        /* they were offline, remove them */
                        victim->is_quitting = TRUE;
                        extract_char(victim, TRUE);
                    }

                    return;
                }

                SET_BIT(victim->pcdata->pflags, PFLAG_CLAN_2LEADER);
                act("$N is now a clan main leader.", ch, NULL, victim, TO_CHAR);
                act("$n sets you as a clan main leader.", ch, NULL, victim, TO_VICT);
            }
            else {
                REMOVE_BIT(victim->pcdata->pflags, PFLAG_CLAN_2LEADER);
                act("$N is no longer a clan main leader.", ch, NULL, victim, TO_CHAR);
                act("$n removes you as a clan main leader.", ch, NULL, victim, TO_VICT);
            }

            break;
        case PFLAG_CLAN_TRUSTED:
            if (!IS_SET(victim->pcdata->pflags, PFLAG_CLAN_TRUSTED)) {
                SET_BIT(victim->pcdata->pflags, PFLAG_CLAN_TRUSTED);
                act("$N is now trusted in your clan.", ch, NULL, victim, TO_CHAR);
                act("$n sets you as trusted in your clan.", ch, NULL, victim, TO_VICT);
            }
            else {
                REMOVE_BIT(victim->pcdata->pflags, PFLAG_CLAN_TRUSTED);
                act("$N is no longer trusted in your clan.", ch, NULL, victim, TO_CHAR);
                act("$n removes your trusted status in your clan.", ch, NULL, victim, TO_VICT);
            }

            break;
    }                            /* switch (job) */

    update_cinfo(victim, FALSE);

    if (ci != NULL) {
        /* they were offline, remove them */
        save_char_obj(victim);
        victim->is_quitting = TRUE;
        extract_char(victim, TRUE);
    }

    return;
}

char               *
get_diplo_name(sh_int value)
{
    char               *name = '\0';

    if (value < -450)
        name = "@@R  WAR  @@N";
    else if (value < -300)
        name = "@@e HATRED@@N";
    else if (value < -150)
        name = "@@dDISLIKE@@N";
    else if (value < 150)
        name = "@@WNEUTRAL@@N";
    else if (value < 300)
        name = "@@aRESPECT@@N";
    else if (value < 450)
        name = "@@l TRUST @@N";
    else
        name = "@@B ALLY  @@N";

    return (name);
}

void
do_politics(CHAR_DATA *ch, char *argument)
{
    sh_int              x, y;
    char                buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];

    if (IS_NPC(ch)) {
        send_to_char("Not for Npcs.\n\r", ch);
        return;
    }

    send_to_char("@@NCurrent Politics of " mudnamecolor "\n\r\n\r", ch);

    buf[0] = '\0';
    buf2[0] = '\0';

    sprintf(buf, "        ");
    safe_strcat(MAX_STRING_LENGTH, buf2, buf);

    for (x = 1; x < MAX_CLAN; x++) {
        sprintf(buf, " %s  ", clan_table[x].clan_abbr);
        safe_strcat(MAX_STRING_LENGTH, buf2, buf);
    }

    buf[0] = '\0';
    sprintf(buf, "\n\r\n\r");
    safe_strcat(MAX_STRING_LENGTH, buf2, buf);

    send_to_char(buf2, ch);

    for (x = 1; x < MAX_CLAN; x++) {
        buf[0] = '\0';
        buf2[0] = '\0';
        sprintf(buf, "%1i %s ", x, clan_table[x].clan_abbr);
        safe_strcat(MAX_STRING_LENGTH, buf2, buf);

        for (y = 1; y < MAX_CLAN; y++) {
            buf[0] = '\0';

            if (x != y) {
                sprintf(buf, "%s ", get_diplo_name(politics_data.diplomacy[x][y]));
                safe_strcat(MAX_STRING_LENGTH, buf2, buf);
            }
            else {
                sprintf(buf, "        ");
                safe_strcat(MAX_STRING_LENGTH, buf2, buf);
            }
        }

        sprintf(buf, "\n\r\n\r");
        safe_strcat(MAX_STRING_LENGTH, buf2, buf);
        send_to_char(buf2, ch);
    }

    if (IS_SET(ch->pcdata->pflags, PFLAG_CLAN_DIPLOMAT))
        for (x = 1; x < MAX_CLAN; x++)
            if (politics_data.end_current_state[x][ch->pcdata->clan])
                sendf(ch, "%s @@Nhas requested an end to your current state of affairs.\n\r", clan_table[x].clan_name);

    return;
}

void
do_negotiate(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    char                arg1[MAX_STRING_LENGTH];
    char                arg2[MAX_STRING_LENGTH];
    sh_int              target_clan;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (IS_NPC(ch) || !IS_SET(ch->pcdata->pflags, PFLAG_CLAN_DIPLOMAT))
        if (!IS_SET(ch->pcdata->pflags, PFLAG_CLAN_BOSS))
            if (!IS_SET(ch->pcdata->pflags, PFLAG_CLAN_2LEADER)) {
                send_to_char("This command is for clan diplomats only.\n\r", ch);
                return;
            }

    if (!is_number(arg1) || arg2[0] == '\0') {
        send_to_char("syntax: negotiate <clan number> <peace/war/end>\n\r", ch);
        return;
    }

    target_clan = atoi(arg1);

    if ((target_clan < 1) || (target_clan > (MAX_CLAN - 1))) {
        send_to_char("That is not a clan!\n\r", ch);
        return;
    }

    if (target_clan == ch->pcdata->clan) {
        send_to_char("Well, that will accomplish a lot..you must be of two minds about the whole thing!\n\r", ch);
        return;
    }

    if (politics_data.daily_negotiate_table[ch->pcdata->clan][target_clan] && !IS_IMMORTAL(ch)) {
        sendf(ch, "@@NYour clan has already negotiated with %s @@Ntoday.\n\r", clan_table[target_clan].clan_name);
        return;
    }

    if (!str_prefix(arg2, "peace")) {
        if (politics_data.diplomacy[ch->pcdata->clan][target_clan] < -450) {
            if (politics_data.end_current_state[ch->pcdata->clan][target_clan]
                && politics_data.end_current_state[target_clan][ch->pcdata->clan]) {
                politics_data.diplomacy[ch->pcdata->clan][target_clan] = -425;
                politics_data.diplomacy[target_clan][ch->pcdata->clan] = -425;

                send_to_char("@@NYou have successfully negotiated an end to this dreaded @@eCLAN WAR@@N. Great Job!!\n\r", ch);
                sprintf(buf, "@@eCLAN:@@N The war between %s and %s has ended. They may no longer PKILL each other!!\n\r",
                    clan_table[ch->pcdata->clan].clan_name, clan_table[target_clan].clan_name);

                info(buf, 1);

                politics_data.end_current_state[ch->pcdata->clan][target_clan] = FALSE;
                politics_data.end_current_state[target_clan][ch->pcdata->clan] = FALSE;
            }
            else {
                sendf(ch, "@@NYou are currently at @@RWAR@@N with %s. Both clans must negotiate an end to the war first.\n\r", clan_table[target_clan].clan_name);
            }
        }
        else if (politics_data.diplomacy[ch->pcdata->clan][target_clan] >= 450) {
            sendf(ch, "You are already allied with %s.\n\r", clan_table[target_clan].clan_name);
            return;
        }
        else {
            politics_data.diplomacy[ch->pcdata->clan][target_clan] += 75;
            politics_data.diplomacy[target_clan][ch->pcdata->clan] += 75;
            politics_data.daily_negotiate_table[ch->pcdata->clan][target_clan] = TRUE;

            if (politics_data.diplomacy[ch->pcdata->clan][target_clan] < 450)
                sendf(ch, "You are requesting a more peaceful state of affairs with %s.\n\r", clan_table[target_clan].clan_name);
            else
                sendf(ch, "You have formed an alliance with %s!\n\r", clan_table[target_clan].clan_name);
        }
    }
    else if (!str_prefix(arg2, "war")) {
        if ((politics_data.diplomacy[ch->pcdata->clan][target_clan]) == -460) {
            sendf(ch, "@@NYou are already warring %s@@N.\n\r", clan_table[target_clan].clan_name);
            return;
        }

        if ((politics_data.diplomacy[ch->pcdata->clan][target_clan] - 85) < -450) {
            sendf(ch, "@@NYou have started a @@eWAR@@N with %s! Watch out!\n\r", clan_table[target_clan].clan_name);

            sprintf(buf, "@@eCLAN:@@N A war has started between %s and %s. They may now PKILL each other!!\n\r",
                clan_table[ch->pcdata->clan].clan_name, clan_table[target_clan].clan_name);

            info(buf, 1);

            politics_data.diplomacy[ch->pcdata->clan][target_clan] = -460;
            politics_data.diplomacy[target_clan][ch->pcdata->clan] = -460;
            politics_data.end_current_state[ch->pcdata->clan][target_clan] = FALSE;
            politics_data.end_current_state[target_clan][ch->pcdata->clan] = FALSE;
        }
        else {
            politics_data.diplomacy[ch->pcdata->clan][target_clan] -= 85;
            politics_data.diplomacy[target_clan][ch->pcdata->clan] -= 85;
            politics_data.daily_negotiate_table[ch->pcdata->clan][target_clan] = TRUE;

            sendf(ch, "You are requesting a more aggressive state of affairs with %s.\n\r", clan_table[target_clan].clan_name);
        }
    }
    else if (!str_prefix(arg2, "end")) {
        if (politics_data.diplomacy[ch->pcdata->clan][target_clan] >= 450) {
            send_to_char("You have broken your alliance.\n\r", ch);
            politics_data.end_current_state[ch->pcdata->clan][target_clan] = FALSE;
            politics_data.end_current_state[target_clan][ch->pcdata->clan] = FALSE;
            politics_data.diplomacy[ch->pcdata->clan][target_clan] = 0;
            politics_data.diplomacy[target_clan][ch->pcdata->clan] = 0;
        }
        else {
            if (politics_data.diplomacy[ch->pcdata->clan][target_clan] < -450) {
                politics_data.end_current_state[ch->pcdata->clan][target_clan] = TRUE;

                if (politics_data.end_current_state[ch->pcdata->clan][target_clan]
                    && politics_data.end_current_state[target_clan][ch->pcdata->clan]) {
                    send_to_char("Both clans have successfully negotiated and end to the war.  Negotiate peace to seal your treaty!\n\r", ch);
                }
                else
                    send_to_char("You have requested an end to this dreaded war, but the other clan has not yet agreed.\n\r", ch);
            }
            else {
                send_to_char("You must be either at war or in an alliance with a clan before you can END it.\n\r", ch);
            }
        }
    }
    else
        send_to_char("That is not a legal diplomatic negotiation!\n\r", ch);

    save_clan_table();
    return;
}

void
do_leav(CHAR_DATA *ch, char *argument)
{
    send_to_char("If you want to LEAVE your clan, spell it out!!\n\r", ch);
    return;
}

void
do_cset(CHAR_DATA *ch, char *argument)
{
    char                arg1[MAX_INPUT_LENGTH];
    char                arg2[MAX_INPUT_LENGTH];
    char                buf[MAX_STRING_LENGTH];
    CHAR_DATA          *victim;
    RULER_DATA         *ruler;
    int                 value;

    smash_tilde(argument);
    argument = one_argument(argument, arg1);
    strcpy(arg2, argument);

    if (IS_NPC(ch))
        return;

    if (arg1[0] == '\0' || arg2[0] == '\0') {
        send_to_char("syntax: cset <player> <clan #>\n\r\n\r", ch);
        do_clan_list(ch, "");
        return;
    }

    if ((victim = get_char_world(ch, arg1)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    value = is_number(arg2) ? atoi(arg2) : -1;

    if (value == -1) {
        send_to_char("syntax: cset <player> <clan #>\n\r\n\r", ch);
        do_clan_list(ch, "");
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("Not on NPC's.\n\r", ch);
        return;
    }

    if (value < 0 || value >= MAX_CLAN) {
        sendf(ch, "%d is not a valid value.\n\r", value);
        sendf(ch, "Use a value between 0 and %d.\n\r\n\r", MAX_CLAN - 1);
        do_clan_list(ch, "");
        return;
    }

    if (victim->pcdata->clan > 0)
        update_cinfo(victim, TRUE);

    victim->pcdata->clan = value;
    if (value > 0)
        new_cinfo(victim);

    sendf(ch, "%s now belongs to clan %s.\n\r", victim->name, clan_table[value].clan_name);
    sprintf(buf, "%s has set %s into clan %s.", ch->name, victim->name, clan_table[value].clan_name);
    monitor_chan(buf, MONITOR_CLAN);

    if (victim->adept_level == 20 && (ruler = get_ruler(victim))) {
        ruler->clan = value;
        save_rulers();
    }

    return;
}

void
do_accept(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    CHAR_DATA          *victim;
    RULER_DATA         *ruler;

    if (IS_NPC(ch))
        return;

    if (ch->pcdata->accept_time > 0) {
        sendf(ch, "You accept %s's invitation and join %s!\n\r", ch->pcdata->accept_name, clan_table[ch->pcdata->accept_clan].clan_name);
        sprintf(buf, "%s joins %s by %s's invitation.", ch->short_descr, clan_table[ch->pcdata->accept_clan].clan_name, ch->pcdata->accept_name);
        monitor_chan(buf, MONITOR_CLAN);

        ch->pcdata->clan = ch->pcdata->accept_clan;
        ch->pcdata->accept_clan = 0;
        ch->pcdata->accept_time = (time_t) 0;
        free_string(ch->pcdata->accept_name);
        ch->pcdata->accept_name = str_dup("");

        new_cinfo(ch);

        if (ch->adept_level == 20 && (ruler = get_ruler(ch))) {
            ruler->clan = ch->pcdata->clan;
            save_rulers();
        }

        return;
    }

    if (ch->pcdata->clan == 0) {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    if (!IS_SET(ch->pcdata->pflags, PFLAG_CLAN_LEADER)
        && !IS_SET(ch->pcdata->pflags, PFLAG_CLAN_2LEADER)
        && !IS_SET(ch->pcdata->pflags, PFLAG_CLAN_BOSS)) {
        send_to_char("You must be a clan boss or leader to use this command!\n\r", ch);
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

    if (IS_IMMORTAL(victim)) {
        send_to_char("You hear the gods laughing at you.  Nice try.\n\r", ch);
        return;
    }

    if (victim == ch) {
        send_to_char("You want to accept yourself... Strange.\n\r", ch);
        return;
    }

    if (victim->pcdata->clan != 0) {
        act("$N is already in a clan.  Maybe $E should leave it first?", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (IS_SET(victim->pcdata->pflags, PFLAG_CLAN_DESERTER)
        || IS_SET(victim->pcdata->pflags, PFLAG_CLAN_LEAVER)) {
        act("$N cannot currently be accepted yet.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (victim->level < 20) {
        act("$N must be at least 20th level to enter a clan.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (victim->pcdata->accept_time > 0) {
        send_to_char("They've already been invited to a clan!\n\r", ch);
        return;
    }

    victim->pcdata->accept_clan = ch->pcdata->clan;
    victim->pcdata->accept_time = current_time + (60 * 2);

    free_string(victim->pcdata->accept_name);
    victim->pcdata->accept_name = str_dup(ch->short_descr);

    act("You invite $N into your clan!", ch, NULL, victim, TO_CHAR);
    act("$n invites you into $s clan! Type accept to join. (You have 2 minutes to do so)", ch, NULL, victim, TO_VICT);

    sprintf(buf, "%s invites %s into clan %s.", ch->short_descr, victim->short_descr, clan_table[ch->pcdata->clan].clan_name);
    monitor_chan(buf, MONITOR_CLAN);

    return;
}

void
do_cwhere(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH], buf2[MSL];
    CHAR_DATA          *victim;
    DESCRIPTOR_DATA    *d;
    bool                found;
    BUILD_DATA_LIST    *pList = NULL;
    ROOM_INDEX_DATA    *room = NULL;

    if (IS_NPC(ch))
        return;

    if (ch->pcdata->clan == 0) {
        send_to_char("You don't belong to a clan!\n\r", ch);
        return;
    }

    send_to_char("Clan members visible to you:\n\r", ch);
    found = FALSE;

    for (d = first_desc; d != NULL; d = d->next) {
        if (d->connected == CON_PLAYING && (victim = d->character) != NULL && !IS_NPC(victim)
            && victim->in_room != NULL && (IS_SET(ch->pcdata->pflags, PFLAG_CLAN_BOSS) || can_see(ch, victim))) {
            if (victim->pcdata->clan != ch->pcdata->clan)
                continue;

            if ((!IS_NPC(victim) || IS_AFFECTED(victim, AFF_CHARM)) && victim->in_room && IS_SET(victim->in_room->affected_by, ROOM_BV_SMOKESCREEN)) {
                ROOM_AFFECT_DATA *paf;

                /* smokescreen found, find out what type it is */
                for (paf = victim->in_room->first_room_affect; paf; paf = paf->next)
                    if (paf->bitvector == ROOM_BV_SMOKESCREEN)
                        break;

                if (paf) {
                    if (   paf->type == gsn_smokescreen_advanced
                        || paf->type == gsn_smokescreen_expert
                        || paf->type == gsn_smokescreen_master)
                        continue;
                }
            }

            if (victim->in_room->area && victim->in_room->area->first_area_room) {
                pList = victim->in_room->area->first_area_room;
                room = pList->data;
            }
            if (room && IS_SET(room->affected_by, ROOM_BV_SMOKESCREEN_AREA)) {
                ROOM_AFFECT_DATA *paf;

                /* smokescreen found, find out what type it is */
                for (paf = room->first_room_affect; paf; paf = paf->next)
                    if (paf->bitvector == ROOM_BV_SMOKESCREEN_AREA)
                        break;

                if (paf) {
                    if (   paf->type == gsn_smokescreen_advanced
                        || paf->type == gsn_smokescreen_expert
                        || paf->type == gsn_smokescreen_master)
                        continue;
                }
            }

            found = TRUE;

            if (IS_SET(ch->pcdata->pflags, PFLAG_CLAN_BOSS)
                || IS_SET(ch->pcdata->pflags, PFLAG_CLAN_2LEADER)
                || IS_SET(ch->pcdata->pflags, PFLAG_CLAN_TRUSTED)
                || IS_SET(ch->pcdata->pflags, PFLAG_CLAN_LEADER)) {
                sprintf(buf, "%s %s@@N ", my_left(victim->short_descr, buf2, 28), victim->in_room->name);
            }
            else
                sprintf(buf, "%s ", my_left(victim->short_descr, buf2, 28));

            if (IS_SET(ch->pcdata->pflags, PFLAG_CLAN_BOSS)) {
                if (IS_SET(victim->pcdata->pflags, PFLAG_CLAN_LEADER))
                    safe_strcat(MAX_STRING_LENGTH, buf, " L ");
                if (IS_SET(victim->pcdata->pflags, PFLAG_CLAN_2LEADER))
                    safe_strcat(MAX_STRING_LENGTH, buf, " M ");
                if (IS_SET(victim->pcdata->pflags, PFLAG_CLAN_ARMOURER))
                    safe_strcat(MAX_STRING_LENGTH, buf, " A ");
                if (IS_SET(victim->pcdata->pflags, PFLAG_CLAN_TREASURER))
                    safe_strcat(MAX_STRING_LENGTH, buf, " t ");
                if (IS_SET(victim->pcdata->pflags, PFLAG_CLAN_TRUSTED))
                    safe_strcat(MAX_STRING_LENGTH, buf, " T ");
                if (IS_SET(victim->pcdata->pflags, PFLAG_CLAN_DIPLOMAT))
                    safe_strcat(MAX_STRING_LENGTH, buf, " D ");
            }

            safe_strcat(MAX_STRING_LENGTH, buf, "\n\r");
            send_to_char(buf, ch);
        }
    }

    if (!found)
        send_to_char("No other clan members were found.\n\r", ch);

    return;
}

void
do_leave(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    RULER_DATA         *ruler;

    if (IS_NPC(ch))
        return;

    if (ch->pcdata->clan == 0) {
        send_to_char("You must be IN a clan before you can leave it!\n\r", ch);
        return;
    }

    if (argument == NULL || *argument == '\0' || strcmp("clan", argument)) {
        send_to_char("You must type 'leave clan' to leave your clan.\n\r", ch);
        return;
    }

    sprintf(buf, "%s has left clan %s.", ch->short_descr, clan_table[ch->pcdata->clan].clan_name);
    monitor_chan(buf, MONITOR_CLAN);

    if (is_at_war(ch)) {
        sprintf(buf, "%s has left clan %s during a war.", ch->short_descr, clan_table[ch->pcdata->clan].clan_name);
        info(buf, 5);
        SET_BIT(ch->pcdata->pflags, PFLAG_CLAN_DESERTER);
        send_to_char("You leave your clan during a war and thus become a deserter.\n\rThis means you may not join a clan for 2 RL days.\n\r", ch);
        ch->pcdata->desert_time = current_time + (60 * 60 * 24 * 2);
        ch->pcdata->safetimer = current_time + 120;
    }
    else {
        SET_BIT(ch->pcdata->pflags, PFLAG_CLAN_LEAVER);
        send_to_char("You leave your clan.  Let's hope they don't get mad!\n\rYou may not join another clan for 2 RL hours.\n\r", ch);
        ch->pcdata->desert_time = current_time + (60 * 60 * 2);
    }

    ch->pcdata->clan = 0;

    if (IS_SET(ch->pcdata->pflags, PFLAG_CLAN_DIPLOMAT))
        REMOVE_BIT(ch->pcdata->pflags, PFLAG_CLAN_DIPLOMAT);
    if (IS_SET(ch->pcdata->pflags, PFLAG_CLAN_TREASURER))
        REMOVE_BIT(ch->pcdata->pflags, PFLAG_CLAN_TREASURER);
    if (IS_SET(ch->pcdata->pflags, PFLAG_CLAN_ARMOURER))
        REMOVE_BIT(ch->pcdata->pflags, PFLAG_CLAN_ARMOURER);
    if (IS_SET(ch->pcdata->pflags, PFLAG_CLAN_2LEADER))
        REMOVE_BIT(ch->pcdata->pflags, PFLAG_CLAN_2LEADER);
    if (IS_SET(ch->pcdata->pflags, PFLAG_CLAN_LEADER))
        REMOVE_BIT(ch->pcdata->pflags, PFLAG_CLAN_LEADER);
    if (IS_SET(ch->pcdata->pflags, PFLAG_CLAN_BOSS))
        REMOVE_BIT(ch->pcdata->pflags, PFLAG_CLAN_BOSS);
    if (IS_SET(ch->pcdata->pflags, PFLAG_CLAN_TRUSTED))
        REMOVE_BIT(ch->pcdata->pflags, PFLAG_CLAN_TRUSTED);

    save_char_obj(ch);
    update_cinfo(ch, TRUE);

    if (ch->adept_level == 20 && (ruler = get_ruler(ch))) {
        ruler->clan = 0;
        save_rulers();
    }

    return;
}

void
do_banish(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA          *victim = NULL;
    RULER_DATA         *ruler;
    CINFO_DATA         *ci = NULL;
    DESCRIPTOR_DATA     d;
    char                buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
        return;

    if (!IS_SET(ch->pcdata->pflags, PFLAG_CLAN_LEADER)
        && !IS_SET(ch->pcdata->pflags, PFLAG_CLAN_2LEADER)
        && !IS_SET(ch->pcdata->pflags, PFLAG_CLAN_BOSS)) {
        send_to_char("Only Clan Leaders may use this command.\n\r", ch);
        return;
    }

    if (argument[0] == '\0') {
        send_to_char("Banish WHO from your clan?\n\r", ch);
        return;
    }

    if ((victim = get_char_world(ch, argument)) == NULL) {
        if (!IS_SET(ch->pcdata->pflags, PFLAG_CLAN_BOSS)) {
            send_to_char("No such person found.\n\r", ch);
            return;
        }
    }

    if (victim == NULL && IS_SET(ch->pcdata->pflags, PFLAG_CLAN_BOSS)) {
        for (ci = first_cinfo; ci != NULL; ci = ci->next)
             if (ci->clan == ch->pcdata->clan && !str_cmp(argument, ci->name))
                  break;

        if (ci != NULL) {
            /* load in victim */
            if (load_char_obj(&d, ci->name, TRUE) == FALSE) {
                free_char(d.character);
                send_to_char("No such person found.\n\r", ch);
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
        else {
            send_to_char("No such person found.\n\r", ch);
            return;
        }
    }

    if (victim == NULL) {
        send_to_char("No such person found.\n\r", ch);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("Can't banish NPCs.\n\r", ch);

        if (ci != NULL) {
            /* they were offline, remove them */
            victim->is_quitting = TRUE;
            extract_char(victim, TRUE);
        }

        return;
    }

    if (IS_SET(victim->pcdata->pflags, PFLAG_CLAN_BOSS)
        && (IS_SET(ch->pcdata->pflags, PFLAG_CLAN_LEADER)
            || IS_SET(ch->pcdata->pflags, PFLAG_CLAN_2LEADER)
        )) {
        send_to_char("You can't banish this person!\n\r", ch);

        if (ci != NULL) {
            /* they were offline, remove them */
            victim->is_quitting = TRUE;
            extract_char(victim, TRUE);
        }

        return;
    }

    if (IS_SET(victim->pcdata->pflags, PFLAG_CLAN_2LEADER)
        && IS_SET(ch->pcdata->pflags, PFLAG_CLAN_LEADER)) {
        send_to_char("You can't banish this person!\n\r", ch);

        if (ci != NULL) {
            /* they were offline, remove them */
            victim->is_quitting = TRUE;
            extract_char(victim, TRUE);
        }

        return;
    }

    if (victim == ch) {
        send_to_char("Dumb idea!\n\r", ch);

        if (ci != NULL) {
            /* they were offline, remove them */
            victim->is_quitting = TRUE;
            extract_char(victim, TRUE);
        }

        return;
    }

    if (victim->pcdata->clan != ch->pcdata->clan) {
        send_to_char("They're not in your clan!\n\r", ch);

        if (ci != NULL) {
            /* they were offline, remove them */
            victim->is_quitting = TRUE;
            extract_char(victim, TRUE);
        }

        return;
    }

    if (IS_SET(victim->pcdata->pflags, PFLAG_CLAN_BOSS)) {
        send_to_char("Nice Try.\n\r", ch);

        if (ci != NULL) {
            /* they were offline, remove them */
            victim->is_quitting = TRUE;
            extract_char(victim, TRUE);
        }

        return;
    }

    if (IS_SET(victim->pcdata->pflags, PFLAG_CLAN_2LEADER)
        && !IS_SET(ch->pcdata->pflags, PFLAG_CLAN_BOSS)) {
        send_to_char("Only the Clan Boss may banish a Main Leader.\n\r", ch);

        if (ci != NULL) {
            /* they were offline, remove them */
            victim->is_quitting = TRUE;
            extract_char(victim, TRUE);
        }

        return;
    }

    /* only do info message if player is online */
    if (ci == NULL && is_at_war(victim)) {
        sprintf(buf, "%s has been removed from clan %s during a war.", victim->short_descr, clan_table[victim->pcdata->clan].clan_name);
        info(buf, 5);

        /* only give them a safetimer if they're not currently fighting. */
        if (victim->fighting == NULL)
            victim->pcdata->safetimer = current_time + 120;
    }

    victim->pcdata->clan = 0;

    if (IS_SET(victim->pcdata->pflags, PFLAG_CLAN_DIPLOMAT))
        REMOVE_BIT(victim->pcdata->pflags, PFLAG_CLAN_DIPLOMAT);
    if (IS_SET(victim->pcdata->pflags, PFLAG_CLAN_TREASURER))
        REMOVE_BIT(victim->pcdata->pflags, PFLAG_CLAN_TREASURER);
    if (IS_SET(victim->pcdata->pflags, PFLAG_CLAN_ARMOURER))
        REMOVE_BIT(victim->pcdata->pflags, PFLAG_CLAN_ARMOURER);
    if (IS_SET(victim->pcdata->pflags, PFLAG_CLAN_2LEADER))
        REMOVE_BIT(victim->pcdata->pflags, PFLAG_CLAN_2LEADER);
    if (IS_SET(victim->pcdata->pflags, PFLAG_CLAN_LEADER))
        REMOVE_BIT(victim->pcdata->pflags, PFLAG_CLAN_LEADER);
    if (IS_SET(victim->pcdata->pflags, PFLAG_CLAN_BOSS))
        REMOVE_BIT(victim->pcdata->pflags, PFLAG_CLAN_BOSS);
    if (IS_SET(victim->pcdata->pflags, PFLAG_CLAN_TRUSTED))
        REMOVE_BIT(victim->pcdata->pflags, PFLAG_CLAN_TRUSTED);

    SET_BIT(victim->pcdata->pflags, PFLAG_CLAN_LEAVER);

    victim->pcdata->desert_time = current_time + (60 * 60 * 2);

    sprintf(buf, "%s has banished %s from clan %s.", ch->name, victim->name, clan_table[ch->pcdata->clan].clan_name);
    monitor_chan(buf, MONITOR_CLAN);

    sendf(victim, "%s banishes you from clan %s!\n\rYou may not join another clan for 2 RL hours.\n\r", ch->short_descr, clan_table[ch->pcdata->clan].clan_name);
    act("$N has been banished.", ch, NULL, victim, TO_CHAR);

    update_cinfo(victim, TRUE);

    if (victim->in_room) {
        ROOM_INDEX_DATA *hall = get_room_index(clan_table[ch->pcdata->clan].clan_room);

        if (hall) {
            /* valid clan hall */
            if (victim->in_room->area == hall->area) {
                /* they're in the clan hall! */
                char_from_room(victim);
                char_to_room(victim, get_room_index(ROOM_VNUM_LIMBO));
                send_to_char("You have been removed from your clan hall.\n\r", victim);
                act("You remove $N from your clan's hall.", ch, NULL, victim, TO_CHAR);
                do_look(victim, "auto");
            }
        }
    }

    save_char_obj(victim);

    if (victim->adept_level == 20 && (ruler = get_ruler(victim))) {
        ruler->clan = 0;
        save_rulers();
    }

    if (ci != NULL) {
        /* they were offline, remove them */
        victim->is_quitting = TRUE;
        extract_char(victim, TRUE);
    }

    return;
}

void
do_make(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    char                arg1[MAX_STRING_LENGTH];
    char                arg2[MAX_STRING_LENGTH];
    OBJ_INDEX_DATA     *pObj;
    OBJ_DATA           *obj;
    int                 cnt;
    int                 num;
    CHAR_DATA          *target;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (IS_NPC(ch)) {
        send_to_char("Not on NPC's idiot!\n\r", ch);
        return;
    }

    if (!IS_SET(ch->pcdata->pflags, PFLAG_CLAN_BOSS)
        && !IS_SET(ch->pcdata->pflags, PFLAG_CLAN_2LEADER)
        && !IS_SET(ch->pcdata->pflags, PFLAG_CLAN_LEADER)
        && !IS_SET(ch->pcdata->pflags, PFLAG_CLAN_ARMOURER)) {
        send_to_char("This command is for Clan Armourers and Leaders only.\n\r", ch);
        return;
    }

    if (!is_number(arg1) || arg2[0] == '\0') {
        send_to_char("syntax: make <number> <player>\n\r", ch);
        send_to_char("Items you can currently make:\n\r\n\r", ch);

        for (cnt = 0; cnt < MAX_CLAN_EQ; cnt++)
            if (clan_table[ch->pcdata->clan].eq[cnt] != -1 && ((pObj = get_obj_index(clan_table[ch->pcdata->clan].eq[cnt])) != NULL))
                sendf(ch, "[%2d] [Level: %3d] %s\n\r", cnt, pObj->level, pObj->short_descr);

        return;
    }

    if ((target = get_char_room(ch, arg2)) == NULL) {
        send_to_char("No one with that name is here with you.\n\r", ch);
        return;
    }

    if (IS_NPC(target)) {
        send_to_char("NOT on NPCs!\n\r", ch);
        return;
    }

    if (target->pcdata->clan != ch->pcdata->clan) {
        act("$N isn't in your clan...", ch, NULL, target, TO_CHAR);
        return;
    }

    num = atoi(arg1);
    if (num < 0 || num >= MAX_CLAN_EQ) {
        do_make(ch, "");
        return;
    }

    if (clan_table[ch->pcdata->clan].eq[num] == -1) {
        do_make(ch, "");
        return;
    }

    if ((pObj = get_obj_index(clan_table[ch->pcdata->clan].eq[num])) == NULL) {
        send_to_char("Couldn't find that object to load...\n\r", ch);
        do_make(ch, "");
        return;
    }

    /* remove this so imm clan can 'make' normal stuff. clan eq should always be
     * flagged as such anyhow..
     if (!IS_SET( pObj->extra_flags, ITEM_CLAN_EQ)) {
     send_to_char("Object exists, but not flagged as Clan Eq.\n\r", ch);
     return;
     }
     */

    for (obj = target->first_carry; obj != NULL; obj = obj->next_in_carry_list) {
        if (obj->wear_loc == -1 && obj->pIndexData == pObj) {
            if (ch != target)
                act("$N already has a $p.", ch, obj, target, TO_CHAR);
            else
                act("You already have a $p.", ch, obj, NULL, TO_CHAR);

            return;
        }
    }

    obj = create_object(pObj, target->level);

    if (target->carry_number + get_obj_number(obj) >= can_carry_n(target)) {
        if (ch != target)
            act("$N has $S hands full.", ch, NULL, target, TO_CHAR);
        else
            send_to_char("You have your hands full.\n\r", ch);

        extract_obj(obj);
        return;
    }

    if (target->carry_weight + get_obj_weight(obj) >= can_carry_w(target)) {
        if (ch != target)
            act("$N can't carry that much weight.", ch, NULL, target, TO_CHAR);
        else
            send_to_char("You can't carry that much weight.\n\r", ch);

        extract_obj(obj);
        return;
    }

    if (!IS_IMMORTAL(ch) && !IS_NPC(target) && ch != target && IS_SET(target->config, PLR_NOGIVE)) {
        act("$n tries to make you $p.", ch, obj, target, TO_VICT);
        act("You try to make $p to $N, but $E doesn't want it.", ch, obj, target, TO_CHAR);

        extract_obj(obj);
        return;
    }

    if (ch != target) {
        act("$n creates $p, and hands it to $N.", ch, obj, target, TO_NOTVICT);
        act("You create $p, and hand it to $N.", ch, obj, target, TO_CHAR);
        act("$n creates $p, and hands it to you.", ch, obj, target, TO_VICT);
        obj_to_char(obj, target);
        sprintf(buf, "%s has made %s for %s.", ch->name, obj->short_descr, target->name);
    }
    else {
        act("You create $p, and put it away.", ch, obj, NULL, TO_CHAR);
        act("$n creates $p, and puts it away.", ch, obj, NULL, TO_ROOM);
        obj_to_char(obj, ch);
        sprintf(buf, "%s has made themself %s.", ch->name, obj->short_descr);
    }

    monitor_chan(buf, MONITOR_CLAN);

    return;
}

void
do_dleft(CHAR_DATA *ch, char *argument)
{
    char                durbuf[64];

    durbuf[0] = 0;

    if (IS_NPC(ch))
        return;

    if (ch->pcdata->desert_time == 0) {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    sendf(ch, "You have %s left of being a %s.\n\r",
        duration(ch->pcdata->desert_time - current_time, durbuf), (IS_SET(ch->pcdata->pflags, PFLAG_CLAN_DESERTER))
        ? "Deserter" : "Leaver");

    return;
}

void
save_cinfo(void)
{
    FILE               *fp;
    char                cinfo_file[MSL];
    CINFO_DATA         *cinfo;

    if (nosave)
        return;

    fclose(fpReserve);
    sprintf(cinfo_file, "%s", CINFO_FILE);

    if ((fp = fopen(cinfo_file, "w")) == NULL)
        bug("Save Cinfo Table: fopen");
    else {
        for (cinfo = first_cinfo; cinfo; cinfo = cinfo->next)
            FPRINTF(fp, "! %s~ %d %d %d %d %d %d %d\n",
                cinfo->name, cinfo->clan, cinfo->position, cinfo->flags, (int) cinfo->lastlogin, cinfo->level, cinfo->remort, cinfo->adept);

        FPRINTF(fp, "E\n");
    }

    fclose(fp);
    fpReserve = fopen(NULL_FILE, "r");

    return;
}

void
load_cinfo(void)
{
    FILE               *fp;
    char                cinfo_file[MSL];
    char                letter;
    CINFO_DATA         *cinfo;

    sprintf(cinfo_file, "%s", CINFO_FILE);

    if ((fp = fopen(cinfo_file, "r")) == NULL)
        bug("Load Cinfo Table: fopen");
    else {
        for (;;) {
            letter = fread_letter(fp);

            if (letter == '!') {
                GET_FREE(cinfo, cinfo_free);

                cinfo->name = fread_string(fp);
                cinfo->clan = fread_number(fp);
                cinfo->position = fread_number(fp);
                cinfo->flags = fread_number(fp);
                cinfo->lastlogin = (time_t) fread_number(fp);
                cinfo->level = fread_number(fp);
                cinfo->remort = fread_number(fp);
                cinfo->adept = fread_number(fp);

                LINK(cinfo, first_cinfo, last_cinfo, next, prev);
            }
            else
                break;
        }
    }

    fclose(fp);

    return;
}

void
update_cinfo(CHAR_DATA *ch, bool remove)
{
    CINFO_DATA         *cinfo;
    int                 flags = 0;

    /* Immortals don't belong on CINFO lists */
    if (!IS_NPC(ch) && IS_IMMORTAL(ch) && !remove)
        return;

    for (cinfo = first_cinfo; cinfo; cinfo = cinfo->next)
        if (!str_cmp(cinfo->name, ch->name))
            break;

    if (!cinfo)
        return;

    if (remove) {
        free_string(cinfo->name);
        UNLINK(cinfo, first_cinfo, last_cinfo, next, prev);
        PUT_FREE(cinfo, cinfo_free);
    }
    else {
        cinfo->clan = ch->pcdata->clan;

        if (IS_SET(ch->pcdata->pflags, PFLAG_CLAN_BOSS))
            cinfo->position = 4;
        else if (IS_SET(ch->pcdata->pflags, PFLAG_CLAN_2LEADER))
            cinfo->position = 3;
        else if (IS_SET(ch->pcdata->pflags, PFLAG_CLAN_LEADER))
            cinfo->position = 2;
        else if (IS_SET(ch->pcdata->pflags, PFLAG_CLAN_ARMOURER))
            cinfo->position = 1;
        else
            cinfo->position = 0;

        if (IS_SET(ch->pcdata->pflags, PFLAG_CLAN_DIPLOMAT))
            SET_BIT(flags, 1);
        if (IS_SET(ch->pcdata->pflags, PFLAG_CLAN_TREASURER))
            SET_BIT(flags, 2);
        if (IS_SET(ch->pcdata->pflags, PFLAG_CLAN_TRUSTED))
            SET_BIT(flags, 4);

        cinfo->flags = flags;
        cinfo->lastlogin = ch->pcdata->lastlogint;
        cinfo->level = ch->level;
        cinfo->remort = get_pseudo_level(ch) - ch->level;

        if (ch->adept_level > 0)
            cinfo->adept = ch->adept_level;
        else
            cinfo->adept = 0;
    }

    save_cinfo();
    return;
}

void
new_cinfo(CHAR_DATA *ch)
{
    CINFO_DATA         *cinfo;

    GET_FREE(cinfo, cinfo_free);
    cinfo->name = str_dup(ch->name);
    LINK(cinfo, first_cinfo, last_cinfo, next, prev);
    update_cinfo(ch, FALSE);

    return;
}

#define CINFO_TOTAL 5

void
do_cinfo(CHAR_DATA *ch, char *argument)
{
    char                buf[MSL];
    char                arg[MIL];
    char                _durbuf[64];
    char               *durbuf = _durbuf;
    CINFO_DATA         *cinfo;
    int                 clan = 0;
    int                 total[CINFO_TOTAL] = { 0, 0, 0, 0, 0 };
    int                 cnt = 0;
    int                 icnt = 0;
    int                 see = 0;
    int                 count = 0;
    int                 length = 0;
    int                 show = 0;

    if (IS_NPC(ch))
        return;

    clan = ch->pcdata->clan;
    _durbuf[0] = 0;

    argument = one_argument(argument, arg);

    if (clan == 0 && !IS_IMMORTAL(ch)) {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    if (IS_IMMORTAL(ch) || IS_SET(ch->pcdata->pflags, PFLAG_CLAN_BOSS))
        see = 3;
    else if (IS_SET(ch->pcdata->pflags, PFLAG_CLAN_2LEADER))
        see = 2;
    else if (IS_SET(ch->pcdata->pflags, PFLAG_CLAN_LEADER))
        see = 1;
    else
        see = 0;

    if (arg[0])
        if (!str_prefix(arg, "flags") && see >= 1)
            show = 1;
        else if (!str_prefix(arg, "levels") && see >= 2)
            show = 2;
        else if (!str_prefix(arg, "logins") && see >= 3)
            show = 3;
        else if (IS_IMMORTAL(ch) && is_number(arg)) {
            show = 0;
            clan = atoi(arg);
        }
        else
            show = 0;
    else
        show = 0;

    if (IS_IMMORTAL(ch) && *argument)
        clan = atoi(argument);

    if (clan <= 0 || clan >= MAX_CLAN) {
        send_to_char("Invalid clan number.\n\r", ch);
        return;
    }

    for (cinfo = first_cinfo; cinfo; cinfo = cinfo->next)
        if (cinfo->clan == clan)
            if (cinfo->position >= 0 && cinfo->position < CINFO_TOTAL)
                total[cinfo->position]++;

    send_to_char("@@d.-------------------------------------------------------------@@g=(@@W Clan Info @@g)=@@d-.\n\r", ch);
    sendf(ch, "| %s @@d|\n\r", center_text(clan_table[clan].clan_name, 75));

    for (cnt = CINFO_TOTAL - 1; cnt >= 0; cnt--) {
        if (total[cnt] > 0) {
            switch (cnt) {
                case 4:
                    send_to_char("@@d|------------------------------------------------------------------@@g=( @@WBoss @@g)=@@d-|\n\r", ch);
                    break;
                case 3:
                    send_to_char("@@d|-----------------------------------------------------------@@g=( @@WMain Leader @@g)=@@d-|\n\r", ch);
                    break;
                case 2:
                    send_to_char("@@d|----------------------------------------------------------------@@g=( @@WLeader @@g)=@@d-|\n\r", ch);
                    break;
                case 1:
                    send_to_char("@@d|--------------------------------------------------------------@@g=( @@WArmourer @@g)=@@d-|\n\r", ch);
                    break;
                default:
                    send_to_char("@@d|----------------------------------------------------------------@@g=( @@WMember @@g)=@@d-|\n\r", ch);
                    break;
            }

            icnt = 0;

            for (cinfo = first_cinfo; cinfo; cinfo = cinfo->next) {
                if (cinfo->clan != clan)
                    continue;
                if (cinfo->position != cnt)
                    continue;

                icnt++;

                if (icnt % 2 == 1)
                    sprintf(buf, "@@d| @@g%s", cinfo->name);
                else
                    sprintf(buf, "@@g%s", cinfo->name);

                count = 16 - my_strlen(cinfo->name);
                for (length = 0; length < count; length++)
                    safe_strcat(MSL, buf, " ");

                send_to_char(buf, ch);

                if (show == 1) {
                    strcpy(buf, "");
                    if (IS_SET(cinfo->flags, 1))
                        safe_strcat(MSL, buf, "[D] ");
                    if (IS_SET(cinfo->flags, 2))
                        safe_strcat(MSL, buf, "[T] ");
                    if (IS_SET(cinfo->flags, 4))
                        safe_strcat(MSL, buf, "[R] ");

                    count = 22 - my_strlen(buf);
                    for (length = 0; length < count; length++)
                        safe_strcat(MSL, buf, " ");

                    send_to_char(buf, ch);
                }
                else if (show == 2) {
                    strcpy(buf, "");
                    if (cinfo->adept > 0)
                        sprintf(buf, "@@W%d @@yadept@@d", cinfo->adept);
                    else if (cinfo->remort > 0)
                        sprintf(buf, "@@p%d@@d", cinfo->level + cinfo->remort);
                    else
                        sprintf(buf, "@@a%d@@d", cinfo->level);

                    count = 22 - my_strlen(buf);
                    for (length = 0; length < count; length++)
                        safe_strcat(MSL, buf, " ");

                    send_to_char(buf, ch);
                }
                else if (show == 3) {
                    strcpy(buf, "");
                    _durbuf[0] = 0;

                    if (cinfo->lastlogin > 0)
                        sprintf(buf, "@@W%s@@g", duration(current_time - cinfo->lastlogin, durbuf));

                    count = 22 - my_strlen(buf);
                    for (length = 0; length < count; length++)
                        safe_strcat(MSL, buf, " ");

                    send_to_char(buf, ch);
                }
                else {
                    sprintf(buf, "%s", " ");
                    count = 21;
                    for (length = 0; length < count; length++)
                        safe_strcat(MSL, buf, " ");

                    send_to_char(buf, ch);
                }

                if (icnt == total[cnt] && icnt % 2 == 1) {
                    /* at end of position list and we have to fill an extra column */
                    send_to_char("                                      @@d|\n\r", ch);
                }
                else if (icnt % 2 == 0)
                    send_to_char("@@d|\n\r", ch);

            }
        }
    }

    {
        int totalmembers = total[0] + total[1] + total[2] + total[3] + total[4];
        sprintf(buf, "@@a%d @@gmember%s@@N", totalmembers, totalmembers == 1 ? "" : "s");
        count = 70 - my_strlen(buf);
        strcpy(buf, "@@d'");

        for (length = 0; length < count; length++)
            safe_strcat(MSL, buf, "-");

        sprintf(buf + strlen(buf), "@@g=( @@a%d @@gmember%s@@N @@g)=@@d-'@@N\n\r", totalmembers, totalmembers == 1 ? "" : "s");
        send_to_char(buf, ch);
    }

    return;
}
