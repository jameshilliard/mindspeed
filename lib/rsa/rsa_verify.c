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
 * This file is a modified version of rsa-verify.c found in
 * U-Boot (http://www.denx.de/wiki/U-Boot)
 */

#include <rsa_verify.h>

#include <common.h>
#include <errno.h>
#include <malloc.h>
#include <sha1.h>
#include <asm/byteorder.h>
#include <asm/unaligned.h>
#include <rsa_public_key.h>

#define RSA2048_BYTES	(2048 / 8)

/* This is the maximum key size we support, in bits */
#define RSA_MAX_KEY_BITS	2048

/* This is the maximum signature length that we support, in bits */
#define RSA_MAX_SIG_BITS	2048

static const uint8_t padding_sha1_rsa2048[RSA2048_BYTES - SHA1_SUM_LEN] = {
	0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0x00, 0x30, 0x21, 0x30,
	0x09, 0x06, 0x05, 0x2b, 0x0e, 0x03, 0x02, 0x1a,
	0x05, 0x00, 0x04, 0x14
};

/**
 * subtract_modulus() - subtract modulus from the given value
 *
 * @key:	Key containing modulus to subtract
 * @num:	Number to subtract modulus from, as little endian word array
 */
static void subtract_modulus(const struct rsa_public_key *key, uint32_t num[])
{
	int64_t acc = 0;
	uint i;

	for (i = 0; i < KEY_LEN_WORDS; i++) {
		acc += (uint64_t)num[i] - key->modulus[i];
		num[i] = (uint32_t)acc;
		acc >>= 32;
	}
}

/**
 * greater_equal_modulus() - check if a value is >= modulus
 *
 * @key:	Key containing modulus to check
 * @num:	Number to check against modulus, as little endian word array
 * @return 0 if num < modulus, 1 if num >= modulus
 */
static int greater_equal_modulus(const struct rsa_public_key *key,
				 uint32_t num[])
{
	uint32_t i;

	for (i = KEY_LEN_WORDS - 1; i >= 0; i--) {
		if (num[i] < key->modulus[i])
			return 0;
		if (num[i] > key->modulus[i])
			return 1;
	}

	return 1;	/* equal */
}

/**
 * montgomery_mul_add_step() - Perform montgomery multiply-add step
 *
 * Operation: montgomery result[] += a * b[] / n0inv % modulus
 *
 * @key:	RSA key
 * @result:	Place to put result, as little endian word array
 * @a:		Multiplier
 * @b:		Multiplicand, as little endian word array
 */
static void montgomery_mul_add_step(const struct rsa_public_key *key,
		uint32_t result[], const uint32_t a, const uint32_t b[])
{
	uint64_t acc_a, acc_b;
	uint32_t d0;
	uint i;

	acc_a = (uint64_t)a * b[0] + result[0];
	d0 = (uint32_t)acc_a * key->n0inv;
	acc_b = (uint64_t)d0 * key->modulus[0] + (uint32_t)acc_a;
	for (i = 1; i < KEY_LEN_WORDS; i++) {
		acc_a = (acc_a >> 32) + (uint64_t)a * b[i] + result[i];
		acc_b = (acc_b >> 32) + (uint64_t)d0 * key->modulus[i] +
				(uint32_t)acc_a;
		result[i - 1] = (uint32_t)acc_b;
	}

	acc_a = (acc_a >> 32) + (acc_b >> 32);

	result[i - 1] = (uint32_t)acc_a;

	if (acc_a >> 32)
		subtract_modulus(key, result);
}

/**
 * montgomery_mul() - Perform montgomery mutitply
 *
 * Operation: montgomery result[] = a[] * b[] / n0inv % modulus
 *
 * @key:	RSA key
 * @result:	Place to put result, as little endian word array
 * @a:		Multiplier, as little endian word array
 * @b:		Multiplicand, as little endian word array
 */
static void montgomery_mul(const struct rsa_public_key *key,
		uint32_t result[], uint32_t a[], const uint32_t b[])
{
	uint i;

	for (i = 0; i < KEY_LEN_WORDS; ++i)
		result[i] = 0;
	for (i = 0; i < KEY_LEN_WORDS; ++i)
		montgomery_mul_add_step(key, result, a[i], b);
}

/**
 * pow_mod() - in-place public exponentiation
 *
 * @key:	RSA key
 * @inout:	Big-endian word array containing value and result
 */
static int pow_mod(const struct rsa_public_key *key, uint32_t *inout)
{
	uint32_t *result, *ptr;
	uint i;

	uint32_t val[KEY_LEN_WORDS], acc[KEY_LEN_WORDS], tmp[KEY_LEN_WORDS];
	result = tmp;	/* Re-use location. */

	/* Convert from big endian byte array to little endian word array. */
	for (i = 0, ptr = inout + KEY_LEN_WORDS - 1; i < KEY_LEN_WORDS; i++, ptr--)
		val[i] = get_unaligned_be32(ptr);

	montgomery_mul(key, acc, val, key->rr);	/* axx = a * RR / R mod M */
	for (i = 0; i < 16; i += 2) {
		montgomery_mul(key, tmp, acc, acc);	/* tmp = acc^2 / R mod M */
		montgomery_mul(key, acc, tmp, tmp);	/* acc = tmp^2 / R mod M */
	}
	montgomery_mul(key, result, acc, val);	/* result = XX * a / R mod M */

	/* Make sure result < mod; result is at most 1x mod too large. */
	if (greater_equal_modulus(key, result))
		subtract_modulus(key, result);

	/* Convert to bigendian byte array */
	for (i = KEY_LEN_WORDS - 1, ptr = inout; (int)i >= 0; i--, ptr++)
		put_unaligned_be32(result[i], ptr);

	return 0;
}

int rsa_verify(const struct rsa_public_key *key, uint8_t *sig, uint32_t sig_len,
	uint8_t *hash)
{
	const uint8_t *padding;
	int ret, pad_len;

	uint32_t *buf;
	buf = malloc(sig_len);

	if (!buf) {
		printf("!buf\n");
		return -EIO;
	}

	if (!key || !sig || !hash) {
		printf("!key || !sig || !hash\n");
		ret = -EIO;
		goto end;
	}

	if (sig_len != (KEY_LEN_WORDS * sizeof(uint32_t))) {
		printf("Signature is of incorrect length %d\n", sig_len);
		ret = -EINVAL;
		goto end;
	}

	/* Sanity check for stack size */
	if (sig_len > RSA_MAX_SIG_BITS / 8) {
		printf("Signature length %u exceeds maximum %d\n", sig_len,
				RSA_MAX_SIG_BITS / 8);
		ret = -EINVAL;
		goto end;
	}

	memcpy(buf, sig, sig_len);

	/* Sanity check for stack size - KEY_LEN_WORDS is in 32-bit words */
	if (KEY_LEN_WORDS > RSA_MAX_KEY_BITS / 32) {
		printf("RSA key words %u exceeds maximum %d\n", KEY_LEN_WORDS,
				RSA_MAX_KEY_BITS / 32);
		ret = -EINVAL;
		goto end;
	}

	ret = pow_mod(key, buf);

	/* Determine padding to use depending on the signature type. */
	padding = padding_sha1_rsa2048;
	pad_len = RSA2048_BYTES - SHA1_SUM_LEN;

	/* Check pkcs1.5 padding bytes. */
	if (memcmp(buf, padding, pad_len)) {
		ret = -EINVAL;
		goto end;
	}

	/* Check hash. */
	if (memcmp((uint8_t *)buf + pad_len, hash, sig_len - pad_len)) {
		ret = -EACCES;
		goto end;
	}

end:
	free(buf);
	return ret;
}
