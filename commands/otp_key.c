/*
 * otp_key - manages OTP codes for IBR secure boot
 *
 * (C) Copyright 2013 Google Inc.
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

#include <c2k_otp.h>
#include <command.h>
#include <errno.h>
#include <fcntl.h>
#include <fs.h>
#include <malloc.h>

#define OTP_KEY_SIZE_BYTES	256
#define OTP_KEY_SIZE_BITS	(OTP_KEY_SIZE_BYTES * 8)
#define OTP_HEADER_OFFSET_BITS	32

/*
 * Converts an RSA public key read in from a file to a bit array suitable
 * for otp_write.
 *
 * Note that this inverts each byte in the key, which is what we think
 * the hardware expects, based on a Mindspeed sample. Yet to be tested.
 */
static void _convert_to_bit_array(uint8_t *key, unsigned int key_size,
		uint8_t *bit_array) {
	int i, j, offset;
	uint8_t key_byte;

	for (i = 0, offset = 0; i < key_size; ++i) {
		key_byte = key[i];

		for (j = 0; j < 8; ++j, ++offset) {
			bit_array[offset] = (key_byte >> j) & 1;
		}
	}
}

/*
 * Dumps a key to stdout, for debugging.
 */
static void _dump_key(char *key_name, uint8_t *key, unsigned int key_size) {
	int i;

	printf("%s:\n", key_name);

	for (i = 0; i < key_size; ++i) {
		if ((i % 16) == 0) {
			printf("\n");
		}
		printf(" %.2x", key[i]);
	}
	printf("\n");
}

static int do_write_key(struct command *cmdtp, int argc, char *argv[])
{
	int fd, rv = 0;
	uint8_t *provided_key, *provided_key_bit_arr, *verify_buf;
	long offset = 0;

	if (argc < 2)
		return COMMAND_ERROR_USAGE;

	provided_key = xmalloc(OTP_KEY_SIZE_BYTES);
	provided_key_bit_arr = xmalloc(OTP_KEY_SIZE_BITS);
	verify_buf = xmalloc(OTP_KEY_SIZE_BYTES);

	fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		perror(argv[1]);
		rv = 1;
		goto end;
	}

	if (read(fd, provided_key, OTP_KEY_SIZE_BYTES) != OTP_KEY_SIZE_BYTES) {
		perror("file read");
		close(fd);
		rv = 1;
		goto end;
	}
	close(fd);

	if (argc > 2) {
		offset = simple_strtol(argv[2], NULL, 10);
		if (offset <= 0 || (offset & 0xFFFFFFFF) != offset) {
			printf("Error: invalid offset %s\n", argv[2]);
			rv = 1;
			goto end;
		}

		printf("For debug purposes, using an offset of %ld bytes\n", offset);

		offset *= 8;	// Convert from bytes to bits;
	} else {
		offset = OTP_HEADER_OFFSET_BITS;
	}

	/* Write the key to the OTP. */
	_convert_to_bit_array(provided_key, OTP_KEY_SIZE_BYTES, provided_key_bit_arr);
	otp_write(offset, provided_key_bit_arr, OTP_KEY_SIZE_BITS);

	/* Verify the write. */

	if (otp_read(offset, verify_buf, OTP_KEY_SIZE_BYTES) != 0) {
		printf("Error: otp_read failed, unable to verify write\n");
		rv = 1;
		goto end;
	}

	if (memcmp(provided_key, verify_buf, OTP_KEY_SIZE_BYTES) != 0) {
		printf("Key writing failed:\n\n");
		_dump_key("Provided key:", provided_key, OTP_KEY_SIZE_BYTES);
		printf("\n");
		_dump_key("Written key:", verify_buf, OTP_KEY_SIZE_BYTES);
		rv = 1;
		goto end;
	}

	printf("Key written and verified.\n");

end:
	free(provided_key);
	free(provided_key_bit_arr);
	free(verify_buf);

	return rv;
}

static const __maybe_unused char cmd_write_key_help[] =
"Usage: write_key <key_file> [offset]\n"
"Writes the given RSA public key to the OTP.\n"
"An optional offset can be used for debugging.\n";

BAREBOX_CMD_START(write_key)
	.cmd		= do_write_key,
	.usage		= "Write key to OTP",
	BAREBOX_CMD_HELP(cmd_write_key_help)
BAREBOX_CMD_END

static int do_verify_key(struct command *cmdtp, int argc, char *argv[])
{
	int fd, rv = 0;
	uint8_t *provided_key, *otp_key;
	long offset = 0;

	if (argc < 2)
		return COMMAND_ERROR_USAGE;

	provided_key = xmalloc(OTP_KEY_SIZE_BYTES);
	otp_key = xmalloc(OTP_KEY_SIZE_BYTES);

	fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		perror(argv[1]);
		rv = 1;
		goto end;
	}

	if (read(fd, provided_key, OTP_KEY_SIZE_BYTES) != OTP_KEY_SIZE_BYTES) {
		perror("file read");
		close(fd);
		rv = 1;
		goto end;
	}
	close(fd);

	if (argc > 2) {
		offset = simple_strtol(argv[2], NULL, 10);
		if (offset <= 0 || (offset & 0xFFFFFFFF) != offset) {
			printf("Error: invalid offset %s\n", argv[2]);
			rv = 1;
			goto end;
		}

		printf("For debug purposes, using an offset of %ld bytes\n", offset);

		offset *= 8;	// Convert from bytes to bits;
	} else {
		offset = OTP_HEADER_OFFSET_BITS;
	}

	if (otp_read(offset, otp_key, OTP_KEY_SIZE_BYTES) != 0) {
		printf("Error: otp_read failed!\n");
		rv = 1;
		goto end;
	}

	if (memcmp(provided_key, otp_key, OTP_KEY_SIZE_BYTES) != 0) {
		printf("OTP key mismatch!\n\n");
		_dump_key("provided_key", provided_key, OTP_KEY_SIZE_BYTES);
		printf("\n");
		_dump_key("otp_key", otp_key, OTP_KEY_SIZE_BYTES);
		rv = 1;
		goto end;
	}

	printf("Verified: OTP key matches provided key.\n");

