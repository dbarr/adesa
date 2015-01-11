#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include "merc.h"
#include "tables.h"
#include "auction.h"

#include <ctype.h>

IDSTRING(rcsid, "$Id: pdelete.c,v 1.7 2003/08/30 04:59:16 dave Exp $");

char               *
find_backup_slot(char *argument)
{
    static char         buf[MSL];
    int                 cnt = 0;
    struct stat         s;

    buf[0] = 0;

    if (*argument == '\0')
        return NULL;

    capitalize(argument);
    *argument = UPPER(*argument);

    for (cnt = 1; cnt < 1000; cnt++) {
        sprintf(buf, PLAYER_DIR "pdeleted/%s.%03d", argument, cnt);

        if (stat(buf, &s) == -1)
            return buf;
    }

    return NULL;
}

void
do_sdelete(CHAR_DATA *ch, char *argument)
{
    DESCRIPTOR_DATA    *d;
    char                strsave[MAX_INPUT_LENGTH];
    char               *strback;
    char                arg1[MAX_INPUT_LENGTH];
    char               *pArg;
    char                cEnd;
    char                buf[MAX_INPUT_LENGTH];
    char                md5buf[33];
    RULER_DATA         *ruler = NULL;

    if (IS_NPC(ch))
        return;

    strcpy(buf, ch->name);
    sprintf(strsave, "%s%s%s%s", PLAYER_DIR, initial(buf), "/", capitalize(buf));

    pArg = arg1;
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
        *pArg++ = *argument++;
    }
    *pArg = '\0';

    if ((ch->pcdata->pwd != '\0')
        && (arg1[0] == '\0')) {
        send_to_char("Syntax: pdelete <password>.\n\r", ch);
        return;
    }
    if ((ch->pcdata->pwd != '\0')
        && (strcmp(md5string(arg1, md5buf), ch->pcdata->pwd))) {
        WAIT_STATE(ch, 40);
        send_to_char("Wrong password.  Wait 10 seconds.\n\r", ch);
        return;
    }

    if (ch->level < 2) {
        do_quit(ch, "NOSAVECHECK");
        return;
    }

    /* check for current auctions.. could abandon the auction and allow them
       to pdelete, but.. bleh */
    {
        AUCTION_DATA       *auc;

        for (auc = first_auction; auc != NULL; auc = auc->next) {
            if (!str_cmp(auc->owner, ch->pcdata->origname) || !str_cmp(auc->bidder, ch->pcdata->origname)) {
                send_to_char("You are involved in a gold or quest point auction! Cannot pdelete yet.\n\r", ch);
                return;
            }
        }
    }

    if ((strback = find_backup_slot(ch->name)) != NULL)
        rename(strsave, strback);
    else
        unlink(strsave);

    send_to_char("Character deleted.\n\r", ch);
    sprintf(buf, "@@e%s @@N@@ePDELETES@@N", ch->pcdata->origname);
    info(buf, 90);

    if (ch->pcdata->clan > 0)
        update_cinfo(ch, TRUE);

    if (ch->adept_level == 20 && (ruler = get_ruler(ch))) {
        free_string(ruler->name);
        free_string(ruler->whoname);
        free_string(ruler->rank);
        UNLINK(ruler, first_ruler, last_ruler, next, prev);
        PUT_FREE(ruler, ruler_free);
        save_rulers();
    }

    d = ch->desc;
    extract_char(ch, TRUE);

    if (d != NULL)
        close_socket(d);

    return;
}
