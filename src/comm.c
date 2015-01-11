
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

/*
 * This file contains all of the OS-dependent stuff:
 *   startup, signals, BSD sockets for tcp/ip, i/o, timing.
 *
 * The data flow for input is:
 *    Game_loop ---> Read_from_descriptor ---> Read
 *    Game_loop ---> Read_from_buffer
 *
 * The data flow for output is:
 *    Game_loop ---> Process_Output ---> Write_to_descriptor -> Write
 *
 * The OS-dependent functions are Read_from_descriptor and Write_to_descriptor.
 * -- Furey  26 Jan 1993
 */

#include <sys/types.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <errno.h>

#define __USE_BSD
#include <stdio.h>
#undef __USE_BSD

#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>

#include "merc.h"
#include "duel.h"
#include "dns.h"

IDSTRING(rcsid, "$Id: comm.c,v 1.87 2004/11/12 01:12:34 dave Exp $");

#undef isascii
#define isascii(c)    (((c) & ~0x7f) == 0)

/*
 * Malloc debugging stuff.
 */
#if defined(sun)
#undef MALLOC_DEBUG
#endif

#if defined(MALLOC_DEBUG)
#include <malloc.h>
extern int malloc_debug args((int));

extern int malloc_verify args((void));
#endif

/*
 * Signal handling.
 * Apollo has a problem with __attribute(atomic) in signal.h,
 *   I dance around it.
 */
#if defined(apollo)
#define __attribute(x)
#endif

#if defined(unix)
#include <signal.h>
#endif

#if defined(apollo)
#undef __attribute
#endif

/*
 * Socket and TCP/IP stuff.
 */
#if     defined(macintosh) || defined(MSDOS)
const char          echo_off_str[]  = { '\0' };
const char          echo_on_str[]   = { '\0' };
const char          go_ahead_str[]  = { '\0' };
const char          compress_will[] = { '\0' };
const char          comress_do[]    = { '\0' };
const char          comress_dont[]  = { '\0' };
#endif

#if     defined(unix)
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/telnet.h>
const unsigned char echo_off_str[] =  { IAC, WILL, TELOPT_ECHO, '\0' };
const unsigned char echo_on_str[] =   { IAC, WONT, TELOPT_ECHO, '\0' };
const unsigned char go_ahead_str[] =  { IAC, GA,   '\0' };
const unsigned char compress_will[] = { IAC, WILL, COMPRESS2,   '\0' };
const unsigned char compress_do[] =   { IAC, DO,   COMPRESS2,   '\0' };
const unsigned char compress_dont[] = { IAC, DONT, COMPRESS2,   '\0' };
#endif

/*
 * OS-dependent declarations.
 */
#if     defined(_AIX)
#include <sys/select.h>
int accept          args((int s, struct sockaddr * addr, int *addrlen));
int bind            args((int s, struct sockaddr * name, int namelen));
void bzero          args((char *b, int length));
int getpeername     args((int s, struct sockaddr * name, int *namelen));
int getsockname     args((int s, struct sockaddr * name, int *namelen));
int gettimeofday    args((struct timeval * tp, struct timezone * tzp));
int listen          args((int s, int backlog));
int setsockopt      args((int s, int level, int optname, void *optval, int optlen));
int socket          args((int domain, int type, int protocol));
#endif

#if     defined(apollo)
#include <unistd.h>
void bzero          args((char *b, int length));
#endif

#if     defined(__hpux)
int accept          args((int s, void *addr, int *addrlen));
int bind            args((int s, const void *addr, int addrlen));
void bzero          args((char *b, int length));
int getpeername     args((int s, void *addr, int *addrlen));
int getsockname     args((int s, void *name, int *addrlen));
int gettimeofday    args((struct timeval * tp, struct timezone * tzp));
int listen          args((int s, int backlog));
int setsockopt      args((int s, int level, int optname, const void *optval, int optlen));
int socket          args((int domain, int type, int protocol));
#endif

#if     defined(interactive)
#include <net/errno.h>
#include <sys/fcntl.h>
#endif

#if     defined(linux)
int close           args((int fd));
int gettimeofday    args((struct timeval * tp, struct timezone * tzp));
int select          args((int width, fd_set * readfds, fd_set * writefds, fd_set * exceptfds, struct timeval * timeout));
int socket          args((int domain, int type, int protocol));
#endif

#if     defined(macintosh)
#include <console.h>
#include <fcntl.h>
#include <unix.h>
struct timeval
{
    time_t              tv_sec;
    time_t              tv_usec;
};

static long         theKeys[4];

int gettimeofday    args((struct timeval * tp, void *tzp));
#endif

#if     defined(MIPS_OS)
extern int          errno;
#endif

#if     defined(MSDOS)
int gettimeofday    args((struct timeval * tp, struct timezone * tzp));
int kbhit           args((void));
#endif

#if     defined(NeXT)
int close           args((int fd));
int fcntl           args((int fd, int cmd, int arg));

# if     !defined(htons)
u_short htons       args((u_short hostshort));
# endif

# if     !defined(ntohl)
u_long ntohl        args((u_long hostlong));
# endif

int read            args((int fd, char *buf, int nbyte));
int select          args((int width, fd_set * readfds, fd_set * writefds, fd_set * exceptfds, struct timeval * timeout));
int write           args((int fd, char *buf, int nbyte));
#endif

#if     defined(sequent)
int accept          args((int s, struct sockaddr * addr, int *addrlen));
int bind            args((int s, struct sockaddr * name, int namelen));
int close           args((int fd));
int fcntl           args((int fd, int cmd, int arg));
int getpeername     args((int s, struct sockaddr * name, int *namelen));
int getsockname     args((int s, struct sockaddr * name, int *namelen));
int gettimeofday    args((struct timeval * tp, struct timezone * tzp));

# if     !defined(htons)
u_short htons       args((u_short hostshort));
# endif
int listen          args((int s, int backlog));

# if     !defined(ntohl)
u_long ntohl        args((u_long hostlong));
# endif

int read            args((int fd, char *buf, int nbyte));
int select          args((int width, fd_set * readfds, fd_set * writefds, fd_set * exceptfds, struct timeval * timeout));
int setsockopt      args((int s, int level, int optname, caddr_t optval, int optlen));
int socket          args((int domain, int type, int protocol));
int write           args((int fd, char *buf, int nbyte));
#endif

/*
 * This includes Solaris SYSV as well
 */

#if defined(sun)
int accept          args((int s, struct sockaddr * addr, int *addrlen));
int bind            args((int s, struct sockaddr * name, int namelen));
void bzero          args((char *b, int length));
int close           args((int fd));
int getpeername     args((int s, struct sockaddr * name, int *namelen));
int getsockname     args((int s, struct sockaddr * name, int *namelen));

# if defined(SYSV)
int gettimeofday    args((struct timeval * tp, void *tzp));
# else
int gettimeofday    args((struct timeval * tp, struct timezone * tzp));
# endif
int listen          args((int s, int backlog));
int select          args((int width, fd_set * readfds, fd_set * writefds, fd_set * exceptfds, struct timeval * timeout));

# if defined(SYSV)
int setsockopt      args((int s, int level, int optname, const char *optval, int optlen));
ssize_t read        args((int fd, void *buf, unsigned nbyte));
ssize_t write       args((int fd, const void *buf, unsigned nbyte));
# else
int setsockopt      args((int s, int level, int optname, void *optval, int optlen));
int read            args((int fd, char *buf, int nbyte));
int write           args((int fd, char *buf, int nbyte));
# endif
int socket          args((int domain, int type, int protocol));
#endif

#if defined(ultrix)
int accept          args((int s, struct sockaddr * addr, int *addrlen));
int bind            args((int s, struct sockaddr * name, int namelen));
void bzero          args((char *b, int length));
int close           args((int fd));
int getpeername     args((int s, struct sockaddr * name, int *namelen));
int getsockname     args((int s, struct sockaddr * name, int *namelen));
int gettimeofday    args((struct timeval * tp, struct timezone * tzp));
int listen          args((int s, int backlog));
int read            args((int fd, char *buf, int nbyte));
int select          args((int width, fd_set * readfds, fd_set * writefds, fd_set * exceptfds, struct timeval * timeout));
int setsockopt      args((int s, int level, int optname, void *optval, int optlen));
int socket          args((int domain, int type, int protocol));
int write           args((int fd, char *buf, int nbyte));
#endif

/*
 * Global variables.
 */
DESCRIPTOR_DATA    *d_next;       /* Next descriptor in loop      */
FILE               *fpReserve;    /* Reserved file handle         */
FILE               *fpcomlog = NULL; /* comlog.txt */

bool                merc_down;    /* Shutdown                     */
bool                wizlock;      /* Game is wizlocked            */
bool                nopk;         /* !pkoks can pkill?            */
bool                dbl_xp;
unsigned int        objid;
char                str_boot_time[MAX_INPUT_LENGTH];
char               *last_reboot_by;
time_t              boot_time;
time_t              reboot_time;
time_t              current_time;    /* Time of this pulse           */
time_t              today_time;

char                *global_nocmd = NULL;
char                *global_nospell = NULL;

char                *nameserver = NULL;

int                 pixbonus = 30;
int                 elfbonus = 40;
int                 arenacharm = 0;
int                 save_max_players_t = 0;
int                 save_max_players = 0;
int                 maxdoncount = 20;
int                 avnuminc = 1;
int                 fighttimer = 10;
int                 writeerrno = 0;
bool                nosave = FALSE;

/* port and control moved from local to global for HOTreboot - Flar */
int                 port;
int                 control;
int                 control2;

int                 close_errno = 0;

/* -S- Mod: Some Globals for auctioning an item */
OBJ_DATA           *auction_item;    /* Item being sold      */
CHAR_DATA          *auction_owner;    /* Item's owner         */
CHAR_DATA          *auction_bidder;    /* Last bidder for item     */
int                *auction_bid;    /* Latest price offered     */
int                *auction_reserve;    /* Reserve Price        */
int                *auction_stage;    /* start, 1st, 2nd, gone    */
bool                auction_flop;    /* Update called externally?    */

/* -S- Mod: Globals to handle questing */
bool                quest;        /* Is there a quest running?    */
bool                auto_quest;    /* Quests start on their own?   */
CHAR_DATA          *quest_mob;    /* Mob which started quest      */
CHAR_DATA          *quest_target;    /* Target of the quest      */
char               *quest_target_name;    /* TargetName of the thief      */
OBJ_DATA           *quest_object;    /* Object to recover        */
int                 quest_timer;    /* Time left to get object  */
int                 quest_wait = 0;    /* Min time until next quest    */
sh_int              quest_personality;    /* mob's crusade personality :) */

/* Zen mod: Diplomatics globals */

POL_DATA            politics_data;

/* Llolth added for consider */
char                hr[MAX_STRING_LENGTH];
char                dr[MAX_STRING_LENGTH];

/*
 * OS-dependent local functions.
 */
#if defined(macintosh) || defined(MSDOS)
void game_loop_mac_msdos args((void));
bool read_from_descriptor args((DESCRIPTOR_DATA *d));
bool write_to_descriptor args((int desc, char *txt, int length));
#endif

#if defined(unix)
void game_loop_unix args((int control));
int init_socket     args((int port));
void new_descriptor args((int control));
bool read_from_descriptor args((DESCRIPTOR_DATA *d));
bool write_to_descriptor args((DESCRIPTOR_DATA *d, char *txt, int length));
bool write_to_descriptor_2 args((int desc, char *txt, int length));

void init_descriptor args((DESCRIPTOR_DATA *dnew, int desc));
#endif

/*
 * Other local functions (OS-independent).
 */
bool check_parse_name args((char *name));
bool check_reconnect args((DESCRIPTOR_DATA *d, char *name, bool fConn));
bool check_playing  args((DESCRIPTOR_DATA *d, char *name));
int main            args((int argc, char **argv));
void nanny          args((DESCRIPTOR_DATA *d, char *argument));
bool process_output args((DESCRIPTOR_DATA *d, bool fPrompt));
void read_from_buffer args((DESCRIPTOR_DATA *d));
void stop_idling    args((CHAR_DATA *ch));
void bust_a_prompt  args((DESCRIPTOR_DATA *d, bool preview));
char               *battle_subprompt args((CHAR_DATA *ch, char *out));
char               *note_subprompt args((CHAR_DATA *ch, char *out));
void free_desc      args((DESCRIPTOR_DATA *d));

extern char        *my_left args((char *src, char *dst, int len));
extern void         dns_setup args((void));
extern void dns_exec(char *iphost, bool ip);
extern char *dns_lookup(char *ip);

extern struct dns_setup dns;

/*+ */ int          global_port;

int
main(int argc, char **argv)
{
    struct timeval      now_time;
    bool                fCopyOver = FALSE;    /* HOTreboot??? Well is it...is it???? - Flar */

    extern int          abort_threshold;

    /*  Taken out for HOTreboot
       #if defined(unix)
       int control;
       #endif
     */
    /*
     * Memory debugging if needed.
     */
    /* #if defined(MALLOC_DEBUG)
       malloc_debug( 2 );
       #endif  */

    /*
     * Init time.
     */

    umask(037);

    gettimeofday(&now_time, NULL);
    current_time = (time_t) now_time.tv_sec;
    today_time = get_today_ctime();

    {
        char                logfilename[MSL];

        sprintf(logfilename, "../log/acklog.%s", get_today_string(today_time));

        if (freopen(logfilename, "a", stdout) == NULL)
            FPRINTF(stderr, "ERROR: reopening %s in main()\n", logfilename);

        setlinebuf(stdout);
    }

    strcpy(str_boot_time, ctime(&current_time));
    boot_time = current_time;
    reboot_time = 0;

    /*
     * Macintosh console initialization.
     */
#if defined(macintosh)
    console_options.nrows = 31;
    cshow(stdout);
    csetmode(C_RAW, stdin);
    cecho2file("log file", 1, stderr);
#endif

    /*
     * Reserve one channel for our use.
     */
    if ((fpReserve = fopen(NULL_FILE, "r")) == NULL) {
        perror(NULL_FILE);
        exit(1);
    }

    /*
     * Get the port number.
     */
    port = 1234;
    if (argc > 1) {
        if (!is_number(argv[1])) {
            FPRINTF(stderr, "Usage: %s [port #]\n", argv[0]);
            exit(1);
        }
        else if ((port = atoi(argv[1])) <= 1024) {
            FPRINTF(stderr, "Port number must be above 1024.\n");
            exit(1);
        }
    }
    /* Check for HOTreboot parameter - Flar */
    if (argv[2] && argv[2][0]) {
        fCopyOver = TRUE;
    }

    else {
        fCopyOver = FALSE;
        rename("../log/comlog.old", "../log/comlog.crash");
        rename("../log/comlog.txt", "../log/comlog.old");
    }


    /*
     * Run the game.
     */
#if defined(macintosh) || defined(MSDOS)
    boot_db();
    log_string("Ack is ready to rock.");
    game_loop_mac_msdos();
#endif

    if (!fCopyOver) {            /* We already have the port if Copyovered. */
        control = init_socket(port);
    }
    /*+ */ global_port = port;
    if (fCopyOver)
        abort_threshold = BOOT_DB_ABORT_THRESHOLD;
    init_alarm_handler();
    boot_db(fCopyOver);

    dns.fd = -1;
    dns.bind_port = port; /* this won't interfere because we use UDP for dns, not TCP */
    dns_setup();

    sprintf(log_buf, "ACK! MUD is ready on port %d.", port);
    log_string(log_buf);
    game_loop_unix(control);

    close(control);

    /*
     * That's all, folks.
     */
    log_string("Normal termination of game.");
    exit(0);
    return 0;
}

#if defined(unix)
int
init_socket(int port)
{
    static struct sockaddr_in sa_zero;
    struct sockaddr_in  sa;
    int                 x = 1;
    int                 fd;

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Init_socket: socket");
        exit(1);
    }

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &x, sizeof(x)) < 0) {
        perror("Init_socket: SO_REUSEADDR");
        close(fd);
        exit(1);
    }

#if defined(SO_DONTLINGER) && !defined(SYSV)
    {
        struct linger       ld;

        ld.l_onoff = 1;
        ld.l_linger = 1000;

        if (setsockopt(fd, SOL_SOCKET, SO_DONTLINGER, (char *) &ld, sizeof(ld)) < 0) {
            perror("Init_socket: SO_DONTLINGER");
            close(fd);
            exit(1);
        }
    }
#endif
    sa = sa_zero;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);

    if (bind(fd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
        perror("Init_socket: bind");
        close(fd);
        exit(1);
    }

    if (listen(fd, 3) < 0) {
        perror("Init_socket: listen");
        close(fd);
        exit(1);
    }

    return fd;
}
#endif

int                 cur_hour = -1;

#ifndef BPORT
int                 cur_min = -1;
#endif
int                 max_players = 0;
int                 cur_players = 0;

#if defined(unix)

/* + */
int                 reopen_flag;
void
reopen_socket(int sig)
{
    reopen_flag = 1;
    signal(SIGUSR1, reopen_socket);
}

/* + */

