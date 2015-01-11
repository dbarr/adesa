
/*****************************************************************************
 *       _/          _/_/_/     _/    _/     _/      ACK! MUD is modified    *
 *      _/_/        _/          _/  _/       _/      Merc2.0/2.1/2.2 code    *
 *     _/  _/      _/           _/_/         _/    (c)Stephen Dooley 1994-6  *
 *    _/_/_/_/      _/          _/  _/              "This mud has not been   *
 *   _/      _/      _/_/_/     _/    _/     _/        tested on animals."   *
 *                                                                           *
 * quest.c :    Routines to generate and handle most aspects of automatic    *
 * ---------    quests for players.                                          *
 *****************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include "merc.h"

IDSTRING(rcsid, "$Id: quest.c,v 1.23 2004/11/18 02:09:49 dave Exp $");

/**** Local Functions ****/
CHAR_DATA          *get_quest_target args((int min_level, int max_level));
CHAR_DATA          *get_quest_giver args((int min_level, int max_level));
OBJ_DATA           *load_quest_object args((CHAR_DATA *target));
char               *my_left  args((char *src, char *dst, int len));
char               *my_right args((char *src, char *dst, int len));

/* qinfo - changed clear_quest() from local function to global */

/* 17 messages, organised by blocks for each personality 
   indented messages are for when the target mob gets killed  */
struct qmessage_type
{
    char               *const message1;
    char               *const message2;
};

