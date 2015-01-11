
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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include "merc.h"
#include "treasury.h"

IDSTRING(rcsid, "$Id: interp.c,v 1.53 2005/01/19 23:43:09 dave Exp $");

bool check_social   args((CHAR_DATA *ch, char *command, char *argument));
bool MP_Commands    args((CHAR_DATA *ch));
void remove_excess_at args((char *argument));

/*
 * Command logging types.
 */
#define LOG_NORMAL      0
#define LOG_ALWAYS      1
#define LOG_NEVER       2

/*
 * Log-all switch.
 */
bool                fLogAll = FALSE;

/*
 * Command table.
 */
const struct cmd_type cmd_table[] = {
    /*
     * Common movement commands.
     */

    {"north", do_north, POS_STANDING, 0, LOG_NORMAL, FALSE},
    {"east", do_east, POS_STANDING, 0, LOG_NORMAL, FALSE},
    {"south", do_south, POS_STANDING, 0, LOG_NORMAL, FALSE},
    {"west", do_west, POS_STANDING, 0, LOG_NORMAL, FALSE},
    {"up", do_up, POS_STANDING, 0, LOG_NORMAL, FALSE},
    {"down", do_down, POS_STANDING, 0, LOG_NORMAL, FALSE},

    /*
     * Common other commands.
     * Placed here so one and two letter abbreviations work.
     */
    {"buy", do_buy, POS_RESTING, 0, LOG_NORMAL, TRUE},
    {"cast", do_cast, POS_FIGHTING, 0, LOG_NORMAL, TRUE},
    {"exits", do_exits, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"get", do_get, POS_RESTING, 0, LOG_NORMAL, TRUE},
    {"gain", do_gain, POS_STANDING, 0, LOG_NORMAL, FALSE},
    {"inventory", do_inventory, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"kill", do_kill, POS_FIGHTING, 0, LOG_NORMAL, FALSE},
    {"look", do_look, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"order", do_order, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"rest", do_rest, POS_RESTING, 0, LOG_NORMAL, TRUE},
    {"sleep", do_sleep, POS_SLEEPING, 0, LOG_NORMAL, TRUE},
    {"stand", do_stand, POS_SLEEPING, 0, LOG_NORMAL, TRUE},
    {"tell", do_tell, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"token", do_token, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"whisper", do_whisper, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"wield", do_wear, POS_RESTING, 0, LOG_NORMAL, TRUE},
    {"wizhelp", do_wizhelp, POS_DEAD, 81, LOG_NORMAL, FALSE},
    {"loot", do_loot, POS_STANDING, 20, LOG_ALWAYS, FALSE},
    /*
     * Informational commands.
     */

    {"test", do_test, POS_DEAD, 0, LOG_NORMAL, FALSE},

    {"affected", do_affected, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"areas", do_areas, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"appraise", do_appraise, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"bug", do_bug, POS_DEAD, 0, LOG_ALWAYS, FALSE},
    {"clist", do_clan_list, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"commands", do_commands, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"compare", do_compare, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"consider", do_consider, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"credits", do_credits, POS_DEAD, 0, LOG_NORMAL, FALSE},

    {"cwhere", do_cwhere, POS_DEAD, CLAN_ONLY, LOG_NORMAL, FALSE},

    {"delete", do_delete, POS_STANDING, 0, LOG_NORMAL, FALSE},
    {"diagnose", do_diagnose, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"equipment", do_equipment, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"eqaffects", do_eqaffects, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"examine", do_examine, POS_RESTING, 0, LOG_NORMAL, TRUE},
    {"help", do_help, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"helps", do_helps, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"shelp", do_shelp, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"rules", do_rules, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"heal", do_heal, POS_STANDING, 0, LOG_NORMAL, TRUE},
    {"idea", do_idea, POS_DEAD, 0, LOG_ALWAYS, FALSE},

    {"note", do_note, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"news", do_news, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"maxplayers", do_max_players, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"report", do_report, POS_DEAD, 0, LOG_NORMAL, TRUE},
    {"pagelength", do_pagelen, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"players", do_players, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"read", do_read, POS_STANDING, 0, LOG_NORMAL, FALSE},
    {"avatar", do_avatar, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"rlist", do_race_list, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"score", do_score, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"slist", do_slist, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"socials", do_socials, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"status", do_status, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"time", do_time, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"typo", do_typo, POS_DEAD, 0, LOG_ALWAYS, FALSE},
    {"weather", do_weather, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"who", do_who, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"whois", do_whois, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"wizlist", do_wizlist, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"write", do_write, POS_STANDING, 0, LOG_NORMAL, FALSE},
    {"edit", do_edit, POS_STANDING, 0, LOG_NORMAL, FALSE},

    /*
     * config +commands standalone. -dave
     */

    {"autogold", do_autogold, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"autosplit", do_autosplit, POS_DEAD, 0, LOG_NORMAL, FALSE},

    /*
     * Configuration commands.
     */

    {"accept", do_accept, POS_DEAD, 0, LOG_NORMAL, FALSE},

    {"alias", do_alias, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"autoexit", do_autoexit, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"autoloot", do_autoloot, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"autosac", do_autosac, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"banish", do_banish, POS_DEAD, CLAN_ONLY, LOG_NORMAL, FALSE},
    {"blank", do_blank, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"brief", do_brief, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"channels", do_channels, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"colour", do_colour, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"color",  do_colour, POS_DEAD, 0, LOG_NORMAL, FALSE}, /* death to americans! */
    {"colist", do_colist, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"combine", do_combine, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"config", do_config, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"compress", do_compress, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"description", do_description, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"dleft", do_dleft, POS_DEAD, 0, LOG_NORMAL, FALSE},

    {"ignore", do_ignore, POS_DEAD, 0, LOG_NORMAL, FALSE},

    {"password", do_password, POS_DEAD, 0, LOG_NEVER, FALSE},
    {"prompt", do_prompt, POS_DEAD, 0, LOG_ALWAYS, FALSE},
    {"bprompt", do_bprompt, POS_DEAD, 0, LOG_ALWAYS, FALSE},
    {"nprompt", do_nprompt, POS_DEAD, 0, LOG_ALWAYS, FALSE},
    {"title", do_title, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"wimpy", do_wimpy, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"worth", do_worth, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"stance", do_stance, POS_FIGHTING, 0, LOG_NORMAL, FALSE},

    /*
     * Communication commands.
     */
    {"answer", do_answer, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"ask", do_ask, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"beep", do_beep, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"clan", do_clan, POS_RESTING, CLAN_ONLY, LOG_NORMAL, FALSE},
    {"creator", do_creator, POS_DEAD, 90, LOG_NORMAL, FALSE},
    {")", do_creator, POS_DEAD, 90, LOG_NORMAL, FALSE},
    /* { "gossip",         do_gossip,      POS_RESTING,     0,  LOG_NORMAL },
       { ".",              do_gossip,      POS_RESTING,     0,  LOG_NORMAL }, */
    {"pemote", do_pemote, POS_RESTING, 0, LOG_NORMAL, TRUE},
    {"emote", do_emote, POS_RESTING, 0, LOG_NORMAL, TRUE},
    {",", do_emote, POS_RESTING, 0, LOG_NORMAL, TRUE},
    {"flame", do_flame, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"finger", do_finger, POS_DEAD, 86, LOG_NORMAL, FALSE},
    {"gtell", do_gtell, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {";", do_gtell, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"music", do_music, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"newbie", do_newbie, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"practice", do_practice, POS_SLEEPING, 0, LOG_NORMAL, FALSE},
    {"pray", do_pray, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"quest", do_quest2, POS_RESTING, 0, LOG_NORMAL, FALSE},

    /* qinfo */
    {"qinfo", do_qinfo, POS_RESTING, 5, LOG_NORMAL, FALSE},

    {"question", do_question, POS_SLEEPING, 0, LOG_NORMAL, FALSE},
    {"race", do_race, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"reply", do_reply, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"ireply", do_ireply, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"say", do_say, POS_RESTING, 0, LOG_NORMAL, TRUE},
    {"'", do_say, POS_RESTING, 0, LOG_NORMAL, TRUE},
    {"shout", do_shout, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"tongue", do_tongue, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"yell", do_yell, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"zzz", do_zzz, POS_SLEEPING, 0, LOG_NORMAL, FALSE},
    {"trivia", do_trivia, POS_RESTING, 0, LOG_NORMAL, FALSE},
/*  {"vamp", do_familytalk, POS_RESTING, VAMP_ONLY, LOG_NORMAL, FALSE}, */
    {"{", do_remorttalk, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"diplomat", do_diptalk, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"crusade", do_crusade, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"adept", do_adepttalk, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"ooc", do_ooc, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"pkok", do_pkok, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"ally", do_ally, POS_RESTING, 20, LOG_NORMAL, FALSE},

    /*
     * Object manipulation commands.
     */

    {"adapt", do_adapt, POS_STANDING, 0, LOG_NORMAL, FALSE},
    {"auction", do_auction, POS_STANDING, 0, LOG_NORMAL, FALSE},
    {"bank", do_bank, POS_STANDING, 0, LOG_NORMAL, FALSE},
    {"bid", do_bid, POS_STANDING, 0, LOG_NORMAL, FALSE},
    {"brandish", do_brandish, POS_RESTING, 0, LOG_NORMAL, TRUE},
    {"cdonate", do_cdonate, POS_RESTING, CLAN_ONLY, LOG_NORMAL, FALSE},
    {"close", do_close, POS_RESTING, 0, LOG_NORMAL, TRUE},
    {"clutch", do_clutch, POS_STANDING, 0, LOG_NORMAL, TRUE},
    {"donate", do_donate, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"drink", do_drink, POS_RESTING, 0, LOG_NORMAL, TRUE},
    {"drop", do_drop, POS_RESTING, 0, LOG_NORMAL, TRUE},
    {"eat", do_eat, POS_RESTING, 0, LOG_NORMAL, TRUE},
    {"enter", do_enter, POS_STANDING, 0, LOG_NORMAL, FALSE},
    {"fill", do_fill, POS_RESTING, 0, LOG_NORMAL, TRUE},
    {"give", do_give, POS_RESTING, 0, LOG_NORMAL, TRUE},
    {"hold", do_wear, POS_RESTING, 0, LOG_NORMAL, TRUE},
    {"list", do_list, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"lock", do_lock, POS_RESTING, 0, LOG_NORMAL, TRUE},
    {"open", do_open, POS_STANDING, 0, LOG_NORMAL, TRUE},

    {"make", do_make, POS_DEAD, CLAN_ONLY, LOG_NORMAL, FALSE},
    {"pick", do_pick, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"put", do_put, POS_RESTING, 0, LOG_NORMAL, TRUE},
    {"quaff", do_quaff, POS_RESTING, 0, LOG_NORMAL, TRUE},
    {"recite", do_recite, POS_RESTING, 0, LOG_NORMAL, TRUE},
    {"remove", do_remove, POS_RESTING, 0, LOG_NORMAL, TRUE},
    {"sedit", do_sedit, POS_DEAD, 90, LOG_ALWAYS, FALSE},
    {"sell", do_sell, POS_RESTING, 0, LOG_NORMAL, TRUE},
    {"take", do_get, POS_RESTING, 0, LOG_NORMAL, TRUE},
    {"sacrifice", do_sacrifice, POS_RESTING, 0, LOG_NORMAL, TRUE},
    {"unlock", do_unlock, POS_RESTING, 0, LOG_NORMAL, TRUE},
    {"value", do_value, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"wear", do_wear, POS_RESTING, 0, LOG_NORMAL, TRUE},
    {"zap", do_zap, POS_RESTING, 0, LOG_NORMAL, TRUE},
    {"connect", do_connect, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"enchant", do_enchant, POS_STANDING, 0, LOG_NORMAL, FALSE},
    {"unpractice", do_unpractice, POS_SLEEPING, 0, LOG_NORMAL, FALSE},

    /*
     * Combat commands.
     */
    {"assist", do_assist, POS_STANDING, 0, LOG_NORMAL, FALSE},
    {"backstab", do_backstab, POS_STANDING, 0, LOG_NORMAL, FALSE},
    {"bs", do_backstab, POS_STANDING, 0, LOG_NORMAL, FALSE},
    {"dirt", do_dirt, POS_FIGHTING, 0, LOG_NORMAL, FALSE},
    {"trip", do_trip, POS_FIGHTING, 0, LOG_NORMAL, FALSE},
    {"smash", do_smash, POS_STANDING, 0, LOG_NORMAL, FALSE},
    {"bash", do_bash, POS_FIGHTING, 0, LOG_NORMAL, FALSE},
    {"berserk", do_berserk, POS_FIGHTING, 0, LOG_NORMAL, FALSE},
    {"circle", do_circle, POS_FIGHTING, 0, LOG_NORMAL, FALSE},
    {"cinfo", do_cinfo, POS_RESTING, 5, LOG_NORMAL, FALSE},
    {"cleft", do_cleft, POS_RESTING, 0, LOG_NORMAL, TRUE},
    {"disarm", do_disarm, POS_FIGHTING, 0, LOG_NORMAL, FALSE},
    {"spar", do_duel, POS_STANDING, 20, LOG_NORMAL, FALSE},
/*  {"feed", do_feed, POS_FIGHTING, VAMP_ONLY, LOG_NORMAL, FALSE}, */
    {"flee", do_flee, POS_FIGHTING, 0, LOG_NORMAL, FALSE},
    {"headbutt", do_headbutt, POS_FIGHTING, 0, LOG_NORMAL, FALSE},
    {"kick", do_kick, POS_FIGHTING, 0, LOG_NORMAL, FALSE},
    {"knee", do_knee, POS_FIGHTING, 0, LOG_NORMAL, FALSE},
    {"murde", do_murde, POS_FIGHTING, 5, LOG_NORMAL, FALSE},
    {"murder", do_murder, POS_FIGHTING, 5, LOG_NORMAL, FALSE},
    {"punch", do_punch, POS_FIGHTING, 0, LOG_NORMAL, FALSE},
    {"rescue", do_rescue, POS_FIGHTING, 0, LOG_NORMAL, TRUE},
    /* staking is now automatic! -dave
       { "stake",      do_stake,   POS_STANDING,   5,  LOG_ALWAYS },
     */
    {"stun", do_stun, POS_FIGHTING, 0, LOG_NORMAL, FALSE},
    {"frenzy", do_frenzy, POS_FIGHTING, 0, LOG_NORMAL, FALSE},
    {"charge", do_charge, POS_FIGHTING, 0, LOG_NORMAL, FALSE},
    {"target", do_target, POS_FIGHTING, 0, LOG_NORMAL, FALSE},
    {"grab", do_grab, POS_FIGHTING, 0, LOG_NORMAL, FALSE},
    {"charmpurge", do_charmpurge, POS_STANDING, 0, LOG_NORMAL, FALSE},

    /*
     * Miscellaneous commands.
     */
    {"afk", do_afk, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"awaymsg", do_afk_msg, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"follow", do_follow, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"gold", do_gold, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"group", do_group, POS_SLEEPING, 0, LOG_NORMAL, FALSE},
    {"guild", do_guild, POS_FIGHTING, 0, LOG_NORMAL, FALSE},
    {"hide", do_hide, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"hunt", do_hunt, POS_STANDING, 0, LOG_NORMAL, FALSE},
    {"dismount", do_dismount, POS_STANDING, 1, LOG_NORMAL, FALSE},
    {"mount", do_mount, POS_STANDING, 1, LOG_NORMAL, FALSE},

    {"leav", do_leav, POS_STANDING, CLAN_ONLY, LOG_NORMAL, FALSE},
    {"leave", do_leave, POS_STANDING, CLAN_ONLY, LOG_NORMAL, FALSE},

    {"qui", do_qui, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"quit", do_quit, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"recall", do_recall, POS_FIGHTING, 0, LOG_NORMAL, TRUE},
    {"/", do_recall, POS_FIGHTING, 0, LOG_NORMAL, TRUE},
    {"kdon", do_kdon, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"keep", do_keep, POS_STANDING, 0, LOG_NORMAL, FALSE},
    {"home", do_clan_recall, POS_STANDING, CLAN_ONLY, LOG_NORMAL, FALSE},
    {"rent", do_rent, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"save", do_save, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"savecheck", do_savecheck, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"scan", do_scan, POS_STANDING, 0, LOG_NORMAL, FALSE},
    {"shadowform", do_shadowform, POS_STANDING, 0, LOG_NORMAL, FALSE},
    {"sneak", do_sneak, POS_STANDING, 0, LOG_NORMAL, FALSE},
    {"spells", do_spells, POS_SLEEPING, 0, LOG_NORMAL, FALSE},
    {"split", do_split, POS_RESTING, 0, LOG_NORMAL, TRUE},
    {"steal", do_steal, POS_STANDING, 0, LOG_NORMAL, FALSE},
    {"train", do_train, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"trade", do_trade, POS_STANDING, 5, LOG_ALWAYS, FALSE},
    {"visible", do_visible, POS_SLEEPING, 0, LOG_NORMAL, FALSE},
    {"wake", do_wake, POS_SLEEPING, 0, LOG_NORMAL, FALSE},
    {"where", do_where, POS_RESTING, 0, LOG_NORMAL, FALSE},

    {"disguise", do_disguise, POS_STANDING, 0, LOG_ALWAYS, FALSE},
    {"politics", do_politics, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"rulers", do_rulers, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"version", do_version, POS_RESTING, 0, LOG_NORMAL, FALSE},

    /*
     *    Vampyre and REMORT SKILLS Zen
     */

/*  {"family", do_family, POS_RESTING, VAMP_ONLY, LOG_NORMAL, FALSE},
    {"disguise", do_disguise, POS_STANDING, 0, LOG_ALWAYS, FALSE},
    {"instruct", do_instruct, POS_STANDING, VAMP_ONLY, LOG_NORMAL, FALSE},
*/
    {"scout", do_scout, POS_STANDING, 1, LOG_NORMAL, FALSE},

    /* 
     * NEW CLAN COMMANDS Zen
     */

    {"ctoggle", do_ctoggle, POS_RESTING, CLAN_ONLY, LOG_NORMAL, FALSE},
    {"negotiate", do_negotiate, POS_RESTING, CLAN_ONLY, LOG_NORMAL, FALSE},
/*  {"council", do_council, POS_RESTING, 0, LOG_NORMAL, FALSE}, */
    {"qpspend", do_qpspend, POS_STANDING, 1, LOG_NORMAL, FALSE},

    /*
     * Immortal commands.
     */

    {"preport", do_preport, POS_DEAD, 0, LOG_NORMAL, TRUE},
    {"sreport", do_sreport, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"pgroup", do_pgroup, POS_SLEEPING, 0, LOG_NORMAL, FALSE},
    {"gauction", do_gauction, POS_STANDING, 0, LOG_NORMAL, FALSE},
    {"qauction", do_qauction, POS_STANDING, 0, LOG_NORMAL, FALSE},
    {"alist", build_arealist, POS_DEAD, 82, LOG_NORMAL, FALSE},
    {"alink", do_alink, POS_DEAD, 82, LOG_NORMAL, FALSE},
    {"board", do_board, POS_DEAD, 89, LOG_ALWAYS, FALSE},
    {"build", do_build, POS_STANDING, 2, LOG_ALWAYS, FALSE},
    {"mpcr", do_mpcr, POS_STANDING, 84, LOG_NORMAL, FALSE},
    {"mpstat", do_mpstat, POS_DEAD, 84, LOG_NORMAL, FALSE},
    {"halls", do_halls, POS_RESTING, 81, LOG_NORMAL, FALSE},

    {"lhunt", do_lhunt, POS_DEAD, 83, LOG_NORMAL, FALSE},
    {"sstat", do_sstat, POS_DEAD, 84, LOG_NORMAL, FALSE},
    {"cset", do_cset, POS_DEAD, 89, LOG_ALWAYS, FALSE},
    {"whoname", do_whoname, POS_DEAD, 87, LOG_ALWAYS, FALSE},

    {"clutchinfo", do_clutchinfo, POS_DEAD, 86, LOG_NORMAL, FALSE},
    {"advance", do_advance, POS_DEAD, 90, LOG_NORMAL, FALSE},
    {"setclass", do_setclass, POS_DEAD, 90, LOG_ALWAYS, FALSE},
    {"areasave", do_areasave, POS_DEAD, 90, LOG_ALWAYS, FALSE},
    {"doggy", do_dog, POS_DEAD, 87, LOG_ALWAYS, FALSE},
    {"dc", do_disconnect, POS_DEAD, 90, LOG_ALWAYS, FALSE},
    {"togbuild", do_togbuild, POS_DEAD, 90, LOG_ALWAYS, FALSE},
    {"togleader", do_togleader, POS_DEAD, 89, LOG_ALWAYS, FALSE},
    {"ihelps", do_ihelps, POS_DEAD, 82, LOG_NORMAL, FALSE},
    {"isnoop", do_isnoop, POS_DEAD, 90, LOG_ALWAYS, FALSE},
    {"trust", do_trust, POS_DEAD, 90, LOG_ALWAYS, FALSE},
    {"resetpassword", do_resetpassword, POS_DEAD, 90, LOG_ALWAYS, FALSE},
    {"iwhere", do_iwhere, POS_DEAD, 84, LOG_NORMAL, FALSE},
    {"fights", do_fights, POS_DEAD, 82, LOG_NORMAL, FALSE},
    {"allow", do_allow, POS_DEAD, 90, LOG_ALWAYS, FALSE},
    {"iban", do_ban, POS_DEAD, 90, LOG_ALWAYS, FALSE},
    {"deny", do_deny, POS_DEAD, 90, LOG_ALWAYS, FALSE},
    {"freeze", do_freeze, POS_DEAD, 89, LOG_ALWAYS, FALSE},
    {"reboo", do_reboo, POS_DEAD, 90, LOG_NORMAL, FALSE},
    {"reboot", do_reboot, POS_DEAD, 90, LOG_ALWAYS, FALSE},
    {"hotreboo", do_hotreboo, POS_DEAD, 89, LOG_ALWAYS, FALSE},
    {"hotreboot", do_hotreboot, POS_DEAD, 89, LOG_ALWAYS, FALSE},
    {"shutdow", do_shutdow, POS_DEAD, 90, LOG_NORMAL, FALSE},
    {"shutdown", do_shutdown, POS_DEAD, 90, LOG_ALWAYS, FALSE},
    {"users", do_users, POS_DEAD, 89, LOG_NORMAL, FALSE},
    {"wizify", do_wizify, POS_DEAD, 90, LOG_ALWAYS, FALSE},
    {"wizlock", do_wizlock, POS_DEAD, 90, LOG_ALWAYS, FALSE},
    {"imtlset", do_imtlset, POS_DEAD, 90, LOG_ALWAYS, FALSE},
    {"for", do_for, POS_DEAD, 88, LOG_ALWAYS, FALSE},
    {"force", do_force, POS_DEAD, 88, LOG_ALWAYS, FALSE},
    {"log", do_log, POS_DEAD, 87, LOG_ALWAYS, FALSE},
    {"maffect", do_maffect, POS_DEAD, 89, LOG_ALWAYS, FALSE},
    {"mload", do_mload, POS_DEAD, 86, LOG_ALWAYS, FALSE},
    {"monitor", do_monitor, POS_DEAD, 82, LOG_NORMAL, FALSE},
    {"mset", do_mset, POS_DEAD, 89, LOG_ALWAYS, FALSE},
    {"mudset", do_mudset, POS_DEAD, 90, LOG_ALWAYS, FALSE},
    {"noemote", do_noemote, POS_DEAD, 85, LOG_ALWAYS, FALSE},
    {"nopray", do_nopray, POS_DEAD, 85, LOG_ALWAYS, FALSE},
    {"nopk", do_nopk, POS_DEAD, 90, LOG_ALWAYS, FALSE},
    {"dblxp", do_dblxp, POS_DEAD, 90, LOG_ALWAYS, FALSE},
    {"notell", do_notell, POS_DEAD, 85, LOG_ALWAYS, FALSE},
    {"oload", do_oload, POS_DEAD, 86, LOG_ALWAYS, FALSE},
    {"oset", do_oset, POS_DEAD, 87, LOG_ALWAYS, FALSE},
    {"owhere", do_owhere, POS_DEAD, 87, LOG_NORMAL, FALSE},
    {"orare", do_orare, POS_DEAD, 90, LOG_NORMAL, FALSE},
    {"opotion", do_opotion, POS_DEAD, 90, LOG_NORMAL, FALSE},
    {"pardon", do_pardon, POS_DEAD, 87, LOG_ALWAYS, FALSE},
    {"peace", do_peace, POS_DEAD, 84, LOG_NORMAL, FALSE},
    {"purge", do_purge, POS_DEAD, 82, LOG_NORMAL, FALSE},
    {"iquest", do_quest, POS_DEAD, 88, LOG_NORMAL, FALSE},
    {"restore", do_restore, POS_DEAD, 86, LOG_ALWAYS, FALSE},
    {"rset", do_rset, POS_DEAD, 89, LOG_ALWAYS, FALSE},
    {"shedit", do_shedit, POS_DEAD, 88, LOG_NORMAL, FALSE},
    {"silence", do_silence, POS_DEAD, 86, LOG_ALWAYS, FALSE},
    {"sla", do_sla, POS_DEAD, 89, LOG_NORMAL, FALSE},
    {"slay", do_slay, POS_DEAD, 89, LOG_ALWAYS, FALSE},
    {"snoop", do_snoop, POS_DEAD, 89, LOG_ALWAYS, FALSE},
    {"sset", do_sset, POS_DEAD, 89, LOG_ALWAYS, FALSE},
    {"transfer", do_transfer, POS_DEAD, 84, LOG_ALWAYS, FALSE},

    {"at", do_at, POS_DEAD, 84, LOG_NORMAL, FALSE},
    {"bamfin", do_bamfin, POS_DEAD, 81, LOG_NORMAL, FALSE},
    {"bamfout", do_bamfout, POS_DEAD, 81, LOG_NORMAL, FALSE},
    {"echo", do_echo, POS_DEAD, 84, LOG_ALWAYS, FALSE},
    {"goto", do_goto, POS_DEAD, 81, LOG_NORMAL, FALSE},
    {"gossip", do_gossip, POS_RESTING, 0, LOG_NORMAL, FALSE},
    {"holylight", do_holylight, POS_DEAD, 81, LOG_NORMAL, FALSE},
    {"invis", do_invis, POS_DEAD, 81, LOG_NORMAL, FALSE},
    {"makeimm", do_immortal, POS_DEAD, 90, LOG_ALWAYS, FALSE},
    {"memory", do_memory, POS_DEAD, 89, LOG_NORMAL, FALSE},
    {"mfind", do_mfind, POS_DEAD, 86, LOG_NORMAL, FALSE},
    {"mfindlev", do_mfindlev, POS_DEAD, 84, LOG_NORMAL, FALSE},
    {"mstat", do_mstat, POS_DEAD, 84, LOG_NORMAL, FALSE},
    {"mwhere", do_mwhere, POS_DEAD, 87, LOG_NORMAL, FALSE},
    {"mlist", do_mlist, POS_DEAD, 83, LOG_NORMAL, FALSE},
    {"ofind", do_ofind, POS_DEAD, 86, LOG_NORMAL, FALSE},
    {"ostat", do_ostat, POS_DEAD, 86, LOG_NORMAL, FALSE},
    {"recho", do_recho, POS_DEAD, 83, LOG_ALWAYS, FALSE},
    {"roomlist", do_roomlist, POS_DEAD, 83, LOG_NORMAL, FALSE},
    {"mecho", do_mecho, POS_DEAD, 85, LOG_ALWAYS, FALSE},
    {"return", do_return, POS_DEAD, 83, LOG_NORMAL, FALSE},
    {"rstat", do_rstat, POS_DEAD, 84, LOG_NORMAL, FALSE},
    {"slookup", do_slookup, POS_DEAD, 83, LOG_NORMAL, FALSE},
    {"switch", do_switch, POS_DEAD, 83, LOG_ALWAYS, FALSE},

    {"immtalk", do_immtalk, POS_DEAD, 81, LOG_NORMAL, FALSE},
    {":", do_immtalk, POS_DEAD, 81, LOG_NORMAL, FALSE},
    {"reward", do_reward, POS_DEAD, 88, LOG_ALWAYS, FALSE},
    {"xpreward", do_xpreward, POS_DEAD, 89, LOG_ALWAYS, FALSE},
    {"xpcalc", do_xpcalc, POS_DEAD, 0, LOG_NORMAL, FALSE},
/*  {"setcouncil", do_togcouncil, POS_DEAD, 89, LOG_ALWAYS, FALSE}, */
    {"resetgain", do_gain_stat_reset, POS_DEAD, 90, LOG_ALWAYS, FALSE},
    {"exlist", do_exlist, POS_DEAD, 87, LOG_NORMAL, FALSE},
    {"vlist", do_vlist, POS_DEAD, 88, LOG_NORMAL, FALSE},
    {"olmsg", do_olmsg, POS_DEAD, 83, LOG_ALWAYS, FALSE},
    {"ofindlev", do_ofindlev, POS_DEAD, 83, LOG_NORMAL, FALSE},
    {"prename", do_rename, POS_DEAD, 90, LOG_ALWAYS, FALSE},
    {"pdelete", do_sdelete, POS_DEAD, 2, LOG_ALWAYS, FALSE},
    {"scheck", do_scheck, POS_DEAD, 90, LOG_NORMAL, FALSE},
    {"immbrand", do_immbrand, POS_DEAD, 90, LOG_NORMAL, FALSE},
    {"otype", do_otype, POS_DEAD, 87, LOG_NORMAL, FALSE},
    {"owear", do_owear, POS_DEAD, 89, LOG_NORMAL, FALSE},
    {"olist", do_olist, POS_DEAD, 83, LOG_NORMAL, FALSE},
    {"oreset", do_oreset, POS_DEAD, 90, LOG_NORMAL, FALSE},
    {"ocount", do_ocount, POS_DEAD, 90, LOG_NORMAL, FALSE},
    {"oflag", do_oflag, POS_DEAD, 90, LOG_NORMAL, FALSE},
    {"mflag", do_mflag, POS_DEAD, 90, LOG_NORMAL, FALSE},
    {"loadlink", do_loadlink, POS_DEAD, 90, LOG_ALWAYS, FALSE},
    {"xlook", do_xlook, POS_DEAD, 85, LOG_NORMAL, FALSE},
    {"xlookaff", do_xlookaff, POS_DEAD, 85, LOG_NORMAL, FALSE},
    {"ctalk", do_ctalk, POS_DEAD, 88, LOG_NORMAL, FALSE},
    {"atalk", do_atalk, POS_DEAD, 88, LOG_NORMAL, FALSE},
    {"nocmd", do_nocmd, POS_DEAD, 88, LOG_ALWAYS, FALSE},
    {"nospell", do_nospell, POS_DEAD, 88, LOG_ALWAYS, FALSE},
    {"deimm", do_deimm, POS_DEAD, 82, LOG_NORMAL, FALSE},
#if 0
    {"rinfo", do_rinfo, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"rsockets", do_rsockets, POS_DEAD, 90, LOG_NORMAL, FALSE},
    {"rconnect", do_rconnect, POS_DEAD, 90, LOG_ALWAYS, FALSE},
    {"rdisconnect", do_rdisconnect, POS_DEAD, 90, LOG_ALWAYS, FALSE},
    {"rignore", do_rignore, POS_DEAD, 90, LOG_ALWAYS, FALSE},
    {"mailq", do_mailqueue, POS_DEAD, 90, LOG_NORMAL, FALSE},
    {"rchanset", do_rchanset, POS_DEAD, 90, LOG_ALWAYS, FALSE},
    {"rping", do_rping, POS_DEAD, 90, LOG_NORMAL, FALSE},
    {"icommand", do_icommand, POS_DEAD, 90, LOG_NORMAL, FALSE},
    {"isetup", do_isetup, POS_DEAD, 90, LOG_ALWAYS, FALSE},
    {"ilist", do_ilist, POS_DEAD, 90, LOG_NORMAL, FALSE},
    {"ichannels", do_ichannels, POS_DEAD, 90, LOG_NORMAL, FALSE},

#endif
    /*
     * Werewolf commands
     */

/*
    {"howl", do_howl, POS_DEAD, WOLF_ONLY, LOG_NORMAL, FALSE},
    {"tribe", do_tribe, POS_DEAD, WOLF_ONLY, LOG_NORMAL, FALSE},
    {"rage", do_rage, POS_FIGHTING, WOLF_ONLY, LOG_NORMAL, FALSE},
    {"imprint", do_imprint, POS_STANDING, WOLF_ONLY, LOG_NORMAL, FALSE},
    {"scent", do_scent, POS_STANDING, WOLF_ONLY, LOG_NORMAL, FALSE},
    {"bite", do_bite, POS_FIGHTING, WOLF_ONLY, LOG_NORMAL, FALSE},
*/

    /*
     * MOBprogram commands.
     */

    {"mpasound", do_mpasound, POS_DEAD, 95, LOG_NORMAL, FALSE},
    {"mpjunk", do_mpjunk, POS_DEAD, 95, LOG_NORMAL, FALSE},
    {"mpecho", do_mpecho, POS_DEAD, 95, LOG_NORMAL, FALSE},
    {"mpechoat", do_mpechoat, POS_DEAD, 95, LOG_NORMAL, FALSE},
    {"mpechoaround", do_mpechoaround, POS_DEAD, 95, LOG_NORMAL, FALSE},
    {"mpkill", do_mpkill, POS_DEAD, 95, LOG_NORMAL, FALSE},
    {"mpmload", do_mpmload, POS_DEAD, 95, LOG_NORMAL, FALSE},
    {"mpoload", do_mpoload, POS_DEAD, 95, LOG_NORMAL, FALSE},
    {"mppurge", do_mppurge, POS_DEAD, 95, LOG_NORMAL, FALSE},
    {"mpgoto", do_mpgoto, POS_DEAD, 95, LOG_NORMAL, FALSE},
    {"mpat", do_mpat, POS_DEAD, 95, LOG_NORMAL, FALSE},
    {"mptransfer", do_mptransfer, POS_DEAD, 95, LOG_NORMAL, FALSE},
    {"mpforce", do_mpforce, POS_DEAD, 95, LOG_NORMAL, FALSE},
    {"mptongue", do_mptongue, POS_DEAD, 95, LOG_NORMAL, FALSE},

    {"rename",  do_objrename, POS_STANDING, 5, LOG_NORMAL, FALSE},
    {"irename", do_irename,   POS_STANDING, 85, LOG_NORMAL, FALSE},

    {"treasury", do_treasury, POS_STANDING, 20, LOG_NORMAL, FALSE},

    {"nosummon", do_nosummon, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"novisit", do_novisit, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"nogive", do_nogive, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"norescue", do_norescue, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"nobond", do_nobond, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"nofollow", do_nofollow, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"noobjspam", do_noobjspam, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"pacifist", do_pacifist, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"showblack", do_showblack, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"answering", do_answering, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"maskqp", do_maskqp, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"nodispel", do_nodispel, POS_FIGHTING, 0, LOG_NORMAL, FALSE},

    {"map", do_map,   POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"maps", do_maps, POS_DEAD, 0, LOG_NORMAL, FALSE},

    {"autostance", do_autostance, POS_DEAD, 0, LOG_NORMAL, FALSE},
    {"stealth",    do_stealth, POS_STANDING, 0, LOG_NORMAL, FALSE},
    {"idlecheck",  do_idlecheck, POS_DEAD, 90, LOG_NORMAL, FALSE},

    /*
     * End of list.
     */
    {"", 0, POS_DEAD, 0, LOG_NORMAL, FALSE}
};

/* Log all commands.. rewrite every 30 mins.. */
void
comlog(CHAR_DATA *ch, int cmd, char *args)
{
    static time_t       ltime;
    extern FILE *fpcomlog;

    if (nosave)
        return;

    if (!fpcomlog || ltime + (30 * 60) <= current_time) {
        if (fpcomlog)
            fclose(fpcomlog);
        if (!(fpcomlog = fopen("../log/comlog.txt", "w")))
            return;
        ltime = current_time;
    }
    FPRINTF(fpcomlog, "%.24s :: %12.12s (%5d): %s %s\n", ctime(&current_time),
        ch->name, (ch->in_room ? ch->in_room->vnum : -1), cmd_table[cmd].name, (cmd_table[cmd].log == LOG_NEVER ? "XXX" : args));
    fflush(fpcomlog);
}

/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */
void
interpret(CHAR_DATA *ch, char *argument)
{
    /* BUG with aliases: they can call themselves, which is
     * a Bad Thing.  When an alias calls interp, we'll add
     * a '~' char as the first char.  Checking for this will
     * tell us if we need to check aliases again. -S-
     */

    bool                alias_call;
    char                command[MAX_INPUT_LENGTH];
    char                logline[MAX_INPUT_LENGTH];
    int                 cmd = 0;
    int                 trust;
    bool                found;
    bool                proceed = FALSE;

    alias_call = FALSE;

    if (!IS_NPC(ch))
        remove_excess_at(argument);

    /* temp bug fix, sort of */
    if (!IS_NPC(ch) && !str_cmp(ch->name, "Isreply")) {
        send_to_char("@@a@@fYou're totally frozen!@@N\n\r", ch);
        return;
    }

    if (ch->position == POS_WRITING)
        /* if player is writing, pass argument straight to write_interpret */
    {
        write_interpret(ch, argument);
        return;
    }

    if (ch->position == POS_BUILDING) {
        if (argument[0] == ':')
            argument++;
        else {
            build_interpret(ch, argument);
            return;
        }
    }

    /*
     * Strip leading spaces.
     */
    while (isspace(*argument))
        argument++;
    if (argument[0] == '\0')
        return;

    if (argument[0] == '~') {
        argument++;
        alias_call = TRUE;
    }

    /*
     * No hiding.
     */
    REMOVE_BIT(ch->affected_by, AFF_HIDE);

    /*
     * Implement freeze command.
     */
    if (!IS_NPC(ch) && IS_SET(ch->act, PLR_FREEZE)) {
        send_to_char("@@a@@fYou're totally frozen!@@N\n\r", ch);
        return;
    }

    if (ch->stunTimer > 0) {
        send_to_char("@@NYou are too @@aSTUNNED@@N to act!\n\r", ch);
        return;
    }

    /*
     * Grab the command word.
     * Special parsing so ' can be a command,
     *   also no spaces needed after punctuation.
     */
    strcpy(logline, argument);
    if (!isalpha(argument[0]) && !isdigit(argument[0])) {
        command[0] = argument[0];
        command[1] = '\0';
        argument++;
        while (isspace(*argument))
            argument++;
    }
    else {
        argument = one_argument(argument, command);
    }

    /*
     * Look for command in command table.
     */
    found = FALSE;
    trust = get_trust(ch);

    /* A special case. nod is a social, but it's also the prefix for the
       nodispel command, so if the command matches nod exactly, bypass the
       command table. */
    if (!str_cmp("nod", command))
        proceed = FALSE;
    else
        proceed = TRUE;

    if (proceed) {
        for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++) {

            /* Stephen Mod:  if level == CLAN_ONLY then for clan member only.
             *  == BOSS_ONLY have to be leader.  
             */

            if (cmd_table[cmd].level == CLAN_ONLY && !IS_NPC(ch)
                && ch->pcdata->clan == 0)
                continue;

            if (cmd_table[cmd].level == BOSS_ONLY && !IS_NPC(ch)
                && !IS_SET(ch->pcdata->pflags, PFLAG_CLAN_BOSS))
                continue;

            /* old one :P 
               if ( command[0] == cmd_table[cmd].name[0]
               &&   !str_prefix( command, cmd_table[cmd].name )
               &&   ( cmd_table[cmd].level <= trust
               || MP_Commands( ch ) ) )
               {
             */
            /* new one, when needed to add support for imtlset */

            if (command[0] == cmd_table[cmd].name[0]
                && !str_prefix(command, cmd_table[cmd].name)
                && (cmd_table[cmd].level <= trust || MP_Commands(ch)
                    || authorized(ch, cmd_table[cmd].name)
                )
                ) {
                /* */
                found = TRUE;
                break;
            }

        }
    }

    if (found && !legal_cmd(ch, cmd))
        return;

    /*
     * Log and snoop.
     */
    if (found) {
        if (cmd_table[cmd].log == LOG_NEVER)
            strcpy(logline, "XXXXXXXX XXXXXXXX XXXXXXXX@@N");

        if ((!IS_NPC(ch) && IS_SET(ch->act, PLR_LOG))
            || fLogAll || cmd_table[cmd].log == LOG_ALWAYS) {
                bool sent_bad = FALSE;

            sprintf(log_buf, "Log %s [%d]: %s", ch->name, (ch->in_room != NULL) ? ch->in_room->vnum : -1, logline);

            log_string(log_buf);

            sprintf(log_buf, "Log %s [%d]: %s", ch->name, ((ch->in_room) ? ch->in_room->vnum : -1), logline);

            if (IS_SET(ch->act, PLR_LOG) && cmd_table[cmd].log != LOG_ALWAYS) {
                monitor_chan(log_buf, MONITOR_BAD);
                sent_bad = TRUE;
            }

            if (!sent_bad) {
                if (cmd_table[cmd].level > LEVEL_HERO)
                    monitor_chan(log_buf, MONITOR_GEN_IMM);
                else
                    monitor_chan(log_buf, MONITOR_GEN_MORT);
            }
        }
    }

    if (ch->desc != NULL && ch->desc->snoop_by != NULL) {    /* -S- Mod */
        char                snp[MAX_STRING_LENGTH];

        sprintf(snp, "[Snoop:%s] %s\n\r", ch->name, logline);
        write_to_buffer(ch->desc->snoop_by, snp, 0);
    }

    if (!found && !IS_NPC(ch) && (!alias_call)) {
        int                 cnt;
        char                foo[MAX_STRING_LENGTH];

        /* Check aliases -S- */

        for (cnt = 0; cnt < MAX_ALIASES; cnt++) {
            if (!str_cmp(ch->pcdata->alias_name[cnt], command)
                && str_cmp(ch->pcdata->alias_name[cnt], "<none>@@N")) {
                found = TRUE;

                if (strlen(ch->pcdata->alias[cnt]) + strlen(argument) > MAX_INPUT_LENGTH - 5) {
                    /* possible abuse for double max_input_length commands using aliases */
                    ch->desc->incomm[(MAX_INPUT_LENGTH - 5 - strlen(ch->pcdata->alias[cnt]))] = 0;
                }
                sprintf(foo, "~%s %s", ch->pcdata->alias[cnt], argument);
                interpret(ch, foo);
                return;
            }
        }
    }

    if (!found) {
        /*
         * Look for command in socials table.
         */
        if (!check_social(ch, command, argument))
            send_to_char("Huh?\n\r", ch);
        return;
    }

    /*
     * Character not in position for command?
     */
    if (ch->position < cmd_table[cmd].position) {
        switch (ch->position) {
            case POS_DEAD:
                send_to_char("Lie still; you are @@dDEAD@@N.\n\r", ch);
                break;

            case POS_MORTAL:
            case POS_INCAP:
                send_to_char("You are @@Rhurt@@N far too bad for that.\n\r", ch);
                break;

            case POS_STUNNED:
                send_to_char("You are too @@estunned@@N to do that.\n\r", ch);
                break;

            case POS_SLEEPING:
                send_to_char("Oh, go back to @@Wsleep!@@N\n\r", ch);
                break;

            case POS_RESTING:
                send_to_char("Naaaaaah... You feel too @@brelaxed@@N...\n\r", ch);
                break;

            case POS_FIGHTING:
                send_to_char("Not until you @@Rstop@@N fighting!\n\r", ch);
                break;

        }
        return;
    }

    /*
     * Dispatch the command.
     */
    if (!IS_NPC(ch)
        && ((ch->stance == STANCE_AMBUSH)
            || (ch->stance == STANCE_AC_BEST))
        && ((str_prefix(command, "kill"))
            && (str_prefix(command, "murder"))
            && (str_prefix(command, "backstab"))
            && (str_prefix(command, "bs"))
            && (str_prefix(command, "whisper"))
            && (str_prefix(command, "steal"))

            /* added commands while in shadows/ninja that wont bring you out */
            && (str_prefix(command, "who"))
            && (str_prefix(command, "score"))
            && (str_prefix(command, "look"))
            && (str_prefix(command, "emote"))
            && (str_prefix(command, ","))
            && (str_prefix(command, "save"))
            && (str_prefix(command, "north"))
            && (str_prefix(command, "east"))
            && (str_prefix(command, "south"))
            && (str_prefix(command, "west"))
            && (str_prefix(command, "up"))
            && (str_prefix(command, "down"))
            && (str_prefix(command, "enter"))
            && (str_prefix(command, "inventory"))
            && (str_prefix(command, "equipment"))
            && (str_prefix(command, "eqaffect"))
            && (str_prefix(command, "stealth"))
            && ((str_prefix(command, "follow") || (!str_prefix(command, "follow") && ch->pcdata->stealth < gsn_stealth_advanced)))
        ))
    {
        send_to_char("You step out of the shadows.\n\r", ch);
        ch->stance = STANCE_WARRIOR;
        ch->stance_ac_mod = 0;
        ch->stance_dr_mod = 0;
        ch->stance_hr_mod = 0;
        act("$n steps out of the Shadows!", ch, NULL, NULL, TO_ROOM);
    }
    comlog(ch, cmd, argument);
    (*cmd_table[cmd].do_fun) (ch, argument);

    tail_chain();
    return;
}

