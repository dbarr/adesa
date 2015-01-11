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

IDSTRING(rcsid, "$Id: auction.c,v 1.28 2004/06/18 18:21:27 dave Exp $");

AUCTION_DATA       *first_auction = NULL;
AUCTION_DATA       *last_auction = NULL;
int auction_revision = 0;

/* do a gold auction */
void
do_gauction(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch) || !ch->pcdata)
        return;

    auction_do(ch, argument, AUCTION_TYPE_GOLD);
    return;
}

/* do a quest point auction */
void
do_qauction(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch) || !ch->pcdata)
        return;

    auction_do(ch, argument, AUCTION_TYPE_QPS);
    return;
}

/* delete an auction */
void
auction_del(AUCTION_DATA *auc)
{
    if (!auc)
        return;

    if (auc->pObj) {
        /* theoretically the only time this should happen is if the bidders
         * pfile is missing, which should be never because we disallow
         * pdeleting while involved in the auction process.. however, to be on
         * the safe side, we'll log it anyhow.
         */
        xlogf("AUCTION DELETE: keyword[%s] reserve[%d] expire[%d] flags[%d] owner[%s] ownern[%s] bidder[%s] biddern[%s] amount[%d] obj[%s]",
            auc->keyword, auc->reserve, (int) auc->expire_time, auc->flags, auc->owner, auc->owner_name, auc->bidder, auc->bidder_name, auc->amount,
            auc->pObj->short_descr);

        extract_obj(auc->pObj);
    }

    free_string(auc->keyword);
    free_string(auc->owner);
    free_string(auc->owner_name);
    free_string(auc->bidder);
    free_string(auc->bidder_name);
    DUNLINK(auc, first_auction, last_auction, next, prev);
    return;
}

void
auction_end(AUCTION_DATA *auc)
{
    if (!auc->bidder || auc->bidder[0] == '\0') {
        auctionf("@@y[@@b%s@@y] @@NNo one bid on %s@@N! Item returned to %s@@N.", auc->keyword, auc->pObj->short_descr, auc->owner_name);
        auction_give(auc, auc->owner, FALSE);
        auction_del(auc);
        return;
    }

    auction_give(auc, auc->bidder, TRUE);
    auction_del(auc);
    return;
}

