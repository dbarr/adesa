
/****************************************************************************
 *                                                                          *
 *     _/       _/_/_/  _/    _/   _/   BOARD.C : Bulletin Board system for *
 *    _/_/     _/       _/  _/     _/   ACK! MUD.  This code may be used by *
 *   _/  _/   _/        _/_/       _/   other parties, providing some form  *
 *  _/_/_/_/   _/       _/  _/          of acknowledgement is displayed :)  *
 * _/      _/   _/_/_/  _/    _/   _/         (C)1994 Stephen Dooley        *
 *                                      Thanks to Mart for some help here.  *
 ****************************************************************************/

#if defined(macintosh)
include < types.h >
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include "merc.h"

IDSTRING(rcsid, "$Id: board.c,v 1.15 2003/08/30 04:59:13 dave Exp $");

#define BOARD_DIR "boards"
#define T2000 -1                /* Terminator for files... */

extern char *my_left2(char *src, char *dst, int len, char fill);

/* Local functions */
BOARD_DATA         *load_board(OBJ_INDEX_DATA *pObj);
void                save_board(BOARD_DATA *board, CHAR_DATA *ch);
void                finished_editing(MESSAGE_DATA *msg, char **dest, CHAR_DATA *ch, bool saved);

/* Some locals used to manage the list of messages: */

BOARD_DATA         *first_board = NULL;
BOARD_DATA         *last_board = NULL;
BOARD_DATA         *board_free = NULL;
MESSAGE_DATA       *message_free = NULL;

/**************************************************************************
 *               MAG Modified outline                                     *
 *       Use a directory called boards under area directory.              *
 *       Store a file for each board called board.vnum makes things a lot *
 *       easier, and means files can be transfered easily between boards  *
 *       use a time based means to get rid of messages.                   *
 *                                                       *
 *       Values:                              *
 *          Value0  :  Expiry time for a message in days.                 *
 *          Value1  :  Min level of player to read the board.             *
 *          Value2  :  Max level of player to write to the board.         *
 *          Value3  :  Board number (usually it's vnum for simplicity)    *
 *                                      *
 *       Uses commands in write.c for string editing.                     *
 *       This file does reading files, writing files and managing boards  *
 *                                      *
 **************************************************************************/

/************************************************************************** 
 *                        Outline of BOARD.C                              *
 *                        ^^^^^^^^^^^^^^^^^^                              *
 * This code was written for use in ACK! MUD.  It should be easy to       *
 * include it into a Diku Merc 2.0+ Mud.                                  *
 *                                                                        *
 * The following functions are needed:                                    *
 * 1) Show the contents of a board to a player.                           *
 * 2) Show a message to a player                                          *
 * 3) Add to a message & finish writing a message.                        *
 * 4) Start writing a message                                             *
 * 5) Remove a message                                                    *
 * 6) Save the messages                                                   *
 * 7) Load the messages (only used at start-up)                           *
 *                                                                        *
 * Also, the code for the commands write and read are in this file.       *
 * WRITE checks for a board, and calls either 4) or 3) above.             *
 * READ calls 2) above.  The LOOK function was ammended to allow players  *
 * to type 'look board' or 'look at board' which calls 1) above.          *
 *                                                                        *
 * MESSAGE DATA holds the vnum of the board where the message was written *
 * and the message info.  There is no seperate save file or structure for *
 * each board.  Instead, they are all stored together.  I have used       *
 * OBJ_TYPE 23 for boards, with the following values used:                *
 * value 0: max number of messages allowed                                *
 * value 1: min level of player to read the board                         *
 * value 2: min level of player to write on the board                     *
 * value 3: the vnum of the board...NOT the vnum of the room...           *
 **************************************************************************/

/**************************************************************************
 * Ick ick ick!  Remove all the dammed builder functions, and use the     *
 * general merc memory functions.  -- Altrag                              *
 **************************************************************************/

