#include "8192cd_cfg.h"
#if defined(CONFIG_RTL_WAPI_SUPPORT)

#define _WAPI_CRYPTO_C_

#ifdef __LINUX_2_6__
#ifdef CONFIG_RTL8672
#include "./romeperf.h"
#else
#include <common/rtl_types.h>
#endif
#include <linux/jiffies.h>
#else
#include "../rtl865x/rtl_types.h"
#endif

#include <linux/random.h>
#include "8192cd.h"
#include "wapi_wai.h"
#include "wapiCrypto.h"
#include "8192cd_util.h"


/*
 * 32-bit integer manipulation macros (big endian)
 */
#ifndef GET_ULONG_BE
#define GET_ULONG_BE(n,b,i)                             \
{                                                       \
    (n) = ( (unsigned long) (b)[(i)    ] << 24 )        \
        | ( (unsigned long) (b)[(i) + 1] << 16 )        \
        | ( (unsigned long) (b)[(i) + 2] <<  8 )        \
        | ( (unsigned long) (b)[(i) + 3]       );       \
}
#endif

#ifndef PUT_ULONG_BE
#define PUT_ULONG_BE(n,b,i)                             \
{                                                       \
    (b)[(i)    ] = (unsigned char) ( (n) >> 24 );       \
    (b)[(i) + 1] = (unsigned char) ( (n) >> 16 );       \
    (b)[(i) + 2] = (unsigned char) ( (n) >>  8 );       \
    (b)[(i) + 3] = (unsigned char) ( (n)       );       \
}
#endif

/*
 * SHA-256 context setup
 */
void sha2_starts( sha2_context *ctx, int is224 )
{
    ctx->total[0] = 0;
    ctx->total[1] = 0;

    if( is224 == 0 )
    {
        /* SHA-256 */
        ctx->state[0] = 0x6A09E667;
        ctx->state[1] = 0xBB67AE85;
        ctx->state[2] = 0x3C6EF372;
        ctx->state[3] = 0xA54FF53A;
        ctx->state[4] = 0x510E527F;
        ctx->state[5] = 0x9B05688C;
        ctx->state[6] = 0x1F83D9AB;
        ctx->state[7] = 0x5BE0CD19;
    }
    else
    {
        /* SHA-224 */
        ctx->state[0] = 0xC1059ED8;
        ctx->state[1] = 0x367CD507;
        ctx->state[2] = 0x3070DD17;
        ctx->state[3] = 0xF70E5939;
        ctx->state[4] = 0xFFC00B31;
        ctx->state[5] = 0x68581511;
        ctx->state[6] = 0x64F98FA7;
        ctx->state[7] = 0xBEFA4FA4;
    }

    ctx->is224 = is224;
}

static void sha2_process( sha2_context *ctx, unsigned char data[64] )
{
    unsigned long temp1, temp2, W[64];
    unsigned long A, B, C, D, E, F, G, H;

    GET_ULONG_BE( W[ 0], data,  0 );
    GET_ULONG_BE( W[ 1], data,  4 );
    GET_ULONG_BE( W[ 2], data,  8 );
    GET_ULONG_BE( W[ 3], data, 12 );
    GET_ULONG_BE( W[ 4], data, 16 );
    GET_ULONG_BE( W[ 5], data, 20 );
    GET_ULONG_BE( W[ 6], data, 24 );
    GET_ULONG_BE( W[ 7], data, 28 );
    GET_ULONG_BE( W[ 8], data, 32 );
    GET_ULONG_BE( W[ 9], data, 36 );
    GET_ULONG_BE( W[10], data, 40 );
    GET_ULONG_BE( W[11], data, 44 );
    GET_ULONG_BE( W[12], data, 48 );
    GET_ULONG_BE( W[13], data, 52 );
    GET_ULONG_BE( W[14], data, 56 );
    GET_ULONG_BE( W[15], data, 60 );

#define  SHR(x,n) ((x & 0xFFFFFFFF) >> n)
#define ROTR(x,n) (SHR(x,n) | (x << (32 - n)))

#define S0(x) (ROTR(x, 7) ^ ROTR(x,18) ^  SHR(x, 3))
#define S1(x) (ROTR(x,17) ^ ROTR(x,19) ^  SHR(x,10))

#define S2(x) (ROTR(x, 2) ^ ROTR(x,13) ^ ROTR(x,22))
#define S3(x) (ROTR(x, 6) ^ ROTR(x,11) ^ ROTR(x,25))

#define F0(x,y,z) ((x & y) | (z & (x | y)))
#define F1(x,y,z) (z ^ (x & (y ^ z)))

#define R(t)                                    \
(                                               \
    W[t] = S1(W[t -  2]) + W[t -  7] +          \
           S0(W[t - 15]) + W[t - 16]            \
)

#define P(a,b,c,d,e,f,g,h,x,K)                  \
{                                               \
    temp1 = h + S3(e) + F1(e,f,g) + K + x;      \
    temp2 = S2(a) + F0(a,b,c);                  \
    d += temp1; h = temp1 + temp2;              \
}

    A = ctx->state[0];
    B = ctx->state[1];
    C = ctx->state[2];
    D = ctx->state[3];
    E = ctx->state[4];
    F = ctx->state[5];
    G = ctx->state[6];
    H = ctx->state[7];

    P( A, B, C, D, E, F, G, H, W[ 0], 0x428A2F98 );
    P( H, A, B, C, D, E, F, G, W[ 1], 0x71374491 );
    P( G, H, A, B, C, D, E, F, W[ 2], 0xB5C0FBCF );
    P( F, G, H, A, B, C, D, E, W[ 3], 0xE9B5DBA5 );
    P( E, F, G, H, A, B, C, D, W[ 4], 0x3956C25B );
    P( D, E, F, G, H, A, B, C, W[ 5], 0x59F111F1 );
    P( C, D, E, F, G, H, A, B, W[ 6], 0x923F82A4 );
    P( B, C, D, E, F, G, H, A, W[ 7], 0xAB1C5ED5 );
    P( A, B, C, D, E, F, G, H, W[ 8], 0xD807AA98 );
    P( H, A, B, C, D, E, F, G, W[ 9], 0x12835B01 );
    P( G, H, A, B, C, D, E, F, W[10], 0x243185BE );
    P( F, G, H, A, B, C, D, E, W[11], 0x550C7DC3 );
    P( E, F, G, H, A, B, C, D, W[12], 0x72BE5D74 );
    P( D, E, F, G, H, A, B, C, W[13], 0x80DEB1FE );
    P( C, D, E, F, G, H, A, B, W[14], 0x9BDC06A7 );
    P( B, C, D, E, F, G, H, A, W[15], 0xC19BF174 );
    P( A, B, C, D, E, F, G, H, R(16), 0xE49B69C1 );
    P( H, A, B, C, D, E, F, G, R(17), 0xEFBE4786 );
    P( G, H, A, B, C, D, E, F, R(18), 0x0FC19DC6 );
    P( F, G, H, A, B, C, D, E, R(19), 0x240CA1CC );
    P( E, F, G, H, A, B, C, D, R(20), 0x2DE92C6F );
    P( D, E, F, G, H, A, B, C, R(21), 0x4A7484AA );
    P( C, D, E, F, G, H, A, B, R(22), 0x5CB0A9DC );
    P( B, C, D, E, F, G, H, A, R(23), 0x76F988DA );
    P( A, B, C, D, E, F, G, H, R(24), 0x983E5152 );
    P( H, A, B, C, D, E, F, G, R(25), 0xA831C66D );
    P( G, H, A, B, C, D, E, F, R(26), 0xB00327C8 );
    P( F, G, H, A, B, C, D, E, R(27), 0xBF597FC7 );
    P( E, F, G, H, A, B, C, D, R(28), 0xC6E00BF3 );
    P( D, E, F, G, H, A, B, C, R(29), 0xD5A79147 );
    P( C, D, E, F, G, H, A, B, R(30), 0x06CA6351 );
    P( B, C, D, E, F, G, H, A, R(31), 0x14292967 );
    P( A, B, C, D, E, F, G, H, R(32), 0x27B70A85 );
    P( H, A, B, C, D, E, F, G, R(33), 0x2E1B2138 );
    P( G, H, A, B, C, D, E, F, R(34), 0x4D2C6DFC );
    P( F, G, H, A, B, C, D, E, R(35), 0x53380D13 );
    P( E, F, G, H, A, B, C, D, R(36), 0x650A7354 );
    P( D, E, F, G, H, A, B, C, R(37), 0x766A0ABB );
    P( C, D, E, F, G, H, A, B, R(38), 0x81C2C92E );
    P( B, C, D, E, F, G, H, A, R(39), 0x92722C85 );
    P( A, B, C, D, E, F, G, H, R(40), 0xA2BFE8A1 );
    P( H, A, B, C, D, E, F, G, R(41), 0xA81A664B );
    P( G, H, A, B, C, D, E, F, R(42), 0xC24B8B70 );
    P( F, G, H, A, B, C, D, E, R(43), 0xC76C51A3 );
    P( E, F, G, H, A, B, C, D, R(44), 0xD192E819 );
    P( D, E, F, G, H, A, B, C, R(45), 0xD6990624 );
    P( C, D, E, F, G, H, A, B, R(46), 0xF40E3585 );
    P( B, C, D, E, F, G, H, A, R(47), 0x106AA070 );
    P( A, B, C, D, E, F, G, H, R(48), 0x19A4C116 );
    P( H, A, B, C, D, E, F, G, R(49), 0x1E376C08 );
    P( G, H, A, B, C, D, E, F, R(50), 0x2748774C );
    P( F, G, H, A, B, C, D, E, R(51), 0x34B0BCB5 );
    P( E, F, G, H, A, B, C, D, R(52), 0x391C0CB3 );
    P( D, E, F, G, H, A, B, C, R(53), 0x4ED8AA4A );
    P( C, D, E, F, G, H, A, B, R(54), 0x5B9CCA4F );
    P( B, C, D, E, F, G, H, A, R(55), 0x682E6FF3 );
    P( A, B, C, D, E, F, G, H, R(56), 0x748F82EE );
    P( H, A, B, C, D, E, F, G, R(57), 0x78A5636F );
    P( G, H, A, B, C, D, E, F, R(58), 0x84C87814 );
    P( F, G, H, A, B, C, D, E, R(59), 0x8CC70208 );
    P( E, F, G, H, A, B, C, D, R(60), 0x90BEFFFA );
    P( D, E, F, G, H, A, B, C, R(61), 0xA4506CEB );
    P( C, D, E, F, G, H, A, B, R(62), 0xBEF9A3F7 );
    P( B, C, D, E, F, G, H, A, R(63), 0xC67178F2 );

    ctx->state[0] += A;
    ctx->state[1] += B;
    ctx->state[2] += C;
    ctx->state[3] += D;
    ctx->state[4] += E;
    ctx->state[5] += F;
    ctx->state[6] += G;
    ctx->state[7] += H;
}

