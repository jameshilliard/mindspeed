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

#ifndef _RSA_PARAMS_H
#define _RSA_PARAMS_H

#include <inttypes.h>

/**
 * get_verify_data() - Returns the necessary verification data
 *
 * Calculates and returns the necessary data for run-time verification by
 * rsa_verify.c. For details on the output parameters, see rsa_verify.c.
 *
 * The caller must free *modulus_ptr and *r_squared_ptr when done with them.
 *
 * @filepath:	Path to an RSA private key file
 * @modulus_ptr:	Output pointer for modulus parameter
 * @r_squared_ptr:	Output pointer for rr parameter
 * @n0_inv:	Output for n0_inv parameter
 * @len:	Output for len parameter (in bits)
 * @return:	0, on success, -ve on error
*/
int rsa_get_verify_data(char *filepath, uint32_t **modulus_ptr,
		uint32_t **r_squared_ptr, uint32_t *n0_inv, int *len);

#endif