void
do_board(CHAR_DATA *ch, char *argument)
{
    BOARD_DATA         *board;
    OBJ_DATA           *obj;
    int                 bval;
    int                 boardnum;

    char                arg1[MAX_INPUT_LENGTH];
    char                arg2[MAX_INPUT_LENGTH];

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if ((arg1[0] == '\0')
        || (arg1[0] != '\0' && !is_name(arg1, "list vnum expire read write clan"))
        ) {
        send_to_char("Board syntax: board <list|vnum|expire|read|write|clan>\n\r", ch);
        return;
    }

    if ((obj = get_obj_here(ch, "board")) == NULL) {
        send_to_char("There's no board in this room.\n\r", ch);
        return;
    }

    boardnum = obj->value[3];

    for (board = first_board; board != NULL; board = board->next) {
        if (board->vnum == boardnum)
            break;
    }

    if (board == NULL)
        board = load_board(obj->pIndexData);

    if (!str_cmp(arg1, "list")) {
        sendf(ch, "vnum: @@y%d@@g expire: @@y%d@@g, read: @@y%d@@g, write: @@y%d@@g, clan: @@e%d@@N\n\r",
            board->vnum, board->expiry_time, board->min_read_lev, board->min_write_lev, board->clan);
        return;
    }

    bval = atoi(arg2);

    if (!str_cmp(arg1, "vnum")) {
        board->vnum = bval;
    }
    if (!str_cmp(arg1, "expire")) {
        board->expiry_time = bval;
    }
    if (!str_cmp(arg1, "read")) {
        board->min_read_lev = bval;
    }
    if (!str_cmp(arg1, "write")) {
        board->min_write_lev = bval;
    }
    if (!str_cmp(arg1, "clan")) {
        board->clan = bval;
    }

    send_to_char("Done. Saving board..\n\r", ch);

    save_board(board, ch);
    return;
}

void
do_show_contents(CHAR_DATA *ch, OBJ_DATA *obj)
{
    /* Show the list of messages that are present on the board that ch is
     * looking at, indicated by board_vnum...
     */

    char                buf[MSL];
    char                nbuf[MIL];
    char                sbuf[MIL];
    MESSAGE_DATA       *msg;
    BOARD_DATA         *board;
    OBJ_INDEX_DATA     *pObj;
    int                 cnt = 0;
    int                 longest_name = 4; /* So "Name" in the header takes up 4 so set this as our minimum */
    int                 board_num;

    pObj = obj->pIndexData;
    board_num = pObj->value[3];

    /* First find the board, and if not there, create one. */
    for (board = first_board; board != NULL; board = board->next) {
        if (board->vnum == board_num)
            break;
    }

    if (board == NULL)
        board = load_board(pObj);

    /* check here to see if player allowed to read board */

    if (board->min_read_lev > get_trust(ch)) {
        send_to_char("You are not allowed to look at this board.\n\r", ch);
        return;
    }

    if ((board->clan != 0)
        && !IS_NPC(ch)
        && ch->pcdata->clan != board->clan && ch->level < MAX_LEVEL) {
        send_to_char("You are not of the right clan to read this board.\n\r", ch);
        return;
    }


    for (msg = board->first_message; msg != NULL; msg = msg->next) {
        if (my_strlen(msg->author) > longest_name)
            longest_name = UMIN(12, my_strlen(msg->author)); /* 12 is max regardless */

        cnt++;
    }

    if (cnt == 0) {                /* then there were no messages here */
        send_to_char("This board has no messages.\n\r", ch);
        return;
    }

    send_to_char("@@N@@d.------------------------------------------------------------@@g=( @@WBoard List @@g)=@@d-.\n\r", ch);

    sendf(ch, "@@d|   @@a# @@d| %s @@d| %s @@d| @@cDate                     @@d|\n\r",
        my_right("@@gName", nbuf, longest_name),
        my_left("@@WSubject", sbuf, 39 - longest_name));

    sendf(ch, "@@d|-----+-%s-+-%s-+--------------------------|\n\r",
        my_left("------------", nbuf, longest_name),
        my_left("-------------------------------------------", sbuf, 39 - longest_name));

    cnt = 0;
    for (msg = board->first_message; msg != NULL; msg = msg->next) {

        /* This is a hack to remove the hardcoded datestamp from the title
         * TODO: Give board files revisions and in a revision change, remove
         * the redundant datestamp from titles
         */
        strcpy(buf, msg->title);
        buf[strlen(buf) - 32] = '\0';

        /* Output each message */
        sendf(ch, "@@d| @@a%3d @@d| @@g%s @@N@@d| @@W%s @@N@@d| @@c%.24s @@d|\n\r",
            ++cnt,
            my_right(msg->author, nbuf, longest_name),
            my_left(buf, sbuf, 39 - longest_name),
            ctime(&msg->datetime));
    }

    send_to_char("@@d'-----------------------------------------------------------------------------'@@N\n\r", ch);

    return;
}