/*
 * SHA-256 process buffer
 */
void sha2_update( sha2_context *ctx, unsigned char *input, int ilen )
{
    int fill;
    unsigned long left;

    if( ilen <= 0 )
        return;

    left = ctx->total[0] & 0x3F;
    fill = 64 - left;

    ctx->total[0] += ilen;
    ctx->total[0] &= 0xFFFFFFFF;

    if( ctx->total[0] < (unsigned long) ilen )
        ctx->total[1]++;

    if( left && ilen >= fill )
    {
        memcpy( (void *) (ctx->buffer + left),
                (void *) input, fill );
        sha2_process( ctx, ctx->buffer );
        input += fill;
        ilen  -= fill;
        left = 0;
    }

    while( ilen >= 64 )
    {
        sha2_process( ctx, input );
        input += 64;
        ilen  -= 64;
    }

    if( ilen > 0 )
    {
        memcpy( (void *) (ctx->buffer + left),
                (void *) input, ilen );
    }
}

static const unsigned char sha2_padding[64] =
{
 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*
 * SHA-256 final digest
 */
void sha2_finish( sha2_context *ctx, unsigned char output[32] )
{
    unsigned long last, padn;
    unsigned long high, low;
    unsigned char msglen[8];

    high = ( ctx->total[0] >> 29 )
         | ( ctx->total[1] <<  3 );
    low  = ( ctx->total[0] <<  3 );

    PUT_ULONG_BE( high, msglen, 0 );
    PUT_ULONG_BE( low,  msglen, 4 );

    last = ctx->total[0] & 0x3F;
    padn = ( last < 56 ) ? ( 56 - last ) : ( 120 - last );

    sha2_update( ctx, (unsigned char *) sha2_padding, padn );
    sha2_update( ctx, msglen, 8 );

    PUT_ULONG_BE( ctx->state[0], output,  0 );
    PUT_ULONG_BE( ctx->state[1], output,  4 );
    PUT_ULONG_BE( ctx->state[2], output,  8 );
    PUT_ULONG_BE( ctx->state[3], output, 12 );
    PUT_ULONG_BE( ctx->state[4], output, 16 );
    PUT_ULONG_BE( ctx->state[5], output, 20 );
    PUT_ULONG_BE( ctx->state[6], output, 24 );

    if( ctx->is224 == 0 )
        PUT_ULONG_BE( ctx->state[7], output, 28 );
}

/*
 * output = SHA-256( input buffer )
 */
void sha2( unsigned char *input, int ilen,
           unsigned char output[32], int is224 )
{
    sha2_context ctx;

    sha2_starts( &ctx, is224 );
    sha2_update( &ctx, input, ilen );
    sha2_finish( &ctx, output );

    memset( &ctx, 0, sizeof( sha2_context ) );
}

#if 0
/*
 * output = SHA-256( file contents )
 */
int sha2_file( char *path, unsigned char output[32], int is224 )
{
    FILE *f;
    size_t n;
    sha2_context ctx;
    unsigned char buf[1024];

    if( ( f = fopen( path, "rb" ) ) == NULL )
        return( 1 );

    sha2_starts( &ctx, is224 );

    while( ( n = fread( buf, 1, sizeof( buf ), f ) ) > 0 )
        sha2_update( &ctx, buf, (int) n );

    sha2_finish( &ctx, output );

    memset( &ctx, 0, sizeof( sha2_context ) );

    if( ferror( f ) != 0 )
    {
        fclose( f );
        return( 2 );
    }

    fclose( f );
    return( 0 );
}
#endif

/*
 * SHA-256 HMAC context setup
 */
void sha2_hmac_starts( sha2_context *ctx, unsigned char *key, int keylen,
                       int is224 )
{
    int i;
    unsigned char sum[32];

    if( keylen > 64 )
    {
        sha2( key, keylen, sum, is224 );
        keylen = ( is224 ) ? 28 : 32;
        key = sum;
    }

    memset( ctx->ipad, 0x36, 64 );
    memset( ctx->opad, 0x5C, 64 );

    for( i = 0; i < keylen; i++ )
    {
        ctx->ipad[i] = (unsigned char)( ctx->ipad[i] ^ key[i] );
        ctx->opad[i] = (unsigned char)( ctx->opad[i] ^ key[i] );
    }

    sha2_starts( ctx, is224 );
    sha2_update( ctx, ctx->ipad, 64 );

    memset( sum, 0, sizeof( sum ) );
}

/*
 * SHA-256 HMAC process buffer
 */
void sha2_hmac_update( sha2_context *ctx, unsigned char *input, int ilen )
{
    sha2_update( ctx, input, ilen );
}

/*
 * SHA-256 HMAC final digest
 */
void sha2_hmac_finish( sha2_context *ctx, unsigned char output[32] )
{
    int is224, hlen;
    unsigned char tmpbuf[32];

    is224 = ctx->is224;
    hlen = ( is224 == 0 ) ? 32 : 28;

    sha2_finish( ctx, tmpbuf );
    sha2_starts( ctx, is224 );
    sha2_update( ctx, ctx->opad, 64 );
    sha2_update( ctx, tmpbuf, hlen );
    sha2_finish( ctx, output );

    memset( tmpbuf, 0, sizeof( tmpbuf ) );
}

/*
 * output = HMAC-SHA-256( hmac key, input buffer )
 */
void sha2_hmac( unsigned char *key, int keylen,
                unsigned char *input, int ilen,
                unsigned char output[32], int is224 )
{
    sha2_context ctx;

    sha2_hmac_starts( &ctx, key, keylen, is224 );
    sha2_hmac_update( &ctx, input, ilen );
    sha2_hmac_finish( &ctx, output );

    memset( &ctx, 0, sizeof( sha2_context ) );
}

/*
* output = HMAC-SHA-256 ( hmac key, input buffer )
*/
void sha256_hmac( unsigned char *key, int keylen,
                unsigned char *input, int ilen,
                unsigned char *output, int hlen)
{
	unsigned char temp[32];
	sha2_hmac(key, keylen, input, ilen, temp, 0);
	memcpy(output, temp, hlen);

	memset(temp, 0, 32);
}

void KD_hmac_sha256( unsigned char *key, int keylen,
                unsigned char *input, int ilen,
                unsigned char *output, int hlen)
{
	int i;
	for(i=0;hlen/32;i++, hlen-=32)
	{
		sha256_hmac(key, keylen, input, ilen, &output[i*32], 32);
		input = &output[i*32];
		ilen = 32;
	}

	if (hlen>0)
		sha256_hmac(key, keylen, input, ilen, &output[i*32], hlen);
}
#if (0)

void WapiSMS4Encryption(uint8 *Key, uint8 *IV, uint8 *Input, uint16 InputLength,
                                                    uint8 *Output, uint16 *OutputLength);

/*
* WAPI Encrypt test
*/

static unsigned char data_before_encrypt[1][96] =
{
	0x88, 0x41, 0x00, 0x00, 0x00, 0x0B, 0xC0, 0x02,
	0x30, 0x73, 0x00, 0xE0, 0x4C, 0x81, 0x72, 0x0A,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x40, 0x5C, 0x36, 0x5C,
	0x36, 0x5C, 0x36, 0x5C, 0x36, 0x5C, 0x36, 0x5C,
	0x36, 0x5C, 0x36, 0x5C, 0xAA, 0xAA, 0x03, 0x00,
	0x00, 0x00, 0x08, 0x06, 0x00, 0x01, 0x08, 0x00,
	0x06, 0x04, 0x00, 0x01, 0x00, 0xE0, 0x4C, 0x81,
	0x72, 0x0A, 0xC0, 0xA8, 0x01, 0x7B, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xC0, 0xA8, 0x01, 0x7B,
	0x07, 0xE7, 0x81, 0x88, 0x2F, 0x98, 0xDF, 0xDD,
	0x9A, 0x23, 0xE2, 0x74, 0xA6, 0xEF, 0x35, 0xC1
};

static unsigned char data_before_encrypt_len[1] = {96};

static unsigned char data_before_encrypt_key[1][16] =
{
	0x83, 0x32, 0x29, 0x16, 0xDE, 0x93, 0x76, 0x38,
	0xAC, 0x13, 0x2F, 0xB2, 0xD4, 0x9B, 0xCA, 0x5A
};

static unsigned char data_before_encrypt_PN[1][16] =
{
	0x40, 0x5C, 0x36, 0x5C, 0x36, 0x5C, 0x36, 0x5C,
	0x36, 0x5C, 0x36, 0x5C, 0x36, 0x5C, 0x36, 0x5C
};

static unsigned char data_after_encrypt[1][96] =
{
	0x88, 0x41, 0x00, 0x00, 0x00, 0x0B, 0xC0, 0x02,
	0x30, 0x73, 0x00, 0xE0, 0x4C, 0x81, 0x72, 0x0A,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x40, 0x5C, 0x36, 0x5C,
	0x36, 0x5C, 0x36, 0x5C, 0x36, 0x5C, 0x36, 0x5C,
	0x36, 0x5C, 0x36, 0x5C, 0xE4, 0x0B, 0xA5, 0xDF,
	0x57, 0x10, 0xA5, 0x0C, 0x81, 0x38, 0xB8, 0xAE,
	0x32, 0xE9, 0x40, 0x6E, 0x57, 0xEC, 0x03, 0xA4,
	0x7F, 0x3C, 0x85, 0x1E, 0xC7, 0xF8, 0xA8, 0x8C,
	0xAA, 0xA7, 0xBC, 0xA7, 0xDC, 0xC8, 0x54, 0x60,
	0xFE, 0xC6, 0xD1, 0x0D, 0x8D, 0x79, 0x0E, 0xED,
	0xB4, 0xAA, 0x10, 0x6E, 0xBF, 0xE1, 0x86, 0xE3
};

/*
* WAPI test vectors
*/

static unsigned char wapi_digest_test_key[4][37] =
{
	{0x01,0x02,0x03,0x04,0x05,0x06,0x7,0x08,
	  0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,
	  0x011,0x12,0x13,0x14,0x15,0x16,0x17,0x18,
	  0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x20},
	  {0x01,0x02,0x03,0x04,0x05,0x06,0x7,0x08,
	  0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,
	  0x011,0x12,0x13,0x14,0x15,0x16,0x17,0x18,
	  0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x20,
	  0x21,0x22,0x23,0x24,0x25},
	  {0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,
	  0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,
	  0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,
	  0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b},
	  {0x4a,0x65,0x66,0x65}
};

static unsigned char wapi_digest_test_keylen[4] =
{ 32, 37, 32, 4 };

static unsigned char wapi_digest_test_buf[4][112] =
{
	{"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopqabcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"},
	{0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,
	0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd,0xcd},
	{"Hi There"},
	{"what do ya want for nothing?"}
};

static unsigned char wapi_digest_test_buflen[4] =
{ 112, 50, 8, 28 };

static unsigned char wapi_digest_test_output[4][32] =
{
	{0x47,0x03,0x05,0xfc,0x7e,0x40,0xfe,0x34,0xd3,0xee,0xb3,0xe7,0x73,0xd9,0x5a,0xab,0x73,0xac,0xf0,0xfd,0x06,0x04,0x47,0xa5,0xeb,0x45,0x95,0xbf,0x33,0xa9,0xd1,0xa3 },
	{0xd4,0x63,0x3c,0x17,0xf6,0xfb,0x8d,0x74,0x4c,0x66,0xde,0xe0,0xf8,0xf0,0x74,0x55,0x6e,0xc4,0xaf,0x55,0xef,0x07,0x99,0x85,0x41,0x46,0x8e,0xb4,0x9b,0xd2,0xe9,0x17 },
	{0x19,0x8a,0x60,0x7e,0xb4,0x4b,0xfb,0xc6,0x99,0x03,0xa0,0xf1,0xcf,0x2b,0xbd,0xc5,0xba,0x0a,0xa3,0xf3,0xd9,0xae,0x3c,0x1c,0x7a,0x3b,0x16,0x96,0xa0,0xb6,0x8c,0xf7 },
	{0x5b,0xdc,0xc1,0x46,0xbf,0x60,0x75,0x4e,0x6a,0x04,0x24,0x26,0x08,0x95,0x75,0xc7,0x5a,0x00,0x3f,0x08,0x9d,0x27,0x39,0x83,0x9d,0xec,0x58,0xb9,0x64,0xec,0x38,0x43}
};

static unsigned char wapi_test_key[3][37] =
{
	{0x01,0x02,0x03,0x04,0x05,0x06,0x7,0x08,
	  0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,
	  0x011,0x12,0x13,0x14,0x15,0x16,0x17,0x18,
	  0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x20},
	{0x01,0x02,0x03,0x04,0x05,0x06,0x7,0x08,
	  0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,
	  0x011,0x12,0x13,0x14,0x15,0x16,0x17,0x18,
	  0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x20,
	  0x21,0x22,0x23,0x24,0x25},
	{0x01,0x02,0x03,0x04,0x05,0x06,0x7,0x08,
	  0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,
	  0x011,0x12,0x13,0x14,0x15}
};

static const int wapi_test_keylen[3] =
{
	32, 37, 16
};

static unsigned char wapi_test_buf[3][49] =
{
	{"pairwise key expansion for infrastructure unicast"},
	{"group key expansion for multicast and broadcast"},
	{"pre-share key expansion for adhoc network"}
};

static const int wapi_test_buflen[3] =
{
	49, 47, 41
};

static unsigned char wapi_test_output[9][48] =
{
	/*	1-3	*/
	{0xe3,0xa6,0x45,0x46,0xf2,0xd1,0xf5,0xee,0xb7,0xd1,0xee,0x06,0xd2,0xc9,0xe5,0x4a,0x2c,0xc9,0xd6,0xce,0xc3,0xb7,0x6f,0xfd,0x62,0x63,0xf4,0x26,0xdc,0x25,0x39,0xaf,0xbd,0x98,0x80,0xa5,0x27,0xa1,0xb5,0x85,0x59,0x4b,0x57,0xce,0x33,0x21,0x4f,0x0c},
	{0x3b,0x6e,0xca,0x4f,0x08,0x76,0xc4,0x3a,0xb3,0x1b,0x26,0x3f,0x2c,0x38,0xb8,0x81,0x21,0xb5,0x68,0xe5,0xf8,0xfd,0x1d,0x4c,0xfa,0x4c,0x7f,0x8c,0x60,0x97,0x04,0x3d,0x7b,0x40,0xa8,0x63,0xb9,0x43,0xb9,0xf5,0xbb,0x37,0x2f,0x3a,0xdd,0xa5,0xda,0x27},
	{0xbc,0x29,0xf3,0xe6,0x09,0x1f,0x6a,0xc9,0x0b,0xa0,0x20,0x61,0x92,0x12,0x48,0x69,0x5f,0xee,0xff,0x1a,0x4c,0xab,0x53,0x3b,0x11,0x67,0xd8,0x54,0x5f,0x93,0x5f,0x28,0x11,0x84,0xc9,0xbb,0x32,0xf9,0x87,0xb9,0x86,0x81,0x0f,0xfb,0x17,0xc4,0x10,0xf5},
	/*	4-6	*/
	{
	0x20,0x8f,0x72,0x54,0xa4,0xbf,0x56,0xf0,0xfa,0x49,0x5f,0xe1,
	0x0c,0x99,0x15,0x05,0x92,0xed,0x79,0xdf,0x57,0x74,0xa9,0x6e,
	0x13,0x97,0x1e,0xc4,0xa1,0x5e,0x16,0xa7,0xed,0x75,0xf5,0xe5,
	0x44,0xbb,0xd3,0x35,0x67,0xeb,0x88,0xe7,0x83,0x24,0xa9,0xd2
	},
	{
	0x33,0x32,0x61,0x7a,0x90,0x8e,0xa5,0xa0,0x7f,0xfa,0x1d,0x23,
	0x79,0xf3,0xd8,0x3e,0x8b,0xe9,0x14,0x1f,0x15,0x53,0x8f,0xd3,
	0xef,0xde,0x58,0x01,0x19,0xe8,0xc5,0x09,0x5d,0x25,0xb2,0xd3,
	0x0a,0xc7,0xa6,0x35,0xad,0xb4,0x3c,0x6c,0xac,0xf0,0xaa,0x2b
	},
	{
	0xf2,0xcb,0xf1,0x1c,0x6d,0x40,0xb8,0x09,0xd0,0xc0,0xed,0x48,
	0x2a,0x4a,0x1b,0x6a,0x15,0x1a,0xf1,0xfb,0x4c,0x80,0xf9,0x80,
	0x5c,0x93,0xe5,0x6e,0xb1,0xcf,0x5c,0xb5,0xec,0xc1,0x3e,0x7a,
	0xbc,0xaf,0xe0,0xa7,0xd2,0x59,0x5d,0x51,0x9b,0x76,0x9a,0x24
	},
	/*	7-9	*/
	{
	0xc0,0x7a,0xd8,0x32,0x25,0x2a,0x0c,0x14,0x76,0x18,0xf4,0xc0,
	0xd0,0x6b,0x35,0xf4,0xf6,0xd6,0x73,0x5d,0x1a,0xa3,0x8e,0x47,
	0x9a,0x7e,0xe0,0xac,0x1c,0x0c,0x38,0x5b,0x2d,0x33,0x28,0x74,
	0x1e,0x4d,0xa0,0xc8,0x76,0xfc,0x6c,0xc9,0xe3,0x60,0xc8,0xd7
	},
	{
	0xf0,0x0b,0xee,0xf2,0xf5,0x5f,0x85,0xd8,0xee,0xb0,0x6f,0x8c,
	0xc4,0x1b,0xe6,0x0e,0xc2,0x69,0xf5,0x82,0x9a,0x0b,0x6e,0xfb,
	0x2d,0x9b,0x49,0x5e,0xb1,0x87,0xd3,0x58,0x59,0x68,0x88,0xc3,
	0xd2,0x6f,0x94,0x9f,0x8d,0x2e,0x41,0xfe,0xbc,0xbb,0xb9,0x9a
	},
	{
	0x05,0x8e,0xb8,0x7c,0xff,0x82,0x66,0x47,0xde,0x50,0x7b,0x14,
	0x17,0xac,0x99,0x6e,0xb5,0x7f,0xcf,0x11,0xfd,0xfc,0x83,0xbe,
	0x59,0xd5,0x85,0xf4,0xa7,0x3e,0x69,0x7d,0xd4,0x38,0xe3,0x34,
	0xfe,0xbb,0x06,0x7d,0x14,0x6f,0x01,0x31,0xa6,0x96,0x4f,0x26
	},
};

static const int wapi_test_outputlen[3] =
{
	48, 48, 48
};


/*
 * FIPS-180-2 test vectors
 */
static unsigned char sha2_test_buf[3][57] =
{
    { "abc" },
    { "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq" },
    { "" }
};

static const int sha2_test_buflen[3] =
{
    3, 56, 1000
};

static const unsigned char sha2_test_sum[6][32] =
{
    /*
     * SHA-224 test vectors
     */
    { 0x23, 0x09, 0x7D, 0x22, 0x34, 0x05, 0xD8, 0x22,
      0x86, 0x42, 0xA4, 0x77, 0xBD, 0xA2, 0x55, 0xB3,
      0x2A, 0xAD, 0xBC, 0xE4, 0xBD, 0xA0, 0xB3, 0xF7,
      0xE3, 0x6C, 0x9D, 0xA7 },
    { 0x75, 0x38, 0x8B, 0x16, 0x51, 0x27, 0x76, 0xCC,
      0x5D, 0xBA, 0x5D, 0xA1, 0xFD, 0x89, 0x01, 0x50,
      0xB0, 0xC6, 0x45, 0x5C, 0xB4, 0xF5, 0x8B, 0x19,
      0x52, 0x52, 0x25, 0x25 },
    { 0x20, 0x79, 0x46, 0x55, 0x98, 0x0C, 0x91, 0xD8,
      0xBB, 0xB4, 0xC1, 0xEA, 0x97, 0x61, 0x8A, 0x4B,
      0xF0, 0x3F, 0x42, 0x58, 0x19, 0x48, 0xB2, 0xEE,
      0x4E, 0xE7, 0xAD, 0x67 },

    /*
     * SHA-256 test vectors
     */
    { 0xBA, 0x78, 0x16, 0xBF, 0x8F, 0x01, 0xCF, 0xEA,
      0x41, 0x41, 0x40, 0xDE, 0x5D, 0xAE, 0x22, 0x23,
      0xB0, 0x03, 0x61, 0xA3, 0x96, 0x17, 0x7A, 0x9C,
      0xB4, 0x10, 0xFF, 0x61, 0xF2, 0x00, 0x15, 0xAD },
    { 0x24, 0x8D, 0x6A, 0x61, 0xD2, 0x06, 0x38, 0xB8,
      0xE5, 0xC0, 0x26, 0x93, 0x0C, 0x3E, 0x60, 0x39,
      0xA3, 0x3C, 0xE4, 0x59, 0x64, 0xFF, 0x21, 0x67,
      0xF6, 0xEC, 0xED, 0xD4, 0x19, 0xDB, 0x06, 0xC1 },
    { 0xCD, 0xC7, 0x6E, 0x5C, 0x99, 0x14, 0xFB, 0x92,
      0x81, 0xA1, 0xC7, 0xE2, 0x84, 0xD7, 0x3E, 0x67,
      0xF1, 0x80, 0x9A, 0x48, 0xA4, 0x97, 0x20, 0x0E,
      0x04, 0x6D, 0x39, 0xCC, 0xC7, 0x11, 0x2C, 0xD0 }
};

/*
 * RFC 4231 test vectors
 */
static unsigned char sha2_hmac_test_key[7][26] =
{
    { "\x0B\x0B\x0B\x0B\x0B\x0B\x0B\x0B\x0B\x0B\x0B\x0B\x0B\x0B\x0B\x0B"
      "\x0B\x0B\x0B\x0B" },
    { "Jefe" },
    { "\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA"
      "\xAA\xAA\xAA\xAA" },
    { "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x10"
      "\x11\x12\x13\x14\x15\x16\x17\x18\x19" },
    { "\x0C\x0C\x0C\x0C\x0C\x0C\x0C\x0C\x0C\x0C\x0C\x0C\x0C\x0C\x0C\x0C"
      "\x0C\x0C\x0C\x0C" },
    { "" }, /* 0xAA 131 times */
    { "" }
};

static const int sha2_hmac_test_keylen[7] =
{
    20, 4, 20, 25, 20, 131, 131
};

static unsigned char sha2_hmac_test_buf[7][153] =
{
    { "Hi There" },
    { "what do ya want for nothing?" },
    { "\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD"
      "\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD"
      "\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD"
      "\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD"
      "\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD" },
    { "\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
      "\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
      "\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
      "\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
      "\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD" },
    { "Test With Truncation" },
    { "Test Using Larger Than Block-Size Key - Hash Key First" },
    { "This is a test using a larger than block-size key "
      "and a larger than block-size data. The key needs to "
      "be hashed before being used by the HMAC algorithm." }
};

static const int sha2_hmac_test_buflen[7] =
{
    8, 28, 50, 50, 20, 54, 152
};

static const unsigned char sha2_hmac_test_sum[14][32] =
{
    /*
     * HMAC-SHA-224 test vectors
     */
    { 0x89, 0x6F, 0xB1, 0x12, 0x8A, 0xBB, 0xDF, 0x19,
      0x68, 0x32, 0x10, 0x7C, 0xD4, 0x9D, 0xF3, 0x3F,
      0x47, 0xB4, 0xB1, 0x16, 0x99, 0x12, 0xBA, 0x4F,
      0x53, 0x68, 0x4B, 0x22 },
    { 0xA3, 0x0E, 0x01, 0x09, 0x8B, 0xC6, 0xDB, 0xBF,
      0x45, 0x69, 0x0F, 0x3A, 0x7E, 0x9E, 0x6D, 0x0F,
      0x8B, 0xBE, 0xA2, 0xA3, 0x9E, 0x61, 0x48, 0x00,
      0x8F, 0xD0, 0x5E, 0x44 },
    { 0x7F, 0xB3, 0xCB, 0x35, 0x88, 0xC6, 0xC1, 0xF6,
      0xFF, 0xA9, 0x69, 0x4D, 0x7D, 0x6A, 0xD2, 0x64,
      0x93, 0x65, 0xB0, 0xC1, 0xF6, 0x5D, 0x69, 0xD1,
      0xEC, 0x83, 0x33, 0xEA },
    { 0x6C, 0x11, 0x50, 0x68, 0x74, 0x01, 0x3C, 0xAC,
      0x6A, 0x2A, 0xBC, 0x1B, 0xB3, 0x82, 0x62, 0x7C,
      0xEC, 0x6A, 0x90, 0xD8, 0x6E, 0xFC, 0x01, 0x2D,
      0xE7, 0xAF, 0xEC, 0x5A },
    { 0x0E, 0x2A, 0xEA, 0x68, 0xA9, 0x0C, 0x8D, 0x37,
      0xC9, 0x88, 0xBC, 0xDB, 0x9F, 0xCA, 0x6F, 0xA8 },
    { 0x95, 0xE9, 0xA0, 0xDB, 0x96, 0x20, 0x95, 0xAD,
      0xAE, 0xBE, 0x9B, 0x2D, 0x6F, 0x0D, 0xBC, 0xE2,
      0xD4, 0x99, 0xF1, 0x12, 0xF2, 0xD2, 0xB7, 0x27,
      0x3F, 0xA6, 0x87, 0x0E },
    { 0x3A, 0x85, 0x41, 0x66, 0xAC, 0x5D, 0x9F, 0x02,
      0x3F, 0x54, 0xD5, 0x17, 0xD0, 0xB3, 0x9D, 0xBD,
      0x94, 0x67, 0x70, 0xDB, 0x9C, 0x2B, 0x95, 0xC9,
      0xF6, 0xF5, 0x65, 0xD1 },

    /*
     * HMAC-SHA-256 test vectors
     */
    { 0xB0, 0x34, 0x4C, 0x61, 0xD8, 0xDB, 0x38, 0x53,
      0x5C, 0xA8, 0xAF, 0xCE, 0xAF, 0x0B, 0xF1, 0x2B,
      0x88, 0x1D, 0xC2, 0x00, 0xC9, 0x83, 0x3D, 0xA7,
      0x26, 0xE9, 0x37, 0x6C, 0x2E, 0x32, 0xCF, 0xF7 },
    { 0x5B, 0xDC, 0xC1, 0x46, 0xBF, 0x60, 0x75, 0x4E,
      0x6A, 0x04, 0x24, 0x26, 0x08, 0x95, 0x75, 0xC7,
      0x5A, 0x00, 0x3F, 0x08, 0x9D, 0x27, 0x39, 0x83,
      0x9D, 0xEC, 0x58, 0xB9, 0x64, 0xEC, 0x38, 0x43 },
    { 0x77, 0x3E, 0xA9, 0x1E, 0x36, 0x80, 0x0E, 0x46,
      0x85, 0x4D, 0xB8, 0xEB, 0xD0, 0x91, 0x81, 0xA7,
      0x29, 0x59, 0x09, 0x8B, 0x3E, 0xF8, 0xC1, 0x22,
      0xD9, 0x63, 0x55, 0x14, 0xCE, 0xD5, 0x65, 0xFE },
    { 0x82, 0x55, 0x8A, 0x38, 0x9A, 0x44, 0x3C, 0x0E,
      0xA4, 0xCC, 0x81, 0x98, 0x99, 0xF2, 0x08, 0x3A,
      0x85, 0xF0, 0xFA, 0xA3, 0xE5, 0x78, 0xF8, 0x07,
      0x7A, 0x2E, 0x3F, 0xF4, 0x67, 0x29, 0x66, 0x5B },
    { 0xA3, 0xB6, 0x16, 0x74, 0x73, 0x10, 0x0E, 0xE0,
      0x6E, 0x0C, 0x79, 0x6C, 0x29, 0x55, 0x55, 0x2B },
    { 0x60, 0xE4, 0x31, 0x59, 0x1E, 0xE0, 0xB6, 0x7F,
      0x0D, 0x8A, 0x26, 0xAA, 0xCB, 0xF5, 0xB7, 0x7F,
      0x8E, 0x0B, 0xC6, 0x21, 0x37, 0x28, 0xC5, 0x14,
      0x05, 0x46, 0x04, 0x0F, 0x0E, 0xE3, 0x7F, 0x54 },
    { 0x9B, 0x09, 0xFF, 0xA7, 0x1B, 0x94, 0x2F, 0xCB,
      0x27, 0x63, 0x5F, 0xBC, 0xD5, 0xB0, 0xE9, 0x44,
      0xBF, 0xDC, 0x63, 0x64, 0x4F, 0x07, 0x13, 0x93,
      0x8A, 0x7F, 0x51, 0x53, 0x5C, 0x3A, 0x35, 0xE2 }
};

/*
 * Checkup routine
 */
int sha2_self_test( int verbose )
{
    int i, j, k, buflen;
    unsigned char buf[1024];
    unsigned char sha2sum[32];
    sha2_context ctx;

    for( i = 0; i < 6; i++ )
    {
        j = i % 3;
        k = i < 3;

        if( verbose != 0 )
            printk( "  SHA-%d test #%d: ", 256 - k * 32, j + 1 );

        sha2_starts( &ctx, k );

        if( j == 2 )
        {
            memset( buf, 'a', buflen = 1000 );

            for( j = 0; j < 1000; j++ )
                sha2_update( &ctx, buf, buflen );
        }
        else
            sha2_update( &ctx, sha2_test_buf[j],
                               sha2_test_buflen[j] );

        sha2_finish( &ctx, sha2sum );

        if( memcmp( sha2sum, sha2_test_sum[i], 32 - k * 4 ) != 0 )
        {
            if( verbose != 0 )
                printk( "failed\n" );

            return( 1 );
        }

        if( verbose != 0 )
            printk( "passed\n" );
    }

    if( verbose != 0 )
        printk( "\n" );

    for( i = 0; i < 14; i++ )
    {
        j = i % 7;
        k = i < 7;

        if( verbose != 0 )
            printk( "  HMAC-SHA-%d test #%d: ", 256 - k * 32, j + 1 );

        if( j == 5 || j == 6 )
        {
            memset( buf, '\xAA', buflen = 131 );
            sha2_hmac_starts( &ctx, buf, buflen, k );
        }
        else
            sha2_hmac_starts( &ctx, sha2_hmac_test_key[j],
                                    sha2_hmac_test_keylen[j], k );

        sha2_hmac_update( &ctx, sha2_hmac_test_buf[j],
                                sha2_hmac_test_buflen[j] );

        sha2_hmac_finish( &ctx, sha2sum );

        buflen = ( j == 4 ) ? 16 : 32 - k * 4;

        if( memcmp( sha2sum, sha2_hmac_test_sum[i], buflen ) != 0 )
        {
            if( verbose != 0 )
                printk( "failed\n" );

            return( 1 );
        }

        if( verbose != 0 )
            printk( "passed\n" );
    }

    if( verbose != 0 )
        printk( "\n" );

    for (i=0;i<4;i++)
    {
    	sha256_hmac(&wapi_digest_test_key[i][0], wapi_digest_test_keylen[i],
		&wapi_digest_test_buf[i][0], wapi_digest_test_buflen[i],
		buf, 32);
	if ( memcmp(buf, wapi_digest_test_output[i], 32) )
		printk("WAPI digest test failed: Case[%d]\n", i);
	else
		printk("WAPI digest test Passed: Case[%d]\n", i);
    }

    for(i=0;i<3;i++)
    {
    	for(j=0;j<3;j++)
    	{
#if 1
    		KD_hmac_sha256(&wapi_test_key[j][0],wapi_test_keylen[j],
			&wapi_test_buf[i][0], wapi_test_buflen[i],
			buf, 48);
#else
		KD_hmac_sha256(wapi_test_buf[i],wapi_test_buflen[i],
			wapi_test_key[i], wapi_test_keylen[i],
			buf, wapi_test_outputlen[i]);
#endif
		if ( memcmp(buf, &wapi_test_output[(i*3)+j][0], 48))
		{
			printk("WAPI test failed: Case[%d]:Index[%d]\n", i,j);
		}
		else
			printk("WAPI test Passed: Case[%d]:Index[%d]\n", i,j);
    	}
    }

#if 0
	printk("Start test WAPI Encryption:\n");
	/*	test wapi encryption	*/
	{
		uint16	buflen2;

		buflen2 = 0;
		WapiSMS4Encryption(&data_before_encrypt_key[0][0], &data_before_encrypt_PN[0][0],
			&data_before_encrypt[0][0], data_before_encrypt_len[0], buf, &buflen2);

		if (buflen2!=data_before_encrypt_len[0])
			printk("WAPI Encryption length error: [%d]:[%d]\n", data_before_encrypt_len[0], buflen2);
		else
			printk("WAPI Encryption length OK: [%d]:[%d]\n", data_before_encrypt_len[0], buflen2);

		if (memcmp(buf, &data_after_encrypt[0][0], buflen2))
		{
			printk("WAPI Encryption failed.\n");
			//memDump(buf, 16, "DATA");
		}
		else
			printk("WAPI Encryption OK.\n");
	}
#endif

    return( 0 );
}

#endif

#if 1

typedef unsigned char muint8;
typedef unsigned short muint16;
typedef unsigned int muint32;

static muint8		wapiCryptoTemp[16];
static muint8		wapiBlockIn[16], wapiTempBlock[16];
static muint32		wapiRK[32];
static muint8		wapiDecrytBuf[MAXDATALEN];
static muint8		wapiDecrytHdrBuf[64];

#ifdef IRAM_FOR_WIRELESS_AND_WAPI_PERFORMANCE
__DRAM_WLAN_HI
#else
const
#endif
muint8 Sbox[256] = {
0xd6,0x90,0xe9,0xfe,0xcc,0xe1,0x3d,0xb7,0x16,0xb6,0x14,0xc2,0x28,0xfb,0x2c,0x05,
0x2b,0x67,0x9a,0x76,0x2a,0xbe,0x04,0xc3,0xaa,0x44,0x13,0x26,0x49,0x86,0x06,0x99,
0x9c,0x42,0x50,0xf4,0x91,0xef,0x98,0x7a,0x33,0x54,0x0b,0x43,0xed,0xcf,0xac,0x62,
0xe4,0xb3,0x1c,0xa9,0xc9,0x08,0xe8,0x95,0x80,0xdf,0x94,0xfa,0x75,0x8f,0x3f,0xa6,
0x47,0x07,0xa7,0xfc,0xf3,0x73,0x17,0xba,0x83,0x59,0x3c,0x19,0xe6,0x85,0x4f,0xa8,
0x68,0x6b,0x81,0xb2,0x71,0x64,0xda,0x8b,0xf8,0xeb,0x0f,0x4b,0x70,0x56,0x9d,0x35,
0x1e,0x24,0x0e,0x5e,0x63,0x58,0xd1,0xa2,0x25,0x22,0x7c,0x3b,0x01,0x21,0x78,0x87,
0xd4,0x00,0x46,0x57,0x9f,0xd3,0x27,0x52,0x4c,0x36,0x02,0xe7,0xa0,0xc4,0xc8,0x9e,
0xea,0xbf,0x8a,0xd2,0x40,0xc7,0x38,0xb5,0xa3,0xf7,0xf2,0xce,0xf9,0x61,0x15,0xa1,
0xe0,0xae,0x5d,0xa4,0x9b,0x34,0x1a,0x55,0xad,0x93,0x32,0x30,0xf5,0x8c,0xb1,0xe3,
0x1d,0xf6,0xe2,0x2e,0x82,0x66,0xca,0x60,0xc0,0x29,0x23,0xab,0x0d,0x53,0x4e,0x6f,
0xd5,0xdb,0x37,0x45,0xde,0xfd,0x8e,0x2f,0x03,0xff,0x6a,0x72,0x6d,0x6c,0x5b,0x51,
0x8d,0x1b,0xaf,0x92,0xbb,0xdd,0xbc,0x7f,0x11,0xd9,0x5c,0x41,0x1f,0x10,0x5a,0xd8,
0x0a,0xc1,0x31,0x88,0xa5,0xcd,0x7b,0xbd,0x2d,0x74,0xd0,0x12,0xb8,0xe5,0xb4,0xb0,
0x89,0x69,0x97,0x4a,0x0c,0x96,0x77,0x7e,0x65,0xb9,0xf1,0x09,0xc5,0x6e,0xc6,0x84,
0x18,0xf0,0x7d,0xec,0x3a,0xdc,0x4d,0x20,0x79,0xee,0x5f,0x3e,0xd7,0xcb,0x39,0x48
};
#ifdef IRAM_FOR_WIRELESS_AND_WAPI_PERFORMANCE
__DRAM_WLAN_HI
#else
const
#endif
muint32 CK[32] = {
	0x00070e15, 0x1c232a31, 0x383f464d, 0x545b6269,
	0x70777e85, 0x8c939aa1, 0xa8afb6bd, 0xc4cbd2d9,
	0xe0e7eef5, 0xfc030a11, 0x181f262d, 0x343b4249,
	0x50575e65, 0x6c737a81, 0x888f969d, 0xa4abb2b9,
	0xc0c7ced5, 0xdce3eaf1, 0xf8ff060d, 0x141b2229,
	0x30373e45, 0x4c535a61, 0x686f767d, 0x848b9299,
	0xa0a7aeb5, 0xbcc3cad1, 0xd8dfe6ed, 0xf4fb0209,
	0x10171e25, 0x2c333a41, 0x484f565d, 0x646b7279 };

#define Rotl(_x, _y) (((_x) << (_y)) | ((_x) >> (32 - (_y))))

#if defined(RTL_CFG_SMS4_ASM)
static threePointer                     para;
extern muint32 __ByteSub(muint32 _A);
extern muint32 __L1(muint32 _B);
extern muint32 __L2(muint32 _B);
extern void __xor_block(void *dst, void *src1, void *src2);
extern void SMS4Crypt(muint8 *Input, muint8 *Output, muint32 *rk);
extern void SMS4KeyExt(muint8 *Key, muint32 *rk);
extern void WapiSMS4Cryption(muint8 *Key, muint8 *IV,
                                                        muint16 InputLength,
                                                        threePointer *para);

#define	ByteSub(_A)	(__ByteSub(_A))
#define	L1(_B)		(__L1(_B))
#define	L2(_B)		(__L2(_B))
#define	xor_block	__xor_block
#define	WapiSMS4Encryption	WapiSMS4Cryption
#define	WapiSMS4Decryption	WapiSMS4Cryption
#else
#define ByteSub(_A) (Sbox[(_A) >> 24] << 24 ^ \
                     Sbox[(_A) >> 16 & 0xFF] << 16 ^ \
                     Sbox[(_A) >>  8 & 0xFF] <<  8 ^ \
                     Sbox[(_A) & 0xFF])