void
game_loop_unix(int control)
{
    static struct timeval null_time;
    struct timeval      last_time;

    signal(SIGPIPE, SIG_IGN);
    /*+ */
    /*  On a SIGUSR1, open and close the control socket (anti-port-locking
     *  thing) -- Spectrum
     */
    reopen_flag = 0;
    signal(SIGUSR1, reopen_socket);

    /*+ */

    gettimeofday(&last_time, NULL);
    current_time = (time_t) last_time.tv_sec;

    /* Main loop */
    while (!merc_down) {
        fd_set              in_set;
        fd_set              out_set;
        fd_set              exc_set;
        DESCRIPTOR_DATA    *d;
        int                 maxdesc;

        /* #if defined(MALLOC_DEBUG)
           if ( malloc_verify( ) != 1 )
           abort( );
           #endif  */

        /*+ */
        /* handle reopening the control socket
         * don't worry about locking here, we assume that SIGUSR1's are
         * relatively rare
         */
        if (reopen_flag) {
            log_string("SIGUSR1 received, reopening control socket");
            close(control);
            control = init_socket(global_port);
            reopen_flag = 0;
        }
        /*+ */

        /*
         * Poll all active descriptors.
         */
        FD_ZERO(&in_set);
        FD_ZERO(&out_set);
        FD_ZERO(&exc_set);
        FD_SET(control, &in_set);
        maxdesc = control;

        for (d = first_desc; d; d = d->next) {
            if ((d->flags && DESC_FLAG_PASSTHROUGH) == 0) {
                maxdesc = UMAX(maxdesc, d->descriptor);
                FD_SET(d->descriptor, &in_set);
                FD_SET(d->descriptor, &out_set);
                FD_SET(d->descriptor, &exc_set);
            }
            else {
                /* Check to see if child process has terminated */
                if (waitpid(d->childpid, NULL, WNOHANG) != 0) {
                    /* Terminated or error */
                    d->childpid = 0;
                    REMOVE_BIT(d->flags, DESC_FLAG_PASSTHROUGH);
                }
            }
        }

        if (dns.fd != -1) {
            FD_SET(dns.fd, &in_set);
            maxdesc = UMAX(maxdesc, dns.fd);
        }

        if (select(maxdesc + 1, &in_set, &out_set, &exc_set, &null_time) < 0) {
            perror("Game_loop: select: poll");
            exit(1);
        }

        /*
         * New connection?
         */

        if (FD_ISSET(control, &in_set))
            new_descriptor(control);

        /*
         * Kick out the freaky folks.
         */
        for (d = first_desc; d != NULL; d = d_next) {
            d_next = d->next;
            if (FD_ISSET(d->descriptor, &exc_set)) {
                FD_CLR(d->descriptor, &in_set);
                FD_CLR(d->descriptor, &out_set);
                if (d->character)
                    save_char_obj(d->character);
                d->outtop = 0;
                close_socket(d);
            }
        }

        /*
         * Process input.
         */
        for (d = first_desc; d != NULL; d = d_next) {
            d_next = d->next;
            d->fcommand = FALSE;

            if (FD_ISSET(d->descriptor, &in_set)) {
                if (d->character != NULL)
                    d->character->timer = 0;
                if (!read_from_descriptor(d)) {
                    FD_CLR(d->descriptor, &out_set);
                    if (d->character != NULL)
                        save_char_obj(d->character);
                    d->outtop = 0;
                    close_socket(d);
                    continue;
                }
            }

            if (d->character != NULL && d->character->wait > 0) {
                --d->character->wait;

                if (d->character == NULL || d->connected != CON_PLAYING)
                    continue;

                if (d->character->pcdata && !IS_NPC(d->character) && !IS_IMMORTAL(d->character))
                    continue;
            }

            read_from_buffer(d);
            if (d->incomm[0] != '\0') {
                d->fcommand = TRUE;
                stop_idling(d->character);
                d->timeout = current_time + 180;    /* spec: stop idling */

                if (d->connected == CON_PLAYING)
                    if (d->showstr_point)
                        show_string(d, d->incomm);
                    else
                        interpret(d->character, d->incomm);
                else
                    nanny(d, d->incomm);

                d->incomm[0] = '\0';
            }
        }

        if (dns.fd != -1 && FD_ISSET(dns.fd, &in_set)) {
            int r;
            unsigned char c[MSL];

            while ((r = read(dns.fd, &c, MSL - 1)) > 0) {
                dns_packet_parse(c, r);
            }

            if (r < 0 && errno != EWOULDBLOCK) {
                close(dns.fd);
                dns.fd = -1;
                dns.retry = current_time + DNS_EXPIRE_SOCKET;
                xlogf("dns socket closed. retrying in %d seconds", DNS_EXPIRE_SOCKET);
            }
            else if (r == 0) {
                log_string("r is 0");
            }
        }

        /*
         * Autonomous game motion.
         */
        update_handler();

        /*
         * Output.
         */
        for (d = first_desc; d != NULL; d = d_next) {
            d_next = d->next;

            /* spec: disconnect people idling on login */
            if (d->connected < 0 && d->timeout < current_time) {
                write_to_descriptor(d, "Login timeout (180s)\n\r", 0);
                close_socket(d);
                continue;
            }

            if ((d->fcommand || d->outtop > 0)
                && FD_ISSET(d->descriptor, &out_set)) {
                if (!process_output(d, TRUE)) {
                    if (d->character != NULL)
                        save_char_obj(d->character);
                    d->outtop = 0;
                    close_socket(d);
                }
            }
        }

        /*
         * Synchronize to a clock.
         * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
         * Careful here of signed versus unsigned arithmetic.
         */
        {
            struct timeval      now_time;
            struct tm          *now_bd_time;
            FILE               *out_file;
            long                secDelta;
            long                usecDelta;

            time_t                temp_time;

            gettimeofday(&now_time, NULL);
            /*      now_bd_time=localtime(&(now_time.tv_sec));  */
            temp_time = (time_t) now_time.tv_sec;
            now_bd_time = localtime(&temp_time);

            if (now_bd_time->tm_hour != cur_hour && !nosave) {
                cur_hour = now_bd_time->tm_hour;
                out_file = fopen("players.num", "a");
                FPRINTF(out_file, "%d %d %i %i %i\n", now_bd_time->tm_year + 1900, now_bd_time->tm_mon + 1, now_bd_time->tm_mday, cur_hour,
                    max_players);
                fclose(out_file);
            }
#ifndef BPORT
            if (now_bd_time->tm_min != cur_min && !nosave) {
                DESCRIPTOR_DATA    *dd;
                int                 current_playing = 0;

                for (dd = first_desc; dd != NULL; dd = dd->next)
                    if (dd->connected == CON_PLAYING)
                        current_playing++;

                cur_min = now_bd_time->tm_min;
                out_file = fopen("cplayers.num", "a");
                if (out_file) {
                    FPRINTF(out_file, "%d %d %i %i %i %i\n",
                        now_bd_time->tm_year + 1900, now_bd_time->tm_mon + 1, now_bd_time->tm_mday, cur_hour, cur_min, current_playing);
                    fclose(out_file);
                }
            }
#endif

            usecDelta = ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
                + 1000000 / PULSE_PER_SECOND;
            secDelta = ((int) last_time.tv_sec) - ((int) now_time.tv_sec);
            while (usecDelta < 0) {
                usecDelta += 1000000;
                secDelta -= 1;
            }

            while (usecDelta >= 1000000) {
                usecDelta -= 1000000;
                secDelta += 1;
            }

            if (secDelta > 0 || (secDelta == 0 && usecDelta > 0)) {
                struct timeval      stall_time;

                stall_time.tv_usec = usecDelta;
                stall_time.tv_sec = secDelta;
                if (select(0, NULL, NULL, NULL, &stall_time) < 0 && errno != EINTR) {
                    perror("Game_loop: select: stall");
                    exit(1);
                }
            }
        }

        gettimeofday(&last_time, NULL);
        current_time = (time_t) last_time.tv_sec;
    }

    return;
}
#endif

void
free_desc(DESCRIPTOR_DATA *d)
{
    DESCRIPTOR_DATA    *sd;

    d->snoop_by = NULL;
    for (sd = first_desc; sd; sd = sd->next)
        if (sd->snoop_by == d)
            sd->snoop_by = NULL;
    if (d->original)
        do_return(d->character, "");
    if (d->character)
        free_char(d->character);
    free_string(d->host);
    free_string(d->ip);
    close(d->descriptor);
    if (d->showstr_head)
        qdispose(d->showstr_head);
    if (d->outbuf)
        dispose(d->outbuf);
}

#if defined(unix)
void
new_descriptor(int control)
{
    static DESCRIPTOR_DATA d_zero;
    char                buf[MSL];
    DESCRIPTOR_DATA    *dnew;

    /*   BAN_DATA *pban;   */
    struct sockaddr_in  sock;

    /*    struct hostent *from;  unused??? */
    int                 desc;
    socklen_t           size;
    BAN_DATA           *pban;

    size = sizeof(sock);
    getsockname(control, (struct sockaddr *) &sock, &size);
    if ((desc = accept(control, (struct sockaddr *) &sock, &size)) < 0) {
        perror("New_descriptor: accept");
        return;
    }

#if defined(SO_SNDBUF)
    {
        int                 a = 65536;

        setsockopt(desc, SOL_SOCKET, SO_SNDBUF, &a, sizeof(a));
    }
#endif

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

    if (fcntl(desc, F_SETFL, FNDELAY) == -1) {
        perror("New_descriptor: fcntl: FNDELAY");
        return;
    }

    /*
     * Cons a new descriptor.
     */
    GET_FREE(dnew, desc_free);
    *dnew = d_zero;
    init_descriptor(dnew, desc);    /* Not sure is this right? */
    /*    *dnew               = d_zero; */
    dnew->descriptor = desc;
    dnew->connected = CON_GET_NAME;
    dnew->showstr_head = NULL;
    dnew->showstr_point = NULL;
    dnew->outsize = 2000;
    /*    dnew->outbuf        = getmem( dnew->outsize );  */
    dnew->flags = 0;
    dnew->childpid = 0;

    size = sizeof(sock);
    if (getpeername(desc, (struct sockaddr *) &sock, &size) < 0) {
        perror("New_descriptor: getpeername");
        dnew->host = str_dup("(unknown)");
        dnew->ip   = str_dup("(unknown)");
    }
    else {
        /*
         * Would be nice to use inet_ntoa here but it takes a struct arg,
         * which ain't very compatible between gcc and system libraries.
         */
        int                 addr;

        addr = ntohl(sock.sin_addr.s_addr);
        sprintf(buf, "%d.%d.%d.%d", (addr >> 24) & 0xFF, (addr >> 16) & 0xFF, (addr >> 8) & 0xFF, (addr) & 0xFF);

        dnew->remote_port = ntohs(sock.sin_port);
        dnew->host = str_dup(dns_lookup(buf));
        dnew->ip = str_dup(buf);
    }

    for (pban = first_ban; pban != NULL; pban = pban->next) {
        if (!str_prefix(pban->name, dnew->host) && !pban->newbie) {
            sprintf(log_buf, "Denying access to %s.", dnew->host);
            monitor_chan(log_buf, MONITOR_BAD);
            write_to_descriptor(dnew, "Your site has been banned from this MUD.\n\r", 0);
            free_desc(dnew);
            PUT_FREE(dnew, desc_free);
            return;
        }
    }

/*
    sprintf(log_buf, "Connection formed from %s (%s).", dnew->host, dnew->ip);
    monitor_chan(log_buf, MONITOR_CONNECT);
*/

    /*
     * Init descriptor data.
     */
    LINK(dnew, first_desc, last_desc, next, prev);

    /* spec: set initial login timeout */
    dnew->timeout = current_time + 180;

    /*
     * Send the greeting.
     */
    {
#ifndef BPORT
        char                buf[MAX_STRING_LENGTH];
#endif
        write_to_buffer(dnew, (char *) compress_will, 0);
#ifndef BPORT
        sprintf(buf, "greeting%d", number_range(0, 2));

        if (!send_help(dnew, buf, HELP_NORMAL, TRUE))
            write_to_buffer(dnew, "Welcome to Adesa. Please enter a name: ", 0);
#else
        write_to_buffer(dnew, "Username: ", 0);
#endif
    }

    return;
}
#endif

void
init_descriptor(DESCRIPTOR_DATA *dnew, int desc)
{
    static DESCRIPTOR_DATA d_zero;

    *dnew = d_zero;
    dnew->descriptor = desc;
    dnew->connected = CON_GET_NAME;
    dnew->showstr_head = NULL;
    dnew->showstr_point = NULL;
    dnew->outsize = 2000;
    dnew->outbuf = getmem(dnew->outsize);
    dnew->flags = 0;
    dnew->childpid = 0;

}

void
close_socket(DESCRIPTOR_DATA *dclose)
{
    CHAR_DATA          *ch;

    if (dclose->outtop > 0)
        process_output(dclose, FALSE);

    if (dclose->snoop_by != NULL) {
        write_to_buffer(dclose->snoop_by, "Your victim has left the game.\n\r", 0);
    }

    {
        DESCRIPTOR_DATA    *d;

        for (d = first_desc; d != NULL; d = d->next) {
            if (d->snoop_by == dclose)
                d->snoop_by = NULL;
        }
    }

    if (dclose->original) {
        if (dclose->character)
            do_return(dclose->character, "");
        else {
            bug("Close_socket: original without ch");
            dclose->character = dclose->original;
            dclose->original = NULL;
        }
    }

    if ((ch = dclose->character) != NULL) {
        if (close_errno <= 0) {
            sprintf(log_buf, "Closing link to %s. (%s)", ch->name, dclose->host);
            log_string(log_buf);
        }
        else {
            sprintf(log_buf, "Closing link to %s. (%s) [%s]", ch->name, dclose->host, strerror(close_errno));
            log_string(log_buf);
        }

        monitor_chan(log_buf, MONITOR_CONNECT);
        if (dclose->connected == CON_PLAYING) {
            DUEL_DATA          *duel;
            DUEL_PLAYER_DATA   *player;

            if (close_errno <= 0)
                act("$n has lost $s link.", ch, NULL, NULL, TO_ROOM);
            else
                act("$n has lost $s link. ($t)", ch, strerror(close_errno), NULL, TO_ROOM);

            ch->desc = NULL;

/* don't do this anymore, allow people to transfer stuff to their linkdead alts.
            if (ch->pcdata && !IS_NPC(ch) && ch->pcdata->trading_room && ch->pcdata->trading_room == ch->in_room)
                trade_abort(ch);
*/
            if (ch->pcdata && !IS_NPC(ch)
                && is_in_duel(ch, DUEL_STAGE_SET)
                && (duel = find_duel(ch))
                && (duel->stage < DUEL_STAGE_GO)
                )
                cancel_duel(duel, ch, DUEL_END_LINKDEAD);

            if (ch->pcdata && !IS_NPC(ch)
                && is_in_duel(ch, DUEL_STAGE_GO)
                && (player = find_duel_player(ch))
                )
                player->linkdead = current_time;
        }
        else {
            free_char(dclose->character);
        }
        /*      stop_fighting( ch, TRUE );
           save_char_obj( ch );
           extract_char( ch, TRUE );  */
    }

    if (d_next == dclose)
        d_next = d_next->next;

    UNLINK(dclose, first_desc, last_desc, next, prev);
    close(dclose->descriptor);
    free_string(dclose->host);
    free_string(dclose->ip);
    if (dclose->outbuf)
        dispose(dclose->outbuf);
    if (dclose->showstr_head)
        qdispose(dclose->showstr_head);
    if (dclose->out_compress) {
        deflateEnd(dclose->out_compress);
        dispose(dclose->out_compress_buf);
        dispose(dclose->out_compress);
    }

    PUT_FREE(dclose, desc_free);

    recalc_playercounts();

    close_errno = 0;
    return;
}

bool
read_from_descriptor(DESCRIPTOR_DATA *d)
{
    int                 iStart;

    /* Hold horses if pending command already. */
    if (d->incomm[0] != '\0')
        return TRUE;

    /* Check for overflow. */
    iStart = strlen(d->inbuf);
    if (iStart >= sizeof(d->inbuf) - 10) {
        sprintf(log_buf, "%s input overflow!", d->host);
        log_string(log_buf);
        sprintf(log_buf, "input overflow by %s (%s)", d->character->name, d->host);
        monitor_chan(log_buf, MONITOR_CONNECT);
        write_to_descriptor(d, "\n\r SPAMMING IS RUDE, BYE BYE! \n\r", 0);
        return FALSE;
    }

    /* Snarf input. */
#if defined(macintosh)
    for (;;) {
        int                 c;

        c = getc(stdin);
        if (c == '\0' || c == EOF)
            break;
        putc(c, stdout);
        if (c == '\r')
            putc('\n', stdout);
        d->inbuf[iStart++] = c;
        if (iStart > sizeof(d->inbuf) - 10)
            break;
    }
#endif

#if defined(MSDOS) || defined(unix)
    for (;;) {
        int                 nRead;

        nRead = read(d->descriptor, d->inbuf + iStart, sizeof(d->inbuf) - 10 - iStart);
        if (nRead > 0) {
            iStart += nRead;
            if (d->inbuf[iStart - 1] == '\n' || d->inbuf[iStart - 1] == '\r')
                break;
        }
        else if (nRead == 0) {
            close_errno = 0;
            return FALSE;
        }
        else if (errno == EWOULDBLOCK)
            break;
        else {
            close_errno = errno;
            return FALSE;
        }
    }
#endif

    d->inbuf[iStart] = '\0';
    return TRUE;
}

/*
 * Transfer one line from input buffer to input line.
 */
void
read_from_buffer(DESCRIPTOR_DATA *d)
{
    int                 i, j, k;

    /*
     * Hold horses if pending command already.
     */
    if (d->incomm[0] != '\0')
        return;

    /*
     * Look for at least one new line.
     */
    for (i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++) {
        if (d->inbuf[i] == '\0')
            return;
    }

    /*
     * Canonical input processing.
     */
    for (i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++) {
        if (k >= MAX_INPUT_LENGTH - 2) {
            write_to_descriptor(d, "Line too long.\n\r", 0);

            /* skip the rest of the line */
            for (; d->inbuf[i] != '\0'; i++) {
                if (d->inbuf[i] == '\n' || d->inbuf[i] == '\r')
                    break;
            }
            d->inbuf[i] = '\n';
            d->inbuf[i + 1] = '\0';
            break;
        }

        if (d->inbuf[i] == '\b' && k > 0)
            --k;
        else if (isascii(d->inbuf[i]) && isprint(d->inbuf[i]))
            d->incomm[k++] = d->inbuf[i];
        else if (d->inbuf[i] == (char) IAC) {
            if (!memcmp(&d->inbuf[i], (char *) compress_do, strlen((char *) compress_do))) {
                i += strlen((char *) compress_do) - 1;
                compressStart(d);
            }
            else if (!memcmp(&d->inbuf[i], (char *) compress_dont, strlen((char *) compress_dont))) {
                i += strlen((char *) compress_dont) - 1;
                compressEnd(d);
            }
        }
    }

    /*
     * Finish off the line.
     */
    if (k == 0)
        d->incomm[k++] = ' ';
    d->incomm[k] = '\0';

    /*
     * Deal with bozos with #repeat 1000 ...
     */
    if (k > 1 || d->incomm[0] == '!') {
        if (d->incomm[0] != '!' && strcmp(d->incomm, d->inlast)) {
            d->repeat = 0;
        }
        else {
            if (++d->repeat >= 30) {
                if (d->connected == CON_PLAYING) {
                    sprintf(log_buf, "%s input spamming! (%s)", d->character->name, d->incomm);
                    log_string(log_buf);
                    monitor_chan(log_buf, MONITOR_CONNECT);
                }
                write_to_descriptor(d, "\n\r***** SHUT UP!! *****\n\r", 0);
                close_socket(d);
                /* old way: strcpy( d->incomm, "quit" ); */
            }
        }
    }

    /*
     * Do '!' substitution.
     */
    if (d->incomm[0] == '!' && d->connected == CON_PLAYING)
        strcpy(d->incomm, d->inlast);
    else
        strcpy(d->inlast, d->incomm);

    /*
     * Shift the input buffer.
     */
    while (d->inbuf[i] == '\n' || d->inbuf[i] == '\r')
        i++;
    for (j = 0; (d->inbuf[j] = d->inbuf[i + j]) != '\0'; j++);
    return;
}