BOARD_DATA         *
load_board(OBJ_INDEX_DATA *pObj)
{
    FILE               *board_file;
    char                buf[255];
    char               *word;
    char                letter;
    time_t              message_time;
    time_t              expiry_time;
    BOARD_DATA         *board;
    MESSAGE_DATA       *message;

    GET_FREE(board, board_free);
    LINK(board, first_board, last_board, next, prev);

    board->expiry_time = pObj->value[0];
    board->min_read_lev = pObj->value[1];
    board->min_write_lev = pObj->value[2];
    board->vnum = pObj->value[3];
    board->clan = 0;
    board->first_message = NULL;
    board->last_message = NULL;

    fclose(fpReserve);

    sprintf(buf, "%s/board.%i", BOARD_DIR, board->vnum);

    if ((board_file = fopen(buf, "r")) != NULL) {
        /* Read in Optional board parameters */
        for (;;) {
            if (feof(board_file))
                break;

            word = fread_word(board_file);
            if (!str_cmp(word, "ExpiryTime")) {
                board->expiry_time = fread_number(board_file);
                fread_to_eol(board_file);
            }
            if (!str_cmp(word, "MinReadLev")) {
                board->min_read_lev = fread_number(board_file);
                fread_to_eol(board_file);
            }
            if (!str_cmp(word, "MaxWriteLev")) {
                board->min_write_lev = fread_number(board_file);
                fread_to_eol(board_file);
            }
            if (!str_cmp(word, "Clan")) {
                board->clan = fread_number(board_file);
                fread_to_eol(board_file);
            }
            if (!str_cmp(word, "Messages")) {
                fread_to_eol(board_file);
                break;
            }
        }

        if (board->expiry_time > 0)
            expiry_time = time(NULL) - (board->expiry_time) * 3600 * 24;
        else
            expiry_time = 0;

        /* Now read in messages */
        for (;;) {
            if (feof(board_file))
                break;

            letter = fread_letter(board_file);
            if (letter == 'S')
                break;

            if (letter != 'M') {
                bug("Letter in message file not M");
                break;
            }

            /* check time */
            message_time = (time_t) fread_number(board_file);
            if (feof(board_file))
                break;

            if (message_time < expiry_time) {
                char               *dumpme;

                dumpme = fread_string(board_file);    /* author  */
                free_string(dumpme);
                dumpme = fread_string(board_file);    /* title   */
                free_string(dumpme);
                dumpme = fread_string(board_file);    /* message */
                free_string(dumpme);
            }
            else {
                GET_FREE(message, message_free);
                message->datetime = message_time;
                message->author = fread_string(board_file);
                message->title = fread_string(board_file);
                message->message = fread_string(board_file);
                LINK(message, board->first_message, board->last_message, next, prev);
                message->board = board;
            }
        }

        /* Now close file */
        fclose(board_file);
    }

    /* board fix(?), wasn't being called if the board file wasnt found */
    fpReserve = fopen(NULL_FILE, "r");

    return board;
}

void
save_board(BOARD_DATA *board, CHAR_DATA *ch)
{
    char                buf[MAX_STRING_LENGTH];
    FILE               *board_file;
    MESSAGE_DATA       *message;

    if (nosave)
        return;

    fclose(fpReserve);

    sprintf(buf, "%s/board.%i", BOARD_DIR, board->vnum);
    if ((board_file = fopen(buf, "w")) == NULL) {
        send_to_char("Cannot save board, please contact an immortal.\n\r", ch);
        bugf("Could not save file board.%i.", board->vnum);
        fpReserve = fopen(NULL_FILE, "r");
        return;
    }

    FPRINTF(board_file, "ExpiryTime  %i\n", board->expiry_time);
    FPRINTF(board_file, "MinReadLev  %i\n", board->min_read_lev);
    FPRINTF(board_file, "MaxWriteLev %i\n", board->min_write_lev);
    FPRINTF(board_file, "Clan        %i\n", board->clan);

    /* Now print messages */
    FPRINTF(board_file, "Messages\n");

    for (message = board->first_message; message; message = message->next) {
        FPRINTF(board_file, "M%i\n", (int) (message->datetime));

        strcpy(buf, message->author);    /* Must do copy, not allowed to change string directly */
        smash_tilde(buf);
        FPRINTF(board_file, "%s~\n", buf);

        strcpy(buf, message->title);
        smash_tilde(buf);
        FPRINTF(board_file, "%s~\n", buf);

        strcpy(buf, message->message);
        smash_tilde(buf);
        FPRINTF(board_file, "%s~\n", buf);

    }

    FPRINTF(board_file, "S\n");

    fclose(board_file);
    fpReserve = fopen(NULL_FILE, "r");

    return;
}