#define L1(_B) ((_B) ^ Rotl(_B, 2) ^ Rotl(_B, 10) ^ Rotl(_B, 18) ^ Rotl(_B, 24))
#define L2(_B) ((_B) ^ Rotl(_B, 13) ^ Rotl(_B, 23))
static inline void
xor_block(void *dst, void *src1, void *src2)
/* 128-bit xor: *dst = *src1 xor *src2. Pointers must be 32-bit aligned  */
{
    ((unsigned *)dst)[0] = ((unsigned *)src1)[0] ^ ((unsigned *)src2)[0];
    ((unsigned *)dst)[1] = ((unsigned *)src1)[1] ^ ((unsigned *)src2)[1];
    ((unsigned *)dst)[2] = ((unsigned *)src1)[2] ^ ((unsigned *)src2)[2];
    ((unsigned *)dst)[3] = ((unsigned *)src1)[3] ^ ((unsigned *)src2)[3];
}
#ifdef IRAM_FOR_WIRELESS_AND_WAPI_PERFORMANCE
int32 init_SMS4_CK_Sbox(void)
{
	static muint8 Sbox_tmp[256] = {
		0xd6,0x90,0xe9,0xfe,0xcc,0xe1,0x3d,0xb7,0x16,0xb6,0x14,0xc2,0x28,0xfb,0x2c,0x05,
		0x2b,0x67,0x9a,0x76,0x2a,0xbe,0x04,0xc3,0xaa,0x44,0x13,0x26,0x49,0x86,0x06,0x99,
		0x9c,0x42,0x50,0xf4,0x91,0xef,0x98,0x7a,0x33,0x54,0x0b,0x43,0xed,0xcf,0xac,0x62,
		0xe4,0xb3,0x1c,0xa9,0xc9,0x08,0xe8,0x95,0x80,0xdf,0x94,0xfa,0x75,0x8f,0x3f,0xa6,
		0x47,0x07,0xa7,0xfc,0xf3,0x73,0x17,0xba,0x83,0x59,0x3c,0x19,0xe6,0x85,0x4f,0xa8,
		0x68,0x6b,0x81,0xb2,0x71,0x64,0xda,0x8b,0xf8,0xeb,0x0f,0x4b,0x70,0x56,0x9d,0x35,
		0x1e,0x24,0x0e,0x5e,0x63,0x58,0xd1,0xa2,0x25,0x22,0x7c,0x3b,0x01,0x21,0x78,0x87,
		0xd4,0x00,0x46,0x57,0x9f,0xd3,0x27,0x52,0x4c,0x36,0x02,0xe7,0xa0,0xc4,0xc8,0x9e,
		0xea,0xbf,0x8a,0xd2,0x40,0xc7,0x38,0xb5,0xa3,0xf7,0xf2,0xce,0xf9,0x61,0x15,0xa1,
		0xe0,0xae,0x5d,0xa4,0x9b,0x34,0x1a,0x55,0xad,0x93,0x32,0x30,0xf5,0x8c,0xb1,0xe3,
		0x1d,0xf6,0xe2,0x2e,0x82,0x66,0xca,0x60,0xc0,0x29,0x23,0xab,0x0d,0x53,0x4e,0x6f,
		0xd5,0xdb,0x37,0x45,0xde,0xfd,0x8e,0x2f,0x03,0xff,0x6a,0x72,0x6d,0x6c,0x5b,0x51,
		0x8d,0x1b,0xaf,0x92,0xbb,0xdd,0xbc,0x7f,0x11,0xd9,0x5c,0x41,0x1f,0x10,0x5a,0xd8,
		0x0a,0xc1,0x31,0x88,0xa5,0xcd,0x7b,0xbd,0x2d,0x74,0xd0,0x12,0xb8,0xe5,0xb4,0xb0,
		0x89,0x69,0x97,0x4a,0x0c,0x96,0x77,0x7e,0x65,0xb9,0xf1,0x09,0xc5,0x6e,0xc6,0x84,
		0x18,0xf0,0x7d,0xec,0x3a,0xdc,0x4d,0x20,0x79,0xee,0x5f,0x3e,0xd7,0xcb,0x39,0x48
		};

	static muint32 CK_tmp[32] = {
		0x00070e15, 0x1c232a31, 0x383f464d, 0x545b6269,
		0x70777e85, 0x8c939aa1, 0xa8afb6bd, 0xc4cbd2d9,
		0xe0e7eef5, 0xfc030a11, 0x181f262d, 0x343b4249,
		0x50575e65, 0x6c737a81, 0x888f969d, 0xa4abb2b9,
		0xc0c7ced5, 0xdce3eaf1, 0xf8ff060d, 0x141b2229,
		0x30373e45, 0x4c535a61, 0x686f767d, 0x848b9299,
		0xa0a7aeb5, 0xbcc3cad1, 0xd8dfe6ed, 0xf4fb0209,
		0x10171e25, 0x2c333a41, 0x484f565d, 0x646b7279 };

	memcpy(Sbox,Sbox_tmp, sizeof(muint8)*256);
	memcpy(CK,CK_tmp,sizeof(muint32)*32);

	return SUCCESS;
}
#endif

