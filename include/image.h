/*
 * (C) Copyright 2000-2005
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 ********************************************************************
 * NOTE: This header file defines an interface to barebox. Including
 * this (unmodified) header file in another file is considered normal
 * use of barebox, and does *not* fall under the heading of "derived
 * work".
 ********************************************************************
 */

#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <linux/types.h>
#ifdef __BAREBOX__
#include <asm/byteorder.h>
#include <stdio.h>
#include <string.h>
#endif

/*
 * Operating System Codes
 */
#define IH_OS_INVALID		0	/* Invalid OS	*/
#define IH_OS_OPENBSD		1	/* OpenBSD	*/
#define IH_OS_NETBSD		2	/* NetBSD	*/
#define IH_OS_FREEBSD		3	/* FreeBSD	*/
#define IH_OS_4_4BSD		4	/* 4.4BSD	*/
#define IH_OS_LINUX		5	/* Linux	*/
#define IH_OS_SVR4		6	/* SVR4		*/
#define IH_OS_ESIX		7	/* Esix		*/
#define IH_OS_SOLARIS		8	/* Solaris	*/
#define IH_OS_IRIX		9	/* Irix		*/
#define IH_OS_SCO		10	/* SCO		*/
#define IH_OS_DELL		11	/* Dell		*/
#define IH_OS_NCR		12	/* NCR		*/
#define IH_OS_LYNXOS		13	/* LynxOS	*/
#define IH_OS_VXWORKS		14	/* VxWorks	*/
#define IH_OS_PSOS		15	/* pSOS		*/
#define IH_OS_QNX		16	/* QNX		*/
#define IH_OS_BAREBOX		17	/* Firmware	*/
#define IH_OS_RTEMS		18	/* RTEMS	*/
#define IH_OS_ARTOS		19	/* ARTOS	*/
#define IH_OS_UNITY		20	/* Unity OS	*/

/*
 * CPU Architecture Codes (supported by Linux)
 */
#define IH_ARCH_INVALID		0	/* Invalid CPU	*/
#define IH_ARCH_ALPHA		1	/* Alpha	*/
#define IH_ARCH_ARM		2	/* ARM		*/
#define IH_ARCH_I386		3	/* Intel x86	*/
#define IH_ARCH_IA64		4	/* IA64		*/
#define IH_ARCH_MIPS		5	/* MIPS		*/
#define IH_ARCH_MIPS64		6	/* MIPS	 64 Bit */
#define IH_ARCH_PPC		7	/* PowerPC	*/
#define IH_ARCH_S390		8	/* IBM S390	*/
#define IH_ARCH_SH		9	/* SuperH	*/
#define IH_ARCH_SPARC		10	/* Sparc	*/
#define IH_ARCH_SPARC64		11	/* Sparc 64 Bit */
#define IH_ARCH_M68K		12	/* M68K		*/
#define IH_ARCH_NIOS		13	/* Nios-32	*/
#define IH_ARCH_MICROBLAZE	14	/* MicroBlaze   */
#define IH_ARCH_NIOS2		15	/* Nios-II	*/
#define IH_ARCH_BLACKFIN	16	/* Blackfin	*/
#define IH_ARCH_AVR32		17	/* AVR32	*/
#define IH_ARCH_LINUX		18	/* Linux	*/

#if defined(__PPC__)
#define IH_ARCH IH_ARCH_PPC
#elif defined(__ARM__)
#define IH_ARCH IH_ARCH_ARM
#elif defined(__I386__) || defined(__x86_64__)
#define IH_ARCH IH_ARCH_I386
#elif defined(__mips__)
#define IH_ARCH IH_ARCH_MIPS
#elif defined(__nios__)
#define IH_ARCH IH_ARCH_NIOS
#elif defined(__m68k__)
#define IH_ARCH IH_ARCH_M68K
#elif defined(__microblaze__)
#define IH_ARCH IH_ARCH_MICROBLAZE
#elif defined(__nios2__)
#define IH_ARCH IH_ARCH_NIOS2
#elif defined(__blackfin__)
#define IH_ARCH IH_ARCH_BLACKFIN
#elif defined(__avr32__)
#define IH_ARCH IH_ARCH_AVR32
#elif defined(CONFIG_LINUX)
#define IH_ARCH IH_ARCH_LINUX
#endif

