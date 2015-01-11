#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "merc.h"
#include "duel.h"
#include "auction.h"
#include "tables.h"

IDSTRING(rcsid, "$Id: duel.c,v 1.22 2004/11/02 20:03:58 dave Exp $");

DUEL_DATA          *first_duel = NULL;
DUEL_DATA          *last_duel = NULL;

extern CHAR_DATA   *auction_bidder;
extern void show_char_to_char args((CHAR_DATA *list, CHAR_DATA *ch, bool automode));

/* local functions */
bool                is_in_duel(CHAR_DATA *ch, bool minstage);
bool                is_watching_duel(CHAR_DATA *ch);
DUEL_DATA          *find_watching_duel(CHAR_DATA *ch);
DUEL_WATCHER_DATA  *find_duel_watcher(CHAR_DATA *ch);
DUEL_DATA          *find_duel(CHAR_DATA *ch);
DUEL_PLAYER_DATA   *find_duel_player(CHAR_DATA *ch);
DUEL_DATA          *create_duel(void);
DUEL_PLAYER_DATA   *create_duel_player(void);
int                 find_duel_room(void);
DUEL_DATA          *new_duel(CHAR_DATA *ch, CHAR_DATA *victim);
void                start_duel(DUEL_DATA *duel);
void                cancel_duel(DUEL_DATA *duel, CHAR_DATA *ch, int type);
void                duel_update(void);

/* checks to see if a player is duelling */
bool
is_in_duel(CHAR_DATA *ch, bool minstage)
{
    DUEL_DATA          *duel;
    DUEL_PLAYER_DATA   *player;

    if (!ch || IS_NPC(ch))
        return FALSE;

    for (duel = first_duel; duel != NULL; duel = duel->next)
        for (player = duel->first_player; player != NULL; player = player->next)
            if (duel->stage >= minstage && ch == player->ch)
                return TRUE;

    return FALSE;
}

bool
is_watching_duel(CHAR_DATA *ch)
{
    DUEL_DATA          *duel;
    DUEL_WATCHER_DATA  *watcher;

    if (!ch || IS_NPC(ch))
        return FALSE;

    for (duel = first_duel; duel != NULL; duel = duel->next)
        for (watcher = duel->first_watcher; watcher != NULL; watcher = watcher->next)
            if (ch == watcher->ch)
                return TRUE;

    return FALSE;
}

DUEL_DATA          *
find_watching_duel(CHAR_DATA *ch)
{
    DUEL_DATA          *duel;
    DUEL_WATCHER_DATA  *watcher;

    if (!ch || IS_NPC(ch))
        return FALSE;

    for (duel = first_duel; duel != NULL; duel = duel->next)
        for (watcher = duel->first_watcher; watcher != NULL; watcher = watcher->next)
            if (ch == watcher->ch)
                return duel;

    return NULL;
}

DUEL_WATCHER_DATA  *
find_duel_watcher(CHAR_DATA *ch)
{
    DUEL_DATA          *duel;
    DUEL_WATCHER_DATA  *watcher;

    if (!ch || IS_NPC(ch))
        return FALSE;

    for (duel = first_duel; duel != NULL; duel = duel->next)
        for (watcher = duel->first_watcher; watcher != NULL; watcher = watcher->next)
            if (ch == watcher->ch)
                return watcher;

    return NULL;
}

/* finds the duel the player is currently participating in */
DUEL_DATA          *
find_duel(CHAR_DATA *ch)
{
    DUEL_DATA          *duel;
    DUEL_PLAYER_DATA   *player;

    if (!ch || IS_NPC(ch))
        return NULL;

    for (duel = first_duel; duel != NULL; duel = duel->next)
        for (player = duel->first_player; player != NULL; player = player->next)
            if (ch == player->ch)
                return duel;

    return NULL;
}

DUEL_PLAYER_DATA   *
find_duel_player(CHAR_DATA *ch)
{
    DUEL_DATA          *duel;
    DUEL_PLAYER_DATA   *player;

    if (!ch || IS_NPC(ch))
        return NULL;

    for (duel = first_duel; duel != NULL; duel = duel->next)
        for (player = duel->first_player; player != NULL; player = player->next)
            if (ch == player->ch)
                return player;

    return NULL;
}

