/*
 * (C) Copyright 2010 Juergen Beisert - Pengutronix
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef __ASM_ARCH_MX23_REGS_H
#define __ASM_ARCH_MX23_REGS_H

/*
 * sanity check
 */
#ifndef _IMX_REGS_H
# error "Please do not include directly. Use imx-regs.h instead."
#endif

#define IMX_MEMORY_BASE		0x40000000
#define IMX_UART1_BASE		0x8006c000
#define IMX_UART2_BASE		0x8006e000
#define IMX_DBGUART_BASE	0x80070000
#define IMX_TIM1_BASE		0x80068000
#define IMX_IOMUXC_BASE		0x80018000
#define IMX_WDT_BASE		0x8005c000
#define IMX_CCM_BASE		0x80040000
#define IMX_I2C1_BASE		0x80058000
#define IMX_SSP1_BASE		0x80010000
#define IMX_FB_BASE		0x80030000
#define IMX_SSP2_BASE		0x80034000
#define IMX_POWER_BASE		0x80044000
#define IMX_USBPHY_BASE		0x8007c000
#define IMX_DIGCTL_BASE		0x8001c000
#define IMX_USB_BASE		0x80080000

#endif /* __ASM_ARCH_MX23_REGS_H */
