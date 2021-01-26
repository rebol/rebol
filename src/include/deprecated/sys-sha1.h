#ifdef  __cplusplus
extern "C" {
#endif

#define SHA_DEFINED

#define SHA_CBLOCK	64
#define SHA_LBLOCK	16
#define SHA_BLOCK	16
#define SHA_LAST_BLOCK  56
#define SHA_LENGTH_BLOCK 8
#define SHA_DIGEST_LENGTH 20

#define SHA_LONG u32

	typedef struct SHAstate_st
	{
		SHA_LONG h0, h1, h2, h3, h4;
		SHA_LONG Nl, Nh;
		SHA_LONG data[SHA_LBLOCK];
		unsigned int num;
	} SHA_CTX;

	void SHA1_Starts(SHA_CTX *c);
	void SHA1_Update(SHA_CTX *c, unsigned char *data, size_t len);
	void SHA1_Finish(SHA_CTX *c, unsigned char *md);
	int SHA1_CtxSize(void);
	//unsigned char *SHA1(unsigned char *d, SHA_LONG n,unsigned char *md);
	//static void SHA1_Transform(SHA_CTX *c, unsigned char *data);

#ifdef  __cplusplus
}
#endif