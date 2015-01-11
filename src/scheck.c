#include <sys/types.h>
#include <sys/time.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"
#include "tables.h"
#include "auction.h"
#include "duel.h"
#include "ssm.h"
#include "treasury.h"

IDSTRING(rcsid, "$Id: scheck.c,v 1.23 2003/09/18 12:45:18 dave Exp $");

/*  String checker, Spectrum 11/96
 *
 *  Basic idea is to walk through all the strings we know about, and mark them
 *  as referenced. Then we check for strings that have a reference count thats
 *  different from ptr->usage and log them
 */

/*
 * Things which are walked (anything else must be touched from these):
 *
 * o char_list
 * o descriptor_list
 * o object_list
 * o mob_index hash table
 * o obj_index hash table
 * o room_index hash table
 * o socials table
 * o areas
 * o notes/ideas/etc
 * o songs
 *
 */

extern BAN_DATA    *first_ban;

extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
extern OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];

/* Main code */

static void
touch(char *str)
{
    BufEntry           *p;

    if (!str)
        return;

    if (str < string_space || str > top_string)
        return;                    /* not in string space */

    p = (BufEntry *) (str - HEADER_SIZE);
    p->ref++;
}

static void
clear(void)
{
    /* Set all reference counts to 0 */

    BufEntry           *p;

    for (p = ssm_buf_head; p; p = p->next)
        p->ref = 0;
}

static BufEntry    *dump_ptr[2];

static long
dump(void)
{
    /* Dump strings that have ref!=usage */

    FILE               *dumpf;
    BufEntry           *p;
    long                count = 0;

    fclose(fpReserve);
    dumpf = fopen("leaks.dmp", "w");

    for (p = ssm_buf_head; p; p = p->next) {
        if (p->usage > 0 && p->usage != p->ref) {
            FPRINTF(dumpf, "usage %2d/%2d, caller %s, string %s\n", p->ref, p->usage, p->caller, p->buf);
            count += abs(p->usage - p->ref);
        }
        dump_ptr[0] = dump_ptr[1];
        dump_ptr[1] = p;
    }

    fclose(dumpf);
    fpReserve = fopen(NULL_FILE, "r");

    return count;
}

static void
walk_mprog_data(MPROG_DATA *prog)
{
    if (!prog)
        return;

    touch(prog->arglist);
    touch(prog->comlist);
    touch(prog->filename);

}

static void
walk_mob_index_data(MOB_INDEX_DATA *m)
{
    MPROG_DATA         *mobprog;

    if (!m)
        return;

    touch(m->player_name);
    touch(m->short_descr);
    touch(m->long_descr);
    touch(m->description);
    touch(m->target);
    touch(m->path);

    for (mobprog = m->first_mprog; mobprog; mobprog = mobprog->next)
        walk_mprog_data(mobprog);

}

static void
walk_pcdata(PC_DATA *p)
{
    int                 i;
    IGNORE_DATA         *ignore;

    if (!p)
        return;

    touch(p->pwd);
    touch(p->bamfin);
    touch(p->bamfout);
    touch(p->title);
    touch(p->room_enter);
    touch(p->room_exit);
    touch(p->immskll);
    touch(p->host);
    touch(p->ip);
    touch(p->who_name);
    touch(p->header);
    touch(p->message);
    touch(p->load_msg);
    touch(p->accept_name);
    touch(p->prompt);
    touch(p->battleprompt);
    touch(p->noteprompt);
    touch(p->origname);

    for (i = 0; i < MAX_ALIASES; i++) {
        touch(p->alias[i]);
        touch(p->alias_name[i]);
    }

    for (i = 0; i < 5; i++) {
        touch(p->pedit_string[i]);
    }

    for (ignore = p->first_ignore; ignore != NULL; ignore = ignore->next)
        touch(ignore->char_ignored);

}

static void
walk_note_data(NOTE_DATA *note)
{
    if (!note)
        return;

    touch(note->from);
    touch(note->to);
    touch(note->subject);
    touch(note->text);
}

static void
walk_brand_data(BRAND_DATA *brand)
{
    if (!brand)
        return;

    touch(brand->branded);
    touch(brand->branded_by);
    touch(brand->dt_stamp);
    touch(brand->message);
    touch(brand->priority);
}

static void
walk_brands(void)
{
    BRAND_DATA         *this_brand;
    DL_LIST            *brands;

    for (brands = first_brand; brands; brands = brands->next) {
        this_brand = brands->this_one;
        walk_brand_data(this_brand);
    }
}

void
walk_notelist(NOTE_DATA *pnote)
{
    for (; pnote; pnote = pnote->next)
        walk_note_data(pnote);
}

void walk_answering_data(ANSWERING_DATA *a)
{
    touch(a->name);
    touch(a->message);
}

void walk_answeringlist(ANSWERING_DATA *a)
{
    for (; a; a = a->next)
        walk_answering_data(a);
}