const struct qmessage_type qmessages[4][17] = {
    {
            {"", ""}, {"", ""}, {"", ""}, {"", ""}, {"", ""}, {"", ""}, {"", ""}, {"", ""},
            {"", ""}, {"", ""}, {"", ""}, {"", ""}, {"", ""}, {"", ""}, {"", ""}, {"", ""},
            {"", ""}
        },
    {
            {"@@aOh my! My %s @@ahas been stolen from me, and I am too young to find it!", ""},
            {"@@aWould someone please search for my %s@@a? I'm sure that it will be much too hard for me to find.", ""},
            {"@@aWhat will I do without my %s@@a?", ""},
            {"@@aCan't anybody find my %s @@afor me?", ""},
            {"@@aHelp me! My %s @@ais still missing!", ""},
            {"@@aKeep searching for my %s@@a, and i'll ask my mom if she knows who stole it!", ""},
            {"@@aMy mom says %s@@a stole my %s@@a. I know it is too hard for me to get back. Oh my, what will I do?",
                "@@aYeay!! It looks like whoever stole my %s @@ais now dead!! Thank you very much!"},
            {"@@aPlease, can you go kill %s@@a? I need my %s@@a!! I can't sleep without it!!",
                "@@aWell, thank you for killing the nasty thief, but can you please return my %s @@ato me?"},
            {"@@aPlease, can you go kill %s@@a? I need my %s@@a!! I can't sleep without it!!",
                "@@aWell, thank you for killing the nasty thief, but can you please return my %s @@ato me?"},
            {"@@aMy mom says %s@@a stole my %s@@a. I know it is too hard for me to get back. Oh my, what will I do?",
                "@@aYeay!! It looks like whoever stole my %s @@ais now dead!! Thank you very much!"},
            {"@@aIt's time for my nap now, and %s still has my %s@@a!! Can anyone please get it back for me?",
                "@@aPlease, time is running out! Return my %s @@ato me!"},
            {"@@a%s@@a is a real meanie for stealing my %s@@a! Can you pretty pretty pretty please get it back for me?",
                "@@aPlease, time is running out! Return my %s @@ato me!"},
            {"@@aIt's time for my nap now, and %s still has my %s@@a!! Can anyone please get it back for me?",
                "@@aPlease, time is running out! Return my %s @@ato me!"},
            {"@@aOh my, I'll never get up to watch cartoons tomorrow now!  %s @@ais still holding my %s@@a ransom, and I need it for my nap!",
                "@@aHow can I sleep without my %s@@a?"},
            {"@@aI give up! %s @@acan keep my %s @@afor all I care! I didn't want to take a nap, anyway!",
                "@@aI give up! I never want to see my %s @@aagain!"},
            {"@@aMommy, can I watch cartoons now, instead of taking a nap?? PLEASE??", ""},
            {"@@aOh THANK YOU, %s@@a!! Now that I have my %s @@aagain, I can go sleepy sleep!", ""}

        },

    {
            {"Hmm, I seem to have lost my %s@@l. Oh well, didn't really need it much, anyway.", ""},
            {"I wonder where I could find another %s@@l? Anyone have any ideas?", ""},
            {"Where can my %s @@lhave gone?", ""},
            {"I guess no one really cares, anyway, but I think I might need a %s @@llater.", ""},
            {"I guess I should try and find my %s@@l, but I really don't feel like it.", ""},
            {"If anyone has an extra %s@@l, I might be willing to reward them for it.", ""},
            {"Hmm, maybe %s@@l knew something I didn't, and thought it was a good idea to steal my %s@@l. Maybe he could use it, I know I can't.",
                "I guess my %s @@ldidn't help him much, since he is now dead!  I do miss it though."},
            {"Hmm, maybe it IS worth something.  Could someone go kill %s@@l and get my %s@@l back for me?",
                "I guess my %s @@ldidn't help him much, since he is now dead!  I do miss it though."},
            {"Hmm, maybe it IS worth something.  Could someone go kill %s@@l and get my %s@@l back for me?",
                "I guess my %s @@ldidn't help him much, since he is now dead!  I do miss it though."},
            {"I would pay a lot if someone would kill %s @@land get my %s@@l back. I don't really know where it went.",
                "Even though it's not worth very much, my %s @@lis kind of important to me. Oh, well, guess I will never see it again."},
            {"Hmm, maybe it IS worth something.  Could someone go kill %s@@l and get my %s@@l back for me?",
                "I guess my %s @@ldidn't help him much, since he is now dead!  I do miss it though."},
            {"I would pay a lot if someone would kill %s @@land get my %s@@l back. I don't really know where it went.",
                "Even though it's not worth very much, my %s @@lis kind of important to me. Oh, well, guess I will never see it again."},
            {"Oh well, since no one will help me, I guess %s @@lcan keep my %s@@l.",
                "It must be cursed, since everyone who has it is dead. I don't think I want my %s @@lafter all!"},
            {"Oh well, since no one will help me, I guess %s @@lcan keep my %s@@l.",
                "It must be cursed, since everyone who has it is dead. I don't think I want my %s @@lafter all!"},
            {"I give up! %s @@lcan keep my %s @@lfor all I care!",
                "I give up! I never want to see my %s @@lagain!"},
            {"Well, I will stop asking now, but don't ever ask ME for any favors, ok?", ""},
            {"Well, looks like %s @@lhas recovered my %s @@lfor me. Not sure I want it anymore, but thanks anyway.", ""}

        },

    /*
       {
       { "BANZAI! My %s @@lhas been stolen from me!  Will someone brave recover it?",   "" },
       { "Oh! Has no one found my %s @@lfor me yet?",                           "" },
       { "Where can my %s @@lhave gone?",                                   "" },
       { "Can't anybody find my %s @@lfor me?",                             "" },
       { "Help me! My %s @@lhas not yet been recovered!",                       "" },
       { "Keep searching for my %s@@l, and i'll find out who stole it!",            "" },
       { "Were there no witnesses to the theft of my %s?@@l",                   "" },
       { "It was %s @@lwho stole my %s @@lfrom me!  Someone help me!",
       "It looks like whoever stole my %s @@lis now dead!!"                     },
       { "Please, time is running out! Recover my %s @@lfrom %s @@lfor me NOW!",
       "Please, time is running out! Return my %s @@lto me!"                },
       { "Please, time is running out! Recover my %s @@lfrom %s @@lfor me NOW!",
       "Please, time is running out! Return my %s @@lto me!"                },
       { "Please, time is running out! Recover my %s @@lfrom %s @@lfor me NOW!",
       "Please, time is running out! Return my %s @@lto me!"                },
       { "Please, time is running out! Recover my %s @@lfrom %s @@lfor me NOW!",
       "Please, time is running out! Return my %s @@lto me!"                },
       { "I give up! %s @@lcan keep my %s @@lfor all I care!",
       "I give up! I never want to see my %s @@lagain!"                     },
       { "I give up! %s @@lcan keep my %s @@lfor all I care!",
       "I give up! I never want to see my %s @@lagain!"                 },
       { "I give up! %s @@lcan keep my %s @@lfor all I care!", 
       "I give up! I never want to see my %s @@lagain!"                 },
       { "Shoot! Just forget about recovering ANYTHING for me, ok?" ,               "" },
       { "At Last! %s @@lhas recovered %s @@lfor me!",                      "" }

       },  */

    {
            {"@@eMuuaahhhaaahaaaa! Some puny mortal has stolen my %s@@e!  I shall seek revenge!!", ""},
            {"@@eI shall send many minions to seek my %s@@e! All that steal from me shall die!!", ""},
            {"@@eSO, you have defeated my servants.  I shall still regain my %s@@e!!", ""},
            {"@@eI am prepared to reward well anyone that aids the return of my %s@@e. Are any of you puny mortals willing to attempt my challenge?",
                ""},
            {"@@eIf you are worthy, I will grant many favors upon anyone that returns my %s@@e.", ""},
            {"@@mMethlok@@e, By the dark powers, I command you to seek my %s@@e! Now, if any of you worthless mortals wish to attempt to return it, I shall grant you many powers!", ""},
            {"@@eAhhh, my servant has returned, and informs me that %s @@estole my %s@@e. They shall be incinerated by the powers that I command!!!",
                "@@mMethlok@@e has informed me that the weakling that stole my %s @@ehas met his maker!!"},
            {"@@eAre none of you powerful enough to kill %s @@eand regain my %s@@e? Bah!! Mortals are useless, except as side dishes!!",
                "@@eThough my taste for blood has been satiated, my %s @@estill evades my grasp!"},
            {"@@eAre none of you powerful enough to kill %s @@eand regain my %s @@e? Bah!! Mortals are useless, except as side dishes!!",
                "@@eThough my taste for blood has been satiated, my %s @@estill evades my grasp!"},
            {"@@eAre none of you powerful enough to kill %s @@eand regain my %s @@e? Bah!! Mortals are useless, except as side dishes!!",
                "@@eThough my taste for blood has been satiated, my %s @@estill evades my grasp!"},
            {"@@eAre none of you powerful enough to kill %s @@eand regain my %s @@e? Bah!! Mortals are useless, except as side dishes!!",
                "@@eThough my taste for blood has been satiated, my %s @@estill evades my grasp!"},
            {"@@eI should have known that a powerless, puny mortal could never be the servant of my vengeance against %s@@e, or regain my %s@@e!!",
                "@@eI shall rain death upon all of you for refusing to return my %s@@e!!!"},
            {"@@eI should have known that a powerless, puny mortal could never be the servant of my vengeance against %s@@e, or regain my %s@@e!!",
                "@@eI shall rain death upon all of you for refusing to return my %s@@e!!!"},
            {"@@eI shall slay your brothers and poison your fields for refusing to seek %s @@eand return my %s@@e!!!!",
                "@@eThough my vengeance has been served, I shall drink your souls for your failure to return my %s@@e!!!"},
            {"@@eI shall slay your brothers and poison your fields for refusing to seek %s @@eand return my %s@@e!!!!",
                "@@eThough my vengeance has been served, I shall drink your souls for your failure to return my %s@@e!!!"},
            {"@@eDeath and great suffering shall be your punishment for failing me!!!?", ""},
            {"@@eWell done.  It seems that %s @@eat least has a modicum of strength, unlike you worthless idiots who failed to return my %s@@e! My curse shall lie upon you for the rest of your short days!", ""}

        }

};

