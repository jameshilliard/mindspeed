/*
 * Copyright (c) 2014, Google Inc.
 *
 * (C) Copyright 2008 Semihalf
 *
 * (C) Copyright 2000-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * This file is a (heavily) modified version of rsa.h found in
 * U-Boot (http://www.denx.de/wiki/U-Boot)
 */

#ifndef _RSA_VERIFY_H
#define _RSA_VERIFY_H

#include <common.h>

/**
 * rsa_verify() - Verifies a RSA PKCS1.5 signature against a hash.
 *
 * @sig:	The RSA signature
 * @sig_len:	The signature length
 * @hash:	The hash to compare against
 * @return:	0 if verified, -ve on error
 */
int rsa_verify(uint8_t *sig, uint32_t sig_len, uint8_t hash[20]);

#endif

