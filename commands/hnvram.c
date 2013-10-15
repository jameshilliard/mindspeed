/*
 * hnvram - load code from an "hnvram" formatted partition
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

#include <common.h>
#include <command.h>
#include <fs.h>
#include <fcntl.h>
#include <linux/ctype.h>
#include <errno.h>
#include <environment.h>
#include <asm/byteorder.h>
#include <malloc.h>

#define HNVRAM_BLOCKSIZE  0x00020000
#define HNVRAM_B0_OFFSET  0x00000000
#define HNVRAM_B1_OFFSET  0x00100000
#define HNVRAM_B2_OFFSET  0x00140000

#if 0
#define BUGMSG(fmt, args...) printf(fmt, ##args)
#else
#define BUGMSG(fmt, args...) do {} while (0)
#endif

/* these keys are stored in binary format for historical reasons */
const char *hnvram_binary_keys[] = {
	"LOADER_VERSION",
	"MAC_ADDR",
	"MAC_ADDR_MOCA",
	"MAC_ADDR_BT",
	"GPN",
	"HDCP_KEY",
	"DTCP_KEY",
};


int read_u8(const char **p) {
	int v = *(const unsigned char *)(*p);
	*p += 1;
	return v;
}


int read_s32_be(const char **p) {
	const unsigned char *vp = (const unsigned char *)*p;
	*p += 4;
	return (vp[0]<<24) + (vp[1]<<16) + (vp[2]<<8) + vp[3];
}


int read_u16_le(const char **p) {
	const unsigned char *up = (const unsigned char *)(*p);
	*p += 2;
	return up[0] + (up[1] << 8);
}


int is_hnvram_binary(const char *name, int namelen) {
	int i;
	for (i = 0; i < ARRAY_SIZE(hnvram_binary_keys); i++) {
		const char *k = hnvram_binary_keys[i];
		if ((int)strlen(k) == namelen && strncmp(k, name, namelen) == 0) {
			return 1;
		}
	}
	return 0;
}


char *encode_hex(const char *s, int len) {
	char *optr, *out = xmalloc(len * 2 + 1);
	for (optr = out; len > 0; len--) {
		sprintf(optr, "%02x", read_u8(&s));
		optr += 2;
	}
	return out;
}


char *encode_mac(const char *mac) {
	int i;
	char *out = xmalloc(6*2 + 5 + 2);
	for (i = 0; i < 6; i++) {
		sprintf(out + i * 3, "%02X:", read_u8(&mac));
	}
	out[6*2 + 5] = '\0';
	return out;
}


static void _copy_setenv(const char *name, int namelen,
		const char *val, int vallen) {
	char *n, *v;
	BUGMSG("csenv\n");
	if (namelen + vallen < 128) {
		n = xmalloc(4 + namelen + 1);
		v = xmalloc(vallen + 1);
		memcpy(n, "HNV_", 4);
		memcpy(n + 4, name, namelen);
		memcpy(v, val, vallen);
		n[namelen+4] = 0;
		v[vallen] = 0;
		setenv(n, v);
		free(n);
		free(v);
	}
}


static void _parse_hnvram(const char *buf, int len) {
	// An hnvram structure.	Format is a tag-length-value sequence of:
	//    [1 byte]   type (1 for notdone, 1 for done)
	//    [4 bytes]  record length
	//    [1 byte]   key length
	//    [x bytes]  key
	//    [4 bytes]  value length
	//    [y bytes]  value
	int rectype, reclen, namelen, vallen;
	const char *name, *val, *p = buf;
	while (p - buf <= len + 11) {
		rectype = read_u8(&p);
		BUGMSG("rectype %02X\n", rectype);
		if (rectype == 0x00) {
			// done
			break;
		}
		if (rectype != 0x01) {
			BUGMSG("error: hnvram invalid rectype %x\n", rectype);
			return;
		}
		reclen = read_s32_be(&p);
		if (reclen <= 6 || (p - buf) + reclen >= len) {
			BUGMSG("error: hnvram invalid reclen\n");
			return;
		}
		namelen = read_u8(&p);
		BUGMSG("namelen %d\n", namelen);
		if (namelen < 1 || (p - buf) + namelen >= len) {
			BUGMSG("error: hnvram invalid namelen\n");
			return;
		}
		name = p;
		p += namelen;
		vallen = read_s32_be(&p);
		BUGMSG("vallen %d\n", vallen);
		if (vallen < 0 || (p - buf) + vallen >= len) {
			BUGMSG("error: hnvram invalid vallen\n");
			return;
		}
		val = p;
		p += vallen;
		if (vallen == 6 && namelen >= 8 &&
				strncmp("mac_addr", name, 8) == 0) {
			char *macstr = encode_mac(val);
			_copy_setenv(name, namelen, macstr, strlen(macstr));
			free(macstr);
		} else if (is_hnvram_binary(name, namelen)) {
			char *hexstr = encode_hex(val, vallen);
			_copy_setenv(name, namelen, hexstr, strlen(hexstr));
			free(hexstr);
		} else {
			_copy_setenv(name, namelen, val, vallen);
		}
	}
}


static int _read_parse_hnvram(int fd, char *buf, off_t offset, size_t len) {
	if (lseek(fd, offset, SEEK_SET) != offset) {
		perror("lseek");
		return 1;
	}
	if (read(fd, buf, len) != len) {
		perror("read");
		return 2;
	}
	_parse_hnvram(buf, len);
	return 0;
}


static int do_hnvram(struct command *cmdtp, int argc, char *argv[])
{
	int fd, rv;
	char *buf;

	if (argc < 3 || strcmp(argv[1], "from") != 0)
		return COMMAND_ERROR_USAGE;
	fd = open(argv[2], O_RDONLY);
	if (fd < 0) {
		perror(argv[2]);
		return 1;
	}

	buf = xmalloc(HNVRAM_BLOCKSIZE);
	rv += _read_parse_hnvram(fd, buf, HNVRAM_B0_OFFSET, HNVRAM_BLOCKSIZE);
	rv += _read_parse_hnvram(fd, buf, HNVRAM_B1_OFFSET, HNVRAM_BLOCKSIZE);
	rv += _read_parse_hnvram(fd, buf, HNVRAM_B2_OFFSET, HNVRAM_BLOCKSIZE);
	free(buf);
	close(fd);

	return 0;
}

static const __maybe_unused char cmd_hnvram_help[] =
"Usage: hnvram from <filename>\n"
"Parse hnvram from <filename> into environment vars named HNV_<name>\n"
"for each var named <name> in the hnvram structure.\n";

BAREBOX_CMD_START(hnvram)
	.cmd		= do_hnvram,
	.usage		= "parse hnvram data into the env",
	BAREBOX_CMD_HELP(cmd_hnvram_help)
BAREBOX_CMD_END
