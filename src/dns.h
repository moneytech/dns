/* ==========================================================================
 * dns.h - Restartable DNS Resolver.
 * --------------------------------------------------------------------------
 * Copyright (c) 2009  William Ahern
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the
 * following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 * ==========================================================================
 */
#include <stddef.h>	/* offsetof() */

#include <string.h>	/* strlen(3) */


enum dns_section {
	DNS_S_QD	= 0x01,
	DNS_S_AN	= 0x02,
	DNS_S_NS	= 0x04,
	DNS_S_AR	= 0x08,

	DNS_S_ALL	= 0x0f
}; /* enum dns_section */


enum dns_class {
	DNS_C_IN	= 1,

	DNS_C_ANY	= 255
}; /* enum dns_class */


enum dns_type {
	DNS_T_A		= 1,
	DNS_T_NS	= 2,
	DNS_T_CNAME	= 5,
	DNS_T_SOA	= 6,
	DNS_T_PTR	= 12,
	DNS_T_MX	= 15,
	DNS_T_TXT	= 16,
	DNS_T_AAAA	= 28,
	DNS_T_SRV	= 33,

	DNS_T_ALL	= 255
}; /* enum dns_type */


enum dns_opcode {
	DNS_OP_QUERY	= 0,
	DNS_OP_IQUERY	= 1,
	DNS_OP_STATUS	= 2,
	DNS_OP_NOTIFY	= 4,
	DNS_OP_UPDATE	= 5,
}; /* dns_opcode */


enum dns_rcode {
	DNS_RC_NOERROR	= 0,
	DNS_RC_FORMERR	= 1,
	DNS_RC_SERVFAIL	= 2,
	DNS_RC_NXDOMAIN	= 3,
	DNS_RC_NOTIMP	= 4,
	DNS_RC_REFUSED	= 5,
	DNS_RC_YXDOMAIN	= 6,
	DNS_RC_YXRRSET	= 7,
	DNS_RC_NXRRSET	= 8,
	DNS_RC_NOTAUTH	= 9,
	DNS_RC_NOTZONE	= 10,
	DNS_RC_BADVERS	= 16,
}; /* dns_rcode */


/*
 * NOTE: These string functions need a small buffer in case the literal
 * integer value needs to be printed and returned. UNLESS this buffer is
 * SPECIFIED, the returned string has ONLY BLOCK SCOPE.
 */
#define DNS_STRMINLEN	15

const char *dns_strsection(enum dns_section, void *, size_t);
#define dns_strsection3(a, b, c) \
				dns_strsection((a), (b), (c))
#define dns_strsection1(a)	dns_strsection((a), (char [DNS_STRMINLEN + 1]){ 0 }, DNS_STRMINLEN + 1)
#define dns_strsection(...)	DNS_PP_CALL(DNS_PP_XPASTE(dns_strsection, DNS_PP_NARG(__VA_ARGS__)), __VA_ARGS__)

const char *dns_strclass(enum dns_class, void *, size_t);
#define dns_strclass3(a, b, c)	dns_strclass((a), (b), (c))
#define dns_strclass1(a)	dns_strclass((a), (char [DNS_STRMINLEN + 1]){ 0 }, DNS_STRMINLEN + 1)
#define dns_strclass(...)	DNS_PP_CALL(DNS_PP_XPASTE(dns_strclass, DNS_PP_NARG(__VA_ARGS__)), __VA_ARGS__)

const char *dns_strtype(enum dns_type, void *, size_t);
#define dns_strtype3(a, b, c)	dns_strtype((a), (b), (c))
#define dns_strtype1(a)		dns_strtype((a), (char [DNS_STRMINLEN + 1]){ 0 }, DNS_STRMINLEN + 1)
#define dns_strtype(...)	DNS_PP_CALL(DNS_PP_XPASTE(dns_strtype, DNS_PP_NARG(__VA_ARGS__)), __VA_ARGS__)


