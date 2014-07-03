/*
 * otp_stability_test - test OTP stability.
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
#include <common.h>
#include <errno.h>
#include <fcntl.h>
#include <fs.h>
#include <malloc.h>
#include <init.h>
#include <types.h>
#include <mach/gpio.h>

#include <asm/hardware.h>
#include <asm/io.h>

#define OTP_KEY_SIZE_BYTES	256
#define OTP_HEADER_OFFSET_BITS	32

#define LED_BLINK_DELAY_MILLISECONDS 150

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

static void flash_led_forever(void) {
	/* Turn off both blue (GPIO_12) and red (GPIO_13) leds to start. */
	comcerto_gpio_set_0(GPIO_12);
	comcerto_gpio_set_0(GPIO_13);

	/* Flash the red LED continuously. */
	while (true) {
		mdelay(LED_BLINK_DELAY_MILLISECONDS);
		comcerto_gpio_set_1(GPIO_13);
		if (ctrlc()) {
			break;
		}

		mdelay(LED_BLINK_DELAY_MILLISECONDS);
		comcerto_gpio_set_0(GPIO_13);
		if (ctrlc()) {
			break;
		}
	}

	/* Reset to barebox norm: red on, blue off. */
	comcerto_gpio_set_0(GPIO_12);
	comcerto_gpio_set_1(GPIO_13);
}

/*
 * Checks that the value in the OTP at an offset matches a given key.
 *
 * Returns 1 if the values match, 0 otherwise.
 */
static int check_otp_value(uint8_t *expected_key, long offset) {
	int rv = 1;
	uint8_t *otp_key;

	otp_key = xmalloc(OTP_KEY_SIZE_BYTES);

	if (otp_read(offset, otp_key, OTP_KEY_SIZE_BYTES) != 0) {
		printf("Error: otp_read failed!\n");

		rv = 0;
		goto end;
	}

	if (memcmp(expected_key, otp_key, OTP_KEY_SIZE_BYTES) != 0) {
		printf("OTP key mismatch!\n\n");
		_dump_key("Expected value", expected_key, OTP_KEY_SIZE_BYTES);
		printf("\n");
		_dump_key("Read value", otp_key, OTP_KEY_SIZE_BYTES);

		rv = 0;
		goto end;
	}

end:
	free(otp_key);
	return rv;
}

static int do_otp_stability_test(struct command *cmdtp, int argc, char *argv[])
{
	int fd, rv = 0;
	uint8_t *provided_key;
	long offset = 0, good_read_count = 0;

	if (argc < 2)
		return COMMAND_ERROR_USAGE;

	provided_key = xmalloc(OTP_KEY_SIZE_BYTES);

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

	while (true) {
		if (!check_otp_value(provided_key, offset)) {
			flash_led_forever();

			/* User has pressed ctrl-c. */
			break;
		}

		good_read_count++;
		if (good_read_count % 1000 == 0) {
			printf("OTP has read correctly %ld times\n", good_read_count);
		}

		if (ctrlc())
			break;
	}

end:
	free(provided_key);

	return rv;
}

static const __maybe_unused char cmd_otp_stability_test_help[] =
"Usage: otp_stability_test <key_file> [offset]\n"
"Performs a stability test on the OTP, ensuring that the correct value\n"
"is always read\n";

BAREBOX_CMD_START(otp_stability_test)
	.cmd		= do_otp_stability_test,
	.usage		= "perform OTP stability test",
	BAREBOX_CMD_HELP(cmd_otp_stability_test_help)
BAREBOX_CMD_END

