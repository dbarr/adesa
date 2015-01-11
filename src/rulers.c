/* This file was basically rewritten from scratch by Erigol ;) */

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

#include "merc.h"
#include "tables.h"

IDSTRING(rcsid, "$Id: rulers.c,v 1.17 2004/10/25 19:37:41 dave Exp $");

#define RULER_REVISION 1
int ruler_revision = 0;

RULER_DATA *
get_ruler(CHAR_DATA *ch)
{
    RULER_DATA *ruler;

    for (ruler = first_ruler; ruler != NULL; ruler = ruler->next)
        if (!str_cmp(ch->name, ruler->name))
            return ruler;

    return NULL;
}

void
save_rulers(void)
{

    FILE               *fp;
    char                ruler_file_name[MAX_STRING_LENGTH];
    RULER_DATA         *ruler;

    if (nosave)
        return;

    fclose(fpReserve);
    sprintf(ruler_file_name, "%s", RULERS_FILE);

    if ((fp = fopen(ruler_file_name, "w")) == NULL) {
        bug("Save ruler list: fopen");
        perror("failed open of rulers.lst in save_ruler");
    }
    else {
        FPRINTF(fp, "X %d\n", RULER_REVISION);

        for (ruler = first_ruler; ruler != NULL; ruler = ruler->next)
            FPRINTF(fp, "R %s~ W%s~ %s~ %d %d %d\n", ruler->name, ruler->whoname, ruler->rank, ruler->clan, (int)ruler->realmtime, ruler->hide);

        FPRINTF(fp, "E\n");
    }

    fclose(fp);

    fpReserve = fopen(NULL_FILE, "r");
    return;

}

/*
   #define RKEY( literal, field, value )  if ( !str_cmp( word, literal ) ) { field  = value; fMatch = TRUE;  break;}
   #define RSKEY( literal, field, value )  if ( !str_cmp( word, literal ) ) { if (field!=NULL) free_string(field);field  = value; fMatch = TRUE;  break;}
 */

void
load_rulers(void)
{

    FILE               *rulersfp;
    char                rulers_file_name[MAX_STRING_LENGTH];
    char                buf[MAX_STRING_LENGTH];
    RULER_DATA         *ruler;

    sprintf(rulers_file_name, "%s", RULERS_FILE);

    if ((rulersfp = fopen(rulers_file_name, "r")) == NULL) {
        bugf("load_rulers(): fopen failed on %s", rulers_file_name);
        return;
    }

    for (;;) {
        char c = fread_letter(rulersfp);

        if (c == 'R') {
            GET_FREE(ruler, ruler_free);
            ruler->name      = fread_string(rulersfp);
            temp_fread_string(rulersfp, buf);
            ruler->rank      = fread_string(rulersfp);
            ruler->clan      = fread_number(rulersfp);
            ruler->realmtime = (time_t)fread_number(rulersfp);

            if (ruler_revision >= 1)
                ruler->hide = fread_number(rulersfp) ? TRUE : FALSE;
            else
                ruler->hide = FALSE;

            (void)fgetc(rulersfp); /* grab the \n so fread_letter will work next loop */

            ruler->whoname = str_dup(buf + 1);
            LINK(ruler, first_ruler, last_ruler, next, prev);
        }
        else if (c == 'X') {
            ruler_revision = fread_number(rulersfp);
            (void)fgetc(rulersfp);
        }
        else
            break;
    }

    fclose(rulersfp);
    return;
}