/* assign memory to a duel */
DUEL_DATA          *
create_duel(void)
{
    DUEL_DATA          *duel;

    CREATE_MEMBER(DUEL_DATA, duel);

    if (!duel)
        return NULL;
    else
        return duel;
}

/* assign memory to a duel player */
DUEL_PLAYER_DATA   *
create_duel_player(void)
{
    DUEL_PLAYER_DATA   *player;

    CREATE_MEMBER(DUEL_PLAYER_DATA, player);

    if (!player)
        return NULL;
    else
        return player;
}

/* find an unoccupied duel room */
int
find_duel_room(void)
{
    DUEL_DATA          *duel;
    int                 a;
    bool                found;

    for (a = DUEL_MIN_ROOM; a <= DUEL_MAX_ROOM; a++) {
        found = FALSE;

        for (duel = first_duel; duel != NULL; duel = duel->next)
            if (a == duel->vnum) {
                found = TRUE;
                break;
            }

        if (!found && get_room_index(a))
            return a;
    }

    return -1;
}

/* accept a duel and save some settings */
void
accept_duel(CHAR_DATA *ch)
{
    DUEL_DATA          *duel = find_duel(ch);
    DUEL_PLAYER_DATA   *player;

    if (duel) {
        if ((player = find_duel_player(ch)) == NULL)
            return;

        player->accepted = TRUE;
    }

    for (player = duel->first_player; player != NULL; player = player->next)
        if (player->accepted == FALSE)
            break;

    if (!player) {
        /* all accepted */

        duel->countdown = DUEL_TIMEOUT_SET;
        duel->stage = DUEL_STAGE_SET;

        challengef("%s accepts %s's spar offer.", ch->short_descr, duel->first_player->ch->short_descr);
        send_to_char("You have 20 seconds to prepare for the spar.\n\r", ch);
        send_to_char("You have 20 seconds to prepare for the spar.\n\r", duel->first_player->ch);
    }

    return;
}

/* create a new duel if ch already isn't in one, otherwise, add victim to ch's
   duel */
DUEL_DATA          *
new_duel(CHAR_DATA *ch, CHAR_DATA *victim)
{
    DUEL_DATA          *duel = find_duel(ch);
    DUEL_PLAYER_DATA   *fch, *vch;
    int                 vnum = 0;

    vch = create_duel_player();
    if (!vch)
        return NULL;

    if (!duel) {
        vnum = find_duel_room();

        if (vnum == -1)
            return NULL;

        duel = create_duel();
        if (!duel) {
            DESTROY_MEMBER(vch);
            return NULL;
        }

        fch = create_duel_player();
        if (!fch) {
            DESTROY_MEMBER(vch);
            DESTROY_MEMBER(duel);
            return NULL;
        }

        fch->ch = ch;
        fch->accepted = TRUE;    /* player initiating duel automatically accepts */
        DLINK(fch, duel->first_player, duel->last_player, next, prev);

        duel->challenged = current_time;
        duel->vnum = vnum;
        duel->stage = DUEL_STAGE_READY;
        DLINK(duel, first_duel, last_duel, next, prev);
    }

    vch->ch = victim;
    DLINK(vch, duel->first_player, duel->last_player, next, prev);

    return duel;
}