/*
 * Low level output function.
 */
bool
process_output(DESCRIPTOR_DATA *d, bool fPrompt)
{
    extern bool         merc_down;

    /*
     * Bust a prompt.
     */
    if (fPrompt && !merc_down && d->connected == CON_PLAYING) {
        if (d->showstr_point)
            write_to_buffer(d, "[Please type (c)ontinue, (r)efresh, (b)ack, (h)elp, (q)uit, or RETURN]:  ", 0);
        else {
            CHAR_DATA          *ch;

            ch = d->original ? d->original : d->character;
            if (IS_SET(ch->act, PLR_BLANK))
                write_to_buffer(d, "\n\r", 2);

            if (ch->hunting || ch->hunt_obj)
                char_hunt(ch);
            /* Take this sucker out so that i can set dynamic default prompt -Uni */
            /*      if ( IS_SET(ch->act, PLR_PROMPT) ) */
            bust_a_prompt(d, FALSE);

            if (IS_SET(ch->act, PLR_TELNET_GA))
                write_to_buffer(d, (char *) go_ahead_str, 0);
        }
    }
    /*
     * Short-circuit if nothing to write.
     */
    if (d->outtop == 0)
        return TRUE;

    /*
     * Snoop-o-rama.
     */
    if (d->snoop_by != NULL) {
        char                foo[MAX_STRING_LENGTH];
        CHAR_DATA          *snoop_ch;

        snoop_ch = d->original != NULL ? d->original : d->character;
        if (snoop_ch != NULL)
            sprintf(foo, "[SNOOP:%s] ", snoop_ch->name);
        write_to_buffer(d->snoop_by, foo, 0);
        write_to_buffer(d->snoop_by, d->outbuf, d->outtop);
        /* send a \n\r to snoop_by? */
    }

    /*
     * OS-dependent output.
     */
    if (!write_to_descriptor(d, d->outbuf, d->outtop)) {
        d->outtop = 0;
        return FALSE;
    }
    else {
        d->outtop = 0;
        return TRUE;
    }
}

#if 0
/* old percbar */
char               *
percbar(int curhp, int maxhp)
{
    static char         _buf[512];
    char               *buf = _buf;
    char               *barcol;
    char               *textcol;
    char                _percbuf[10];
    char               *percbuf = _percbuf;
    int                 perc, pos, cnt;

    _buf[0] = 0;
    _percbuf[0] = 0;

    if (curhp > 0 && maxhp > 0) {
        perc = URANGE(0, curhp * 100 / maxhp, 100);

        if (perc > 0)
            pos = perc / 10;
        else
            pos = 0;
    }
    else {
        perc = 0;
        pos = 0;
    }

    sprintf(_percbuf, "%d%%", perc);

    if (perc >= 100) {
        barcol = "@@3";
        textcol = "@@r";
    }
    else if (perc >= 90) {
        barcol = "@@3";
        textcol = "@@g";
    }
    else if (perc >= 75) {
        barcol = "@@4";
        textcol = "@@y";
    }
    else if (perc >= 50) {
        barcol = "@@4";
        textcol = "@@g";
    }
    else if (perc >= 30) {
        barcol = "@@5";
        textcol = "@@p";
    }
    else if (perc >= 15) {
        barcol = "@@2";
        textcol = "@@e";
    }
    else {
        barcol = "@@2";
        textcol = "@@g";
    }

    *buf++ = '@';
    *buf++ = '@';
    *buf++ = 'N';

    if (pos) {
        *buf++ = barcol[0];
        *buf++ = barcol[1];
        *buf++ = barcol[2];
    }

    *buf++ = textcol[0];
    *buf++ = textcol[1];
    *buf++ = textcol[2];

    for (cnt = 1; cnt <= 10; cnt++) {
        if (cnt == pos + 1 && pos > 0) {
            *buf++ = '@';
            *buf++ = '@';
            *buf++ = 'N';
            *buf++ = textcol[0];
            *buf++ = textcol[1];
            *buf++ = textcol[2];
        }

        if (cnt == 4)
            *buf++ = percbuf[0];
        else if (cnt == 5)
            *buf++ = percbuf[1];
        else if (cnt == 6 && perc >= 10)
            *buf++ = percbuf[2];
        else if (cnt == 6 && perc < 10)
            *buf++ = ' ';
        else if (cnt == 7 && perc >= 100)
            *buf++ = percbuf[3];
        else if (cnt == 7 && perc < 100)
            *buf++ = ' ';
        else
            *buf++ = ' ';
    }

    *buf = 0;

    return _buf;
}
#endif

char               *
percbar(int curhp, int maxhp, int width)
{
    static char         _buf[MSL];
    char               *buf = _buf;
    char               *barcol;
    char               *textcol;
    char                _centerbuf[MSL];
    char               *centerbuf = _centerbuf;
    int                 perc, pos, cnt, found = 0;

    _buf[0] = 0;
    _centerbuf[0] = 0;

    if (curhp > 0 && maxhp > 0) {
        perc = URANGE(0, (int) ((float) curhp / maxhp * 100.0), 100);

        if (perc > 0)
            pos = perc / 100.0 * width;
        else
            pos = 0;
    }
    else {
        perc = 0;
        pos = 0;
    }

    sprintf(_centerbuf, "%d%%", perc);
    sprintf(_centerbuf, "%s", center_text(_centerbuf, width));

    if (perc >= 100) {
        barcol = "@@3";
        textcol = "@@r";
    }
    else if (perc >= 90) {
        barcol = "@@3";
        textcol = "@@g";
    }
    else if (perc >= 75) {
        barcol = "@@4";
        textcol = "@@y";
    }
    else if (perc >= 50) {
        barcol = "@@4";
        textcol = "@@g";
    }
    else if (perc >= 30) {
        barcol = "@@5";
        textcol = "@@p";
    }
    else if (perc >= 15) {
        barcol = "@@2";
        textcol = "@@e";
    }
    else {
        barcol = "@@2";
        textcol = "@@g";
    }

    *buf++ = '@';
    *buf++ = '@';
    *buf++ = 'N';

    if (pos) {
        *buf++ = barcol[0];
        *buf++ = barcol[1];
        *buf++ = barcol[2];
    }

    *buf++ = textcol[0];
    *buf++ = textcol[1];
    *buf++ = textcol[2];

    for (cnt = 1; cnt <= width; cnt++) {
        if (cnt >= pos + 1 && pos > 0 && found == FALSE) {
            *buf++ = '@';
            *buf++ = '@';
            *buf++ = 'N';
            *buf++ = textcol[0];
            *buf++ = textcol[1];
            *buf++ = textcol[2];
            found = TRUE;
        }

        *buf++ = *centerbuf++;
    }

    *buf++ = '@';
    *buf++ = '@';
    *buf++ = 'N';
    *buf = 0;

    return _buf;
}

char               *
number_comma2(int number, bool use)
{
    static char         buf[64];

    if (use)
        strncpy(buf, number_comma(number), 64);
    else
        sprintf(buf, "%d", number);

    return buf;
}

char *show_exits(CHAR_DATA *ch, char *buf)
{
    EXIT_DATA *pexit;
    char *out = buf;
    char *dirs = "NESWUD";
    int door;

    if (!ch || !ch->in_room)
        return out;

    for (door = 0; door < 6; door++) {
        if ((pexit = ch->in_room->exit[door])) {
            if (!IS_IMMORTAL(ch) && IS_SET(pexit->exit_info, EX_NODETECT) && IS_SET(pexit->exit_info, EX_CLOSED))
                continue;

            if (str_cmp(pexit->keyword, ""))
                continue;

            if (!IS_SET(pexit->exit_info, EX_CLOSED))
                *out++ = dirs[door];
            else if (!IS_NPC(ch) && ch->pcdata->learned[gsn_find_doors] > number_percent()) {
                *out++ = '('; *out++ = dirs[door]; *out++ = ')';
            }
        }
    }

    *out = '\0';
    return buf;
}

/*
 * Bust a prompt (player settable prompt)
 * coded by Morgenes for Aldara Mud
 */
void
bust_a_prompt(DESCRIPTOR_DATA *d, bool preview)
{
    char                buf[MAX_STRING_LENGTH];
    char                buf2[MAX_STRING_LENGTH];
    const char         *str;
    const char         *i = " ";
    char               *point;
    CHAR_DATA          *ch;
    bool                shown_bprompt = FALSE;
    bool                shown_nprompt = FALSE;
    bool                use_comma = FALSE;
    DUEL_DATA          *duel;
    DUEL_WATCHER_DATA  *watcher;

    ch = d->character;

    /* if editing a note, show a REAL simple prompt ;P */
    if (ch->position == POS_WRITING && !IS_NPC(ch) && !preview) {
        write_to_buffer(d, ">", 0);
        return;
    }

    /* If editing, show an 'info-prompt' -S- */
    if (ch->position == POS_BUILDING && !IS_NPC(ch) && !preview) {
        ROOM_INDEX_DATA    *room;
        OBJ_INDEX_DATA     *obj;
        MOB_INDEX_DATA     *mob;
        char                msg[MAX_STRING_LENGTH];    /* Mode */
        char                msg2[MAX_STRING_LENGTH];    /* what */
        char                msg3[MAX_STRING_LENGTH];    /* all */

        if (ch->act_build == ACT_BUILD_NOWT) {
            sprintf(msg, "NONE");
            sprintf(msg2, "@@gUse @@Wredit@@g, @@Wmedit@@g, @@Wmpedit@@g, or @@Woedit@@g to select a mode.");
        }

        if (ch->act_build == ACT_BUILD_REDIT) {
            sprintf(msg, "ROOM");
            if (ch->build_vnum == -1)
                sprintf(msg2, "No vnum set");
            else {
                room = get_room_index(ch->build_vnum);
                if (room != NULL)
                    sprintf(msg2, "@@d[@@g%5d@@d]:@@N %s", ch->build_vnum, room->name);
            }
        }

        if (ch->act_build == ACT_BUILD_OEDIT) {
            sprintf(msg, "OBJECT");
            if (ch->build_vnum == -1)
                sprintf(msg2, "No vnum set");
            else {
                obj = get_obj_index(ch->build_vnum);
                if (obj != NULL)
                    sprintf(msg2, "@@d[@@g%5d@@d]:@@N %s", ch->build_vnum, obj->short_descr);
            }
        }

        if (ch->act_build == ACT_BUILD_MEDIT) {
            sprintf(msg, "MOBILE");
            if (ch->build_vnum == -1)
                sprintf(msg2, "No vnum set");
            else {
                mob = get_mob_index(ch->build_vnum);
                if (mob != NULL)
                    sprintf(msg2, "@@d[@@g%5d@@d]:@@N %s", ch->build_vnum, mob->short_descr);
            }
        }
        if (ch->act_build == ACT_BUILD_MPEDIT) {
            sprintf(msg, "MOBPROG");
            if (ch->build_vnum == -1)
                sprintf(msg2, "No vnum set");
            else {
                mob = get_mob_index(ch->build_vnum);
                if (mob != NULL)
                    sprintf(msg2, "@@d[@@g%5d@@d]:@@N %s", ch->build_vnum, mob->short_descr);
            }
        }

        sprintf(msg3, "\n\r@@N@@g[@@bBUILD: @@y%s@@g] %s@@N\n\r", msg, msg2);
        write_to_buffer(d, msg3, 0);
        return;
    }

    if (!IS_NPC(ch) && !preview && ch->pcdata && (duel = find_watching_duel(ch)) && duel->stage == DUEL_STAGE_GO) {
        CHAR_DATA          *dch, *dvictim;

        for (watcher = duel->first_watcher; watcher != NULL; watcher = watcher->next)
            if (ch == watcher->ch)
                break;

        if (watcher) {
            dch = duel->first_player->ch;
            dvictim = duel->first_player->next->ch;

            if (dch->position == POS_FIGHTING || dvictim->position == POS_FIGHTING) {
                sprintf(buf, "@@d[@@gSPAR@@d] @@g[%s @@N@@d(%s@@N@@d)@@g] ", dch->short_descr, percbar(dch->hit, dch->max_hit, 10));
                sprintf(buf + strlen(buf), "@@g[%s @@N@@d(%s@@N@@d)@@g]@@N\n\r", dvictim->short_descr, percbar(dvictim->hit, dvictim->max_hit, 10));

                send_to_char(buf, ch);
            }
        }
    }

    /* mobs get a default prompt only */
    if (IS_NPC(ch)) {
        sendf(ch, "<%d/%d %d/%d %d/%d>\n\r",
            ch->hit, ch->max_hit,
            ch->mana, ch->max_mana,
            ch->move, ch->max_move);

        return;
    }

    if (!preview && ch->pcdata && ch->pcdata->idlecheck > 0) {
        char dbuf[64];

        if (current_time > ch->pcdata->idlecheck) {
             sendf(ch, "@@e[@@y[@@W[ @@gAn Immortal requests your @@yATTENTION@@g. Please use the @@Wpray@@g command. You should have replied @@W%s @@gago! @@W]@@y]@@e]@@N\n\r", duration(current_time - ch->pcdata->idlecheck, dbuf));
        }
        else {
             sendf(ch, "@@e[@@y[@@W[ @@gAn Immortal requests your @@yATTENTION@@g. Please use the @@Wpray@@g command. You have @@W%s @@gto reply. @@W]@@y]@@e]@@N\n\r", duration(ch->pcdata->idlecheck - current_time, dbuf));
        }
    }

    if (!preview && (!IS_SET(ch->act, PLR_PROMPT) || ch->pcdata->prompt == NULL || ch->pcdata->prompt[0] == '\0')) {
        send_to_char("> ", ch);
        return;
    }

    if (!preview && strpos(ch->pcdata->prompt, "%N") == -1) {
        char                mybuf[MSL];

        if (ch->pcdata && !IS_NPC(ch) && ch->pcdata->noteprompt[0] != '\0')
            send_to_char(note_subprompt(ch, mybuf), ch);

        shown_nprompt = TRUE;
    }

    if (!preview && strpos(ch->pcdata->prompt, "%n") == -1) {
        char                mybuf[MSL];

        if (ch->pcdata && !IS_NPC(ch) && ch->pcdata->battleprompt[0] != '\0')
            send_to_char(battle_subprompt(ch, mybuf), ch);

        shown_bprompt = TRUE;
    }

    if (!preview && ch->pcdata->safetimer > 0) {
        char                mbuf[MSL], dbuf[MSL];

        dbuf[0] = 0;

        sprintf(mbuf, "@@c[@@a[@@W%s@@a]@@c]@@N ", duration(abs(ch->pcdata->safetimer - current_time), dbuf));
        send_to_char(mbuf, ch);
    }

    if (!preview && (duel = find_duel(ch)) && duel->stage == DUEL_STAGE_SET) {
        char dbuf[MSL];

        dbuf[0] = 0;
        sendf(ch, "@@b[@@y[@@W%s@@y]@@b]@@N ", duration(abs(duel->countdown), dbuf));
    }

    if (!preview && IS_IMMORTAL(ch) && ch->level == MAX_LEVEL && first_rename != NULL) {
        RENAME_DATA *rename;
        int renames = 0;

        for (rename = first_rename; rename != NULL; rename = rename->next)
            renames++;

        sendf(ch, "@@N@@c[@@a[@@W%d@@a]@@c]@@N ", renames);
    }

    point = buf;
    str = ch->pcdata->prompt;

    while (*str != '\0') {
        if (*str != '%' && !use_comma) {
            *point++ = *str++;
            continue;
        }
        ++str;

        i = NULL;

        switch (*str) {
            default:
                i = "> ";
                break;
            case ',':
                use_comma = TRUE;
                continue;
                break;
            case 'a':
                if (ch->level < 5)
                    sprintf(buf2, "%d", ch->alignment);
                else
                    sprintf(buf2, "%s", IS_GOOD(ch) ? "good" : IS_EVIL(ch) ? "evil" : "neutral");
                i = buf2;
                break;
            case 'A':
                if (!IS_NPC(ch)) {
                    if (ch->num_messages > 0)
                        sprintf(buf2, "@@e@@f%d@@N", ch->num_messages);
                    else
                        sprintf(buf2, "%d", ch->num_messages);
                }
                else
                    sprintf(buf2, " ");
                i = buf2;
                break;
            case 'c':
                if (!IS_NPC(ch))
                    sprintf(buf2, "\n\r");
                else
                    sprintf(buf2, " ");
                i = buf2;
                break;
            case 'd':
                sprintf(buf2, "%s", number_comma2(ch->num_followers, use_comma));
                i = buf2;
                break;
            case 'D':
                sprintf(buf2, "%s", number_comma2(max_orders(ch), use_comma));
                i = buf2;
                break;
            case 'e':
                sprintf(buf2, "%s", number_comma2(ch->energy, use_comma));
                i = buf2;
                break;
            case 'E':
                sprintf(buf2, "%s", number_comma2(ch->max_energy, use_comma));
                i = buf2;
                break;
            case 'f':
                if (IS_NPC(ch))
                    sprintf(buf2, "0");
                else
                    sprintf(buf2, "%s", number_comma2(ch->pcdata->fighttimer, use_comma));
                i = buf2;
                break;
            case 'g':
                sprintf(buf2, "%s", number_comma2(ch->gold, use_comma));
                i = buf2;
                break;
            case 'h':
                sprintf(buf2, "%s", number_comma2(ch->hit, use_comma));
                i = buf2;
                break;
            case 'H':
                sprintf(buf2, "%s", number_comma2(ch->max_hit, use_comma));
                i = buf2;
                break;
            case 'i':
                if (IS_NPC(ch))
                    break;
                if (IS_IMMORTAL(ch))
                    sprintf(buf2, "INVIS: %d", IS_SET(ch->act, PLR_WIZINVIS) ? ch->invis : 0);
                else {
                    if ((IS_AFFECTED(ch, AFF_INVISIBLE))
                        || (IS_AFFECTED(ch, AFF_HIDE))
                        || (item_has_apply(ch, ITEM_APPLY_INV))
                        || (item_has_apply(ch, ITEM_APPLY_HIDE))) {
                        sprintf(buf2, "%s", "INVIS");
                    }
                    else {
                        sprintf(buf2, "%s", "VIS");
                    }
                }
                i = buf2;
                break;
            case 'j':
                sprintf(buf2, "%s", (ch->max_hit > 0) ? number_comma2(ch->hit * 100 / ch->max_hit, use_comma) : number_comma2(-1, use_comma));
                i = buf2;
                break;
            case 'J':
                sprintf(buf2, "%s", (ch->max_mana > 0) ? number_comma2(ch->mana * 100 / ch->max_mana, use_comma) : number_comma2(-1, use_comma));
                i = buf2;
                break;
            case 'k':
                sprintf(buf2, "%s", (ch->max_move > 0) ? number_comma2(ch->move * 100 / ch->max_move, use_comma) : number_comma2(-1, use_comma));
                i = buf2;
                break;
            case 'K':
                sprintf(buf2, "%s", (ch->max_energy > 0) ? number_comma2(ch->energy * 100 / ch->max_energy, use_comma) : number_comma2(-1, use_comma));
                i = buf2;
                break;
            case 'm':
                sprintf(buf2, "%s", number_comma2(ch->mana, use_comma));
                i = buf2;
                break;
            case 'M':
                sprintf(buf2, "%s", number_comma2(ch->max_mana, use_comma));
                i = buf2;
                break;
            case 'n':
                if (!shown_bprompt && !preview) {
                    char                mybuf[MSL];

                    if (ch->pcdata && !IS_NPC(ch) && ch->pcdata->battleprompt[0] != '\0') {
                        sprintf(buf2, "%s", battle_subprompt(ch, mybuf));
                        i = buf2;
                    }

                    shown_bprompt = TRUE;

                }
                break;
            case 'N':
                if (!shown_nprompt && !preview) {
                    char                mybuf[MSL];

                    if (ch->pcdata && !IS_NPC(ch) && ch->pcdata->noteprompt[0] != '\0') {
                        sprintf(buf2, "%s", note_subprompt(ch, mybuf));
                        i = buf2;
                    }

                    shown_nprompt = TRUE;
                }
                break;
            case 'p':
                if (!IS_NPC(ch)) {
                    if (weather_info.moon_loc == MOON_DOWN)
                        sprintf(buf2, "%s", "DOWN");
                    else if (weather_info.moon_loc == MOON_PEAK)
                        sprintf(buf2, "PEAK:%s", get_moon_phase_name());
                    else
                        sprintf(buf2, "LOW:%s", get_moon_phase_name());
                }
                else
                    sprintf(buf2, " ");
                i = buf2;
                break;
            case 'r':
                if (ch->in_room != NULL)
                    sprintf(buf2, "%s", ch->in_room->name);
                else
                    sprintf(buf2, " ");
                i = buf2;
                break;
            case 'R':
                if (IS_IMMORTAL(ch) && ch->in_room != NULL)
                    sprintf(buf2, "%s", number_comma2(ch->in_room->vnum, use_comma));
                else
                    sprintf(buf2, " ");
                i = buf2;
                break;
            case 's':
                if (!IS_NPC(ch))
                    sprintf(buf2, "%s", stance_app[ch->stance].name);
                else
                    sprintf(buf2, " ");
                i = buf2;
                break;
            case 't':
                if (!IS_NPC(ch))
                    sprintf(buf2, "%d %s", (time_info.hour % 12 == 0) ? 12 : time_info.hour % 12, time_info.hour >= 12 ? "pm" : "am");
                else
                    sprintf(buf2, " ");
                i = buf2;
                break;
            case 'v':
                sprintf(buf2, "%s", number_comma2(ch->move, use_comma));
                i = buf2;
                break;
            case 'V':
                sprintf(buf2, "%s", number_comma2(ch->max_move, use_comma));
                i = buf2;
                break;
            case 'x':
                sprintf(buf2, "%s", number_comma2(ch->exp, use_comma));
                i = buf2;
                break;
            case 'X': {
                char buf3[MIL];

                sprintf(buf2, "%s", show_exits(ch, buf3));
                i = buf2;
                break;
            }
            case 'z':
                if (ch->in_room != NULL)
                    sprintf(buf2, "%s", ch->in_room->area->name);
                else
                    sprintf(buf2, " ");
                i = buf2;
                break;
            case '!':
                sprintf(buf2, "%s", number_comma2(GET_HITROLL(ch), use_comma));
                i = buf2;
                break;
            case '+':
                sprintf(buf2, "%s", number_comma2(GET_DAMROLL(ch), use_comma));
                i = buf2;
                break;
            case '*':
                sprintf(buf2, "%s", number_comma2(GET_AC(ch), use_comma));
                i = buf2;
                break;
            case '%':
                sprintf(buf2, "%%");
                i = buf2;
                break;
        }

        use_comma = FALSE;

        ++str;

        if (i)
            while ((*point = *i) != '\0')
                ++point, ++i;
    }

    write_to_buffer(d, buf, point - buf);
    return;
}

