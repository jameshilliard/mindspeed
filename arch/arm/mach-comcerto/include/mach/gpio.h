/*
 *  arch/arm/mach-comcerto/include/mach/gpio.h
 *
 *  Copyright (C) 2008 Mindspeed Technologies, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#ifndef __COMCERTO_GPIO_H__
#define __COMCERTO_GPIO_H__

/***** GPIO  *****/
#define COMCERTO_GPIO_OUTPUT_REG			(COMCERTO_APB_GPIO_BASE + 0x00)
#define COMCERTO_GPIO_OE_REG				(COMCERTO_APB_GPIO_BASE + 0x04)
#define COMCERTO_GPIO_INT_CFG_REG			(COMCERTO_APB_GPIO_BASE + 0x08)
#define COMCERTO_GPIO_ARM_UNALIGNED_LOGIC_ENABLE	(COMCERTO_APB_GPIO_BASE + 0x0C)
#define COMCERTO_GPIO_INPUT_REG				(COMCERTO_APB_GPIO_BASE + 0x10)
#define COMCERTO_GPIO_APB_WS				(COMCERTO_APB_GPIO_BASE + 0x14)
#define COMCERTO_GPIO_SYSTEM_CONFIG			(COMCERTO_APB_GPIO_BASE + 0x1C)
#define COMCERTO_GPIO_MBIST				(COMCERTO_APB_GPIO_BASE + 0x20)
#define COMCERTO_GPIO_TDM_MUX				(COMCERTO_APB_GPIO_BASE + 0x28)
#define COMCERTO_GPIO_ARM_ID				(COMCERTO_APB_GPIO_BASE + 0x30)
#define COMCERTO_GPIO_PAD_CTRL				(COMCERTO_APB_GPIO_BASE + 0x34)
#define COMCERTO_GPIO_BOOTSTRAP_STATUS			(COMCERTO_APB_GPIO_BASE + 0x40)
#define COMCERTO_GPIO_BOOTSTRAP_OVERRIDE		(COMCERTO_APB_GPIO_BASE + 0x44)
#define COMCERTO_GPIO_USB_PHY_BIST_STATUS_REG		(COMCERTO_APB_GPIO_BASE + 0x48)
#define COMCERTO_GPIO_GENERAL_CONTROL_REG		(COMCERTO_APB_GPIO_BASE + 0x4C)
#define COMCERTO_GPIO_DEVICE_ID_REG			(COMCERTO_APB_GPIO_BASE + 0x50)
#define COMCERTO_GPIO_ARM_MEMORY_SENSE_AMP		(COMCERTO_APB_GPIO_BASE + 0x54)
#define COMCERTO_GPIO_PIN_SELECT_REG			(COMCERTO_APB_GPIO_BASE + 0x58)
#define COMCERTO_GPIO_MISC_PIN_SELECT_REG		(COMCERTO_APB_GPIO_BASE + 0x60)
#define COMCERTO_GPIO_PAD_CONFIG0                       (COMCERTO_APB_GPIO_BASE + 0x100)
#define COMCERTO_GPIO_PAD_CONFIG3                       (COMCERTO_APB_GPIO_BASE + 0x10C)
#define COMCERTO_GPIO_PAD_CONFIG4                       (COMCERTO_APB_GPIO_BASE + 0x110)
#define COMCERTO_GPIO_PAD_CONFIG5                       (COMCERTO_APB_GPIO_BASE + 0x114)
#define COMCERTO_GPIO_MEM_EMA_CONFIG0                       (COMCERTO_APB_GPIO_BASE + 0x1A0)
#define COMCERTO_GPIO_MEM_EMA_CONFIG1                       (COMCERTO_APB_GPIO_BASE + 0x1A4)

#define GPIO_0		0x00000001
#define GPIO_1		0x00000002
#define GPIO_2		0x00000004
#define GPIO_3		0x00000008
#define GPIO_4		0x00000010
#define GPIO_5		0x00000020
#define GPIO_6	        0x00000040
#define GPIO_7		0x00000080
#define GPIO_8		0x00000100
#define GPIO_9		0x00000200
#define GPIO_10		0x00000400
#define GPIO_11		0x00000800
#define GPIO_12		0x00001000
#define GPIO_13		0x00002000
#define GPIO_14		0x00004000
#define GPIO_15		0x00008000
#define GPIO_16		0x00010000
#define GPIO_17		0x00020000
#define GPIO_18		0x00040000
#define GPIO_19		0x00080000
#define GPIO_20		0x00100000
#define GPIO_21		0x00200000
#define GPIO_22		0x00400000
#define GPIO_23		0x00800000
#define GPIO_24		0x01000000
#define GPIO_25		0x02000000
#define GPIO_26		0x04000000
#define GPIO_27		0x08000000
#define GPIO_28		0x10000000
#define GPIO_29		0x20000000
#define GPIO_30		0x40000000
#define GPIO_31		0x80000000



/* Bootstrap configuration bit definitions */
#define BOOTSTRAP_BOOT_OPT_MASK           0x7
#define BOOTSTRAP_BOOT_OPT_SHIFT          0

#define BOOTSTRAP_SERDES2_CNF_SGMII	(1 << 10)
#define BOOTSTRAP_SERDES1_CNF_SATA0	(1 << 11)
#define BOOTSTRAP_SERDES2_CNF_SATA1	(1 << 12)


/* GPIO Pin Select Pins */
#define EXP_NAND_RDY	GPIO_29
#define EXP_NAND_CS	GPIO_28

#define DISABLE_FABRIC_REMAP 0x10

#endif
