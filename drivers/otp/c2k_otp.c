#include <common.h>
#include <driver.h>
#include <init.h>
#include <asm/io.h>
#include <mach/hardware.h>
#include <mach/otp.h>
#include <mach/clkcore.h>
#include <clock.h>

#define NP1	4
#define NP2	16

extern u32 HAL_get_axi_clk(void);

void write_protect_unlock(void)
{
	writel(0xEBCF0000, OTP_CONFIG_LOCK_0);  /* config lock0 */
	writel(0xEBCF1111, OTP_CONFIG_LOCK_1); /* config lock1 */
	writel(0x0, OTP_CEB_INPUT);
}

void otp_smart_write_sequence(u32 offset, u8 prog_data)
{
	uint64_t start;

	/* Drive the address now */
	writel(offset, OTP_ADDR_INPUT);
	/* Write data to the DATA_IN register */
	writel(prog_data, OTP_DATA_INPUT);

	/* DLE drive  "1" */
	writel(0x1, OTP_DLE_INPUT);
	/* Wait for at least 20nsec */
	ndelay(20);

	/* WEB drive  "0" */
	writel(0x0, OTP_WEB_INPUT);
	/* Wait for at least 20nsec */
	ndelay(20);

	/* WEB drive  "1" */
	writel(0x1, OTP_WEB_INPUT);
	/* Wait for at least 20nsec */
	ndelay(20);

	/* DLE drive  "0" */
	writel(0x0, OTP_DLE_INPUT);

	/* Write '1' to PGMEN to trigger the whole write and verify operation until
	 * PGMEN will be deasserted by HW */
	writel(0x1, OTP_PGMEN_INPUT);

	/* Wait for PGMEN to go low for 11.2 u sec */
	start = get_time_ns();
	while (readl(OTP_PGMEN_INPUT) & 1 &&
			!is_timeout(start, 1*MSECOND));
	if (readl(OTP_PGMEN_INPUT) & 1) {
		printf("Timeout waiting for PGMEN "
				"to be deasserted\n");
	}
}

/*
 * Writes to the OTP.
 *
 * Take care: although this method takes a u8 array for the
 * data being written, each element represents only one bit
 * in the OTP. If set to 1, the OTP is flashed at that bit.
 * If set to 0, it is not. It is undefined what happens if it
 * is set to a value other than 0 or 1.
 *
 * @offset:    The offset to write to, in bits.
 * @prog_data: A *bit* array to write data from.
 * @size:      The number of *bits* to write.
 */
int otp_write(u32 offset, u8 *prog_data, int size)
{
	int i, k;
	u32 pgm2cpump_counter, cpump2web_counter, web_counter, web2cpump_counter;
	u32 cpump2pgm_counter, dataout_counter;
	u32 read_data;
	u32 axi_clk = HAL_get_axi_clk();

	if (NULL == prog_data)
		return RETCODE_ERROR;

	if (size <= 0)
		return RETCODE_ERROR;

	/* Setting up counters to program */
	pgm2cpump_counter = axi_clk & 0x7FF ; 				/* 1 uSec */
	cpump2web_counter = (axi_clk*3) & 0x7FF ;			/* 3 uSec */
	web_counter = (axi_clk*5) & 0x7FF ;					/* 5 uSec */
	web2cpump_counter = (axi_clk*2) & 0x7FF ;			/* 2 uSec */
	cpump2pgm_counter = axi_clk & 0x7FF ;				/* 1 uSec */
	dataout_counter = ((axi_clk * 7 + 99) / 100) & 0x1FF ;	/* 70 nSec */

	/* program the counters */
	writel(pgm2cpump_counter, OTP_PGM2CPUMP_COUNTER);
	writel(cpump2web_counter, OTP_CPUMP2WEB_COUNTER);
	writel(web_counter, OTP_WEB_COUNTER);
	writel(web2cpump_counter, OTP_WEB2CPUMP_COUNTER);
	writel(cpump2pgm_counter, OTP_CPUMP2PGM_COUNTER);
	writel(dataout_counter, OTP_DATA_OUT_COUNTER);

	write_protect_unlock();

	udelay(1);

	/* rstb drive 0 */
	writel(0x0, OTP_RSTB_INPUT);
	/* Wait for at least 20nsec */
	ndelay(20);

	/* rstb drive 1 to have pulse  */
	writel(0x1, OTP_RSTB_INPUT);
	/* Wait for at least 1usec */
	udelay(1);

	for(i = 0 ; i < size ; i++) {

		if (!prog_data[i]) {
			/* Skip bits that are 0 because 0 is the default value.
			 * */
			continue;
		}

		for(k = 0 ; k < NP1-1 ; k++)
			otp_smart_write_sequence(offset + i, prog_data[i]);

		for(k = 0 ; k < NP2+1 ; k++) {
			ndelay(100);
			otp_smart_write_sequence(offset + i, prog_data[i]);

			/* Verify Data */
			read_data = readl(OTP_DATA_OUTPUT);

			/* Adjust bit offset */
			read_data = ((read_data >> ((offset+i) & 0x7)) & 0x1);

			if(read_data == prog_data[i])
				break;
		}
		if(read_data != prog_data[i]) {
			printf("Warning : failed to write OTP value at bit %d (%d attempts) !\n",
					offset + i, NP1 + NP2);
			/* Not returrning Error here for debug */
		}
	}

	return 0;
}

