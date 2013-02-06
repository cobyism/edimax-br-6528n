#include "gunzip.h"


static const ulong crc_32_tab[256] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
	0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
	0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
	0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
	0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
	0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
	0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
	0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
	0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
	0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
	0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
	0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
	0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
	0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
	0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
	0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
	0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
	0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
	0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
	0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

static const huft_code mask_bits[] = {
	0x0000,
	0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
	0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};

/* Tables for deflate from PKZIP's appnote.txt. */
static const unsigned border[] = {    /* Order of the bit length code lengths */
	16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

static const ushort cplext[] = {         /* Extra bits for literal codes 257..285 */
	0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
	3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 99, 99
}; /* 99==invalid */

static const ushort cplens[] = {         /* Copy lengths for literal codes 257..285 */
	3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
	35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0
}; /* note: see note #13 above about the 258 in this list. */

static const ushort cpdext[] = {         /* Extra bits for distance codes */
	0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
	7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13
};

static const ushort cpdist[] = {         /* Copy offsets for distance codes 0..29 */
	1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
	257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
	8193, 12289, 16385, 24577
};

static huft_code bit_reverse(huft_code code, huft_bits bits)
{
	huft_code ret = 0;
	int i;

	for (i = 0; i < bits; i++) {
		if (code & (1 << i))
			ret |= (1 << (bits - i - 1));
	}
	
	return ret;
}

static huft_code huft_get_code(
	huft_bits *b,           /* code lengths in bits (all assumed <= B_MAX) */
	int n,                  /* number of codes (assumed <= N_MAX) */
	int k                   /* index */
	)
{
	huft_code code = 0;
	huft_bits bits = b[k];
	int i;

	if (bits == 0)
		return 0;

	for (i = 0; i < n; i++) {
		if (b[i] < bits && b[i] > 0)
			code += 1 << (b[k] - b[i]);
		else if ((i < k) && (b[i] == bits))
			code += 1;
	}

	return bit_reverse(code, bits);
}

static int huft_build_table(
	huft_bits *b,           /* code lengths in bits (all assumed <= B_MAX) */
	int n,                  /* number of codes (assumed <= N_MAX) */
	huft_code *t         /* result: starting table */
	)
{
	int i;

	for (i = 0; i < n; i++) {
		t[i] = huft_get_code(b, n, i);
	}

	return 0;
}

static int huft_get_value(
	huft_bits *b,           /* code lengths in bits (all assumed <= B_MAX) */
	huft_code *t,           /* huft table */
	int n,                  /* number of codes (assumed <= N_MAX) */
	huft_code code,
	huft_bits *bits
	)
{
	int i, j;

	for (i = 0; i < n; i++) {
		if (b[i] == 0)
			continue;

		if ((code & ~(0xffff << b[i])) == t[i])
			break;
	}

	if (i == n)
		return -1;

	*bits = b[i];
	return i;
}

static huft_code peek_bits(gunzip_t *guz, huft_bits n)
{
	huft_code r;

	while (guz->bufbits < (n))
	{
		guz->bitbuf |= ((ulong)get_uchar(guz)) << guz->bufbits;
		guz->bufbits += 8;
	}
	r = guz->bitbuf & mask_bits[n];

	return r;
}

static void skip_bits(gunzip_t *guz, huft_bits n)
{
	guz->bitbuf >>= (n);
	guz->bufbits -= (n);
}

static huft_code get_bits(gunzip_t *guz, huft_bits n)
{
	huft_code r;

	while (guz->bufbits < (n))
	{
		guz->bitbuf |= ((ulong)get_uchar(guz)) << guz->bufbits;
		guz->bufbits += 8;
	}
	r = guz->bitbuf & mask_bits[n];

	guz->bitbuf >>= (n);
	guz->bufbits -= (n);

	return r;
}

static int inflate_codes(gunzip_t *guz,
	huft_bits *ll,
	huft_code *tl,     /* literal/length decoder tables */
	huft_bits *ld,
	huft_code *td,     /* distance decoder tables */
	int nl,            /* number of bits decoded by tl[] */
	int nd             /* number of bits decoded by td[] */
	)
{
	register int i, j;
	register huft_code code;
	register huft_bits bits;
	register int value, n;
	register uchar *outbuf;
	register ulong outptr;
	register huft_code codex;
	huft_bits bits_tmp;
	huft_code ftl[256], ftlex[512], ftd[256];
	int ftnex = 0;
	int len, dist;

	outbuf = guz->outbuf;
	outptr = guz->outptr;

	for (i = 0; i < 256; i++) {
		ftl[i] = 0xffff;
		ftd[i] = 0xffff;
	}

	for (i = 0; i < 512; i++)
		ftlex[i] = 0xffff;

	for (i = 0; i < nl; i++) {

		bits = ll[i];
		if (bits > 0 && bits <= 8) {
			code = (ushort)i | (bits << 9);
			for (j = 0; j < (1 << (8 - bits)); j++) {
				ftl[tl[i] | (j << bits)] = code;
			}

		} else if (bits > 8 && bits < 12 && ftnex < 64) {
			code = tl[i] & 0xff;
			if (ftl[code] == 0xffff) {
				n = (ftnex++) << 3;
				ftl[code] = 0x8000 | n;
			} else
				n = ftl[code] & 0x7fff;

			code = (ushort)i | (bits << 9);
			n += tl[i] >> 8;
			for (j = 0; j < (1 << (11 - bits)); j++) {
				ftlex[n | (j << (bits - 8))] = code;
			}
		}
	}

	for (i = 0; i < nd; i++) {
		bits = ld[i];
		code = (ushort)i + (bits << 9);
		if (bits > 0 && bits <= 8) {
			for (j = 0; j < (1 << (8 - bits)); j++) {
				ftd[td[i] | (j << bits)] = code;
			}
		}
	}

	while (1) {

		code = peek_bits(guz, 8);

		if (!(ftl[code] & 0x8000)) {
			value = ftl[code] & 0x01ff;
			bits = ftl[code] >> 9;

		} else if (ftl[code] != 0xffff) {
			n = ftl[code] & 0x7fff;
			code = peek_bits(guz, 11);
			codex = code >> 8;
			if (ftlex[n + codex] != 0xffff) {
				value = ftlex[n + codex] & 0x01ff;
				bits = ftlex[n + codex] >> 9;
			} else {
				code = peek_bits(guz, 16);
				value = huft_get_value(ll, tl, nl, code, &bits_tmp);
				bits = bits_tmp;
				if (value == -1)
					return 1;         /* error in compressed data */
			}

		} else {
			code = peek_bits(guz, 16);
			value = huft_get_value(ll, tl, nl, code, &bits_tmp);
			bits = bits_tmp;
			if (value == -1)
				return 1;         /* error in compressed data */
		}
		skip_bits(guz, bits);

		if (value < 256) {
			outbuf[outptr++] = (uchar)value;
		} else if (value > 256) {
			code = get_bits(guz, cplext[value - 257]);
			len = cplens[value - 257] + code;

			code = peek_bits(guz, 8);
			if (ftd[code] != 0xffff) {
				value = ftd[code] & 0x01ff;
				bits = ftd[code] >> 9;
			} else {
				code = peek_bits(guz, 16);
				value = huft_get_value(ld, td, nd, code, &bits_tmp);
				bits = bits_tmp;
				if (value == -1)
					return 1;         /* error in compressed data */
			}
			skip_bits(guz, bits);
			dist = cpdist[value] + get_bits(guz, cpdext[value]);

			for (i = 0; i < len; i++)
				outbuf[outptr++] = outbuf[outptr - dist];
		} else {
			break;
		}
	}

	guz->outbuf = outbuf;
	guz->outptr = outptr;

	return 0;
}

static int inflate_stored(gunzip_t *guz)
{
	ulong n;           /* number of bytes in block */

	/* go to byte boundary */
	n = guz->bufbits & 7;
	get_bits(guz, n);

	/* get the length and its complement */
	n = get_bits(guz, 16);
	if (n != (~get_bits(guz, 16) & 0xffff))
		return 1;                   /* error in compressed data */

	/* read and output the compressed data */
	while (n--)
		output_char(guz, (uchar)get_bits(guz, 8));

	return 0;
}

static int inflate_fixed(gunzip_t *guz)
{
	huft_bits ll[288], ld[32];
	huft_code tl[288], td[32];
	huft_bits bits;
	huft_code code;
	int value;
	int i, len, dist;

	/* set up literal table */
	for (i = 0; i < 144; i++)
		ll[i] = 8;
	for (; i < 256; i++)
		ll[i] = 9;
	for (; i < 280; i++)
		ll[i] = 7;
	for (; i < 288; i++)          /* make a complete, but wrong code set */
		ll[i] = 8;
	huft_build_table(ll, 288, tl);

	for (i = 0; i < 30; i++)
		ld[i] = 5;
	huft_build_table(ld, 30, td);

	return inflate_codes(guz, ll, tl, ld, td, 288, 30);

}

static int build_bits_table(gunzip_t *guz,
	huft_bits *lb,
	huft_code *tb,
	int nb,
	huft_bits *ll,
	int nl
	)
{
	huft_bits bits;
	huft_code code;
	int value, i, n;

	i = 0;
	while (i < nl) {
		code = peek_bits(guz, 16);
		value = huft_get_value(lb, tb, 19, code, &bits);
		if (value == -1)
			return 1;         /* error in compressed data */
		skip_bits(guz, bits);
		if (value < 16) {
			ll[i++] = value;
		} else if (value == 16) {
			code = get_bits(guz, 2);
			for (n = 0; n < code + 3; n++) {
				ll[i++] = ll[i - 1];
			}
		} else if (value == 17) {
			code = get_bits(guz, 3);
			for (n = 0; n < code + 3; n++)
				ll[i++] = 0;
		} else if (value == 18)	{
			code = get_bits(guz, 7);
			for (n = 0; n < code + 11; n++)
				ll[i++] = 0;
		}
	}

	return 0;
}

static int inflate_dynamic(gunzip_t *guz)
{
	huft_bits lb[19], ll[288], ld[32];
	huft_code tb[19], tl[288], td[32];
	huft_bits bits;
	huft_code code;
	int nb;          /* number of bit length codes */
	int nl;          /* number of literal/length codes */
	int nd;          /* number of distance codes */
	int value, i, n;

	/* read in table lengths */
	nl = 257 + get_bits(guz, 5);      /* number of literal/length codes */
	nd = 1 + get_bits(guz, 5);        /* number of distance codes */
	nb = 4 + get_bits(guz, 4);        /* number of bit length codes */
	if (nl > 286 || nd > 30)
		return 1;                 /* bad lengths */

	/* read in bit-length-code lengths */
	for (i = 0; i < nb; i++)
		lb[border[i]] = get_bits(guz, 3);
	for (; i < 19; i++)
		lb[border[i]] = 0;
	huft_build_table(lb, 19, tb);

	build_bits_table(guz, lb, tb, 19, ll, nl);
	huft_build_table(ll, nl, tl);

	build_bits_table(guz, lb, tb, 19, ld, nd);
	huft_build_table(ld, nd, td);

	return inflate_codes(guz, ll, tl, ld, td, nl, nd);
}

static int inflate_block(gunzip_t *guz, ulong *e)
{
	ulong t;           /* block type */

	/* read in last block bit */
	*e = get_bits(guz, 1);

	/* read in block type */
	t = get_bits(guz, 2);

	/* inflate that block type */
	if (t == 0)
		return inflate_stored(guz);
	if (t == 1)
		return inflate_fixed(guz);
	if (t == 2)
		return inflate_dynamic(guz);

	/* bad block type */
	return 2;
}

static int inflate(gunzip_t *guz)
{
	ulong e;                /* last block flag */
	int r;                /* result code */
	unsigned h;           /* maximum struct huft's malloc'ed */
	void *ptr;

	/* decompress until the last block */
	do {
		if ((r = inflate_block(guz, &e)) != 0) {
			return r;
		}
	} while (!e);

	/* Undo too much lookahead. The next read will be byte aligned so we
	 * can discard unused bits in the last meaningful byte.
	 */
	while (guz->bufbits >= 8) {
		guz->bufbits -= 8;
		guz->inptr--;
	}

	/* return success */
	return 0;
}

static void init_gunzip_struct(gunzip_t *guz,
	uchar *inbuf, ulong insize, uchar *outbuf, ulong outsize)
{
	guz->inbuf = inbuf;
	guz->insize = insize;
	guz->inptr = 0;

	guz->outbuf = outbuf;
	guz->outsize = outsize;
	guz->outptr = 0;

	guz->bitbuf = 0;
	guz->bufbits = 0;
}

ulong get_crc(uchar *buf, int len)
{
	ulong crc;
	uchar ch;
	int i;

	crc = 0xffffffffL;
	for (i = 0; i < len; i++) {
		ch = buf[i];
		crc = crc_32_tab[((int)crc ^ ch) & 0xff] ^ (crc >> 8);
	}
	crc = (crc ^ 0xffffffffL);

	return crc;
}

int gunzip(uchar *inbuf, ulong *insize, uchar *outbuf, ulong *outsize)
{
	gunzip_t guz_struct;
	gunzip_t *guz;
	uchar magic[2];           /* magic header */
	uchar method;
	uchar flags;
	ulong orig_crc = 0;       /* original crc */
	ulong orig_len = 0;       /* original uncompressed length */
	int res;

	guz = &guz_struct;
	init_gunzip_struct(guz, inbuf, *insize, outbuf, *outsize);

	magic[0] = get_uchar(guz);
	magic[1] = get_uchar(guz);
	method = get_uchar(guz);

	if (magic[0] != 0x1f || magic[1] != 0x8b)
		return ERR_BADMAGIC;

	/* We only support method #8, DEFLATED */
	if (method != 8)
		return ERR_BADMETHOD;

	flags  = get_uchar(guz);
	if (flags & ENCRYPTED)
		return ERR_ENCRYPTED;

	if (flags & CONTINUATION)
		return ERR_MULTIPART;

	if (flags & RESERVED)
		return ERR_INVALIDFLAGS;

	get_ulong(guz);        /* Get timestamp */

	get_uchar(guz);        /* Ignore extra flags for the moment */
	get_uchar(guz);        /* Ignore OS type for the moment */

	if (flags & EXTRA_FIELD) {
		ushort len;

		len = get_ushort(guz);
		while (len--)
			get_uchar(guz);
	}

	/* Get original file name if it was truncated */
	if (flags & ORIG_NAME) {
		/* Discard the old name */
		while (get_uchar(guz) != 0);
	}

	/* Discard file comment if any */
	if (flags & COMMENT) {
		while (get_uchar(guz) != 0);
	}

	/* Decompress */
	if ((res = inflate(guz)) != 0) {
		switch (res) {
		case 1:
			/* invalid compressed format (err=1) */
			res = ERR_BADFORMAT1;
			break;
		case 2:
			/* invalid compressed format (err=2) */
			res = ERR_BADFORMAT2;
			break;
		case 3:
			/* out of memory */
			res = ERR_MEM;
			break;
		default:
			/* invalid compressed format (other) */
			res = ERR_BADFORMAT;
		}
		return res;
	}

	/* Get the crc and original length */
	/* crc32  (see algorithm.doc)
	 * uncompressed input size modulo 2^32
	 */
	orig_crc = get_ulong(guz);
	orig_len = get_ulong(guz);

	/* Validate decompression */
//	if (orig_crc != get_crc(guz->outbuf, guz->outptr))
//		return ERR_CRC;

	if (orig_len != guz->outptr)
		return ERR_LENGTH;

	*insize = guz->inptr;
	*outsize = guz->outptr;

	return 0;
}