/* give the auctioned item to the winner, or if no one won, the owner */
void
auction_give(AUCTION_DATA *auc, char *to, bool spend)
{
    bool                toplay = FALSE;
    bool                topfile = FALSE;
    bool                ownerplay = FALSE;
    bool                ownerpfile = FALSE;
    CHAR_DATA          *ch, *owner = NULL;
    DESCRIPTOR_DATA     d, downer;
    CHAR_DATA          *victim = NULL;

    for (ch = first_player; ch != NULL; ch = ch->next_player)
        if (ch->pcdata && !str_cmp(ch->pcdata->origname, to)) {
            victim = ch;
            toplay = TRUE;
            break;
        }

    /* only find owner if to isn't owner! */
    if (str_cmp(auc->owner, to))
        for (ch = first_player; ch != NULL; ch = ch->next_player)
            if (ch->pcdata && !str_cmp(ch->pcdata->origname, auc->owner)) {
                owner = ch;
                ownerplay = TRUE;
                break;
            }

    if (!toplay) {
        topfile = load_char_obj(&d, to, TRUE);

        if (!topfile) {
            if (spend)
                auctionf("@@y[@@b%s@@y] @@NWinning bidder (%s@@N)'s pfile no longer exists for %s@@N. Item returned to %s@@N.",
                    auc->keyword, auc->bidder_name, auc->pObj->short_descr, auc->owner_name);
            else
                auctionf("@@y[@@b%s@@y] @@NOwner (%s@@N)'s pfile no longer exists for %s@@N. Item DESTROYED.",
                    auc->keyword, auc->owner, auc->pObj->short_descr);

            free_char(d.character);

            if (str_cmp(to, auc->owner))
                auction_give(auc, auc->owner, FALSE);

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

    if (!ownerplay && str_cmp(auc->owner, to)) {
        ownerpfile = load_char_obj(&downer, auc->owner, TRUE);

        if (!ownerpfile) {
            auctionf("Owner (%s)'s pfile no longer exists for %s. They receive no payment!", auc->owner, auc->pObj->short_descr);

            free_char(downer.character);
            owner = NULL;
        }
        else {
            owner = downer.character;
            downer.character = NULL;
            owner->desc = NULL;

            LINK(owner, first_char, last_char, next, prev);
            LINK(owner, first_player, last_player, next_player, prev_player);

            if (owner->in_room != NULL)
                char_to_room(owner, owner->in_room);
            else
                char_to_room(owner, get_room_index(2));
        }
    }

    obj_to_char(auc->pObj, victim);

    if (spend) {
        if (IS_SET(auc->flags, AUCTION_TYPE_GOLD)) {
            victim->balance -= auc->amount;

            if (owner)
                owner->balance += auc->amount;
        }
        else {
            victim->quest_points -= auc->amount;

            if (owner)
                owner->quest_points += auc->amount;
        }

        auctionf("@@y[@@b%s@@y] @@N%s wins the auction of %s @@Nfor %s %s@@N!",
            auc->keyword, auc->bidder_name, auc->pObj->short_descr, number_comma(auc->amount), BIDTYPE(auc)
            );

        if (victim && victim->desc)
            sendf(victim, "@@NYou receive %s for %s %s@@N.\n\r", auc->pObj->short_descr, number_comma(auc->amount), BIDTYPE(auc));

        if (owner && owner->desc)
            sendf(owner, "@@NYou receive %s %s@@N for %s@@N.\n\r", number_comma(auc->amount), BIDTYPE(auc), auc->pObj->short_descr);
    }

    if (victim && !toplay && topfile) {
        save_char_obj(victim);
        victim->is_quitting = TRUE;
        extract_char(victim, TRUE);
    }
    else if (victim && toplay)
        save_char_obj(victim);

    if (owner && !ownerplay && ownerpfile) {
        save_char_obj(owner);
        owner->is_quitting = TRUE;
        extract_char(owner, TRUE);
    }
    else if (owner && ownerplay)
        save_char_obj(owner);

    auc->pObj = NULL;

    return;
}

/* function establishing some limitations to auction objs */
bool
valid_auction_obj(CHAR_DATA *ch, OBJ_DATA *obj, int type, int reserve, int duration)
{
    extern OBJ_DATA    *quest_object;
    extern OBJ_DATA    *auction_item;
    RENAME_DATA        *rename;

    /* immortals can auction anything! */
    if (IS_IMMORTAL(ch))
        return TRUE;

    if (IS_SET(type, AUCTION_TYPE_GOLD)) {
        if (reserve < 5000) {
            send_to_char("@@NMinimum gold reserve is 5,000 @@yGP@@N.\n\r", ch);
            return FALSE;
        }

        if (reserve > 5000000) {
            send_to_char("@@NMaximum gold reserve is 5,000,000 @@yGP@@N.\n\r", ch);
            return FALSE;
        }
    }
    else {
        if (reserve < 1) {
            send_to_char("@@NMinimum quest point reserve is 1 @@aQP@@N.\n\r", ch);
            return FALSE;
        }

        if (reserve > 1000) {
            send_to_char("@@NMaximum quest point reserve is 1,000 @@aQP@@N.\n\r", ch);
            return FALSE;
        }
    }

    if (duration < 60) {
        send_to_char("Minimum auction duration is a minute.\n\r", ch);
        return FALSE;
    }

    if (duration > MAX_AUCTION_DURATION) {
        send_to_char("Maximum auction duration is a week.\n\r", ch);
        return FALSE;
    }

    if (obj->timer > 0) {
        send_to_char("Timered objects can't be auctioned.\n\r", ch);
        return FALSE;
    }

    if (obj->wear_loc != WEAR_NONE) {
        send_to_char("You can only auction items in your inventory.\n\r", ch);
        return FALSE;
    }

    if (obj->item_type == ITEM_QUEST && IS_SET(type, AUCTION_TYPE_QPS)) {
        send_to_char("You can't auction a quest point item for quest points!\n\r", ch);
        return FALSE;
    }

    for (rename = first_rename; rename != NULL; rename = rename->next)
        if (rename->id == obj->id) {
            send_to_char("You can't auction an item that has been submitted to be renamed.\n\r", ch);
            return FALSE;
        }

    if (IS_SET(obj->extra_flags, ITEM_NOSAVE)) {
        send_to_char("You can't auction nosave items.\n\r", ch);
        return FALSE;
    }

    if (IS_SET(obj->extra_flags, ITEM_CLAN_EQ)) {
        send_to_char("You can't auction clan items.\n\r", ch);
        return FALSE;
    }

    if (obj == auction_item) {
        send_to_char("That item is currently being auctioned.\n\r", ch);
        return FALSE;
    }

    if (obj == quest_object) {
        send_to_char("Wouldn't returning the quest object be better?\n\r", ch);
        return FALSE;
    }

    if (obj->pIndexData && (obj->pIndexData->vnum == 13 || obj->pIndexData->vnum == 14)) {
        send_to_char("You can't auction Realm Equipment.\n\r", ch);
        return FALSE;
    }

    if (obj->item_type == ITEM_CONTAINER && obj->first_in_carry_list != NULL) {
        send_to_char("You can only auction empty containers.\n\r", ch);
        return FALSE;
    }

    if (IS_SET(obj->extra_flags, ITEM_NO_AUCTION)) {
        send_to_char("You can't auction that item.\n\r", ch);
        return FALSE;
    }

    switch (obj->item_type) {
        case ITEM_LIGHT:
        case ITEM_SCROLL:
        case ITEM_WAND:
        case ITEM_STAFF:
        case ITEM_WEAPON:
        case ITEM_ARMOR:
        case ITEM_POTION:
        case ITEM_CLUTCH:
        case ITEM_QUEST:
        case ITEM_PILL:
        case ITEM_PIECE:
        case ITEM_CONTAINER:
        case ITEM_ENCHANTMENT:
            break;
        default:
            send_to_char("You can't auction that item type.\n\r", ch);
            return FALSE;
            break;
    }

    return TRUE;
}

/* main function: add, delete, end, bid, create gold/quest point auctions */
void
auction_do(CHAR_DATA *ch, char *argument, int type)
{
    char                buf[MSL], buf2[MSL];
    char                arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH], arg3[MAX_INPUT_LENGTH], arg4[MAX_INPUT_LENGTH];
    AUCTION_DATA       *auc;

    argument = one_argument(argument, arg);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);
    argument = one_argument(argument, arg4);
    buf[0] = 0;
    buf2[0] = 0;

    if (!str_cmp("list", arg)) {
        auction_do_list(ch, type);
        return;
    }

    if (!str_cmp("del", arg) && IS_IMMORTAL(ch)) {
        int                 cnt = 0;
        int                 amt = 0;

        if (arg2[0] == '\0') {
            sendf(ch, "syntax: %s del <keyword>\n\r", AUCCMD(type));
            return;
        }

        if (is_number(arg2))
            amt = abs(atoi(arg2));

        for (auc = first_auction; auc != NULL; auc = auc->next)
            if (IS_SET(auc->flags, type) && (!str_cmp(auc->keyword, arg2) || (amt && ++cnt == amt)))
                break;

        if (auc == NULL) {
            sendf(ch, "Unable to find that %s auction.\n\r", AUCTYPE(type));
            return;
        }

        auctionf("@@y[@@b%s@@y] @@N%s cancels the %s auction for %s@@N. Item returned to %s@@N.", auc->keyword, ch->short_descr, AUCTYPE(type),
            auc->pObj->short_descr, auc->owner_name);
        auction_give(auc, auc->owner, FALSE);
        auction_del(auc);
        save_auctions();
        return;
    }                            /* end: del */

    if (!str_cmp("end", arg) && IS_IMMORTAL(ch)) {
        if (arg2[0] == '\0') {
            sendf(ch, "syntax: %s end <keyword>\n\r", AUCCMD(type));
            return;
        }

        for (auc = first_auction; auc != NULL; auc = auc->next)
            if (IS_SET(auc->flags, type) && !str_cmp(auc->keyword, arg2))
                break;

        if (auc == NULL) {
            sendf(ch, "Unable to find that %s auction.\n\r", AUCTYPE(type));
            return;
        }

        auctionf("@@y[@@b%s@@y] @@N%s prematurely ends the %s auction for %s@@N!", auc->keyword, ch->short_descr, AUCTYPE(type),
            auc->pObj->short_descr);
        auc->expire_time = 0;
        auc_update();
        return;
    }                            /* end: end */

    if (!str_cmp("add", arg) && (type == AUCTION_TYPE_GOLD || ch->adept_level > 0)) {
        OBJ_DATA           *obj;
        char               *keyword;

        int                 reserve = 0;
        int                 dur = 0;
        int                 ttype = 0;

        if (ch->pcdata->trading_with != NULL) {
            send_to_char("Finish your trade first!\n\r", ch);
            return;
        }

        if (arg4[0] == '\0') {
            sendf(ch, "syntax: %s add <item> <reserve> <expire>\n\r", AUCCMD(type));
            return;
        }

        if ((obj = get_obj_carry(ch, arg2)) == NULL) {
            send_to_char("Can't find that object.\n\r", ch);
            return;
        }

        if (!is_number(arg3)) {
            send_to_char("Reserve must be numerical.\n\r", ch);
            return;
        }

        reserve = abs(atoi(arg3));
        dur = dur_to_secs(arg4);

        SET_BIT(ttype, type);

        /* not a valid auction obj */
        if (!valid_auction_obj(ch, obj, ttype, reserve, dur))
            return;

        if (ch->carry_number >= can_carry_n(ch)) {
            send_to_char("Your inventory is full.\n\r", ch);
            return;
        }

        if (ch->carry_weight >= can_carry_w(ch)) {
            send_to_char("Your weight is at maximum.\n\r", ch);
            return;
        }

        if (type == AUCTION_TYPE_GOLD
            && (   ch->gold < 5000
                || (IS_SET(obj->extra_flags, ITEM_NODROP) && available_qps(ch) < 10)
               )
           ) {
            if (ch->gold < 5000)
                send_to_char("@@gIt costs 5,000 @@yGP@@g to add a gold auction item.@@N\n\r", ch);
            else
                send_to_char("@@gIt costs 5,000 @@yGP@@g and @@a10@@g QP to add a nodrop-flagged gold auction item.@@N\n\r", ch);

            return;
        }

        if (   type == AUCTION_TYPE_QPS
            && (   available_qps(ch) < 1
                || (IS_SET(obj->extra_flags, ITEM_NODROP) && available_qps(ch) < 11)
               )
           ) {
            if (!IS_SET(obj->extra_flags, ITEM_NODROP))
                send_to_char("@@gIt costs 1 @@aQP@@g to add a quest point auction item.@@N\n\r", ch);
            else
                send_to_char("@@gIt costs 11 @@aQP@@g to add a nodrop-flagged quest point auction item.@@N\n\r", ch);

            return;
        }

        {
            int                 mine = 0;
            AUCTION_DATA       *mauc;

            for (mauc = first_auction; mauc != NULL; mauc = mauc->next)
                if (!str_cmp(mauc->owner, ch->pcdata->origname))
                    mine++;

            if (mine >= 10) {
                send_to_char("You are auctioning too many items! 10 is the limit.\n\r", ch);
                return;
            }
        }

        if (!argument || argument[0] == '\0') {
            sendf(ch, "@@N@@gYou will be auctioning %s@@N@@g with a reserve of %s %s@@g to expire in @@W%s@@g.\n\r",
                obj->short_descr, number_comma(reserve), BIDTYPE2(ttype), duration(dur, buf2));

            if (type == AUCTION_TYPE_GOLD) {
                if (!IS_SET(obj->extra_flags, ITEM_NODROP))
                    send_to_char("This will cost you 5,000 @@yGP@@g.\n\r", ch);
                else
                    send_to_char("This will cost you 5,000 @@yGP@@g and 10 @@aQP@@g.\n\r", ch);
            }
            else {
                if (!IS_SET(obj->extra_flags, ITEM_NODROP))
                    send_to_char("This will cost you 1 @@aQP@@g.\n\r", ch);
                else
                    send_to_char("This will cost you 11 @@aQP@@g.\n\r", ch);
            }

            strcpy(ch->pcdata->cookies[COOKIE_AUCTION], generate_cookie());
            ch->pcdata->cookiesexpire[COOKIE_AUCTION] = current_time + 60;

            if (type == AUCTION_TYPE_GOLD)
                ch->pcdata->cookies[COOKIE_AUCTION][0] = 'g';
            else
                ch->pcdata->cookies[COOKIE_AUCTION][0] = 'q';

            sendf(ch, "Send this command to add your auction:\n\r\n\r    @@N@@W%s @@N@@W%s @@N@@W%s @@N@@W%s @@N@@W%s @@N@@W%s@@N@@g.\n\r\n\r",
                AUCCMD(type), arg, arg2, arg3, arg4, ch->pcdata->cookies[COOKIE_AUCTION]);

            send_to_char("You have one minute to confirm. Remember to check if this is really the\n\rcorrect item you wish to auction. No reimbursements will be given for\n\raccidentally auctioning the wrong item!@@N\n\r", ch);

            return;
        }
        else if (ch->pcdata->cookies[COOKIE_AUCTION] == NULL || ch->pcdata->cookies[COOKIE_AUCTION][0] == '\0') {
            sendf(ch, "syntax: %s add <item> <reserve> <expire>\n\r", AUCCMD(type));
            return;
        }
        else if (   str_cmp(ch->pcdata->cookies[COOKIE_AUCTION], argument)
                 || (ch->pcdata->cookies[COOKIE_AUCTION][0] == 'g' && type != AUCTION_TYPE_GOLD)
                 || (ch->pcdata->cookies[COOKIE_AUCTION][0] == 'q' && type != AUCTION_TYPE_QPS)) {
            send_to_char("The confirmation cookie you entered was incorrect. Abandoning auction.\n\r", ch);
            ch->pcdata->cookies[COOKIE_AUCTION][0] = '\0';
            ch->pcdata->cookiesexpire[COOKIE_AUCTION] = (time_t)0;
            return;
        }

        /* they entered the correct cookie. add the auction */

        if ((keyword = get_unique_keyword()) == NULL) {
            send_to_char("Couldn't find a spare keyword!\n\r", ch);
            return;
        }

        CREATE_MEMBER(AUCTION_DATA, auc);

        if (!auc) {
            send_to_char("Unable to allocate memory for auction structure.\n\r", ch);
            return;
        }

        auc->keyword = str_dup(keyword);
        auc->owner_name = str_dup(ch->short_descr);
        auc->owner = str_dup(ch->pcdata->origname);
        auc->bidder = str_dup("");
        auc->bidder_name = str_dup("");
        auc->amount = 0;
        auc->expire_time = (time_t) (current_time + dur);
        auc->reserve = reserve;
        auc->pObj = obj;
        auc->flags = ttype;

        obj_from_char(obj);
        DLINK(auc, first_auction, last_auction, next, prev);

        auctionf("@@y[@@b%s@@y] @@N%s adds a %s auction: %s@@N. Ends in %s.@@N",
            auc->keyword, ch->short_descr, AUCTYPE2(auc), obj->short_descr, duration(dur, buf2)
            );

        if      (type == AUCTION_TYPE_GOLD && !IS_SET(obj->extra_flags, ITEM_NODROP))
            ch->gold -= 5000;
        else if (type == AUCTION_TYPE_GOLD && IS_SET(obj->extra_flags, ITEM_NODROP)) {
            ch->gold -= 5000;
            ch->quest_points -= 10;
        }
        else if (!IS_SET(obj->extra_flags, ITEM_NODROP))
            ch->quest_points--;
        else
            ch->quest_points -= 11;

        ch->pcdata->cookies[COOKIE_AUCTION][0] = '\0';
        ch->pcdata->cookiesexpire[COOKIE_AUCTION] = (time_t)0;

        save_char_obj(ch);
        save_auctions();
        return;
    }                            /* end: add */

    if (!str_cmp("bid", arg) && (type == AUCTION_TYPE_GOLD || ch->adept_level > 0)) {
        int                 amt = 0;
        int                 to_beat = 0;
        int                 avail = 0;

        if (ch->pcdata->trading_with != NULL) {
            send_to_char("Finish your trade first!\n\r", ch);
            return;
        }

        if (arg3[0] != '\0') {
            amt = abs(atoi(arg3));
        }
        else {
            sendf(ch, "syntax: %s bid <keyword> <amount>\n\r", AUCCMD(type));
            return;
        }

        for (auc = first_auction; auc != NULL; auc = auc->next)
            if (IS_SET(auc->flags, type) && !str_cmp(auc->keyword, arg2))
                break;

        if (auc == NULL) {
            sendf(ch, "Unable to find that %s auction.\n\r", AUCTYPE(type));
            return;
        }

        if (!str_cmp(auc->owner, ch->pcdata->origname)) {
            send_to_char("You can't bid on your own auctions.\n\r", ch);
            return;
        }

        if (ch->carry_number >= can_carry_n(ch)) {
            send_to_char("Your inventory is full, please remove at least one item from it.\n\r", ch);
            return;
        }

        if (ch->carry_weight >= can_carry_w(ch)) {
            send_to_char("Your weight is at maximum, please lose some weight!\n\r", ch);
            return;
        }

        if (IS_SET(auc->flags, AUCTION_TYPE_GOLD)) {
            avail = (!str_cmp(auc->bidder, ch->pcdata->origname)) ? available_gold(ch) + auc->amount : available_gold(ch);

            if (auc->amount > 0)
                to_beat = auc->amount + UMAX(1, (int) (auc->amount * 0.05));
            else
                to_beat = auc->reserve;
        }
        else {
            avail = (!str_cmp(auc->bidder, ch->pcdata->origname)) ? available_qps(ch) + auc->amount : available_qps(ch);

            if (auc->amount > 0)
                to_beat = auc->amount + 1;
            else
                to_beat = auc->reserve;
        }

        if (amt < to_beat) {
            if (IS_SET(auc->flags, AUCTION_TYPE_GOLD))
                sendf(ch, "@@NYou must beat the latest bid by 5%%, which is @@y%s@@N GP.\n\r", number_comma(to_beat));
            else
                sendf(ch, "@@NTo beat the latest bid, bid @@a%s@@N QP.\n\r", number_comma(to_beat));

            return;
        }
        else if (IS_SET(auc->flags, AUCTION_TYPE_GOLD) && amt > avail) {
            send_to_char("You don't have that much gold.\n\r", ch);
            return;
        }
        else if (IS_SET(auc->flags, AUCTION_TYPE_QPS) && amt > avail) {
            send_to_char("You don't have that many quest points.\n\r", ch);
            return;
        }

        if (auc->bidder)
            free_string(auc->bidder);

        if (auc->bidder_name)
            free_string(auc->bidder_name);

        auc->bidder = str_dup(ch->pcdata->origname);
        auc->bidder_name = str_dup(ch->short_descr);
        auc->amount = amt;

        auctionf("@@y[@@b%s@@y] @@N%s bids %s %s@@N on %s@@N.", auc->keyword, ch->short_descr, number_comma(amt), BIDTYPE(auc),
            auc->pObj->short_descr);

        if (abs(auc->expire_time - current_time) < 60)
            auc->expire_time += 360;

        if (!can_save(ch, auc->pObj))
            send_to_char("@@eWarning: @@gYou can't save this item.\n\r", ch);

        if (auc->pObj->item_type == ITEM_QUEST && auc->pObj->value[3] != 0 && get_pseudo_level(ch) > auc->pObj->value[3])
            send_to_char("@@eWarning: @@gYou aren't low enough level to deposit this quest item.\n\r", ch);

        save_auctions();
        return;
    }                            /* end: bid */

    if (!str_cmp("id", arg)) {
        if (arg2[0] == '\0') {
            sendf(ch, "syntax: %s id <keyword>\n\r", AUCCMD(type));
            return;
        }

        for (auc = first_auction; auc != NULL; auc = auc->next)
            if (IS_SET(auc->flags, type) && !str_cmp(auc->keyword, arg2))
                break;

        if (auc == NULL) {
            sendf(ch, "Unable to find that %s auction.\n\r", AUCTYPE(type));
            return;
        }

        spell_identify(0, 0, ch, auc->pObj, NULL);
        return;
    }                            /* end: id */

    do_help(ch, AUCCMD(type));
    return;
}

