/*
 * (C) Copyright 2014 Google Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _SECURE_BOOT_H
#define _SECURE_BOOT_H

#include <common.h>

#define SB_HEADER_LEN	16
#define SB_INFO_LEN	4080
#define SB_SIG_LEN	256
#define SB_FAKE_SIG	0x90091efb

typedef enum {
	UNKNOWN = -1,
	UNSECURE = 0,
	SECURE = 1
} secure_boot_mode_t;

/*
 * Determines whether secure boot mode is enabled, i.e. whether the OTP is set.
 */
secure_boot_mode_t get_secure_boot_mode(void);

/*
 * Extracts an unsigned 4-byte little endian int from a byte pointer.
 *
 * It is the responsibility of the caller to move the pointer on after
 * calling this method.
 *
 * Note that we use uint32_t with the assumption that this will always
 * have a size of at least 4 bytes.
 */
uint32_t _get_le_uint32(uint8_t *ptr);

#endif
