#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>

#define __USE_BSD
#include <stdio.h>
#undef __USE_BSD

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "merc.h"
#include "duel.h"
#include "dns.h"

#define BIND_ADDR "0.0.0.0"
#define DNS_PORT 53

struct dns_setup dns;

void dns_setup(void);
char *dns_lookup(char *ip);
static int dn_find(unsigned char *exp_dn, unsigned char *msg, unsigned char **dnptrs, unsigned char **lastdnptr);
int dn_skipname(unsigned char *comp_dn, unsigned char *eom);
int dn_comp(unsigned char *exp_dn, unsigned char *comp_dn, int length, unsigned char **dnptrs, unsigned char **lastdnptr);
int dn_expand(unsigned char *msg, unsigned char *eomorig, unsigned char *comp_dn, unsigned char *exp_dn, int length);
static int extract_rrs(DNS_DATA *, unsigned char *, size_t, int, int);

void dns_setup(void)
{
    struct sockaddr_in sa;
    struct hostent *host;
    int             x = 1;
    extern char *nameserver;

    if (dns.fd != -1)
        return; /* already setup */

    dns.fd           = -1;
    dns.ip[0]        = '\0';
    dns.retry        = current_time + DNS_EXPIRE_SOCKET; /* when to reconnect socket */
    dns.first_lookup = NULL;
    dns.last_lookup  = NULL;
    dns.first_cache  = NULL;
    dns.last_cache   = NULL;

    if (!nameserver || nameserver[0] == '\0') {
        dns.fd = -1;
        return;
    }

    if ((dns.fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        xlogf("dns_setup: socket");
        dns.fd = -1;
        return;
    }

    if (fcntl(dns.fd, F_SETFL, O_NONBLOCK) == -1) {
        close(dns.fd);
        dns.fd = -1;
        return;
    }

    if (setsockopt(dns.fd, SOL_SOCKET, SO_REUSEADDR, (char *) &x, sizeof(x)) < 0) {
        xlogf("dns_setup: SO_REUSEADDR");
        close(dns.fd);
        dns.fd = -1;
        return;
    }

#if defined(SO_DONTLINGER)
    {
        struct linger       ld;

        ld.l_onoff = 1;
        ld.l_linger = 1000;

        if (setsockopt(dns.fd, SOL_SOCKET, SO_DONTLINGER, (char *) &ld, sizeof(ld)) < 0) {
            xlogf("dns_setup: SO_DONTLINGER");
            close(dns.fd);
            dns.fd = -1;
            return;
        }
    }
#endif

    memset(&sa, 0, sizeof(struct sockaddr_in));
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_family = AF_INET;

    if (!(host = gethostbyname(BIND_ADDR))) {
        xlogf("dns_setup: gethostbyname");
        close(dns.fd);
        dns.fd = -1;
        return;
    }

    memcpy((char *)&sa.sin_addr, host->h_addr, host->h_length);
    sa.sin_family = host->h_addrtype;
    sa.sin_port = htons(dns.bind_port);

    if (bind(dns.fd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        xlogf("dns_setup: bind");
        close(dns.fd);
        dns.fd = -1;
        return;
    }

    if (!(host = gethostbyname(nameserver))) {
        xlogf("dns_setup: gethostbyname");
    }

    memset(&sa, 0, sizeof(sa));
    memcpy((char *)&sa.sin_addr, host->h_addr, host->h_length);
    sa.sin_family = host->h_addrtype;
    sa.sin_port = htons(DNS_PORT);

    if (connect(dns.fd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        xlogf("dns_setup: connect");
        close(dns.fd);
        dns.fd = -1;
        return;
    }

    return;
}

void dns_exec(char *iphost, bool ip)
{
    DNS_DATA *dd = NULL, *odd;
    struct dns_packet_header hdr;
    struct dns_query qry;
    unsigned char pkt[DNS_MAX_PACKET_SIZE + DNS_MAX_NAMELEN];
    int nlen, plen;

    for (odd = dns.first_cache; odd != NULL; odd = odd->next) {
        if (ip && !strcmp(iphost, odd->ip)) {
            return;
        }
        else if (!strcmp(iphost, odd->host)) {
            return;
        }
    }

    for (odd = dns.first_lookup; odd != NULL; odd = odd->next) {
        if (ip && !strcmp(iphost, odd->ip)) {
            dd = odd;
            break;
        }
        else if (!strcmp(iphost, odd->host)) {
            dd = odd;
            break;
        }
    }

    if (odd == NULL) {
        CREATE_MEMBER(DNS_DATA, dd);
        dd->id = number_range(1, 30000); /* yes, i know this has the potential for collisions, TODO */
        dd->flags = DNS_FLAG_LOOKUP;
        dd->expire = current_time + DNS_EXPIRE_LOOKUP;
    }

    /* set up our header */
    memset(&hdr, 0, sizeof(hdr));
    hdr.id = htons(dd->id);
    hdr.rd = 1; /* we want recursion from the nameserver */
    hdr.qdcount = htons(1); /* always one question for us */

    /* now set up our query.  in most cases we don't do anything special with
     * the user data in dlp, but for the PTR type we try to rewrite the
     * (assumedly) presentation format address into one suitable to the
     * nameserver. */

    if (ip) {
        int c[4];

        sscanf((char *)iphost, "%d.%d.%d.%d", &c[3], &c[2], &c[1], &c[0]);
        sprintf((char *)qry.name, "%d.%d.%d.%d.in-addr.arpa", c[0], c[1], c[2], c[3]);
        strcpy((char *)dd->ip, iphost);
        qry.class = htons(DNS_C_IN);
        qry.type = htons(DNS_T_PTR);
/*        xlogf("trying to resolve: %s", qry.name); */
    } else {
        strcpy((char *)qry.name, iphost);
        qry.class = htons(DNS_C_IN);
        qry.type = htons(DNS_T_A);
/*        xlogf("trying to resolve: %s", qry.name); */
    }
    
    /* now construct the packet and send it along.. */
    memcpy(pkt, &hdr, sizeof(hdr));
    plen = sizeof(hdr);
    plen += dn_comp(qry.name, pkt + plen, DNS_MAX_NAMELEN, NULL, NULL);
    memcpy(pkt + plen, &qry.type, sizeof(uint16_t));
    plen += sizeof(uint16_t);
    memcpy(pkt + plen, &qry.class, sizeof(uint16_t));
    plen += sizeof(uint16_t);
    
    /* and send the packet */
    if ((nlen = write(dns.fd, pkt, plen)) != plen)
        xlogf("failed to send dns query for %s (tried %d, sent %d)", qry.name, plen, nlen);
/*    else
        xlogf("sent dns query for %s (tried %d, sent %d) to fd %d", qry.name, plen, nlen, dns.fd);
*/

    if (odd == NULL)
        DLINK(dd, dns.first_lookup, dns.last_lookup, next, prev);

/*
    if (IS_SET(dd->flags, DNS_FLAG_LOOKUP))
        return (unsigned char *)"(looking up)";
    else if (IS_SET(dd->flags, DNS_FLAG_REVERSE))
        return (unsigned char *)"(reverse lookup)";
*/

    return;
}

char *dns_lookup(char *ip)
{
    DNS_DATA *dd;

    for (dd = dns.first_cache; dd != NULL; dd = dd->next)
        if (!strcmp(ip, dd->ip)) {
            return (char *)dd->host;
        }

    return ip;
}

/* Defines for handling compressed domain names */
#define INDIR_MASK   0xc0

/*
 * Expand compressed domain name 'comp_dn' to full domain name. 'msg'
 * is a pointer to the begining of the message, 'eomorig' points to the
 * first location after the message, 'exp_dn' is a pointer to a buffer
 * of size 'length' for the result. Return size of compressed name or
 * -1 if there was an error.
 */
int dn_expand(unsigned char *msg, unsigned char *eomorig, unsigned char *comp_dn, unsigned char *exp_dn, int length) {
    unsigned char *cp, *dn;
    int n, c;
    unsigned char *eom;
    int len = -1, checked = 0;

    dn = exp_dn;
    cp = comp_dn;
    eom = exp_dn + length;
    /* fetch next label in domain name */
    while ((n = *cp++)) {
    /* Check for indirection */
    switch (n & INDIR_MASK) {
        case 0:
        if (dn != exp_dn) {
            if (dn >= eom)
            return (-1);
            *dn++ = '.';
        }
        if (dn + n >= eom)
            return (-1);
            checked += n + 1;
        while (--n >= 0) {
            if ((c = *cp++) == '.') {
            if (dn + n + 2 >= eom)
                return (-1);
            *dn++ = '\\';
            }
            *dn++ = c;
            if (cp >= eomorig)  /* out of range */
            return (-1);
        }
        break;

        case INDIR_MASK:
        if (len < 0)
            len = cp - comp_dn + 1;
        cp = msg + (((n & 0x3f) << 8) | (*cp & 0xff));
        if (cp < msg || cp >= eomorig)  /* out of range */
            return (-1);
        checked += 2;
        /* Check for loops in the compressed name; if we've
           looked at the whole message, there must be a loop. */
        if (checked >= eomorig - msg)
            return (-1);
        break;

        default:
        return (-1);    /* flag error */
    }
    }
    *dn = '\0';
    if (len < 0)
    len = cp - comp_dn;
    return (len);
}
/* Compress domain name 'exp_dn' into 'comp_dn'. Return the size of the
 * compressed name or -1. 'length' is the size of the array pointed to
 * by 'comp_dn'. 'dnptrs' is a list of pointers to previous compressed
 * names. dnptrs[0] is a pointer to the beginning of the message. The
 * list ends with NULL. 'lastdnptr' is a pointer to the end of the
 * arrary pointed to by 'dnptrs'. Side effect is to update the list of
 * pointers for labels inserted into the message as we compress the
 * name. If 'dnptr' is NULL, we don't try to compress names. If
 * 'lastdnptr' is NULL, we don't update the list.  */
int dn_comp(unsigned char *exp_dn, unsigned char *comp_dn, int length, unsigned char **dnptrs, unsigned char **lastdnptr) {
    unsigned char *cp, *dn;
    int c, l;
    unsigned char **cpp, **lpp, *sp, *eob;
    unsigned char *msg;

    dn = exp_dn;
    cp = comp_dn;
    eob = cp + length;
    cpp = lpp = NULL; /* fix for warning */
    if (dnptrs != NULL) {
    if ((msg = *dnptrs++) != NULL) {
        for (cpp = dnptrs; *cpp != NULL; cpp++);
        lpp = cpp;      /* end of list to search */
    }
    }
    else
    msg = NULL;

    for (c = *dn++; c != '\0';) {
    /* look to see if we can use pointers */
    if (msg != NULL) {
        if ((l = dn_find(dn - 1, msg, dnptrs, lpp)) >= 0) {
        if (cp + 1 >= eob)
            return (-1);
        *cp++ = (l >> 8) | INDIR_MASK;
        *cp++ = l % 256;
        return (cp - comp_dn);
        }
        /* not found, save it */
        if (lastdnptr != NULL && cpp < lastdnptr - 1) {
        *cpp++ = cp;
        *cpp = NULL;
        }
    }
    sp = cp++;      /* save ptr to length byte */
    do {
        if (c == '.') {
        c = *dn++;
        break;
        }
        if (c == '\\') {
        if ((c = *dn++) == '\0')
        break;
        }
        if (cp >= eob) {
        if (msg != NULL)
        *lpp = NULL;
        return (-1);
        }
        *cp++ = c;
    } while ((c = *dn++) != '\0');

    /* catch trailing '.'s but not '..' */
    if ((l = cp - sp - 1) == 0 && c == '\0') {
        cp--;
        break;
    }
    if (l <= 0 || l > DNS_MAX_SEGLEN) {
        if (msg != NULL)
        *lpp = NULL;
        return (-1);
    }
    *sp = l;
    }
    if (cp >= eob) {
    if (msg != NULL)
        *lpp = NULL;
    return (-1);
    }
    *cp++ = '\0';
    return (cp - comp_dn);
}
/*
 * Skip over a compressed domain name. Return the size or -1.
 */
int dn_skipname(unsigned char *comp_dn, unsigned char *eom) {
    unsigned char *cp;
    int n;

    cp = comp_dn;
    while (cp < eom && (n = *cp++)) {
    /* check for indirection */
    switch (n & INDIR_MASK) {
        case 0:     /* normal case, n == len */
        cp += n;
        continue;
        default:        /* illegal type */
        return (-1);
        case INDIR_MASK:    /* indirection */
        cp++;
    }
    break;
    }
    return (cp - comp_dn);
}
/*
 * Search for expanded name from a list of previously compressed names.
 * Return the offset from msg if found or -1. dnptrs is the pointer to
 * the first name on the list, not the pointer to the start of the
 * message.
 */
static int dn_find(unsigned char *exp_dn, unsigned char *msg, unsigned char **dnptrs, unsigned char **lastdnptr) {
    unsigned char *dn, *cp, **cpp;
    int n;
    unsigned char *sp;

    for (cpp = dnptrs; cpp < lastdnptr; cpp++) {
    dn = exp_dn;
    sp = cp = *cpp;
    while ((n = *cp++)) {
        /* check for indirection */
        switch (n & INDIR_MASK) {
        case 0:     /* normal case, n == len */
            while (--n >= 0) {
            if (*dn == '.')
                goto next;
            if (*dn == '\\')
                dn++;
            if (*dn++ != *cp++)
                goto next;
            }
            if ((n = *dn++) == '\0' && *cp == '\0')
            return (sp - msg);
            if (n == '.')
            continue;
            goto next;

        default:        /* illegal type */
            return (-1);

        case INDIR_MASK:    /* indirection */
            cp = msg + (((n & 0x3f) << 8) | *cp);
        }
    }
    if (*dn == '\0')
        return (sp - msg);
next:;
    }
    return (-1);
}

void dns_packet_parse(unsigned char *pkt, size_t plen) {
    struct dns_packet_header hdr;
    struct dns_query qry;

    int pidx = 0; /* index into the packet.  basically a count of how many
             bytes we have sucked out of it so far. */
    DNS_DATA *dd;
    int rc;

    if (plen < sizeof(struct dns_packet_header) + 4) {
        xlogf("received malformed dns packet (size %d too small!)", plen);
        return;
    }

    /* bring in the header, and then reformat it for our host byte order.
     * find the query with the id (if any) and do some quick error checks */
    memcpy(&hdr, pkt, sizeof(hdr));
    pidx += sizeof(hdr);
    hdr.id = ntohs(hdr.id);
    hdr.qdcount = ntohs(hdr.qdcount);
    hdr.ancount = ntohs(hdr.ancount);
    hdr.nscount = ntohs(hdr.nscount);
    hdr.adcount = ntohs(hdr.adcount);

    /* search for our lookup */
    for (dd = dns.first_lookup; dd != NULL; dd = dd->next) {
        if (dd->id == hdr.id)
            break;
    }

    /* if we didn't find it send a debug notice (since we don't want to warn
     * and allow a DoS from people flooding in fake dns packets) and return */
    if (dd == NULL) {
        xlogf("Got dns reply with unknown id %d", hdr.id);
        return;
    }

    /* now do some sanity checks on the header */
    if (hdr.qdcount != 1 || !hdr.qr || hdr.opcode) {
        xlogf("received a mangled dns packet (qdcount=%d qr=%d, opcode=%d)", hdr.qdcount, hdr.qr, hdr.opcode);
        return;
    }

    if (hdr.tc) {
        /* XXX: we should do TCP lookups in the case of truncated replies.
         * will do that in the future.. */
        xlogf("got truncated dns reply for %s", dd->ip);
        return;
    }

    /* extract our question, too */
    if ((rc = dn_expand(pkt, pkt + plen, pkt + pidx, qry.name, DNS_MAX_NAMELEN + 1)) < 1) {
        xlogf("dn_expand (%d) returned %d", __LINE__, rc);
        return;
    }

    pidx += rc;
    memcpy(&qry.type, pkt + pidx, sizeof(qry.type));
    qry.type = ntohs(qry.type);
    pidx += sizeof(qry.type);
    memcpy(&qry.class, pkt + pidx, sizeof(qry.class));
    qry.class = ntohs(qry.class);
    pidx += sizeof(qry.class);

    /* Examine our return code.  See if the server doesn't much like us. */
    switch (hdr.rcode) {
    case DNS_R_OK:
        break; /* skip */
    case DNS_R_NXDOMAIN:
    case DNS_R_SERVFAIL:
    case DNS_R_BADFORMAT:
    case DNS_R_NOTIMP:
    case DNS_R_REFUSED:
    default:
        /* these are non-recoverable errors */
        xlogf("nameserver returned error on query for %s (%d %s)", qry.name, hdr.rcode,
            (hdr.rcode == DNS_R_BADFORMAT ? "bad format" :
             (hdr.rcode == DNS_R_NOTIMP ? "not implemented" :
              (hdr.rcode == DNS_R_REFUSED ? "query refused" :
               "unknown"))));
        dd->flags = DNS_FLAG_CACHED;
        return;
    }

    /* mwokay.  now we just need to suck out the answer/authority/additional
     * sections.  Simple enough ... just use the function below.. */
    pidx = extract_rrs(dd, pkt, plen, pidx, hdr.ancount);
/*    pidx = extract_rrs(pkt, plen, pidx, hdr.nscount, &dlp->rrs.ns);
    extract_rrs(pkt, plen, pidx, hdr.adcount, &dlp->rrs.ad); */

    /* If rcode was NXDOMAIN or we have no answer section then we set the
     * NXDOMAIN flag.  Apparently the reply can contain NSes in the authority
     * section to refer us to them, and we *should* (XXX) honor that, but for
     * now we don't. */
    /* Otherwise we should look to see if we asked for something that wasn't a
     * CNAME and only got CNAMEs back, and then follow up on those CNAMEs.  We
     * don't do that -- yet. (XXX) */

    return;

}

/* this function extracts count RRs from pkt starting at index 'pidx' and
 * places them in list.  It returns the new value of pidx when done.
 * Additionally, for certain types, we decompress the rdata here (specifically
 * A, AAAA, HINFO, MINFO, WKS, SOA, NS, CNAME, and PTR) */
static int extract_rrs(DNS_DATA *dd, unsigned char *pkt, size_t plen, int pidx, int count) {
    struct dns_rr rr;
    int rc;
    unsigned char *rdata;

    while (count) {
        if ((rc = dn_expand(pkt, pkt + plen, pkt + pidx, rr.name, DNS_MAX_NAMELEN + 1)) < 1) {
            xlogf("dn_expand (%d) returned %d", __LINE__, rc);
            return pidx;
        }

        pidx += rc;
        memcpy(&rr.type, pkt + pidx, sizeof(uint16_t));
        rr.type = ntohs(rr.type);
        pidx += sizeof(uint16_t);
        memcpy(&rr.class, pkt + pidx, sizeof(uint16_t));
        rr.class = ntohs(rr.class);
        pidx += sizeof(uint16_t);
        memcpy(&rr.ttl, pkt + pidx, sizeof(uint32_t));
        rr.ttl = ntohl(rr.ttl);
        pidx += sizeof(uint32_t);
        memcpy(&rr.rdlen, pkt + pidx, sizeof(uint16_t));
        rr.rdlen = ntohs(rr.rdlen);
        pidx += sizeof(uint16_t);

        if (rr.rdlen > 0) {
            rdata = pkt + pidx;
            pidx += rr.rdlen;

            /* Fill in the data.  We do a copy over first, and if we need to
             * realloc and make changes below we do that too */
            memcpy(rr.txt, rdata, rr.rdlen);

            switch (rr.type) {
                case DNS_T_NS:
                case DNS_T_HINFO:
                case DNS_T_MINFO:
                case DNS_T_MX:
                case DNS_T_WKS:
                case DNS_T_SOA:
                default:
                    break;
                case DNS_T_A:
                    if (rr.class == DNS_C_IN) {
                        rr.rdlen = IPADDR_MAXLEN + 1;
                        inet_ntop(PF_INET, rdata, (char *)rr.txt, rr.rdlen);

                        if (!strcmp(rr.txt, dd->ip)) {
                            DUNLINK(dd, dns.first_lookup, dns.last_lookup, next, prev);
                            DLINK(dd, dns.first_cache, dns.last_cache, next, prev);
                            dd->flags = DNS_FLAG_CACHED;
                            dd->expire = current_time + DNS_EXPIRE_NORMAL; /* cached resolved entries for a day */
                        }
                        else {
/*                            xlogf("dns mismatch, rr.txt[%s] dd->ip[%s]", rr.txt, dd->ip); */
                            DUNLINK(dd, dns.first_lookup, dns.last_lookup, next, prev);
                            DLINK(dd, dns.first_cache, dns.last_cache, next, prev);
                            dd->flags = DNS_FLAG_CACHED;
                            dd->expire = current_time + DNS_EXPIRE_BAD; /* cache mismatched entries for an hour */
                            strcpy((char *)dd->host, (char *)dd->ip);
                        }

                        {
                            CHAR_DATA *ch;
                            DESCRIPTOR_DATA *d;

                            for (d = first_desc; d != NULL; d = d->next) {
                                if (!str_cmp((char *)dd->ip, d->ip)) {
                                    free_string(d->host);
                                    d->host = str_dup((char *)dd->host);
                                }
                            }

                            for (ch = first_player; ch != NULL; ch = ch->next_player) {
                                if (!str_cmp((char *)dd->ip, ch->pcdata->ip)) {
                                    free_string(ch->pcdata->host);
                                    ch->pcdata->host = str_dup((char *)dd->host);
                                }
                            }
                        }

                        return pidx;
                    }

                    break;
                case DNS_T_CNAME:
                case DNS_T_PTR:
                    rr.rdlen = DNS_MAX_NAMELEN + 1;
                    if ((rc = dn_expand(pkt, pkt + plen, rdata, rr.txt, DNS_MAX_NAMELEN + 1)) < 1) {
                        xlogf("dn_expand (%d) returned %d", __LINE__, rc);
                        return pidx;
                    }

                    strncpy(dd->host, rr.txt, DNS_MAX_SEGLEN);
                    allow_set((char *)dd->host, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-.");
                    dd->flags = DNS_FLAG_REVERSE;
                    (void)dns_exec((char *)dd->host, FALSE);
                    return pidx;

                    break;
            }
        }

        count--;
    }

    return pidx;
}

void dns_update(void)
{
    DNS_DATA *dd, *ddnext;

    for (dd = dns.first_lookup; dd != NULL; dd = ddnext) {
        ddnext = dd->next;

        if (current_time >= dd->expire) {
/*            xlogf("expiring lookup %s", dd->ip); */
            DUNLINK(dd, dns.first_lookup, dns.last_lookup, next, prev);
            DESTROY_MEMBER(dd);
        }
    }

    for (dd = dns.first_cache; dd != NULL; dd = ddnext) {
        ddnext = dd->next;

        if (current_time >= dd->expire) {
/*            xlogf("expiring cache %s (%s)", dd->host, dd->ip); */
            DUNLINK(dd, dns.first_cache, dns.last_cache, next, prev);
            DESTROY_MEMBER(dd);
        }
    }

    /* problem with the dns socket, try to reconnect? */
    if (dns.fd == -1 && current_time >= dns.retry)
        dns_setup();

    return;
}