void
do_quest(CHAR_DATA *ch, char *argument)
{
    char                arg1[MAX_INPUT_LENGTH];
    char                arg2[MAX_INPUT_LENGTH];
    char                arg3[MAX_INPUT_LENGTH];

    extern bool         quest;
    extern bool         auto_quest;
    extern CHAR_DATA   *quest_mob;
    extern CHAR_DATA   *quest_target;
    extern char        *quest_target_name;
    extern OBJ_DATA    *quest_object;
    extern int          quest_timer;
    extern int          quest_wait;
    extern sh_int       quest_personality;

    int                 hunt_flags = 0;

    char                buf[MAX_STRING_LENGTH];
    char                new_long_desc[MAX_STRING_LENGTH];

    argument = one_argument(argument, arg1);

    if (arg1[0] == '\0') {        /* Display status */
        if (!quest) {
            send_to_char("There is no quest currently running.\n\r", ch);
            if (auto_quest)
                send_to_char("Quests are currently running automatically.\n\r", ch);
            if (quest_wait > 0) {
                sprintf(buf, "The next quest may occur in %d minutes.\n\r", quest_wait);
                send_to_char(buf, ch);
            }
            return;
        }
        else
            send_to_char("There is currently a quest running ", ch);

        if (auto_quest)
            send_to_char("(Automatically)", ch);

        send_to_char("\n\rQuest Details:\n\r\n\r", ch);
        if (quest_mob) {

            sprintf(buf, "The questing mobile is: %s [In Room %d]\n\r", quest_mob->short_descr, quest_mob->in_room->vnum);
            send_to_char(buf, ch);
        }
        else {
            send_to_char("The questing mobile is dead!\n\r", ch);
        }
        if (quest_target) {
            sprintf(buf, "Target Mobile is: %s [In Room %d]\n\r", quest_target->short_descr, quest_target->in_room->vnum);
            send_to_char(buf, ch);
        }
        else
            send_to_char("The target mobile is dead!\n\r", ch);

        /* qinfo */
        if (quest_target_name != '\0') {
            sprintf(buf, "Target Mobile Name is: %s\n\r", quest_target_name);
            send_to_char(buf, ch);
        }
        else
            send_to_char("The target mobile name could not be found.\n\r", ch);

        sprintf(buf, "Target Object is: %s.\n\r", quest_object->short_descr);
        send_to_char(buf, ch);

        sprintf(buf, "Quest Object is worth: %d QP, %d Prac, %d GP\n\r", quest_object->value[0], quest_object->value[1], quest_object->value[2]);
        send_to_char(buf, ch);

        sprintf(buf, "The Quest has been running for %d/15 minutes.\n\r", quest_timer);
        send_to_char(buf, ch);

        return;
    }
    if (!strcmp(arg1, "stop")) {
        if (quest) {
            sprintf(buf, "@@NThe quest has been stopped by an @@mImmortal@@N. Please Gossip if you have already gotten the item.\n\r");
            do_echo(ch, buf);
            clear_quest();
        }
        return;
    }

    if (!strcmp(arg1, "start")) {
        /* generate a new quest! */
        if (quest) {
            send_to_char("There is already a quest running...\n\r", ch);
            return;
        }

        generate_auto_quest();
        return;
    }

    if (!str_cmp(arg1, "auto")) {
        auto_quest = !auto_quest;
        if (auto_quest)
            send_to_char("AutoQuest now initiated!\n\r", ch);
        else
            send_to_char("AutoQuest disabled.\n\r", ch);

        save_mudsets();
        return;
    }

    if (!str_cmp(arg1, "mob")) {
        argument = one_argument(argument, arg2);

        if (arg2[0] == '\0') {
            send_to_char("Choose WHO as the quest mob?\n\r", ch);
            return;
        }

        if (quest) {
            send_to_char("Not while a quest is already running!\n\r", ch);
            return;
        }

        if ((quest_mob = get_char_room(ch, arg2)) == NULL) {
            send_to_char("Couldn't find em.\n\r", ch);
            return;
        }

        if (!IS_NPC(quest_mob)) {
            send_to_char("Geez! Mobs only, no players!\n\r", ch);
            return;
        }

        sprintf(buf, "You have chosen %s @@Nas your quest mob!\n\r", quest_mob->short_descr);
        send_to_char(buf, ch);
        return;
    }

    if (!str_cmp(arg1, "target")) {
        argument = one_argument(argument, arg2);

        if (arg2[0] == '\0') {
            send_to_char("Choose WHO as the quest target?\n\r", ch);
            return;
        }

        if (quest) {
            send_to_char("Not while a quest is already running!\n\r", ch);
            return;
        }

        if ((quest_target = get_char_room(ch, arg2)) == NULL) {
            send_to_char("Couldn't find em.\n\r", ch);
            return;
        }

        if (!IS_NPC(quest_target)) {
            send_to_char("Geez! Mobs only, no players!\n\r", ch);
            return;
        }

        sprintf(buf, "You have chosen %s @@Nas your quest target!\n\r", quest_target->short_descr);
        send_to_char(buf, ch);
        return;
    }

    if (!str_cmp(arg1, "manual")) {
        int                 manual_personality;
        int                 manual_qp_worth;

        argument = one_argument(argument, arg2);
        argument = one_argument(argument, arg3);

        if (arg2[0] == '\0') {
            send_to_char("iquest syntax: iquest manual <personality> <qp worth>\n\r", ch);
            return;
        }

        if (!quest_mob || !quest_target) {
            send_to_char("You must first select a quest mob and target using iquest <mob|target> <name>\n\r", ch);
            return;
        }

        if (quest) {
            send_to_char("Not while a quest is already running!\n\r", ch);
            return;
        }

        manual_personality = atoi(arg2);
        manual_qp_worth = atoi(arg3);

        if (manual_personality < 1 || manual_personality > 3) {
            send_to_char("Personality range is 1 to 3.\n\r", ch);
            return;
        }

        if (manual_qp_worth < 1 || manual_qp_worth > 15) {
            send_to_char("Qp worth range is 1 to 15.\n\r", ch);
            return;
        }

        if (manual_personality == 1) {
            hunt_flags = HUNT_WORLD | HUNT_OPENDOOR;
            quest_personality = 1;
        }
        else if (manual_personality == 2) {
            hunt_flags = HUNT_WORLD | HUNT_OPENDOOR | HUNT_PICKDOOR;
            quest_personality = 2;
        }
        else {
            hunt_flags = HUNT_WORLD | HUNT_OPENDOOR | HUNT_PICKDOOR | HUNT_UNLOCKDOOR;
            quest_personality = 3;
        }

        quest_object = load_quest_object(quest_target);
        if (quest_object == NULL) {
            quest = FALSE;
            quest_wait = number_range(1, 3);
            return;
        }

        quest_target_name = NAME(quest_target);

        /* Set values on quest item for Qp, Pracs, Exp, Gold */
        quest_object->value[0] = manual_qp_worth;
        quest_object->value[1] = UMAX(1, (quest_target->level / 18));
        quest_object->value[2] = (quest_target->level * 50);
        quest_object->value[3] = quest_personality;

        quest_timer = 0;
        quest = TRUE;
        new_long_desc[0] = '\0';
        if (quest_mob->long_descr_orig != NULL)
            free_string(quest_mob->long_descr_orig);
        quest_mob->long_descr_orig = str_dup(quest_mob->long_descr);
        sprintf(new_long_desc, "%s @@Nsays have you found my %s ?\n\r", quest_mob->short_descr, quest_object->short_descr);
        if (quest_mob->long_descr != NULL)
            free_string(quest_mob->long_descr);
        quest_mob->long_descr = str_dup(new_long_desc);

        new_long_desc[0] = '\0';
        if (quest_target->long_descr_orig != NULL)
            free_string(quest_target->long_descr_orig);
        quest_target->long_descr_orig = str_dup(quest_target->long_descr);
        sprintf(new_long_desc, "%s @@Nsays I stole the %s !!!\n\r", quest_target->short_descr, quest_object->short_descr);
        if (quest_target->long_descr != NULL)
            free_string(quest_target->long_descr);
        quest_target->long_descr = str_dup(new_long_desc);

        return;
    }

    return;
}

