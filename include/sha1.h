/*
 *  Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *  based on:
 *  FIPS-180-1 compliant SHA-1 implementation
 *
 *  Copyright (C) 2003-2006  Christophe Devine
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License, version 2.1 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA  02110-1301  USA
 */

#ifndef _SHA1_H
#define _SHA1_H

#include <common.h>

#define SHA1_SUM_LEN	20

typedef struct
{
	uint32_t total[2];	/*!< number of bytes processed */
	uint32_t state[5];	/*!< intermediate digest state */
	uint8_t buffer[64];	/*!< data block being processed */
}
sha1_context;

void sha1_starts(sha1_context *ctx);

void sha1_update(sha1_context *ctx, uint8_t *input, uint32_t len);

void sha1_finish(sha1_context *ctx, uint8_t output[20]);

#endif