/* start a duel, transfer necessary people */
void
start_duel(DUEL_DATA *duel)
{
    DUEL_PLAYER_DATA   *player;
    AFFECT_DATA        *af, *naf;
    ROOM_INDEX_DATA    *room;

    if (!duel)
        return;

    duel->started = current_time;
    duel->stage = DUEL_STAGE_GO;

    if ((room = get_room_index(duel->vnum)) == NULL)
        return;

    for (player = duel->first_player; player != NULL; player = player->next) {
        int chooserace = 0;

        if (!player->ch)
            continue;

        player->hp = player->ch->hit;
        player->mana = player->ch->mana;
        player->move = player->ch->move;
        player->energy = player->ch->energy;

        if (!IS_SET(duel->flags, DUEL_RANDSTAT)) {
            player->str   = player->ch->pcdata->max_str;
            player->intel = player->ch->pcdata->max_int;
            player->dex   = player->ch->pcdata->max_dex;
        }
        else {
            player->str   = number_range(12, 23);
            player->intel = number_range(12, 23);
            player->dex   = number_range(12, 23);
        }

        player->race = player->ch->race;

        if (IS_SET(duel->flags, DUEL_RANDRACE)) {
            while ((chooserace = number_range(0, MAX_RACE - 1)) == RACE_HMN
                    || chooserace == RACE_HOB
                    || chooserace == RACE_OGR
                    || chooserace == RACE_TTN
                    || chooserace == RACE_DWF);

            player->ch->race = chooserace;
        }

        player->ch->pcdata->safetimer = 0;

        /* no resting/sleeping duellists! */
        player->ch->position = POS_STANDING;

        for (af = player->ch->first_affect; af != NULL; af = af->next) {
            if (af->type == gsn_shield_fire
                || af->type == gsn_shield_ice
                || af->type == gsn_shield_shock
                || af->type == gsn_shield_demon
                )
                continue;

            GET_FREE(naf, affect_free);
            naf->type = af->type;
            naf->duration = af->duration;
            naf->location = af->location;
            naf->modifier = af->modifier;
            naf->bitvector = af->bitvector;
            naf->caster = af->caster;
            naf->level = af->level;
            naf->save = af->save;
            LINK(naf, player->first_affect, player->last_affect, next, prev);
        }

        char_from_room(player->ch);
        char_to_room(player->ch, get_room_index(duel->vnum));
        if (player->ch->riding && IS_NPC(player->ch->riding)) {
            char_from_room(player->ch->riding);
            char_to_room(player->ch->riding, get_room_index(2));
        }

        /* TODO: transfer charmies here, if charmies are allowed */
    }

    if (IS_SET(duel->flags, DUEL_NOMAGIC))
        SET_BIT(room->room_flags, ROOM_NO_MAGIC);
    else
        REMOVE_BIT(room->room_flags, ROOM_NO_MAGIC);

    for (player = duel->first_player; player != NULL; player = player->next) {
        if (!player->ch)
            continue;

        do_look(player->ch, "auto");
    }
}