/*
 * get_quest_target : This attempts to pick a random mobile to hold the quest
 * item for the player (questor).  Various checks are made to ensure that the
 * questor has a chance of killing the mobile, etc.
 * Returns NULL if it didn't get a mobile this time.
 */

CHAR_DATA          *
get_quest_target(int min_level, int max_level)
{
    CHAR_DATA          *target;
    int                 index = 0;
    int                 num_mobs = 0;

    for (target = first_char; target != NULL; target = target->next) {
        if (   !IS_NPC(target)
            || target->level < min_level
            || target->level > max_level
            || IS_SET(target->in_room->area->flags, AREA_NOSHOW)
            || IS_SET(target->act, ACT_SENTINEL)
            || IS_SET(target->act, ACT_PET)
            || !target->pIndexData
            || target->pIndexData->pShop
            || IS_AFFECTED(target, AFF_CHARM)
            || target->master != NULL
            || target->fighting != NULL)
            continue;

        num_mobs++;
    }

   index = number_range(1, num_mobs);

    if (max_level > 140)
        max_level = 140;

    for (target = first_char; target != NULL; target = target->next) {
        if (   !IS_NPC(target)
            || target->level < min_level
            || target->level > max_level
            || IS_SET(target->in_room->area->flags, AREA_NOSHOW)
            || IS_SET(target->act, ACT_SENTINEL)
            || IS_SET(target->act, ACT_PET)
            || !target->pIndexData
            || target->pIndexData->pShop
            || IS_AFFECTED(target, AFF_CHARM)
            || target->master != NULL
            || target->fighting != NULL)
            continue;

        if (--index > 0)
            continue;

        break;
    }

    return target;
}

