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

struct pcr_value_pair {
	uint8_t digest[PCR_DIGEST_LENGTH];
	uint8_t extended[PCR_DIGEST_LENGTH];
};

struct bootmode_pcr_values {
	struct pcr_value_pair normal_boot;
	struct pcr_value_pair recovery_mode;
};

static struct bootmode_pcr_values pcr_values_secure_boot = {
#ifdef CONFIG_DEVELOPER_BAREBOX
	/* secure boot, normal boot, dev firmware [0, 0, 2] */
	.normal_boot = {
		.digest = {0x1e, 0xf6, 0x24, 0x48, 0x2d, 0x62, 0x0e, 0x43, 0xe6, 0xd3,
			   0x4d, 0xa1, 0xaf, 0xe4, 0x62, 0x67, 0xfc, 0x69, 0x5d, 0x9b},
		.extended = {0xee, 0x56, 0xc3, 0x2d, 0x5f, 0x7b, 0xfa, 0x56, 0xba, 0x92,
			     0x6f, 0x66, 0x27, 0x0e, 0xc2, 0xe9, 0xd0, 0x1c, 0x64, 0xb3},
	},

	/* secure boot, recovery mode, dev firmware [0, 1, 2] */
	.recovery_mode = {
		.digest = {0x0c, 0x7a, 0x62, 0x3f, 0xd2, 0xbb, 0xc0, 0x5b, 0x06, 0x42,
			   0x3b, 0xe3, 0x59, 0xe4, 0x02, 0x1d, 0x36, 0xe7, 0x21, 0xad},
		.extended = {0x08, 0x20, 0xc4, 0xb6, 0x7b, 0x33, 0xb2, 0x81, 0x25, 0x6b,
			     0x03, 0x53, 0xdc, 0xc9, 0xfa, 0x43, 0x66, 0xe4, 0x27, 0xaf},
	},
#else
	/* secure boot, normal boot, prod firmware [0, 0, 1] */
	.normal_boot = {
		.digest = {0x25, 0x47, 0xcc, 0x73, 0x6e, 0x95, 0x1f, 0xa4, 0x91, 0x98,
			   0x53, 0xc4, 0x3a, 0xe8, 0x90, 0x86, 0x1a, 0x3b, 0x32, 0x64},
		.extended = {0x06, 0x4a, 0xec, 0x9b, 0xbd, 0x94, 0xde, 0xa1, 0x23, 0x1a,
			     0xe7, 0x57, 0x67, 0x64, 0x7f, 0x09, 0x8c, 0x39, 0x8e, 0x79},
	},
	/* secure boot, recovery mode, prod firmware [0, 1, 1] */
	.recovery_mode = {
		.digest = {0xee, 0xe4, 0x47, 0xed, 0xc7, 0x9f, 0xea, 0x1c, 0xa7, 0xc7,
			   0xd3, 0x4e, 0x46, 0x32, 0x61, 0xcd, 0xa4, 0xba, 0x33, 0x9e},
		.extended = {0x17, 0x18, 0xb0, 0x87, 0xa1, 0x6a, 0x4e, 0x80, 0xe7, 0x50,
			     0x74, 0xc8, 0x11, 0x09, 0xde, 0x23, 0x1a, 0x04, 0xc0, 0x0b},
	},
#endif
};

static struct bootmode_pcr_values pcr_values_nonsecure_boot = {
#ifdef CONFIG_DEVELOPER_BAREBOX
	/* non-secure boot, normal boot, dev firmware [1, 0, 2] */
	.normal_boot = {
		.digest = {0xfa, 0x01, 0x0d, 0x26, 0x64, 0xcc, 0x5b, 0x3b, 0x82, 0xee,
			   0x48, 0x8f, 0xe2, 0xb9, 0xf5, 0x0f, 0x49, 0x32, 0xeb, 0x8f},
		.extended = {0x29, 0x56, 0x57, 0x4c, 0x9d, 0x6a, 0x95, 0xd3, 0x15, 0x8a,
			     0x81, 0x45, 0x71, 0x6a, 0x25, 0x7c, 0x3f, 0x6d, 0x62, 0xc1},
	},
	/* non-secure boot, recovery mode, dev firmware [1, 1, 2] */
	.recovery_mode = {
		.digest = {0x12, 0xa3, 0x40, 0xd7, 0x89, 0x7f, 0xe7, 0x13, 0xfc, 0x8f,
			   0x02, 0xac, 0x53, 0x65, 0xb8, 0x6e, 0xbf, 0x35, 0x31, 0x78},
		.extended = {0x36, 0x56, 0x6d, 0xe7, 0x9c, 0x23, 0x94, 0x2f, 0x1b, 0x79,
			     0xcf, 0xd7, 0xde, 0x56, 0xe6, 0x9b, 0xe4, 0xfe, 0xcd, 0x72},
	},
#else
	/* non-secure boot, normal boot, dev firmware [1, 0, 2] */
	.normal_boot = {
		.digest = {0xfa, 0x01, 0x0d, 0x26, 0x64, 0xcc, 0x5b, 0x3b, 0x82, 0xee,
			   0x48, 0x8f, 0xe2, 0xb9, 0xf5, 0x0f, 0x49, 0x32, 0xeb, 0x8f},
		.extended = {0x29, 0x56, 0x57, 0x4c, 0x9d, 0x6a, 0x95, 0xd3, 0x15, 0x8a,
			     0x81, 0x45, 0x71, 0x6a, 0x25, 0x7c, 0x3f, 0x6d, 0x62, 0xc1},
	},
	/* non-secure boot, recovery mode, dev firmware [1, 1, 2] */
	.recovery_mode = {
		.digest = {0x12, 0xa3, 0x40, 0xd7, 0x89, 0x7f, 0xe7, 0x13, 0xfc, 0x8f,
			   0x02, 0xac, 0x53, 0x65, 0xb8, 0x6e, 0xbf, 0x35, 0x31, 0x78},
		.extended = {0x36, 0x56, 0x6d, 0xe7, 0x9c, 0x23, 0x94, 0x2f, 0x1b, 0x79,
			     0xcf, 0xd7, 0xde, 0x56, 0xe6, 0x9b, 0xe4, 0xfe, 0xcd, 0x72},
	},
#endif
};

