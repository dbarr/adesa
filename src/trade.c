#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include "merc.h"
#include "auction.h"

IDSTRING(rcsid, "$Id: trade.c,v 1.10 2003/08/30 04:59:17 dave Exp $");

extern bool can_save args((CHAR_DATA *ch, OBJ_DATA *obj));

void
trade_clear(CHAR_DATA *ch)
{
    int                 cnt;

    if (IS_NPC(ch) || !ch->pcdata)
        return;

    ch->pcdata->trading_with = NULL;
    ch->pcdata->trading_accepts = FALSE;
    ch->pcdata->trading_room = NULL;
    ch->pcdata->trading_gold = 0;

    for (cnt = 0; cnt < MAX_TRADEITEMS; cnt++)
        ch->pcdata->trading_objs[cnt] = NULL;

    return;
}

void
trade_abort(CHAR_DATA *ch)
{
    if (IS_NPC(ch) || !ch->pcdata)
        return;

    if (ch->pcdata->trading_with == NULL)
        return;

    act("Your trading with $N has been cancelled.", ch, NULL, ch->pcdata->trading_with, TO_CHAR);
    act("Your trading with $n has been cancelled.", ch, NULL, ch->pcdata->trading_with, TO_VICT);

    trade_clear(ch->pcdata->trading_with);
    trade_clear(ch);

    return;
}

void
trade_transfer(CHAR_DATA *ch)
{
    CHAR_DATA          *trading_with;
    OBJ_DATA           *obj;
    int                 cnt, gold, qpcost;
    char                buf[MSL];

    if (IS_NPC(ch) || !ch->pcdata)
        return;

    if ((trading_with = ch->pcdata->trading_with) == NULL)
        return;

    if (IS_NPC(trading_with) || !trading_with->pcdata)
        return;

    for (cnt = 0; cnt < MAX_TRADEITEMS; cnt++) {
        if ((obj = trading_with->pcdata->trading_objs[cnt]) != NULL) {
            obj_from_char(obj);
            obj_to_char(obj, ch);

            if (IS_SET(obj->extra_flags, ITEM_NODROP)) {
                if (IS_SET(obj->extra_flags, ITEM_ANTI_GOOD)
                    || IS_SET(obj->extra_flags, ITEM_ANTI_NEUTRAL)
                    || IS_SET(obj->extra_flags, ITEM_ANTI_EVIL)
                    )
                    qpcost = 2;
                else
                    qpcost = 10;

                trading_with->quest_points -= qpcost;
                sprintf(buf, "@@N$p costs you @@a%d @@NQPs.", qpcost);
                act(buf, trading_with, obj, NULL, TO_CHAR);
            }

            act("@@NYou receive $p@@N!", ch, obj, NULL, TO_CHAR);
        }
    }

    if ((gold = trading_with->pcdata->trading_gold) > 0) {
        trading_with->balance -= gold;
        ch->balance += gold;
        sprintf(buf, "@@NYou receive @@y%s @@NGPs!\n\r", number_comma(gold));
        send_to_char(buf, ch);
    }

    return;
}

void
trade_finish(CHAR_DATA *ch)
{
    CHAR_DATA          *trading_with;

    if (IS_NPC(ch) || !ch->pcdata)
        return;

    if ((trading_with = ch->pcdata->trading_with) == NULL)
        return;

    if (IS_NPC(trading_with) || !trading_with->pcdata)
        return;

    send_to_char("Trading is complete!\n\r", ch);
    send_to_char("Trading is complete!\n\r", trading_with);

    trade_transfer(trading_with);
    trade_transfer(ch);

    save_char_obj(trading_with);
    save_char_obj(ch);

    trade_clear(trading_with);
    trade_clear(ch);

    return;
}

int
trade_itemcount(CHAR_DATA *ch)
{
    int                 cnt, amt = 0;

    for (cnt = 0; cnt < MAX_TRADEITEMS; cnt++)
        if (ch->pcdata->trading_objs[cnt] != NULL)
            amt += get_obj_number(ch->pcdata->trading_objs[cnt]);

    return amt;
}

int
trade_itemweight(CHAR_DATA *ch)
{
    int                 cnt, amt = 0;

    for (cnt = 0; cnt < MAX_TRADEITEMS; cnt++)
        if (ch->pcdata->trading_objs[cnt] != NULL)
            amt += get_obj_weight(ch->pcdata->trading_objs[cnt]);

    return amt;
}