#ifdef IRAM_FOR_WIRELESS_AND_WAPI_PERFORMANCE
__IRAM_WLAN_HI
static void SMS4Crypt(muint8 *Input, muint8 *Output, muint32 *rk)
#else
static void SMS4Crypt(muint8 *Input, muint8 *Output, muint32 *rk)
#endif
{
	 muint32 r, mid, x0, x1, x2, x3, *p;
	 p = (muint32 *)Input;
	 x0 = p[0];
	 x1 = p[1];
	 x2 = p[2];
	 x3 = p[3];
#ifdef LITTLE_ENDIAN
	 x0 = Rotl(x0, 16); x0 = ((x0 & 0x00FF00FF) << 8) ^ ((x0 & 0xFF00FF00) >> 8);
	 x1 = Rotl(x1, 16); x1 = ((x1 & 0x00FF00FF) << 8) ^ ((x1 & 0xFF00FF00) >> 8);
	 x2 = Rotl(x2, 16); x2 = ((x2 & 0x00FF00FF) << 8) ^ ((x2 & 0xFF00FF00) >> 8);
	 x3 = Rotl(x3, 16); x3 = ((x3 & 0x00FF00FF) << 8) ^ ((x3 & 0xFF00FF00) >> 8);
#endif
	 for (r = 0; r < 32; r += 4)
	 {
		  mid = x1 ^ x2 ^ x3 ^ rk[r + 0];
		  mid = ByteSub(mid);
		  x0 ^= L1(mid);
		  mid = x2 ^ x3 ^ x0 ^ rk[r + 1];
		  mid = ByteSub(mid);
		  x1 ^= L1(mid);
		  mid = x3 ^ x0 ^ x1 ^ rk[r + 2];
		  mid = ByteSub(mid);
		  x2 ^= L1(mid);
		  mid = x0 ^ x1 ^ x2 ^ rk[r + 3];
		  mid = ByteSub(mid);
		  x3 ^= L1(mid);
	 }
#ifdef LITTLE_ENDIAN
	 x0 = Rotl(x0, 16); x0 = ((x0 & 0x00FF00FF) << 8) ^ ((x0 & 0xFF00FF00) >> 8);
	 x1 = Rotl(x1, 16); x1 = ((x1 & 0x00FF00FF) << 8) ^ ((x1 & 0xFF00FF00) >> 8);
	 x2 = Rotl(x2, 16); x2 = ((x2 & 0x00FF00FF) << 8) ^ ((x2 & 0xFF00FF00) >> 8);
	 x3 = Rotl(x3, 16); x3 = ((x3 & 0x00FF00FF) << 8) ^ ((x3 & 0xFF00FF00) >> 8);
#endif
	 p = (muint32 *)Output;
	 p[0] = x3;
	 p[1] = x2;
	 p[2] = x1;
	 p[3] = x0;
}

