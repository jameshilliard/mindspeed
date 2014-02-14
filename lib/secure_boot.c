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

#include <secure_boot.h>

#include <c2k_otp.h>

secure_boot_mode_t get_secure_boot_mode(void)
{
	uint8_t config_byte;

	if (otp_read(8, &config_byte, 1) != 0) {
		/* The read failed, so we don't know if it's secure! */
		return UNKNOWN;
	}

	if ((config_byte & 0x2) == 0x2) {
		return SECURE;
	}

	return UNSECURE;
}

uint32_t _get_le_uint32(uint8_t *ptr) {
	uint32_t value = 0;

	value |= *ptr++;
	value |= *ptr++ << 8;
	value |= *ptr++ << 16;
	value |= *ptr++ << 24;

	return value;
}