void
do_rulers(CHAR_DATA *ch, char *argument)
{
    RULER_DATA         *ruler;
    char                buf[MSL];
    char                buf2[MSL];
    char                buf3[MSL];
    int                 clan_totals[MAX_CLAN];
    int                 cnt, cnt2;
    bool                found = FALSE;
    struct tm           *tmv = NULL;

    if (IS_NPC(ch)) {
        send_to_char("This option is not for NPCs.\n\r", ch);
        return;
    }

    if (*argument && IS_IMMORTAL(ch) && get_trust(ch) >= 88) {
        argument = one_argument(argument, buf);
        argument = one_argument(argument, buf2);

        if (!str_cmp("add", buf)) {
            CHAR_DATA *victim;

            if (buf2[0] == '\0') {
                send_to_char("syntax: rulers add <player> [hide]\n\r", ch);
                return;
            }

            if ((victim = get_char_world(ch, buf2)) == NULL) {
                send_to_char("No such person.\n\r", ch);
                return;
            }

            if (IS_NPC(victim)) {
                send_to_char("Not on NPCs.\n\r", ch);
                return;
            }

            GET_FREE(ruler, ruler_free);
            ruler->name      = str_dup(victim->name);
            ruler->whoname   = str_dup(victim->pcdata->who_name);
            ruler->rank      = str_dup("@@WLord@@N");
            ruler->clan      = victim->pcdata->clan;
            ruler->realmtime = current_time;
            ruler->hide      = !str_cmp("hide", argument) ? TRUE : FALSE;
            LINK(ruler, first_ruler, last_ruler, next, prev);

            save_rulers();
            return;
        }
        else if (!str_cmp("del", buf)) {
            if (buf2[0] == '\0') {
                send_to_char("syntax: rulers del <player>\n\r", ch);
                return;
            }

            for (ruler = first_ruler; ruler; ruler = ruler->next)
                if (!str_cmp(ruler->name, buf2))
                    break;

            if (!ruler) {
                send_to_char("Unable to find that ruler.\n\r", ch);
                return;
            }

            free_string(ruler->name);
            free_string(ruler->whoname);
            free_string(ruler->rank);

            UNLINK(ruler, first_ruler, last_ruler, next, prev);
            PUT_FREE(ruler, ruler_free);

            save_rulers();
            return;
        }
        else if (!str_cmp("hide", buf)) {
            if (buf2[0] == '\0') {
                send_to_char("syntax: rulers hide <player>\n\r", ch);
                return;
            }

            for (ruler = first_ruler; ruler; ruler = ruler->next)
                if (!str_cmp(ruler->name, buf2))
                    break;

            if (!ruler) {
                send_to_char("Unable to find that ruler.\n\r", ch);
                return;
            }

            ruler->hide = !ruler->hide;
            save_rulers();
            return;
        }

        send_to_char("syntax: rulers <add|del|hide> <player> [hide]\n\r", ch);
        return;
    }

    for (cnt = 0; cnt < MAX_CLAN; cnt++)
        clan_totals[cnt] = 0;

    for (ruler = first_ruler; ruler; ruler = ruler->next) {
        if (ruler->clan < 0 || ruler->clan >= MAX_CLAN)
            continue;

        if (ruler->hide == FALSE || IS_IMMORTAL(ch)) {
            clan_totals[ruler->clan]++;
            if (!found) {
                send_to_char("@@N@@d.-------------------------------------------------@@g=( @@WRuler List @@g)=-@@d.\n\r", ch);
                send_to_char("@@d|   @@gRuler Rank @@d| @@gName          @@d| @@gClan  @@d|    @@gWhoname     @@d| @@gWhen     @@d|\n\r", ch);
                send_to_char("@@d|--------------+---------------+-------+----------------+----------|\n\r", ch);

                found = TRUE;
            }

            tmv = localtime(&ruler->realmtime);
            buf3[0] = 0;
            if (ruler->realmtime > 0) {
                strftime(buf3, 9, "%b %Y", tmv);
            }

            if (!IS_IMMORTAL(ch))
                sendf(ch, "@@N@@d|@@N %s @@N@@d| @@g%s @@N@@d| @@N%s @@N@@d| @@g%s @@N@@d| @@g%8.8s @@d|@@N\n\r",
                    my_right(ruler->rank, buf, 12),
                    my_left(ruler->name, buf2, 13),
                    clan_table[ruler->clan].clan_abbr,
                    ruler->whoname,
                    buf3
                );
            else
                sendf(ch, "@@N@@d|@@N %s @@N@@d|%s@@g%s @@N@@d| @@N%s @@N@@d| @@g%s @@N@@d| @@g%8.8s @@d|@@N\n\r",
                    my_right(ruler->rank, buf, 12),
                    ruler->hide ? "@@y*" : " ",
                    my_left(ruler->name, buf2, 13),
                    clan_table[ruler->clan].clan_abbr,
                    ruler->whoname,
                    buf3
                );
        }
    }

    if (!found) {
        send_to_char("There are no rulers.\n\r", ch);
        return;
    }

    send_to_char("@@d'------------------------------------------------------------------'@@N\n\r", ch);

    send_to_char("\n\r@@N@@d.-------------------------------@@g=( @@WClan Ruler Totals @@g)=-@@d.\n\r", ch);
    send_to_char("@@d| @@gClan                @@WTotal @@d| @@gClan                @@WTotal @@d|\n\r", ch);
    send_to_char("@@d|---------------------------+---------------------------|\n\r", ch);

    buf[0] = '\0'; buf2[0] = '\0'; buf3[0] = '\0'; cnt2 = 0;

    for (cnt = 0; cnt < MAX_CLAN; cnt++)
        if (clan_totals[cnt] > 0) {
            if (cnt2 % 2 == 0)
                sprintf(buf, "@@d|");

            sprintf(buf2, "%d", clan_totals[cnt]);
            sprintf(buf + strlen(buf), " @@N%s @@N@@W %s @@d|",
                my_left(clan_table[cnt].clan_abbr, buf3, 23 - strlen(buf2)),
                buf2);

            if (cnt2 % 2 == 1) {
                sprintf(buf + strlen(buf), "@@N\n\r");
                send_to_char(buf, ch);
                buf[0] = '\0';
            }

            cnt2++;
        }

    if (cnt2 % 2 == 1) {
        sprintf(buf + strlen(buf), "@@d                           |@@N\n\r");
        send_to_char(buf, ch);
    }

    send_to_char("@@d'-------------------------------------------------------'@@N\n\r", ch);
    return;
}
