/*
 * (C) Copyright 2000-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

/*
 * Boot support
 */
#include <common.h>
#include <watchdog.h>
#include <driver.h>
#include <command.h>
#include <image.h>
#include <malloc.h>
#include <zlib.h>
#include <bzlib.h>
#include <environment.h>
#include <asm/byteorder.h>
#include <xfuncs.h>
#include <getopt.h>
#include <fcntl.h>
#include <fs.h>
#include <errno.h>
#include <boot.h>
#include <rtc.h>
#include <init.h>
#include <asm-generic/memory_layout.h>
#include <rsa_verify.h>
#include <sha1.h>
#include <secure_boot.h>

#ifdef CONFIG_NAND_COMCERTO_ECC_HW_BCH
extern uint32_t temp_nand_ecc_errors[];
#endif
/*
 *  Continue booting an OS image; caller already has:
 *  - copied image header to global variable `header'
 *  - checked header magic number, checksums (both header & image),
 *  - verified image architecture (PPC) and type (KERNEL or MULTI),
 *  - loaded (first part of) image to header load address,
 *  - disabled interrupts.
 */
typedef void boot_os_Fcn(struct command *cmdtp, int flag,
			  int	argc, char *argv[],
			  ulong	addr,		/* of image to boot */
			  ulong	*len_ptr,	/* multi-file image length table */
			  int	verify);	/* getenv("verify")[0] != 'n' */

#ifndef CFG_BOOTM_LEN
#define CFG_BOOTM_LEN	0x800000	/* use 8MByte as default max gunzip size */
#endif

#ifdef CONFIG_SILENT_CONSOLE
static void
fixup_silent_linux ()
{
	char buf[256], *start, *end;
	char *cmdline = getenv ("bootargs");

	/* Only fix cmdline when requested */
	if (!(gd->flags & GD_FLG_SILENT))
		return;

	debug ("before silent fix-up: %s\n", cmdline);
	if (cmdline) {
		if ((start = strstr (cmdline, "console=")) != NULL) {
			end = strchr (start, ' ');
			strncpy (buf, cmdline, (start - cmdline + 8));
			if (end)
				strcpy (buf + (start - cmdline + 8), end);
			else
				buf[start - cmdline + 8] = '\0';
		} else {
			strcpy (buf, cmdline);
			strcat (buf, " console=");
		}
	} else {
		strcpy (buf, "console=");
	}

	setenv ("bootargs", buf);
	debug ("after silent fix-up: %s\n", buf);
}
#endif /* CONFIG_SILENT_CONSOLE */

int relocate_image(struct image_handle *handle, void *load_address)
{
	image_header_t *hdr = &handle->header;
	unsigned long len  = image_get_size(hdr);
	unsigned long data = (unsigned long)(handle->data);

#if defined CONFIG_CMD_BOOTM_ZLIB || defined CONFIG_CMD_BOOTM_BZLIB
	uint	unc_len = CFG_BOOTM_LEN;
#endif

	switch (image_get_comp(hdr)) {
	case IH_COMP_NONE:
		if(image_get_load(hdr) == data) {
			printf ("   XIP ... ");
		} else {
			memmove ((void *) image_get_load(hdr), (uchar *)data, len);
		}
		break;
#ifdef CONFIG_CMD_BOOTM_ZLIB
	case IH_COMP_GZIP:
		printf ("   Uncompressing ... ");
		if (gunzip (load_address, unc_len,
			    (uchar *)data, &len) != 0)
			return -1;
		break;
#endif
#ifdef CONFIG_CMD_BOOTM_BZLIB
	case IH_COMP_BZIP2:
		printf ("   Uncompressing ... ");
		/*
		 * If we've got less than 4 MB of malloc() space,
		 * use slower decompression algorithm which requires
		 * at most 2300 KB of memory.
		 */
		if (BZ2_bzBuffToBuffDecompress (load_address,
						&unc_len, (char *)data, len,
						MALLOC_SIZE < (4096 * 1024), 0)
						!= BZ_OK)
			return -1;
		break;
#endif
	default:
		printf("Unimplemented compression type %d\n",
		       image_get_comp(hdr));
		return -1;
	}

	return 0;
}
EXPORT_SYMBOL(relocate_image);