/*
 * load_quest_object : This takes a pointer to OBJ_INDEX_DATA and places the
 * object onto the target.
 */

OBJ_DATA           *
load_quest_object(CHAR_DATA *target)
{
    OBJ_INDEX_DATA     *pObj;
    OBJ_DATA           *object;
    int                 foo;

    foo = number_range(OBJ_VNUM_QUEST_MIN, OBJ_VNUM_QUEST_MAX);

    pObj = get_obj_index(foo);

    if (pObj == NULL) {
        bugf("load_quest_object : Invalid object vnum %d.", foo);
        return NULL;
    }

    object = create_object(pObj, 1);
    obj_to_char(object, target);

    return object;
}

CHAR_DATA          *
get_quest_giver(int min_level, int max_level)
{
    CHAR_DATA          *target;
    int                 index = 0;
    int                 num_mobs = 0;

    for (target = first_char; target != NULL; target = target->next) {
        if (   !IS_NPC(target)
            || target->level < min_level
            || target->level > max_level
            || IS_SET(target->in_room->area->flags, AREA_NOSHOW)
            || IS_SET(target->act, ACT_SENTINEL)
            || IS_SET(target->act, ACT_PET)
            || !target->pIndexData
            || target->pIndexData->pShop
            || IS_AFFECTED(target, AFF_CHARM)
            || target->master != NULL
            || target->fighting != NULL)
            continue;

        num_mobs++;
    }

   index = number_range(1, num_mobs);

    if (max_level > 140)
        max_level = 140;

    for (target = first_char; target != NULL; target = target->next) {
        if (   !IS_NPC(target)
            || target->level < min_level
            || target->level > max_level
            || IS_SET(target->in_room->area->flags, AREA_NOSHOW)
            || IS_SET(target->act, ACT_SENTINEL)
            || IS_SET(target->act, ACT_PET)
            || !target->pIndexData
            || target->pIndexData->pShop
            || IS_AFFECTED(target, AFF_CHARM)
            || target->master != NULL
            || target->fighting != NULL)
            continue;

        if (--index > 0)
            continue;

        break;
    }

    return target;
}

/*
 * quest_inform : Makes the questing mobile give out information to the
 * players on the mud.  Starts off real simple, and gets more helpful as
 * time runs out :P
 */

void
quest_inform(void)
{
    char                buf[MAX_STRING_LENGTH];
    extern CHAR_DATA   *quest_mob;
    extern CHAR_DATA   *quest_target;
    extern OBJ_DATA    *quest_object;
    extern int          quest_timer;
    extern sh_int       quest_personality;
    extern const struct qmessage_type qmessages[4][17];

    /* Work out what the mob should tell the players.... */
    /* Add random element to each case so quests look different each time? */
    if (quest_timer < 6) {
        sprintf(buf, qmessages[quest_personality][quest_timer].message1, quest_object->short_descr);
    }
    else {
        if (quest_target)
            sprintf(buf, qmessages[quest_personality][quest_timer].message1, quest_target->short_descr, quest_object->short_descr);
        else
            sprintf(buf, qmessages[quest_personality][quest_timer].message2, quest_object->short_descr);
    }

    quest_timer++;
    if (quest_mob && quest_timer < 16)
        do_crusade(quest_mob, buf);

    if (!quest_mob) {
        clear_quest();
    }
    return;
}

