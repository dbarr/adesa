#define __USE_BSD
#include <stdio.h>
#undef __USE_BSD
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>
#include <time.h>
#include "merc.h"

/* DLINK/DUNLINK/CREATE_MEMBER/DESTROY_MEMBER */
#include "duel.h"
#include "auction.h"

#include "treasury.h"

IDSTRING(rcsid, "$Id: treasury.c,v 1.7 2004/06/18 18:21:29 dave Exp $");

struct treasury_data treasury[MAX_CLAN];

void load_treasuries(void)
{
    FILE *fp;
    char *word;
    char filename[MIL];
    int cnt = 0;
    TREASURY_HISTORY_DATA *history;

    sprintf(filename, "%s", TREASURY_FILE);

    if ((fp = fopen(filename, "r")) == NULL)
        return;

    word = fread_word(fp);

    if (!str_cmp(word, "#CLANS")) {
        for (cnt = 0; cnt < MAX_CLAN; cnt++) {
            treasury[cnt].qp = fread_number(fp);
            treasury[cnt].gold = fread_number(fp);
            treasury[cnt].first_history = NULL;
            treasury[cnt].last_history = NULL;
        }
    }

    word = fread_word(fp);

    if (!str_cmp(word, "#HISTORIES")) {
        for (;;) {
            word = !feof(fp) ? fread_word(fp) : "End";

            if (!str_cmp(word, "Clan")) {
                cnt = fread_number(fp);

                if (cnt < 0 || cnt >= MAX_CLAN) {
                    xlogf("load_treasuries(): bad # for clan history, found %d", cnt);
                    fclose(fp);
                    return;
                }
            }
            else if (!str_cmp(word, "History")) {
                CREATE_MEMBER(TREASURY_HISTORY_DATA, history);

                if (!history)
                    continue;

                history->timestamp = (time_t)fread_number(fp);
                history->action    = fread_number(fp);
                history->amount    = fread_number(fp);
                history->who       = fread_string(fp);

                if (history->timestamp + TREASURY_HISTORY_EXPIRE < current_time) {
                    free_string(history->who);
                    DESTROY_MEMBER(history);
                }
                else
                    DLINK(history, treasury[cnt].first_history, treasury[cnt].last_history, next, prev);
            }
            else
                break;
        }
    }

    fclose(fp);
    return;
}

void save_treasuries(void)
{
    FILE *fp;
    char filename[MIL];
    int cnt;
    TREASURY_HISTORY_DATA *history;

    sprintf(filename, "%s", TREASURY_FILE);

    if ((fp = fopen(filename, "w")) == NULL)
        return;

    FPRINTF(fp, "#CLANS\n");

    for (cnt = 0; cnt < MAX_CLAN; cnt++)
        FPRINTF(fp, "%d %d\n", treasury[cnt].qp, treasury[cnt].gold);

    FPRINTF(fp, "#HISTORIES\n");

    for (cnt = 0; cnt < MAX_CLAN; cnt++) {
        FPRINTF(fp, "Clan %d\n", cnt);

        for (history = treasury[cnt].first_history; history != NULL; history = history->next)
            FPRINTF(fp, "History %d %d %d %s~\n", (int)history->timestamp, history->action, history->amount, history->who);
    }

    FPRINTF(fp, "End\n");

    fclose(fp);
}

void do_treasury_history(CHAR_DATA *ch, int clan);