/*
 * Checks whether the kernel image is in legacy or current format.
 *
 * It is technically possible for an attacker to wait for this method
 * to be called, then to swap out the kernel image. However, that doesn't
 * gain them anything, except (probably) a failed boot.
 *
 * Returns 0 on success, non-zero on failure. The is_legacy parameter is set
 * to indicate whether or not the kernel image is a legacy one. (1: is legacy,
 * 0: is non-legacy).
 */
static int _check_if_legacy_kernel_image(const char *filename, int *is_legacy)
{
	int fd, rv = 0;
	uint8_t sb_header[SB_HEADER_LEN];

	*is_legacy = 0;

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		return -1;
	}

	if (read(fd, sb_header, SB_HEADER_LEN) != SB_HEADER_LEN) {
		rv = -1;
		goto end;
	}

	/*
	 * Unfortunately, I can only think of one check we can do to try and detect
	 * legacy kernel images, based on the header of the non-legacy format:
	 *
	 *   1. The header has 8 bytes of 0 padding at the end.
	 */

	if (_get_le_uint32(&sb_header[8]) != 0 ||
			_get_le_uint32(&sb_header[12]) != 0) {
		*is_legacy = 1;
	}

end:
	close(fd);

	return rv;
}

struct image_handle *map_image(const char *filename, int verify,
		int legacy_format, int secure_boot)
{
	int fd;
	uint32_t checksum, len;
	struct image_handle *handle;
	image_header_t *header;

	/* Used for secure boot. */
	sha1_context ctx;
	uint32_t total_image_len, sig_offset, verity_table_len;
	uint8_t sb_header[SB_HEADER_LEN], verity_info[SB_INFO_LEN], *verity_table;
	uint8_t hash[SHA1_SUM_LEN], sig[SB_SIG_LEN];

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		printf("could not open: %s\n", errno_str());
		return NULL;
	}

	handle = xzalloc(sizeof(struct image_handle) * 2);
	header = &handle->header;
	verity_table = NULL;

	if (legacy_format && secure_boot) {
		printf("Error: Cannot perform a secure boot of a legacy kernel image!\n");
		goto err_out;
	}

	if (!legacy_format) {
		if (secure_boot) {
			puts ("   Authenticating Image ... ");

			/*
			 * As the OS data is read in a piece at a time, into various structs, we
			 * construct the SHA data along the way and then verify it at the end.
			 */
			sha1_starts(&ctx);
		}

		if (read(fd, sb_header, SB_HEADER_LEN) != SB_HEADER_LEN) {
			printf("Could not read signature header!\n");
			goto err_out;
		}

		/* The verity header is currently unused, but is included in the hash. */
		if (read(fd, verity_info, SB_INFO_LEN) != SB_INFO_LEN) {
			printf("Could not read verity info!\n");
			goto err_out;
		}

		if (secure_boot) {
			sha1_update(&ctx, verity_info, SB_INFO_LEN);
		}
	}

	if (read(fd, header, image_get_header_size()) < 0) {
		printf("could not read: %s\n", errno_str());
		goto err_out;
	}

	if (secure_boot) {
		/* Do the SHA update before the CRC check, as CRC mutates the header. */
		sha1_update(&ctx, (uchar *) header, image_get_header_size());
	}

	if (image_get_magic(header) != IH_MAGIC) {
		puts ("\nBad OS Header Magic Number\n");
		goto err_out;
	}

	checksum = image_get_hcrc(header);
	header->ih_hcrc = 0;

	if (crc32 (0, (uchar *)header, image_get_header_size()) != checksum) {
		puts ("\nBad OS Header Checksum\n");
		goto err_out;
	}
	len  = image_get_size(header);

	if (!legacy_format) {
		/* In the non-legacy format, OS image (incl header) is 4096-byte padded. */
		len += image_get_header_size();
		len = ((len + 4095) / 4096) * 4096;
		len -= image_get_header_size();
	}

	handle->data = memmap(fd, PROT_READ);
	if (handle->data == (void *)-1) {
		handle->data = xmalloc(len);
		handle->flags = IH_MALLOC;
		if (read(fd, handle->data, len) < 0) {
			printf("could not read: %s\n", errno_str());
			goto err_out;
		}
	} else {
		handle->data = (void *)((unsigned long)handle->data +
						       SB_HEADER_LEN +
						       SB_INFO_LEN +
						       image_get_header_size());
	}

	if (!legacy_format) {
		/* The final piece of data to be read is the verity table. */
		total_image_len = _get_le_uint32(&sb_header[0]);
		verity_table_len = total_image_len - SB_INFO_LEN -
				image_get_header_size() - len;
		verity_table = xmalloc(verity_table_len);

		if (read(fd, verity_table, verity_table_len) != verity_table_len) {
			printf("\nCould not read verity table!\n");
			goto err_out;
		}

		if (secure_boot) {
			/* Finish off the SHA-1 hash. */
			sha1_update(&ctx, handle->data, len);
			sha1_update(&ctx, verity_table, verity_table_len);
			sha1_finish(&ctx, hash);

			/* The signature can be found via an offset relative to the SB header end. */
			sig_offset = _get_le_uint32(&sb_header[4]);
			lseek(fd, SB_HEADER_LEN + sig_offset, SEEK_SET);
			if (read(fd, sig, SB_SIG_LEN) != SB_SIG_LEN) {
				printf("\nCould not read signature!\n");
				goto err_out;
			}

			if (rsa_verify(sig, SB_SIG_LEN, hash) != 0) {
				printf("Authentication failed!\n");
				goto err_out;
			}

			puts ("OK\n");
		}

		/*
		 * Slightly awkwardly, the OS CRC verification requires the original OS
		 * length, not the padded version length, so we have to restore it first.
		 */
		len = image_get_size(header);
	}

	if (verify) {
		puts ("   Verifying Checksum ... ");
		if (crc32 (0, handle->data, len) != image_get_dcrc(header)) {
			printf ("Bad Data CRC\n");

#ifdef CONFIG_NAND_COMCERTO_ECC_HW_BCH
			uint8_t i;
			for (i = 0; i < 4; i++)
				printf("temp_nand_ecc_errors[%d] = %d \n", i, temp_nand_ecc_errors[i]);
#endif
			goto err_out;
		}
		puts ("OK\n");
	}

	image_print_contents(header);

	close(fd);

	/*
	 * Multi-file uImage support.  If this is a multi-file uImage, then
	 * the header will say so, and the blobs in the file are in this
	 * predefined sequence: kernel, initrd, devicetree, [other...].
	 *
	 * The code above has already verified the checksum of the entire
	 * thing (ie. all blobs together) and loaded it all into a single
	 * RAM location.  Each blob in the file is already aligned to the
	 * nearest 32 bits.  So now we just need to appease the loader by
	 * producing one image_handle for each blob and creating "virtual"
	 * headers for each one.
	 *
	 * Earlier versions of this function returned only one handle.  Now
	 * we return an array of handles, terminated with a handle that
	 * is all-zeroes (in particular, the 'data' field is NULL).  Other
	 * than the code for unmap_image(), this is backward compatible with
	 * the old return value.
	 */
	if (image_get_type(header) == IH_TYPE_MULTI) {
		/*
		 * The bloblen section precedes the actual blobs.
		 * It's a series of 32-bit blob lengths, terminated
		 * by a 0.
		 */
		int numblobs, i, fixup;
		uint32_t *bloblen = handle->data, blobofs;

		printf("Multi-file uImage detected.\n");

		for (numblobs = 0; bloblen[numblobs]; numblobs++) { }
		if (!numblobs) {
			printf("Weird: multi-file image with zero blobs?\n");
			return handle;  /* leave header intact */
		}

		fixup = (numblobs + 1) * 4;
		blobofs = fixup;

		handle = xrealloc(handle,
			sizeof(struct image_handle) * (numblobs + 1));
		memset(&handle[1], 0,
			sizeof(struct image_handle) * numblobs);

		/*
		 * The kernel data area might be malloc'd, which means we
		 * can't just adjust handle[0].data to skip the bloblen[]
		 * header, because that would stop free() from working.
		 * Instead, move the load address backward so that the kernel
		 * itself will end up being loaded where it's supposed to be.
		 *
		 * We also have to set the size of the kernel to include
		 * the bloblen[] array.
		 */
		image_set_load(&handle[0].header,
			image_get_load(&handle[0].header) - fixup);
		image_set_size(&handle[0].header,
			uimage_to_cpu(bloblen[0]) + fixup);

		for (i = 0; i < numblobs; i++) {
			uint32_t type = 0;
			uint32_t len = uimage_to_cpu(bloblen[i]);
			if (i != 0) {
				memcpy(&handle[i], &handle[0],
					sizeof(handle[0]));
				handle[i].data = handle[0].data + blobofs;

				/*
				 * Blobs other than the first are not
				 * themselves separately malloc'd.
				 */
				handle[i].flags &= ~IH_MALLOC;

				/*
				 * The load address is only supplied for the
				 * first blob, which is the kernel. The others
				 * can be loaded anywhere; just set the load
				 * address to indicate their current location.
				 *
				 * NOTE(apenwarr): Sensitive to memory map.
				 * The ARM kernel (at least) writes over
				 * very low memory, plus memory right after
				 * the kernel image, during boot.  This code
				 * works only because in our situation we
				 * put the malloc() area fairly high in RAM,
				 * which the kernel doesn't overwrite until
				 * it finishes extracting the initrd.
				 */
				image_set_load(&handle[i].header,
					(uint32_t)handle[i].data);
				image_set_size(&handle[i].header, len);
				image_set_ep(&handle[i].header, 0);
			}

			switch (i) {
			case 0: type = IH_TYPE_KERNEL; break;
			case 1: type = IH_TYPE_RAMDISK; break;
			case 2: type = IH_TYPE_FLATDT; break;
			default: type= IH_TYPE_INVALID; break;
			}
			image_set_type(&handle[i].header, type);

			printf("Blob #%d: load=%08x len=%08x ptr=%p %s\n", i,
				image_get_load(&handle[i].header),
				image_get_size(&handle[i].header),
				handle[i].data,
				image_get_type_name(
					image_get_type(&handle[i].header)));

			/* all blobs are 32-bit aligned */
			blobofs += (len + 3) & ~3;
		}
	}

	return handle;