/* qinfo additions to this function */
void
quest_complete(CHAR_DATA *ch)
{
    extern CHAR_DATA   *quest_mob;
    extern char        *quest_target_name;
    extern OBJ_DATA    *quest_object;
    extern sh_int       quest_personality;
    extern const struct qmessage_type qmessages[4][17];
    QUEST_DATA         *questinfo;
    QUEST_DATA         *questcheck;
    sh_int              questcount = 0;

    char                buf[MAX_STRING_LENGTH];

    sprintf(buf, qmessages[quest_personality][16].message1, NAME(ch), quest_object->short_descr);
    do_crusade(quest_mob, buf);

    for (questcheck = first_quest; questcheck != NULL; questcheck = questcheck->next) {
        questcount++;
    }

    questcheck = first_quest;

    if (questcount >= MAX_QINFO && questcheck != NULL) {
        UNLINK(questcheck, first_quest, last_quest, next, prev);
        free_string(questcheck->mob);
        free_string(questcheck->thief);
        free_string(questcheck->ch);
        PUT_FREE(questcheck, quest_free);
    }

    GET_FREE(questinfo, quest_free);
    if (quest_mob)
        questinfo->mob = str_dup(NAME(quest_mob));
    else
        questinfo->mob = str_dup("");
    if (quest_target_name != '\0')
        questinfo->thief = str_dup(quest_target_name);
    else
        questinfo->thief = str_dup("");
    if (ch)
        questinfo->ch = str_dup(NAME(ch));
    else
        questinfo->ch = str_dup("");
    if (quest_object)
        questinfo->qp = quest_object->value[0];
    else
        questinfo->qp = -1;

    questinfo->flags = 0;
    SET_BIT(questinfo->flags, QUEST_NORMAL);
    if (!IS_NPC(ch) && IS_SET(ch->config, PLR_MASKQP))
        SET_BIT(questinfo->flags, QUEST_MASK);

    LINK(questinfo, first_quest, last_quest, next, prev);

    clear_quest();
    return;
}

/* qinfo additions to this function */
void
quest_cancel()
{
    extern CHAR_DATA   *quest_mob;
    extern char        *quest_target_name;
    extern OBJ_DATA    *quest_object;
    QUEST_DATA         *questinfo;
    QUEST_DATA         *questcheck;
    sh_int              questcount = 0;

    if (quest_mob)
        do_crusade(quest_mob, "Shoot! Just forget about recovering ANYTHING for me, ok?");

    if (quest_mob) {
        for (questcheck = first_quest; questcheck != NULL; questcheck = questcheck->next) {
            questcount++;
        }

        questcheck = first_quest;

        if (questcount >= MAX_QINFO && questcheck != NULL) {
            UNLINK(questcheck, first_quest, last_quest, next, prev);
            free_string(questcheck->mob);
            free_string(questcheck->thief);
            free_string(questcheck->ch);
            PUT_FREE(questcheck, quest_free);
        }

        GET_FREE(questinfo, quest_free);
        if (quest_mob)
            questinfo->mob = str_dup(NAME(quest_mob));
        else
            questinfo->mob = str_dup("");
        if (quest_target_name != '\0')
            questinfo->thief = str_dup(quest_target_name);
        else
            questinfo->thief = str_dup("");

        questinfo->ch = str_dup("");

        if (quest_object)
            questinfo->qp = quest_object->value[0];
        else
            questinfo->qp = -1;

        questinfo->flags = 0;
        SET_BIT(questinfo->flags, QUEST_ENDED);

        LINK(questinfo, first_quest, last_quest, next, prev);
    }

    clear_quest();
    return;
}

void
clear_quest()
{
    extern bool         quest;
    extern CHAR_DATA   *quest_mob;
    extern CHAR_DATA   *quest_target;

    /* qinfo */
    extern char        *quest_target_name;
    extern OBJ_DATA    *quest_object;
    extern int          quest_timer;
    extern int          quest_wait;
    extern sh_int       quest_personality;

    /* Clear ALL values, ready for next quest */

    quest = FALSE;
    extract_obj(quest_object);
    if (quest_mob) {
        free_string(quest_mob->long_descr);
        quest_mob->long_descr = str_dup(quest_mob->long_descr_orig);
        free_string(quest_mob->long_descr_orig);
        quest_mob->long_descr_orig = NULL;
    }
    if (quest_target) {
        free_string(quest_target->long_descr);
        quest_target->long_descr = str_dup(quest_target->long_descr_orig);
        free_string(quest_target->long_descr_orig);
        quest_target->long_descr_orig = NULL;
    };

    quest_mob = NULL;
    quest_target = NULL;
    /* qinfo */
    quest_target_name = '\0';
    quest_object = NULL;
    quest_timer = 0;
    quest_wait = 2 + number_range(1, 4);
    quest_personality = 0;

    return;
}