/*
 * Image Types
 *
 * "Standalone Programs" are directly runnable in the environment
 *	provided by barebox; it is expected that (if they behave
 *	well) you can continue to work in barebox after return from
 *	the Standalone Program.
 * "OS Kernel Images" are usually images of some Embedded OS which
 *	will take over control completely. Usually these programs
 *	will install their own set of exception handlers, device
 *	drivers, set up the MMU, etc. - this means, that you cannot
 *	expect to re-enter barebox except by resetting the CPU.
 * "RAMDisk Images" are more or less just data blocks, and their
 *	parameters (address, size) are passed to an OS kernel that is
 *	being started.
 * "Multi-File Images" contain several images, typically an OS
 *	(Linux) kernel image and one or more data images like
 *	RAMDisks. This construct is useful for instance when you want
 *	to boot over the network using BOOTP etc., where the boot
 *	server provides just a single image file, but you want to get
 *	for instance an OS kernel and a RAMDisk image.
 *
 *	"Multi-File Images" start with a list of image sizes, each
 *	image size (in bytes) specified by an "uint32_t" in network
 *	byte order. This list is terminated by an "(uint32_t)0".
 *	Immediately after the terminating 0 follow the images, one by
 *	one, all aligned on "uint32_t" boundaries (size rounded up to
 *	a multiple of 4 bytes - except for the last file).
 *
 * "Firmware Images" are binary images containing firmware (like
 *	barebox or FPGA images) which usually will be programmed to
 *	flash memory.
 *
 * "Script files" are command sequences that will be executed by
 *	barebox's command interpreter; this feature is especially
 *	useful when you configure barebox to use a real shell (hush)
 *	as command interpreter (=> Shell Scripts).
 */

#define IH_TYPE_INVALID		0	/* Invalid Image		*/
#define IH_TYPE_STANDALONE	1	/* Standalone Program		*/
#define IH_TYPE_KERNEL		2	/* OS Kernel Image		*/
#define IH_TYPE_RAMDISK		3	/* RAMDisk Image		*/
#define IH_TYPE_MULTI		4	/* Multi-File Image		*/
#define IH_TYPE_FIRMWARE	5	/* Firmware Image		*/
#define IH_TYPE_SCRIPT		6	/* Script file			*/
#define IH_TYPE_FILESYSTEM	7	/* Filesystem Image (any type)	*/
#define IH_TYPE_FLATDT		8	/* Binary Flat Device Tree Blob	*/

/*
 * Compression Types
 */
#define IH_COMP_NONE		0	/*  No	 Compression Used	*/
#define IH_COMP_GZIP		1	/* gzip	 Compression Used	*/
#define IH_COMP_BZIP2		2	/* bzip2 Compression Used	*/

#define IH_MAGIC	0x27051956	/* Image Magic Number		*/
#define IH_NMLEN		32	/* Image Name Length		*/

/*
 * all data in network byte order (aka natural aka bigendian)
 */

typedef struct image_header {
	uint32_t	ih_magic;	/* Image Header Magic Number	*/
	uint32_t	ih_hcrc;	/* Image Header CRC Checksum	*/
	uint32_t	ih_time;	/* Image Creation Timestamp	*/
	uint32_t	ih_size;	/* Image Data Size		*/
	uint32_t	ih_load;	/* Data	 Load  Address		*/
	uint32_t	ih_ep;		/* Entry Point Address		*/
	uint32_t	ih_dcrc;	/* Image Data CRC Checksum	*/
	uint8_t		ih_os;		/* Operating System		*/
	uint8_t		ih_arch;	/* CPU architecture		*/
	uint8_t		ih_type;	/* Image Type			*/
	uint8_t		ih_comp;	/* Compression Type		*/
	uint8_t		ih_name[IH_NMLEN];	/* Image Name		*/
} image_header_t;

struct image_handle {
	image_header_t	header;
	void *data;
#define IH_MALLOC	1
	int flags;
};

#if defined(CONFIG_CMD_BOOTM_SHOW_TYPE) || !defined(__BAREBOX__)
const char *image_get_os_name(uint8_t os);
const char *image_get_arch_name(uint8_t arch);
const char *image_get_type_name(uint8_t type);
const char *image_get_comp_name(uint8_t comp);
#else
static inline const char *image_get_os_name(uint8_t os)
{
	return NULL;
}

static inline const char *image_get_arch_name(uint8_t arch)
{
	return NULL;
}

