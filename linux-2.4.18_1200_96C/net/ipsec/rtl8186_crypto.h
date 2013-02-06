
#ifndef __RTL8186_CRYPTO_HEADER__
#define __RTL8186_CRYPTO_HEADER__

/* Register address */
#define RTL8186_IPSSDAR         0xBD100000      // IPSec Source Descriptor Starting Address Register
#define RTL8186_IPSDDAR		0xBD100004	// IPSec Destination Descriptor Starting Address Register
#define RTL8186_IPSCFR		0xBD100008	// IPSec Configuration Register
#define RTL8186_IPSCR		0xBD100009	// IPSec Command Register
#define RTL8186_IPSIMR		0xBD10000A	// IPSec Interrupt Mast Register
#define RTL8186_IPSISR		0xBD10000B	// IPSec Interrupt Status Register
#define RTL8186_IPSCTR		0xBD10000C	// IPSec Control Register

#define RTL8186_PRNG		0xBD180018

#ifdef BIT
#undef BIT
#endif
#define BIT(x)	( 1 << (x))

#define IPSEC_DEBUG 0
#define DMA_LOOPBACK 0
#define ENABLE_ISR 0
#define AES_FIRST_TEST 0
#define SHOW_TIME 0
#define SHOW_MS 0
#define TEST_AES_OFST_24 0

#define VAR_ALIGNMENT
#define DMA_COPY
#define RESET_EVERYONE // i.e. 1 DD is enough
#define QUETY_CLI	

#ifndef __KERNEL__
	typedef signed char s8;
	typedef unsigned char u8;

	typedef signed short s16;
	typedef unsigned short u16;

	typedef signed int s32;
	typedef unsigned int u32;
#endif

#ifdef RESET_EVERYONE
#define DST_DESC_NUM		1		// number of destination descriptors, up to 64
#define SRC_DESC_NUM		4
#else
#define DST_DESC_NUM		3		// number of destination descriptors, up to 64
//#define DST_DESC_NUM		64
#define SRC_DESC_NUM		8
//#define SRC_DESC_NUM		(4*DST_DESC_NUM)	// number of source descriptors, must >= 4
#endif /* RESET_EVERYONE */
#define DESC_SIZE		32		// 32 bytes
#define KAE_NUM			DST_DESC_NUM		// number of Key Array Elements
#define KAE_SIZE		160		// 160 bytes for Key Array Element
#define KAE_DES_LEN		32
#define KAE_HMAC_LEN		128
#define KAE_DES_HMAC_LEN	160

#define KAE_IV_OFST		24

#define KAE_DES_HMAC_OPAD_OFST	32
#define KAE_DES_HMAC_IPAD_OFST	96

#define KAE_HMAC_OPAD_OFST	0
#define KAE_HMAC_IPAD_OFST	64

#define PAD_NUM			DST_DESC_NUM		// number of paddings
#define PAD_SIZE		64		// 64 bytes for MD5/SHA-1 padding and length of data


// for AES
typedef unsigned char _uint8 ;
typedef unsigned long int _uint32 ;

struct rtl8186_aes_context
{
    int nr;             /* number of rounds */
    _uint32 erk[64];     /* encryption round keys */
    _uint32 drk[64];     /* decryption round keys */
};

int  rtl8186_aes_set_key( struct rtl8186_aes_context *ctx, _uint8 *key, int nbits );

 
enum RTL8186_CRYPTO_DESC_ATTRIBUTE { // i.e. descriptor
	// Offset 0
	OWN_OFST = 0,
	OWN_BY_IPSEC = BIT(31),
	EOR = BIT(30),
	_FS = BIT(29),
	LS = BIT(28),
	PKT_DMA = BIT(27),
	AUL_SFT = 16,
	AUL_MSK = (0x7FF << AUL_SFT),
//	MS_SFT = 14,
	MS_MSK = ( BIT(15) | BIT(14) ),
	MS_CRYPT = 0,
	MS_HMAC = BIT(14),
	MS_HMAC_CRYPT = BIT(15),
	MS_CRYPT_HMAC = ( BIT(15) | BIT(14) ),
	MD5 = BIT(13),
	_3DES = BIT(12),
	AES = BIT(11),
	
	// Src, i.e. Tx
	DDL_SFT = 0,
	DDL_MSK =  (0x7FF << DDL_SFT),
	// Dst, i.e. Rx 
	DBL_SFT = 0,
	DBL_MSK =  (0x7FF << DBL_SFT),

	// Offset 4
	DDI_OFST = 4,
	DDI_SFT = 24,
	DDI_MSK = (0xFF << DDI_SFT),
	A2EO_SFT = 16,
	A2EO_MSK = (0xFF << A2EO_SFT),
	KAM_SFT = 13,
	KAM_MSK = (0x7 << KAM_SFT),
	KAM_ENC = (0x05 << KAM_SFT),
	KAM_DEC = (0x02 << KAM_SFT),
	CBC = BIT(12),
	ENL_SFT = 0,
	ENL_MSK =  (0x7FF << ENL_SFT),
	
	// Offset 8
	SDBP_OFST = 8,
	DDBP_OFST = 8,
	
	// Offset 12
	// Src
	SBDL_OFST = 12,
	SBDL_SFT = 16,
	SBDL_MSK =  (0x7FF << SBDL_SFT),
	SBL_SFT = 0,
	SBL_MSK =  (0x7FF << SBL_SFT),
	// Dst
	ICV_OFST = 12,
	
	// Offset 16
	NDAP_OFST = 16
};



#endif //__RTL8186_CRYPTO_HEADER__

