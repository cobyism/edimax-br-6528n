#ifndef	WAPICRYPTO_H
#define	WAPICRYPTO_H	1

#define		ENCRYPT		0     
#define		DECRYPT		1
//#define	RTL_CFG_SMS4_ASM	1

/**
 * \brief          SHA-256 context structure
 */
typedef struct
{
    unsigned long total[2];     /*!< number of bytes processed  */
    unsigned long state[8];     /*!< intermediate digest state  */
    unsigned char buffer[64];   /*!< data block being processed */

    unsigned char ipad[64];     /*!< HMAC: inner padding        */
    unsigned char opad[64];     /*!< HMAC: outer padding        */
    int is224;                  /*!< 0 => SHA-256, else SHA-224 */
}
sha2_context;

/**
 * \brief          SHA-256 context setup
 *
 * \param ctx      context to be initialized
 * \param is224    0 = use SHA256, 1 = use SHA224
 */
void sha2_starts( sha2_context *ctx, int is224 );

/**
 * \brief          SHA-256 process buffer
 *
 * \param ctx      SHA-256 context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void sha2_update( sha2_context *ctx, unsigned char *input, int ilen );

/**
 * \brief          SHA-256 final digest
 *
 * \param ctx      SHA-256 context
 * \param output   SHA-224/256 checksum result
 */
void sha2_finish( sha2_context *ctx, unsigned char output[32] );

/**
 * \brief          Output = SHA-256( input buffer )
 *
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   SHA-224/256 checksum result
 * \param is224    0 = use SHA256, 1 = use SHA224
 */
void sha2( unsigned char *input, int ilen,
           unsigned char output[32], int is224 );

#if 0
/**
 * \brief          Output = SHA-256( file contents )
 *
 * \param path     input file name
 * \param output   SHA-224/256 checksum result
 * \param is224    0 = use SHA256, 1 = use SHA224
 *
 * \return         0 if successful, 1 if fopen failed,
 *                 or 2 if fread failed
 */
int sha2_file( char *path, unsigned char output[32], int is224 );
#endif
/**
 * \brief          SHA-256 HMAC context setup
 *
 * \param ctx      HMAC context to be initialized
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 * \param is224    0 = use SHA256, 1 = use SHA224
 */
void sha2_hmac_starts( sha2_context *ctx, unsigned char *key, int keylen,
                       int is224 );

/**
 * \brief          SHA-256 HMAC process buffer
 *
 * \param ctx      HMAC context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void sha2_hmac_update( sha2_context *ctx, unsigned char *input, int ilen );

/**
 * \brief          SHA-256 HMAC final digest
 *
 * \param ctx      HMAC context
 * \param output   SHA-224/256 HMAC checksum result
 */
void sha2_hmac_finish( sha2_context *ctx, unsigned char output[32] );

/**
 * \brief          Output = HMAC-SHA-256( hmac key, input buffer )
 *
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   HMAC-SHA-224/256 result
 * \param is224    0 = use SHA256, 1 = use SHA224
 */
void sha2_hmac( unsigned char *key, int keylen,
                unsigned char *input, int ilen,
                unsigned char output[32], int is224 );

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int sha2_self_test( int verbose );

typedef	struct _WPISMS4HEADER {
	uint8		keyIdx;
	uint8		reserved;
	uint8		pn[WAPI_PN_LEN];
	uint8		data[0];
}	wpiSMS4Hdr;

#if	defined(RTL_CFG_SMS4_ASM)
typedef	struct __3Pointer {
	void	*ptr1;
	void	*ptr2;
	void	*ptr3;
}	threePointer;
#endif

//extern unsigned short	rtl_SMS4_rxSeq[RX_QUEUE_NUM];
/*
* output = HMAC-SHA-256 ( hmac key, input buffer )
*/
void sha256_hmac( unsigned char *key, int keylen,
                unsigned char *input, int ilen,
                unsigned char *output, int hlen);

void KD_hmac_sha256( unsigned char *key, int keylen,
                unsigned char *input, int ilen,
                unsigned char *output, int hlen);

void WapiSMS4ForMNKEncrypt(uint8 *key, uint8*IV, uint8*input, uint32 inputLength, uint8 *output, uint8 *outputLength, uint32 CryptFlag);
void GenerateRandomData(unsigned char * data, unsigned int len);
#ifdef IRAM_FOR_WIRELESS_AND_WAPI_PERFORMANCE
int32 init_SMS4_CK_Sbox(void);
#endif


void SecSWSMS4Encryption(struct rtl8192cd_priv	*priv, struct tx_insn* txcfg);
int32 SecSWSMS4Decryption(struct rtl8192cd_priv	*priv, struct stat_info *pstat, struct rx_frinfo* pfrinfo);
#endif	/*	WAPICRYPTO_H	*/