int
auction_list_cmp(const void *x, const void *y)
{
    AUCTION_DATA       *dx = *(AUCTION_DATA **) x;
    AUCTION_DATA       *dy = *(AUCTION_DATA **) y;

    if (dx->expire_time < dy->expire_time)
        return -1;
    else if (dx->expire_time == dy->expire_time)
        return 0;
    else
        return 1;
}

/* auction listing is big/crazy enough to warrant its own function */
void
auction_do_list(CHAR_DATA *ch, int type)
{
    int                 auction_total = 0;
    int                 auction_count = 0;
    int                 c;
    char                buf[MSL], buf2[MSL], buf3[MSL], buf4[MSL];
    AUCTION_DATA       *auc, *auc2;
    void              **asort;

    for (auc = first_auction; auc != NULL; auc = auc->next)
        if (IS_SET(auc->flags, type))
            auction_total++;

    if (auction_total == 0) {
        if (type == AUCTION_TYPE_GOLD)
            send_to_char("No gold auctions found.\n\r", ch);
        else
            send_to_char("No quest point auctions found.\n\r", ch);

        return;
    }

    asort = (void **) malloc(sizeof(AUCTION_DATA *) * auction_total);
    memset(asort, 0, sizeof(AUCTION_DATA *) * auction_total);

    c = 0;

    for (auc = first_auction; auc != NULL; auc = auc->next)
        if (IS_SET(auc->flags, type))
            asort[c++] = (void *) auc;

    qsort(asort, c, sizeof(void *), auction_list_cmp);

    /* send header */
    if (auction_total == 1) {
        if (type == AUCTION_TYPE_GOLD)
            send_to_char("@@N@@d.------------------@@g=( @@yGold @@WAuctions @@g)=@@d-.\n\r", ch);
        else
            send_to_char("@@N@@d.-----------@@g=( @@aQuest Point @@WAuctions @@g)=@@d-.\n\r", ch);
    }
    else {
        if (type == AUCTION_TYPE_GOLD)
            send_to_char("@@N@@d.---------------------------------------------------------@@g=( @@yGold @@WAuctions @@g)=@@d-.\n\r", ch);
        else
            send_to_char("@@N@@d.--------------------------------------------------@@g=( @@aQuest Point @@WAuctions @@g)=@@d-.\n\r", ch);
    }

    for (c = 0; c < auction_total; c++) {
        auc = asort[c];

        buf[0] = 0;
        buf2[0] = 0;
        buf3[0] = 0;
        buf4[0] = 0;
        auc2 = NULL;
        auction_count++;

        if (auction_count < auction_total && auction_count % 2 == 1) {
            if (c < auction_total)
                auc2 = asort[c + 1];
            else
                auc2 = NULL;
        }

        if (auction_count > 1 && auction_count == auction_total && auction_total % 2 == 1)
            sprintf(buf + strlen(buf), "@@d|--------------------------------------+--------------------------------------'@@N\n\r");
        else if (auction_count > 1)
            sprintf(buf + strlen(buf), "@@d|--------------------------------------+--------------------------------------|@@N\n\r");

        /* first line */
        sprintf(buf + strlen(buf), "@@d| @@y[@@b%3s@@y] @@N", auc->keyword);
        sprintf(buf + strlen(buf), "%s", my_left(auc->pObj->short_descr, buf2, 27 - UMAX(1, my_strlen(auc->owner_name))));
        sprintf(buf + strlen(buf), " @@N(%s@@N) ", auc->owner_name);
        buf2[0] = 0;
        buf3[0] = 0;
        buf4[0] = 0;

        if (auc2) {
            sprintf(buf + strlen(buf), "@@d| @@y[@@b%3s@@y] @@N", auc2->keyword);
            sprintf(buf + strlen(buf), "%s", my_left(auc2->pObj->short_descr, buf2, 27 - UMAX(1, my_strlen(auc2->owner_name))));
            sprintf(buf + strlen(buf), " @@N(%s@@N) @@d|\n\r", auc2->owner_name);
        }
        else
            sprintf(buf + strlen(buf), "@@d|\n\r");

        /* second line */
        if (auc->bidder && auc->bidder[0] && auc->amount > 0)
            sprintf(buf2, "@@gBid: %s %s@@N@@g by @@N%s@@N", number_comma(auc->amount), BIDTYPE(auc), auc->bidder_name);
        else
            sprintf(buf2, "@@gReserve: %s %s@@N@@g", number_comma(auc->reserve), BIDTYPE(auc));

        buf3[0] = 0;
        sprintf(buf + strlen(buf), "@@d| %s ", my_left(buf2, buf3, 36));

        if (auc2) {
            if (auc2->bidder && auc2->bidder[0] && auc2->amount > 0)
                sprintf(buf2, "@@gBid: %s %s@@N@@g by @@N%s@@N", number_comma(auc2->amount), BIDTYPE(auc2), auc2->bidder_name);
            else
                sprintf(buf2, "@@gReserve: %s %s@@N@@g", number_comma(auc2->reserve), BIDTYPE(auc2));

            buf3[0] = 0;
            sprintf(buf + strlen(buf), "@@d| %s @@d|\n\r", my_left(buf2, buf3, 36));
        }
        else
            sprintf(buf + strlen(buf), "@@d|\n\r");

        /* third line */
        buf3[0] = 0;
        sprintf(buf2, "@@gEnds In: %s%s@@N", (auc->expire_time - current_time <= 60) ? "@@e" : "@@g", duration(auc->expire_time - current_time,
                buf3));
        buf3[0] = 0;
        sprintf(buf + strlen(buf), "@@d| %s ", my_left(buf2, buf3, 36));

        if (auc2) {
            buf3[0] = 0;
            sprintf(buf2, "@@gEnds In: %s%s@@N", (auc2->expire_time - current_time <= 60) ? "@@e" : "@@g", duration(auc2->expire_time - current_time,
                    buf3));
            buf3[0] = 0;
            sprintf(buf + strlen(buf), "@@d| %s @@d|\n\r", my_left(buf2, buf3, 36));
        }
        else
            sprintf(buf + strlen(buf), "@@d|\n\r");

        send_to_char(buf, ch);

        if (auc2) {
            auction_count++;
            c++;
            continue;
        }
    }

    if (auction_total % 2 == 1)
        send_to_char("@@d'--------------------------------------'@@N\n\r", ch);
    else
        send_to_char("@@d'--------------------------------------+--------------------------------------'@@N\n\r", ch);

    free(asort);

    return;
}

