/*
 *      Ack!Mud 4.0
 *  Zenithar
 *  qpspend.c--handles setting up rulers for areas
 *  Copyright Stephen Zepp 1997
 *
 *  This code may be freely shared and used by anyone, as
 *  long as credit is given to the author and Ack!Mud 4.0
 *
 */

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

#include "merc.h"
#include "tables.h"
#include "auction.h"

IDSTRING(rcsid, "$Id: spendqp.c,v 1.14 2004/06/18 18:21:28 dave Exp $");

void
save_brands()
{

    FILE               *fp;
    char                brand_file_name[MAX_STRING_LENGTH];
    DL_LIST            *brand;
    BRAND_DATA         *this_brand;

    if (nosave)
        return;

    fclose(fpReserve);
    sprintf(brand_file_name, "%s", BRANDS_FILE);

    if ((fp = fopen(brand_file_name, "w")) == NULL) {
        bug("Save brands list: fopen");
        perror("failed open of brands.lst in save_brands");
    }
    else {
        for (brand = first_brand; brand != NULL; brand = brand->next) {
            this_brand = brand->this_one;
            FPRINTF(fp, "#BRAND~\n");
            FPRINTF(fp, "%s~\n", this_brand->branded);
            FPRINTF(fp, "%s~\n", this_brand->branded_by);
            FPRINTF(fp, "%s~\n", this_brand->dt_stamp);
            FPRINTF(fp, "%s~\n", this_brand->message);
            FPRINTF(fp, "%s~\n", this_brand->priority);

        }
        FPRINTF(fp, "#END~\n\n");
    }

    fflush(fp);
    fclose(fp);

    fpReserve = fopen(NULL_FILE, "r");
    return;

}

void
load_brands(void)
{

    FILE               *brandsfp;
    char                brands_file_name[MAX_STRING_LENGTH];
    char                buf[MAX_STRING_LENGTH];
    BRAND_DATA         *this_brand;
    DL_LIST            *brand_member;

    sprintf(brands_file_name, "%s", BRANDS_FILE);

    sprintf(buf, "Loading %s\n\r", brands_file_name);
    monitor_chan(buf, MONITOR_CLAN);

    if ((brandsfp = fopen(brands_file_name, "r")) == NULL) {
        bug("Load brands Table: fopen");
        perror("failed open of brands_table.dat in load_brands_table");
    }
    else {
        for (;;) {

            char               *word;

            word = fread_string(brandsfp);
            if (!str_cmp(word, "#BRAND")) {
                GET_FREE(this_brand, brand_data_free);
                GET_FREE(brand_member, dl_list_free);
                this_brand->branded = fread_string(brandsfp);
                this_brand->branded_by = fread_string(brandsfp);
                this_brand->dt_stamp = fread_string(brandsfp);
                this_brand->message = fread_string(brandsfp);
                this_brand->priority = fread_string(brandsfp);

                free_string(word);

                brand_member->this_one = this_brand;
                brand_member->next = NULL;
                brand_member->prev = NULL;
                LINK(brand_member, first_brand, last_brand, next, prev);

            }
            else if (!str_cmp(word, "#END")) {
                free_string(word);
                break;
            }
            else {
                free_string(word);
                monitor_chan("Load_brands: bad section.", MONITOR_BAD);
                break;
            }
        }

        fclose(brandsfp);

        sprintf(buf, "Done Loading %s\n\r", brands_file_name);
        monitor_chan(buf, MONITOR_CLAN);

    }
}