void do_treasury(CHAR_DATA *ch, char *argument)
{
    char arg[MIL], arg2[MIL];
    char buf[MSL];
    TREASURY_HISTORY_DATA *history;
    int amount = 0;
    int action;

    if (!IS_IMMORTAL(ch) && (IS_NPC(ch) || ch->pcdata->clan == 0)) {
        send_to_char("You must be in a clan to perform this command.\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg);
    argument = one_argument(argument, arg2);

    if (!str_cmp(arg, "withdraw")) {
        ROOM_INDEX_DATA *room;
        AREA_DATA *area;

        if (!IS_SET(ch->pcdata->pflags, PFLAG_CLAN_BOSS) && !IS_SET(ch->pcdata->pflags, PFLAG_CLAN_2LEADER) && !IS_SET(ch->pcdata->pflags, PFLAG_CLAN_TREASURER)) {
            send_to_char("You must be a boss, a main leader, or a treasurer to make a withdrawal.\n\r", ch);
            return;
        }

        if (!(room = get_room_index(clan_table[ch->pcdata->clan].clan_room))
            || !(area = room->area)
            || ch->in_room->area != area) {
            send_to_char("You must be in your clan hall to withdraw from the treasury.\n\r", ch);
            return;
        }

        if (!str_cmp(arg2, "qp"))
            action = TREASURY_ACTION_WITHDRAW_QP;
        else if (!str_cmp(arg2, "gold"))
            action = TREASURY_ACTION_WITHDRAW_GOLD;
        else {
            send_to_char("syntax: treasury withdraw <gold|qp> <amount>\n\r", ch);
            return;
        }

        amount = abs(atoi(argument));

        if (amount < 1) {
            send_to_char("You can't withdraw nothing.\n\r", ch);
            return;
        }

        if (action == TREASURY_ACTION_WITHDRAW_QP && amount > treasury[ch->pcdata->clan].qp) {
            sendf(ch, "@@gYou can't withdrawl @@a%s @@gQP because there's only @@a%s @@gQP available.@@N\n\r",
                number_comma_r(amount, buf), number_comma(treasury[ch->pcdata->clan].qp));
            return;
        }
        else if (action == TREASURY_ACTION_WITHDRAW_GOLD && amount > treasury[ch->pcdata->clan].gold) {
            sendf(ch, "@@gYou can't withdrawl @@y%s @@gGP because there's only @@y%s @@gGP available.@@N\n\r",
                number_comma_r(amount, buf), number_comma(treasury[ch->pcdata->clan].gold));
            return;
        }

        CREATE_MEMBER(TREASURY_HISTORY_DATA, history);

        if (!history) {
            send_to_char("Unable to create history structure.\n\r", ch);
            return;
        }

        history->timestamp = current_time;
        history->action    = action;
        history->amount    = amount;
        history->who       = str_dup(ch->short_descr);

        DLINK(history, treasury[ch->pcdata->clan].first_history, treasury[ch->pcdata->clan].last_history, next, prev);

        if (action == TREASURY_ACTION_WITHDRAW_QP) {
            treasury[ch->pcdata->clan].qp -= amount;
            ch->quest_points += amount;
            sendf(ch, "@@gYou withdraw @@a%s @@gQP from the @@N%s@@N@@g treasury. There is now @@a%s @@gQP remaining.@@N\n\r",
                number_comma_r(amount, buf), clan_table[ch->pcdata->clan].clan_name, number_comma(treasury[ch->pcdata->clan].qp));

            sprintf(log_buf, "@@g%s withdraws @@a%s @@gQP from the @@N%s@@N@@g treasury. There is now @@a%s @@gQP remaining.@@N\n\r",
                ch->short_descr, number_comma_r(amount, buf), clan_table[ch->pcdata->clan].clan_name, number_comma(treasury[ch->pcdata->clan].qp));
            monitor_chan(log_buf, MONITOR_GEN_MORT);

            save_treasuries();
            save_char_obj(ch);
        }
        else if (action == TREASURY_ACTION_WITHDRAW_GOLD) {
            treasury[ch->pcdata->clan].gold -= amount;
            ch->gold += amount;
            sendf(ch, "@@gYou withdraw @@y%s @@gGP from the @@N%s@@N@@g treasury. There is now @@y%s @@gGP remaining.@@N\n\r",
                number_comma_r(amount, buf), clan_table[ch->pcdata->clan].clan_name, number_comma(treasury[ch->pcdata->clan].gold));

            sprintf(log_buf, "@@g%s withdraws @@y%s @@gGP from the @@N%s@@N@@g treasury. There is now @@y%s @@gGP remaining.@@N\n\r",
                ch->short_descr, number_comma_r(amount, buf), clan_table[ch->pcdata->clan].clan_name, number_comma(treasury[ch->pcdata->clan].gold));
            monitor_chan(log_buf, MONITOR_GEN_MORT);

            save_treasuries();
            save_char_obj(ch);
        }

        return;
    } /* end: withdraw */

    if (!str_cmp(arg, "deposit")) {
        ROOM_INDEX_DATA *room;
        AREA_DATA *area;

        if (!(room = get_room_index(clan_table[ch->pcdata->clan].clan_room))
            || !(area = room->area)
            || ch->in_room->area != area) {
            send_to_char("You must be in your clan hall to deposit to the treasury.\n\r", ch);
            return;
        }

        if (!str_cmp(arg2, "qp"))
            action = TREASURY_ACTION_DEPOSIT_QP;
        else if (!str_cmp(arg2, "gold"))
            action = TREASURY_ACTION_DEPOSIT_GOLD;
        else {
            send_to_char("syntax: treasury deposit <gold|qp> <amount>\n\r", ch);
            return;
        }

        if (action == TREASURY_ACTION_DEPOSIT_QP && ch->adept_level <= 0) {
            send_to_char("You must be an adept to deposit quest points into your treasury.\n\r", ch);
            return;
        }

        amount = abs(atoi(argument));

        if (amount < 1) {
            send_to_char("You can't deposit nothing.\n\r", ch);
            return;
        }

        if (action == TREASURY_ACTION_DEPOSIT_QP && amount > available_qps(ch)) {
            sendf(ch, "@@gYou can't deposit @@a%s @@gQP because you only have @@a%s @@gQP available.@@N\n\r",
                number_comma_r(amount, buf), number_comma(available_qps(ch)));
            return;
        }

        if (action == TREASURY_ACTION_DEPOSIT_GOLD && amount > ch->gold) {
            sendf(ch, "@@gYou can't deposit @@y%s @@gGP because you only have @@y%s @@gGP available.@@N\n\r",
                number_comma_r(amount, buf), number_comma(ch->gold));
            return;
        }

        CREATE_MEMBER(TREASURY_HISTORY_DATA, history);

        if (!history) {
            send_to_char("Unable to create history structure.\n\r", ch);
            return;
        }

        history->timestamp = current_time;
        history->action    = action;
        history->amount    = amount;
        history->who       = str_dup(ch->short_descr);

        DLINK(history, treasury[ch->pcdata->clan].first_history, treasury[ch->pcdata->clan].last_history, next, prev);

        if (action == TREASURY_ACTION_DEPOSIT_QP) {
            ch->quest_points -= amount;
            treasury[ch->pcdata->clan].qp += amount;
            sendf(ch, "@@gYou deposit @@a%s @@gQP into the @@N%s@@N@@g treasury. It now contains @@a%s @@gQP.@@N\n\r",
                number_comma_r(amount, buf), clan_table[ch->pcdata->clan].clan_name, number_comma(treasury[ch->pcdata->clan].qp));

            sprintf(log_buf, "@@g%s deposits @@a%s @@gQP into the @@N%s@@N@@g treasury. It now contains @@a%s @@gQP.@@N\n\r",
                ch->short_descr, number_comma_r(amount, buf), clan_table[ch->pcdata->clan].clan_name, number_comma(treasury[ch->pcdata->clan].qp));
            monitor_chan(log_buf, MONITOR_GEN_MORT);

            save_treasuries();
            save_char_obj(ch);
        }
        else if (action == TREASURY_ACTION_DEPOSIT_GOLD) {
            ch->gold -= amount;
            treasury[ch->pcdata->clan].gold += amount;
            sendf(ch, "@@gYou deposit @@y%s @@gGP into the @@N%s@@N@@g treasury. It now contains @@y%s @@gGP.@@N\n\r",
                number_comma_r(amount, buf), clan_table[ch->pcdata->clan].clan_name, number_comma(treasury[ch->pcdata->clan].gold));

            sprintf(log_buf, "@@g%s deposits @@y%s @@gGP into the @@N%s@@N@@g treasury. It now contains @@y%s @@gGP.@@N\n\r",
                ch->short_descr, number_comma_r(amount, buf), clan_table[ch->pcdata->clan].clan_name, number_comma(treasury[ch->pcdata->clan].gold));
            monitor_chan(log_buf, MONITOR_GEN_MORT);

            save_treasuries();
            save_char_obj(ch);
        }

        return;
    } /* end: deposit */

    if (!str_cmp(arg, "balance")) {
        if (!IS_IMMORTAL(ch))
        sendf(ch, "@@gThe @@N%s@@N@@g treasury contains @@a%s @@gQP and @@y%s @@gGP.@@N\n\r",
            clan_table[ch->pcdata->clan].clan_name, number_comma_r(treasury[ch->pcdata->clan].qp, buf), number_comma(treasury[ch->pcdata->clan].gold));
        else {
            int cnt;

            for (cnt = 0; cnt < MAX_CLAN; cnt++)
                sendf(ch, "@@N%s: @@a%s @@gQP, @@y%s @@gGP@@N\n\r", 
                    clan_table[cnt].clan_abbr, number_comma_r(treasury[cnt].qp, buf), number_comma(treasury[cnt].gold));
        }

        return;
    } /* end: balance */

    if (!str_cmp(arg, "history")) {
        int clan;

        if (!IS_IMMORTAL(ch) || arg2[0] == '\0' || !is_number(arg2))
            do_treasury_history(ch, ch->pcdata->clan);
        else if ((clan = atoi(arg2)) < 0 || clan >= MAX_CLAN)
            send_to_char("Invalid clan number.\n\r", ch);
        else
            do_treasury_history(ch, clan);

        return;
    }

    do_help(ch, "treasury");
    return;
}

void do_treasury_history(CHAR_DATA *ch, int clan)
{
    TREASURY_HISTORY_DATA *history, *history_next;
    char tbuf[MIL], nbuf[MIL], abuf[MIL];
    char *type = NULL;
    char a;

    if (treasury[clan].first_history == NULL) {
        send_to_char("There have been no transactions for your treasury.\n\r", ch);
        return;
    }

    send_to_char("@@N@@d.-------------------------------------------------@@g=( @@WClan Treasury History @@g)=@@d-.\n\r", ch);
    send_to_char("@@d| @@gType            @@d| @@gName         @@d|          @@gAmount @@d| @@cDate                     @@d|\n\r", ch);
    send_to_char("@@d|-----------------+--------------+-----------------+--------------------------|\n\r", ch);

    for (history = treasury[clan].first_history; history; history = history_next) {
        history_next = history->next;

        if (history->timestamp + TREASURY_HISTORY_EXPIRE < current_time) {
            free_string(history->who);
            DUNLINK(history, treasury[clan].first_history, treasury[clan].last_history, next, prev);
            continue;
        }

        switch (history->action) {
            case TREASURY_ACTION_DEPOSIT_GOLD:  a = 'y'; type = "@@gDeposit";    break;
            case TREASURY_ACTION_DEPOSIT_QP:    a = 'a'; type = "@@gDeposit";    break;
            case TREASURY_ACTION_WITHDRAW_GOLD: a = 'y'; type = "@@WWithdrawal"; break;
            case TREASURY_ACTION_WITHDRAW_QP:   a = 'a'; type = "@@WWithdrawal"; break;
            default:                            a = 'd'; type = "@@eWAR";        break;
        }

        sendf(ch, "@@d| %s @@d| @@g%s @@d| @@%c%s @@g%s @@d| @@c%.24s @@d|\n\r",
            my_left(type, tbuf, 15),
            my_left(history->who, nbuf, 12),
            a, my_right(number_comma(history->amount), abuf, 12), a == 'y' ? "GP" : "QP",
            ctime(&history->timestamp));
    }

    send_to_char("@@d'-----------------------------------------------------------------------------'@@N\n\r", ch);
    return;
}