OBJ_DATA           *
fread_auction_obj(FILE * fp)
{
    static OBJ_DATA     obj_zero;
    OBJ_DATA           *obj;
    char               *word;
    bool                fMatch;
    bool                fVnum;
    int                 Temp_Obj = 0, OldVnum = 0;

    GET_FREE(obj, obj_free);
    *obj = obj_zero;
    obj->name = str_dup("");
    obj->short_descr = str_dup("");
    obj->description = str_dup("");

    fVnum = TRUE;

    for (;;) {
        word = feof(fp) ? "End" : fread_word(fp);
        fMatch = FALSE;

        switch (UPPER(word[0])) {
            case '*':
                fMatch = TRUE;
                fread_to_eol(fp);
                break;
            case 'A':
                if (!str_cmp(word, "Affect")) {
                    AFFECT_DATA        *paf;

                    GET_FREE(paf, affect_free);
                    paf->type = fread_number(fp);
                    paf->duration = fread_number(fp);
                    paf->modifier = fread_number(fp);
                    paf->location = fread_number(fp);
                    paf->bitvector = fread_number(fp);
                    LINK(paf, obj->first_apply, obj->last_apply, next, prev);
                    fMatch = TRUE;
                    break;
                }
                break;
            case 'C':

                if (auction_revision < 1)
                    KEY("ClassFlags", obj->item_apply, fread_number(fp));

                KEY("Cost", obj->cost, fread_number(fp));
                break;
            case 'D':
                SKEY("Description", obj->description, fread_string(fp));
                break;
            case 'E':
                KEY("ExtraFlags", obj->extra_flags, fread_number(fp));

                if (!str_cmp(word, "ExtraDescr")) {
                    EXTRA_DESCR_DATA   *ed;

                    GET_FREE(ed, exdesc_free);
                    ed->keyword = fread_string(fp);
                    ed->description = fread_string(fp);
                    LINK(ed, obj->first_exdesc, obj->last_exdesc, next, prev);
                    fMatch = TRUE;
                }

                if (!str_cmp(word, "End")) {
                    if (!fVnum) {
                        AFFECT_DATA        *paf;
                        EXTRA_DESCR_DATA   *ed;

                        monitor_chan("fread_auction_obj: incomplete object.", MONITOR_BAD);
                        free_string(obj->name);
                        free_string(obj->description);
                        free_string(obj->short_descr);

                        while ((paf = obj->first_apply) != NULL) {
                            obj->first_apply = paf->next;
                            PUT_FREE(paf, affect_free);
                        }

                        while ((ed = obj->first_exdesc) != NULL) {
                            obj->first_exdesc = ed->next;
                            free_string(ed->keyword);
                            free_string(ed->description);
                            PUT_FREE(ed, exdesc_free);
                        }

                        PUT_FREE(obj, obj_free);
                        return NULL;
                    }
                    else {
                        LINK(obj, first_obj, last_obj, next, prev);
                        obj->pIndexData->count++;

                        if (Temp_Obj) {
                            int                 newvnum;
                            OBJ_INDEX_DATA     *pObjIndex;
                            int                 nMatch = 0;
                            int                 vnum;

                            newvnum = TEMP_VNUM;

                            if (newvnum == TEMP_VNUM) {
                                for (vnum = 0; nMatch < top_obj_index; vnum++) {
                                    if ((pObjIndex = get_obj_index(vnum)) != NULL) {
                                        nMatch++;

                                        if (!str_cmp(obj->short_descr, pObjIndex->short_descr)) {
                                            obj->pIndexData = pObjIndex;
                                            break;
                                        }
                                    }
                                }
                            }

                        }

                        return obj;
                    }
                }
                break;

            case 'I':
                KEY("Id",        obj->id,         fread_unumber(fp));
                KEY("ItemApply", obj->item_apply, fread_number(fp));
                KEY("ItemType",  obj->item_type,  fread_number(fp));
                break;

            case 'L':
                KEY("Level", obj->level, fread_number(fp));
                break;
            case 'N':
                KEY("Name", obj->name, fread_string(fp));
                break;
            case 'O':
                if (!str_cmp(word, "Objfun")) {
                    char               *dumpme;

                    dumpme = fread_string(fp);
                    obj->obj_fun = obj_fun_lookup(dumpme);
                    free_string(dumpme);
                    fMatch = TRUE;
                }
                break;
            case 'S':
                SKEY("ShortDescr", obj->short_descr, fread_string(fp));

                if (!str_cmp(word, "Spell")) {
                    int                 iValue;
                    int                 sn;

                    iValue = fread_number(fp);
                    sn = skill_lookup(fread_word(fp));
                    if (iValue < 0 || iValue > 3)
                        monitor_chan("fread_auction_obj: bad iValue", MONITOR_BAD);
                    else if (sn < 0)
                        monitor_chan("fread_auction_obj: unknown skill", MONITOR_BAD);
                    else
                        obj->value[iValue] = sn;

                    fMatch = TRUE;
                    break;
                }
                break;
            case 'T':
                KEY("Timer", obj->timer, fread_number(fp));
                break;
            case 'V':
                if (!str_cmp(word, "Values")) {
                    obj->value[0] = fread_number(fp);
                    obj->value[1] = fread_number(fp);
                    obj->value[2] = fread_number(fp);
                    obj->value[3] = fread_number(fp);
                    fMatch = TRUE;
                    break;
                }

                if (!str_cmp(word, "Vnum")) {
                    int                 vnum;

                    vnum = fread_number(fp);

                    if ((obj->pIndexData = get_obj_index(vnum)) == NULL || vnum == TEMP_VNUM) {
                        /* Set flag saying that object is temporary */
                        Temp_Obj = 1;
                        OldVnum = vnum;
                        vnum = TEMP_VNUM;
                        obj->pIndexData = get_obj_index(vnum);
                    }
                    else
                        fVnum = TRUE;

                    fMatch = TRUE;
                    break;
                }
                break;
            case 'W':
                KEY("WearFlags", obj->wear_flags, fread_number(fp));
                KEY("WearLoc", obj->wear_loc, fread_number(fp));
                KEY("Weight", obj->weight, fread_number(fp));
                break;
        }

        if (!fMatch) {
            monitor_chan("fread_auction_obj: no match.", MONITOR_BAD);
            fread_to_eol(fp);
        }
    }

    return NULL;
}