char               *
battle_subprompt(CHAR_DATA *ch, char *out)
{
    char               *buf = out;
    char                buf2[MSL];
    char               *p;
    char               *i;
    char                c;
    CHAR_DATA          *tank;
    CHAR_DATA          *victim;
    int                 len = 0;

    *out = '\0';

    if (IS_NPC(ch) || !ch->pcdata || ch->pcdata->battleprompt[0] == '\0' || !ch->fighting || ch->position != POS_FIGHTING)
        return out;

    tank = ch->fighting->fighting;
    victim = ch->fighting;

    p = ch->pcdata->battleprompt;

    while ((c = *p++) != '\0' && len++ < 512) {
        if (c != '%') {
            *buf++ = c;
            continue;
        }

        c = *p++;
        len++;
        i = NULL;

        switch (c) {
            default:
                break;
            case '\0':
                *buf = '\0';
                return out;
                break;
            case '%':
                *buf++ = '%';
                len++;
                break;
            case 'c':
                *buf++ = '\n';
                *buf++ = '\r';
                len += 2;
                break;
            case 'h':
                sprintf(buf2, "%d", ch->hit);
                i = buf2;
                break;
            case 'H':
                sprintf(buf2, "%d", ch->max_hit);
                i = buf2;
                break;
            case 'n':
                if (tank) {
                    sprintf(buf2, "%s", tank == ch ? "YOU" : PERS(tank, ch));
                    i = buf2;
                }
                break;
            case 'N':
                if (victim) {
                    sprintf(buf2, "%s", victim == ch ? "YOU" : PERS(victim, ch));
                    i = buf2;
                }
                break;
            case 'b':
                if (tank) {
                    sprintf(buf2, "%s", percbar(tank->hit, tank->max_hit, 10));
                    i = buf2;
                }
                break;
            case 'B':
                if (victim) {
                    sprintf(buf2, "%s", percbar(victim->hit, victim->max_hit, 10));
                    i = buf2;
                }
                break;
            case 'm':
                if (!IS_NPC(ch) && IS_IMMORTAL(ch) && victim) {
                    sprintf(buf2, "%s", percbar(victim->mana, victim->max_mana, 10));
                    i = buf2;
                }
                break;
            case 'd':
                if (tank) {
                    sprintf(buf2, "%s", (tank == ch) ? "YOU" : "TANK");
                    i = buf2;
                }
                break;
            case 'o':
            case 'O':
                if ((c == 'o' && tank) || (c == 'O' && victim)) {
                    int                 percent;
                    char                wound[MSL];

                    if (c == 'o') {
                        if (tank->max_hit > 0)
                            percent = tank->hit * 100 / tank->max_hit;
                        else
                            percent = -1;
                    }
                    else {
                        if (victim->max_hit > 0)
                            percent = victim->hit * 100 / victim->max_hit;
                        else
                            percent = -1;
                    }

                    if (percent >= 100)
                        sprintf(wound, "@@rGroovy@@N");
                    else if (percent >= 90)
                        sprintf(wound, "@@GScuffed@@N");
                    else if (percent >= 75)
                        sprintf(wound, "@@yStill Kick'n@@N");
                    else if (percent >= 50)
                        sprintf(wound, "@@bTrashed@@N");
                    else if (percent >= 30)
                        sprintf(wound, "@@mLeaking Stuff@@N");
                    else if (percent >= 15)
                        sprintf(wound, "@@RA GutPile@@N");
                    else if (percent >= 0)
                        sprintf(wound, "@@eBLOODMUSH!@@N");
                    else
                        sprintf(wound, "Almost dead");

                    sprintf(buf2, "%s", wound);
                    i = buf2;
                }

                break;

            case 'e':
            case 'E':
            {
                char                t, u;
                char                all[3];
                int                 w;

                if ((c == 'e' && tank) || (c == 'E' && victim)) {
                    if ((t = *p++) == '\0' || !isdigit(t)) {
                        p--;
                        break;
                    }
                    else if ((u = *p++) == '\0' || !isdigit(t)) {
                        p--;
                        break;
                    }

                    all[0] = t;
                    all[1] = u;
                    all[2] = '\0';
                    w = atoi(all);

                    if (w < 10)
                        w = 10;
                    else if (w > 60)
                        w = 60;

                    if (c == 'e')
                        sprintf(buf2, "%s", percbar(tank->hit, tank->max_hit, w));
                    else
                        sprintf(buf2, "%s", percbar(victim->hit, victim->max_hit, w));

                    i = buf2;
                }
            }
            case 'f':
            case 'F':
            {
                char                t, u;
                char                all[3];
                char               *you = "YOU";
                char               *x;
                int                 w;
                int                 space, d;

                if ((c == 'f' && tank) || (c == 'F' && victim)) {
                    if ((t = *p++) == '\0' || !isdigit(t)) {
                        p--;
                        break;
                    }
                    else if ((u = *p++) == '\0' || !isdigit(t)) {
                        p--;
                        break;
                    }

                    all[0] = t;
                    all[1] = u;
                    all[2] = '\0';
                    w = atoi(all);

                    if (w < 10)
                        w = 10;
                    else if (w > 60)
                        w = 60;

                    if (c == 'f') {
                        x = tank == ch ? you : PERS(tank, ch);
                        my_left(x, buf2, w);
                        space = w - my_strlen(buf2);
                        for (d = 0; d < space; d++) {
                            safe_strcat(MSL, buf2, " ");
                        }
                    }
                    else {
                        x = victim == ch ? you : PERS(victim, ch);
                        my_left(x, buf2, w);
                        space = w - my_strlen(buf2);
                        for (d = 0; d < space; d++) {
                            safe_strcat(MSL, buf2, " ");
                        }
                    }

                    i = buf2;
                }
            }

                break;
        }

        if (i)
            while ((c = *i++) != '\0' && len++ < 512)
                *buf++ = c;
    }

    *buf = '\0';

    return out;
}

char               *
note_subprompt(CHAR_DATA *ch, char *out)
{
    char               *buf = out;
    char                buf2[MSL];
    char                c;
    char               *p;
    char               *i;
    int                 unread = 0;
    NOTE_DATA          *note;
    int                 len = 0;

    *out = '\0';

    if (IS_NPC(ch) || !ch->pcdata || ch->pcdata->noteprompt[0] == '\0')
        return out;

    p = ch->pcdata->noteprompt;

    for (note = first_note; note != NULL; note = note->next) {
        /* if the note is not to them and they don't have a specialname */
        if (str_cmp(note->to, ch->name) && !str_cmp(ch->name, ch->pcdata->origname))
            continue;

        /* they have a specialname and the note isn't to either of their "names" */
        if (str_cmp(ch->name, ch->pcdata->origname) && str_cmp(note->to, ch->name) && str_cmp(note->to, ch->pcdata->origname))
            continue;

        if (note->unread)
            unread++;
    }

    if (unread == 0)
        return out;

    while ((c = *p++) != '\0' && len++ < 512) {
        if (c != '%') {
            *buf++ = c;
            continue;
        }

        c = *p++;
        len++;
        i = NULL;

        switch (c) {
            default:
                break;
            case '\0':
                *buf = '\0';
                return out;
                break;
            case '%':
                *buf++ = '%';
                len++;
                break;
            case 'c':
                *buf++ = '\n';
                *buf++ = '\r';
                len += 2;
                break;
            case 'a':
                sprintf(buf2, "%d", unread);
                i = buf2;
                break;
            case 's':
                if (unread > 1) {
                    *buf++ = 's';
                    len++;
                }
                break;
            case 'S':
                if (unread > 1) {
                    *buf++ = 'S';
                    len++;
                }
                break;
        }

        if (i)
            while ((c = *i++) != '\0' && len++ < 512)
                *buf++ = c;
    }

    *buf = '\0';

    return out;
}

/*
 * Append onto an output buffer.
 */

#define COLOUR_MARGIN 20

void
write_to_buffer(DESCRIPTOR_DATA *d, const char *txt, int length)
{
    if (d == NULL)
        return;

    if (d->outbuf == NULL) {
        bugf("write_to_buffer with NULL outbuf, string=%s", txt);
        return;
    }

    /*
     * Find length in case caller didn't.
     */
    if (length <= 0)
        length = strlen(txt);

    /*
     * Initial \n\r if needed.
     */
    if (d->outtop == 0 && !d->fcommand) {
        d->outbuf[0] = '\n';
        d->outbuf[1] = '\r';
        d->outtop = 2;
    }

    /*
     * Expand the buffer as needed.
     */
    while (d->outtop + length + 1 + COLOUR_MARGIN >= d->outsize) {
        char               *outbuf;

        outbuf = getmem(2 * d->outsize);
        strncpy(outbuf, d->outbuf, d->outtop);
        dispose(d->outbuf);
        d->outbuf = outbuf;
        d->outsize *= 2;
    }

    /*
     * Copy.
     */

    /* ONLY COPY length number of Bytes, otherwise things can go WRONG */
    /* MAG mod to strncpy */
    /* Old was  strcpy( d->outbuf + d->outtop, txt ); */

    /* Before Mod2: strncpy( d->outbuf + d->outtop, txt, length ); */

    /* MAG Mod2: Convert Colour strings, while copying. Also expand buffer as neede. */
    {
        char                c;
        char                lookup;
        char               *dest;
        int                 count = length;
        CHAR_DATA          *ch;
        char               *colstr;
        int                 collen, cnt;
        bool                rawcol = FALSE;

        dest = d->outbuf + d->outtop;
        ch = d->original != NULL ? d->original : d->character;
        if (ch && d->connected == CON_PLAYING && ch->pcdata && !IS_NPC(ch) && IS_IMMORTAL(ch) && IS_SET(ch->pcdata->monitor, MONITOR_RAWCOL))
            rawcol = TRUE;

        for (; count > 0;) {
            c = *(txt++);
            if (c != '@' || rawcol) {
                *(dest++) = c;
                count--;
                continue;
            }
            else {
                if (*txt != '@') {
                    *(dest++) = c;
                    count--;
                    continue;
                }

                txt++;            /* txt now points at colour code. */
                c = *(txt++);    /* c is colour code. */
                length = length - 3;
                count = count - 3;

                if (c == '@') {
                    length++;
                    *(dest++) = c;
                    continue;
                }

                ch = d->original != NULL ? d->original : d->character;
                if (ch != NULL && !IS_SET(ch->act, PLR_COLOUR) && c != '-' && c != '_')
                    continue;
                /* set to default highlight or dim set by player */
                lookup = c;
                if (ch != NULL && !IS_NPC(ch)) {    /* shouldn't happen, but... */
                    if (lookup == '!')
                        lookup = ch->pcdata->hicol;
                    else if (lookup == '.')
                        lookup = ch->pcdata->dimcol;
                    else if (lookup == 'k' && IS_SET(ch->config, PLR_SHOWBLACK))
                        lookup = 'd';
                }

                for (cnt = 0; cnt < MAX_ANSI; cnt++)
                    if (ansi_table[cnt].letter == lookup)
                        break;

                /* lsd colour variation! whee! */

                if ((ch != NULL && !IS_NPC(ch)) && (is_affected(ch, gsn_lsd))) {
                    bool                cntgood = FALSE;

                    while (cntgood == FALSE) {
                        cnt = number_range(1, 16);
                        switch (cnt) {
                            case 7:
                            case 9:
                            case 10:
                                cntgood = FALSE;
                                break;
                            default:
                                cntgood = TRUE;
                                break;
                        }
                    }
                }

                if (lookup == '_') {
                    colstr = "@@";
                    collen = 2;
                }
                else if (lookup == '-') {
                    colstr = "@";
                    collen = 1;
                }
                else if (cnt == MAX_ANSI) {
                    colstr = ansi_table[10].value;
                    collen = ansi_table[10].stlen;
                }
                else {
                    colstr = ansi_table[cnt].value;
                    collen = ansi_table[cnt].stlen;
                }

                while (d->outtop + length + collen + 1 >= d->outsize) {
                    char               *outbuf;

                    outbuf = getmem(2 * d->outsize);
                    strncpy(outbuf, d->outbuf, d->outtop + length - count);
                    dispose(d->outbuf);
                    d->outbuf = outbuf;
                    d->outsize *= 2;
                }

                dest = d->outbuf + d->outtop + length - count;
                strncpy(dest, colstr, collen);
                dest += collen;

                length = length + collen;
            }
        }
    }

    /* Make sure we have a \0 at the end */
    *(d->outbuf + d->outtop + length) = '\0';

    d->outtop += length;
    return;
}