void
do_trade(CHAR_DATA *ch, char *argument)
{
    char                buf[MSL];
    char                arg1[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim;
    OBJ_DATA           *obj;
    OBJ_DATA           *pObj;
    int                 cnt;
    int                 qpcost = 0;

    if (IS_NPC(ch) || !ch->pcdata)
        return;

    if (*argument == '\0') {
        do_help(ch, "trade");
        return;
    }

    if (ch->in_room && ch->in_room->vnum != 3066) {
        send_to_char("You must be in the Trade Room to use this command.\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg1);

    if (!str_cmp(arg1, "start")) {
        if (*argument == '\0') {
            send_to_char("Start a trade with who?\n\r", ch);
            return;
        }

        if ((victim = get_char_room(ch, argument)) == NULL) {
            send_to_char("They aren't here.\n\r", ch);
            return;
        }

        if (IS_NPC(victim) || !victim->pcdata) {
            send_to_char("Not with NPCs.\n\r", ch);
            return;
        }

        if (ch == victim) {
            send_to_char("You're funny. :P\n\r", ch);
            return;
        }

        if (victim->pcdata->trading_with != NULL) {
            if (victim->pcdata->trading_with != ch)
                send_to_char("They are already in a trade.\n\r", ch);
            else
                send_to_char("They're already trading.. WITH YOU!\n\r", ch);

            return;
        }

        if (ch->pcdata->trading_with != NULL) {
            send_to_char("You're already in a trade.\n\r", ch);
            return;
        }

        if (ch->in_room == NULL || victim->in_room == NULL) {
            send_to_char("Can't find your room!\n\r", ch);
            return;
        }

        trade_clear(ch);
        trade_clear(victim);
        ch->pcdata->trading_with = victim;
        victim->pcdata->trading_with = ch;
        ch->pcdata->trading_room = ch->in_room;
        victim->pcdata->trading_room = victim->in_room;

        act("You are now trading with $N.", ch, NULL, victim, TO_CHAR);
        act("You are now trading with $n.", ch, NULL, victim, TO_VICT);

        return;
    }

    if ((victim = ch->pcdata->trading_with) == NULL) {
        send_to_char("@@NYou must start a trade first. See @@ahelp trade@@N for details.\n\r", ch);
        return;
    }

    if (!str_cmp(arg1, "abort")) {
        trade_abort(ch);
        return;
    }

    if (!str_cmp(arg1, "add")) {
        extern OBJ_DATA    *quest_object;
        extern OBJ_DATA    *auction_item;
        RENAME_DATA        *rename;

        if (*argument == '\0') {
            send_to_char("Trade what item?\n\r", ch);
            return;
        }

        if (ch->pcdata->trading_accepts || victim->pcdata->trading_accepts) {
            send_to_char("You can't add items while either side has accepted the trade.\n\r", ch);
            return;
        }

        if (ch->pcdata->trading_objs[MAX_TRADEITEMS - 1] != NULL) {
            sprintf(buf, "Sorry, maximum trade items is %d.\n\r", MAX_TRADEITEMS);
            send_to_char(buf, ch);

            return;
        }

        if ((obj = get_obj_carry(ch, argument)) == NULL) {
            send_to_char("You do not have that item.\n\r", ch);
            return;
        }

        if (obj->wear_loc != WEAR_NONE) {
            send_to_char("You can only trade items in your inventory.\n\r", ch);
            return;
        }

        if (victim->carry_number + trade_itemcount(ch) + get_obj_number(obj) > can_carry_n(victim)) {
            act("$N has $S hands full.", ch, NULL, victim, TO_CHAR);
            return;
        }

        if (victim->carry_weight + trade_itemweight(ch) + get_obj_weight(obj) > can_carry_w(victim)) {
            act("$N can't carry that much weight.", ch, NULL, victim, TO_CHAR);
            return;
        }

        if (!can_see_obj(victim, obj)) {
            act("$N can't see it.", ch, NULL, victim, TO_CHAR);
            return;
        }

        if (obj->timer > 0) {
            send_to_char("This item has a timer and can't be traded.\n\r", ch);
            return;
        }

        switch (obj->item_type) {
            case ITEM_BEACON:
            case ITEM_PORTAL:
            case ITEM_FURNITURE:
            case ITEM_MONEY:
            case ITEM_STAKE:
            case ITEM_BOAT:
            case ITEM_CORPSE_NPC:
            case ITEM_CORPSE_PC:
            case ITEM_FOUNTAIN:
            case ITEM_BOARD:
            case ITEM_SPELL_MATRIX:
                send_to_char("You cannot trade this item type.\n\r", ch);
                return;
                break;
        }

        if (obj->item_type == ITEM_CONTAINER && obj->first_in_carry_list != NULL) {
            send_to_char("You can only trade empty containers.\n\r", ch);
            return;
        }

        if (obj == quest_object) {
            send_to_char("Wouldn't returning the quest item be better?\n\r", ch);
            return;
        }

        if (obj == auction_item) {
            send_to_char("You can't trade an item that's being auctioned.\n\r", ch);
            return;
        }

        if (obj->pIndexData && (obj->pIndexData->vnum == 13 || obj->pIndexData->vnum == 14)) {
            send_to_char("You can't trade Realm Equipment.\n\r", ch);
            return;
        }

        for (rename = first_rename; rename != NULL; rename = rename->next)
            if (rename->id == obj->id) {
                send_to_char("You can't trade an item that is submitted to be renamed.\n\r", ch);
                return;
            }

        qpcost = 0;
        for (cnt = 0; cnt < MAX_TRADEITEMS; cnt++) {
            if ((pObj = ch->pcdata->trading_objs[cnt]) == NULL)
                continue;

            if (IS_SET(pObj->extra_flags, ITEM_NODROP)) {
                if (IS_SET(pObj->extra_flags, ITEM_ANTI_GOOD)
                    || IS_SET(pObj->extra_flags, ITEM_ANTI_NEUTRAL)
                    || IS_SET(pObj->extra_flags, ITEM_ANTI_EVIL)
                    )
                    qpcost += 2;
                else
                    qpcost += 10;
            }

            if (obj == pObj) {
                send_to_char("You are already trading that item.\n\r", ch);
                return;
            }
        }

        if (IS_SET(obj->extra_flags, ITEM_NODROP)) {
            if (IS_SET(obj->extra_flags, ITEM_ANTI_GOOD)
                || IS_SET(obj->extra_flags, ITEM_ANTI_NEUTRAL)
                || IS_SET(obj->extra_flags, ITEM_ANTI_EVIL)
                ) {
                if (available_qps(ch) - qpcost < 2) {
                    send_to_char("@@NIt costs @@a2 @@NQPs to transfer an anti-aligned nodrop item.\n\r", ch);
                    return;
                }
                else {
                    send_to_char("@@NWarning: Transferring this anti-aligned nodrop item will cost you @@a2 @@NQPs.\n\r", ch);
                }
            }
            else {
                if (available_qps(ch) - qpcost < 10) {
                    send_to_char("@@NIt costs @@a10 @@NQPs to trade a nodrop item.\n\r", ch);
                    return;
                }
                else {
                    send_to_char("@@NWarning: Trading this nodrop item will cost you @@a10 @@NQPs.\n\r", ch);
                }
            }
        }

        act("You add $p to the trading list.", ch, obj, NULL, TO_CHAR);
        act("$n adds $p to the trading list.", ch, obj, victim, TO_VICT);

        if (obj->item_type == ITEM_QUEST && obj->value[3] != 0 && get_pseudo_level(victim) > obj->value[3])
            send_to_char("Warning: You aren't low enough level to deposit this quest item.\n\r", victim);

        if (!can_save(victim, obj))
            send_to_char("Warning: You can't save this item.\n\r", victim);

        for (cnt = 0; cnt < MAX_TRADEITEMS; cnt++)
            if (ch->pcdata->trading_objs[cnt] == NULL)
                break;

        ch->pcdata->trading_objs[cnt] = obj;

        return;
    }

    if (!str_cmp(arg1, "accept")) {
        if (ch->pcdata->trading_accepts) {
            send_to_char("You've already accepted.\n\r", ch);
            return;
        }

        ch->pcdata->trading_accepts = TRUE;

        act("You accept the trade.", ch, NULL, NULL, TO_CHAR);

        if (!victim->pcdata->trading_accepts)
            act("@@N$n accepts the trade. Type @@atrade accept@@N to agree and finish the trade.", ch, NULL, victim, TO_VICT);
        else {
            act("$n accepts the trade.", ch, NULL, victim, TO_VICT);
            trade_finish(ch);
        }

        return;
    }

    if (!str_cmp(arg1, "gold")) {
        int                 aucamt = 0;
        int                 amt = 0;

        if (*argument == '\0') {
            send_to_char("Trade how much gold?\n\r", ch);
            return;
        }

        if (ch->pcdata->trading_accepts || victim->pcdata->trading_accepts) {
            send_to_char("You can't add gold while either side has accepted the trade.\n\r", ch);
            return;
        }

        if (ch->pcdata->trading_gold > 0) {
            send_to_char("You have already set a gold value to trade.\n\r", ch);
            return;
        }

        aucamt = available_gold(ch);

        if ((amt = abs(atoi(argument))) > aucamt) {
            sprintf(buf, "@@NHow can you trade @@y%s @@NGP ", number_comma(amt));
            send_to_char(buf, ch);
            sprintf(buf, "when you only have @@y%s @@NGP available?", number_comma(aucamt));
            send_to_char(buf, ch);

            if (aucamt > 0) {
                sprintf(buf, " (the rest, @@y%s @@NGP, is reserved for auctions)", number_comma(ch->balance - aucamt));
                send_to_char(buf, ch);
            }

            send_to_char("\n\r", ch);

            return;
        }

        ch->pcdata->trading_gold = amt;
        sprintf(buf, "@@NYou set the gold trade to @@y%s @@NGP.\n\r", number_comma(amt));
        send_to_char(buf, ch);
        sprintf(buf, "@@N%s sets the gold trade to @@y%s @@NGP.\n\r", PERS(ch, victim), number_comma(amt));
        send_to_char(buf, victim);

        return;
    }

    if (!str_cmp(arg1, "id")) {
        int                 num = 0;

        if (*argument == '\0') {
            send_to_char("Identify which number trade item?\n\r", ch);
            return;
        }

        if ((num = abs(atoi(argument))) < 1 || num > trade_itemcount(victim)) {
            send_to_char("That number item doesn't exist.\n\r", ch);
            return;
        }

        pObj = NULL;

        for (cnt = 0; cnt < MAX_TRADEITEMS; cnt++)
            if (num == cnt + 1 && (pObj = victim->pcdata->trading_objs[cnt]) != NULL)
                break;

        if (pObj == NULL) {
            send_to_char("That number item doesn't exist.\n\r", ch);
            return;
        }

        spell_identify(0, 0, ch, pObj, NULL);

        return;
    }

    if (!str_cmp(arg1, "list")) {
        bool foundch = FALSE, foundvictim = FALSE;

        send_to_char("@@N@@gYou are trading:\n\r\n\r", ch);

        if (ch->pcdata->trading_gold > 0) {
            sprintf(buf, "Gold:  @@y%s@@g GP.\n\r", number_comma(ch->pcdata->trading_gold));
            send_to_char(buf, ch);
        }

        for (cnt = 0; cnt < MAX_TRADEITEMS; cnt++) {
            if ((obj = ch->pcdata->trading_objs[cnt]) != NULL) {
                if (!foundch)
                    send_to_char("Items:\n\r", ch);

                sprintf(buf, "@@g       %s@@N\n\r", obj->short_descr);
                send_to_char(buf, ch);
                foundch = TRUE;
            }
        }

        if (!foundch && ch->pcdata->trading_gold <= 0)
            send_to_char("@@g   Nothing yet.\n\r\n\r", ch);
        else
            send_to_char("\n\r", ch);

        act("@@N@@gto $N@@N@@g, in return for:\n\r", ch, NULL, victim, TO_CHAR);

        if (victim->pcdata->trading_gold > 0) {
            sprintf(buf, "Gold:  @@y%s@@g GP.\n\r", number_comma(victim->pcdata->trading_gold));
            send_to_char(buf, ch);
        }

        for (cnt = 0; cnt < MAX_TRADEITEMS; cnt++) {
            if ((obj = victim->pcdata->trading_objs[cnt]) != NULL) {
                if (!foundvictim) {
                    send_to_char("Items:\n\r", ch);
                }

                sprintf(buf, "@@W[%2d]@@g   %s@@N\n\r", cnt + 1, obj->short_descr);
                send_to_char(buf, ch);

                if (obj->item_type == ITEM_QUEST && obj->value[3] != 0 && get_pseudo_level(ch) > obj->value[3])
                    send_to_char("@@g  Warning: You aren't low enough level to deposit this quest item.@@N\n\r", ch);

                if (!can_save(ch, obj))
                    send_to_char("@@g  Warning: You can't save this item.@@N\n\r", ch);

                foundvictim = TRUE;
            }
        }

        if (!foundvictim && victim->pcdata->trading_gold <= 0)
            send_to_char("@@g   Nothing yet.@@N\n\r", ch);

        return;
    }

    return;
}
