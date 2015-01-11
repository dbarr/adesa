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
#include "merc.h"

IDSTRING(rcsid, "$Id: write.c,v 1.44 2003/10/30 02:50:40 dave Exp $");

extern char         str_empty[1];

/* A str function in build.c, used to duplicate strings into perm. mem. */
char               *build_simpstrdup(char *);
void wordwrap_format args((char *source, char *dest, int limit));

void
write_start(char **dest, void *retfunc, void *retparm, CHAR_DATA *ch)
{
    BUF_DATA_STRUCT    *buf_data;
    char               *buf;

    /* Check that *dest != &str_empty[0]  when calling this func. */
    /* If it is, it's because we've run out of memory. */

    buf = getmem(MAX_STRING_LENGTH);
    if (buf == NULL) {
        bug("Not enough memory for string editing.");
        *dest = &str_empty[0];
        send_to_char("WARNING: No memory left. Things will start to go wrong.\n\r", ch);
        return;
    }

    /* Alloc mem. for a new buffer. */
    GET_FREE(buf_data, buf_free);
    LINK(buf_data, first_buf, last_buf, next, prev);

    buf_data->ch = ch;
    buf_data->dest = dest;
    buf_data->buf = buf;
    buf_data->pos = 0;
    buf_data->returnfunc = (RET_FUN *) retfunc;
    buf_data->returnparm = retparm;

    *buf = '\0';

    buf_data->old_char_pos = ch->position;
    ch->position = POS_WRITING;

    *dest = buf;

    return;
}

void write_interpret
args((CHAR_DATA *ch, char *argument))
{

    BUF_DATA_STRUCT    *buf_data;
    char               *buf;
    int                 curlen;

    for (buf_data = first_buf; buf_data != NULL; buf_data = buf_data->next) {
        if (buf_data->ch == ch)
            break;
    }

    if (buf_data == NULL) {
        bugf("Call to write_interpret when not writing (char=%s)\n\r", ch->name);
        ch->position = POS_STANDING;
        return;
    }

    buf = buf_data->buf;

    /* Check to see if text was a command or simply addition */
    if (argument[0] != '.') {
        curlen = strlen(buf);
        if (curlen > (MAX_STRING_LENGTH - strlen(argument) - 100)) {
            send_to_char("String too long, cannot add new line.\n\r", ch);
            return;
        }
        for (buf = buf + curlen; *argument != '\0';)
            *(buf++) = *(argument++);
        *(buf++) = '\n';
        *(buf++) = '\r';
        *buf = '\0';
        return;
    }

    /* We have a command. */
    /* Commands are .help .save .preview .- .clear .lines */
    argument++;
    if (argument[0] == '\0' || UPPER(argument[0]) == 'S' || UPPER(argument[0]) == 'Q') {
        bool                save;
        char               **dest;
        CHAR_DATA          *ch;

        if (UPPER(argument[0]) == 'Q')
            save = 0;
        else
            save = 1;

        dest = buf_data->dest;
        ch = buf_data->ch;

        /* Save routine. */

        if (save) {
            /* Check that dest still points to buf (to check for corruption) */
            if (*dest != buf) {
                bug("write_interpret: Original destination has been overwritten.");
                send_to_char("Cannot save, string pointer been modified.\n\r", ch);
                /* if it gets to this stage, it crashes at dispose() :/
                   will look further into it */
                return;
            }
            else {
                *dest = str_dup(buf);
                if ((buf_data->returnfunc) != NULL)
                    (*buf_data->returnfunc) (buf_data->returnparm, dest, ch, TRUE);
            }
        }
        else {
            *dest = &str_empty[0];
            if ((buf_data->returnfunc) != NULL)
                (*buf_data->returnfunc) (buf_data->returnparm, dest, ch, FALSE);
        }

        /* Re-use memory. */
        if (buf_data->buf != NULL)
            dispose(buf_data->buf);

        UNLINK(buf_data, first_buf, last_buf, next, prev);

        /* Re-set char */
        ch->position = buf_data->old_char_pos;

        PUT_FREE(buf_data, buf_free);

        return;
    }

    if (UPPER(argument[0]) == 'H') {
        /* Help */
        CHAR_DATA          *ch;

        ch = buf_data->ch;

        do_help(ch, "editor");
        return;
    }

    if (UPPER(argument[0]) == 'P') {
        send_to_char(buf, ch);
        return;
    }

    if (UPPER(argument[0]) == 'F') {
        char                my_buf[MSL * 2];
        char               *mybuf = my_buf;
        char               *src = buf;
        int                 cnt = 0;

        wordwrap_format(buf, mybuf, 79);
        mybuf = my_buf;
        while (*mybuf && cnt++ < MSL * 2 - 1)
            *src++ = *mybuf++;

        *src = '\0';

        return;
    }

    if (argument[0] == '-') {
        char *rev;
        char last[MSL];

        if (!buf || buf[0] == '\0' || strcmp(buf, "\n\r") == 0) {
            send_to_char("No last line to delete!\n\r", ch);
            return;
        }

        rev = buf + strlen(buf) - 2; /* skip the \n\r at the end of the whole string */

        while (rev >= buf && *rev-- != '\r');
        strcpy(last, rev + 1);
        *(rev + 1) = '\0';

        /* don't send \n\r because the last line will already contain it */
        sendf(ch, "Deleted the last line: %s", last);
        return;
    }

    if (UPPER(argument[0]) == 'R') {
        char arg[MIL], arg2[MIL];
        char mybuf[MSL], _line[MSL], _linerep[MSL];
        char *line = _line;
        char *linerep = _linerep;
        char *in = buf;
        int cnt = 0;

        argument = one_argument_nolower(argument, arg); /* skip first argument, it's just .R(eplace) */
        argument = one_argument_nolower(argument, arg);
        argument = one_argument_nolower(argument, arg2);

        if (arg[0] == '\0' || arg2[0] == '\0') {
            send_to_char("syntax: .r <search> <replace>\n\r", ch);
            return;
        }

        sendf(ch, "@@N@@gReplacing '@@N%s@@N@@g' with '@@N%s@@N@@g'.@@N\n\r", arg, arg2);

        memset(_linerep, '\0', sizeof(_linerep));
        mybuf[0] = '\0';

        while (*in) {
            if (*in == '\r') {
                /* end of line */
                *line++ = *in++;
                *line = '\0';

                strnreplace(arg, arg2, _line, _linerep, MIL - 1);

                linerep = _linerep + strlen(_linerep);
                /* since we buffer limit, its possible we might lose the end of a line, so make sure we still have one */
                if (*(linerep - 1) != '\r') {
                    *(linerep - 2) = '\n';
                    *(linerep - 1) = '\r';
                }

                *linerep = '\0';

                cnt += strlen(_linerep);
                if (cnt > MSL - 100)
                    break;

                strcat(mybuf, _linerep);
                memset(_line, '\0', sizeof(_line)); line = _line;
                memset(_linerep, '\0', sizeof(_linerep)); linerep = _linerep;
            }
            else
                *line++ = *in++;
        }

        strncpy(buf, mybuf, MSL - 99);
        return;
    }

    if (UPPER(argument[0]) == 'C') {
        buf[0] = '\0';
        send_to_char("Done.\n\r", buf_data->ch);
        return;
    }

    send_to_char("Command not known, type .help for help.\n\r", buf_data->ch);
    return;
}