#ifdef IRAM_FOR_WIRELESS_AND_WAPI_PERFORMANCE
__IRAM_WLAN_HI
static void SMS4KeyExt(muint8 *Key, muint32 *rk)
#else
static void SMS4KeyExt(muint8 *Key, muint32 *rk)
#endif
{
	 muint32 r, mid, x0, x1, x2, x3, *p;
	 p = (muint32 *)Key;
	 x0 = p[0];
	 x1 = p[1];
	 x2 = p[2];
	 x3 = p[3];
#ifdef LITTLE_ENDIAN
	 x0 = Rotl(x0, 16); x0 = ((x0 & 0xFF00FF) << 8) ^ ((x0 & 0xFF00FF00) >> 8);
	 x1 = Rotl(x1, 16); x1 = ((x1 & 0xFF00FF) << 8) ^ ((x1 & 0xFF00FF00) >> 8);
	 x2 = Rotl(x2, 16); x2 = ((x2 & 0xFF00FF) << 8) ^ ((x2 & 0xFF00FF00) >> 8);
	 x3 = Rotl(x3, 16); x3 = ((x3 & 0xFF00FF) << 8) ^ ((x3 & 0xFF00FF00) >> 8);
#endif

	 x0 ^= 0xa3b1bac6;
	 x1 ^= 0x56aa3350;
	 x2 ^= 0x677d9197;
	 x3 ^= 0xb27022dc;
	 for (r = 0; r < 32; r += 4)
	 {
		  mid = x1 ^ x2 ^ x3 ^ CK[r + 0];
		  mid = ByteSub(mid);
		  rk[r + 0] = x0 ^= L2(mid);
		  mid = x2 ^ x3 ^ x0 ^ CK[r + 1];
		  mid = ByteSub(mid);
		  rk[r + 1] = x1 ^= L2(mid);
		  mid = x3 ^ x0 ^ x1 ^ CK[r + 2];
		  mid = ByteSub(mid);
		  rk[r + 2] = x2 ^= L2(mid);
		  mid = x0 ^ x1 ^ x2 ^ CK[r + 3];
		  mid = ByteSub(mid);
		  rk[r + 3] = x3 ^= L2(mid);
	 }
}