void
do_qpspend(CHAR_DATA *ch, char *argument)
{
    char                arg1[MAX_INPUT_LENGTH];
    char                buf[MAX_STRING_LENGTH];
    char                brandbuf[MSL];
    char                catbuf[MSL];

    sprintf(brandbuf, "%s", "");
    sprintf(catbuf, "%s", "");

    smash_tilde(argument);
    argument = one_argument(argument, arg1);
    /*    argument = one_argument( argument, arg2 );  */

    if (IS_NPC(ch)) {
        send_to_char("Not for NPC's.\n\r", ch);
        return;
    }

    if (arg1[0] == '\0') {
        send_to_char("@@gsyntax: qpspend enter <message>\n\r", ch);
        send_to_char("                exit <message>\n\r", ch);
        send_to_char("                flag <item> [-]<glow|magic|hum|evil|invis>\n\r", ch);
        send_to_char("                rescue\n\r", ch);
        send_to_char("                home\n\r", ch);
        send_to_char("                corpse\n\r", ch);
        send_to_char("                mobdeath\n\r", ch);
        send_to_char("\n\r", ch);
        send_to_char("See @@ahelp qpspend@@g for details.@@N\n\r", ch);

        return;
    }

    if (!str_cmp(arg1, "enter")) {
        if (my_strlen(argument) > 128) {
            send_to_char("That enter message is too long.\n\r", ch);
            return;
        }

	if (argument[0] == '\0') {
            sendf(ch, "Your current enter message is: @@N%s@@N.\n", ch->pcdata->room_enter);
            return;
        }

        strip_set(argument, "$");
        free_string(ch->pcdata->pedit_string[0]);
        ch->pcdata->pedit_string[0] = str_dup(argument);
        return;
    }
    if (!str_cmp(arg1, "exit")) {
        if (my_strlen(argument) > 128) {
            send_to_char("That exit message is too long.\n\r", ch);
            return;
        }

        if (argument[0] == '\0') {
            sendf(ch, "Your current exit message is: @@N%s@@N.\n", ch->pcdata->room_exit);
            return;
        }

        strip_set(argument, "$");
        free_string(ch->pcdata->pedit_string[1]);
        ch->pcdata->pedit_string[1] = str_dup(argument);
        return;
    }

    if (!str_cmp(arg1, "flag")) {
        OBJ_DATA           *obj;
        char                arg2[MAX_INPUT_LENGTH];
        char                arg3[MAX_INPUT_LENGTH];
        unsigned int        flag;
        bool                remove = FALSE;

        strcpy(arg1, argument);
        argument = one_argument(argument, arg2);
        one_argument(argument, arg3);

        if (arg3[0] == '\0') {
            send_to_char("syntax: qpspend flag <item> [-]<glow|magic|hum|evil|invis>\n\r", ch);
            return;
        }

        if ((obj = get_obj_list(ch, arg2, ch->first_carry)) == NULL) {
            send_to_char("Cannot find that object.\n\r", ch);
            return;
        }

        if (*argument == '-') {
            remove = TRUE;
            argument++;
        }
        flag = table_lookup(tab_obj_flags, argument);

        switch (flag) {
            case ITEM_GLOW:
            case ITEM_HUM:
            case ITEM_EVIL:
            case ITEM_MAGIC:
            case ITEM_INVIS:
                break;
            default:
                send_to_char("Invalid flag type. Valid types are: glow magic hum evil invis\n\r", ch);
                return;
                break;
        }

        if (IS_SET(obj->extra_flags, flag) && !remove) {
            send_to_char("That object already has that flag.\n\r", ch);
            return;
        }

        if (!IS_SET(obj->extra_flags, flag) && remove) {
            send_to_char("You can't remove a flag that doesn't exist in the first place!\n\r", ch);
            return;
        }

        ch->pcdata->pedit_string[2] = str_dup(arg1);
        send_to_char("Type qpspend show to confirm your settings, then qpspend buy to purchase.\n\r", ch);
        return;
    }

    if (!str_cmp(arg1, "show")) {
        char                move_buf[MSL];
        char                test_string[MSL];
        sh_int              qp_cost = 0;

        if (!str_cmp(ch->pcdata->pedit_string[0], "none")) {
            sprintf(test_string, ch->pcdata->room_enter);
        }
        else {
            sprintf(test_string, ch->pcdata->pedit_string[0]);
            qp_cost++;
        }

        sprintf(move_buf, "$R$n %s $T.", test_string);

        act(move_buf, ch, NULL, rev_name[1], TO_CHAR);

        if (!str_cmp(ch->pcdata->pedit_string[1], "none")) {
            sprintf(test_string, ch->pcdata->room_exit);
        }
        else {
            sprintf(test_string, ch->pcdata->pedit_string[1]);
            qp_cost++;
        }

        sprintf(move_buf, "$R$n %s $T.", test_string);

        act(move_buf, ch, NULL, dir_name[1], TO_CHAR);

        if (str_cmp(ch->pcdata->pedit_string[2], "none")) {
            OBJ_DATA           *obj;
            char                arg2[MAX_INPUT_LENGTH];
            char                arg3[MAX_INPUT_LENGTH];
            char                buf[MSL];
            char               *argm = ch->pcdata->pedit_string[2];
            bool                remove = FALSE;
            unsigned int        flag;

            argm = one_argument(ch->pcdata->pedit_string[2], arg2);
            one_argument(ch->pcdata->pedit_string[2], arg3);

            if (arg3[0] != '\0') {
                if ((obj = get_obj_list(ch, arg2, ch->first_carry)) != NULL) {
                    if (*argm == '-') {
                        remove = TRUE;
                        argm++;
                    }

                    flag = table_lookup(tab_obj_flags, argm);

                    if (flag > 0) {
                        sprintf(buf, "Object %s %s with flag: %s\n\r",
                            obj->short_descr, !remove ? "set" : "unset", bit_table_lookup(tab_obj_flags, flag)
                            );
                        send_to_char(buf, ch);
                        qp_cost++;
                    }
                }
            }
        }
        sprintf(buf, "Purchase cost is %d qps.\n\r", qp_cost);
        send_to_char(buf, ch);
        return;
    }

    if (!str_cmp(arg1, "buy")) {
        sh_int              qp_cost = 0;
        char               *strtime;
        sh_int              i;

        if (str_cmp(ch->pcdata->pedit_string[0], "none"))
            qp_cost++;
        if (str_cmp(ch->pcdata->pedit_string[1], "none"))
            qp_cost++;
        if (str_cmp(ch->pcdata->pedit_string[2], "none"))
            qp_cost++;
        if (available_qps(ch) < qp_cost) {
            send_to_char("You don't have enough quest points!\n\r", ch);
            for (i = 0; i < 5; i++) {
                if (str_cmp(ch->pcdata->pedit_string[i], "none")) {
                    free_string(ch->pcdata->pedit_string[i]);
                    ch->pcdata->pedit_string[i] = str_dup("none");
                }
            }
            return;
        }
        else if (qp_cost == 0) {
            send_to_char("No changes.\n\r", ch);
            return;
        }
        else {
            if (str_cmp(ch->pcdata->pedit_string[0], "none")) {
                free_string(ch->pcdata->room_enter);
                ch->pcdata->room_enter = str_dup(ch->pcdata->pedit_string[0]);
                free_string(ch->pcdata->pedit_string[0]);
                ch->pcdata->pedit_string[0] = str_dup("none");
                sprintf(catbuf, "Enter message changed to %s\n\r", ch->pcdata->room_enter);
                safe_strcat(MSL, brandbuf, catbuf);
            }
            if (str_cmp(ch->pcdata->pedit_string[1], "none")) {
                free_string(ch->pcdata->room_exit);
                ch->pcdata->room_exit = str_dup(ch->pcdata->pedit_string[1]);
                free_string(ch->pcdata->pedit_string[1]);
                ch->pcdata->pedit_string[1] = str_dup("none");
                sprintf(catbuf, "Exit message changed to %s\n\r", ch->pcdata->room_exit);
                safe_strcat(MSL, brandbuf, catbuf);
            }
            if (str_cmp(ch->pcdata->pedit_string[2], "none")) {
                OBJ_DATA           *obj;
                char                arg1[MAX_INPUT_LENGTH];
                char                arg2[MAX_INPUT_LENGTH];
                char                buf[MSL];
                char               *argument = ch->pcdata->pedit_string[2];
                bool                remove = FALSE;
                unsigned int        flag;

                argument = one_argument(ch->pcdata->pedit_string[2], arg1);
                one_argument(ch->pcdata->pedit_string[2], arg2);

                if (arg2[0] != '\0') {
                    if ((obj = get_obj_list(ch, arg1, ch->first_carry)) != NULL) {
                        if (*argument == '-') {
                            remove = TRUE;
                            argument++;
                        }

                        flag = table_lookup(tab_obj_flags, argument);

                        if (flag > 0) {
                            if (!remove)
                                SET_BIT(obj->extra_flags, flag);
                            else
                                REMOVE_BIT(obj->extra_flags, flag);

                            sprintf(buf, "Object %s %s with flag: %s\n\r",
                                obj->short_descr, !remove ? "set" : "unset", bit_table_lookup(tab_obj_flags, flag)
                                );
                            safe_strcat(MSL, brandbuf, buf);

                        }
                    }
                    else {
                        send_to_char("Object went missing ;).\n\r", ch);
                        qp_cost--;
                    }
                }

                free_string(ch->pcdata->pedit_string[2]);
                ch->pcdata->pedit_string[2] = str_dup("none");
            }

            if (qp_cost > 0) {
                ch->quest_points -= qp_cost;
                do_save(ch, "");
                {
                    BRAND_DATA         *brand;
                    DL_LIST            *brand_member;

                    GET_FREE(brand, brand_data_free);
                    GET_FREE(brand_member, dl_list_free);
                    brand->branded = str_dup(ch->name);
                    brand->branded_by = str_dup("@@rSystem@@N");
                    brand->priority = str_dup("normal");
                    brand->message = str_dup(brandbuf);
                    strtime = ctime(&current_time);
                    strtime[strlen(strtime) - 1] = '\0';
                    brand->dt_stamp = str_dup(strtime);
                    brand_member->next = NULL;
                    brand_member->prev = NULL;
                    brand_member->this_one = brand;
                    LINK(brand_member, first_brand, last_brand, next, prev);
                    save_brands();
                    send_to_char("Your messages have been updated, and logged for inspection by an Immortal.\n\r", ch);
                }
            }
            return;
        }
        return;
    }
    if (!str_cmp(arg1, "rescue")) {
        if (ch->in_room->vnum == 1) {
            send_to_char("No one can rescue you from here.\n\r", ch);
            return;
        }

        if (available_qps(ch) < 10) {
            send_to_char("You don't have enough quest points. Cost is 10 qp.\n\r", ch);
            return;
        }
        else {
            ROOM_INDEX_DATA    *location;

            if ((location = get_room_index(ch->pcdata->recall_vnum)) == NULL)
                location = get_room_index(3001);
            act("Let's blow this pop stand and never look back!", ch, NULL, NULL, TO_CHAR);
            act("@@y$n@@g blows this pop stand and never looks back!", ch, NULL, NULL, TO_ROOM);
            char_from_room(ch);
            char_to_room(ch, location);
            do_look(ch, "");
            act("$n steps into the room from a @@apulsating @@mvortex@@N.", ch, NULL, NULL, TO_ROOM);
            ch->quest_points -= 10;
            do_save(ch, "");
            return;
        }
    }
    if (!str_cmp(arg1, "home")) {
        if (available_qps(ch) < 50) {
            send_to_char("You don't have enough quest points. Cost is 50 qp.\n\r", ch);
            return;
        }
        else {
            if (ch->in_room->sector_type != SECT_RECALL_OK) {
                send_to_char("This is not a legal location to call your home.\n\r", ch);
                return;
            }
            else if ((ch->in_room == NULL) || (ch->in_room->vnum < 3)) {
                send_to_char("You are LOST!\n\r", ch);
                return;
            }
            else {

                ch->pcdata->recall_vnum = ch->in_room->vnum;
                ch->quest_points -= 50;
                do_save(ch, "");
                send_to_char("You know call this room your home, and will recall here!\n\r", ch);
                return;
            }
        }
    }

    if (!str_cmp(arg1, "corpse")) {
        if (available_qps(ch) < 10) {
            send_to_char("You don't have enough quest points. Cost is 10 qp.\n\r", ch);
            return;
        }
        else if ((ch->in_room == NULL) || (ch->in_room->vnum < 3)) {
            send_to_char("You are completly LOST!!\n\r", ch);
            return;
        }
        else {
            OBJ_DATA           *obj;
            bool                found = FALSE;

            for (obj = first_obj; obj != NULL; obj = obj->next) {
                if (((obj->pIndexData->vnum) == OBJ_VNUM_CORPSE_PC)
                    && (!str_cmp(ch->name, obj->owner))
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
                return;
            }
            ch->quest_points -= 10;
            do_save(ch, "");
            return;
        }
    }
    if (!str_cmp("mobdeath", arg1)) {
        if (available_qps(ch) < 10) {
            send_to_char("You don't have enough quest points. Cost is 10 qp.\n\r", ch);
            return;
        }

        if (ch->pcdata->mkilled <= 0) {
            send_to_char("Your mob deaths are already at 0.\n\r", ch);
            return;
        }

        ch->pcdata->mkilled--;
        ch->quest_points -= 10;
        sendf(ch, "Your mob deaths have been lowered from %d to %d.\n\r", ch->pcdata->mkilled + 1, ch->pcdata->mkilled);
        xlogf("%s's mob deaths have been lowered from %d to %d.\n\r", ch->name, ch->pcdata->mkilled + 1, ch->pcdata->mkilled);
        do_save(ch, "");
        return;
    }

    if (!str_cmp(arg1, "clear")) {
        sh_int              i;

        for (i = 0; i < 5; i++) {
            free_string(ch->pcdata->pedit_string[i]);
            ch->pcdata->pedit_string[i] = str_dup("none");
        }
        return;
    }

}

void
do_immbrand(CHAR_DATA *ch, char *argument)
{
    DL_LIST            *brand_list;
    DL_LIST            *this_brand;
    BRAND_DATA         *brand;
    char                buf[MAX_STRING_LENGTH];
    char                buf1[MAX_STRING_LENGTH * 7];
    char                arg[MAX_INPUT_LENGTH];
    int                 vnum = 0;
    int                 anum = 0;

    if (IS_NPC(ch))
        return;

    argument = one_argument(argument, arg);
    smash_tilde(argument);

    if (arg[0] == '\0') {
        do_immbrand(ch, "read");
        return;
    }

    if (!str_cmp(arg, "list")) {
        vnum = 0;
        buf1[0] = '\0';
        for (brand_list = first_brand; brand_list; brand_list = brand_list->next) {
            brand = brand_list->this_one;
            sprintf(buf, "[%3d] @@r%s@@W: @@GBrander@@W: %s  @@a%s @@ePriority: %s@@N\n\r",
                vnum, brand->branded, brand->branded_by, brand->dt_stamp, brand->priority);
            strcat(buf1, buf);
            vnum++;
        }

        if (vnum == 0)
            send_to_char("There are no outstanding brands.\n\r", ch);
        else {
            /* act message */
            send_to_char(buf1, ch);
        }
        return;
    }

    if (!str_cmp(arg, "read")) {
        if (is_number(argument)) {
            anum = atoi(argument);
        }
        else {
            send_to_char("Read which brand?\n\r", ch);
            return;
        }

        vnum = 0;
        buf1[0] = '\0';
        for (brand_list = first_brand; brand_list; brand_list = brand_list->next) {
            if (vnum++ == anum) {
                brand = brand_list->this_one;
                sprintf(buf, "[%3d] @@r%s@@W: @@GBrander@@W: %s  @@a%s @@ePriority: %s@@N\n\r",
                    anum, brand->branded, brand->branded_by, brand->dt_stamp, brand->priority);
                strcat(buf1, buf);
                strcat(buf1, brand->message);
                send_to_char(buf1, ch);
                return;
            }
            else
                continue;
            send_to_char("No such brand.\n\r", ch);
            return;
        }
    }

    if (!str_cmp(arg, "write") || !str_cmp(arg, "edit")) {
        if (ch->current_brand == NULL) {
            GET_FREE(ch->current_brand, brand_data_free);
            ch->current_brand->branded = str_dup("");
            ch->current_brand->branded_by = str_dup("");
            ch->current_brand->message = str_dup("");
            ch->current_brand->dt_stamp = str_dup("");
            ch->current_brand->priority = str_dup("");
        }

        build_strdup(&ch->current_brand->message, "$edit", TRUE, ch);
        return;
    }

    if (!str_cmp(arg, "player")) {
        if (ch->current_brand == NULL) {
            GET_FREE(ch->current_brand, brand_data_free);
            ch->current_brand->branded = str_dup("");
            ch->current_brand->branded_by = str_dup("");
            ch->current_brand->message = str_dup("");
            ch->current_brand->dt_stamp = str_dup("");
            ch->current_brand->priority = str_dup("");
        }

        free_string(ch->current_brand->branded);
        ch->current_brand->branded = str_dup(argument);
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if (!str_cmp(arg, "priority")) {
        if (ch->current_brand == NULL) {
            GET_FREE(ch->current_brand, brand_data_free);
            ch->current_brand->branded = str_dup("");
            ch->current_brand->branded_by = str_dup("");
            ch->current_brand->message = str_dup("");
            ch->current_brand->dt_stamp = str_dup("");
            ch->current_brand->priority = str_dup("");
        }
        free_string(ch->current_brand->priority);
        ch->current_brand->priority = str_dup(argument);
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if (!str_cmp(arg, "clear")) {
        if (ch->current_brand) {
            free_string(ch->current_brand->branded);
            free_string(ch->current_brand->branded_by);
            free_string(ch->current_brand->message);
            free_string(ch->current_brand->dt_stamp);
            free_string(ch->current_brand->priority);
            PUT_FREE(ch->current_brand, brand_data_free);
            ch->current_brand = NULL;
        }
        save_brands();
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if (!str_cmp(arg, "show")) {
        if (!ch->current_brand) {
            send_to_char("You have no brand in progress.\n\r", ch);
            return;
        }

        sprintf(buf, "[%3d] %s: Brander: %s  Date: %s Priority: %s\n\r",
            vnum, ch->current_brand->branded, ch->current_brand->branded_by, ch->current_brand->dt_stamp, ch->current_brand->priority);
        strcat(buf1, buf);
        strcat(buf1, ch->current_brand->message);
        send_to_char(buf1, ch);
        return;
    }

    if (!str_cmp(arg, "post")) {

        char               *strtime;

        if (!ch->current_brand) {
            send_to_char("You have no brand in progress.\n\r", ch);
            return;
        }

        if (!str_cmp(ch->current_brand->branded, "")) {
            send_to_char("You need to provide a player name .\n\r", ch);
            return;
        }

        if (!str_cmp(ch->current_brand->message, "")) {
            send_to_char("You need to provide a message.\n\r", ch);
            return;
        }

        strtime = ctime(&current_time);
        strtime[strlen(strtime) - 1] = '\0';
        free_string(ch->current_brand->dt_stamp);
        ch->current_brand->dt_stamp = str_dup(strtime);
        free_string(ch->current_brand->branded_by);
        ch->current_brand->branded_by = str_dup(ch->name);
        GET_FREE(this_brand, dl_list_free);
        this_brand->next = NULL;
        this_brand->prev = NULL;
        this_brand->this_one = ch->current_brand;
        LINK(this_brand, first_brand, last_brand, next, prev);
        ch->current_brand = NULL;
        save_brands();
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if (!str_cmp(arg, "remove")) {
        if (!is_number(argument)) {
            send_to_char("Remove which letter number?\n\r", ch);
            return;
        }

        anum = atoi(argument);
        vnum = 0;
        for (brand_list = first_brand; brand_list; brand_list = brand_list->next) {
            if (vnum++ == anum) {
                break;
            }
        }
        if (brand_list != NULL) {
            UNLINK(brand_list, first_brand, last_brand, next, prev);
            brand = brand_list->this_one;
            free_string(brand->branded);
            free_string(brand->branded_by);
            free_string(brand->message);
            free_string(brand->dt_stamp);
            free_string(brand->priority);
            PUT_FREE(brand, brand_data_free);
            brand_list->this_one = NULL;
            PUT_FREE(brand_list, dl_list_free);
            save_brands();
            return;
        }

        send_to_char("No such brand.\n\r", ch);
        return;
    }

    send_to_char("Huh?  Type 'help letter' for usage.\n\r", ch);
    return;
}