bool
check_social(CHAR_DATA *ch, char *command, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    CHAR_DATA          *victim;
    int                 cmd;
    bool                found;

    found = FALSE;
    for (cmd = 0; social_table[cmd].name[0] != '\0'; cmd++) {
        if (command[0] == social_table[cmd].name[0]
            && !str_prefix(command, social_table[cmd].name)) {
            found = TRUE;
            break;
        }
    }

    if (!found)
        return FALSE;

    if (!IS_NPC(ch) && IS_SET(ch->act, PLR_NO_EMOTE)) {
        send_to_char("You are anti-social!\n\r", ch);
        return TRUE;
    }

    switch (ch->position) {
        case POS_DEAD:
            send_to_char("Lie still; you are @@dDEAD@@N.\n\r", ch);
            return TRUE;

        case POS_INCAP:
        case POS_MORTAL:
            send_to_char("You are @@Rhurt@@N far too bad for that.\n\r", ch);
            return TRUE;

        case POS_STUNNED:
            send_to_char("You are too @@estunned@@N to do that.\n\r", ch);
            return TRUE;

        case POS_SLEEPING:
            /*
             * I just know this is the path to a 12" 'if' statement.  :(
             * But two players asked for it already!  -- Furey
             */
            if (!str_cmp(social_table[cmd].name, "snore"))
                break;
            send_to_char("In your @@Wdreams@@N, or what?\n\r", ch);
            return TRUE;

    }

    one_argument(argument, arg);
    victim = NULL;
    if (arg[0] == '\0') {
        act(social_table[cmd].others_no_arg, ch, NULL, victim, TO_ROOM);
        act(social_table[cmd].char_no_arg, ch, NULL, victim, TO_CHAR);
    }
    else if ((victim = get_char_room(ch, arg)) == NULL) {
        send_to_char("They aren't here.\n\r", ch);
    }
    else if (victim == ch) {
        act(social_table[cmd].others_auto, ch, NULL, victim, TO_ROOM);
        act(social_table[cmd].char_auto, ch, NULL, victim, TO_CHAR);
    }
    else {
        act(social_table[cmd].others_found, ch, NULL, victim, TO_NOTVICT);
        act(social_table[cmd].char_found, ch, NULL, victim, TO_CHAR);
        act(social_table[cmd].vict_found, ch, NULL, victim, TO_VICT);

        if (!IS_NPC(ch) && IS_NPC(victim)
            && !IS_AFFECTED(victim, AFF_CHARM)
            && IS_AWAKE(victim)) {
            switch (number_bits(4)) {
                case 0:
                case 1:
                case 2:
                case 3:
                case 4:
                    act(social_table[cmd].others_found, victim, NULL, ch, TO_NOTVICT);
                    act(social_table[cmd].char_found, victim, NULL, ch, TO_CHAR);
                    act(social_table[cmd].vict_found, victim, NULL, ch, TO_VICT);
                    break;

                case 5:
                case 6:
                case 7:
                case 8:
                case 9:
                case 10:
                case 11:
                case 12:
                    act("$n slaps $N.", victim, NULL, ch, TO_NOTVICT);
                    act("You slap $N.", victim, NULL, ch, TO_CHAR);
                    act("$n slaps you.", victim, NULL, ch, TO_VICT);
                    break;
            }
        }
    }

    return TRUE;
}