void
fwrite_auction_obj(OBJ_DATA *obj, FILE * fp)
{
    AFFECT_DATA        *paf;
    EXTRA_DESCR_DATA   *ed;

    FPRINTF(fp, "#OBJECT\n");
    FPRINTF(fp, "Revision     %d\n", AUCTION_REVISION);

    if (obj->id > 0) FPRINTF(fp, "Id           %u\n",  obj->id);

    FPRINTF(fp, "Name         %s~\n", obj->name);
    FPRINTF(fp, "ShortDescr   %s~\n", obj->short_descr);
    FPRINTF(fp, "Description  %s~\n", obj->description);
    FPRINTF(fp, "Vnum         %d\n", obj->pIndexData->vnum);
    FPRINTF(fp, "ExtraFlags   %d\n", obj->extra_flags);
    FPRINTF(fp, "WearFlags    %d\n", obj->wear_flags);
    FPRINTF(fp, "WearLoc      %d\n", obj->wear_loc);

    if (obj->obj_fun != NULL)
        FPRINTF(fp, "Objfun       %s~\n", rev_obj_fun_lookup((void *) obj->obj_fun));

    FPRINTF(fp, "ItemApply    %d\n", obj->item_apply);
    FPRINTF(fp, "ItemType     %d\n", obj->item_type);
    FPRINTF(fp, "Weight       %d\n", obj->weight);
    FPRINTF(fp, "Level        %d\n", obj->level);
    FPRINTF(fp, "Timer        %d\n", obj->timer);
    FPRINTF(fp, "Cost         %d\n", obj->cost);
    FPRINTF(fp, "Values       %d %d %d %d\n", obj->value[0], obj->value[1], obj->value[2], obj->value[3]);

    switch (obj->item_type) {
        case ITEM_POTION:
        case ITEM_SCROLL:
            if (obj->value[1] > 0)
                FPRINTF(fp, "Spell 1      '%s'\n", skill_table[obj->value[1]].name);
            if (obj->value[2] > 0)
                FPRINTF(fp, "Spell 2      '%s'\n", skill_table[obj->value[2]].name);
            if (obj->value[3] > 0)
                FPRINTF(fp, "Spell 3      '%s'\n", skill_table[obj->value[3]].name);
            break;
        case ITEM_PILL:
        case ITEM_STAFF:
        case ITEM_WAND:
            if (obj->value[3] > 0)
                FPRINTF(fp, "Spell 3      '%s'\n", skill_table[obj->value[3]].name);
            break;
    }

    for (paf = obj->first_apply; paf != NULL; paf = paf->next)
        FPRINTF(fp, "Affect       %d %d %d %d %d\n", paf->type, paf->duration, paf->modifier, paf->location, paf->bitvector);

    for (ed = obj->first_exdesc; ed != NULL; ed = ed->next)
        FPRINTF(fp, "ExtraDescr   %s~ %s~\n", ed->keyword, ed->description);

    FPRINTF(fp, "End\n\n");
    return;
}

