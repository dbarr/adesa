/*
 * dns.h: header file for the dns module
 * 
 * Copyright 2002 the Ithildin Project.
 * See the COPYING file for more information on licensing and use.
 * 
 * $Id: dns.h,v 1.3 2003/08/17 07:40:32 dave Exp $
 */

#ifndef DNS_DNS_H
#define DNS_DNS_H

/* if the dns socket goes bad, how long before trying to reconnect? */
#define DNS_EXPIRE_SOCKET 300

/* maximum length a lookup can take before being discarded */
#define DNS_EXPIRE_LOOKUP 30

/* maximum length to store a successfully cached lookup */
#define DNS_EXPIRE_NORMAL (60 * 60 * 24)

/* maximum length to store a unsuccessfully cached lookup */
#define DNS_EXPIRE_BAD    (60 * 60 * 1)

/*
 * This file defines the various structures useful for communicating with a DNS
 * server.  Most of these are not alltogether useful from an outside
 * perspective, but are provided here anyways to keep things in order.  I
 * gleaned most of this from a cross of the bind headers and reading RFC1035,
 * with a little bit of ingenuity on my own part.
 */

/* First off, some generic definitions and enumerations which should be useful
 * for us.  This includes the rr type and rr class variants, and some size
 * limitations. */

#define DNS_MAX_PACKET_SIZE 512	    /* maximum size of a dns packet */
#define DNS_MAX_NAMELEN	    1024    /* maximum length of an FQDN */
#define DNS_MAX_SEGLEN	    63	    /* maximum length of an FQDN segment. */
#define DNS_HEADER_LEN	    12	    /* size of a query/answer header. */
#define DNS_DEFAULT_PORT    53	    /* default DNS server port */
#define IPADDR_MAXLEN       63

struct dns_data
{
    DNS_DATA *next;
    DNS_DATA *prev;

    uint16_t    id;                        /* query ID */
#define DNS_FLAG_LOOKUP 1
#define DNS_FLAG_REVERSE 2
#define DNS_FLAG_CACHED 4
    int         flags;
    unsigned char        host[DNS_MAX_SEGLEN + 1]; /* hostname */
    unsigned char        ip[DNS_MAX_SEGLEN + 1];   /* ip address */
    time_t      expire;                    /* when will we expire this entry? */
};

struct dns_setup
{
    int         fd;                        /* file descriptor of dns socket */
    unsigned char ip[DNS_MAX_SEGLEN + 1];  /* ip address of nameserver we're using */
    int         bind_port;                 /* port we bind on when connecting to nameserver */
    time_t      retry;                     /* if our connection is lost, when to try to reconnect */
    DNS_DATA    *first_lookup;
    DNS_DATA    *last_lookup;
    DNS_DATA    *first_cache;
    DNS_DATA    *last_cache;
};

/* various query classes available.  I've never seen a use for anything but the
 * 'IN' class, but we include these for completeness. */
typedef enum {
    DNS_C_IN = 1,		/* INternet class */
    DNS_C_CHAOS = 3,		/* MIT CHAOS net */
    DNS_C_HESIOD = 4,		/* MIT Hesiod (..zuh?) */
    DNS_C_NONE = 254,		/* classless requests */
    DNS_C_ANY = 255		/* wildcard class */
} dns_class_t;
extern const char *const dns_class_strlist[];

#define dns_class_conv_str(x) dns_class_strlist[(x)]
dns_class_t dns_str_conv_class(const char *);