void
wordwrap_format(char *source, char *dest, int limit)
{
    char               *src = source;
    char               *dst = dest;
    char                cword[MSL];
    char               *word = cword;
    char                c = '\0';
    int                 cnt = 0;
    int                 foundspace = 0;
    int                 foundat = 0;

    if (*src == '\0') {
        *dst = '\0';
        return;
    }

    cword[0] = '\0';

    for (; *src; src++) {
        c = *src;

        /* silently ignore \r's. we add our own anyway */
        if (c == '\r')
            continue;

        if (c == '\n')
            c = ' ';

        /* treat colour codes differently */
        if (c == '@') {
            *word++ = c;
            *word = '\0';
            foundat++;
            cnt++;
            continue;
        }

        /* found a @@ combo. since a @@x code doesn't actually mean 3 characters
         * decide how many to reduce. */
        if (foundat >= 2) {
            *word++ = c;
            *word = '\0';
            cnt++;
            foundat = 0;

            if (c == '_')
                cnt--;
            else if (c == '-')
                cnt -= 2;
            else
                cnt -= 3;

            if (cnt < limit)
                continue;
            else {
                cnt--;
                c = ' ';
            }
        }

        /* normal character, increase our counter */
        cnt++;
        if (cnt >= limit) {
            /* we are at the end of the line and the last character is a space */
            if (c == ' ') {
                if (*(dst - 1) == ' ' && cword[0] == '\0')
                  dst--;

                for (word = cword; *word != '\0'; word++)
                    *dst++ = *word;

                word = cword;
                *word = '\0';

                *dst++ = '\n';
                *dst++ = '\r';
            }
            else { /* at the end of the line, last character is not a space, this means we will
                    * need to end our line early and put the last word on the next line */
                if (*(dst - 1) == ' ')
                  dst--;

                if (foundspace) {
                    *word++ = c;
                    *word = '\0';
                }
                else {
                    *word++ = c;
                    *word = '\0';
                    for (word = cword; *word != '\0'; word++)
                        *dst++ = *word;

                    word = cword;
                    *word = '\0';
                }

                *dst++ = '\n';
                *dst++ = '\r';
            }

            foundspace = 0;

            if (cword[0] == '\0')
                cnt = 0;
            else
                cnt = my_strlen(cword);
        }
        else if (c == ' ') {
            foundspace = 1;

            for (word = cword; *word != '\0'; word++)
                *dst++ = *word;

            if (cword[0] != '\0')
                *dst++ = ' ';

            word = cword;
            *word = '\0';
        }
        else {
            *word++ = c;
            *word = '\0';
        }
    }

    if (dst > dest && *(dst - 1) == ' ' && cword[0] == '\0')
        dst--;

    for (word = cword; *word != '\0'; word++)
        *dst++ = *word;

    *dst++ = '\n';
    *dst++ = '\r';
    *dst = '\0';
}

void
abort_writing(CHAR_DATA *ch)
{
    BUF_DATA_STRUCT    *buf_data;
    char               **dest;

    for (buf_data = first_buf; buf_data != NULL; buf_data = buf_data->next) {
        if (buf_data->ch == ch)
            break;
    }

    if (buf_data == NULL)
        return;

    dest = buf_data->dest;
    *dest = &str_empty[0];

    if ((buf_data->returnfunc) != NULL)
        (*buf_data->returnfunc) (buf_data->returnparm, dest, ch, FALSE);

    dispose(buf_data->buf);
    UNLINK(buf_data, first_buf, last_buf, next, prev);

    PUT_FREE(buf_data, buf_free);

    return;
}