void
load_auctions(void)
{
    FILE               *fp;
    char               *word;
    AUCTION_DATA       *auc;

    if ((fp = fopen(AUCTION_FILE, "r")) == NULL)
        return;

    for (;;) {
        word = fread_word(fp);

        if (feof(fp))
            break;

        if (!str_cmp("#AUCTION", word)) {
            CREATE_MEMBER(AUCTION_DATA, auc);

            if (!auc)
                break;

            auc->keyword = fread_string(fp);
            auc->reserve = fread_number(fp);
            auc->expire_time = fread_number(fp);
            auc->flags = fread_number(fp);
            auc->owner = fread_string(fp);
            auc->owner_name = fread_string(fp);
            auc->bidder = fread_string(fp);
            auc->bidder_name = fread_string(fp);
            auc->amount = fread_number(fp);
            auc->pObj = NULL;

            word = fread_word(fp);
            if (!str_cmp("#OBJECT", word)) {
                auc->pObj = fread_auction_obj(fp);
            }

            if (!auc->pObj) {
                DESTROY_MEMBER(auc);
                break;
            }

            DLINK(auc, first_auction, last_auction, next, prev);
        }
        else
            break;
    }

    fclose(fp);
    return;
}

void
save_auctions(void)
{
    FILE               *fp;
    AUCTION_DATA       *auc;

    if (nosave)
        return;

    if ((fp = fopen(AUCTION_FILE, "w")) == NULL)
        return;

    for (auc = first_auction; auc != NULL; auc = auc->next) {
        if (!auc->pObj)
            continue;

        FPRINTF(fp, "#AUCTION\n");
        FPRINTF(fp, "%s~ %d %d %d %s~ %s~ %s~ %s~ %d\n", auc->keyword, auc->reserve, (int) auc->expire_time, auc->flags, auc->owner, auc->owner_name,
            auc->bidder, auc->bidder_name, auc->amount);

        fwrite_auction_obj(auc->pObj, fp);
    }

    FPRINTF(fp, "#END\n");

    fclose(fp);
    return;
}

