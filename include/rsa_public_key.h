#ifndef _RSA_PUBLIC_KEY_H
#define _RSA_PUBLIC_KEY_H

#include <common.h>

#define KEY_LEN_WORDS 64

/**
 * struct rsa_public_key - holder for a public key
 *
 * An RSA public key consists of a modulus (typically called N), the inverse
 * and R^2, where R is 2^(# key bits).
 */
struct rsa_public_key {
	uint32_t n0inv; /* -1 / modulus[0] mod 2^32 */
	uint32_t modulus[64];      /* modulus as little endian array */
	uint32_t rr[64];   /* R^2 as little endian array */
};

int rsa_get_public_key(int board_id, const struct rsa_public_key **key);

#endif
