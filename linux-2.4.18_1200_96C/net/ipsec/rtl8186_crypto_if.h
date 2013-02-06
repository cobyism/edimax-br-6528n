#ifndef __RTL8186_CRYPTO_IF_H
#define __RTL8186_CRYPTO_IF_H
#include <linux/list.h>
#include "rtl8186_crypto.h"

#define RTL8186_MAX_CRYPT_LEN		1500 // 1720 in fact
#define RTL8186_MAX_DMA_LEN		1600


/* PRNG */
#define RTL8186_PRNG_SET(v)	(*(volatile u32 *)RTL8186_PRNG = v)
#define RTL8186_PRNG_READ()	(*(volatile u32 *)RTL8186_PRNG)

/* Flags */
#define RTL_CRYPT_ASYNC		0x01	/* When done, invocating call_back funcation */

/* Commands */
#define RTL_CRYPT_NO_OP		0x00
#define RTL_CRYPT_ENCODE	0x00
#define RTL_CRYPT_DECODE	0x80 
#define RTL_CRYPT_DES		0x01	// default CBC mode
#define RTL_CRYPT_3DES		0x02	// default CBC mode
#define RTL_CRYPT_AES		0x20	// default CBC mode
# define RTL_CRYPT_ECB		0x10	// Meaningful only for [3]DES/AES, default CBC mode 
#define RTL_CRYPT_HMAC_MD5	0x04
#define RTL_CRYPT_HMAC_SHA1	0x08
# define RTL_CRYPT_APPEND96	0x40	// Meaningful only for HMAC, append ICV to output 

/* Return codes */
#define RTL_CRYPT_DONE			1
#define RTL_CRYPT_OK			0
#define RTL_CRYPT_ERROR			-1
#define RTL_CRYPT_NO_RESOURCE 	-2

struct rtl8186_crypto_request
{
	struct list_head node;	// for request_queue
	unsigned int  id;	// Automatically assigned
	char 	*inbuf;		/* Input buffer */
	char 	*outbuf;	/* Output buffer */
	int 	inlen;		/* Length of input buffer (authenticated range) */
						// inlen <= 1720
	int 	outlen;		/* On input, max size. On Output, real size of output */

	unsigned int flags;					

	volatile unsigned int cmds;	/* Operation to perform */
	int	crypt_offset;	// En/Decrypt Offset, from inbuf, to start operation 
	int crypt_len;		// En/Decrypt Length of operation 

	unsigned char *crypt_key;	// K1(0-7) K2(8-15) K3(16-24)
	unsigned char *crypt_key_d;	// for AES decryption
	unsigned char *crypt_iv;

	//unsigned char hmac_key[20];	
	unsigned char *hmac_key_pad; // OPAD and IPAD	
	unsigned char hmac_result[20];	/* On output, contains the hash result */
	unsigned char hmac_key_len;		// 20: SHA1, 16: MD5
	unsigned char hmac_reslen;		// 12

	void 	(*callback)(struct rtl8186_crypto_request *);	/* function to call when done request. This struct * is passed as argument. */
	void 	*priv;						/* Private to requester */
	
	volatile unsigned char is_done;
};


/* Prototypes */

struct rtl8186_crypto_request *rtl8186_crypto_alloc_request(int flag);
int rtl8186_crypto_execute(struct rtl8186_crypto_request *req);	
int rtl8186_crypto_query_request(struct rtl8186_crypto_request *req);
void rtl8186_crypto_free_request(struct rtl8186_crypto_request *r);
void rtl8186_crypto_set_hmac_key(struct rtl8186_crypto_request *r, char * hmac_key, int len);

#endif // __RTL8186_CRYPTO_IF_H
