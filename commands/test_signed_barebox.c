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

#include <command.h>
#include <errno.h>
#include <fs.h>
#include <fcntl.h>
#include <malloc.h>
#include <rsa_verify.h>
#include <sha1.h>
#include <linux/stat.h>

/*
 * Extracts an unsigned 4-byte little endian int from a byte pointer.
 *
 * It is the responsibility of the caller to move the pointer on after
 * calling this method.
 *
 * Note that we use uint32_t with the assumption that this will always
 * have a size of at least 4 bytes.
 */
static uint32_t _get_le_32_byte_int(uint8_t *ptr) {
  uint32_t value = 0;

  value |= *ptr++;
  value |= *ptr++ << 8;
  value |= *ptr++ << 16;
  value |= *ptr++ << 24;

  return value;
}

/*
 * Parses a signed barebox image, extracts the signature, and calculates a hash.
 *
 * TODO(smcgruer): When actual boot verification is implemented, this should be
 * moved to a shared library rather than duplicating it here and there.
 */
static int parse_image(uint8_t *image_ptr, uint8_t **sig, uint8_t hash[20])
{
	sha1_context ctx;
	uint32_t image_len, sig_offset;

	/* First 4 bytes should be the image length. */
	image_len = _get_le_32_byte_int(image_ptr);
	image_ptr += 4;

	/* Next 4 bytes are the signature offset after the header. */
	sig_offset = _get_le_32_byte_int(image_ptr);
	image_ptr += 4;

	/* Skip the header padding. */
	image_ptr += 8;

	/* Calculate the hash. */
	sha1_starts(&ctx);
	sha1_update(&ctx, image_ptr, image_len);
	sha1_finish(&ctx, hash);

	/* Skip to the signature. */
	image_ptr += sig_offset;

	*sig = image_ptr;

	return 0;
}

static int do_test_signed_barebox(struct command *cmdtp, int argc, char *argv[])
{
	uint8_t *image, *sig, hash[20];
	int image_fd, image_len, rv;
	struct stat st;

	if (argc < 2)
		return COMMAND_ERROR_USAGE;

	image_fd = open(argv[1], O_RDONLY);
	if (image_fd < 0) {
		perror(argv[1]);
		return 1;
	}

	stat(argv[1], &st);
	image_len = st.st_size;

	image = xmalloc(image_len);
	if (read(image_fd, image, image_len) != image_len) {
		printf("Error: unable to read image file\n");
		close(image_fd);
		free(image);
		return 1;
	}
	close(image_fd);

	if (parse_image(image, &sig, hash)) {
		printf("Error: Parse image failed\n");
		close(image_fd);
		free(image);
		free(sig);

		return 1;
	}

	rv = rsa_verify(sig, 256, hash);
	if (rv == 0) {
		printf("Image verified successfully!\n");
	} else {
		printf("Error: Image failed verification\n");
	}

	free(image);

	return rv;
}

static const __maybe_unused char cmd_test_signed_barebox_help[] =
"Usage: test_signed_barebox signed_image\n"
"Checks whether a given barebox image has been correctly signed.\n";

BAREBOX_CMD_START(test_signed_barebox)
	.cmd		= do_test_signed_barebox,
	.usage		= "Test barebox signature",
	BAREBOX_CMD_HELP(cmd_test_signed_barebox_help)
BAREBOX_CMD_END