#ifdef IRAM_FOR_WIRELESS_AND_WAPI_PERFORMANCE
__MIPS16
__IRAM_WLAN_HI
#endif
void WapiSMS4Cryption(muint8 *Key, muint8 *IV, muint8 *Input, muint16 InputLength,
                                                muint8 *Output, muint16 *OutputLength)
{
	muint32   blockNum,i,j;
	muint16	remainder;
        muint8  *pBlockIn, *pBlockOut, *pBlockTemp;

	remainder = InputLength & 0x0F;
	blockNum = InputLength >> 4;

	for(i=0;i<16;i++)
		wapiBlockIn[i] = IV[15-i];

      SMS4KeyExt((muint8 *)Key, wapiRK);

 	pBlockIn = wapiBlockIn;
	pBlockOut = wapiTempBlock;
	for(i=0; i<blockNum; i++)
	{
		SMS4Crypt((muint8 *)pBlockIn, pBlockOut, wapiRK);
		xor_block(Output, Input, pBlockOut);
		pBlockTemp = pBlockIn;
		pBlockIn = pBlockOut;
		pBlockOut = pBlockTemp;
		Output += 16;
		Input += 16;
	}

	if (remainder>0)
	{
		*OutputLength = (i<<4) + remainder;
		SMS4Crypt((muint8 *)pBlockIn, pBlockOut, wapiRK);

	for(j=0; j<remainder; j++)
	{
			Output[j] = Input[j] ^ pBlockOut[j];
		}
	}
	else
		*OutputLength = i<<4;
}

#if 1
#define	WapiSMS4Encryption	WapiSMS4Cryption
#define	WapiSMS4Decryption	WapiSMS4Cryption
#else
void WapiSMS4Encryption(muint8 *Key, muint8 *IV, muint8 *Input, muint16 InputLength,
                                                    muint8 *Output, muint16 *OutputLength)
{

    WapiSMS4Cryption(Key, IV, Input, InputLength, Output, OutputLength, ENCRYPT);
}
#ifdef IRAM_FOR_WIRELESS_AND_WAPI_PERFORMANCE
__MIPS16
__IRAM_WLAN_HI
#else
#endif
void WapiSMS4Decryption(muint8 *Key, muint8 *IV, muint8 *Input, muint16 InputLength,
                                                    muint8 *Output, muint16 *OutputLength)
{
    // OFB mode: is also ENCRYPT flag
    WapiSMS4Cryption(Key, IV, Input, InputLength, Output, OutputLength, ENCRYPT);
}
#endif
#endif