static void
walk_char_data(CHAR_DATA *ch)
{
    if (!ch)
        return;

    walk_notelist(ch->pnote);
    walk_pcdata(ch->pcdata);
    walk_answeringlist(ch->first_message);

    touch(ch->searching);
    touch(ch->name);
    touch(ch->short_descr);
    touch(ch->long_descr);
    touch(ch->long_descr_orig);
    touch(ch->description);
    touch(ch->afk_msg);
    touch(ch->searching);
    touch(ch->target);

}

static void
walk_extra_descr_data(EXTRA_DESCR_DATA *ed)
{
    if (!ed)
        return;

    touch(ed->keyword);
    touch(ed->description);
}

static void
walk_obj_index_data(OBJ_INDEX_DATA *o)
{
    EXTRA_DESCR_DATA   *ed;

    if (!o)
        return;

    for (ed = o->first_exdesc; ed; ed = ed->next)
        walk_extra_descr_data(ed);

    touch(o->name);
    touch(o->short_descr);
    touch(o->description);
    touch(o->owner);
}

static void
walk_obj_data(OBJ_DATA *o)
{
    EXTRA_DESCR_DATA   *ed;

    if (!o)
        return;

    for (ed = o->first_exdesc; ed; ed = ed->next)
        walk_extra_descr_data(ed);

    touch(o->owner);
    touch(o->name);
    touch(o->short_descr);
    touch(o->description);

}

static void
walk_exit_data(EXIT_DATA *e)
{
    if (!e)
        return;

    touch(e->keyword);
    touch(e->description);
}

static void
walk_reset_data(RESET_DATA *r)
{
    if (!r)
        return;

    touch(r->notes);
    touch(r->auto_message);
}

static void
walk_area_data(AREA_DATA *ad)
{

    RESET_DATA         *reset;

    if (!ad)
        return;

    touch(ad->filename);
    touch(ad->name);
    touch(ad->owner);
    touch(ad->can_read);
    touch(ad->can_write);
    touch(ad->keyword);            /* spec- missed strings */
    touch(ad->level_label);        /* spec - missed strings */
    touch(ad->reset_msg);
    touch(ad->nocmd);
    touch(ad->nospell);

    for (reset = ad->first_reset; reset; reset = reset->next)
        walk_reset_data(reset);

}

static void
walk_room_index_data(ROOM_INDEX_DATA *r)
{
    int                 i;
    EXTRA_DESCR_DATA   *ed;

    /*  BUILD_DATA_LIST *reset;  */
    if (!r)
        return;

    for (i = 0; i < 6; i++)
        walk_exit_data(r->exit[i]);

    /* exit[i] == old_exit[i], so don't walk it twice */

    for (ed = r->first_exdesc; ed; ed = ed->next)
        walk_extra_descr_data(ed);
    /*  for ( reset = r->first_room_reset; reset; reset = reset->next )
       walk_reset_data( reset->data );    */
    touch(r->name);
    touch(r->description);
    touch(r->auto_message);

    touch(r->nocmd);
    touch(r->nospell);
}

static void
walk_teleport_data(TELEPORT_DATA *t)
{
    touch(t->in);
    touch(t->out);
}

static void
walk_social_type(struct social_type *s)
{

    if (!s)
        return;
    touch(s->name);
    touch(s->char_no_arg);
    touch(s->others_no_arg);
    touch(s->char_found);
    touch(s->others_found);
    touch(s->vict_found);
    touch(s->char_auto);
    touch(s->others_auto);
}

static void
walk_descriptor_data(DESCRIPTOR_DATA *d)
{
    if (!d)
        return;

    touch(d->host);
    touch(d->ip);
}

static void
walk_ban_data(BAN_DATA *b)
{
    touch(b->name);
    touch(b->banned_by);
}

static void
walk_socials(void)
{
    extern int          maxSocial;
    int                 i;

    for (i = 0; i < maxSocial; i++)
        walk_social_type(&social_table[i]);
}

static void
walk_chars(void)
{
    CHAR_DATA          *ch;

    for (ch = first_char; ch; ch = ch->next)
        walk_char_data(ch);
}

static void
walk_descriptors(void)
{
    DESCRIPTOR_DATA    *d;

    for (d = first_desc; d; d = d->next)
        walk_descriptor_data(d);
}

static void
walk_objects(void)
{
    OBJ_DATA           *o;

    for (o = first_obj; o; o = o->next)
        walk_obj_data(o);
}

static void
walk_areas(void)
{
    AREA_DATA          *ad;

    for (ad = first_area; ad; ad = ad->next)
        walk_area_data(ad);
}

static void
walk_mob_indexes(void)
{
    MOB_INDEX_DATA     *m;
    int                 i;

    for (i = 0; i < MAX_KEY_HASH; i++)
        for (m = mob_index_hash[i]; m; m = m->next)
            walk_mob_index_data(m);
}