extern int is_recovery_mode(void);

static const uint8_t sha256_gfsc100[PCR_DIGEST_LENGTH] = {
	0x4f, 0x8c, 0x4d, 0xaf, 0x4d, 0xf3, 0x2e, 0xaf, 0xb5, 0x57,
	0xff, 0x76, 0x58, 0x86, 0x02, 0x94, 0x42, 0xdc, 0xce, 0x06
};

static const uint8_t PCR_uninitialized[PCR_DIGEST_LENGTH] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static void pcr_value_to_string(const uint8_t *value, char *str) {
	int i;
	for (i = 0; i < PCR_DIGEST_LENGTH; i++) {
		sprintf(&str[i * 2], "%02x", value[i]);
	}
}

uint32_t tpm_init(void) {
	uint32_t result;
	uint8_t disable;
	uint8_t deactivated;
	uint8_t pcr_value[PCR_DIGEST_LENGTH];
	TPM_STCLEAR_FLAGS sflags;

	RETURN_ON_FAILURE(tlcl_lib_init());

	result = tlcl_startup();
	if ((result != TPM_SUCCESS) &&
		(is_recovery_mode() || (result != TPM_E_INVALID_POSTINIT))) {
		/* Invalid postinit indicates that TPM_Startup has already been
		   executed. This situation occurs on reboots on the GFSC100
		   platform because a reboot does not powercycle the TPM */
		printk(KERN_WARNING "TPM_Startup failed: 0x%08x\n", result);
		return result;
	}

	/* Continue or start the self-test of all TPM functions. Untested
	   functions will return TPM_NEEDS_SELFTEST otherwise */
	RETURN_ON_FAILURE(tlcl_continue_self_test());

	if (!is_recovery_mode()) {
		struct pcr_value_pair *bootmode_pcr_values;

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

		if (get_secure_boot_mode() == SECURE) {
			bootmode_pcr_values = &pcr_values_secure_boot.normal_boot;
		} else {
			bootmode_pcr_values = &pcr_values_nonsecure_boot.normal_boot;
		}

		RETURN_ON_FAILURE(tlcl_read_pcr(0, pcr_value));
		if (!memcmp(pcr_value, PCR_uninitialized, PCR_DIGEST_LENGTH)) {
			/* PCR0 not initialized, extend it with the boot mode */
			RETURN_ON_FAILURE(tlcl_extend(0, bootmode_pcr_values->digest, NULL));
		} else {
			/* PCR0 is initialized from previous boot, verify the value matches */
			if (memcmp(pcr_value, bootmode_pcr_values->extended, PCR_DIGEST_LENGTH)) {
				char str[PCR_DIGEST_LENGTH * 2 + 1];
				printk("TPM: PCR0 mismatch\n");
				pcr_value_to_string(bootmode_pcr_values->extended, str);
				printk("  expected: %s\n", str);
				pcr_value_to_string(pcr_value, str);
				printk("  is: %s\n", str);
				return TPM_E_MUST_REBOOT;
			}
		}
	} else {
		const uint8_t *digest;

		printk(KERN_DEBUG "TPM: Clearing\n");
		RETURN_ON_FAILURE(tlcl_assert_physical_presence());
		RETURN_ON_FAILURE(tlcl_force_clear());

		RETURN_ON_FAILURE(tlcl_set_enable());
		RETURN_ON_FAILURE(tlcl_set_deactivated(0));

		if (get_secure_boot_mode() == SECURE) {
			digest = pcr_values_secure_boot.recovery_mode.digest;
		} else {
			digest = pcr_values_nonsecure_boot.recovery_mode.digest;
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