void
do_delete(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA           *object;
    BOARD_DATA         *board;
    MESSAGE_DATA       *msg;
    OBJ_INDEX_DATA     *pObj;
    int                 vnum;
    int                 mess_num;
    int                 cnt = 0;
    extern BUF_DATA_STRUCT *first_buf;
    BUF_DATA_STRUCT    *mybuf;
    char                to_check[MAX_INPUT_LENGTH];
    char                private_name[MAX_INPUT_LENGTH];
    char               *to_person;
    bool                candelete = FALSE;

    if (IS_NPC(ch)) {
        send_to_char("NPCs may *not* delete messages.  So there.\n\r", ch);
        return;
    }

    if (argument[0] == '\0') {
        send_to_char("You need to specify a message number to delete!\n\r", ch);
        return;
    }

    for (object = ch->in_room->first_content; object != NULL; object = object->next_in_room) {
        if (object->item_type == ITEM_BOARD)
            break;
    }

    if (object == NULL) {
        send_to_char("There is no board in this room.", ch);
        return;
    }

    pObj = object->pIndexData;
    vnum = pObj->value[3];

    /* First find the board, and if not there, create one. */
    for (board = first_board; board != NULL; board = board->next) {
        if (board->vnum == vnum)
            break;
    }

    if (board == NULL)
        board = load_board(pObj);

    /* check here to see if player allowed to write to board */

    if (board->min_write_lev > get_trust(ch)) {
        send_to_char("You are not allowed to delete on this board.\n\r", ch);
        return;
    }

    if ((board->clan != 0)
        && ch->pcdata->clan != board->clan && ch->level < MAX_LEVEL) {
        send_to_char("You are not of the right clan to delete on this board.\n\r", ch);
        return;
    }

    cnt = 0;
    mess_num = is_number(argument) ? atoi(argument) : 0;

    for (msg = board->first_message; msg != NULL; msg = msg->next)
        if (++cnt == mess_num)    /* then this the message!!! */
            break;

    if (msg == NULL) {
        send_to_char("No such message!\n\r", ch);
        return;
    }

    to_person = one_argument(msg->title, to_check);
    to_person = one_argument(to_person, private_name);

    if (   !str_cmp(ch->name, msg->author)
        || get_trust(ch) == MAX_LEVEL
        || (board->clan > 0 && IS_SET(ch->pcdata->pflags, PFLAG_CLAN_BOSS))
        || (!str_cmp("to:", to_check) && private_name[0] != '\0' && !str_cmp(ch->name, private_name))
       )
        candelete = TRUE;

    if (!candelete) {
        send_to_char("You cannot delete this message.\n\r", ch);
        return;
    }

    for (mybuf = first_buf; mybuf != NULL; mybuf = mybuf->next)
        if (msg == mybuf->returnparm) {
            send_to_char("Someone is editing that message!\n\r", ch);
            return;
        }

    /* Now delete message */

    UNLINK(msg, board->first_message, board->last_message, next, prev);
    free_string(msg->author);
    free_string(msg->title);
    free_string(msg->message);
    PUT_FREE(msg, message_free);

    save_board(board, ch);

    return;
}