/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */

bool
write_to_descriptor(DESCRIPTOR_DATA *d, char *txt, int length)
{

    if (length <= 0)
        length = strlen(txt);

    if (d->out_compress)
        return writeCompressed(d, txt, length);
    else
        return write_to_descriptor_2(d->descriptor, txt, length);
}

bool
write_to_descriptor_2(int desc, char *txt, int length)
{
    int                 iStart;
    int                 nWrite;
    int                 nBlock;

#if defined(macintosh) || defined(MSDOS)
    if (desc == 0)
        desc = 1;
#endif

    if (length <= 0)
        length = strlen(txt);

    for (iStart = 0; iStart < length; iStart += nWrite) {
        nBlock = UMIN(length - iStart, 4096);
        if ((nWrite = write(desc, txt + iStart, nBlock)) < 0) {
            close_errno = errno;
            return FALSE;
        }
    }

    return TRUE;
}

void
show_menu_to(DESCRIPTOR_DATA *d)
{
    CHAR_DATA          *ch = d->character;
    char                buf[MAX_STRING_LENGTH];
    char                menu[MAX_STRING_LENGTH];

    sprintf(menu, "\n\rCharacter Creation Menu.\n\r\n\r");
    strcat(menu,
        "(NOTE): Please note that character creation is @@eNOT@@N stock ACK.\n\r        Please type @@y@@fCHANGES@@N for a list of modifications.\n\r\n\r");
    strcat(menu, "Options:\n\r");

    sprintf(buf, "        1. Set Gender.       Currently:%s\n\r",
        !IS_SET(d->check, CHECK_SEX) ? "Not Set." : ch->sex == SEX_NEUTRAL ? "Neutral." : ch->sex == SEX_MALE ? "Male." : "Female.");
    strcat(menu, buf);

    sprintf(buf, "        2. Set Race.         Currently:%s\n\r", !IS_SET(d->check, CHECK_RACE) ? "Not Set." : race_table[ch->race].race_title);
    strcat(menu, buf);

    strcat(menu, "        3. Set Attributes.  Currently:");
    if (IS_SET(d->check, CHECK_STATS))
        sprintf(buf, "\n\r        Str[%d] Int[%d] Wis[%d] Dex[%d] Con[%d] (Total: %d)\n\r",
            ch->pcdata->max_str, ch->pcdata->max_int, ch->pcdata->max_wis, ch->pcdata->max_dex, ch->pcdata->max_con,
            ch->pcdata->max_str + ch->pcdata->max_int + ch->pcdata->max_wis + ch->pcdata->max_dex + ch->pcdata->max_con);
    else
        sprintf(buf, "Not Set.\n\r");

    strcat(menu, buf);

    strcat(menu, "        4. Set Class Order.  Currently:");
    if (IS_SET(d->check, CHECK_CLASS)) {
        int                 fubar;

        sprintf(buf, "\n\r        ");
        for (fubar = 0; fubar < MAX_CLASS; fubar++) {
            strcat(menu, class_table[ch->pcdata->order[fubar]].who_name);
            strcat(menu, ". ");
        }
        strcat(menu, "\n\r");
    }
    else
        strcat(menu, "Not Set.\n\r");

    strcat(menu, "        5. Exit Creation Process.\n\r");

    strcat(menu, "\n\rPlease Select 1-5: ");
    write_to_buffer(d, menu, 0);
    return;
}

void
show_smenu_to(DESCRIPTOR_DATA *d)
{
    CHAR_DATA          *ch = d->character;
    char                buf[MAX_STRING_LENGTH];
    char                menu[MAX_STRING_LENGTH];

    sprintf(menu, "\n\rCharacter Creation: Gender.\n\r\n\r");

    strcat(menu, "Please Select:\n\r");
    strcat(menu, "              M : Male.\n\r");
    strcat(menu, "              F : Female.\n\r");
    strcat(menu, "              N : Neutral.\n\r\n\r");

    if (IS_SET(d->check, CHECK_SEX))
        sprintf(buf, "Current Choice: %s\n\r", ch->sex == SEX_NEUTRAL ? "Neutral." : ch->sex == SEX_MALE ? "Male." : "Female.");
    else
        sprintf(buf, "No Current Selection.\n\r");

    strcat(menu, buf);
    strcat(menu, "\n\rPlease Select M/F/N: ");

    write_to_buffer(d, menu, 0);
    return;
}

void
show_rmenu_to(DESCRIPTOR_DATA *d)
{
    char                menu[MAX_STRING_LENGTH];
    char                buf[MAX_STRING_LENGTH];
    int                 iRace;

    sprintf(menu, "\n\rCharacter Creation: Race.\n\r\n\r");
    strcat(menu, "Notes: a) Race determines abilities in different classes.\n\r");
    strcat(menu, "       b) Race determines your attributes.\n\r\n\r");
    strcat(menu, "Abr   Name        Str Int Wis Dex Con\n\r");
    strcat(menu, "---   ----        --- --- --- --- ---\n\r");

    for (iRace = 0; iRace < MAX_RACE; iRace++) {
        sprintf(buf, "%3s   %-10s", race_table[iRace].race_name, race_table[iRace].race_title);
        strcat(menu, buf);

        sprintf(buf, "  %-2d  %-2d  %-2d  %-2d  %-2d\n\r",
            race_table[iRace].race_str,
            race_table[iRace].race_int, race_table[iRace].race_wis, race_table[iRace].race_dex, race_table[iRace].race_con);
        strcat(menu, buf);
    }

    strcat(menu, "\n\rPlease Select Your Race (Abr): ");
    write_to_buffer(d, menu, 0);
    return;
}

void
show_amenu_to(DESCRIPTOR_DATA *d)
{
    CHAR_DATA          *ch = d->character;
    char                menu[MAX_STRING_LENGTH];
    int                 total = 0;

    /* Make the 'rolls', set ch->max_*, and display */
    if (!IS_SET(d->check, CHECK_RACE)) {
        sprintf(menu, "\n\rYou must select a race first.\n\r");
        write_to_buffer(d, menu, 0);
        d->connected = CON_MENU;
        show_menu_to(d);
        return;
    }

    total = ch->pcdata->max_str + ch->pcdata->max_int + ch->pcdata->max_wis + ch->pcdata->max_dex + ch->pcdata->max_con;

    sprintf(menu, "@@N@@gCharacter Creation: Attributes\n\r\n\rStr@@d[@@a%2d@@d] @@gInt@@d[@@a%2d@@d] @@gWis@@d[@@a%2d@@d] @@gDex@@d[@@a%2d@@d] @@gCon@@d[@@a%2d@@d] @@g(Total: @@y%2d@@g, Left to spend: @@e%2d@@g)\n\r\n\r@@NCommands: Str-, Str+, Int-, Int+, Wis-, Wis+, Dex-, Dex+, Con-, Con+\n\ror: A to Accept, H for help: ",
        ch->pcdata->max_str, ch->pcdata->max_int, ch->pcdata->max_wis, ch->pcdata->max_dex, ch->pcdata->max_con,
        total, 90 - total);

    write_to_buffer(d, menu, 0);
    return;
}

void
show_ahelp_menu_to(DESCRIPTOR_DATA *d)
{

    char                menu[MAX_STRING_LENGTH];

    sprintf(menu, "%s", "");
    strcat(menu, "Str affects items you can wear and weight you can carry, and your hitroll and damroll.\n\r");
    strcat(menu, "Int affects your mana gain, how many Npcs you can control effectively, and spell success.\n\r");
    strcat(menu, "Wis affects how many practices you get, your mana, and your saving against spells.\n\r");
    strcat(menu, "Dex affects your ac, how many items you can carry, and your ability to dodge.\n\r");
    strcat(menu, "Con affects how many hitpoints you gain per level.\n\r");
    strcat(menu, "\n\rPlease Select: (A)ccept, return to menu, (H)help stats, or (R)eroll: ");

    write_to_buffer(d, menu, 0);
    return;
}

void
show_cmenu_to(DESCRIPTOR_DATA *d)
{
    char                menu[MAX_STRING_LENGTH];
    char                buf[MAX_STRING_LENGTH];
    int                 iClass;

    sprintf(menu, "Character Creation: Class Order.\n\r\n\r");
    strcat(menu, "This option allows you to select the order of your classes.\n\r");
    strcat(menu, "Being a MultiClass Mud, this order is very important, as it\n\r");
    strcat(menu, "will determine how easily you progress in each class, and\n\r");
    strcat(menu, "how well you can use the skills/spells of each class.\n\r");
    strcat(menu, "There are five classes.  Please list, in order of best to\n\r");
    strcat(menu, "worst, the order your classes will be.\n\r");
    strcat(menu, "(The 1st you pick will be your prime class, gaining a +1 bonus.\n\r");
    strcat(menu, "For example, psi mag cle thi war.\n\r");
    strcat(menu, "Abr    Prime Atr    Name\n\r");
    strcat(menu, "---    ---------    ----\n\r");

    for (iClass = 0; iClass < MAX_CLASS; iClass++) {
        sprintf(buf, "%3s    %3s    %-10s\n\r", class_table[iClass].who_name, class_table[iClass].attr, class_table[iClass].class_name);
        strcat(menu, buf);
    }
    strcat(menu, "\n\rOrder: ");
    write_to_buffer(d, menu, 0);
    return;
}

/*
 * Deal with sockets that haven't logged in yet.
 */
