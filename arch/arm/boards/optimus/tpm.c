/*
 * (C) Copyright 2015 Google Inc.
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

#include <common.h>
#include <tpm_lite/tlcl.h>
#include <secure_boot.h>

#define RETURN_ON_FAILURE(tpm_cmd) do {			\
	uint32_t result_;				\
	if ((result_ = (tpm_cmd)) != TPM_SUCCESS) {			\
		printk(KERN_DEBUG "TPM: %08x returned by " #tpm_cmd     \
			"\n", (int)result_);                            \
		return result_;						\
	}								\
	} while (0)

#define PCR_DIGEST_LENGTH	20

#define SHA1_NON_SECURE_BOOT	0
#define SHA1_SECURE_BOOT	1

static const uint8_t bootmode_digests[2][PCR_DIGEST_LENGTH] = {
	/* non-secure boot [0, 0, 2] */
	{0x1e, 0xf6, 0x24, 0x48, 0x2d, 0x62, 0x0e, 0x43, 0xe6, 0xd3,
	 0x4d, 0xa1, 0xaf, 0xe4, 0x62, 0x67, 0xfc, 0x69, 0x5d, 0x9b},

	/* secure boot [0, 0, 1] */
	{0x25, 0x47, 0xcc, 0x73, 0x6e, 0x95, 0x1f, 0xa4, 0x91, 0x98,
	 0x53, 0xc4, 0x3a, 0xe8, 0x90, 0x86, 0x1a, 0x3b, 0x32, 0x64}
};

static const uint8_t sha256_gfsc100[PCR_DIGEST_LENGTH] = {
	0x4f, 0x8c, 0x4d, 0xaf, 0x4d, 0xf3, 0x2e, 0xaf, 0xb5, 0x57,
	0xff, 0x76, 0x58, 0x86, 0x02, 0x94, 0x42, 0xdc, 0xce, 0x06
};

static const uint8_t PCR_uninitialized[PCR_DIGEST_LENGTH] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

uint32_t tpm_init(void) {
	uint32_t result;
	uint8_t disable;
	uint8_t deactivated;
	uint8_t pcr_value[PCR_DIGEST_LENGTH];
	TPM_STCLEAR_FLAGS sflags;

	RETURN_ON_FAILURE(tlcl_lib_init());

	result = tlcl_startup();
	if ((result != TPM_SUCCESS) &&
		(result != TPM_E_INVALID_POSTINIT)) {
		/* Invalid postinit indicates that TPM_Startup has already been
		   executed. This situation occurs on reboots on the GFSC100
		   platform because a reboot does not powercycle the TPM */
		printk(KERN_WARNING "TPM_Startup failed: 0x%08x\n", result);
		return result;
	}

	/* Continue or start the self-test of all TPM functions. Untested
	   functions will return TPM_NEEDS_SELFTEST otherwise */
	RETURN_ON_FAILURE(tlcl_continue_self_test());

	RETURN_ON_FAILURE(tlcl_get_flags(&disable, &deactivated, NULL));
	if (disable || deactivated) {
		printk(KERN_DEBUG "TPM: disabled (%d) or deactivated (%d). "
			"Fixing...\n", disable, deactivated);
		RETURN_ON_FAILURE(tlcl_assert_physical_presence());
		RETURN_ON_FAILURE(tlcl_set_enable());
		RETURN_ON_FAILURE(tlcl_set_deactivated(0));
	}

	RETURN_ON_FAILURE(tlcl_get_stclear_flags(&sflags));
	if (sflags.deactivated) {
		printk(KERN_DEBUG "TPM: Must reboot to re-enable\n");
		return TPM_E_MUST_REBOOT;
	}

	RETURN_ON_FAILURE(tlcl_read_pcr(0, pcr_value));
	if (!memcmp(pcr_value, PCR_uninitialized, PCR_DIGEST_LENGTH)) {
		/* PCR0 not initialized, extend it with the boot mode */
		const uint8_t *digest;

		if (get_secure_boot_mode() == SECURE) {
			digest = bootmode_digests[SHA1_SECURE_BOOT];
		} else {
			digest = bootmode_digests[SHA1_NON_SECURE_BOOT];
		}

		RETURN_ON_FAILURE(tlcl_extend(0, digest, NULL));
	}

	RETURN_ON_FAILURE(tlcl_read_pcr(1, pcr_value));
	if (!memcmp(pcr_value, PCR_uninitialized, PCR_DIGEST_LENGTH)) {
		/* PCR1 not initialized, extend it with the platform name */
		RETURN_ON_FAILURE(tlcl_extend(1, sha256_gfsc100, NULL));
	}

	printk(KERN_DEBUG "TPM: Initialization successful\n");

	return TPM_SUCCESS;
}
EXPORT_SYMBOL(tpm_init)