#ifdef IRAM_FOR_WIRELESS_AND_WAPI_PERFORMANCE
__MIPS16
__IRAM_WLAN_HI
#endif
void WapiSMS4CalculateMic(muint8 *Key, muint8 *IV, muint8 *Input1, muint8 Input1Length,
                                                 muint8 *Input2, muint16 Input2Length, muint8 *Output, muint8 *OutputLength)
{
	muint32   blockNum,i;
	muint32	remainder;

	remainder = Input1Length & 0x0F;
	blockNum = Input1Length >> 4;

	for(i=0;i<16;i++)
		wapiBlockIn[i] = IV[15-i];

	SMS4KeyExt((muint8 *)Key, wapiRK);
	SMS4Crypt((muint8 *)wapiBlockIn, Output, wapiRK);

	for(i=0; i<blockNum; i++)
	{
	      xor_block(wapiBlockIn, Input1, Output);
		SMS4Crypt((muint8 *)wapiBlockIn, Output, wapiRK);
		Input1 += 16;
	}

	if(remainder !=0)
	{
	      memset(wapiTempBlock+remainder, 0, 16-remainder);
	      memcpy(wapiTempBlock, Input1, remainder);

             xor_block(wapiBlockIn, wapiTempBlock, Output);
        	SMS4Crypt((muint8 *)wapiBlockIn, Output, wapiRK);
      }

	remainder = Input2Length & 0x0F;
	blockNum = Input2Length >> 4;

  	for(i=0; i<blockNum; i++)
	{
	      xor_block(wapiBlockIn, Input2, Output);
		SMS4Crypt((muint8 *)wapiBlockIn, Output, wapiRK);
		Input2 += 16;
	}

	if(remainder !=0)
	{
	      memset(wapiTempBlock+remainder, 0, 16-remainder);
	      memcpy(wapiTempBlock, Input2, remainder);

            xor_block(wapiBlockIn, wapiTempBlock, Output);
            SMS4Crypt((muint8 *)wapiBlockIn, Output, wapiRK);
	}

	*OutputLength = 16;
}

#endif

void WapiSMS4ForMNKEncrypt(uint8 *key, uint8*IV, uint8*input, uint32 inputLength, uint8 *output, uint8 *outputLength, uint32 CryptFlag)
{
	uint32	blockNum,i,j;
	uint32	remainder;

	*outputLength = 0;
	remainder = inputLength % 16;
	blockNum = inputLength/16;
	if(remainder !=0)
		blockNum++;

	if(remainder !=0)
	{
		for(j= inputLength;j<16*blockNum;j++)
		{
			input[j] = 0;
		}
	}

	memcpy(wapiBlockIn,IV,16);
	SMS4KeyExt((uint8 *)key, wapiRK);

	for(i=0;i<blockNum;i++)
	{
		SMS4Crypt(wapiBlockIn, wapiTempBlock, wapiRK);
		*outputLength+=16;

		for(j=0;j<16;j++)
		{
			output[i*16+j] = input[i*16+j] ^ wapiTempBlock[j];
		}

		memcpy(wapiBlockIn,wapiTempBlock,16);
	}

}

#ifdef IRAM_FOR_WIRELESS_AND_WAPI_PERFORMANCE
//__MIPS16
__IRAM_WLAN_HI
#endif
void SecCalculateMicSMS4(
	uint8		KeyIdx,
	uint8*		MicKey,
	uint8*		pHeader,
	uint8*		pData,
	uint16		DataLen,
	uint8*		MicBuffer
	)
{
	static uint8      TempBuf[34];
	uint8      TempLen;
	uint8      MicLen;
	uint8      QosOffset;
	uint8*     IV;
	uint16*     pTemp;

	/*	construct the first part	*/
#if 0
	memcpy(TempBuf, pHeader, 2); //FrameCtrl
#else
	*((uint16*)TempBuf) = *((uint16*)pHeader);
#endif
	pTemp = (uint16*)TempBuf;

#if 0
#ifdef LITTLE_ENDIAN
	*pTemp &= 0xc78f;       //bit4,5,6,11,12,13
#else
	*pTemp &= 0x8fc7;
#endif
#else
	*((unsigned short*)TempBuf) &= 0x8fc7;
#endif
	memcpy((TempBuf+2), (pHeader+4), 12); //Addr1, Addr2
	memcpy((TempBuf+14), (pHeader+22), 2); // SeqCtrl
	pTemp = (uint16*)(TempBuf + 14);

#if 0
#ifdef LITTLE_ENDIAN
	*pTemp &= 0x000f;
#else
	*pTemp &= 0x0f00;
#endif
#else
	*((uint16*)(TempBuf+14)) &= 0x0f00;
#endif
	memcpy((TempBuf+16), (pHeader+16), 6); //Addr3

	if (get_tofr_ds(pHeader) == 3)
	{
		memcpy((TempBuf+22), (pHeader+24), 6);
		QosOffset = 30;
	}
	else
	{
		memset((TempBuf+22), 0, 6);
		QosOffset = 24;
	}

	if( is_qos_data(pHeader) ) //QosCtrl
	{
		memcpy((TempBuf+28), (pHeader+QosOffset), 2);
		TempLen = 34;
		IV = pHeader + QosOffset + 4;
	}
	else
	{
		TempLen = 32;
		IV = pHeader + QosOffset + 2;
	}

	TempBuf[TempLen-1] = (uint8)(DataLen & 0xff);
	TempBuf[TempLen-2] = (uint8)((DataLen & 0xff00)>>8);
	TempBuf[TempLen-3] = 0;
	TempBuf[TempLen-4] = KeyIdx;

	WapiSMS4CalculateMic(MicKey, IV, TempBuf, TempLen,
                 pData, DataLen, MicBuffer, &MicLen);


	wapiAssert(MicLen==16);
}

#if 1
/* WAPI SW Enc: must have done Coalesce!*/
/*
#ifdef IRAM_FOR_WIRELESS_AND_WAPI_PERFORMANCE
//__MIPS16
__IRAM_WLAN_HI
#endif
*/
void SecSWSMS4Encryption(struct rtl8192cd_priv	*priv, struct tx_insn* txcfg)
{
	uint16				OutputLength;
	wapiStaInfo			*wapiInfo;
	uint8				*pHeader;
	uint8				KeyIdx;
	uint8				*MicBuffer;
	uint8				*pMicKey;
	uint8				*pDataKey;
	uint8				*pPN;
	wpiSMS4Hdr			*SMS4Hdr;
/*	uint8				pTemp[SMS4_MIC_LEN];		*/
	uint8				*pBuf;
	int32				bPNOverflow;

#if 0
	if (txcfg->fr_type != _SKB_FRAME_TYPE_)
		return;
#endif
	pHeader = txcfg->phdr;	/*	txcfg->hdr_len	*/
	{
		WAPI_LOCK(&priv->pshare->lock);
		if ( !txcfg->pstat )
		{
			/* multicast */
			KeyIdx = priv->wapiMCastKeyId;
			pPN = priv->txMCast;
			pMicKey = priv->wapiMCastKey[KeyIdx].micKey;
			pDataKey = priv->wapiMCastKey[KeyIdx].dataKey;
			bPNOverflow = WapiIncreasePN(pPN, 1);
			if (bPNOverflow==WAPI_RETURN_SUCCESS)
			{
				/*	MSK update	*/
				wapiUpdateMSK(priv, NULL);
			}
			priv->wapiMCastKeyUpdateCnt--;
			if (priv->wapiMCastKeyUpdateCnt<=0)
			{
				priv->wapiMCastKeyUpdateCnt = priv->pmib->wapiInfo.wapiUpdateMCastKeyPktNum;
				if (priv->pmib->wapiInfo.wapiUpdateMCastKeyType==wapi_pktnum_update||
					priv->pmib->wapiInfo.wapiUpdateMCastKeyType==wapi_all_update)
				{
					/*	MSK update	*/
					wapiUpdateMSK(priv, NULL);
				}
			}
		}
		else
		{
			/* unicast */
			wapiAssert(txcfg);
			wapiAssert(txcfg->pstat);
			wapiAssert(txcfg->pstat->wapiInfo);
			wapiInfo = txcfg->pstat->wapiInfo;
			KeyIdx = wapiInfo->wapiUCastKeyId;
			pPN = wapiInfo->wapiPN.txUCast;
			pMicKey = wapiInfo->wapiUCastKey[KeyIdx].micKey;
			pDataKey = wapiInfo->wapiUCastKey[KeyIdx].dataKey;
			bPNOverflow = WapiIncreasePN(pPN, 2);
			if (bPNOverflow==WAPI_RETURN_SUCCESS)
			{
				/*	USK update	*/
				wapiUpdateUSK(priv, txcfg->pstat);
			}

			wapiInfo->wapiUCastKeyUpdateCnt--;
			if (wapiInfo->wapiUCastKeyUpdateCnt<=0)
			{
				wapiInfo->wapiUCastKeyUpdateCnt = priv->pmib->wapiInfo.wapiUpdateUCastKeyPktNum;

				if (wapiInfo->priv->pmib->wapiInfo.wapiUpdateUCastKeyType==wapi_pktnum_update||
					wapiInfo->priv->pmib->wapiInfo.wapiUpdateUCastKeyType==wapi_all_update)
				{
					/*	USK update	*/
					wapiUpdateUSK(priv, txcfg->pstat);
				}
			}
		}

		wapiAssert((((struct sk_buff *)txcfg->pframe)->end-((struct sk_buff *)txcfg->pframe)->tail)>SMS4_MIC_LEN);
		pBuf = ((struct sk_buff *)txcfg->pframe)->data-8;
		MicBuffer = ((struct sk_buff *)txcfg->pframe)->data+txcfg->fr_len;
#if 0
		memcpy(wapiCryptoTemp, pBuf+txcfg->fr_len+8, SMS4_MIC_LEN);
#endif
#if 1
		*((uint32*)pBuf) = 0xAAAA0300;
		*((uint16*)(pBuf+4)) = 0;
#else
		memcpy(pBuf, rfc1042_header, WLAN_LLC_HEADER_SIZE);
#endif
		/* pBuf[6]&pBuf[7] == etherType	*/

		SMS4Hdr = (wpiSMS4Hdr*)(pHeader + txcfg->hdr_len);
		SMS4Hdr->keyIdx = KeyIdx;
		SMS4Hdr->reserved = 0;
		memcpy(SMS4Hdr->pn, pPN, WAPI_PN_LEN);

		SecCalculateMicSMS4(KeyIdx, pMicKey, pHeader, pBuf, txcfg->fr_len+8, MicBuffer);

#if 0
		memcpy(pBuf+txcfg->fr_len+8, MicBuffer, SMS4_MIC_LEN);
#endif
		/* encryption for data	*/
#if	defined(RTL_CFG_SMS4_ASM)
		para.ptr1 = pBuf;
		para.ptr2 = pBuf;
		para.ptr3 = &OutputLength;

                WapiSMS4Encryption(pDataKey, pPN, txcfg->fr_len+8+SMS4_MIC_LEN, &para);
#else
		WapiSMS4Encryption(pDataKey, pPN, pBuf, txcfg->fr_len+8+SMS4_MIC_LEN, pBuf, &OutputLength);
#endif
		wapiAssert(OutputLength==txcfg->fr_len+8+SMS4_MIC_LEN);

#if 0
		memcpy(SMS4Hdr->data, pBuf, 8);
#else
		*((uint32*)SMS4Hdr->data) = *((uint32*)pBuf);
		*((uint32*)(SMS4Hdr->data+4)) = *((uint32*)(pBuf+4));
#endif
#if 0
		memcpy(MicBuffer, pBuf+txcfg->fr_len+8, SMS4_MIC_LEN);
		memcpy(pBuf+txcfg->fr_len+8, wapiCryptoTemp, SMS4_MIC_LEN);	/* for skb share info */
#endif
		WAPI_UNLOCK(&priv->pshare->lock);
	}
}

