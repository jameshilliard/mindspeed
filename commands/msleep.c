/*
 * msleep.c - delay execution
 *
 * Copyright (c) 2015 Google Inc.
 * Copyright (c) 2007 Sascha Hauer <s.hauer@pengutronix.de>, Pengutronix
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <common.h>
#include <command.h>
#include <clock.h>

static int do_msleep(struct command *cmdtp, int argc, char *argv[])
{
        uint64_t start;
        ulong delay;

        if (argc != 2)
                return COMMAND_ERROR_USAGE;

        delay = simple_strtoul(argv[1], NULL, 10);

        start = get_time_ns();
        while (!is_timeout(start, delay * MSECOND)) {
                if (ctrlc())
                        return 1;
        }

        return 0;
}

BAREBOX_CMD_START(msleep)
        .cmd            = do_msleep,
        .usage          = "delay execution for n milliseconds",
BAREBOX_CMD_END
