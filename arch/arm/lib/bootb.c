#include <boot.h>
#include <c2k_otp.h>
#include <common.h>
#include <command.h>
#include <init.h>
#include <clock.h>
#include <config.h>
#include <malloc.h>
#include <sha1.h>
#include <rsa_verify.h>
#include <mach/comcerto-2000.h>
#include <mach/gpio.h>
#include <asm/io.h>

#define ULOADER_PART_SIZE 0x20000 /* 128 KB */
#define BAREBOX_PART_SIZE 0x80000
#define BAREBOX_HEADER_SIZE 0x10	/* 16 bytes */
#define BAREBOX_LODING_ADDR \
	(COMCERTO_AXI_DDR_BASE + 0x1000000 - BAREBOX_HEADER_SIZE)

#ifdef CONFIG_COMCERTO_NAND_ULOADER
extern int read_nand(ulong src, ulong offset, ulong count);
#endif

static int bb_timeout(int timeout)
{
	int ret = 1;
	int countdown;
	uint64_t start, second;
	char c;

	start = get_time_ns();
	second = start;

	countdown = timeout;

	printf("Press Ctrl-X to interrupt");
	printf("\n%2d", countdown--);

	do {
		if (tstc()) {
			c = getc();
			/* User can get a barebox prompt by pressing Ctrl-X */
			if (c == 24) {
				ret = 1;
				goto  out;
			}
		}

		if (is_timeout(second, SECOND)) {
			printf("\b\b%2d", countdown--);
			second += SECOND;
		}
	} while (!is_timeout(start, timeout * SECOND));

	ret = 0;
out:
	printf("\n");

	return ret;
}

static void bb_go(void *addr)
{
	int	(*func)(void);

	printf("\n## Starting Barebox at 0x%p ...\n", addr);

	console_flush();

	func = addr;

	shutdown_barebox();
	func();

	/*
	 * The application returned. Since we have shutdown barebox and
	 * we know nothing about the state of the cpu/memory we can't
	 * do anything here.
	 */
	while (1);
}

#if !defined(CONFIG_COMCERTO_NAND_ULOADER) && (defined(CONFIG_SPI_FLASH) || defined(CONFIG_CSI_FLASH))
extern int spi_copy_read(char *, unsigned int , unsigned int , unsigned int );
#endif

#if !defined(CONFIG_COMCERTO_NAND_ULOADER) && (defined(CONFIG_SPI_FLASH) || defined(CONFIG_CSI_FLASH))
static void copy_bb_from_spi_flash(char *dst, int count)
{
        unsigned int l=0;
        unsigned int sec=2;
        unsigned int size=0;

	size = count;

	while (l < (count/SPI_FLASH_SECTOR_SIZE))
        {	
		spi_copy_read((char*)dst, SPI_FLASH_SECTOR_SIZE, sec, 0);

		size -= SPI_FLASH_SECTOR_SIZE;
		dst += SPI_FLASH_SECTOR_SIZE;
		sec += 1;
		l++;
	}

	if(size)
        {
		spi_copy_read((char*)dst, size, sec, 0);
	}

        printf("BB Copying Done\n");

        return;
}
#endif

#ifndef CONFIG_FORCE_BAREBOX_AUTH
/*
 * Determines whether or not the uLoader has been securely booted.
 *
 * If there is any doubt then we go with 'yes', since that restricts
 * what harm we can do (e.g. can't boot unsigned or wrongly signed
 * barebox).
 */
static bool is_secure_boot(void)
{
	bool is_secure;
	u8 *config_byte;

	config_byte = malloc(1);
	if (config_byte == NULL) {
		printf("Warning: config_byte malloc failed; assumed secure boot.\n");
		return true;
	}

	if (otp_read(8, config_byte, 1) != 0) {
		printf("Warning: otp_read failed; assuming secure boot.\n");
		return true;
	}

	/* The auth bit is bit 1 of the config byte */
	is_secure = *config_byte & 0x2;

	free(config_byte);

	return is_secure;
}
#endif

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

static int verify_image(u8 *image_ptr, u32 max_image_len) {
	sha1_context ctx;
	u8 *sig, hash[20];
	u32 image_len, sig_offset;

	image_len = _get_le_32_byte_int(image_ptr);
	if (image_len > max_image_len) {
		printf("ERROR: barebox image verification failed (bad header)\n");
		return -1;
	}
	image_ptr += 4;

	sig_offset = _get_le_32_byte_int(image_ptr);
	image_ptr += 4;

	image_ptr += 8;

	sha1_starts(&ctx);
	sha1_update(&ctx, image_ptr, image_len);
	sha1_finish(&ctx, hash);

	image_ptr += sig_offset;
	sig = image_ptr;

	return rsa_verify(sig, 256, hash);
}

static int do_bootb_barebox(void)
{
	volatile u32 *src = (u32 *)(COMCERTO_AXI_EXP_BASE + ULOADER_PART_SIZE); /* Start of NOR + uLdr size(128K) */
	volatile u32 *dst = (u32*)BAREBOX_LODING_ADDR;
	int count = BAREBOX_PART_SIZE;
	u32 bootopt;
	int timeout = 1;
	bool secure_boot;

#ifdef CONFIG_FORCE_BAREBOX_AUTH
	secure_boot = true;
#else
	secure_boot = is_secure_boot();
#endif

	if(!secure_boot && bb_timeout(timeout))
		return 0;

#ifdef CONFIG_COMCERTO_NAND_ULOADER
	/* this option is used when uloader is in NOR flash or I2C EEPROM and 
	barebox is in NAND */
	printf("\nCopying Bb from NAND\n");
	read_nand(BAREBOX_LODING_ADDR, 0x0, BAREBOX_PART_SIZE);
#else

#if defined(CONFIG_SPI_FLASH) || defined(CONFIG_CSI_FLASH)
	bootopt = ((readl(COMCERTO_GPIO_SYSTEM_CONFIG) >>  BOOTSTRAP_BOOT_OPT_SHIFT) & BOOTSTRAP_BOOT_OPT_MASK);
	
	if(!bootopt){
		//With SPI boot, the barebox will also reside in SPI flash
		printf("\nCopying Barebox from SPI Flash\n");
		copy_bb_from_spi_flash((char*)BAREBOX_LODING_ADDR, count);
	} else {
#endif
		/*
		-With NOR boot the barebox will be loaded from NOR flash.
		-With I2C boot the barebox will be loaded either from NOR flash or NAND. If NAND then this part of code
		won't be compiled as CONFIG_COMCERTO_NAND_ULOADER will be selected.
		-With other boot option like UART boot, barebox will be loaded from NOR by default
		*/
		printf("\nCopying Barebox from NOR Flash\n");
		memcpy((void *)dst, (void *)src, count);
#if defined(CONFIG_SPI_FLASH) || defined(CONFIG_CSI_FLASH)
	}
#endif /* */

#endif

	if (secure_boot) {
		printf("\nSecure boot detected; verifying Barebox image\n");

		if (verify_image((u8 *) BAREBOX_LODING_ADDR, BAREBOX_PART_SIZE) == 0) {
			printf("Barebox image verified!\n");
		} else {
			printf("ERROR: Barebox image verification failed!\n");
			return -1;
		}
	} else {
		printf("Secure boot not enabled; skipping barebox verification.\n");
	}

	bb_go((void*)(BAREBOX_LODING_ADDR + BAREBOX_HEADER_SIZE));

	return -1;
}

late_initcall(do_bootb_barebox);

