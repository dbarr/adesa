
/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvements copyright (C) 1992, 1993 by Michael         *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefiting.  We hope that you share your changes too.  What goes       *
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
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "merc.h"
#include "tables.h"
#include "auction.h"

IDSTRING(rcsid, "$Id: enchant.c,v 1.11 2004/05/21 08:24:16 dave Exp $");

void
do_enchant(CHAR_DATA *ch, char *argument)
{
    char                arg1[MSL];
    char                arg2[MSL];
    char                debug[MSL];
    char                msg_buf[MSL];
    char                cat_buf[MSL];
    char                enchant_buf[MSL];
    char                enchant_catbuf[MSL];

    OBJ_DATA           *matrix;
    OBJ_DATA           *unique = NULL;
    OBJ_DATA           *this_obj;

    AFFECT_DATA        *this_aff;
    int                 cur_str = 0, cur_int = 0, cur_dex = 0, cur_wis = 0, cur_con = 0;
    int                 mod_str = 0, mod_int = 0, mod_dex = 0, mod_wis = 0, mod_con = 0;
    int                 cur_mana = 0, cur_hitroll = 0, cur_move = 0, cur_armor = 0, cur_save = 0, cur_hit = 0, cur_damroll = 0;
    int                 mod_mana = 0, mod_hitroll = 0, mod_move = 0, mod_armor = 0, mod_save = 0, mod_hit = 0, mod_damroll = 0;
    sh_int              mod_weight = 0;
    bool                good_item = FALSE, legal_enchant = TRUE, bad_enchant_mix = FALSE;
    sh_int              qp_cost = 50;
    int                 mod = 0;
    sh_int              min_level = 10;
    int                 new_extras = 0, new_apply = ITEM_APPLY_NONE;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (IS_NPC(ch)) {
        send_to_char("Not for Npcs.\n\r", ch);
        return;
    }

    if (arg1[0] == '\0' || (!is_name(arg1, "show buy"))) {
        send_to_char("Syntax for enchanting objects:\n\r", ch);
        send_to_char("  First, you must place at least one wearable item, and 1 or more enchantments\n\r", ch);
        send_to_char("  into a spell matrix container, which you must hold.  Then, you must type\n\r", ch);
        send_to_char("  enchant show to see the results of applying those enchantments to the item.\n\r", ch);
        send_to_char("  You may not enchant an item so that its power would be greater than you can\n\r", ch);
        send_to_char("  control at your current level.  Finally, once you are satisfied with the new\n\r", ch);
        send_to_char("  item's powers, type enchant buy.\n\r", ch);
        return;
    }

    /* first, we check for good enchantments and an item template in the matrix */

    if ((matrix = get_eq_char(ch, WEAR_HOLD)) == NULL) {
        send_to_char("@@NYou must be holding a @@lspell matrix@@N for this ability!\n\r", ch);
        return;
    }

    if (matrix->item_type != ITEM_SPELL_MATRIX) {
        send_to_char("@@NThat is not a @@lspell matrix@@N you are holding, you fool!!\n\r", ch);
        return;
    }

    /* now, we tear apart the enchantments, and put them in add_mod vars */

    for (this_obj = matrix->first_in_carry_list; this_obj != NULL; this_obj = this_obj->next_in_carry_list) {
        if ((this_obj->item_type == ITEM_WEAPON)
            || (this_obj->item_type == ITEM_ARMOR)
            || (this_obj->item_type == ITEM_LIGHT)) {
            good_item = TRUE;
            unique = this_obj;
            continue;
        }
        if (this_obj->item_type != ITEM_ENCHANTMENT) {
            bad_enchant_mix = TRUE;
            if (!str_cmp(arg2, "debug")) {
                sprintf(debug, "Non-enchantment item in matrix..%s\n\r", this_obj->name);
                send_to_char(debug, ch);
            }
            break;
        }
        if (this_obj->value[0] == ENCHANT_OBJFUNS) {
            bad_enchant_mix = TRUE;
        }
        else if (this_obj->value[0] == ENCHANT_APPLY_FLAGS) {
            if ((this_obj->value[3] > 0)
                && (!IS_SET(new_apply, (1 << (this_obj->value[3] - 1)))))
                SET_BIT(new_apply, (1 << (this_obj->value[3] - 1)));
        }
        else if (this_obj->value[0] == ENCHANT_EXTRA_FLAGS) {
            if ((this_obj->value[3] > 0)
                && (!IS_SET(new_extras, (1 << (this_obj->value[3] - 1)))))
                SET_BIT(new_extras, (1 << (this_obj->value[3] - 1)));
        }
        else {

            mod = this_obj->value[1];
            switch (this_obj->value[0]) {
                default:
                    if (!str_cmp(arg2, "debug")) {
                        sprintf(debug, "Bad location in v0 of enchantment %s\n\r", this_obj->name);
                        send_to_char(debug, ch);
                    }
                    break;

                case APPLY_NONE:
                    break;
                case APPLY_STR:
                    mod_str += mod;
                    break;
                case APPLY_DEX:
                    mod_dex += mod;
                    break;
                case APPLY_INT:
                    mod_int += mod;
                    break;
                case APPLY_WIS:
                    mod_wis += mod;
                    break;
                case APPLY_CON:
                    mod_con += mod;
                    break;
                case APPLY_SEX:
                    break;
                case APPLY_CLASS:
                    break;
                case APPLY_LEVEL:
                    mod_weight -= 5;
                    break;
                case APPLY_AGE:
                    break;
                case APPLY_HEIGHT:
                    break;
                case APPLY_WEIGHT:
                    mod_weight += mod;
                    break;
                case APPLY_MANA:
                    mod_mana += mod;
                    break;
                case APPLY_HIT:
                    mod_hit += mod;
                    break;
                case APPLY_MOVE:
                    mod_move += mod;
                    break;
                case APPLY_GOLD:
                    break;
                case APPLY_EXP:
                    break;
                case APPLY_AC:
                    mod_armor += mod;
                    break;
                case APPLY_HITROLL:
                    mod_hitroll += mod;
                    break;
                case APPLY_DAMROLL:
                    mod_damroll += mod;
                    break;
                case APPLY_SAVING_PARA:
                    mod_save += mod;
                    break;
                case APPLY_SAVING_ROD:
                    mod_save += mod;
                    break;
                case APPLY_SAVING_PETRI:
                    mod_save += mod;
                    break;
                case APPLY_SAVING_BREATH:
                    mod_save += mod;
                    break;
                case APPLY_SAVING_SPELL:
                    mod_save += mod;
                    break;
            }

        }
        qp_cost += this_obj->value[2];
    }

    /* now, we have the enchantment totals, and qp costs, need to get the current non-spell affs */
    /* now, we tear apart the current item's affs and store them in cur_mod vars */
    if (unique == NULL) {
        if (!str_cmp(arg2, "debug")) {
            sprintf(debug, "%s", "No item within matrix.\n\r");
            send_to_char(debug, ch);
        }
        return;
    }

    if (((get_remort_level(ch) < unique->level)
            && (IS_OBJ_STAT(unique, ITEM_REMORT)))
        || (get_pseudo_level(ch) < unique->level)) {
        send_to_char("You can't use this item in the first place..enchanting it is NOT going to help!\n\r", ch);
        return;
    }

    if (ch->adept_level < unique->level && IS_OBJ_STAT(unique, ITEM_ADEPT)) {
        send_to_char("You can't use this item in the first place..enchanting it is NOT going to help!\n\r", ch);
        return;
    }

    if (IS_OBJ_STAT(unique, ITEM_UNIQUE))
        qp_cost -= 50;

    for (this_aff = unique->first_apply; this_aff != NULL; this_aff = this_aff->next) {
        if (this_aff->type != -1)
            continue;

        mod = this_aff->modifier;
        switch (this_aff->location) {
            default:
                if (!str_cmp(arg2, "debug")) {
                    sprintf(debug, "Unknown aff on %s\n\r", unique->name);
                    send_to_char(debug, ch);
                }
                break;

            case APPLY_NONE:
                break;
            case APPLY_STR:
                cur_str += mod;
                break;
            case APPLY_DEX:
                cur_dex += mod;
                break;
            case APPLY_INT:
                cur_int += mod;
                break;
            case APPLY_WIS:
                cur_wis += mod;
                break;
            case APPLY_CON:
                cur_con += mod;
                break;
            case APPLY_SEX:
                break;
            case APPLY_CLASS:
                break;
            case APPLY_LEVEL:
                break;
            case APPLY_AGE:
                break;
            case APPLY_HEIGHT:
                break;
            case APPLY_WEIGHT:
                break;
            case APPLY_MANA:
                cur_mana += mod;
                break;
            case APPLY_HIT:
                cur_hit += mod;
                break;
            case APPLY_MOVE:
                cur_move += mod;
                break;
            case APPLY_GOLD:
                break;
            case APPLY_EXP:
                break;
            case APPLY_AC:
                cur_armor += mod;
                break;
            case APPLY_HITROLL:
                cur_hitroll += mod;
                break;
            case APPLY_DAMROLL:
                cur_damroll += mod;
                break;
            case APPLY_SAVING_PARA:
                cur_save += mod;
                break;
            case APPLY_SAVING_ROD:
                cur_save += mod;
                break;
            case APPLY_SAVING_PETRI:
                cur_save += mod;
                break;
            case APPLY_SAVING_BREATH:
                cur_save += mod;
                break;
            case APPLY_SAVING_SPELL:
                cur_save += mod;
                break;
        }
    }

    if (!str_cmp(arg1, "show")) {
        sprintf(msg_buf, "Your enchantment will add the following to %s\n\r", unique->short_descr);

        if (new_extras != 0) {
            sprintf(cat_buf, "Extra Flags : %s\n\r", extra_bit_name(new_extras));
            safe_strcat(MSL, msg_buf, cat_buf);
        }
        if (new_apply != ITEM_APPLY_NONE) {
            sprintf(cat_buf, "Magic Flags : %s\n\r", bit_table_lookup(tab_item_apply, (new_apply)));
            safe_strcat(MSL, msg_buf, cat_buf);
        }

        if (mod_str != 0) {
            sprintf(cat_buf, "Str: %d  Total: %d  ", mod_str, mod_str + cur_str);
            safe_strcat(MSL, msg_buf, cat_buf);
            if (mod_str + cur_str > 10) {
                legal_enchant = FALSE;
                sprintf(cat_buf, "%s", "@@RILLEGAL@@N\n\r");
            }
            else {
                sprintf(cat_buf, "%s", "\n\r");
            }
            safe_strcat(MSL, msg_buf, cat_buf);
            min_level = UMAX((mod_str + cur_str) * 10, min_level);

        }
        if (mod_int != 0) {
            sprintf(cat_buf, "Int: %d  Total: %d  ", mod_int, mod_int + cur_int);
            safe_strcat(MSL, msg_buf, cat_buf);
            if (mod_int + cur_int > 10) {
                legal_enchant = FALSE;
                sprintf(cat_buf, "%s", "@@RILLEGAL@@N\n\r");
            }
            else {
                sprintf(cat_buf, "%s", "\n\r");
            }
            safe_strcat(MSL, msg_buf, cat_buf);
            min_level = UMAX((mod_int + cur_int) * 10, min_level);
        }
        if (mod_wis != 0) {
            sprintf(cat_buf, "Wis: %d  Total: %d  ", mod_wis, mod_wis + cur_wis);
            safe_strcat(MSL, msg_buf, cat_buf);
            if (mod_wis + cur_wis > 10) {
                legal_enchant = FALSE;
                sprintf(cat_buf, "%s", "@@RILLEGAL@@N\n\r");
            }
            else {
                sprintf(cat_buf, "%s", "\n\r");
            }
            safe_strcat(MSL, msg_buf, cat_buf);
            min_level = UMAX((mod_wis + cur_wis) * 10, min_level);

        }
        if (mod_dex != 0) {
            sprintf(cat_buf, "Dex: %d  Total: %d\n\r  ", mod_dex, mod_dex + cur_dex);
            safe_strcat(MSL, msg_buf, cat_buf);
            if (mod_dex + cur_dex > 10) {
                legal_enchant = FALSE;
                sprintf(cat_buf, "%s", "@@RILLEGAL@@N\n\r");
            }
            else {
                sprintf(cat_buf, "%s", "\n\r");
            }
            safe_strcat(MSL, msg_buf, cat_buf);
            min_level = UMAX((mod_dex + cur_dex) * 10, min_level);
        }
        if (mod_con != 0) {
            sprintf(cat_buf, "Con: %d  Total: %d  ", mod_con, mod_con + cur_con);
            safe_strcat(MSL, msg_buf, cat_buf);
            if (mod_con + cur_con > 10) {
                legal_enchant = FALSE;
                sprintf(cat_buf, "%s", "@@RILLEGAL@@N\n\r");
            }
            else {
                sprintf(cat_buf, "%s", "\n\r");
            }
            safe_strcat(MSL, msg_buf, cat_buf);
            min_level = UMAX((mod_con + cur_con) * 10, min_level);
        }
        if (mod_hit != 0) {
            sprintf(cat_buf, "Hit Points: %d  Total: %d  ", mod_hit, mod_hit + cur_hit);
            safe_strcat(MSL, msg_buf, cat_buf);
            if (mod_hit + cur_hit > 480) {
                legal_enchant = FALSE;
                sprintf(cat_buf, "%s", "@@RILLEGAL@@N\n\r");
            }
            else {
                sprintf(cat_buf, "%s", "\n\r");
            }
            safe_strcat(MSL, msg_buf, cat_buf);
            min_level = UMAX((mod_hit + cur_hit) / 4, min_level);
        }
        if (mod_mana != 0) {
            sprintf(cat_buf, "Mana: %d  Total: %d  ", mod_mana, mod_mana + cur_mana);
            safe_strcat(MSL, msg_buf, cat_buf);
            if (mod_mana + cur_mana > 480) {
                legal_enchant = FALSE;
                sprintf(cat_buf, "%s", "@@RILLEGAL@@N\n\r");
            }
            else {
                sprintf(cat_buf, "%s", "\n\r");
            }
            safe_strcat(MSL, msg_buf, cat_buf);
            min_level = UMAX((mod_mana + cur_mana) / 4, min_level);
        }
        if (mod_move != 0) {
            sprintf(cat_buf, "Move: %d  Total: %d  ", mod_move, mod_move + cur_move);
            safe_strcat(MSL, msg_buf, cat_buf);
            if (mod_move + cur_move > 480) {
                legal_enchant = FALSE;
                sprintf(cat_buf, "%s", "@@RILLEGAL@@N\n\r");
            }
            else {
                sprintf(cat_buf, "%s", "\n\r");
            }
            safe_strcat(MSL, msg_buf, cat_buf);
            min_level = UMAX((mod_move + cur_move) / 4, min_level);
        }

        if (mod_armor != 0) {
            sprintf(cat_buf, "AC: %d  Total: %d  ", mod_armor, mod_armor + cur_armor);
            safe_strcat(MSL, msg_buf, cat_buf);
            if (mod_armor + cur_armor < -480) {
                legal_enchant = FALSE;
                sprintf(cat_buf, "%s", "@@RILLEGAL@@N\n\r");
            }
            else {
                sprintf(cat_buf, "%s", "\n\r");
            }
            safe_strcat(MSL, msg_buf, cat_buf);
            min_level = UMAX((mod_armor + cur_armor) / -4, min_level);

        }
        if (mod_hitroll != 0) {
            sprintf(cat_buf, "Hitroll: %d  Total: %d  ", mod_hitroll, mod_hitroll + cur_hitroll);
            safe_strcat(MSL, msg_buf, cat_buf);
            if (mod_hitroll + cur_hitroll > 50) {
                legal_enchant = FALSE;
                sprintf(cat_buf, "%s", "@@RILLEGAL@@N\n\r");
            }
            else {
                sprintf(cat_buf, "%s", "\n\r");
            }
            safe_strcat(MSL, msg_buf, cat_buf);
            min_level = UMAX((mod_hitroll + cur_hitroll) * 2 + 20, min_level);
        }
        if (mod_damroll != 0) {
            sprintf(cat_buf, "Damroll: %d  Total: %d  ", mod_damroll, mod_damroll + cur_damroll);
            safe_strcat(MSL, msg_buf, cat_buf);
            if (mod_damroll + cur_damroll > 50) {
                legal_enchant = FALSE;
                sprintf(cat_buf, "%s", "@@RILLEGAL@@N\n\r");
            }
            else {
                sprintf(cat_buf, "%s", "\n\r");
            }
            safe_strcat(MSL, msg_buf, cat_buf);
            min_level = UMAX((mod_damroll + cur_damroll) * 2 + 20, min_level);
        }
        if (mod_save != 0) {
            sprintf(cat_buf, "Save vs Spell: %d  Total: %d  ", mod_save, mod_save + cur_save);
            safe_strcat(MSL, msg_buf, cat_buf);
            if (mod_save + cur_save < -36) {
                legal_enchant = FALSE;
                sprintf(cat_buf, "%s", "@@RILLEGAL@@N\n\r");
            }
            else {
                sprintf(cat_buf, "%s", "\n\r");
            }
            safe_strcat(MSL, msg_buf, cat_buf);
            min_level = UMAX((mod_save + cur_save) / -3 * 10, min_level);

        }
        if (mod_weight != 0) {
            sprintf(cat_buf, "New item weight: %d  ", mod_weight + unique->weight);
            safe_strcat(MSL, msg_buf, cat_buf);
            if (mod_weight + unique->weight < 1) {
                legal_enchant = FALSE;
                sprintf(cat_buf, "%s", "@@RILLEGAL@@N\n\r");
            }
            else {
                sprintf(cat_buf, "%s", "\n\r");
            }
            safe_strcat(MSL, msg_buf, cat_buf);

        }
        if (IS_SET(unique->extra_flags, ITEM_CLAN_EQ)) {
            sprintf(cat_buf, "%s is clan equipment, and may not be enchanted.\n\r", unique->short_descr);
            safe_strcat(MSL, msg_buf, cat_buf);
            legal_enchant = FALSE;
        }

        if (legal_enchant) {
            sprintf(cat_buf, "Total Cost: %d @@aqp@@N\n\r", qp_cost);
        }
        else {
            sprintf(cat_buf, "%s", "This is not a legal enchantment.  Remove some enchantments, and try again.\n\r");
        }
        safe_strcat(MSL, msg_buf, cat_buf);
        if (legal_enchant) {
            if (available_qps(ch) >= qp_cost)
            {
                sprintf(cat_buf, "%s", "You may type enchant buy to purchase this upgrade.\n\r");
            }
            else {
                sprintf(cat_buf, "%s", "You do not have enough quest points for this upgrade.\n\r");
            }
            safe_strcat(MSL, msg_buf, cat_buf);
            if (min_level > unique->level && !IS_SET(unique->extra_flags, ITEM_ADEPT)) {
                sprintf(cat_buf,
                    "@@eWARNING: @@WEnchanting %s with these affects will make the item level %d.\n\r",
                    unique->short_descr, min_level);
                safe_strcat(MSL, msg_buf, cat_buf);
            }
        }

        send_to_char(msg_buf, ch);
        return;
    }

    if (!str_cmp(arg1, "buy")) {
        AFFECT_DATA        *this_aff_next;
        AFFECT_DATA        *new_aff;
        OBJ_DATA           *this_obj_next;

        if (available_qps(ch) < qp_cost) {
            send_to_char("You don't have enough quest points for this enchantment.\n\r", ch);
            return;
        }
        if (qp_cost == 0) {
            send_to_char("Now that would be worthwile. Let's reconsider, and try adding some enchantments!\n\r", ch);
            return;
        }
        if ((mod_str != 0) && (mod_str + cur_str > 10)) {
            legal_enchant = FALSE;
        }
        if ((mod_int != 0) && (mod_int + cur_int > 10)) {
            legal_enchant = FALSE;
        }
        if ((mod_wis != 0) && (mod_wis + cur_wis > 10)) {
            legal_enchant = FALSE;
        }
        if ((mod_dex != 0) && (mod_dex + cur_dex > 10)) {
            legal_enchant = FALSE;
        }
        if ((mod_con != 0) && (mod_con + cur_con > 10)) {
            legal_enchant = FALSE;
        }
        if ((mod_hit != 0) && (mod_hit + cur_hit > 480)) {
            legal_enchant = FALSE;
        }
        if ((mod_mana != 0) && (mod_mana + cur_mana > 480)) {
            legal_enchant = FALSE;
        }
        if ((mod_move != 0) && (mod_move + cur_move > 480)) {
            legal_enchant = FALSE;
        }
        if ((mod_armor != 0) && (mod_armor + cur_armor < -480)) {
            legal_enchant = FALSE;
        }
        if ((mod_hitroll != 0) && (mod_hitroll + cur_hitroll > 50)) {
            legal_enchant = FALSE;
        }
        if ((mod_damroll != 0) && (mod_damroll + cur_damroll > 50)) {
            legal_enchant = FALSE;
        }
        if ((mod_save != 0) && (mod_save + cur_save < -36)) {
            legal_enchant = FALSE;
        }
        if ((mod_weight != 0) && (mod_weight + unique->weight < 1)) {
            legal_enchant = FALSE;
        }
        if (IS_SET(unique->extra_flags, ITEM_CLAN_EQ)) {
            legal_enchant = FALSE;
        }

        if (legal_enchant == FALSE) {
            do_enchant(ch, "show");
            return;
        }
        for (this_aff = unique->first_apply; this_aff != NULL; this_aff = this_aff_next) {
            this_aff_next = this_aff->next;
            if (this_aff->type != -1)
                continue;
            UNLINK(this_aff, unique->first_apply, unique->last_apply, next, prev);
            PUT_FREE(this_aff, affect_free);
        }
        if (mod_str + cur_str != 0) {
            GET_FREE(new_aff, affect_free);
            new_aff->type = -1;
            new_aff->location = APPLY_STR;
            new_aff->modifier = mod_str + cur_str;
            new_aff->duration = -1;
            LINK(new_aff, unique->first_apply, unique->last_apply, next, prev);
            if (mod_str != 0)
                min_level = UMAX((mod_str + cur_str) * 10, min_level);
        }
        if (mod_int + cur_int != 0) {
            GET_FREE(new_aff, affect_free);
            new_aff->type = -1;
            new_aff->location = APPLY_INT;
            new_aff->modifier = mod_int + cur_int;
            new_aff->duration = -1;
            LINK(new_aff, unique->first_apply, unique->last_apply, next, prev);
            if (mod_int != 0)
                min_level = UMAX((mod_int + cur_int) * 10, min_level);

        }
        if (mod_wis + cur_wis != 0) {
            GET_FREE(new_aff, affect_free);
            new_aff->type = -1;
            new_aff->location = APPLY_WIS;
            new_aff->modifier = mod_wis + cur_wis;
            new_aff->duration = -1;
            LINK(new_aff, unique->first_apply, unique->last_apply, next, prev);
            if (mod_wis != 0)
                min_level = UMAX((mod_wis + cur_wis) * 10, min_level);

        }
        if (mod_dex + cur_dex != 0) {
            GET_FREE(new_aff, affect_free);
            new_aff->type = -1;
            new_aff->location = APPLY_DEX;
            new_aff->modifier = mod_dex + cur_dex;
            new_aff->duration = -1;
            LINK(new_aff, unique->first_apply, unique->last_apply, next, prev);
            if (mod_dex != 0)
                min_level = UMAX((mod_dex + cur_dex) * 10, min_level);

        }
        if (mod_con + cur_con != 0) {
            GET_FREE(new_aff, affect_free);
            new_aff->type = -1;
            new_aff->location = APPLY_CON;
            new_aff->modifier = mod_con + cur_con;
            new_aff->duration = -1;
            LINK(new_aff, unique->first_apply, unique->last_apply, next, prev);
            if (mod_con != 0)
                min_level = UMAX((mod_con + cur_con) * 10, min_level);

        }
        if (mod_hit + cur_hit != 0) {
            GET_FREE(new_aff, affect_free);
            new_aff->type = -1;
            new_aff->location = APPLY_HIT;
            new_aff->modifier = mod_hit + cur_hit;
            new_aff->duration = -1;
            LINK(new_aff, unique->first_apply, unique->last_apply, next, prev);
            if (mod_hit != 0)
                min_level = UMAX((mod_hit + cur_hit) / 4, min_level);

        }
        if (mod_mana + cur_mana != 0) {
            GET_FREE(new_aff, affect_free);
            new_aff->type = -1;
            new_aff->location = APPLY_MANA;
            new_aff->modifier = mod_mana + cur_mana;
            new_aff->duration = -1;
            LINK(new_aff, unique->first_apply, unique->last_apply, next, prev);
            if (mod_mana != 0)
                min_level = UMAX((mod_mana + cur_mana) / 4, min_level);

        }
        if (mod_move + cur_move != 0) {
            GET_FREE(new_aff, affect_free);
            new_aff->type = -1;
            new_aff->location = APPLY_MOVE;
            new_aff->modifier = mod_move + cur_move;
            new_aff->duration = -1;
            LINK(new_aff, unique->first_apply, unique->last_apply, next, prev);
            if (mod_move != 0)
                min_level = UMAX((mod_move + cur_move) / 4, min_level);

        }
        if (mod_hitroll + cur_hitroll != 0) {
            GET_FREE(new_aff, affect_free);
            new_aff->type = -1;
            new_aff->location = APPLY_HITROLL;
            new_aff->modifier = mod_hitroll + cur_hitroll;
            new_aff->duration = -1;
            LINK(new_aff, unique->first_apply, unique->last_apply, next, prev);
            if (mod_hitroll != 0)
                min_level = UMAX((mod_hitroll + cur_hitroll) * 2 + 20, min_level);

        }
        if (mod_damroll + cur_damroll != 0) {
            GET_FREE(new_aff, affect_free);
            new_aff->type = -1;
            new_aff->location = APPLY_DAMROLL;
            new_aff->modifier = mod_damroll + cur_damroll;
            new_aff->duration = -1;
            LINK(new_aff, unique->first_apply, unique->last_apply, next, prev);
            if (mod_damroll != 0)
                min_level = UMAX((mod_damroll + cur_damroll) * 2 + 20, min_level);

        }
        if (mod_save + cur_save != 0) {
            GET_FREE(new_aff, affect_free);
            new_aff->type = -1;
            new_aff->location = APPLY_SAVING_SPELL;
            new_aff->modifier = mod_save + cur_save;
            new_aff->duration = -1;
            LINK(new_aff, unique->first_apply, unique->last_apply, next, prev);
            if (mod_save != 0)
                min_level = UMAX((mod_save + cur_save) / -3 * 10, min_level);

        }
        if (mod_armor + cur_armor != 0) {
            GET_FREE(new_aff, affect_free);
            new_aff->type = -1;
            new_aff->location = APPLY_AC;
            new_aff->modifier = mod_armor + cur_armor;
            new_aff->duration = -1;
            LINK(new_aff, unique->first_apply, unique->last_apply, next, prev);
            if (mod_armor != 0)
                min_level = UMAX((mod_armor + cur_armor) / -4, min_level);

        }
        if (!IS_SET(unique->extra_flags, ITEM_UNIQUE))
            SET_BIT(unique->extra_flags, ITEM_UNIQUE);
        if (IS_OBJ_STAT(unique, ITEM_REMORT)) {
            min_level = UMAX(80 + (unique->level / 4), min_level);
            REMOVE_BIT(unique->extra_flags, ITEM_REMORT);
        }
        if (mod_weight + unique->weight > 0) {
            unique->weight = mod_weight + unique->weight;
            ch->carry_weight = UMAX((ch->carry_weight + mod_weight), 0);
        }
        new_extras = new_extras | unique->extra_flags;
        new_apply = new_apply | unique->item_apply;

        if (unique->extra_flags != new_extras)
            unique->extra_flags = new_extras;
        if (unique->item_apply != new_apply)
            unique->item_apply = new_apply;

        ch->quest_points -= qp_cost;
        sprintf(enchant_buf, "%s", "Enchantments Used: ");
        for (this_obj = matrix->first_in_carry_list; this_obj != NULL; this_obj = this_obj_next) {
            this_obj_next = this_obj->next_in_carry_list;
            if (this_obj->item_type == ITEM_ENCHANTMENT) {
                sprintf(enchant_catbuf, "%s (%d) ", this_obj->name, this_obj->pIndexData->vnum);
                safe_strcat(MSL, enchant_buf, enchant_catbuf);
                extract_obj(this_obj);
            }
            safe_strcat(MSL, enchant_buf, "\n\r");
        }
        if (unique->level < min_level && !IS_SET(unique->extra_flags, ITEM_ADEPT))
            unique->level = UMIN(120, min_level);

        act("$n dances about wildly, chanting weird mantras, and gestures crazily over $p.", ch, matrix, NULL, TO_ROOM);
        act("You enchant $p with additional powers!", ch, unique, NULL, TO_CHAR);
        do_save(ch, "");

        {
            FILE               *fp;
            char                brandbuf[MSL];
            char                cat2_buf[MSL];
            AFFECT_DATA        *one_aff;
            char               *strtime;

            strtime = ctime(&current_time);
            strtime[strlen(strtime) - 1] = 0;

            sprintf(brandbuf, "%s UNIQUE %s keyword[%s] vnum %d flags[%s] level %d\n",
                strtime, ch->name, unique->name, unique->pIndexData->vnum, extra_bit_name(unique->extra_flags), unique->level);

            for (one_aff = unique->first_apply; one_aff != NULL; one_aff = one_aff->next) {
                if (one_aff->location != APPLY_NONE && one_aff->modifier != 0) {
                    sprintf(cat2_buf, "Affects %s by %d.\n", affect_loc_name(one_aff->location), one_aff->modifier);
                    safe_strcat(MSL, brandbuf, cat2_buf);

                }
            }
            sprintf(brandbuf + strlen(brandbuf), "Total cost: %d qp\n", qp_cost);

            if (!nosave && (fp = fopen(BRANDSLOG_FILE, "a")) != NULL) {
                FPRINTF(fp, "%s", brandbuf);
                fclose(fp);
            }

            send_to_char("Your messages have been updated, and logged for inspection by an Immortal.\n\r", ch);
        }
        return;
    }

    do_enchant(ch, "");
    return;
}