void
do_duel(CHAR_DATA *ch, char *argument)
{
    char                buf[MSL];
    char                arg[MAX_INPUT_LENGTH];
    char                *fl = NULL;
    CHAR_DATA          *victim;
    DUEL_DATA          *duel;
    DUEL_PLAYER_DATA   *player;
    DUEL_WATCHER_DATA  *watcher;

    if (IS_NPC(ch) || !ch->in_room)
        return;

    argument = one_argument(argument, arg);

    if (arg[0] == '\0' || IS_NPC(ch)) {
        send_to_char("syntax: spar <player> [flags]\n\r"
                     "        spar accept\n\r"
                     "        spar watch <player>\n\r"
                     "        spar look\n\r"
            , ch);

        return;
    }

    if (ch->in_room->vnum != DUEL_START_ROOM && !IS_IMMORTAL(ch) && str_cmp("watch", arg) && str_cmp("look", arg)) {
        send_to_char
            ("You must be in the Sparring Room to spar someone, or accept a spar. It is located up and west of Market Square in Midgaard.\n\r", ch);
        return;
    }

    if (!str_cmp("debug", arg) && IS_IMMORTAL(ch)) {
        for (duel = first_duel; duel != NULL; duel = duel->next) {
            sprintf(buf, "flags[%d] vnum[%d] challenged[%d] started[%d] countdown[%d] stage[%d]\n\r",
                duel->flags, duel->vnum, (int) duel->challenged, (int) duel->started, duel->countdown, duel->stage);
            send_to_char(buf, ch);

            for (player = duel->first_player; player != NULL; player = player->next) {
                DUEL_OBJ_DATA      *dobj;

                sprintf(buf, "  name[%s] accepted[%d] linkdead[%d] hp[%d] mana[%d] move[%d] str[%d] intel[%d] dex[%d] energy[%d]\n\r",
                    player->ch->short_descr,
                    (int) player->accepted, (int) player->linkdead, player->hp, player->mana, player->move, player->str, player->intel, player->dex, player->energy);
                send_to_char(buf, ch);

                for (dobj = player->first_obj; dobj != NULL; dobj = dobj->next) {
                    sprintf(buf, "    burning: %s\n\r", dobj->obj->short_descr);
                    send_to_char(buf, ch);
                }
            }
        }

        return;
    }

    if (!str_cmp("accept", arg)) {
        if (!is_in_duel(ch, TRUE)) {
            send_to_char("You must be offered to spar to accept one.\n\r", ch);
            return;
        }

        if ((player = find_duel_player(ch)) != NULL && player->accepted == TRUE) {
            send_to_char("You have already accepted to spar.\n\r", ch);
            return;
        }

        accept_duel(ch);

        return;
    }

    if (!str_cmp("watch", arg)) {
        if (is_watching_duel(ch)) {
            send_to_char("You are already watching a spar.\n\r", ch);
            return;
        }

        if ((victim = get_char_world(ch, argument)) == NULL) {
            send_to_char("Watch who's spar?\n\r", ch);
            return;
        }

        if (IS_NPC(victim)) {
            send_to_char("NPCs don't spar!\n\r", ch);
            return;
        }

        if (!is_in_duel(victim, DUEL_STAGE_SET)) {
            send_to_char("They are not sparring, or both parties haven't accepted yet.\n\r", ch);
            return;
        }

        if (is_in_duel(ch, DUEL_STAGE_READY)) {
            send_to_char("You are either in the process of accepting a spar, preparing for a spar, or sparring!\n\r", ch);
            return;
        }

        if ((duel = find_duel(victim)) == NULL) {
            send_to_char("Unable to locate that spar.\n\r", ch);
            return;
        }

        CREATE_MEMBER(DUEL_WATCHER_DATA, watcher);
        if (!watcher) {
            send_to_char("Unable to create a watcher entity.\n\r", ch);
            return;
        }

        watcher->ch = ch;
        DLINK(watcher, duel->first_watcher, duel->last_watcher, next, prev);

        if (duel->stage == DUEL_STAGE_SET)
            send_to_char("You prepare to watch a spar.\n\r", ch);
        else
            send_to_char("You start watching a spar.\n\r", ch);

        return;
    } /* spar watch */

    if (!str_cmp("look", arg)) {
        ROOM_INDEX_DATA *room;

        if (!is_watching_duel(ch)) {
            send_to_char("You are not watching a spar.\n\r", ch);
            return;
        }

        if ((duel = find_watching_duel(ch)) == NULL || (room = get_room_index(duel->vnum)) == NULL) {
            send_to_char("Unable to find the spar you are watching.\n\r", ch);
            return;
        }

        if (duel->stage != DUEL_STAGE_GO) {
            send_to_char("The spar has not started yet.\n\r", ch);
            return;
        }

        show_char_to_char(room->first_person, ch, FALSE);
        return;
    } /* spar look */

    if ((victim = get_char_room(ch, arg)) == NULL) {
        send_to_char("Start a spar with who?\n\r", ch);
        return;
    }

    if (ch == victim) {
        send_to_char("Unfortunately, you may not spar yourself.\n\r", ch);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("You can't spar mobs.\n\r", ch);
        return;
    }

    if (victim->level < 20) {
        send_to_char("They are not level 20 or above.\n\r", ch);
        return;
    }

    if (is_in_duel(ch, DUEL_STAGE_READY)) {
        send_to_char("You are already in a spar.\n\r", ch);
        return;
    }
    else if (is_in_duel(victim, DUEL_STAGE_READY)) {
        send_to_char("They are currently in a spar.\n\r", ch);
        return;
    }

    if (IS_SET(ch->deaf2, CHANNEL2_CHALLENGE)) {
        send_to_char("Turn on the CHALLENGE channel and try again.\n\r", ch);
        return;
    }

    if (IS_SET(victim->deaf2, CHANNEL2_CHALLENGE)) {
        send_to_char("They are ignoring the CHALLENGE channel.\n\r", ch);
        return;
    }

    if (victim->desc == NULL) {
        send_to_char("Try sparring them when they're not linkdead.\n\r", ch);
        return;
    }

    if (is_watching_duel(ch)) {
        send_to_char("Not while you're watching a spar.\n\r", ch);
        return;
    }

    if (is_watching_duel(victim)) {
        send_to_char("They are watching a spar.\n\r", ch);
        return;
    }

    /* hardcoded anti-group range, ie. level 80 vs level 59. */
    if (ch->level < 10 || victim->level < 40 || (ch->level + 20 < victim->level)
        || (victim->level + 20 < ch->level)
        ) {
        send_to_char("You currently cannot spar this person.\n\r", ch);
        return;
    }

    if (ch == auction_bidder) {
        send_to_char("You are bidding on an auction at this time.\n\r", ch);
        return;
    }

    if (victim == auction_bidder) {
        send_to_char("They are bidding on an auction at this time.\n\r", ch);
        return;
    }

    {
        AUCTION_DATA       *auc;

        for (auc = first_auction; auc != NULL; auc = auc->next) {
            if (ch->pcdata && !str_cmp(auc->bidder, ch->pcdata->origname) && (auc->expire_time - current_time) < (DUEL_TIMEOUT_GO * 2)) {
                send_to_char("You are winning an auction that ends soon. Try later.\n\r", ch);
                return;
            }
            else if (victim->pcdata && !str_cmp(auc->bidder, victim->pcdata->origname) && (auc->expire_time - current_time) < (DUEL_TIMEOUT_GO * 2)) {
                send_to_char("They are winning an auction that ends soon. Try later.\n\r", ch);
                return;
            }
        }
    }

    {
        OBJ_DATA           *obj;

        for (obj = ch->first_carry; obj != NULL; obj = obj->next_in_carry_list) {
            if (obj->wear_loc == WEAR_NONE)
                continue;

            if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch))
                || (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch))
                || (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch))
                ) {
                send_to_char("You are wearing anti- flagged eq which would zap off.\n\r", ch);
                return;
            }
        }

        for (obj = victim->first_carry; obj != NULL; obj = obj->next_in_carry_list) {
            if (obj->wear_loc == WEAR_NONE)
                continue;

            if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(victim))
                || (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(victim))
                || (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(victim))
                ) {
                send_to_char("They are wearing anti- flagged eq which would zap off.\n\r", ch);
                return;
            }
        }
    }

    if ((duel = new_duel(ch, victim)) == NULL) {
        send_to_char("Unable to create the spar.\n\r", ch);
        return;
    }

    duel->flags = 0;

    do {
        int value = 0;
        argument = one_argument(argument, arg);

        if (arg[0] != '\0' && (value = table_lookup(tab_duel_types, arg)) > 0)
            SET_BIT(duel->flags, value);
    } while (arg[0] != '\0');

    duel->stage = DUEL_STAGE_READY;

    fl = bit_table_lookup2(tab_duel_types, duel->flags);

    if (fl && fl[0] != '\0')
        sprintf(buf, " @@N@@d(@@g%s@@N@@d)@@g", fl);
    else
        buf[0] = '\0';

    challengef("%s offers to spar %s%s.@@N",
        ch->short_descr,
        victim->short_descr,
        buf
    );

    fl = bit_table_lookup2(tab_duel_types, duel->flags);

    if (fl && fl[0] != '\0')
        sprintf(buf, " @@N@@d(@@gspar flags: %s@@N@@d)@@g", fl);
    else
        buf[0] = '\0';

    send_to_char("@@gRemember, items you consume in spars will not be returned once the spar is finished!@@N\n\r", ch);
    sendf(victim, "@@gYou have been offered to spar by %s%s. Type @@aspar accept@@g if you wish to accept.\n\rRemember, items you consume in spars will not be returned once the spar is finished!@@N\n\r",
        ch->short_descr, buf);

    if (!can_group(ch, victim)) {
        char                buf[MSL];

        sprintf(buf, "@@eWARNING: @@g%s is out of your group range and the spar may be unfair.@@N\n\r", ch->short_descr);
        send_to_char(buf, victim);
        sprintf(buf, "@@eWARNING: @@g%s is out of your group range and the spar may be unfair.@@N\n\r", victim->short_descr);
        send_to_char(buf, ch);
    }

    return;
}