static void
walk_obj_indexes(void)
{
    OBJ_INDEX_DATA     *o;
    int                 i;

    for (i = 0; i < MAX_KEY_HASH; i++)
        for (o = obj_index_hash[i]; o; o = o->next)
            walk_obj_index_data(o);
}

static void
walk_room_indexes(void)
{
    ROOM_INDEX_DATA    *r;
    int                 i;

    for (i = 0; i < MAX_KEY_HASH; i++)
        for (r = room_index_hash[i]; r; r = r->next)
            walk_room_index_data(r);
}

static void
walk_notes(void)
{
    walk_notelist(first_note);
}

#if 0
static void
walk_songs(void)
{
    int                 i;

    for (i = 0; i < MAX_SONGS; i++)
        if (song_table[i].name)
            walk_song_data(&song_table[i]);
}
#endif

static void
walk_bans(void)
{
    BAN_DATA           *b;

    for (b = first_ban; b; b = b->next)
        walk_ban_data(b);
}

#if 0
static void
walk_bounties(void)
{
    BOUNTY_DATA        *b;

    for (b = bounty_list; b; b = b->next)
        walk_bounty_data(b);
}
#endif

static void
walk_message_data(MESSAGE_DATA *m)
{
    if (!m)
        return;

    touch(m->message);
    touch(m->author);
    touch(m->title);
}

void
walk_messages(MESSAGE_DATA *m)
{
    for (; m; m = m->next)
        walk_message_data(m);
}

void
walk_boards(void)
{
    BOARD_DATA         *board;
    extern BOARD_DATA  *first_board;

    for (board = first_board; board; board = board->next)
        walk_messages(board->first_message);
}

void
walk_teleports(void)
{
    TELEPORT_DATA *tele;

    for (tele = first_tele; tele; tele = tele->next)
        walk_teleport_data(tele);
}

void walk_ruler_data(RULER_DATA *r)
{
    touch(r->name);
    touch(r->whoname);
    touch(r->rank);
};

void walk_rulers(void)
{
    RULER_DATA *r;

    for (r = first_ruler; r; r = r->next)
        walk_ruler_data(r);

}

void walk_mudsets(void)
{
    int cnt;

    for (cnt = 0; mudset_table[cnt].name[0] != '\0'; cnt++)
        if (mudset_table[cnt].type == MUDSET_TYPE_STRING)
            touch(*(char **)mudset_table[cnt].var);
}

void walk_quest_data(QUEST_DATA *q)
{
    touch(q->mob);
    touch(q->thief);
    touch(q->ch);
}

void walk_quests(void)
{
    QUEST_DATA *q;

    for (q = first_quest; q; q = q->next)
        walk_quest_data(q);
}

void walk_shelp_data(SHELP_DATA *s)
{
    touch(s->name);
    touch(s->duration);
    touch(s->modify);
    touch(s->type);
    touch(s->target);
    touch(s->desc);
}

void walk_shelps(void)
{
    SHELP_DATA *s;

    for (s = first_shelp; s; s = s->next)
        walk_shelp_data(s);
}

void walk_cinfo_data(CINFO_DATA *c)
{
    touch(c->name);
}

void walk_cinfos(void)
{
    CINFO_DATA *c;

    for (c = first_cinfo; c; c = c->next)
        walk_cinfo_data(c);
}

void walk_auction_data(AUCTION_DATA *a)
{
    touch(a->keyword);
    touch(a->owner);
    touch(a->owner_name);
    touch(a->bidder);
    touch(a->bidder_name);
}

void walk_auctions(void)
{
    AUCTION_DATA *a;

    for (a = first_auction; a; a = a->next)
        walk_auction_data(a);
}

void walk_treasury_history(TREASURY_HISTORY_DATA *h)
{
    touch(h->who);
}

void walk_treasuries(void)
{
    TREASURY_HISTORY_DATA *h;
    int cnt;

    for (cnt = 0; cnt < MAX_CLAN; cnt++)
        for (h = treasury[cnt].first_history; h != NULL; h = h->next)
            walk_treasury_history(h);
}

void
do_scheck(CHAR_DATA *ch, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    extern bool         disable_timer_abort;
    extern char        *last_reboot_by;

    disable_timer_abort = TRUE;
    clear();

    touch(last_reboot_by);
    walk_socials();
    walk_chars();
    walk_descriptors();
    walk_objects();
    walk_areas();
    walk_bans();

    walk_mob_indexes();
    walk_obj_indexes();
    walk_room_indexes();
    walk_notes();
    walk_boards();
    walk_brands();
    walk_teleports();
    walk_rulers();
    walk_mudsets();
    walk_quests();
    walk_shelps();
    walk_cinfos();
    walk_auctions();
    walk_treasuries();

    sprintf(buf, "%ld leaks dumped to leaks.dmp\n\r", dump());
    send_to_char(buf, ch);
    disable_timer_abort = FALSE;
}