/*
 * Return true if an argument is completely numeric.
 */
bool
is_number(char *arg)
{
    if (*arg == '\0')
        return FALSE;

    if (*arg == '+' || *arg == '-')
        arg++;

    for (; *arg != '\0'; arg++) {
        if (!isdigit(*arg))
            return FALSE;
    }

    return TRUE;
}

/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
int
number_argument(char *argument, char *arg)
{
    char               *pdot;
    int                 number;

    for (pdot = argument; *pdot != '\0'; pdot++) {
        if (*pdot == '.') {
            *pdot = '\0';
            number = atoi(argument);
            *pdot = '.';
            strcpy(arg, pdot + 1);
            return number;
        }
    }

    strcpy(arg, argument);
    return 1;
}

/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */
char               *
one_argument(char *argument, char *arg_first)
{
    char                cEnd;

    while (isspace(*argument))
        argument++;

    cEnd = ' ';
    if (*argument == '\'' || *argument == '"')
        cEnd = *argument++;

    while (*argument != '\0') {
        if (*argument == cEnd) {
            argument++;
            break;
        }
        *arg_first = LOWER(*argument);
        arg_first++;
        argument++;
    }
    *arg_first = '\0';

    while (isspace(*argument))
        argument++;

    return argument;
}

char               *
one_argument_nolower(char *argument, char *arg_first)
{
    char                cEnd;

    while (isspace(*argument))
        argument++;

    cEnd = ' ';
    if (*argument == '\'' || *argument == '"')
        cEnd = *argument++;

    while (*argument != '\0') {
        if (*argument == cEnd) {
            argument++;
            break;
        }
        *arg_first++ = *argument++;
    }
    *arg_first = '\0';

    while (isspace(*argument))
        argument++;

    return argument;
}