err_out:
	close(fd);
	if (handle->flags & IH_MALLOC)
		free(handle->data);
	free(handle);
	if (verity_table)
		free(verity_table);
	return NULL;
}
EXPORT_SYMBOL(map_image);

void unmap_image(struct image_handle *handle)
{
	struct image_handle *first_handle = handle;
	while (handle->data) {
		if (handle->flags & IH_MALLOC)
			free(handle->data);
		handle->data = NULL;
		handle++;
	}
	free(first_handle);
}
EXPORT_SYMBOL(unmap_image);

static LIST_HEAD(handler_list);

int register_image_handler(struct image_handler *handler)
{
	list_add_tail(&handler->list, &handler_list);
	return 0;
}

static int initrd_handler_parse_options(struct image_data *data, int opt,
		char *optarg)
{
	switch(opt) {
	case 'r':
		printf("use initrd %s\n", optarg);
		data->initrd = map_image(optarg, data->verify, 1, 0);
		if (!data->initrd)
			return -1;
		return 0;
	default:
		return 1;
	}
}

static struct image_handler initrd_handler = {
	.cmdline_options = "r:",
	.cmdline_parse = initrd_handler_parse_options,
	.help_string = " -r <initrd>    specify an initrd image",
};

static int initrd_register_image_handler(void)
{
	return register_image_handler(&initrd_handler);
}