struct dns_header {
		unsigned qid:16;

#if BYTE_ORDER == BIG_ENDIAN
		unsigned qr:1;
		unsigned opcode:4;
		unsigned aa:1;
		unsigned tc:1;
		unsigned rd:1;

		unsigned ra:1;
		unsigned unused:3;
		unsigned rcode:4;
#else
		unsigned rd:1;
		unsigned tc:1;
		unsigned aa:1;
		unsigned opcode:4;
		unsigned qr:1;

		unsigned rcode:4;
		unsigned unused:3;
		unsigned ra:1;
#endif

		unsigned qdcount:16;
		unsigned ancount:16;
		unsigned nscount:16;
		unsigned arcount:16;
}; /* struct dns_header */

#define dns_header(p)	((struct dns_header *)&(p)->data[0])


#ifndef DNS_P_DICTSIZE
#define DNS_P_DICTSIZE	8
#endif

struct dns_packet {
	unsigned short dict[DNS_P_DICTSIZE];

	struct { struct dns_packet *cqe_next, *cqe_prev; } cqe;

	size_t size, end;
	unsigned char data[1];
}; /* struct dns_packet */


#define dns_p_new(n)	(dns_p_init((struct dns_packet *)&(union { unsigned char b[offsetof(struct dns_packet, data) + (n)]; struct dns_packet p; }){ { 0 } }, offsetof(struct dns_packet, data) + (n)))

struct dns_packet *dns_p_init(struct dns_packet *, size_t);

#define dns_p_opcode(P)	(dns_header(P)->opcode)

#define dns_p_rcode(P)	(dns_header(P)->rcode)

unsigned dns_p_count(struct dns_packet *, enum dns_section);

int dns_p_push(struct dns_packet *, enum dns_section, const void *, size_t, enum dns_type, enum dns_class, unsigned, const void *);

void dns_p_dictadd(struct dns_packet *, unsigned short);



#define DNS_D_MAXLABEL	63	/* + 1 '\0' */
#define DNS_D_MAXNAME	255	/* + 1 '\0' */

#define DNS_D_ANCHOR	1	/* anchor domain w/ root "." */
#define DNS_D_CLEAVE	2	/* cleave sub-domain */

#define dns_d_new3(a, b, f)	dns_d_init(&(char[DNS_D_MAXNAME + 1]){ 0 }, DNS_D_MAXNAME + 1, (a), (b), (f))
#define dns_d_new2(a, f)	dns_d_new3((a), strlen((a)), (f))
#define dns_d_new1(a)		dns_d_new3((a), strlen((a)), DNS_D_ANCHOR)
#define dns_d_new(...)		DNS_PP_CALL(DNS_PP_XPASTE(dns_d_new, DNS_PP_NARG(__VA_ARGS__)), __VA_ARGS__)

char *dns_d_init(void *, size_t, const void *, size_t, int);

size_t dns_d_anchor(void *, size_t, const void *, size_t);

size_t dns_d_cleave(void *, size_t, const void *, size_t);

size_t dns_d_comp(void *, size_t, const void *, size_t, struct dns_packet *, int *);

size_t dns_d_expand(void *, size_t, unsigned short, struct dns_packet *, int *);

unsigned short dns_d_skip(unsigned short, struct dns_packet *);

int dns_d_push(struct dns_packet *, const void *, size_t);


struct dns_rr {
	enum dns_section section;

	struct {
		unsigned short p;
		unsigned short len;
	} dn;

	enum dns_type type;
	enum dns_class class;
	unsigned ttl;

	struct {
		unsigned short p;
		unsigned short len;
	} rd;
}; /* struct dns_rr */


int dns_rr_copy(struct dns_packet *, struct dns_rr *, struct dns_packet *);

int dns_rr_parse(struct dns_rr *, unsigned short, struct dns_packet *);

unsigned short dns_rr_skip(unsigned short, struct dns_packet *);


#define DNS_RR_I_STATE_INITIALIZER	{ 12, DNS_S_QD, 0 }

#define dns_rr_i_new(...)		(&(struct dns_rr_i){ .state = DNS_RR_I_STATE_INITIALIZER, __VA_ARGS__ })

