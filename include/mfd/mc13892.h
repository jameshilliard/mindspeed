/*
 * Copyright (C) 2009 Marc Kleine-Budde <mkl@pengutronix.de>
 *
 * This file is released under the GPLv2
 *
 * Derived from:
 * - arch-mxc/pmic_external.h --  contains interface of the PMIC protocol driver
 *   Copyright 2008-2009 Freescale Semiconductor, Inc. All Rights Reserved.
 *
 */

#ifndef __ASM_ARCH_MC13892_H
#define __ASM_ARCH_MC13892_H

enum mc13892_reg {
	MC13892_REG_INT_STATUS0		= 0x00,
	MC13892_REG_INT_MASK0		= 0x01,
	MC13892_REG_INT_SENSE0		= 0x02,
	MC13892_REG_INT_STATUS1		= 0x03,
	MC13892_REG_INT_MASK1		= 0x04,
	MC13892_REG_INT_SENSE1		= 0x05,
	MC13892_REG_PU_MODE_S		= 0x06,
	MC13892_REG_IDENTIFICATION	= 0x07,
	MC13892_REG_UNUSED0		= 0x08,
	MC13892_REG_ACC0		= 0x09,
	MC13892_REG_ACC1		= 0x0a,
	MC13892_REG_UNUSED1		= 0x0b,
	MC13892_REG_UNUSED2		= 0x0c,
	MC13892_REG_POWER_CTL0		= 0x0d,
	MC13892_REG_POWER_CTL1		= 0x0e,
	MC13892_REG_POWER_CTL2		= 0x0f,
	MC13892_REG_REGEN_ASSIGN	= 0x10,
	MC13892_REG_UNUSED3		= 0x11,
	MC13892_REG_MEM_A		= 0x12,
	MC13892_REG_MEM_B		= 0x13,
	MC13892_REG_RTC_TIME		= 0x14,
	MC13892_REG_RTC_ALARM		= 0x15,
	MC13892_REG_RTC_DAY		= 0x16,
	MC13892_REG_RTC_DAY_ALARM	= 0x17,
	MC13892_REG_SW_0		= 0x18,
	MC13892_REG_SW_1		= 0x19,
	MC13892_REG_SW_2		= 0x1a,
	MC13892_REG_SW_3		= 0x1b,
	MC13892_REG_SW_4		= 0x1c,
	MC13892_REG_SW_5		= 0x1d,
	MC13892_REG_SETTING_0		= 0x1e,
	MC13892_REG_SETTING_1		= 0x1f,
	MC13892_REG_MODE_0		= 0x20,
	MC13892_REG_MODE_1		= 0x21,
	MC13892_REG_POWER_MISC		= 0x22,
	MC13892_REG_UNUSED4		= 0x23,
	MC13892_REG_UNUSED5		= 0x24,
	MC13892_REG_UNUSED6		= 0x25,
	MC13892_REG_UNUSED7		= 0x26,
	MC13892_REG_UNUSED8		= 0x27,
	MC13892_REG_UNUSED9		= 0x28,
	MC13892_REG_UNUSED10		= 0x29,
	MC13892_REG_UNUSED11		= 0x2a,
	MC13892_REG_ADC0		= 0x2b,
	MC13892_REG_ADC1		= 0x2c,
	MC13892_REG_ADC2		= 0x2d,
	MC13892_REG_ADC3		= 0x2e,
	MC13892_REG_ADC4		= 0x2f,
	MC13892_REG_CHARGE		= 0x30,
	MC13892_REG_USB0		= 0x31,
	MC13892_REG_USB1		= 0x32,
	MC13892_REG_LED_CTL0		= 0x33,
	MC13892_REG_LED_CTL1		= 0x34,
	MC13892_REG_LED_CTL2		= 0x35,
	MC13892_REG_LED_CTL3		= 0x36,
	MC13892_REG_UNUSED12		= 0x37,
	MC13892_REG_UNUSED13		= 0x38,
	MC13892_REG_TRIM0		= 0x39,
	MC13892_REG_TRIM1		= 0x3a,
	MC13892_REG_TEST0		= 0x3b,
	MC13892_REG_TEST1		= 0x3c,
	MC13892_REG_TEST2		= 0x3d,
	MC13892_REG_TEST3		= 0x3e,
	MC13892_REG_TEST4		= 0x3f,
};

enum mc13892_revision {
	MC13892_REVISION_1_0,
	MC13892_REVISION_1_1,
	MC13892_REVISION_1_2,
	MC13892_REVISION_2_0,
	MC13892_REVISION_2_0a,
	MC13892_REVISION_2_1,
	MC13892_REVISION_3_0,
	MC13892_REVISION_3_1,
	MC13892_REVISION_3_2,
	MC13892_REVISION_3_2a,
	MC13892_REVISION_3_3,
	MC13892_REVISION_3_5,
};

enum mc13892_mode {
	MC13892_MODE_I2C,
	MC13892_MODE_SPI,
};

struct mc13892 {
	struct cdev		cdev;
	struct i2c_client	*client;
	struct spi_device	*spi;
	enum mc13892_mode	mode;
	enum mc13892_revision	revision;
};

extern struct mc13892 *mc13892_get(void);

extern int mc13892_reg_read(struct mc13892 *mc13892, enum mc13892_reg reg, u32 *val);
extern int mc13892_reg_write(struct mc13892 *mc13892, enum mc13892_reg reg, u32 val);
extern int mc13892_set_bits(struct mc13892 *mc13892, enum mc13892_reg reg, u32 mask, u32 val);

static inline enum mc13892_revision mc13892_get_revision(struct mc13892 *mc13892)
{
	return mc13892->revision;
}

#endif /* __ASM_ARCH_MC13892_H */
