/*
 * Copyright (c) 2014, Google Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

/*
 * This file is a modified version of rsa-sign.c found in
 * U-Boot (http://www.denx.de/wiki/U-Boot)
 */

#include "rsa_params.h"

#include <openssl/err.h>
#include <openssl/ssl.h>

#if OPENSSL_VERSION_NUMBER >= 0x10000000L
#define HAVE_ERR_REMOVE_THREAD_STATE
#endif

static int rsa_err(const char *msg)
{
	unsigned long sslErr = ERR_get_error();

	fprintf(stderr, "%s", msg);
	fprintf(stderr, ": %s\n",
	ERR_error_string(sslErr, 0));

	return -1;
}

/**
 * rsa_get_pub_key() - read a public key from a .pub file
 *
 * @path	Path to .pub file
 * @rsap	Returns RSA object, or NULL on failure
 * @return 0 if ok, -ve on error (in which case *rsap will be set to NULL)
 */
static int rsa_get_pub_key(char *path, RSA **rsap)
{
	RSA *rsa = NULL;
	FILE *f;
	int ret;

	*rsap = NULL;
	f = fopen(path, "r");
	if (!f) {
		fprintf(stderr, "Couldn't open RSA pub: '%s': %s\n", path, strerror(errno));
		return -EACCES;
	}

	if (!PEM_read_RSA_PUBKEY(f, &rsa, NULL, NULL)) {
		rsa_err("Couldn't read pub file");
		ret = -EINVAL;
		goto err_rsa;
	}

	fclose(f);
	*rsap = rsa;

	return 0;

err_rsa:
	fclose(f);
	return ret;
}

static int rsa_init(void)
{
	int ret;

	ret = SSL_library_init();
	if (!ret) {
		fprintf(stderr, "Failure to init SSL library\n");
		return -1;
	}
	SSL_load_error_strings();

	OpenSSL_add_all_algorithms();
	OpenSSL_add_all_digests();
	OpenSSL_add_all_ciphers();

	return 0;
}

static void rsa_remove(void)
{
	CRYPTO_cleanup_all_ex_data();
	ERR_free_strings();
#ifdef HAVE_ERR_REMOVE_THREAD_STATE
	ERR_remove_thread_state(NULL);
#else
	ERR_remove_state(0);
#endif
	EVP_cleanup();
}


/*
 * rsa_get_params(): - Get the important parameters of an RSA public key
 */
int rsa_get_params(RSA *key, uint32_t *n0_invp, BIGNUM **modulusp,
		BIGNUM **r_squaredp)
{
	BIGNUM *big1, *big2, *big32, *big2_32;
	BIGNUM *n, *r, *r_squared, *tmp;
	BN_CTX *bn_ctx = BN_CTX_new();
	int ret = 0;

	/* Initialize BIGNUMs */
	big1 = BN_new();
	big2 = BN_new();
	big32 = BN_new();
	r = BN_new();
	r_squared = BN_new();
	tmp = BN_new();
	big2_32 = BN_new();
	n = BN_new();
	if (!big1 || !big2 || !big32 || !r || !r_squared || !tmp || !big2_32 ||
			!n) {
		fprintf(stderr, "Out of memory (bignum)\n");
		return -ENOMEM;
	}

	if (!BN_copy(n, key->n) || !BN_set_word(big1, 1L) ||
			!BN_set_word(big2, 2L) || !BN_set_word(big32, 32L))
		ret = -1;

	/* big2_32 = 2^32 */
	if (!BN_exp(big2_32, big2, big32, bn_ctx))
		ret = -1;

	/* Calculate n0_inv = -1 / n[0] mod 2^32 */
	if (!BN_mod_inverse(tmp, n, big2_32, bn_ctx) ||
			!BN_sub(tmp, big2_32, tmp))
		ret = -1;
	*n0_invp = BN_get_word(tmp);

	/* Calculate R = 2^(# of key bits) */
	if (!BN_set_word(tmp, BN_num_bits(n)) ||
			!BN_exp(r, big2, tmp, bn_ctx))
		ret = -1;

	/* Calculate r_squared = R^2 mod n */
	if (!BN_copy(r_squared, r) ||
			!BN_mul(tmp, r_squared, r, bn_ctx) ||
			!BN_mod(r_squared, tmp, n, bn_ctx))
		ret = -1;

	*modulusp = n;
	*r_squaredp = r_squared;

	BN_free(big1);
	BN_free(big2);
	BN_free(big32);
	BN_free(r);
	BN_free(tmp);
	BN_free(big2_32);
	if (ret) {
		fprintf(stderr, "Bignum operations failed\n");
		return -ENOMEM;
	}

	return ret;
}

/*
 * Convert a BIGNUM to a little endian array of uint32_ts.
 *
 * The caller is responsible for free-ing *arr after use.
 */
static int bignum_to_uint32_t(BIGNUM *num, int num_bits, uint32_t **arr)
{
	int nwords = num_bits / 32;
	int size;
	uint32_t *ptr;
	BIGNUM *tmp, *big2, *big32, *big2_32;
	BN_CTX *ctx;

	tmp = BN_new();
	big2 = BN_new();
	big32 = BN_new();
	big2_32 = BN_new();
	if (!tmp || !big2 || !big32 || !big2_32) {
		fprintf(stderr, "Out of memory (bignum)\n");
		return -ENOMEM;
	}
	ctx = BN_CTX_new();
	if (!tmp) {
		fprintf(stderr, "Out of memory (bignum context)\n");
		return -ENOMEM;
	}
	BN_set_word(big2, 2L);
	BN_set_word(big32, 32L);
	BN_exp(big2_32, big2, big32, ctx); /* B = 2^32 */

	size = nwords * sizeof(uint32_t);
	*arr = malloc(size);
	if (!(*arr)) {
		fprintf(stderr, "Out of memory (%d bytes)\n", size);
		return -ENOMEM;
	}

	/* Write out bignum as little endian array of integers */
	for (ptr = *arr; ptr < *arr + nwords; ptr++) {
		BN_mod(tmp, num, big2_32, ctx); /* n = N mod B */
		*ptr = BN_get_word(tmp);
		BN_rshift(num, num, 32); /* N = N/B */
	}

	BN_free(tmp);
	BN_free(big2);
	BN_free(big32);
	BN_free(big2_32);

	return 0;
}

int rsa_get_verify_data(char *filepath, uint32_t **modulus_ptr,
		uint32_t **r_squared_ptr, uint32_t *n0_inv, int *bits)
{
	BIGNUM *modulus, *r_squared;
	int ret;
	RSA *rsa;

	ret = rsa_init();
	if (ret)
		return ret;

	ret = rsa_get_pub_key(filepath, &rsa);
	if (ret)
		return ret;
	ret = rsa_get_params(rsa, n0_inv, &modulus, &r_squared);
	if (ret)
		return ret;
	*bits = BN_num_bits(modulus);

	bignum_to_uint32_t(modulus, *bits, modulus_ptr);
	bignum_to_uint32_t(r_squared, *bits, r_squared_ptr);

	BN_free(modulus);
	BN_free(r_squared);
	rsa_remove();

	if (ret)
		return -EIO;

	return 0;
}