void
do_show_message(CHAR_DATA *ch, int mess_num, OBJ_DATA *obj)
{
    /* Show message <mess_num> to character. 
     * check that message vnum == board vnum
     */

    BOARD_DATA         *board;
    OBJ_INDEX_DATA     *pObj;
    int                 vnum;
    MESSAGE_DATA       *msg;
    int                 cnt = 0;
    int                 cnt2 = 0;
    bool                mfound = FALSE;
    char                _buf2[MSL];
    char               *buf2 = _buf2;
    char                to_check[MAX_INPUT_LENGTH];
    char               *to_person;
    char                private_name[MAX_INPUT_LENGTH];

    pObj = obj->pIndexData;
    vnum = pObj->value[3];

    /* First find the board, and if not there, create one. */
    for (board = first_board; board != NULL; board = board->next) {
        if (board->vnum == vnum)
            break;
    }

    if (board == NULL)
        board = load_board(pObj);

    /* check here to see if player allowed to read board */

    if (board->min_read_lev > get_trust(ch)) {
        send_to_char("You are not allowed to look at this board.\n\r", ch);
        return;
    }

    if ((board->clan != 0)
        && !IS_NPC(ch)
        && ch->pcdata->clan != board->clan && ch->level < MAX_LEVEL) {
        send_to_char("You are not of the right clan to read this board.\n\r", ch);
        return;
    }

    for (msg = board->first_message; msg != NULL; msg = msg->next) {
        if (++cnt == mess_num) {    /* then this the message!!! */
            mfound = TRUE;

            to_person = one_argument(msg->title, to_check);
            to_person = one_argument(to_person, private_name);
            if (!str_cmp(to_check, "to:")
                && str_prefix(private_name, ch->name)
                && str_cmp(msg->author, ch->name)) {
                send_to_char("This is a private message.\n\r", ch);
                break;
            }

            /*
             * Temporary fix for messages longer than MSL
             */
            if ((strlen(msg->author)
                    + strlen(msg->title)
                    + strlen(msg->message)) > MAX_STRING_LENGTH - 100) {
                send_to_char("Message too long!\n\r", ch);
                break;
            }
            /* End Temporary fix */

            for (cnt2 = 0; msg->title[cnt2] != '\0'; cnt2++)
                if (msg->title[cnt2] != '\n' && msg->title[cnt2] != '\r')
                    *buf2++ = msg->title[cnt2];

            *buf2 = 0;

            sendf(ch, "** [%d] %12s : %s ** \n\r\n\r%s\n\r", cnt, msg->author, _buf2, msg->message);
            break;
        }
    }

    if (!mfound) {
        send_to_char("No such message!\n\r", ch);
    }

    return;
}

void
do_write(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA           *object;
    BOARD_DATA         *board;
    MESSAGE_DATA       *msg;
    OBJ_INDEX_DATA     *pObj;
    int                 vnum;
    extern char         str_empty[1];
    char                buf[MAX_STRING_LENGTH];
    int                 cnt = 0;

    if (IS_NPC(ch)) {
        send_to_char("NPCs may *not* write messages.  So there.\n\r", ch);
        return;
    }

    if (argument[0] == '\0') {
        send_to_char("You need to specify a subject for your message.\n\r", ch);
        return;
    }

    if (!ch->in_room)
        return;

    for (object = ch->in_room->first_content; object != NULL; object = object->next_in_room) {
        if (object->item_type == ITEM_BOARD)
            break;
    }

    if (object == NULL) {
        send_to_char("There is no board in this room.\n\r", ch);
        return;
    }

    pObj = object->pIndexData;
    vnum = pObj->value[3];

    /* First find the board, and if not there, create one. */
    for (board = first_board; board != NULL; board = board->next) {
        if (board->vnum == vnum)
            break;
    }

    if (board == NULL)
        board = load_board(pObj);

    /* check here to see if player allowed to write to board */

    if (board->min_write_lev > get_trust(ch)) {
        send_to_char("You are not allowed to write on this board.\n\r", ch);
        return;
    }

    if ((board->clan != 0)
        && ch->pcdata->clan != board->clan && ch->level < MAX_LEVEL) {
        send_to_char("You are not of the right clan to write on this board.\n\r", ch);
        return;
    }

    for (msg = board->first_message; msg != NULL; msg = msg->next)
        cnt++;

    if (cnt >= 300) {
        send_to_char("Too many messages on this board. Try again later.\n\r", ch);
        sprintf(log_buf, "%s tried adding a message to a full board in vnum %d.", ch->name, ch->in_room->vnum);
        monitor_chan(log_buf, MONITOR_OBJ);
        return;
    }

    if (my_strlen(argument) > 35) {
        send_to_char("Subjects are limited to 35 characters. Try making your subject smaller.\n\r", ch);
        return;
    }

    msg = NULL;
    GET_FREE(msg, message_free);    /* Dont put message in list till we  */
    msg->datetime = time(NULL);    /* we are sure we can edit.          */
    sprintf(buf, "%s @@a%s@@N", argument, (char *) ctime(&current_time));
    if (msg->title != NULL)
        free_string(msg->title);
    msg->title = str_dup(buf);
    if (msg->author != NULL)
        free_string(msg->author);
    msg->author = str_dup(ch->name);
    msg->message = NULL;
    msg->board = board;

    /* Now actually run the edit prog. */
    write_start(&msg->message, (void *) finished_editing, msg, ch);

    if (msg->message != &str_empty[0]) {
        send_to_char("Editing message. Type .help for help.\n\r", ch);
        LINK(msg, board->first_message, board->last_message, next, prev);
    }
    else {
        send_to_char("Could not add message.\n\r", ch);
        PUT_FREE(msg, message_free);
    }
    return;
}

