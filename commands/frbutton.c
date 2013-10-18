/*
 * Copyright (c) 2013, Google Inc.
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <asm/hardware.h>
#include <asm/io.h>
#include <gpio.h>

static int do_frbutton(struct command *cmdtp, int argc, char *argv[])
{
	int status;
	/* Factory reset button is connected to GPIO 6 and is active low. */
	status = !comcerto_gpio_read(GPIO_6);
	printf("Factory reset button pressed: %s\n", status ? "yes" : "no" );
	return !status;
}

BAREBOX_CMD_START(frbutton)
	.cmd		= do_frbutton,
	.usage		= "return status of factory reset button",
BAREBOX_CMD_END

