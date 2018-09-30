#ifdef  __cplusplus
extern "C" {
#endif

#define SHA256_DEFINED
#define SHA256_BLOCK_SIZE 32            // SHA256 outputs a 32 byte digest

	// Note: Original defined WORD as a 32-bit entity and BYTE.  This conflicts
	// with definitions in Windows.  Modified to use REBYTE and REBCNT

	typedef struct {
		REBYTE data[64];
		REBCNT datalen;
		REBU64 bitlen;
		REBCNT state[8];
	} SHA256_CTX;

	/*********************** FUNCTION DECLARATIONS **********************/
	void SHA256_Init(SHA256_CTX *ctx);
	void SHA256_Update(SHA256_CTX *ctx, const REBYTE data[], REBCNT len);
	void SHA256_Final(REBYTE hash[], SHA256_CTX *ctx);
	int  SHA256_CtxSize(void);


#ifdef  __cplusplus
}
#endif