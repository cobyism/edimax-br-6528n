/*
 * ppp_mppe_crypto.h - cryptografic funtion prototypes for MPPE
 *
 *  This code is Public Domain. Please see comments below.
 *
 *  I have just put SHA1 and ARCFOUR declarations into one file
 *  in order to not pollute kernel namespace. 
 *
 *  Jan Dubiec <jdx@slackware.pl>, 2003-07-08
 */

#ifndef _PPP_MPPE_CRYPTO_
#define _PPP_MPPE_CRYPTO_

/* SHA1 definitions and prototypes */
typedef struct {
    unsigned long state[5];
    unsigned long count[2];
    unsigned char buffer[64];
} SHA1_CTX;

#define SHA1_SIGNATURE_SIZE 20

extern void SHA1_Init(SHA1_CTX *);
extern void SHA1_Update(SHA1_CTX *, const unsigned char *, unsigned int);
extern void SHA1_Final(unsigned char[SHA1_SIGNATURE_SIZE], SHA1_CTX *);

/* ARCFOUR (aka RC4) definitions and prototypes */
typedef struct {
    unsigned i;
    unsigned j;
    unsigned char S[256];
} arcfour_context;

extern void arcfour_setkey(arcfour_context *, const unsigned char *, unsigned);
extern void arcfour_encrypt(arcfour_context *, const unsigned char *, unsigned,
			    unsigned char *);
#define arcfour_decrypt arcfour_encrypt

#endif /* _PPP_MPPE_CRYPTO_ */