/* Deals with taking message out of list if user aborts... */

void
finished_editing(MESSAGE_DATA *msg, char **dest, CHAR_DATA *ch, bool saved)
{
#ifdef CHECK_VALID_BOARD
    MESSAGE_DATA       *SrchMsg;
#endif

    if (!saved) {
#ifdef CHECK_VALID_BOARD
        for (SrchMsg = msg->board->messages; SrchMsg != NULL; SrchMsg = SrchMsg->next)
            if (SrchMsg == msg)
                break;

        if (SrchMsg == NULL) {
            /* Could not find this message in board list, just lose memory. */
            return;
        }
#endif

        UNLINK(msg, msg->board->first_message, msg->board->last_message, next, prev);
        PUT_FREE(msg, message_free);
    }
    else {
        save_board(msg->board, ch);
    }
    return;
}

void
do_read(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA           *obj;

    if ((argument[0] == '\0') || !is_number(argument)) {
        send_to_char("Read what?\n\r", ch);
        return;
    }

    for (obj = ch->in_room->first_content; obj != NULL; obj = obj->next_in_room) {
        if (obj->item_type == ITEM_BOARD)
            break;
    }

    if (obj == NULL) {
        send_to_char("There is no board in this room.\n\r", ch);
        return;
    }

    /* Hopefully, by now there should be a board in the room, and the
     * player should have supplied some sort of argument....
     */

    do_show_message(ch, atoi(argument), obj);
    return;
}

void
do_edit_message(CHAR_DATA *ch, int mess_num, OBJ_DATA *obj)
{
    /* Show message <mess_num> to character. 
     * check that message vnum == board vnum
     */

    BOARD_DATA         *board;
    OBJ_INDEX_DATA     *pObj;
    int                 vnum;
    MESSAGE_DATA       *msg;
    extern BUF_DATA_STRUCT *first_buf;
    BUF_DATA_STRUCT    *mybuf;
    int                 cnt = 0;
    bool                mfound = FALSE;

    pObj = obj->pIndexData;
    vnum = pObj->value[3];

    /* First find the board, and if not there, create one. */
    for (board = first_board; board != NULL; board = board->next) {
        if (board->vnum == vnum)
            break;
    }

    if (board == NULL)
        board = load_board(pObj);

    /* check here to see if player allowed to read board */

    if (board->min_read_lev > get_trust(ch)) {
        send_to_char("You are not allowed to edit this board.\n\r", ch);
        return;
    }

    if ((board->clan != 0)
        && !IS_NPC(ch)
        && ch->pcdata->clan != board->clan && ch->level < MAX_LEVEL) {
        send_to_char("You are not of the right clan to edit this board.\n\r", ch);
        return;
    }

    for (msg = board->first_message; msg != NULL; msg = msg->next) {
        if (++cnt == mess_num) {    /* then this the message!!! */
            mfound = TRUE;

            for (mybuf = first_buf; mybuf != NULL; mybuf = mybuf->next)
                if (msg == mybuf->returnparm) {
                    send_to_char("Someone is already editing that message!\n\r", ch);
                    return;
                }

            if (str_cmp(msg->author, ch->name)) {
                send_to_char("Not your message to edit!\n\r", ch);
                return;
            }
            else {
                build_strdup(&msg->message, "$edit", TRUE, ch);
            }

        }
    }
    if (!mfound)
        send_to_char("No such message!\n\r", ch);

    return;
}

void
do_edit(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA           *obj;

    if ((argument[0] == '\0') || !is_number(argument)) {
        send_to_char("Edit what?\n\r", ch);
        return;
    }

    for (obj = ch->in_room->first_content; obj != NULL; obj = obj->next_in_room) {
        if (obj->item_type == ITEM_BOARD)
            break;
    }

    if (obj == NULL) {
        send_to_char("There is no board in this room.\n\r", ch);
        return;
    }

    /* Hopefully, by now there should be a board in the room, and the
     * player should have supplied some sort of argument....
     */

    do_edit_message(ch, atoi(argument), obj);
    return;
}