/* and the various query types. */
typedef enum {
    DNS_T_A = 1,		/* Address */
    DNS_T_NS = 2,		/* Name Server */
    DNS_T_MD = 3,		/* Mail Destination */
    DNS_T_MF = 4,		/* Mail Forwarder */
    DNS_T_CNAME = 5,		/* Canonical name (alias) */
    DNS_T_SOA = 6,		/* Start of authority */
    DNS_T_MB = 7,		/* MailBox name */
    DNS_T_MG = 8,		/* Mail Group */
    DNS_T_MR = 9,		/* Mail Rename */
    DNS_T_NULL = 10,		/* null record (?) */
    DNS_T_WKS = 11,		/* Well Known Service */
    DNS_T_PTR = 12,		/* IPv4 address to name pointer */
    DNS_T_HINFO = 13,		/* Host Info */
    DNS_T_MINFO = 14,		/* Mailbox Info */
    DNS_T_MX = 15,		/* Mail eXchange */
    DNS_T_TXT = 16,		/* text storage */
    DNS_T_RP = 17,		/* Responsible Person */
    DNS_T_AFSDB = 18,		/* AFS Database (?) */
    DNS_T_X25 = 19,		/* X.25 address */
    DNS_T_ISDN = 20,		/* ISDN address */
    DNS_T_RT = 21,		/* Router */
    DNS_T_NSAP = 22,		/* NSAP address */
    /* DNS_T_NSAP_PTR (deprecated) */
    DNS_T_SIG = 24,		/* Security SIGnature */
    DNS_T_KEY = 25,		/* Security key */
    DNS_T_PX = 26,		/* X.400 mail mapping */
    /* DNS_T_GPOS (withdrawn) */
    DNS_T_AAAA = 28,		/* IPv6 address */
    DNS_T_LOC = 29,		/* LOCation information */
    DNS_T_NXT = 30,		/* NeXT domain (security) (?) */
    DNS_T_EID = 31,		/* Endpoint IDentifier */
    DNS_T_NIMLOC = 32,		/* NIMrod LOCator (..zuh?) */
    DNS_T_SRV = 33,		/* Server selection */
    DNS_T_ATMA = 34,		/* ATM Address */
    DNS_T_NAPTR = 35,		/* Naming Authority Pointer */
    DNS_T_ANY = 255		/* Wildcard match. */
} dns_type_t;

/* response codes we might receive from the server */
typedef enum {
    DNS_R_OK = 0,		/* no error */
    DNS_R_BADFORMAT = 1,	/* badly formatted packet */
    DNS_R_SERVFAIL = 2,		/* server failure */
    DNS_R_NXDOMAIN = 3,		/* nonexistant domain */
    DNS_R_NOTIMP = 4,		/* function not implemented */
    DNS_R_REFUSED = 5		/* operation refused */
} dns_rescode_t;

/*
 * Next we define the structures for dns packet headers and dns RRs (resource
 * records).  External users will probably not interact with dns packet
 * headers, and the structure may be moved.
 */
#define dns_int uint32_t

struct dns_packet_header {
    dns_int id:16;		/* question ID */

    /* Below here are the flags, which come in a different order depending on
     * endian-ness. */

#if 0
    dns_int qr:1;		/* response flag */
    dns_int opcode:4;		/* operation code */
    dns_int aa:1;		/* authoritative answer flag */
    dns_int tc:1;		/* truncated message flag */
    dns_int rd:1;		/* recursion desired */
    dns_int ra:1;		/* recursion available */
    dns_int unused:3;		/* unused (by us) flags */
    dns_int rcode:4;		/* response code */
#endif
    dns_int rd:1;
    dns_int tc:1;
    dns_int aa:1;
    dns_int opcode:4;
    dns_int qr:1;
    dns_int rcode:4;
    dns_int unused:3;
    dns_int ra:1;

    dns_int qdcount:16;		/* question count */
    dns_int ancount:16;		/* answer count */
    dns_int nscount:16;		/* authority count */
    dns_int adcount:16;		/* additional count */
};

#undef dns_int

/*
 * Here we define the query and RR (resource record) types.  Appropriate
 * functions are provided to extract these from a raw dns packet.
 */
struct dns_query {
    unsigned char name[DNS_MAX_NAMELEN + 1];/* name being queried */
    uint16_t type;		    /* type desired */
    uint16_t class;		    /* and class desired */
};

struct dns_rr {
    unsigned char name[DNS_MAX_NAMELEN + 1];/* name (if expanded) */
    dns_type_t type;		    /* type of RR */
    dns_class_t class;		    /* and class */
    uint32_t ttl;		    /* time to live */
    uint16_t rdlen;		    /* length of response data */
	unsigned char txt[DNS_MAX_NAMELEN + 1];	    /* textual data */
};

/* the borrowed dn_* functions from bind */
int dn_expand(unsigned char *msg, unsigned char *eomorig, unsigned char *comp_dn, unsigned char *exp_dn, int length);
int dn_comp(unsigned char *exp_dn, unsigned char *comp_dn, int length, unsigned char **dnptrs, unsigned char **lastdnptr);
int dn_skipname(unsigned char *comp_dn, unsigned char *eom);

/* and the external functions from packet.c */
void dns_packet_parse(unsigned char *, size_t);
int dns_lookup_send(void);

void dns_update(void);

#endif
/* vi:set ts=8 sts=4 sw=4 tw=79: */