void
generate_auto_quest()
{

    extern bool         quest;

    extern CHAR_DATA   *quest_mob;
    extern CHAR_DATA   *quest_target;
    extern char        *quest_target_name;
    extern OBJ_DATA    *quest_object;
    extern int          quest_timer;
    extern int          quest_wait;
    extern sh_int       quest_personality;
    int                 hunt_flags = 0;
    char                new_long_desc[MAX_STRING_LENGTH];
    sh_int              loop_counter = 0;
    bool                all_adepts = TRUE;
    CHAR_DATA          *ch;
    int                 a = 140;
    int                 b = 0;
    sh_int              average_level = 0;

    /* generate a new quest! */
    if (quest) {
        return;
    }

    quest_mob = NULL;
    quest_target = NULL;

    for (ch = first_player; ch != NULL; ch = ch->next_player)
        if (ch->adept_level < 1 && !IS_SET(ch->pcdata->pflags, PFLAG_SAFE)) {
            all_adepts = FALSE;
            break;
        }

    if (number_range(0, 99) < 50 && all_adepts == FALSE) {
        average_level = number_range(0, 99);
/*
        if (average_level < 20) {
            a = number_range(5, 25);
            b = number_range(30, 45);
            hunt_flags = HUNT_WORLD | HUNT_OPENDOOR;
            quest_personality = 1;
        }
        else
*/
        if (average_level < 40) {
            a = number_range(40, 55);
            b = number_range(60, 84);
            hunt_flags = HUNT_WORLD | HUNT_OPENDOOR | HUNT_PICKDOOR;
            quest_personality = 2;
        }
        else {
            a = number_range(100, 110);
            b = number_range(115, 140);
            hunt_flags = HUNT_WORLD | HUNT_OPENDOOR | HUNT_PICKDOOR | HUNT_UNLOCKDOOR;
            quest_personality = 3;
        }
    }
    else {
        if (first_desc && first_desc->connected == CON_PLAYING) {
            DESCRIPTOR_DATA    *d;
            sh_int              player_count = 0, total_levels = 0;

            for (d = first_desc; d; d = d->next) {
                if (d->connected != CON_PLAYING || IS_IMMORTAL(d->character) || (d->character && d->character->pcdata && IS_SET(d->character->pcdata->pflags, PFLAG_SAFE)))
                    continue;
                player_count += 1;
                total_levels += get_pseudo_level(d->character);
            }

            player_count = UMAX(1, player_count);
            average_level = (total_levels / player_count);
        }

/*
        if (average_level < 45) {
            a = number_range(5, 25);
            b = number_range(30, 45);
            hunt_flags = HUNT_WORLD | HUNT_OPENDOOR;
            quest_personality = 1;
            average_level = 10;
        }
        else
*/
        if (average_level < 85 && all_adepts == FALSE) {
            a = number_range(40, 55);
            b = number_range(60, 84);
            hunt_flags = HUNT_WORLD | HUNT_OPENDOOR | HUNT_PICKDOOR;
            quest_personality = 2;
        }
        else {
            a = number_range(100, 110);
            b = number_range(115, 140);
            hunt_flags = HUNT_WORLD | HUNT_OPENDOOR | HUNT_PICKDOOR | HUNT_UNLOCKDOOR;
            quest_personality = 3;
        }

    }

    while ((quest_mob == NULL)
        && (loop_counter < 500)) {
        loop_counter++;
        quest_mob = get_quest_giver(a, b);
        if ((quest_mob == NULL)
            || ((h_find_dir(get_room_index(ROOM_VNUM_TEMPLE), quest_mob->in_room, hunt_flags) < 0)))
            quest_mob = NULL;
    }

    if (quest_mob == NULL) {
        quest = FALSE;
        quest_wait = number_range(1, 3);
        return;
    }
    loop_counter = 0;
    while ((quest_target == NULL)
        && (loop_counter < 500)) {
        loop_counter++;
        quest_target = get_quest_target(a, b);
        if ((quest_target == NULL)
            || ((h_find_dir(get_room_index(ROOM_VNUM_TEMPLE), quest_target->in_room, hunt_flags) < 0)))
            quest_target = NULL;

        if (quest_mob == quest_target)
            quest_target = NULL;
    }

    if (quest_target == NULL) {
        quest = FALSE;
        quest_wait = number_range(1, 3);
        return;
    }

    quest_target_name = NAME(quest_target);

    quest_object = load_quest_object(quest_target);
    if (quest_object == NULL) {
        quest = FALSE;
        quest_wait = number_range(1, 3);
        return;
    }
    /* Set values on quest item for Qp, Pracs, Exp, Gold */
    quest_object->value[0] = UMAX(1, (quest_target->level / 20)) + number_range(0, 1);;
    quest_object->value[1] = UMAX(1, (quest_target->level / 18)) + number_range(0, 2);;
    quest_object->value[2] = (quest_target->level * 50);
    quest_object->value[3] = quest_personality;

    if (number_percent() < 10) {
        quest_object->value[0] += 2;
        quest_object->value[1] += 3;
        quest_object->value[2] *= 2;

    }

    quest_timer = 0;
    quest = TRUE;
    new_long_desc[0] = '\0';
    if (quest_mob->long_descr_orig != NULL)
        free_string(quest_mob->long_descr_orig);
    quest_mob->long_descr_orig = str_dup(quest_mob->long_descr);
    sprintf(new_long_desc, "%s @@Nsays have you found my %s ?\n\r", quest_mob->short_descr, quest_object->short_descr);
    if (quest_mob->long_descr != NULL)
        free_string(quest_mob->long_descr);
    quest_mob->long_descr = str_dup(new_long_desc);
    new_long_desc[0] = '\0';
    if (quest_target->long_descr_orig != NULL)
        free_string(quest_target->long_descr_orig);
    quest_target->long_descr_orig = str_dup(quest_target->long_descr);
    sprintf(new_long_desc, "%s @@Nsays I stole the %s !!!\n\r", quest_target->short_descr, quest_object->short_descr);
    if (quest_target->long_descr != NULL)
        free_string(quest_target->long_descr);
    quest_target->long_descr = str_dup(new_long_desc);

    return;

}

    /* qinfo */