late_initcall(initrd_register_image_handler);

static int handler_parse_options(struct image_data *data, int opt, char *optarg)
{
	struct image_handler *handler;
	int ret;

	list_for_each_entry(handler, &handler_list, list) {
		if (!handler->cmdline_parse)
			continue;

		ret = handler->cmdline_parse(data, opt, optarg);
		if (ret > 0)
			continue;

		return ret;
	}

	return -1;
}

static int do_bootm(struct command *cmdtp, int argc, char *argv[])
{
	ulong	iflag;
	int	opt, secure_boot, legacy_format;
	image_header_t *os_header;
	struct image_handle *os_handle = NULL, *initrd_handle = NULL;
	struct image_handler *handler;
	struct image_data data;
	char options[53]; /* worst case: whole alphabet with colons */
#if !(defined(CONFIG_FORCE_KERNEL_AUTH) || defined(CONFIG_DEVELOPER_BAREBOX))
	secure_boot_mode_t boot_mode;
#endif

	memset(&data, 0, sizeof(struct image_data));
	data.verify = 1;

	/* Collect options from registered handlers */
	strcpy(options, "nh");
	list_for_each_entry(handler, &handler_list, list) {
		if (handler->cmdline_options)
			strcat(options, handler->cmdline_options);
	}

	while((opt = getopt(argc, argv, options)) > 0) {
		switch(opt) {
		case 'n':
			data.verify = 0;
			break;
		case 'h':
			printf("bootm advanced options:\n");

			list_for_each_entry(handler, &handler_list, list) {
				if (handler->help_string)
					printf("%s\n", handler->help_string);
			}

			return 0;
		default:
			if (!handler_parse_options(&data, opt, optarg))
				continue;

			return 1;
		}
	}

	if (optind == argc)
		return COMMAND_ERROR_USAGE;

#ifdef CONFIG_FORCE_KERNEL_AUTH
	printf("Secure boot forced; will authenticate kernel.\n");
	secure_boot = 1;
#elif defined(CONFIG_DEVELOPER_BAREBOX)
	printf("Developer barebox detected; will not authenticate kernel.\n");
	secure_boot = 0;
#else
	boot_mode = get_secure_boot_mode();
	if (boot_mode == UNKNOWN) {
		printf("Error: Unable to determine secure boot status!\n");
		return 1;
	}
	secure_boot = (boot_mode == SECURE);
#endif

	if (_check_if_legacy_kernel_image(argv[optind], &legacy_format)) {
		printf("Error: Unable to determine whether kernel is legacy/non-legacy!\n");
		return 1;
	}

	os_handle = map_image(argv[optind], data.verify, legacy_format, secure_boot);
	if (!os_handle)
		return 1;
	data.os = os_handle;
	if (!data.initrd && os_handle[1].data)
		data.initrd = &os_handle[1];

	os_header = &os_handle->header;

	if (image_get_arch(os_header) != IH_ARCH) {
		printf("Unsupported Architecture 0x%x\n",
		       image_get_arch(os_header));
		goto err_out;
	}

	/*
	 * We have reached the point of no return: we are going to
	 * overwrite all exception vector code, so we cannot easily
	 * recover from any failures any more...
	 */

	iflag = disable_interrupts();

	puts ("OK\n");

	/* loop through the registered handlers */
	list_for_each_entry(handler, &handler_list, list) {
		if (image_get_os(os_header) == handler->image_type) {
			handler->bootm(&data);
			printf("handler returned!\n");
			goto err_out;
		}
	}

	printf("no image handler found for image type %d\n",
	       image_get_os(os_header));

err_out:
	if (os_handle)
		unmap_image(os_handle);
	if (initrd_handle)
		unmap_image(initrd_handle);
	return 1;
}