void
nanny(DESCRIPTOR_DATA *d, char *argument)
{
    char                buf[MAX_STRING_LENGTH];
    char                msg[MAX_STRING_LENGTH];
    char                md5buf[33];
    CHAR_DATA          *ch;
    char               *pwdnew;
    int                 iClass;
    bool                fOld;
    int                 cnt;

    while (isspace(*argument))
        argument++;

    ch = d->character;

    if (d->connected == CON_GET_NAME) {
        char                _arg1[MSL];
        char               *arg1 = _arg1;

        if (argument[0] == '\0') {
            close_socket(d);
            return;
        }

        while (*argument != '\0') {
            if (*argument == ' ') {
                argument++;
                break;
            }
            else
                *arg1++ = *argument++;
        }

        *arg1 = '\0';

        _arg1[0] = UPPER(_arg1[0]);

        sprintf(buf, "%s provided as name from login from site %s.", _arg1, d->host);
        monitor_chan(buf, MONITOR_CONNECT);

        if (!check_parse_name(_arg1)) {
            sprintf(buf, "Illegal name %s from site %s.", _arg1, d->host);
            monitor_chan(buf, MONITOR_CONNECT);
            write_to_buffer(d, "Illegal name, try another.\n\rName: ", 0);
            return;
        }

        fOld = load_char_obj(d, _arg1, FALSE);
        ch = d->character;

        if (IS_SET(ch->act, PLR_DENY)) {
            sprintf(log_buf, "Denying access to %s@%s.", arg1, d->host);
            log_string(log_buf);
            monitor_chan(log_buf, MONITOR_CONNECT);
            write_to_buffer(d, "You are denied access.\n\r", 0);
            close_socket(d);
            return;
        }

        if (check_reconnect(d, arg1, FALSE)) {
            fOld = TRUE;
        }
        else {
            if (wizlock && !IS_HERO(ch) && !ch->wizbit) {
#ifndef BPORT
                write_to_buffer(d, "\n\r             " mudnamenocolor " IS CURRENTLY WIZLOCKED.\n\r", 0);
                write_to_buffer(d, "Please Try Connecting Again In A Little While, When Any Problems\n\r", 0);
                write_to_buffer(d, "       We Are Working On Have Been Solved.  Thank You.\n\r", 0);
#else
                write_to_buffer(d, "Access denied.\n\r", 0);
#endif
                close_socket(d);
                return;
            }
            if (check_playing(d, ch->name))
                return;
        }

        if (fOld) {
            BAN_DATA           *pban;

            for (pban = first_ban; pban != NULL; pban = pban->next) {
                if (!str_prefix(pban->name, d->host) && (pban->newbie == FALSE)) {
                    char                buf[MAX_STRING_LENGTH];

                    sprintf(buf, "Denying access to banned site %s", d->host);
                    monitor_chan(buf, MONITOR_CONNECT);
                    write_to_descriptor(d, "Your site has been banned from this Mud.  BYE BYE!\n\r", 0);
                    d->connected = CON_QUITTING;
                    close_socket(d);
                    return;
                }
            }

            /* Old player */
            if (*argument != '\0' && strcmp(argument, "CHALLENGE") == 0 && strlen(ch->pcdata->pwd) == 32) {
                int                 cnt;
                char                chalbuf[MSL];

                for (cnt = 0; cnt < MSL - 1; cnt++)
                    chalbuf[cnt] = (char) number_range(1, 255);

                chalbuf[MSL - 1] = 0;

                strcpy(d->challenge, md5string(chalbuf, md5buf));

                write_to_buffer(d, "Password (CHALLENGE=", 0);
                write_to_buffer(d, d->challenge, 0);
                write_to_buffer(d, "):\n\r", 0);
                write_to_buffer(d, (char *) echo_off_str, 0);
                d->connected = CON_GET_OLD_PASSWORD;
            }
            else {
                write_to_buffer(d, "Password: ", 0);
                write_to_buffer(d, (char *) echo_off_str, 0);
                memset(d->challenge, 0, 33);
                d->connected = CON_GET_OLD_PASSWORD;
            }

            return;
        }
        else {
            BAN_DATA           *pban;

            /* New player */
            /* New characters with same name fix by Salem's Lot */
            if (check_playing(d, ch->name))
                return;

            for (pban = first_ban; pban != NULL; pban = pban->next) {
                if (!str_prefix(pban->name, d->host)) {
                    char                buf[MAX_STRING_LENGTH];

                    sprintf(buf, "Denying access to banned site %s", d->host);
                    monitor_chan(buf, MONITOR_CONNECT);
                    write_to_descriptor(d, "Your site has been banned from this Mud.  BYE BYE!\n\r", 0);
                    d->connected = CON_QUITTING;
                    close_socket(d);
                    return;
                }
            }

            sprintf(buf, "Did I get that right, %s (Y/N)? ", _arg1);
            write_to_buffer(d, buf, 0);
            d->connected = CON_CONFIRM_NEW_NAME;
            return;
        }
    }

    if (d->connected == CON_GET_OLD_PASSWORD) {
#if defined(unix)
        write_to_buffer(d, "\n\r", 2);
#endif
        if (d->challenge[0] == '\0') {
            char                buf1[MSL];
            char                buf2[MSL];

            if (strlen(ch->pcdata->pwd) < 32) {
                strcpy(buf1, crypt(argument, ch->pcdata->pwd));
                strcpy(buf2, ch->pcdata->pwd);
            }
            else {
                strcpy(buf1, md5string(argument, md5buf));
                strcpy(buf2, ch->pcdata->pwd);
            }

            if (strcmp(buf1, buf2)) {
                write_to_buffer(d, "Wrong password.\n\r", 0);
                sprintf(buf, "FAILED LOGIN for %s from site %s.", ch->name, d->host);
                monitor_chan(buf, MONITOR_CONNECT);
                log_string(buf);
                ch->pcdata->failures++;
                save_char_obj(ch);
                close_socket(d);
                return;
            }
            else if (strlen(buf1) < 32) {
                if (ch->pcdata->pwd)
                    free_string(ch->pcdata->pwd);
                ch->pcdata->pwd = str_dup(md5string(argument, md5buf));

                write_to_buffer(d, "Your password algorithm has changed from DES to MD5. See help md5 for information.\n\r", 0);
            }
        }
        else {
            char                buf1[MSL];

            sprintf(buf1, "%s%s", d->challenge, ch->pcdata->pwd);
            (void) md5string(buf1, md5buf);

            if (strcmp(argument, md5buf)) {
                write_to_buffer(d, "Challenge failed.\n\r", 0);
                sprintf(buf, "FAILED CHALLENGE for %s from site %s.", ch->name, d->host);
                monitor_chan(buf, MONITOR_CONNECT);
                ch->pcdata->failures++;
                save_char_obj(ch);
                close_socket(d);
                return;
            }
        }

        write_to_buffer(d, (char *) echo_on_str, 0);

        if (check_reconnect(d, ch->name, TRUE))
            return;

        if (check_playing(d, ch->name))
            return;

        if (ch->lvl[ch->class] == -1)
            ch->lvl[ch->class] = ch->level;

        if (IS_HERO(ch)) {
            do_wizhelp(ch, "motd");
        }
        else {
            do_help(ch, "motd");
        }

        d->connected = CON_READ_MOTD;
    }

    if (d->connected == CON_CONFIRM_NEW_NAME) {
        switch (*argument) {
            case 'y':
            case 'Y':

                sprintf(buf,
                    "New character.\n\r\n\rPlease note that we don't tolerate names that are inappropriate and in bad\n\rtaste, although there is no specific theme your name has to abide by.\n\r\n\rGive me a password for %s: %s",
                    ch->name, echo_off_str);
                write_to_buffer(d, buf, 0);
                d->connected = CON_GET_NEW_PASSWORD;
                return;

            case 'n':
            case 'N':
                write_to_buffer(d, "Ok, what IS it, then? ", 0);
                free_char(d->character);
                d->character = NULL;
                d->connected = CON_GET_NAME;
                return;

            default:
                write_to_buffer(d, "Please type Yes or No? ", 0);
                return;
        }
        return;
    }

    if (d->connected == CON_GET_NEW_PASSWORD) {
#if defined(unix)
        write_to_buffer(d, "\n\r", 2);
#endif

        if (strlen(argument) < 5) {
            write_to_buffer(d, "Password must be at least five characters long.\n\rPassword: ", 0);
            return;
        }

        if (strlen(argument) > 64) {
            write_to_buffer(d, "Password must be 64 characters or less.\n\rPassword: ", 0);
            return;
        }

        pwdnew = md5string(argument, md5buf);

        /* not a crypt pass, doesn't apply
           for ( p = pwdnew; *p != '\0'; p++ )
           {
           if ( *p == '~' )
           {
           write_to_buffer( d,
           "New password not acceptable, try again.\n\rPassword: ",
           0 );
           return;
           }
           }
         */

        free_string(ch->pcdata->pwd);
        ch->pcdata->pwd = str_dup(pwdnew);
        write_to_buffer(d, "Please retype password: ", 0);
        d->connected = CON_CONFIRM_NEW_PASSWORD;
        return;
    }

    if (d->connected == CON_CONFIRM_NEW_PASSWORD) {
#if defined(unix)
        write_to_buffer(d, "\n\r", 2);
#endif

        if (strcmp(md5string(argument, md5buf), ch->pcdata->pwd)) {
            write_to_buffer(d, "Passwords don't match.\n\rRetype password: ", 0);
            d->connected = CON_GET_NEW_PASSWORD;
            return;
        }
        write_to_buffer(d, (char *) echo_on_str, 0);
        show_menu_to(d);
        d->connected = CON_MENU;
        return;
    }

    if (d->connected == CON_MENU) {
        int                 number;

        if (!str_cmp(argument, "changes")) {
            write_to_buffer(d, "Races have been given additional and more powerful bonuses. Also, some base\n\r", 0);
            write_to_buffer(d, "stats have been tweaked.\n\r\n\r", 0);
            write_to_buffer(d, "Human     -  Receives +1 to one max stat and +1 to another max stat of\n\r", 0);
            write_to_buffer(d, "             their choice upon reaching Level 15 Adept.\n\r", 0);
            write_to_buffer(d, "Hobbit    -  If a hobbit is 'sneaking', its room->room movement will not be\n\r", 0);
            write_to_buffer(d, "             seen by anyone at any time, regardless of level.\n\r", 0);
            write_to_buffer(d, "Dwarf     -  When dualwielding, they may still use a shield OR a hold item.\n\r", 0);
            write_to_buffer(d, "Elf       -  Less likely to be stunned and grabbed.\n\r", 0);
            write_to_buffer(d, "Gnome     -  Better dispel magic: can bust through high level cloaks, and will\n\r", 0);
            write_to_buffer(d, "             potentially dispel more effects with each successful dispel.\n\r", 0);
            write_to_buffer(d, "Ogre      -  Ogres lose 0xp when they die.\n\r", 0);
            write_to_buffer(d, "Drow      -  Takes less damage from shockshield.\n\r", 0);
            write_to_buffer(d, "Lamia     -  Immune to 'poison' and 'curse'.\n\r", 0);
            write_to_buffer(d, "Dragon    -  Takes less damage from fireshield and cloak:flaming.\n\r", 0);
            write_to_buffer(d, "Centaur   -  Immune to any spells which heat equipment.\n\r", 0);
            write_to_buffer(d, "Titan     -  Cannot be dispelled by charmed mobiles.\n\r", 0);
            write_to_buffer(d, "Pixie     -  It is harder to backstab and circle a Pixie.\n\r", 0);
            write_to_buffer(d, "Minotaur  -  Minotaurs' hits are NOT affected by iceshield (more damage).\n\r", 0);
            write_to_buffer(d, "Troll     -  Boosted hitpoint regen.\n\r\n\r", 0);
            write_to_buffer(d, "Class alignment between mortal and remortal classes has been corrected.\n\r", 0);
            write_to_buffer(d, "They are now linked as follows:\n\r\n\r", 0);
            write_to_buffer(d, "Mag -> Sor\n\r", 0);
            write_to_buffer(d, "Cle -> Mon\n\r", 0);
            write_to_buffer(d, "Thi -> Ass\n\r", 0);
            write_to_buffer(d, "War -> Kni\n\r", 0);
            write_to_buffer(d, "Psi -> Nec\n\r\n\r\n\r", 0);

            show_menu_to(d);
            return;
        }
        if (!is_number(argument)) {
            write_to_buffer(d, "\n\rPlease Enter A Number.\n\r", 0);
            show_menu_to(d);
            return;
        }
        number = atoi(argument);
        if (number < 1 && number > 5) {
            write_to_buffer(d, "\n\rPlease Enter A Number Between 1 And 5.\n\r", 0);
            show_menu_to(d);
            return;
        }

        switch (number) {
            case 1:
                d->connected = CON_GET_NEW_SEX;
                show_smenu_to(d);
                break;
            case 2:
                d->connected = CON_GET_RACE;
                show_rmenu_to(d);
                break;
            case 3:
                d->connected = CON_GET_STATS;
                show_amenu_to(d);
                break;
            case 4:
                d->connected = CON_GET_NEW_CLASS;
                show_cmenu_to(d);
                break;
            case 5:
                if (!IS_SET(d->check, CHECK_SEX) || !IS_SET(d->check, CHECK_CLASS)
                    || !IS_SET(d->check, CHECK_STATS) || !IS_SET(d->check, CHECK_RACE)) {
                    write_to_buffer(d, "ALL Options Must Be Selected First.\n\r", 0);
                    show_menu_to(d);
                    return;
                }
                sprintf(log_buf, "%s@%s new player.", ch->name, d->host);
                log_string(log_buf);
                monitor_chan(log_buf, MONITOR_CONNECT);
                write_to_buffer(d, "\n\r", 2);
                ch->pcdata->pagelen = 20;

                if (ch->short_descr)
                    free_string(ch->short_descr);
                ch->short_descr = str_dup(ch->name);
                if (ch->pcdata->host)
                    free_string(ch->pcdata->host);
                ch->pcdata->host = str_dup(d->host);
                if (ch->pcdata->ip)
                    free_string(ch->pcdata->ip);
                ch->pcdata->ip = str_dup(d->ip);

                do_help(ch, "newun");
                d->connected = CON_READ_MOTD;
                /* Display motd, and all other malarky */
                break;
        }
        return;
    }

    if (d->connected == CON_GET_STATS) {
        int total = ch->pcdata->max_str + ch->pcdata->max_int + ch->pcdata->max_wis + ch->pcdata->max_dex + ch->pcdata->max_con;

        if (is_name(argument, "str- str+ int- int+ wis- wis+ dex- dex+ con- con+")) {
            char *max90 = "Attribute total maximum is 90.\n\r\n\r";
            char *max22 = "22 is the maximum for any individual attribute.\n\r\n\r";

            if      (!str_cmp(argument, "str-")) {
                if (ch->pcdata->max_str <= race_table[ch->race].race_str - 3)
                    write_to_buffer(d, "Str is already at a minimum for this race.\n\r\n\r", 0);
                else
                    ch->pcdata->max_str--;
            }
            else if (!str_cmp(argument, "str+")) {
                if (ch->pcdata->max_str >= race_table[ch->race].race_str + 3)
                    write_to_buffer(d, "Str is already at a maximum for this race.\n\r\n\r", 0);
                else if (total == 90)
                    write_to_buffer(d, max90, 0);
                else if (ch->pcdata->max_str == 22)
                    write_to_buffer(d, max22, 0);
                else
                    ch->pcdata->max_str++;
            }
            else if (!str_cmp(argument, "int-")) {
                if (ch->pcdata->max_int <= race_table[ch->race].race_int - 3)
                    write_to_buffer(d, "Int is already at a minimum for this race.\n\r\n\r", 0);
                else
                    ch->pcdata->max_int--;
            }
            else if (!str_cmp(argument, "int+")) {
                if (ch->pcdata->max_int >= race_table[ch->race].race_int + 3)
                    write_to_buffer(d, "Int is already at a maximum for this race.\n\r\n\r", 0);
                else if (total == 90)
                    write_to_buffer(d, max90, 0);
                else if (ch->pcdata->max_int == 22)
                    write_to_buffer(d, max22, 0);
                else
                    ch->pcdata->max_int++;
            }
            else if (!str_cmp(argument, "wis-")) {
                if (ch->pcdata->max_wis <= race_table[ch->race].race_wis - 3)
                    write_to_buffer(d, "Wis is already at a minimum for this race.\n\r\n\r", 0);
                else
                    ch->pcdata->max_wis--;
            }
            else if (!str_cmp(argument, "wis+")) {
                if (ch->pcdata->max_wis >= race_table[ch->race].race_wis + 3)
                    write_to_buffer(d, "Wis is already at a maximum for this race.\n\r\n\r", 0);
                else if (total == 90)
                    write_to_buffer(d, max90, 0);
                else if (ch->pcdata->max_wis == 22)
                    write_to_buffer(d, max22, 0);
                else
                    ch->pcdata->max_wis++;
            }
            else if (!str_cmp(argument, "dex-")) {
                if (ch->pcdata->max_dex <= race_table[ch->race].race_dex - 3)
                    write_to_buffer(d, "Dex is already at a minimum for this race.\n\r\n\r", 0);
                else
                    ch->pcdata->max_dex--;
            }
            else if (!str_cmp(argument, "dex+")) {
                if (ch->pcdata->max_dex >= race_table[ch->race].race_dex + 3)
                    write_to_buffer(d, "Dex is already at a maximum for this race.\n\r\n\r", 0);
                else if (total == 90)
                    write_to_buffer(d, max90, 0);
                else if (ch->pcdata->max_dex == 22)
                    write_to_buffer(d, max22, 0);
                else
                    ch->pcdata->max_dex++;
            }
            else if (!str_cmp(argument, "con-")) {
                if (ch->pcdata->max_con <= race_table[ch->race].race_con - 3)
                    write_to_buffer(d, "Con is already at a minimum for this race.\n\r\n\r", 0);
                else
                    ch->pcdata->max_con--;
            }
            else if (!str_cmp(argument, "con+")) {
                if (ch->pcdata->max_con >= race_table[ch->race].race_con + 3)
                    write_to_buffer(d, "Con is already at a maximum for this race.\n\r\n\r", 0);
                else if (total == 90)
                    write_to_buffer(d, max90, 0);
                else if (ch->pcdata->max_con == 22)
                    write_to_buffer(d, max22, 0);
                else
                    ch->pcdata->max_con++;
            }

            show_amenu_to(d);
            return;
        }

        switch (argument[0]) {
            case 'A':
            case 'a': {
                if (total > 90) {
                    write_to_buffer(d, "Total is above 90, try reducing an attribute.\n\r", 0);
                    show_amenu_to(d);
                    return;
                }

                if (!IS_SET(d->check, CHECK_STATS))
                    SET_BIT(d->check, CHECK_STATS);
                d->connected = CON_MENU;
                show_menu_to(d);
                break;
            }
            case 'H':
            case 'h':
                show_ahelp_menu_to(d);
                break;
            default:
                write_to_buffer(d, "Enter Str-, Str+, Int-, Int+, Wis-, Wis+, Dex-, Dex+, Con-, Con+, A or H: ", 0);
                break;
        }
        return;
    }

    if (d->connected == CON_GET_NEW_SEX) {
        switch (argument[0]) {
            case 'm':
            case 'M':
                ch->sex = SEX_MALE;
                ch->login_sex = SEX_MALE;
                break;
            case 'f':
            case 'F':
                ch->sex = SEX_FEMALE;
                ch->login_sex = SEX_FEMALE;
                break;
            case 'n':
            case 'N':
                ch->sex = SEX_NEUTRAL;
                ch->login_sex = SEX_NEUTRAL;
                break;
            default:
                write_to_buffer(d, "That's not a sex.\n\rWhat IS your sex? ", 0);
                show_smenu_to(d);
                return;
        }
        write_to_buffer(d, "\n\r\n\r", 0);
        if (!IS_SET(d->check, CHECK_SEX))
            SET_BIT(d->check, CHECK_SEX);
        d->connected = CON_MENU;
        show_menu_to(d);
        return;
    }

    if (d->connected == CON_GET_NEW_CLASS) {
        sh_int              classes[MAX_CLASS];
        sh_int              parity[MAX_CLASS];    /* Nowt to do with parity really */
        sh_int              index[MAX_CLASS];
        char                arg[MAX_STRING_LENGTH];
        int                 cnt;
        int                 foo;
        bool                ok = TRUE;

        /* Parity set to 1 for each class found. */
        for (cnt = 0; cnt < MAX_CLASS; cnt++)
            parity[cnt] = -1;

        for (cnt = 0; cnt < MAX_CLASS; cnt++) {
            argument = one_argument(argument, arg);
            if (arg[0] == '\0') {
                ok = FALSE;
                break;
            }
            for (foo = 0; foo < MAX_CLASS; foo++)
                if (!str_cmp(arg, class_table[foo].who_name)) {
                    classes[cnt] = foo;
                    index[foo] = cnt;
                    parity[foo] = 1;
                    break;
                }
            if (foo == MAX_CLASS) {
                ok = FALSE;
                break;
            }
        }

        /* If 5 unique classes given, parity[cnt] == 1 */
        for (cnt = 0; cnt < MAX_CLASS; cnt++)
            if (parity[cnt] == -1)    /* Then a class was missed */
                ok = FALSE;

        if (!ok) {
            write_to_buffer(d, "Invalid Order... Please Try Again. You must list each class, by abbreviation, such as CLE WAR MAG THI PSI.\n\r", 0);
            show_cmenu_to(d);
            return;
        }

        /* Copy classes across to pcdata */
        for (cnt = 0; cnt < MAX_CLASS; cnt++) {
            ch->pcdata->order[cnt] = classes[cnt];
            ch->pcdata->index[cnt] = index[cnt];
        }

        d->connected = CON_MENU;
        if (!IS_SET(d->check, CHECK_CLASS))
            SET_BIT(d->check, CHECK_CLASS);
        show_menu_to(d);
        return;
    }

    if (d->connected == CON_GET_RACE) {
        for (iClass = 0; iClass < MAX_RACE; iClass++) {
            if (!str_cmp(argument, race_table[iClass].race_name)) {
                ch->race = iClass;
                break;
            }
        }

        if (iClass == MAX_RACE) {
            write_to_buffer(d, "Invalid Choice.\n\r", 0);
            show_rmenu_to(d);
            return;
        }
        if (IS_SET(d->check, CHECK_STATS))
            REMOVE_BIT(d->check, CHECK_STATS);

        /* when they choose a race, set their initial racial attributes,
         * or if they choose a different race, reset them */
        ch->pcdata->max_str = race_table[ch->race].race_str;
        ch->pcdata->max_int = race_table[ch->race].race_int;

        /* wis is the worst, set this to its minimum so people might boost
         * the other attributes instead */
        ch->pcdata->max_wis = race_table[ch->race].race_wis - 3;

        ch->pcdata->max_dex = race_table[ch->race].race_dex;
        ch->pcdata->max_con = race_table[ch->race].race_con;

        if (!IS_SET(d->check, CHECK_RACE))
            SET_BIT(d->check, CHECK_RACE);
        show_menu_to(d);
        d->connected = CON_MENU;
        return;
    }

    if (d->connected == CON_READ_MOTD) {
        /* Prime level idea dropped.  Give ch 1 level in their best class */
        if (ch->level == 0) {
            ch->class = ch->pcdata->order[0];
            ch->lvl[ch->class] = 1;
        }

        LINK(ch, first_char, last_char, next, prev);
        LINK(ch, first_player, last_player, next_player, prev_player);
        d->connected = CON_PLAYING;

        send_to_char("\n\rWelcome to " mudnamecolor ".  May your visit here be ... mutated.\n\r", ch);

        if (ch->level == 0) {
            /* OBJ_DATA *obj; unused */

            switch (class_table[ch->class].attr_prime) {
                case APPLY_STR:
                    ch->pcdata->max_str++;
                    break;
                case APPLY_INT:
                    ch->pcdata->max_int++;
                    break;
                case APPLY_WIS:
                    ch->pcdata->max_wis++;
                    break;
                case APPLY_DEX:
                    ch->pcdata->max_dex++;
                    break;
                case APPLY_CON:
                    ch->pcdata->max_con++;
                    break;
            }

            ch->level = 1;

            /* FIXME: this temp fix for m/c stuff */
            /* All Races get 5 classes now.. */

            ch->lvl[ch->class] = 1;
            for (cnt = 0; cnt < MAX_CLASS; cnt++)
                if (cnt != ch->class)
                    ch->lvl[cnt] = 0;

            ch->exp = 0;
            ch->hit = ch->max_hit;
            ch->mana = ch->max_mana;
            ch->move = ch->max_move;
            ch->pcdata->mana_from_gain = ch->max_mana;
            ch->pcdata->hp_from_gain = ch->max_hit;
            ch->pcdata->move_from_gain = ch->max_move;

            ch->pcdata->clan = 0;    /* no clan */
            sprintf(buf, " needs a new title!");
            set_title(ch, buf);

            {
                char                race_skill[MSL];
                char               *race_skill_list;

                race_skill_list = race_table[ch->race].skill1;
                for (;;) {
                    race_skill_list = one_argument(race_skill_list, race_skill);
                    if (skill_lookup(race_skill) < 0)
                        break;
                    ch->pcdata->learned[skill_lookup(race_skill)] = 101;
                }
            }

            ch->deaf = 0;
            ch->deaf2 = 0;
            ch->pcdata->lastlogint = current_time;
            ch->logon = current_time;
            ch->pcdata->news_last_read = current_time;

            char_to_room(ch, get_room_index(ROOM_VNUM_SCHOOL));
            /* Needed in case newbie drops an item. */

        }
        else if (ch->in_room != NULL) {
            char_to_room(ch, ch->in_room);
        }
        else {
            char_to_room(ch, get_room_index(ROOM_VNUM_LIMBO));
        }

        /* check for login failures, then clear count. */
        if (ch->pcdata->failures != 0 && ch->level != 1) {
            sprintf(msg, "WARNING:  There have been %d failed login attempts.\n\r", ch->pcdata->failures);
            send_to_char(msg, ch);
            ch->pcdata->failures = 0;
        }

        if (ch->level > 1) {

            char                buf[MAX_STRING_LENGTH];
            char               *buf_ptr = buf;

            buf[0] = 0;

            if (ch->pcdata->lastlogint > 0)
                sprintf(msg, "\n\rLast successful login from: %s (%s ago)\n\r\n\r", ch->pcdata->host,
                    duration((current_time - ch->pcdata->lastlogint), buf_ptr));
            else
                sprintf(msg, "\n\rLast successful login from: %s\n\r\n\r", ch->pcdata->host);

            buf[0] = 0;
            send_to_char(msg, ch);

            sprintf(msg, "%s connected. (%s, was %s) [%s]",
                ch->name, d->host, ch->pcdata->host, ch->pcdata->lastlogint > 0 ? duration((current_time - ch->pcdata->lastlogint), buf_ptr)
                : "unknown");
            log_string(msg);
            monitor_chan(msg, MONITOR_CONNECT);

            ch->pcdata->lastlogint = current_time;
            ch->logon = current_time;

            update_cinfo(ch, FALSE);

            if ((ch->level > 80)) {
                sprintf(msg, "WARNING!!! %s logged in with level %d.\n\r", ch->name, ch->level);
                log_string(msg);
            }

            if (ch->pcdata->host != NULL)
                free_string(ch->pcdata->host);
            ch->pcdata->host = str_dup(d->host);
            if (ch->pcdata->ip != NULL)
                free_string(ch->pcdata->ip);
            ch->pcdata->ip = str_dup(d->ip);
        }

        /* new note checking originally done here */

        act("$n enters " mudnamecolor ".", ch, NULL, NULL, TO_ROOM);

        sprintf(buf, "%s has entered the game.", ch->name);
        monitor_chan(buf, MONITOR_CONNECT);

        /*
           if (  ( number_range( 0, 99 ) < ( ch->balance / 10000000 ) ) 
           && ( ch->balance > ( get_pseudo_level( ch ) * 100000 ) ) )
           {
           int  loss;
           int  new_balance;
           loss = number_range( ch->balance * .3, ch->balance * .6 );
           new_balance = UMAX( ch->balance - loss, get_pseudo_level( ch ) * 100000 );
           ch->balance = UMIN( ch->balance, new_balance );
           }
         */

        if (!IS_NPC(ch)) {
            if (!str_cmp(ch->pcdata->room_enter, "")) {
                switch (ch->race) {

                    case 0:
                        ch->pcdata->room_enter = str_dup("saunters arrogantly in from");
                        ch->pcdata->room_exit = str_dup("walks");
                        break;

                    case 1:
                        ch->pcdata->room_enter = str_dup("sneaks in from");
                        ch->pcdata->room_exit = str_dup("disappears");
                        break;

                    case 2:
                        ch->pcdata->room_enter = str_dup("barges into you from");
                        ch->pcdata->room_exit = str_dup("charges");
                        break;

                    case 3:
                        ch->pcdata->room_enter = str_dup("quietly glides in from");
                        ch->pcdata->room_exit = str_dup("glides quietly");
                        break;

                    case 4:
                        ch->pcdata->room_enter = str_dup("appears in from");
                        ch->pcdata->room_exit = str_dup("fades away");
                        break;

                    case 5:
                        ch->pcdata->room_enter = str_dup("stomps angrily in from");
                        ch->pcdata->room_exit = str_dup("angrily stomps");
                        break;
                    case 6:
                        ch->pcdata->room_enter = str_dup("skitters in from");
                        ch->pcdata->room_exit = str_dup("skitters");
                        break;
                    case 7:
                        ch->pcdata->room_enter = str_dup("slithers in from");
                        ch->pcdata->room_exit = str_dup("slithers");
                        break;
                    case 8:
                        ch->pcdata->room_enter = str_dup("flys at you from");
                        ch->pcdata->room_exit = str_dup("flys away");
                        break;
                    case 9:
                        ch->pcdata->room_enter = str_dup("gallops in from");
                        ch->pcdata->room_exit = str_dup("gallops");
                        break;
                    case 10:
                        ch->pcdata->room_enter = str_dup("marches in from");
                        ch->pcdata->room_exit = str_dup("marches");
                        break;
                    case 11:
                        ch->pcdata->room_enter = str_dup("flitters in from");
                        ch->pcdata->room_exit = str_dup("flitters");
                        break;
                    case 12:
                        ch->pcdata->room_enter = str_dup("boars you down from");
                        ch->pcdata->room_exit = str_dup("rushes");
                        break;

                }
            }
        }

        if (ch->pcdata->hp_from_gain < 0)
            reset_gain_stats(ch);
        /*       ch->affected_by = 0;   */

        ch->is_quitting = FALSE;
        d->connected = CON_SETTING_STATS;
        {
            OBJ_DATA           *wear_object;
            AFFECT_DATA        *this_aff;
            AFFECT_DATA        *this_aff_next;

            ch->max_mana = ch->pcdata->mana_from_gain;
            ch->max_hit = ch->pcdata->hp_from_gain;
            ch->max_move = ch->pcdata->move_from_gain;
            ch->saving_throw = get_pseudo_level(ch) / 10;
            ch->hitroll = 0;
            ch->damroll = 0;
            ch->armor = 100;
            if (ch->login_sex != -1)
                ch->sex = ch->login_sex;
            ch->affected_by = 0;

            if (ch->level == 1) {
                wear_object = create_object(get_obj_index(20115), 0);
                if (wear_object) {
                    obj_to_char(wear_object, ch);
                    wear_object->wear_loc = WEAR_LIGHT;
                }
            }

            for (wear_object = ch->first_carry; wear_object != NULL; wear_object = wear_object->next_in_carry_list) {
                if (wear_object->wear_loc > WEAR_NONE)
                    equip_char(ch, wear_object, wear_object->wear_loc);
            }
            /* add spells saved to stats  */

            for (this_aff = ch->first_saved_aff; this_aff != NULL; this_aff = this_aff_next) {
                this_aff_next = this_aff->next;
                UNLINK(this_aff, ch->first_saved_aff, ch->last_saved_aff, next, prev);

                if (this_aff->type == gsn_shield_fire)
                    do_cast(ch, "fireshield");
                else if (this_aff->type == gsn_shield_ice)
                    do_cast(ch, "iceshield");
                else if (this_aff->type == gsn_shield_shock)
                    do_cast(ch, "shockshield");
                else if (this_aff->type == gsn_shield_demon)
                    do_cast(ch, "demonshield");
                else
                    affect_to_char(ch, this_aff);
                PUT_FREE(this_aff, affect_free);
            }
        }

        if (ch->pcdata && ch->pcdata->autostance && ch->pcdata->autostance[0] != '\0')
            do_stance(ch, ch->pcdata->autostance);

        d->connected = CON_PLAYING;

        dns_exec(d->ip, TRUE);
        do_look(ch, "auto");

        {
            NOTE_DATA *note;
            int cnt = 0;

            for (note = first_note; note; note = note->next) {
                /* if the note is not an announcement, continue */
                if (str_cmp(note->to, "announcement"))
                    continue;

                /* old note? don't show it */
                if (ch->pcdata->news_last_read >= note->date_stamp)
                    continue;

                cnt++;
            }

            if (cnt == 1) {
                send_to_char("@@eThere is @@y1@@e unread announcement. Type @@ynews read@@e to read it.@@N\n\r", ch);
            }
            else if (cnt > 1) {
                sendf(ch, "@@eThere are @@y%d@@e unread announcements. Type @@ynews read@@e to read them.@@N\n\r", cnt);
            }
        }

        recalc_playercounts();
        return;
    }

    return;
}