void
cancel_duel(DUEL_DATA *duel, CHAR_DATA *ch, int type)
{
    DUEL_PLAYER_DATA   *player;
    DUEL_WATCHER_DATA  *watcher;
    DUEL_OBJ_DATA      *dobj;
    CHAR_DATA          *victim = NULL;
    AFFECT_DATA        *af;

    if (!duel)
        return;

    if (ch) {
        victim = (ch == duel->first_player->ch) ? duel->first_player->next->ch : duel->first_player->ch;
    }
    else {
        ch = duel->first_player->ch;
        victim = duel->first_player->next->ch;
    }

    /* TODO: fix all of these messages.. they're all ugly hacks right now */
    if (duel->stage == DUEL_STAGE_READY) {
        if (type == DUEL_END_TIMEOUT)
            challengef("%s's offer to spar %s remains unanswered.", ch->short_descr, victim->short_descr);
        else if (type == DUEL_END_MOVEROOM) {
            if (ch == duel->first_player->ch)
                challengef("%s abandons the spar against %s.", ch->short_descr, victim->short_descr);
            else
                challengef("%s declines %s's spar offer.", ch->short_descr, victim->short_descr);
        }
    }
    else if (duel->stage == DUEL_STAGE_SET) {
        if (type == DUEL_END_MOVEROOM) {
            if (ch == duel->first_player->ch)
                challengef("%s abandons the spar against %s.", ch->short_descr, victim->short_descr);
            else
                challengef("%s declines %s's spar offer.", ch->short_descr, victim->short_descr);
        }
        else if (type == DUEL_END_LINKDEAD) {
            challengef("%s goes linkdead and aborts the spar against %s.", ch->short_descr, victim->short_descr);
        }
    }
    else if (duel->stage == DUEL_STAGE_GO) {
        switch (type) {
            default:
            case DUEL_END_WIN:
                if (!IS_NPC(ch))
                    challengef("%s WINS the spar against %s!", ch->short_descr, victim->short_descr);
                else
                    challengef("%s LOSES the spar against %s because they died to a mob!", ch->short_descr, victim->short_descr);
                break;
            case DUEL_END_OBJ:
                challengef("%s WINS the spar against %s!", victim->short_descr, ch->short_descr);
                break;
            case DUEL_END_TIMEOUT:
                challengef("The spar between %s and %s has timed out. No one wins.", ch->short_descr, victim->short_descr);
                break;
            case DUEL_END_SUICIDE:
                challengef("%s LOSES the spar against %s because they committed suicide!", ch->short_descr, victim->short_descr);
                break;
            case DUEL_END_LINKDEAD:
                challengef("%s LOSES the spar against %s because they were linkdead for too long.", ch->short_descr, victim->short_descr);
                break;
            case DUEL_END_IDLE:
                challengef("%s LOSES the spar against %s because they were idle for too long.", ch->short_descr, victim->short_descr);
                break;
        }
    }

    while (duel->last_player) {
        player = duel->last_player;

        while (player->last_obj) {
            dobj = player->last_obj;
            DUNLINK(dobj, player->first_obj, player->last_obj, next, prev);
            DESTROY_MEMBER(dobj);
        }

        if (duel->stage == DUEL_STAGE_GO) {
            stop_fighting(player->ch, TRUE);
            player->ch->position = POS_STANDING;
            player->ch->stunTimer = 0;
            player->ch->hit = player->hp;
            player->ch->mana = player->mana;
            player->ch->move = player->move;
            player->ch->energy = player->energy;
            player->ch->energy_wait_count = player->ch->energy_wait;
            player->ch->race = player->race;

            while (player->ch->first_affect)
                affect_remove(player->ch, player->ch->first_affect);

            player->ch->affected_by = 0;

            for (af = player->first_affect; af != NULL; af = af->next)
                affect_to_char(player->ch, af);

            while (player->first_affect) {
                af = player->first_affect;
                UNLINK(af, player->first_affect, player->last_affect, next, prev);
                PUT_FREE(af, affect_free);
            }

            char_from_room(player->ch);

            if (get_room_index(DUEL_START_ROOM))
                char_to_room(player->ch, get_room_index(DUEL_START_ROOM));
            else
                char_to_room(player->ch, get_room_index(2));

            /* temporarily set it to this stage so we can save successfully */
            duel->stage = DUEL_STAGE_READY;
            do_save(player->ch, "");
            duel->stage = DUEL_STAGE_GO;

            do_look(player->ch, "auto");
        }

        DUNLINK(player, duel->first_player, duel->last_player, next, prev);
        DESTROY_MEMBER(player);
    }

    while (duel->last_watcher) {
        watcher = duel->last_watcher;

        DUNLINK(watcher, duel->first_watcher, duel->last_watcher, next, prev);
        DESTROY_MEMBER(watcher);
    }

    DUNLINK(duel, first_duel, last_duel, next, prev);
    DESTROY_MEMBER(duel);
}