static inline const char *image_get_type_name(uint8_t type)
{
	return NULL;
}

static inline const char *image_get_comp_name(uint8_t comp)
{
	return NULL;
}
#endif

#define uimage_to_cpu(x)		be32_to_cpu(x)
#define cpu_to_uimage(x)		cpu_to_be32(x)

static inline uint32_t image_get_header_size(void)
{
	return sizeof(image_header_t);
}

#define image_get_hdr_u32(x)					\
static inline uint32_t image_get_##x(const image_header_t *hdr)	\
{								\
	return uimage_to_cpu(hdr->ih_##x);			\
}

image_get_hdr_u32(magic);	/* image_get_magic */
image_get_hdr_u32(hcrc);	/* image_get_hcrc */
image_get_hdr_u32(time);	/* image_get_time */
image_get_hdr_u32(size);	/* image_get_size */
image_get_hdr_u32(load);	/* image_get_load */
image_get_hdr_u32(ep);		/* image_get_ep */
image_get_hdr_u32(dcrc);	/* image_get_dcrc */

#define image_get_hdr_u8(x)					\
static inline uint8_t image_get_##x(const image_header_t *hdr)	\
{								\
	return hdr->ih_##x;					\
}
image_get_hdr_u8(os);		/* image_get_os */
image_get_hdr_u8(arch);		/* image_get_arch */
image_get_hdr_u8(type);		/* image_get_type */
image_get_hdr_u8(comp);		/* image_get_comp */

static inline char *image_get_name(const image_header_t *hdr)
{
	return (char*)hdr->ih_name;
}

static inline uint32_t image_get_data_size(const image_header_t *hdr)
{
	return image_get_size(hdr);
}

/**
 * image_get_data - get image payload start address
 * @hdr: image header
 *
 * image_get_data() returns address of the image payload. For single
 * component images it is image data start. For multi component
 * images it points to the null terminated table of sub-images sizes.
 *
 * returns:
 *     image payload data start address
 */
static inline ulong image_get_data(const image_header_t *hdr)
{
	return ((ulong)hdr + image_get_header_size());
}

static inline uint32_t image_get_image_size(const image_header_t *hdr)
{
	return (image_get_size(hdr) + image_get_header_size());
}

static inline ulong image_get_image_end(const image_header_t *hdr)
{
	return ((ulong)hdr + image_get_image_size(hdr));
}

#define image_set_hdr_u32(x)						\
static inline void image_set_##x(image_header_t *hdr, uint32_t val)	\
{									\
	hdr->ih_##x = cpu_to_uimage(val);				\
}

image_set_hdr_u32(magic);	/* image_set_magic */
image_set_hdr_u32(hcrc);	/* image_set_hcrc */
image_set_hdr_u32(time);	/* image_set_time */
image_set_hdr_u32(size);	/* image_set_size */
image_set_hdr_u32(load);	/* image_set_load */
image_set_hdr_u32(ep);		/* image_set_ep */
image_set_hdr_u32(dcrc);	/* image_set_dcrc */

#define image_set_hdr_u8(x)						\
static inline void image_set_##x(image_header_t *hdr, uint8_t val)	\
{									\
	hdr->ih_##x = val;						\
}

image_set_hdr_u8(os);		/* image_set_os */
image_set_hdr_u8(arch);		/* image_set_arch */
image_set_hdr_u8(type);		/* image_set_type */
image_set_hdr_u8(comp);		/* image_set_comp */

static inline void image_set_name(image_header_t *hdr, const char *name)
{
	strncpy(image_get_name(hdr), name, IH_NMLEN);
}

ulong image_multi_count(const image_header_t *hdr);
void image_multi_getimg(const image_header_t *hdr, ulong idx,
			ulong *data, ulong *len);

void image_print_size(uint32_t size);

void image_print_contents(const void *ptr);

/* commamds/bootm.c */
void	print_image_hdr (image_header_t *hdr);

/*
 * Load an image into memory. Returns a pointer to the loaded
 * image.
 */
struct image_handle *map_image(const char *filename, int verify,
		int legacy_format, int secure_boot);
void unmap_image(struct image_handle *handle);

/*
 * Relocate an image to load_address by uncompressing
 * or just copying.
 */
int relocate_image(struct image_handle *handle, void *load_address);

#endif	/* __IMAGE_H__ */