/*
 * Parse a name for acceptability.
 */
bool
check_parse_name(char *name)
{
    /*
     * Reserved words.
     */
    if (is_name(name, "all auto everymob localmobs immortal zen self someone"))
        return FALSE;

    /*
     * Length restrictions.
     */
    if (strlen(name) < 3)
        return FALSE;

#if defined(MSDOS)
    if (strlen(name) > 8)
        return FALSE;
#endif

#if defined(macintosh) || defined(unix)
    if (strlen(name) > 12)
        return FALSE;
#endif

    /*
     * Alphanumerics only.
     * Lock out IllIll twits.
     */
    {
        char               *pc;
        bool                fIll;

        fIll = TRUE;
        for (pc = name; *pc != '\0'; pc++) {
            if (!isalpha(*pc))
                return FALSE;
            if (LOWER(*pc) != 'i' && LOWER(*pc) != 'l')
                fIll = FALSE;
        }

        if (fIll)
            return FALSE;
    }

    /*
     * Prevent players from naming themselves after mobs.
     */
    {
        extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
        MOB_INDEX_DATA     *pMobIndex;
        int                 iHash;

        for (iHash = 0; iHash < MAX_KEY_HASH; iHash++) {
            for (pMobIndex = mob_index_hash[iHash]; pMobIndex != NULL; pMobIndex = pMobIndex->next) {
                if (is_name(name, pMobIndex->player_name))
                    return FALSE;
            }
        }
    }

    return TRUE;
}

/*
 * Look for link-dead player to reconnect.
 */
bool
check_reconnect(DESCRIPTOR_DATA *d, char *name, bool fConn)
{
    CHAR_DATA          *ch;
    OBJ_DATA           *obj;
    DUEL_PLAYER_DATA   *player;

    for (ch = first_char; ch != NULL; ch = ch->next) {
        if (!IS_NPC(ch)
            && (!fConn || ch->desc == NULL)
            && !str_cmp(d->character->name, ch->name)) {
            if (fConn == FALSE) {
                free_string(d->character->pcdata->pwd);
                d->character->pcdata->pwd = str_dup(ch->pcdata->pwd);
            }
            else {
                free_char(d->character);
                d->character = ch;
                ch->desc = d;
                ch->timer = 0;

                send_to_char("Reconnecting.\n\r", ch);
                act("$n reconnects.", ch, NULL, NULL, TO_ROOM);
                sprintf(log_buf, "%s reconnected. (%s, was %s)", ch->name, d->host, ch->pcdata->host);
                log_string(log_buf);
                monitor_chan(log_buf, MONITOR_CONNECT);
                d->connected = CON_PLAYING;
                if (ch->pcdata && !IS_NPC(ch)
                    && is_in_duel(ch, DUEL_STAGE_GO)
                    && (player = find_duel_player(ch))
                    )
                    player->linkdead = 0;

                if (ch->pcdata->host != NULL)
                    free_string(ch->pcdata->host);
                ch->pcdata->host = str_dup(d->host);
                if (ch->pcdata->ip != NULL)
                    free_string(ch->pcdata->ip);
                ch->pcdata->ip = str_dup(d->ip);

                dns_exec(d->ip, TRUE);

                recalc_playercounts();

                /*
                 * Contributed by Gene Choi
                 */
                if ((obj = get_eq_char(ch, WEAR_LIGHT)) != NULL && obj->item_type == ITEM_LIGHT && obj->value[2] != 0 && ch->in_room != NULL)
                    ++ch->in_room->light;
            }
            return TRUE;
        }
    }

    return FALSE;
}

/*
 * Check if already playing.
 */
bool
check_playing(DESCRIPTOR_DATA *d, char *name)
{
    DESCRIPTOR_DATA    *dold;
    char                buf[MAX_STRING_LENGTH];

    for (dold = first_desc; dold; dold = dold->next) {
        if (dold != d
            && dold->character != NULL
            && dold->connected != CON_GET_NAME
            && dold->connected != CON_GET_OLD_PASSWORD && !str_cmp(name, dold->original ? dold->original->name : dold->character->name)) {
            sprintf(buf, "Player from site %s tried to login as %s (already playing) !", d->host, name);
            monitor_chan(buf, MONITOR_CONNECT);

            write_to_descriptor(dold, "Someone else has logged in as you. Goodbye.\n\r", 0);
            close_socket(dold);

            check_reconnect(d, name, TRUE);
            return TRUE;
        }
    }

    return FALSE;
}

void
stop_idling(CHAR_DATA *ch)
{
    if (ch == NULL
        || ch->desc == NULL || ch->desc->connected != CON_PLAYING || ch->was_in_room == NULL || ch->in_room != get_room_index(ROOM_VNUM_LIMBO))
        return;

    ch->timer = 0;
    char_from_room(ch);
    char_to_room(ch, ch->was_in_room);
    ch->was_in_room = NULL;
    act("$n has returned from the void.", ch, NULL, NULL, TO_ROOM);
    return;
}

/*
 * Write to one char.
 */
void
send_to_char(const char *txt, CHAR_DATA *ch)
{
    if (ch == NULL)
        return;
    if (txt == NULL || ch->desc == NULL)
        return;
    /* Large leak fixed here.. -- Altrag */
    if (ch->desc->showstr_head != NULL) {
        char               *ssh;

        ssh = qgetmem(strlen(ch->desc->showstr_head) + strlen(txt) + 1);
        strcpy(ssh, ch->desc->showstr_head);
        strcat(ssh, txt);
        if (ch->desc->showstr_point)
            ch->desc->showstr_point += (ssh - ch->desc->showstr_head);
        else
            ch->desc->showstr_point = ssh;
        qdispose(ch->desc->showstr_head);
        ch->desc->showstr_head = ssh;
    }
    else {
        ch->desc->showstr_head = qgetmem(strlen(txt) + 1);
        strcpy(ch->desc->showstr_head, txt);
        ch->desc->showstr_point = ch->desc->showstr_head;
    }
    if (ch->desc->showstr_point == ch->desc->showstr_head)
        show_string(ch->desc, "");
    return;
}

/* The heart of the pager.  Thanks to N'Atas-Ha, ThePrincedom
   for porting this SillyMud code for MERC 2.0 and laying down the groundwork.
   Thanks to Blackstar, hopper.cs.uiowa.edu 4000 for which
   the improvements to the pager was modeled from.  - Kahn */
/* Leak fixes.. alloc_mem'd stuff shouldnt be free_string'd. -- Altrag */
/* Spec: buffer overflow fixes, internal buffer sizes increased */

void
show_string(struct descriptor_data *d, char *input)
{
    char                buffer[MAX_STRING_LENGTH * 2];
    char                buf[MAX_INPUT_LENGTH];
    register char      *scan, *chk;
    int                 lines = 0, toggle = 1;
    int                 space;

    one_argument(input, buf);

    switch (UPPER(buf[0])) {
        case '\0':
        case 'C':                /* show next page of text */
            lines = 0;
            break;

        case 'R':                /* refresh current page of text */
            lines = -1 - (d->character->pcdata->pagelen);
            break;

        case 'B':                /* scroll back a page of text */
            lines = -(2 * d->character->pcdata->pagelen);
            break;

        case 'H':                /* Show some help */
            write_to_buffer(d, "C, or Return = continue, R = redraw this page,\n\r", 0);
            write_to_buffer(d, "B = back one page, H = this help, Q or other keys = exit.\n\r\n\r", 0);
            lines = -1 - (d->character->pcdata->pagelen);
            break;

        default:                /*otherwise, stop the text viewing */
            if (d->showstr_head) {
                qdispose(d->showstr_head);
                d->showstr_head = 0;
            }
            d->showstr_point = 0;
            return;

    }

    /* do any backing up necessary */
    if (lines < 0) {
        for (scan = d->showstr_point; scan > d->showstr_head; scan--)
            if ((*scan == '\n') || (*scan == '\r')) {
                toggle = -toggle;
                if (toggle < 0)
                    if (!(++lines))
                        break;
            }
        d->showstr_point = scan;
    }

    /* show a chunk */
    lines = 0;
    toggle = 1;

    space = MAX_STRING_LENGTH * 2 - 100;
    for (scan = buffer;; scan++, d->showstr_point++) {
        space--;
        if (((*scan = *d->showstr_point) == '\n' || *scan == '\r')
            && (toggle = -toggle) < 0 && space > 0)
            lines++;
        else if (!*scan || (d->character && !IS_NPC(d->character)
                && lines >= d->character->pcdata->pagelen) || space <= 0) {

            *scan = '\0';
            write_to_buffer(d, buffer, strlen(buffer));

            /* See if this is the end (or near the end) of the string */
            for (chk = d->showstr_point; isspace(*chk); chk++);
            if (!*chk) {
                if (d->showstr_head) {
                    qdispose(d->showstr_head);
                    d->showstr_head = 0;
                }
                d->showstr_point = 0;
            }
            return;
        }
    }

    return;
}

/*
 * The primary output interface for formatted output.
 */