void
duel_update(void)
{
    DUEL_DATA          *duel;
    DUEL_DATA          *duel_next;
    DUEL_PLAYER_DATA   *player;

    for (duel = first_duel; duel != NULL; duel = duel_next) {
        duel_next = duel->next;

        /* check if we're still in the acceptance stage of the duel */
        if (duel->stage == DUEL_STAGE_READY) {
            /* check if all parties have accepted */
            for (player = duel->first_player; player != NULL; player = player->next)
                if (player->accepted == FALSE)
                    break;

            if (player) {
                /* someone hasn't accepted yet, check for timeout */
                if (current_time >= duel->challenged + DUEL_TIMEOUT_READY) {
                    cancel_duel(duel, NULL, DUEL_END_TIMEOUT);
                }
            }
        }
        else if (duel->stage == DUEL_STAGE_SET) {
            if (--duel->countdown == 0) {
                /* we are ready for a duel! */
                challengef("%s and %s start sparring!", duel->first_player->ch->short_descr, duel->first_player->next->ch->short_descr);
                start_duel(duel);
            }
        }
        else if (duel->stage == DUEL_STAGE_GO) {
            if (current_time >= duel->started + DUEL_TIMEOUT_GO) {
                cancel_duel(duel, NULL, DUEL_END_TIMEOUT);
            }
            else {
                for (player = duel->first_player; player != NULL; player = player->next) {
                    if (player->linkdead > 0 && current_time >= player->linkdead + DUEL_TIMEOUT_LINKDEAD) {
                        cancel_duel(duel, player->ch, DUEL_END_LINKDEAD);
                        break;
                    }
                }
            }
        }
    }

    return;
}

void
duel_rawkill(CHAR_DATA *ch, CHAR_DATA *victim, int type)
{
    DUEL_DATA          *duel;
    DUEL_PLAYER_DATA   *player;

    if (!victim)
        return;

    if ((duel = find_duel(victim)) == NULL)
        return;

    if ((player = find_duel_player(victim)) == NULL)
        return;

    if (duel->stage == DUEL_STAGE_SET) {
        /* they died while waiting to duel(?) */
        cancel_duel(duel, NULL, DUEL_END_TIMEOUT);
        raw_kill(victim, "");
        return;
    }

    if (ch != victim) {
        if (type == DUEL_RAWKILL_NORMAL)
            cancel_duel(duel, ch, DUEL_END_WIN);
        else
            cancel_duel(duel, victim, DUEL_END_OBJ);
    }
    else
        cancel_duel(duel, ch, DUEL_END_SUICIDE);

    return;
}