struct dns_rr_i {
	enum dns_section section;
	const void *name;
	enum dns_type type;
	enum dns_class class;

	struct {
		unsigned short next;
		unsigned short section;
		unsigned short index;
	} state;
}; /* struct dns_rr_i */

struct dns_rr_i *dns_rr_i_init(struct dns_rr_i *);

unsigned dns_rr_grep(struct dns_rr *, unsigned, struct dns_rr_i *, struct dns_packet *, int *);

#define dns_rr_foreach_(rr, P, ...)	\
	for (struct dns_rr_i i##__LINE__ = (struct dns_rr_i){ __VA_ARGS__ }; dns_rr_grep((rr), 1, &i##__LINE__, (P), &(int){ 0 }); )

#define dns_rr_foreach(...)	dns_rr_foreach_(__VA_ARGS__, .state = DNS_RR_I_STATE_INITIALIZER)


struct dns_a {
	unsigned long addr;
}; /* struct dns_a */

int dns_a_parse(struct dns_a *, struct dns_rr *, struct dns_packet *);
int dns_a_push(struct dns_packet *, struct dns_a *);
size_t dns_a_print(void *, size_t, struct dns_a *);


struct dns_aaaa {
	unsigned char addr[16];
}; /* struct dns_aaaa */

int dns_aaaa_parse(struct dns_aaaa *, struct dns_rr *, struct dns_packet *);
int dns_aaaa_push(struct dns_packet *, struct dns_aaaa *);
size_t dns_aaaa_print(void *, size_t, struct dns_aaaa *);


struct dns_mx {
	unsigned short preference;
	char host[256];
}; /* struct dns_mx */

int dns_mx_parse(struct dns_mx *, struct dns_rr *, struct dns_packet *);
int dns_mx_push(struct dns_packet *, struct dns_mx *);
size_t dns_mx_print(void *, size_t, struct dns_mx *);


struct dns_ns {
	char host[256];
}; /* struct dns_ns */

int dns_ns_parse(struct dns_ns *, struct dns_rr *, struct dns_packet *);
int dns_ns_push(struct dns_packet *, struct dns_ns *);
size_t dns_ns_print(void *, size_t, struct dns_ns *);


struct dns_srv {
	
}; /* struct dns_srv */



#ifndef DNS_TXT_MINDATA
#define DNS_TXT_MINDATA	1024
#endif

struct dns_txt {
	size_t size, len;
	unsigned char data[DNS_TXT_MINDATA];
}; /* struct dns_txt */

struct dns_txt *dns_txt_init(struct dns_txt *, size_t);
int dns_txt_parse(struct dns_txt *, struct dns_rr *, struct dns_packet *);
int dns_txt_push(struct dns_packet *, struct dns_txt *);
size_t dns_txt_print(void *, size_t, struct dns_txt *);


union dns_any {
	struct dns_a a;
	struct dns_aaaa aaaa;
	struct dns_mx mx;
	struct dns_ns ns;
	struct dns_srv srv;
	struct dns_txt txt, rdata;
}; /* union dns_any */

union dns_any *dns_any_init(union dns_any *, size_t);
int dns_any_parse(union dns_any *, struct dns_rr *, struct dns_packet *);
int dns_any_push(struct dns_packet *, union dns_any *, enum dns_type);
size_t dns_any_print(void *, size_t, union dns_any *, enum dns_type);


struct dns_hints {

}; /* struct dns_hints */





/*
 * C99 MACRO MAGIC.
 */
#define DNS_PP_NARG_(a, b, c, d, e, f, g, N,...) N
#define DNS_PP_NARG(...)	DNS_PP_NARG_(__VA_ARGS__, 7, 6, 5, 4, 3, 2, 1, 0)
#define DNS_PP_CALL(F, ...)	F(__VA_ARGS__)
#define DNS_PP_PASTE(x, y)	x##y
#define DNS_PP_XPASTE(x, y)	DNS_PP_PASTE(x, y)