bool
IS_SWITCHED(CHAR_DATA *ch)
{
    if (!IS_NPC(ch))
        return FALSE;

    if (ch->desc == NULL)
        return FALSE;

    return TRUE;

}

bool
MP_Commands(CHAR_DATA *ch)
{                                /* Can MOBProged mobs
                                   use mpcommands? TRUE if yes.
                                   - Kahn */
    if (IS_SWITCHED(ch))
        return FALSE;

    if (IS_NPC(ch)
        && ch->pIndexData->progtypes && !IS_AFFECTED(ch, AFF_CHARM))
        return TRUE;

    return FALSE;
}

#if 0
char               *
remove_excess_at(char *test)
{
    static char        *text;
    char                dest[MAX_INPUT_LENGTH];
    char                c;
    char                d[2];
    int                 a = -1;

    d[1] = '\0';
    strcpy(dest, "");
    text = test;

    if (strlen(text) > 2 && (strlen(text) < MAX_INPUT_LENGTH)) {
        while ((c = *(text++)) != 0) {
            d[0] = c;
            if (c != '@') {
                if (a > -1) {
                    strcat(dest, "@@");
                    a = -1;
                }
                strcat(dest, d);
            }
            else {
                if ((c = *(text++)) == '@') {
                    a++;
                }
                else if (c == 0) {
                    strcat(dest, d);
                    break;
                }
                else if (a < 0) {
                    strcat(dest, d);
                    d[0] = c;
                    strcat(dest, d);
                }
                else {
                    strcat(dest, "@@");
                    d[0] = c;
                    strcat(dest, d);
                    a = -1;
                }
            }
        }
        dest[strlen(dest)] = '\0';
        text = dest;
    }
    return text;
}
#endif

/****
 * This function converts three or more @'s into @@.
 * If there's any @'s at the end (more than 1), it also removes these.
 *                                                      --- Erigol
 ****/

void
remove_excess_at(char *argument)
{
    char               *ptr = argument;
    char               *dest = argument;
    int                 count = 0;

    for (; *ptr; ptr++) {
        if (*ptr == '@') {
            count++;
            continue;
        }

        if (count == 1) {
            *dest++ = '@';
            count = 0;
        }
        else if (count >= 2) {
            *dest++ = '@';
            *dest++ = '@';
            count = 0;
        }

        *dest++ = *ptr;
    }

/*  don't allow @ at the end of a line, screws up bleeding, ie. %s@@N where
    %s ends in @ means @@@N which is wrong. if someone REALLY needs to do @
    at the end of a line, they can use @@-
    if (count == 1)
        *dest++ = '@';
*/

    *dest = '\0';
}