void
auc_update(void)
{
    AUCTION_DATA       *auc;
    AUCTION_DATA       *auc_next;
    CHAR_DATA          *ch;
    char                buf2[MSL];
    bool                save = FALSE;
    int                 left = 0;

    for (auc = first_auction; auc != NULL; auc = auc_next) {
        auc_next = auc->next;
        left = abs(auc->expire_time - current_time);

        if (current_time >= auc->expire_time) {
            save = TRUE;
            auction_end(auc);
        }
        else if (left % 86400 == 0 || left % 3600 == 0 || left == 1800 || left == 600 || left == 300 || left == 60
            /* potentially too spammy for player-run auctions.
               || left == 30
               || left == 20
               || left == 10
               || left == 5
               || left == 3
               || left == 2
               || left == 1
             */
            ) {
            buf2[0] = 0;

            if (!auc->pObj)
                continue;

            auctionf("@@y[@@b%s@@y] @@NOnly %s left to bid on %s auction: %s@@N!", auc->keyword, duration(left, buf2), AUCTYPE2(auc),
                auc->pObj->short_descr);
        }
    }

    for (ch = first_player; ch != NULL; ch = ch->next_player) {
        if (   ch->pcdata
            && ch->pcdata->cookiesexpire[COOKIE_AUCTION] > 0
            && current_time >= ch->pcdata->cookiesexpire[COOKIE_AUCTION]) {
            ch->pcdata->cookies[COOKIE_AUCTION][0] = '\0';
            ch->pcdata->cookiesexpire[COOKIE_AUCTION] = (time_t)0;
        }
    }

    if (save)
        save_auctions();
}