BAREBOX_CMD_HELP_START(bootm)
BAREBOX_CMD_HELP_USAGE("bootm [-n] image\n")
BAREBOX_CMD_HELP_SHORT("Boot an application image.\n")
BAREBOX_CMD_HELP_OPT  ("-n",  "Do not verify the image (speeds up boot process)\n")
BAREBOX_CMD_HELP_END

BAREBOX_CMD_START(bootm)
	.cmd		= do_bootm,
	.usage		= "boot an application image",
	BAREBOX_CMD_HELP(cmd_bootm_help)
BAREBOX_CMD_END

/**
 * @page bootm_command

\todo What does bootm do, what kind of image does it boot?

 */

#ifdef CONFIG_CMD_IMI
static int do_iminfo(struct command *cmdtp, int argc, char *argv[])
{
	int	arg;
	ulong	addr;
	int     rcode=0;

	if (argc < 2) {
		return image_info (load_addr);
	}

	for (arg=1; arg <argc; ++arg) {
		addr = simple_strtoul(argv[arg], NULL, 16);
		if (image_info (addr) != 0) rcode = 1;
	}
	return rcode;
}

static int image_info (ulong addr)
{
	ulong	data, len, checksum;
	image_header_t *hdr = &header;

	printf ("\n## Checking Image at %08lx ...\n", addr);

	/* Copy header so we can blank CRC field for re-calculation */
	memmove (&header, (char *)addr, image_get_header_size());

	if (image_get_magic(hdr) != IH_MAGIC) {
		puts ("   Bad Magic Number\n");
		return 1;
	}

	data = (ulong)&header;
	len  = image_get_header_size();

	checksum = image_get_hcrc(hdr);
	hdr->ih_hcrc = 0;

	if (crc32 (0, (uchar *)data, len) != checksum) {
		puts ("   Bad Header Checksum\n");
		return 1;
	}

	/* for multi-file images we need the data part, too */
	print_image_hdr ((image_header_t *)addr);

	data = addr + image_get_header_size();
	len  = image_get_size(hdr);

	puts ("   Verifying Checksum ... ");
	if (crc32 (0, (uchar *)data, len) != image_get_dcrc(hdr)) {
		puts ("   Bad Data CRC\n");
		return 1;
	}
	puts ("OK\n");
	return 0;
}

BAREBOX_CMD_HELP_START(iminfo)
BAREBOX_CMD_HELP_USAGE("iminfo\n")
BAREBOX_CMD_HELP_SHORT("Print header information for an application image.\n")
BAREBOX_CMD_HELP_END

BAREBOX_CMD_START(iminfo)
	.cmd		= do_iminfo,
	.usage		= "print header information for an application image",
	BAREBOX_CMD_HELP(cmd_iminfo_help)
BAREBOX_CMD_END

#endif	/* CONFIG_CMD_IMI */

#ifdef CONFIG_BZLIB
void bz_internal_error(int errcode)
{
	printf ("BZIP2 internal error %d\n", errcode);
}
#endif /* CONFIG_BZLIB */

/**
 * @file
 * @brief Boot support for Linux
 */

/**
 * @page boot_preparation Preparing for Boot
 *
 * This chapter describes what's to be done to forward the control from
 * barebox to Linux. This part describes the generic part, below you can find
 * the architecture specific part.
 *
 * - @subpage arm_boot_preparation
 * - @subpage ppc_boot_preparation
 * - @subpage x86_boot_preparation
 */