void
do_qinfo(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    char                catbuf[MAX_STRING_LENGTH];
    char                _buf2[MSL];
    char               *buf2 = _buf2;
    QUEST_DATA         *questinfo;
    sh_int              count = 0;
    int                 space = 0;
    int                 c = 0;

    buf[0] = 0;
    catbuf[0] = 0;
    _buf2[0] = 0;

    if (IS_NPC(ch))
        return;

    if (first_quest == NULL) {
        send_to_char("There have been no previous quests.\n\r", ch);
        return;
    }

    send_to_char("@@d.------------------------------------------------------------=@@g(@@W Quest Info @@g)@@d=-.\n\r", ch);
    send_to_char("@@d| @@aS @@c## @@d| @@gVictim                  @@d| @@gThief                  @@d|      @@gQuestor @@d| @@yQP @@d|\n\r", ch);
    send_to_char("@@d|------+-------------------------+------------------------+--------------+----|\n\r", ch);

    for (questinfo = first_quest; questinfo != NULL; questinfo = questinfo->next) {
        sprintf(catbuf, "@@d| %s @@c%2d @@d| @@N", (IS_SET(questinfo->flags, QUEST_NORMAL) ? "@@aC" : (IS_SET(questinfo->flags, QUEST_ENDED) ? "@@aE" : "@@eK")), ++count);
        safe_strcat(MSL, buf, catbuf);

        buf2 = my_left(questinfo->mob, buf2, 23);
        safe_strcat(MSL, buf, buf2);
        space = 23 - my_strlen(buf2);
        for (c = 0; c < space; c++) {
            safe_strcat(MSL, buf, " ");
        }

        safe_strcat(MSL, buf, " @@N@@d| @@N");

        buf2 = my_left(questinfo->thief, buf2, 22);
        safe_strcat(MSL, buf, buf2);
        space = 22 - my_strlen(buf2);
        for (c = 0; c < space; c++) {
            safe_strcat(MSL, buf, " ");
        }

        safe_strcat(MSL, buf, " @@N@@d| @@N");

        space = 12 - my_strlen((strlen(questinfo->ch) > 0) ? capitalize(questinfo->ch) : "            ");
        for (c = 0; c < space; c++) {
            safe_strcat(MSL, buf, " ");
        }
        safe_strcat(MSL, buf, (strlen(questinfo->ch) > 0) ? capitalize(questinfo->ch) : "            ");

        if (questinfo->qp && (!IS_SET(questinfo->flags, QUEST_MASK) || IS_IMMORTAL(ch)))
            sprintf(catbuf, " @@N@@d| @@y%2d @@d|\n\r", questinfo->qp);
        else
            sprintf(catbuf, " @@N@@d|    @@d|\n\r");

        safe_strcat(MSL, buf, catbuf);
    }

    send_to_char(buf, ch);
    send_to_char("@@d'-----------------------------------------------------------------------------'@@N\n\r", ch);

    return;
}

/* returns left section of string (takes colour codes into account) */
char               *
my_left(char *src, char *dst, int len)
{
    char               *s = src;
    char               *d = dst;
    int                 a = 0;

    for (a = 0; a < len && *s;) {
        if (*s == '\n' || *s == '\r') {
            s++;
            continue;
        }

        if (*s != '@' || (*s == '@' && *(s + 1) != '@')) {
            *d++ = *s++;
            a++;
        }
        else if (*(s + 2) != '\0') {
            *d++ = *s++;
            *d++ = *s++;
            *d++ = *s++;
        }
        else
            break;
    }

    *d = 0;
    a = len - my_strlen(dst);

    while (a-- > 0)
        *d++ = ' ';

    *d = 0;

    return dst;
}

/* returns right section of string (takes colour codes into account) */
char               *
my_right(char *src, char *dst, int len)
{
    char               *s = src;
    char               *d = dst;
    int                 a = 0, b = 0;

    b = len - my_strlen(src);

    if (b <= 0)
        a = 0;
    else
        while (b-- > 0) {
            a++;
            *d++ = ' ';
        }

    for (; a < len && *s;) {
        if (*s == '\n' || *s == '\r') {
            s++;
            continue;
        }

        if (*s != '@' || (*s == '@' && *(s + 1) != '@')) {
            *d++ = *s++;
            a++;
        }
        else if (*(s + 2) != '\0') {
            *d++ = *s++;
            *d++ = *s++;
            *d++ = *s++;
        }
        else
            break;
    }

    *d = 0;

    return dst;
}

/* returns left section of string (takes colour codes into account) */
char               *
my_left2(char *src, char *dst, int len, char fill)
{
    char               *s = src;
    char               *d = dst;
    int                 a = 0;

    for (a = 0; a < len && *s;) {
        if (*s == '\n' || *s == '\r') {
            s++;
            continue;
        }

        if (*s != '@' || (*s == '@' && *(s + 1) != '@')) {
            *d++ = *s++;
            a++;
        }
        else if (*(s + 2) != '\0') {
            *d++ = *s++;
            *d++ = *s++;
            *d++ = *s++;
        }
        else
            break;
    }

    *d = 0;
    a = len - my_strlen(dst);

    while (a-- > 0)
        *d++ = fill;

    *d = 0;

    return dst;
}
