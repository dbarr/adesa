#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include "merc.h"

IDSTRING(rcsid, "$Id: lists.c,v 1.13 2003/09/18 12:45:18 dave Exp $");

/*
 * All of the global linked lists, in one clump.  Variables here,
 * declarations in lists.h
 * -- Altrag
 */
AREA_DATA          *first_area = NULL;
AREA_DATA          *last_area = NULL;

/* teleport */
TELEPORT_DATA      *first_tele = NULL;
TELEPORT_DATA      *last_tele = NULL;

/* qinfo */
QUEST_DATA         *first_quest = NULL;
QUEST_DATA         *last_quest = NULL;

CINFO_DATA         *first_cinfo = NULL;
CINFO_DATA         *last_cinfo = NULL;

BAN_DATA           *first_ban = NULL;
BAN_DATA           *last_ban = NULL;
CHAR_DATA          *first_char = NULL;
CHAR_DATA          *last_char = NULL;
CHAR_DATA          *first_player = NULL;
CHAR_DATA          *last_player = NULL;
DESCRIPTOR_DATA    *first_desc = NULL;
DESCRIPTOR_DATA    *last_desc = NULL;
SHELP_DATA         *first_shelp = NULL;
SHELP_DATA         *last_shelp = NULL;
NOTE_DATA          *first_note = NULL;
NOTE_DATA          *last_note = NULL;
OBJ_DATA           *first_obj = NULL;
OBJ_DATA           *last_obj = NULL;
SHOP_DATA          *first_shop = NULL;
SHOP_DATA          *last_shop = NULL;
CORPSE_DATA        *first_corpse = NULL;
CORPSE_DATA        *last_corpse = NULL;
CONTROL_LIST       *first_control_list = NULL;
CONTROL_LIST       *last_control_list = NULL;
QUEUED_INTERACT_LIST *first_queued_interact = NULL;
QUEUED_INTERACT_LIST *last_queued_interact = NULL;
INFLUENCE_LIST     *first_influence_list = NULL;
INFLUENCE_LIST     *last_influence_list = NULL;
RULER_DATA         *first_ruler = NULL;
RULER_DATA         *last_ruler = NULL;
DL_LIST            *first_brand = NULL;
DL_LIST            *last_brand = NULL;

MPROG_ACT_LIST     *first_mpact = NULL;
MPROG_ACT_LIST     *last_mpact = NULL;

PORTAL_DATA        *portal_free = NULL;
AFFECT_DATA        *affect_free = NULL;
ROOM_AFFECT_DATA   *raffect_free = NULL;
AREA_DATA          *area_free = NULL;

/* teleport */
TELEPORT_DATA      *tele_free = NULL;

/* qinfo */
QUEST_DATA         *quest_free = NULL;

CINFO_DATA         *cinfo_free = NULL;

/* answering machine stuff -ogma- */
ANSWERING_DATA     *answering_free = NULL;
IGNORE_DATA        *ignore_free = NULL;

BAN_DATA           *ban_free = NULL;
CHAR_DATA          *char_free = NULL;
CHAR_DATA          *player_free = NULL;
DESCRIPTOR_DATA    *desc_free = NULL;
EXIT_DATA          *exit_free = NULL;
EXTRA_DESCR_DATA   *exdesc_free = NULL;
SHELP_DATA         *shelp_free = NULL;
MOB_INDEX_DATA     *mid_free = NULL;
NOTE_DATA          *note_free = NULL;
OBJ_DATA           *obj_free = NULL;
OBJ_INDEX_DATA     *oid_free = NULL;
PC_DATA            *pcd_free = NULL;
RESET_DATA         *reset_free = NULL;
ROOM_INDEX_DATA    *rid_free = NULL;
SHOP_DATA          *shop_free = NULL;
MPROG_DATA         *mprog_free = NULL;
MPROG_ACT_LIST     *mpact_free = NULL;
BUILD_DATA_LIST    *build_free = NULL;
SHIELD_DATA        *shield_free = NULL;
MEMBER_DATA        *member_free = NULL;
CORPSE_DATA        *corpse_free = NULL;
MARK_DATA          *mark_free = NULL;
MARK_LIST_MEMBER   *mark_list_free = NULL;
INTERACT_DATA      *interact_free = NULL;
INFLUENCE_DATA     *influence_free = NULL;
RULER_DATA         *ruler_free = NULL;
CONTROL_DATA       *control_data_free = NULL;
CONTROL_LIST       *control_list_free = NULL;
QUEUED_INTERACT_LIST *queued_interact_free = NULL;
INFLUENCE_LIST     *influence_list_free = NULL;
DL_LIST            *dl_list_free = NULL;
BRAND_DATA         *brand_data_free = NULL;

CHAR_DATA          *violence_marker = NULL;

LOCKER_DATA        *first_locker = NULL;
LOCKER_DATA        *last_locker = NULL;
LOCKER_DATA        *locker_free = NULL;

RENAME_DATA        *first_rename = NULL;
RENAME_DATA        *last_rename = NULL;
RENAME_DATA        *rename_free = NULL;

BUF_DATA_STRUCT    *first_buf = NULL;
BUF_DATA_STRUCT    *last_buf = NULL;
BUF_DATA_STRUCT    *buf_free = NULL;
