#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MD5_DEFINED

#define MD5_CBLOCK	64
#define MD5_LBLOCK	16
#define MD5_BLOCK	16
#define MD5_LAST_BLOCK  56
#define MD5_LENGTH_BLOCK 8
#define MD5_DIGEST_LENGTH 16

#define MD5_LONG u32

typedef struct MD5state_st {
	MD5_LONG A,B,C,D;
	MD5_LONG Nl,Nh;
	MD5_LONG data[MD5_LBLOCK];
	int num;
} MD5_CTX;

void MD5_Starts(MD5_CTX *c);
void MD5_Update(MD5_CTX *c, unsigned char *data, MD5_LONG len);
void MD5_Finish(MD5_CTX *c, unsigned char *md);
int MD5_CtxSize(void);
unsigned char *MD5(unsigned char *d, MD5_LONG n, unsigned char *md);

#define ULONG	MD5_LONG
#define UCHAR	unsigned char
#define UINT	unsigned int

#undef c2l
#define c2l(c,l)	(l = ((MD5_LONG)(*((c)++)))     , \
			 l|=(((MD5_LONG)(*((c)++)))<< 8), \
			 l|=(((MD5_LONG)(*((c)++)))<<16), \
			 l|=(((MD5_LONG)(*((c)++)))<<24))

#undef p_c2l
#define p_c2l(c,l,n)	{ \
			switch (n) { \
			case 0: l =((MD5_LONG)(*((c)++))); \
			case 1: l|=((MD5_LONG)(*((c)++)))<< 8; \
			case 2: l|=((MD5_LONG)(*((c)++)))<<16; \
			case 3: l|=((MD5_LONG)(*((c)++)))<<24; \
				} \
			}

/* NOTE the pointer is not incremented at the end of this */
#undef c2l_p
#define c2l_p(c,l,n)	{ \
			l=0; \
			(c)+=n; \
			switch (n) { \
			case 3: l =((MD5_LONG)(*(--(c))))<<16; \
			case 2: l|=((MD5_LONG)(*(--(c))))<< 8; \
			case 1: l|=((MD5_LONG)(*(--(c))))    ; \
				} \
			}

#undef p_c2l_p
#define p_c2l_p(c,l,sc,len) { \
			switch (sc) \
				{ \
			case 0: l =((MD5_LONG)(*((c)++))); \
				if (--len == 0) break; \
			case 1: l|=((MD5_LONG)(*((c)++)))<< 8; \
				if (--len == 0) break; \
			case 2: l|=((MD5_LONG)(*((c)++)))<<16; \
				} \
			}

#undef l2c
#define l2c(l,c)	(*((c)++)=(unsigned char)(((l)    )&0xff), \
			 *((c)++)=(unsigned char)(((l)>> 8)&0xff), \
			 *((c)++)=(unsigned char)(((l)>>16)&0xff), \
			 *((c)++)=(unsigned char)(((l)>>24)&0xff))

/* NOTE - c is not incremented as per l2c */
#undef l2cn
#define l2cn(l1,l2,c,n)	{ \
			c+=n; \
			switch (n) { \
			case 8: *(--(c))=(unsigned char)(((l2)>>24)&0xff); \
			case 7: *(--(c))=(unsigned char)(((l2)>>16)&0xff); \
			case 6: *(--(c))=(unsigned char)(((l2)>> 8)&0xff); \
			case 5: *(--(c))=(unsigned char)(((l2)    )&0xff); \
			case 4: *(--(c))=(unsigned char)(((l1)>>24)&0xff); \
			case 3: *(--(c))=(unsigned char)(((l1)>>16)&0xff); \
			case 2: *(--(c))=(unsigned char)(((l1)>> 8)&0xff); \
			case 1: *(--(c))=(unsigned char)(((l1)    )&0xff); \
				} \
			}

/* A nice byte order reversal from Wei Dai <weidai@eskimo.com> */
#if defined(WIN32)
/* 5 instructions with rotate instruction, else 9 */
#define Endian_Reverse32(a) \
	{ \
	MD5_LONG l=(a); \
	(a)=((ROTATE(l,8)&0x00FF00FF)|(ROTATE(l,24)&0xFF00FF00)); \
	}
#else
/* 6 instructions with rotate instruction, else 8 */
#define Endian_Reverse32(a) \
	{ \
	MD5_LONG l=(a); \
	l=(((l&0xFF00FF00)>>8L)|((l&0x00FF00FF)<<8L)); \
	(a)=ROTATE(l,16L); \
	}
#endif
/*
#define	F(x,y,z)	(((x) & (y))  |  ((~(x)) & (z)))
#define	G(x,y,z)	(((x) & (z))  |  ((y) & (~(z))))
*/

/* As pointed out by Wei Dai <weidai@eskimo.com>, the above can be
 * simplified to the code below.  Wei attributes these optimisations
 * to Peter Gutmann's SHS code, and he attributes it to Rich Schroeppel.
 */
#define	F(x,y,z)	((((y) ^ (z)) & (x)) ^ (z))
#define	G(x,y,z)	((((x) ^ (y)) & (z)) ^ (y))
#define	H(x,y,z)	((x) ^ (y) ^ (z))
#define	I(x,y,z)	(((x) | (~(z))) ^ (y))

#undef ROTATE
#if defined(WIN32)
#define ROTATE(a,n)     _lrotl(a,n)
#else
#define ROTATE(a,n)     (((a)<<(n))|(((a)&0xffffffff)>>(32-(n))))
#endif


#define R0(a,b,c,d,k,s,t) { \
	a+=((k)+(t)+F((b),(c),(d))); \
	a=ROTATE(a,s); \
	a+=b; };\

#define R1(a,b,c,d,k,s,t) { \
	a+=((k)+(t)+G((b),(c),(d))); \
	a=ROTATE(a,s); \
	a+=b; };

#define R2(a,b,c,d,k,s,t) { \
	a+=((k)+(t)+H((b),(c),(d))); \
	a=ROTATE(a,s); \
	a+=b; };

#define R3(a,b,c,d,k,s,t) { \
	a+=((k)+(t)+I((b),(c),(d))); \
	a=ROTATE(a,s); \
	a+=b; };

#define INIT_DATA_A (MD5_LONG)0x67452301L
#define INIT_DATA_B (MD5_LONG)0xefcdab89L
#define INIT_DATA_C (MD5_LONG)0x98badcfeL
#define INIT_DATA_D (MD5_LONG)0x10325476L

static void md5_block(MD5_CTX *c, MD5_LONG *p);