/*
#ifdef IRAM_FOR_WIRELESS_AND_WAPI_PERFORMANCE
__IRAM_WLAN_HI
#endif
*/
int32 SecSWSMS4Decryption(
	struct rtl8192cd_priv	*priv, struct stat_info *pstat, struct rx_frinfo* pfrinfo)
{
	uint8				PNOffset;
	uint8				DataOffset;
	uint16				OutputLength;
	wapiStaInfo			*wapiInfo;
	uint8				*pHeader;
	uint8				*pRA;
	uint8				*pTA;
	uint8				KeyIdx;
/*	uint8				MicBuffer[SMS4_MIC_LEN];	*/
	uint8				*pMicKey;
	uint8				*pDataKey;
	uint8				*pRecvPN;
	uint8				*pSecData;
	uint8				*pRecvMic;
	uint16				DataLen;
	uint16				PktLen;
	wpiSMS4Hdr			*SMS4Hdr;
	uint8				qosIdx=0;
	struct stat_info 		*rxPstat;

	WAPI_LOCK(&priv->pshare->lock);
	wapiInfo = pstat->wapiInfo;
	pHeader = pfrinfo->pskb->data;
#if 0
	if (get_tofr_ds(pHeader) == 3)
	{
		if( is_qos_data(pHeader) )
		{
			PNOffset = WLAN_HDR_A4_QOS_LEN;
		}
		else
		{
			PNOffset = WLAN_HDR_A4_LEN;
		}
	}
	else
	{
		if( is_qos_data(pHeader) )
		{
			PNOffset = WLAN_HDR_A3_QOS_LEN;
		}
		else
		{
			PNOffset = WLAN_HDR_A3_LEN;
		}
	}
	printk("rx pn offset %d %d\n", pfrinfo->hdr_len, PNOffset);
#else
	PNOffset = pfrinfo->hdr_len;
#endif

	DataOffset = PNOffset + WAPI_EXT_LEN;

	SMS4Hdr = (wpiSMS4Hdr*)(pHeader+PNOffset);

	pRA = pHeader + 4;
	pTA = pHeader + 10;
	KeyIdx = SMS4Hdr->keyIdx;
	pRecvPN = SMS4Hdr->pn;
	pSecData = pHeader + DataOffset;
	PktLen = pfrinfo->pktlen;
	DataLen = PktLen-DataOffset;
	pRecvMic = pHeader + PktLen - SMS4_MIC_LEN - WAPI_ALIGNMENT_OFFSET;

	if( (*pRA)&0x1 )
	{
		rxPstat = get_stainfo(priv, pTA);
		if (rxPstat==NULL || rxPstat->wapiInfo==NULL ||
			rxPstat->wapiInfo->wapiMCastEnable==0 ||
			(KeyIdx!=priv->wapiMCastKeyId&&priv->wapiMCastKeyUpdate==0))
		{

			WAPI_UNLOCK(&priv->pshare->lock);
			return FAIL;
		}

		/*	need check the PN increasing, and record the last rx PN	*/
		/*	need check the PN increasing, and record the last rx PN	*/
		if (WapiComparePN(pRecvPN, priv->rxMCast)==WAPI_RETURN_FAILED)
		{
			WAPI_UNLOCK(&priv->pshare->lock);
			return FAIL;
		}

		memcpy(priv->rxMCast, pRecvPN, WAPI_PN_LEN);

		pMicKey = priv->wapiMCastKey[KeyIdx].micKey;
		pDataKey = priv->wapiMCastKey[KeyIdx].dataKey;
	}
	else
	{
		if (wapiInfo->wapiUCastRxEnable==0
			|| (KeyIdx!=wapiInfo->wapiUCastKeyId&&priv->wapiMCastKeyUpdate==0))
		{
			WAPI_UNLOCK(&priv->pshare->lock);
			return FAIL;
		}

#ifdef SEMI_QOS
		{
			if (pfrinfo->tid==0||pfrinfo->tid==3)
				qosIdx = 0;
			else if (pfrinfo->tid<3)
				qosIdx = 1;
			else if (pfrinfo->tid<6)
				qosIdx = 2;
			else
				qosIdx = 3;
		}
#endif

		/*	need check the PN increasing, and record the last rx PN	*/
		if ((pRecvPN[WAPI_PN_LEN-1]&0x1)==0x1)
		{
			wapiAssert(0);
			WAPI_UNLOCK(&priv->pshare->lock);
			return FAIL;
		}

#if 0
		if (WapiComparePN(pRecvPN, &wapiInfo->wapiPN.rxUCast[qosIdx][0])==WAPI_RETURN_FAILED)
		{
			memcpy(wapiCryptoTemp, pRecvPN, WAPI_PN_LEN);
			WapiIncreasePN(wapiCryptoTemp, ((rtl_SMS4_rxSeq[qosIdx]-pfrinfo->seq+1)<<1));

			if (WapiComparePN(wapiCryptoTemp, &wapiInfo->wapiPN.rxUCast[qosIdx][0])==WAPI_RETURN_FAILED)
			{
				wapiAssert(0);
				memset(rtl_SMS4_rxSeq, 0, RX_QUEUE_NUM*sizeof(unsigned short));
				WAPI_UNLOCK(&priv->pshare->lock);
				return FAIL;
			}
		}
		memcpy(&wapiInfo->wapiPN.rxUCast[qosIdx][0], pRecvPN, WAPI_PN_LEN);
		rtl_SMS4_rxSeq[qosIdx]=pfrinfo->seq;
#else
		if (pfrinfo->seq>=wapiInfo->wapiPN.rxSeq[qosIdx])
		{
			if (WapiComparePN(pRecvPN, &wapiInfo->wapiPN.rxUCast[qosIdx][0])==WAPI_RETURN_FAILED)
			{
				WAPI_UNLOCK(&priv->pshare->lock);
				return FAIL;
			}
			memcpy(&wapiInfo->wapiPN.rxUCast[qosIdx][0], pRecvPN, WAPI_PN_LEN);
			wapiInfo->wapiPN.rxSeq[qosIdx]=pfrinfo->seq;
		}
		else
		{
			memcpy(wapiCryptoTemp, pRecvPN, WAPI_PN_LEN);
			WapiIncreasePN(wapiCryptoTemp, ((wapiInfo->wapiPN.rxSeq[qosIdx]-pfrinfo->seq+1)<<1));
			if (WapiComparePN(wapiCryptoTemp, &wapiInfo->wapiPN.rxUCast[qosIdx][0])==WAPI_RETURN_FAILED)
			{
				WAPI_UNLOCK(&priv->pshare->lock);
				return FAIL;
			}
		}
#endif

		pMicKey = wapiInfo->wapiUCastKey[KeyIdx].micKey;
		pDataKey = wapiInfo->wapiUCastKey[KeyIdx].dataKey;
	}

	memcpy(wapiDecrytBuf, pSecData, DataLen);
	memcpy(wapiDecrytHdrBuf, pHeader, DataOffset);

#if	defined(RTL_CFG_SMS4_ASM)
	para.ptr1 = pSecData;
	para.ptr2 = pSecData;
	para.ptr3 = &OutputLength;
	WapiSMS4Decryption(pDataKey, pRecvPN, DataLen, &para);
#else
	WapiSMS4Decryption(pDataKey, pRecvPN, wapiDecrytBuf, DataLen, pSecData-WAPI_ALIGNMENT_OFFSET, &OutputLength);
#endif
	wapiAssert(OutputLength == DataLen);

	DataLen -= SMS4_MIC_LEN;

	SecCalculateMicSMS4(KeyIdx, pMicKey, wapiDecrytHdrBuf, pSecData-WAPI_ALIGNMENT_OFFSET, DataLen, wapiCryptoTemp);

	if (memcmp(wapiCryptoTemp, pRecvMic, SMS4_MIC_LEN))
	{
		wapiAssert(0);
		WAPI_UNLOCK(&priv->pshare->lock);
		return FALSE;
	}

	WAPI_UNLOCK(&priv->pshare->lock);
	return TRUE;
}
#endif

/************************************************************
generate radom number
************************************************************/
void
GenerateRandomData(unsigned char * data, unsigned int len)
{
	unsigned int i, num;
	unsigned char *pRu8;
#ifdef __LINUX_2_6__
	srandom32(jiffies);
#endif

	for (i=0; i<len; i++) {
#ifdef __LINUX_2_6__
		num = random32();
#else
		get_random_bytes(&num, 4);
#endif
		pRu8 = (unsigned char*)&num;
		data[i] = pRu8[0]^pRu8[1]^pRu8[2]^pRu8[3];
	}
}

#endif
