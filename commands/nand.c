/*
 * Copyright (c) 2008 Sascha Hauer <s.hauer@pengutronix.de>, Pengutronix
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
#include <fs.h>
#include <linux/stat.h>
#include <errno.h>
#include <malloc.h>
#include <getopt.h>
#include <xfuncs.h>
#include <init.h>
#include <ioctl.h>
#include <nand.h>
#include <linux/mtd/mtd-abi.h>
#include <fcntl.h>
#include <libgen.h>

#define NAND_ADD (1 << 0)
#define NAND_DEL (1 << 1)
#define NAND_MARKBAD (1 << 2)

static int do_nand(struct command *cmdtp, int argc, char *argv[])
{
	int opt;
	int command = 0, badblock = 0;

	while((opt = getopt(argc, argv, "adb:")) > 0) {
		if (command) {
			printf("only one command may be given\n");
			return 1;
		}

		switch (opt) {
		case 'a':
			command = NAND_ADD;
			break;
		case 'd':
			command = NAND_DEL;
			break;
		case 'b':
			command = NAND_MARKBAD;
			badblock = simple_strtoul(optarg, NULL, 0);
		}
	}

	if (command & NAND_ADD) {
		int ret = 0;
		while (optind < argc) {
			if (dev_add_bb_dev(basename(argv[optind]), NULL))
				ret = 1;

			optind++;
		}
		if (ret) return ret;
	}

	if (command & NAND_DEL) {
		while (optind < argc) {
			dev_remove_bb_dev(basename(argv[optind]));
			optind++;
		}
	}

	if (command & NAND_MARKBAD) {
		if (optind < argc) {
			int ret = 0, fd;

			printf("marking block at 0x%08x on %s as bad\n", badblock, argv[optind]);

			fd = open(argv[optind], O_RDWR);
			if (fd < 0) {
				perror("open");
				return 1;
			}

			ret = ioctl(fd, MEMSETBADBLOCK, (void *)badblock);
			if (ret)
				perror("ioctl");

			close(fd);
			return ret;
		}
	}

	return 0;
}

static const __maybe_unused char cmd_nand_help[] =
"Usage: nand [OPTION]...\n"
"nand related commands\n"
"  -a  <dev>  register a bad block aware device ontop of a normal nand device\n"
"  -d  <dev>  deregister a bad block aware device\n"
"  -b  <ofs> <dev> mark block at offset ofs as bad\n";

BAREBOX_CMD_START(nand)
	.cmd		= do_nand,
	.usage		= "NAND specific handling",
	BAREBOX_CMD_HELP(cmd_nand_help)
BAREBOX_CMD_END