void
subact(const char *format, CHAR_DATA *ch, CHAR_DATA *to, const void *arg1, const void *arg2, int type)
{
    static char        *const he_she[] = { "it", "he", "she" };
    static char        *const him_her[] = { "it", "him", "her" };
    static char        *const his_her[] = { "its", "his", "her" };
    CHAR_DATA          *vch = (CHAR_DATA *) arg2;
    OBJ_DATA           *obj1 = (OBJ_DATA *) arg1;
    OBJ_DATA           *obj2 = (OBJ_DATA *) arg2;
    char                buf[MAX_STRING_LENGTH];
    char                fname[MAX_INPUT_LENGTH];
    char                tmp_str[MSL];
    const char         *str;
    const char         *i = "";
    char               *point;
    bool                do_crlf = TRUE;
    bool                can_see_message = TRUE;
    RULER_DATA         *ruler;
    char                c;

    point = buf;
    str = format;
    while (*str != '\0') {
        if (*str != '$') {
            *point++ = *str++;
            continue;
        }

        ++str;

        if (arg2 == NULL && *str >= 'M' && *str <= 'N') {
            bugf("Act: missing arg2 for code $%c, string=%s", *str, format);
            i = " !!!!! ";
        }
        else if (arg2 == NULL && *str >= 'S' && *str <= 'T') {
            bugf("Act: missing arg2 for code $%c, string=%s", *str, format);
            i = " !!!!! ";
        }
        else if (arg2 == NULL && *str == 'E') {
            bugf("Act: missing arg2 for code $%c, string=%s", *str, format);
            i = " !!!!! ";
        }
        else {
            switch ((c = *str)) {
                /* taken codes: dDeEkKLmMnNpPRsStTX */

                default:
                    bugf("Act: bad code $%c, string=%s", *str, format);
                    i = " !!!!! ";
                    break;

                case 'e': i = he_she[URANGE(0, ch->sex, 2)];  break;
                case 'E': i = he_she[URANGE(0, vch->sex, 2)]; break;
                case 'm': i = him_her[URANGE(0, ch->sex, 2)];  break;
                case 'M': i = him_her[URANGE(0, vch->sex, 2)]; break;
                case 'n': i = PERS(ch, to);  break;
                case 'N': i = PERS(vch, to); break;
                case 's': i = his_her[URANGE(0, ch->sex, 2)];  break;
                case 'S': i = his_her[URANGE(0, vch->sex, 2)]; break;
                case 't': i = (char *) arg1; break;
                case 'T': i = (char *) arg2; break;

                case 'd': /* door */
                case 'D':
                case 'k': /* obj keyword */
                case 'K':
                    if (arg2 == NULL || ((char *) arg2)[0] == '\0') {
                        i =   (c == 'd') ? "the door"
                            : (c == 'D') ? "The door"
                            : (UPPER(c) == 'K') ? "" : "";
                    }
                    else {
                        one_argument((char *) arg2, fname);

                        if (UPPER(c) == 'D')
                            sprintf(tmp_str, "%s %s", *str == 'd' ? "the" : "The", fname);
                        else
                            sprintf(tmp_str, "%s", fname);

                        i = tmp_str;

                        if (fname[0] == '^')
                            i = "something";
                    }
                    break;

                case 'L':
                    can_see_message = TRUE;
                    if (IS_IMMORTAL(to)) {
                        if (IS_SET(ch->act, PLR_WIZINVIS) && ch->invis > get_trust(to))
                            can_see_message = FALSE;
                    }
                    else {
                        if (IS_SET(ch->act, PLR_WIZINVIS) && get_trust(to) < ch->invis)
                            can_see_message = FALSE;

                        if ((IS_AFFECTED(ch, AFF_SNEAK) || item_has_apply(ch, ITEM_APPLY_SNEAK))
                            && ((get_pseudo_level(ch) - 20 + number_range(1, 30)) > get_pseudo_level(to)))
                            can_see_message = FALSE;
                    }

                    break;

                /* if $O is at the start of a line, it means it's an objfun message */
                case 'O':
                    can_see_message = TRUE;
                    if (!IS_NPC(to) && IS_SET(to->config, PLR_NOOBJFUN))
                        can_see_message = FALSE;
                    break;

                case 'p':
                    if (obj1)
                        i = can_see_obj(to, obj1) ? obj1->short_descr : "something";
                    else
                        i = " !!!!! ";
                    break;
                case 'P':
                    if (obj2)
                        i = can_see_obj(to, obj2) ? obj2->short_descr : "something";
                    else
                        i = " !!!!! ";
                    break;

                case 'R':
                    if (can_see(to, ch) && !IS_NPC(ch) && ch->adept_level == 20 && (ruler = get_ruler(ch)) && strcmp(ruler->rank, "@@N")) {
                        sprintf(tmp_str, "%s ", ruler->rank);
                        i = (char *)tmp_str;
                    }
                    else
                        i = "";
                    break;

                case 'X':
                    can_see_message = TRUE;
                    if (IS_IMMORTAL(to)) {
                        if (IS_SET(ch->act, PLR_WIZINVIS) && ch->invis > get_trust(to))
                            can_see_message = FALSE;
                    }
                    else {
                        if (IS_SET(ch->act, PLR_WIZINVIS) && get_trust(to) < ch->invis)
                            can_see_message = FALSE;
                        if (!IS_NPC(ch) && ch->race == RACE_HOB)
                            can_see_message = FALSE;
                        else if (!IS_NPC(ch) && ch->stance == STANCE_AMBUSH && ch->pcdata->stealth > 0)
                            can_see_message = FALSE;
                        else if ((IS_AFFECTED(ch, AFF_SNEAK) || item_has_apply(ch, ITEM_APPLY_SNEAK))
                            && ((get_pseudo_level(ch) - 20 + number_range(1, 30)) > get_pseudo_level(to)))
                            can_see_message = FALSE;
                    }

                    break;

            }

            if (i[0] == '^') {
                ++i;
            }
        }

        ++str;
        while ((*point = *i) != '\0')
            ++point, ++i;
    }

    if (do_crlf) {
        *point++ = '\n';
        *point++ = '\r';
    }

    buf[0] = UPPER(buf[0]);
    *point = '\0';

    if (to->desc && can_see_message)
        write_to_buffer(to->desc, buf, point - buf);

    if (MOBtrigger)
        mprog_act_trigger(buf, to, ch, obj1, vch);

}

void
act(const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, int type)
{
    CHAR_DATA          *to;
    CHAR_DATA          *vch = (CHAR_DATA *) arg2;
    DUEL_DATA          *duel;
    DUEL_WATCHER_DATA  *watcher;

    if (format == NULL || format[0] == '\0' || ch == NULL)
        return;

    if ((ch->is_free != FALSE) || (ch->in_room == NULL)) {
        bugf("bad ch, string=%s", format);
        return;
    }

    to = ch->in_room->first_person;

    if (type == TO_VICT) {
        if (vch == NULL) {
            bugf("Act: null vch with TO_VICT, string=%s", format);
            return;
        }

        to = vch->in_room->first_person;
    }

    for (; to != NULL; to = to->next_in_room) {
        if ((to->desc == NULL && (IS_NPC(to) && !(to->pIndexData->progtypes & ACT_PROG))) || !IS_AWAKE(to))
            continue;
        if (type == TO_CHAR && to != ch)
            continue;
        if (type == TO_VICT && (to != vch || to == ch))
            continue;
        if (type == TO_ROOM && to == ch)
            continue;
        if (type == TO_NOTVICT && (to == ch || to == vch))
            continue;

        subact(format, ch, to, arg1, arg2, type);
    }

    if (type == TO_ROOM || type == TO_NOTVICT) {
        char                fbuf[MSL];

        sprintf(fbuf, "@@d[@@gSPAR@@d]@@N %s", format);

        for (duel = first_duel; duel != NULL; duel = duel->next)
            if (duel->stage == DUEL_STAGE_GO && duel->vnum == ch->in_room->vnum)
                break;

        if (duel) {
            for (watcher = duel->first_watcher; watcher != NULL; watcher = watcher->next)
                subact(fbuf, ch, watcher->ch, arg1, arg2, type);
        }
    }

    MOBtrigger = TRUE;
    return;
}

/*
 * Macintosh support functions.
 */
#if defined(macintosh)
int
gettimeofday(struct timeval *tp, void *tzp)
{
    tp->tv_sec = time(NULL);
    tp->tv_usec = 0;
}
#endif

void
do_finger(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA          *victim;
    char                name[MAX_STRING_LENGTH];
    char                buf[MAX_STRING_LENGTH];
    bool                found = FALSE;
    DESCRIPTOR_DATA     d;
    DESCRIPTOR_DATA    *this_d;

    argument = one_argument(argument, name);

    for (this_d = first_desc; this_d != NULL; this_d = this_d->next) {
        if ((this_d->connected > 0) && !str_cmp(this_d->character->name, name)) {
            do_whois(ch, name);
            return;
        }
    }

    found = load_char_obj(&d, name, TRUE);

    if (!found) {
        sprintf(buf, "No pFile found for '%s'.\n\r", capitalize(name));
        send_to_char(buf, ch);
        return;
    }

    victim = d.character;
    d.character = NULL;
    victim->desc = NULL;

    sprintf(buf, "Name: %s.\n\r", capitalize(victim->name));
    send_to_char(buf, ch);

    sprintf(buf, "Last Login was from: %s.\n\r", victim->pcdata->host);
    send_to_char(buf, ch);

    sprintf(buf, "pFile was last saved at: %s", ctime(&victim->pcdata->lastlogint));
    send_to_char(buf, ch);

    free_char(victim);

    return;
}

void
send_to_descrips(const char *message)
{
#ifdef BPORT
    log_string(message);
#endif
    return;
}

/* Here it is boys and girls the HOT reboot function and all its nifty  * little parts!! - Flar
 */
void
do_hotreboo(CHAR_DATA *ch, char *argument)
{
    send_to_char("If you want to do a @@R@@fHOT@@Breboot@@N....spell it out.\n\r", ch);
    return;
}

extern int          port, control;    /* db.c */

#define HOTREBOOT_REVISION 3

void
do_hotreboot(CHAR_DATA *ch, char *argument)
{
    FILE               *fp;
    DESCRIPTOR_DATA    *d, *d_next;
    char                buf[MSL];
    extern int          AreasModified;
    DUEL_DATA          *duel, *duelnext;

    if (nosave) {
        send_to_char("MUD is in nosave mode, you must forcefully shutdown.\n\r", ch);
        return;
    }

    if (AreasModified > 0) {
        send_to_char("Areas are currently modified, try again when they finish saving.\n\r", ch);
        return;
    }

    fp = fopen(COPYOVER_FILE, "w");

    if (!fp) {
        send_to_char("Copyover file not writeable, aborted.\n\r", ch);
        xlogf("Could not write to copyover file: %s", COPYOVER_FILE);
        perror("do_copyover:fopen");
        return;
    }

    for (duel = first_duel; duel != NULL; duel = duelnext) {
        duelnext = duel->next;

        cancel_duel(duel, NULL, DUEL_END_TIMEOUT);
    }

    if (auction_item != NULL)
        do_auction(ch, "stop");

    sprintf(buf,
        "\n\r"
        "**** HOTreboot by An Immortal - Please remain ONLINE ****\n\r"
        "*********** We will be back in a few seconds! ***********\n\r"
        "\n\r");

    FPRINTF(fp, "%d %d %d %d\n%s~\n", HOTREBOOT_REVISION, max_players, (unsigned int) boot_time, control, ch->short_descr);

    /* For each PLAYING descriptor (non-negative), save its state */
    for (d = first_desc; d; d = d_next) {
        CHAR_DATA          *och = CH(d);

        d_next = d->next;        /* We delete from the list , so need to save this */

        if (!d->character || d->connected < 0 || (d->character && d->character->in_room == NULL)) {    /* drop those logging on */
            write_to_descriptor(d, "\n\rSorry, ACK! MUD rebooting. Come back in a few seconds.\n\r", 0);
            close_socket(d);    /* throw'em out */
        }
        else {
            if (IS_NPC(och))
                continue;

            if (IS_SET(och->pcdata->pflags, PFLAG_SPECIALNAME)) {
                FPRINTF(fp, "%d %s %s %s %d %d %d %d\n", d->descriptor, och->pcdata->origname, d->host, d->ip, (d->out_compress != NULL), och->stance, och->in_room->vnum, och->pcdata->stealth);
            }
            else
                FPRINTF(fp, "%d %s %s %s %d %d %d %d\n", d->descriptor, och->name, d->host, d->ip, (d->out_compress != NULL), och->stance, och->in_room->vnum, och->pcdata->stealth);

            if (och->level == 1) {
                write_to_descriptor(d, "Since you are level one, and level one characters do not save....you have been advanced!\n\r", 0);
                och->level = 2;
                och->lvl[3] = 2;
            }

            save_char_obj(och);
            write_to_descriptor(d, buf, 0);
        }

        if (d->out_compress != NULL) {
            compressEnd(d);
        }
    }

    FPRINTF(fp, "-1\n");
    fclose(fp);

    /* Close reserve and other always-open files and release other resources */

    fclose(fpReserve);

    if (fpcomlog)
        fclose(fpcomlog);

    if (dns.fd != -1)
        close(dns.fd);

    sprintf(buf, "%d", port);

    execl(EXE_FILE, EXE_FILE, buf, "copyover", (char *) NULL);

    /* Failed - sucessful exec will not return */
    perror("do_copyover: execl");
    send_to_char("HOTreboot FAILED! Something is wrong in the shell!\n\r", ch);

    /* Here you might want to reopen fpReserve */
}

/* Recover from a copyover - load players */
void
copyover_recover()
{
    DESCRIPTOR_DATA    *d;
    FILE               *fp;
    char                name[100];
    char                host[MSL];
    char                ip[MSL];
    int                 desc;
    bool                fOld;
    int                 compress;
    int                 revision;
    int                 stance;
    int                 vnum;
    int                 stealth = 0;
    extern bool         disable_timer_abort;

    last_reboot_by = str_dup("");

    today_time = get_today_ctime();

    xlogf("Copyover recovery initiated");
    disable_timer_abort = TRUE;
    fp = fopen(COPYOVER_FILE, "r");

    if (!fp) {                    /* there are some descriptors open which will hang forever then ? */
        perror("copyover_recover:fopen");
        xlogf("Copyover file not found. Exitting.\n\r");
        exit(1);
    }

    unlink(COPYOVER_FILE);        /* In case something crashes - doesn't prevent reading  */

    revision = fread_number(fp);
    max_players = fread_number(fp);
    boot_time = fread_number(fp);
    reboot_time = current_time;
    control = fread_number(fp);

    free_string(last_reboot_by);
    last_reboot_by = fread_string(fp);

    for (;;) {
        compress = 0;

        if (revision < 2) {
            fscanf(fp, "%d %s %s %d %d %d\n", &desc, name, host, &compress, &stance, &vnum);
            strcpy(ip, host);
        }
        else if (revision < 3) {
            fscanf(fp, "%d %s %s %s %d %d %d\n", &desc, name, host, ip, &compress, &stance, &vnum);
        }
        else {
            fscanf(fp, "%d %s %s %s %d %d %d %d\n", &desc, name, host, ip, &compress, &stance, &vnum, &stealth);
        }

        if (desc == -1)
            break;

        /* Write something, and check if it goes error-free */
        if (!write_to_descriptor_2(desc, "\n\rRestoring from HOTreboot...\n\r", 0)) {
            close(desc);        /* nope */
            continue;
        }

        GET_FREE(d, desc_free);
        init_descriptor(d, desc);    /* set up various stuff */

        d->host = str_dup(host);
        d->ip   = str_dup(ip);
        d->next = NULL;
        d->prev = NULL;

        d->connected = CON_COPYOVER_RECOVER;    /* -15, so close_socket frees the char */
        LINK(d, first_desc, last_desc, next, prev);

        if (compress) {
            compressStart(d);
        }

        /* Now, find the pfile */

        fOld = load_char_obj(d, name, FALSE);

        if (!fOld) {            /* Player file not found?! */
            write_to_descriptor(d, "\n\rSomehow, your character was lost in the HOTreboot. Sorry, you must relog in.\n\r", 0);
            close_socket(d);
        }
        else {                    /* ok! */

            CHAR_DATA          *this_char;

            write_to_descriptor(d, "\n\rCopyover recovery complete.\n\r", 0);

            d->character->in_room = get_room_index(vnum);

            if (!d->character->in_room)
                d->character->in_room = get_room_index(ROOM_VNUM_LIMBO);

            /* Insert in the char_list */
            d->character->next = NULL;
            d->character->prev = NULL;
            this_char = d->character;

            LINK(this_char, first_char, last_char, next, prev);
            LINK(this_char, first_player, last_player, next_player, prev_player);

            char_to_room(d->character, d->character->in_room);
            if (d->character->position == POS_RIDING)
                d->character->position = POS_STANDING;
            do_look(d->character, "");
            act("$n's atoms materialize and reform.", d->character, NULL, NULL, TO_ROOM);
            /*  d->connected = CON_PLAYING;  */

            if (this_char->pcdata->hp_from_gain < 0)
                reset_gain_stats(this_char);
            /*       this_char->affected_by = 0;   */

            this_char->is_quitting = FALSE;
            d->connected = CON_SETTING_STATS;
            {
                OBJ_DATA           *wear_object;
                AFFECT_DATA        *this_aff;
                AFFECT_DATA        *this_aff_next;

                this_char->max_mana = this_char->pcdata->mana_from_gain;
                this_char->max_hit = this_char->pcdata->hp_from_gain;
                this_char->max_move = this_char->pcdata->move_from_gain;
                this_char->saving_throw = get_pseudo_level(this_char) / 10;
                this_char->hitroll = 0;
                this_char->damroll = 0;
                this_char->armor = 100;
                if (this_char->login_sex != -1)
                    this_char->sex = this_char->login_sex;
                this_char->affected_by = 0;

                for (wear_object = this_char->first_carry; wear_object != NULL; wear_object = wear_object->next_in_carry_list) {
                    if (wear_object->wear_loc > WEAR_NONE)
                        equip_char(this_char, wear_object, wear_object->wear_loc);
                }
                /* add spells saved to stats  */

                for (this_aff = this_char->first_saved_aff; this_aff != NULL; this_aff = this_aff_next) {
                    this_aff_next = this_aff->next;
                    UNLINK(this_aff, this_char->first_saved_aff, this_char->last_saved_aff, next, prev);

                    if (this_aff->type == gsn_shield_fire)
                        do_cast(this_char, "fireshield");
                    else if (this_aff->type == gsn_shield_ice)
                        do_cast(this_char, "iceshield");
                    else if (this_aff->type == gsn_shield_shock)
                        do_cast(this_char, "shockshield");
                    else if (this_aff->type == gsn_shield_demon)
                        do_cast(this_char, "demonshield");
                    else
                        affect_to_char(this_char, this_aff);
                    PUT_FREE(this_aff, affect_free);
                }

                this_char->pcdata->stealth = stealth;

                if (stance_app[stance].ac_mod > 0)
                    this_char->stance_ac_mod = (stance_app[stance].ac_mod * (20 - get_pseudo_level(this_char) / 12));
                else
                    this_char->stance_ac_mod = stance_app[stance].ac_mod * get_pseudo_level(this_char) / 12;
                if (stance_app[stance].dr_mod < 0)
                    this_char->stance_dr_mod = (stance_app[stance].dr_mod * (20 - get_pseudo_level(this_char) / 12));
                else
                    this_char->stance_dr_mod = stance_app[stance].dr_mod * get_pseudo_level(this_char) / 10;

                if (stance_app[stance].hr_mod < 0)
                    this_char->stance_hr_mod = (stance_app[stance].hr_mod * (20 - get_pseudo_level(this_char) / 12));
                else
                    this_char->stance_hr_mod = stance_app[stance].hr_mod * get_pseudo_level(this_char) / 10;
                this_char->stance = stance;
            }

            d->connected = CON_PLAYING;

        }

    }

    recalc_playercounts();

    /* avoid leaks! */
    fclose(fp);

    disable_timer_abort = FALSE;
}

void
hang(const char *str)
{
    bug(str);
    raise(SIGQUIT);
}

time_t
get_today_ctime(void)
{

    struct tm          *tp;
    time_t              today;

    tp = localtime(&current_time);
    tp->tm_sec = 0;
    tp->tm_min = 0;
    tp->tm_hour = 0;
    today = mktime(tp);

    return today;
}

char               *
get_today_string(time_t today)
{
    static char         buf[MSL];
    struct tm          *tp;

    tp = localtime(&today);

    strftime(buf, sizeof(buf), "%Y-%m-%d", tp);

    return buf;
}