/*
 * Reads from the OTP.
 *
 * @offset:    The offset to read from, in bits.
 * @read_data: The buffer to read data into.
 * @size:      The number of *bytes* to read.
 *
 * @return     0 if successful, non-zero otherwise
 */
int otp_read(u32 offset, u8 *read_data, int size)
{
	int i;
	u32 read_tmp = 0, dataout_counter;
	u32 axi_clk = HAL_get_axi_clk();

	if (NULL == read_data)
		return RETCODE_ERROR;

	if (size <= 0)
		return RETCODE_ERROR;

	dataout_counter = ((axi_clk * 7 + 99) / 100) & 0x1FF ;	/* 70 nSec */

	/* configure the OTP_DATA_OUT_COUNTER for read operation.
	    70 nsec is needed except for blank check test, in which 1.5 usec is needed.*/
	writel(dataout_counter, OTP_DATA_OUT_COUNTER);

	write_protect_unlock();
	udelay(1);

	/* rstb drive 0 */
	writel(0x0, OTP_RSTB_INPUT);
	/* Wait for at least 20nsec */
	ndelay(20);
	/* rstb drive 1 to have pulse  */
	writel(0x1, OTP_RSTB_INPUT);
	/* Wait for at least 1usec */
	udelay(1);

	/* Write the desired address to the ADDR register */
	writel(offset, OTP_ADDR_INPUT);
	/* read_enable drive */
	writel(0x1, OTP_READEN_INPUT);
	/* Wait for at least 70nsec/1.5usec depends on operation type */
	ndelay(70);

	/* Read First Byte */
	read_tmp = readl(OTP_DATA_OUTPUT);
	*read_data = read_tmp & 0xFF;

	/* For consecutive read */
	for(i = 1 ; i < size ; i++)
	{
		offset = offset + 8;

		/* start reading from data out register */
		writel(offset, OTP_ADDR_INPUT);
		/* Wait for at least 70nsec/1.5usec depends on operation type */
		ndelay(70);

		read_tmp = readl(OTP_DATA_OUTPUT);
		*(read_data + i) = read_tmp & 0xFF;
	}

	/* reading is done make the read_enable low */
	writel(0x0, OTP_READEN_INPUT);

	/* lock CEB register, return to standby mode */
	writel(0x1, OTP_CEB_INPUT);

	return RETCODE_OK;
}
EXPORT_SYMBOL(otp_read);

static int c2k_otp_probe(struct device_d *pdev)
{
	printf("c2k_otp_probe.\n");
	return 0;
}

struct driver_d c2k_otp_driver = {
        .name    = "c2k_otp",
	.probe	 = c2k_otp_probe,
};

int c2k_otp_init(void)
{
	return register_driver(&c2k_otp_driver);
}

device_initcall(c2k_otp_init);