end:
	free(provided_key);
	free(otp_key);

	return rv;
}

static const __maybe_unused char cmd_verify_key_help[] =
"Usage: verify_key <key_file> [offset]\n"
"Verifies that the key in key_file matches the one written to the OTP.\n"
"An optional offset can be used for debugging\n";

BAREBOX_CMD_START(verify_key)
	.cmd		= do_verify_key,
	.usage		= "verify key written to OTP",
	BAREBOX_CMD_HELP(cmd_verify_key_help)
BAREBOX_CMD_END

static int do_enable_auth(struct command *cmdtp, int argc, char *argv[])
{
	uint8_t bytes[2], one = 1;

	/* Disable JTAG */
	otp_write(0, &one, 1);

	/* Disable debug mode */
	otp_write(8, &one, 1);

	/* Enable authentication */
	otp_write(9, &one, 1);

	/* Set the key size. This is actually a combination of writing 0 to offset 10
	 * and 1 to offset 11, but there's no need to write 0s. */
	otp_write(11, &one, 1);

	/* The other item to be set is the package type, to offset 12, but for us this
	 * is 0, so there's no need to write it. */

	/* Now verify that the correct bytes were written */
	if (otp_read(0, bytes, 2) != 0) {
		printf("Error: otp_read failed, unable to verify authentication enabled\n");
		return 1;
	}

	if (bytes[0] != 0x1 || bytes[1] != 0xB) {
		printf("Error: Verification failed!\n");
		printf("Byte 0 was %.2x (expected %.2x)\n", bytes[0], 0x1);
		printf("Byte 1 was %.2x (expected %.2x)\n", bytes[1], 0xB);
		return 1;
	}

	printf("Verification suceeded. Authentication enabled.\n");

	return 0;
}

static const __maybe_unused char cmd_enable_auth_help[] =
"Usage: enable_auth\n"
"Flips the authentication bit in the OTP.\n"
"WARNING: After running this, device will only boot securely.\n";

BAREBOX_CMD_START(enable_auth)
	.cmd		= do_enable_auth,
	.usage		= "flip auth bit in OTP",
	BAREBOX_CMD_HELP(cmd_enable_auth_help)
BAREBOX_CMD_END

static int do_enable_auth_jtag_on(struct command *cmdtp, int argc, char *argv[])
{
	uint8_t bytes[2], one = 1;

	/* Disable debug mode */
	otp_write(8, &one, 1);

	/* Enable authentication */
	otp_write(9, &one, 1);

	/* Set the key size. This is actually a combination of writing 0 to offset 10
	 * and 1 to offset 11, but there's no need to write 0s. */
	otp_write(11, &one, 1);

	/* The other item to be set is the package type, to offset 12, but for us this
	 * is 0, so there's no need to write it. */

	/* Now verify that the correct bytes were written */
	if (otp_read(0, bytes, 2) != 0) {
		printf("Error: otp_read failed, unable to verify authentication enabled\n");
		return 1;
	}

	if ((bytes[1] & 0xB) != 0xB) {
		printf("Error: Verification failed!\n");
		printf("Byte 0 was %.2x\n", bytes[0]);
		printf("Byte 1 was %.2x\n", bytes[1]);
		return 1;
	}

	printf("Verification suceeded. Authentication enabled.\n");

	return 0;
}

static const __maybe_unused char cmd_enable_auth_jtag_on_help[] =
"Usage: enable_auth_jtag_on\n"
"Flips the authentication bit in the OTP but leaves JTAG on.\n"
"WARNING: After running this, device will only boot securely.\n";

BAREBOX_CMD_START(enable_auth_jtag_on)
	.cmd		= do_enable_auth_jtag_on,
	.usage		= "set auth bit in OTP, leave JTAG enabled",
	BAREBOX_CMD_HELP(cmd_enable_auth_jtag_on_help)
BAREBOX_CMD_END

static int do_print_otp_config(struct command *cmdtp, int argc, char *argv[])
{
	uint8_t bytes[2];

	/* Now verify that the correct bytes were written */
	if (otp_read(0, bytes, 2) != 0) {
		printf("Error: otp_read failed\n");
		return 1;
	}

	printf("0x%.2x 0x%.2x\n", bytes[0], bytes[1]);

	return 0;
}

static const __maybe_unused char cmd_print_otp_config_help[] =
"Usage: print_otp_config\n";

BAREBOX_CMD_START(print_otp_config)
	.cmd		= do_print_otp_config,
	.usage		= "print OTP config bytes",
	BAREBOX_CMD_HELP(cmd_print_otp_config_help)
BAREBOX_CMD_END