/* available qps for non-auction related usage */
int
available_qps(CHAR_DATA *ch)
{
    AUCTION_DATA       *auc;
    RENAME_DATA        *rename;
    int                 amt;

    if (!ch->pcdata)
        return 0;

    amt = ch->quest_points;

    for (auc = first_auction; auc != NULL; auc = auc->next)
        if (IS_SET(auc->flags, AUCTION_TYPE_QPS) && ch->pcdata && !str_cmp(ch->pcdata->origname, auc->bidder))
            amt -= auc->amount;

    /* renames now reserve the qps */
    for (rename = first_rename; rename != NULL; rename = rename->next)
        if (!str_cmp(ch->pcdata->origname, rename->playername))
            amt -= 15;

    return UMAX(0, amt);
}

/* available gold for non-auction related usage */
int
available_gold(CHAR_DATA *ch)
{
    AUCTION_DATA       *auc;
    int                 amt;

    if (!ch->pcdata)
        return 0;

    amt = ch->balance;

    for (auc = first_auction; auc != NULL; auc = auc->next)
        if (IS_SET(auc->flags, AUCTION_TYPE_GOLD) && ch->pcdata && !str_cmp(ch->pcdata->origname, auc->bidder))
            amt -= auc->amount;

    return UMAX(0, amt);
}

char               *
get_unique_keyword(void)
{
    static char         buf[4];
    bool                found = FALSE;
    int                 tries = 0;

    AUCTION_DATA       *auc;

    for (;;) {
        /* no inf loops */
        if (tries++ > 5000)
            return NULL;

        buf[0] = number_range('a', 'z');
        buf[1] = number_range('0', '9');
        buf[2] = number_range('0', '9');
        buf[3] = 0;

        for (found = FALSE, auc = first_auction; auc != NULL; auc = auc->next)
            if (!strcmp(buf, auc->keyword)) {
                found = TRUE;
                break;
            }

        if (found)
            continue;
        else
            break;
    }

    return buf;
}
