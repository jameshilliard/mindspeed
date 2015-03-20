#include <boot.h>
#include <c2k_otp.h>
#include <common.h>
#include <command.h>
#include <init.h>
#include <clock.h>
#include <config.h>
#include <malloc.h>
#include <secure_boot.h>
#include <sha1.h>
#include <rsa_public_key.h>
#include <rsa_verify.h>
#include <xyzModem.h>
#include <mach/comcerto-2000.h>
#include <mach/gpio.h>
#include <asm/io.h>
#include <board_id.h>

#define ULOADER_PART_SIZE 0x20000 /* 128 KB */
#define BAREBOX_PART_SIZE 0x80000
#define BAREBOX_HEADER_SIZE 0x10	/* 16 bytes */
#define BAREBOX_LODING_ADDR \
	(COMCERTO_AXI_DDR_BASE + 0x1000000 - BAREBOX_HEADER_SIZE)

#ifdef CONFIG_COMCERTO_NAND_ULOADER
extern int read_nand(ulong src, ulong offset, ulong count);
#endif

#if 0 
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
#endif

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

#ifdef CONFIG_COMCERTO_ULOADER_UART_DOWNLOAD
static int getcxmodem(void)
{
	if (tstc())
		return (getc());
	return -1;
}

static int load_serial_ymodem(void *dst)
{
	int err;
	int res;
	connection_info_t info;
	void *addr = dst;

	info.mode = xyzModem_ymodem;
	res = xyzModem_stream_open(&info, &err);
	if (res) {
		printf("%s\n", xyzModem_error(err));
		return 1;
	}
	err = 0;
	res = xyzModem_stream_read(addr, 16*1024*1024, &err);
	xyzModem_stream_close(&err);
	xyzModem_stream_terminate(false, &getcxmodem);
	if (err) {
		printf("%s\n", xyzModem_error(err));
		return 1;
	}

	printf("## Total Size = %d Bytes\n", res);

	return 0;
}
#else /* CONFIG_COMCERTO_ULOADER_UART_DOWNLOAD */
static int load_serial_ymodem(void *dst)
{
	printf("YMODEM download not supported\n");
	return -1;
}
#endif /* CONFIG_COMCERTO_ULOADER_UART_DOWNLOAD */

static int _verify_image(u8 *image_ptr, u32 max_image_len) {
	sha1_context ctx;
	const struct rsa_public_key *public_key = NULL;
	u8 *sig, hash[20];
	u32 image_len, sig_offset;

	image_len = _get_le_uint32(image_ptr);
	if (image_len > max_image_len) {
		printf("ERROR: barebox image verification failed (bad header)\n");
		return -1;
	}
	image_ptr += 4;

	sig_offset = _get_le_uint32(image_ptr);
	image_ptr += 4;

	image_ptr += 8;

	sha1_starts(&ctx);
	sha1_update(&ctx, image_ptr, image_len);
	sha1_finish(&ctx, hash);

	image_ptr += sig_offset;
	sig = image_ptr;

	if (rsa_get_public_key(OPTIMUS_BOARD_ID, &public_key) != 0) {
		printf("ERROR: could not verify barebox image (no public key)\n");
		return -1;
	}

	return rsa_verify(public_key, sig, 256, hash);
}

static int verify_image(u8 *image_ptr, u32 max_image_len) {
#ifdef CONFIG_FORCE_BAREBOX_AUTH
	bool secure_boot = true;
#else
	bool secure_boot;
	secure_boot_mode_t boot_mode = get_secure_boot_mode();

	if (boot_mode == UNKNOWN) {
		printf("Error: Unable to determine secure boot mode.\n");
		return -1;
	}
	secure_boot = (boot_mode == SECURE);
#endif

	if (secure_boot) {
		printf("\nSecure boot detected; verifying Barebox image\n");

		if (_verify_image(image_ptr, max_image_len) == 0) {
			printf("Barebox image verified!\n");
		} else {
			printf("ERROR: Barebox image verification failed!\n");
			return -1;
		}
	} else {
		printf("Secure boot not enabled; skipping barebox verification.\n");
	}
	return 0;
}

static int do_bootb_barebox(void)
{
	void *src = (void *) (COMCERTO_AXI_EXP_BASE + ULOADER_PART_SIZE); /* Start of NOR + uLdr size(128K) */
	void *dst = (void *) BAREBOX_LODING_ADDR;
	int count = BAREBOX_PART_SIZE;
	int i;
	u32 bootopt;
#if 0
	int timeout = 1;
	if(!secure_boot && bb_timeout(timeout))
		return 0;
#endif

#ifdef CONFIG_COMCERTO_NAND_ULOADER
	/* this option is used when uloader is in NOR flash or I2C EEPROM and 
	barebox is in NAND */
	printf("\nCopying Bb from NAND\n");
	read_nand(BAREBOX_LODING_ADDR, 0x0, BAREBOX_PART_SIZE);
#else

	bootopt = ((readl(COMCERTO_GPIO_SYSTEM_CONFIG) >>  BOOTSTRAP_BOOT_OPT_SHIFT) & BOOTSTRAP_BOOT_OPT_MASK);
	
	switch(bootopt){
		case BOOT_UART:
#define MAX_UART_DOWNLOAD_ATTEMPTS 3
			for (i=0; i < MAX_UART_DOWNLOAD_ATTEMPTS; i++) {
				printf("\nReady to receive Barebox over UART (bootopt=%d)\n", \
						bootopt);
				if (load_serial_ymodem((void *) dst)) continue;
				if (verify_image(dst, count) == 0)
					break;
			}
			if (i == MAX_UART_DOWNLOAD_ATTEMPTS) return -1;
			break;
		default:
			/*
			   -With NOR boot the barebox will be loaded from NOR flash.
			   -With I2C boot the barebox will be loaded either from NOR flash or NAND. 
			   If NAND then this part of code won't be compiled as CONFIG_COMCERTO_NAND_ULOADER 
			   will be selected.
			   -With other boot option like UART boot, barebox will be loaded from NOR by default
			 */
			printf("\nCopying Barebox from NOR Flash(bootopt=%d)\n", \
					bootopt);
			memcpy((void *)dst, (void *)src, count);
			if (verify_image(dst, count) == 0) break;

			printf("\nTrying secondary partition\n");
			src += count;
			memcpy((void *)dst, (void *)src, count);
			if (verify_image(dst, count) == 0) break;
			return -1;
	}

#endif

	bb_go((void*)(BAREBOX_LODING_ADDR + BAREBOX_HEADER_SIZE));

	return -1;
}

late_initcall(do_bootb_barebox);
