
/*
 * linux/arch/arm/mach-comcerto/pcie-comcerto2000.h
 *
 * Copyright (C) 2004,2005 Mindspeed Technologies, Inc.
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


#ifndef __PCIE_C2000__
#define __PCIE_C2000__

#define NUM_PCIE_PORTS	2

struct pcie_port {
	int 			root_bus_nr;
	unsigned long		base;
	unsigned long		remote_mem_baseaddr; // COMCERTO_AXI_PCIe#_SLV_BASE 0xA/B000_0000
	unsigned long		app_base;
	unsigned long		cfg0_base; 	/* Remote Configration Type-0 Entry: 0xaff1_0000 */ 
	unsigned long		cfg1_base;
	int 			port_mode;
	int 			link_state;
	struct pcie_app_reg	*app_regs;
/*** ouy+ ***/
	unsigned long		dbi_base;

        int                     msi_base;
        dma_addr_t              msi_mbox_handle;
        void                    *msi_mbox_baseaddr;

        int                     intx_base;
        int                     irq;

        struct clk              *ref_clock;
};

#define PCIE_PORT_MODE_NONE 	-1
#define PCIE_PORT_MODE_EP 	CFG0_DEV_TYPE_EP
#define PCIE_PORT_MODE_RC 	CFG0_DEV_TYPE_RC



/* The following register definitions are as per "DWC_regs_rev04.doc" document */

struct pcie_app_reg {
	u32	cfg0;
	u32	cfg1;
	u32	cfg2;
	u32	cfg3;
	u32	cfg4;   // diag_ctrl_bus b[15:13]: request downstream port to start Link-Hot-Reset
			// 1xx: Force Fast Link Mode, x01: InvertLsb of LCRC, x10: InvertLsb of ECRC
	u32	cfg5;
	u32	cfg6;
	u32	sts0;
	u32	sts1;
	u32	sts2;
	u32	sts3;
	u32	pwr_cfg_bdgt_data;
	u32	pwr_cfg_bdgt_fn;
	u32	radm_sts;
	u32	pwr_sts_bdgt;
	u32	intr_sts;
	u32	intr_en;
	u32	intr_msi_sts;
	u32	intr_msi_en;
};

/* Serdes initialization values for PCIe */
static struct serdes_regs_s pcie_phy_reg_file_48[] = /*** pcie_phy_reg_file_48[] ***/
	{
          /* Common CMU block */
          { 0x000 << 2, 0x06},
          { 0x001 << 2, 0x00},
          { 0x002 << 2, 0x89}, //
          { 0x003 << 2, 0x00},
          { 0x004 << 2, 0x60}, //
          { 0x005 << 2, 0x09}, //
          { 0x006 << 2, 0x0e}, //
          { 0x007 << 2, 0x00},
          { 0x008 << 2, 0x00},
          { 0x009 << 2, 0x00},
          { 0x00A << 2, 0x00},
          { 0x00B << 2, 0x00},
          { 0x00C << 2, 0x00},
          { 0x00D << 2, 0x00},
          { 0x00E << 2, 0x00},
          { 0x00F << 2, 0x00},
          { 0x010 << 2, 0x00},
          { 0x011 << 2, 0x00},
          { 0x012 << 2, 0x00},
          { 0x013 << 2, 0x00},
          { 0x014 << 2, 0x00},
          { 0x015 << 2, 0x00},
          { 0x016 << 2, 0x00},
          { 0x017 << 2, 0x00},
          { 0x018 << 2, 0x00},
          { 0x019 << 2, 0x00},
          { 0x01A << 2, 0x00},
          { 0x01B << 2, 0x00},
          { 0x01C << 2, 0x00},
          { 0x01D << 2, 0x00},
          { 0x01E << 2, 0x00},
          { 0x01F << 2, 0x00},
          { 0x020 << 2, 0x00},
          { 0x021 << 2, 0x00},
          { 0x022 << 2, 0xa0},
          { 0x023 << 2, 0x6c}, //
          { 0x024 << 2, 0x00},
          { 0x025 << 2, 0x00},
          { 0x026 << 2, 0x00},
          { 0x027 << 2, 0x00},
          { 0x028 << 2, 0x00},
          { 0x029 << 2, 0x00},
          { 0x02A << 2, 0x00},
          { 0x02B << 2, 0x00},
          { 0x02C << 2, 0x00},
          { 0x02D << 2, 0x00},
          { 0x02E << 2, 0x04},
          { 0x02F << 2, 0x50},
          { 0x030 << 2, 0x70},
          { 0x031 << 2, 0x02},
          { 0x032 << 2, 0x25},
          { 0x033 << 2, 0x40},
          { 0x034 << 2, 0x01},
          { 0x035 << 2, 0x40},
          { 0x036 << 2, 0x00},
          { 0x037 << 2, 0x00},
          { 0x038 << 2, 0x00},
          { 0x039 << 2, 0x00},
          { 0x03A << 2, 0x00},
          { 0x03B << 2, 0x00},
          { 0x03C << 2, 0x00},
          { 0x03D << 2, 0x00},
          { 0x03E << 2, 0x00},
          { 0x03F << 2, 0x00},
          { 0x040 << 2, 0x00},
          { 0x041 << 2, 0x00},
          { 0x042 << 2, 0x00},
          { 0x043 << 2, 0x00},
          { 0x044 << 2, 0x00},
          { 0x045 << 2, 0x00},
          { 0x046 << 2, 0x00},
          { 0x047 << 2, 0x00},
          { 0x048 << 2, 0x00},
          { 0x049 << 2, 0x00},
          { 0x04A << 2, 0x00},
          { 0x04B << 2, 0x00},
          { 0x04C << 2, 0x00},
          { 0x04D << 2, 0x00},
          { 0x04E << 2, 0x00},
          { 0x04F << 2, 0x00},
          { 0x050 << 2, 0x00},
          { 0x051 << 2, 0x00},
          { 0x052 << 2, 0x00},
          { 0x053 << 2, 0x00},
          { 0x054 << 2, 0x00},
          { 0x055 << 2, 0x00},
          { 0x056 << 2, 0x00},
          { 0x057 << 2, 0x00},
          { 0x058 << 2, 0x00},
          { 0x059 << 2, 0x00},
          { 0x05A << 2, 0x00},
          { 0x05B << 2, 0x00},
          { 0x05C << 2, 0x00},
          { 0x05D << 2, 0x00},
          { 0x05E << 2, 0x00},
          { 0x05F << 2, 0x00},
          { 0x060 << 2, 0x00},
          { 0x061 << 2, 0x2e},
          { 0x062 << 2, 0x08}, //
          { 0x063 << 2, 0x5e},
          { 0x064 << 2, 0x00},
          { 0x065 << 2, 0x42},
          { 0x066 << 2, 0xd1},
          { 0x067 << 2, 0x90}, //
          { 0x068 << 2, 0x08},
          { 0x069 << 2, 0x90}, //
          { 0x06A << 2, 0x2c}, //
          { 0x06B << 2, 0x32}, //
          { 0x06C << 2, 0x59}, //
          { 0x06D << 2, 0x03}, //
          { 0x06E << 2, 0x00},
          { 0x06F << 2, 0x00},
          { 0x070 << 2, 0x00},
          { 0x071 << 2, 0x00},
          { 0x072 << 2, 0x00},
          /* Lane0 Block */
          { 0x200 << 2, 0x00},
          { 0x201 << 2, 0x00},
          { 0x202 << 2, 0x00},
          { 0x203 << 2, 0x00},
          { 0x204 << 2, 0x00},
          { 0x205 << 2, 0x10},
          { 0x206 << 2, 0x84},
          { 0x207 << 2, 0x04},
          { 0x208 << 2, 0xe0},
          { 0x210 << 2, 0x23},
          { 0x211 << 2, 0x00},
          { 0x212 << 2, 0x00},
          { 0x213 << 2, 0x04}, //
          { 0x214 << 2, 0xc0}, //
          { 0x215 << 2, 0x18}, //
          { 0x216 << 2, 0x00},
          { 0x217 << 2, 0x68},
          { 0x218 << 2, 0xa2},
          { 0x219 << 2, 0x1e},
          { 0x21A << 2, 0x18},
          { 0x21B << 2, 0x0d},
          { 0x21C << 2, 0x0d},
          { 0x21D << 2, 0x00},
          { 0x21E << 2, 0x00},
          { 0x21F << 2, 0x00},
          { 0x220 << 2, 0x00},
          { 0x221 << 2, 0x00},
          { 0x222 << 2, 0x00},
          { 0x223 << 2, 0x00},
          { 0x224 << 2, 0x00},
          { 0x225 << 2, 0x00},
          { 0x226 << 2, 0x00},
          { 0x227 << 2, 0x00},
          { 0x228 << 2, 0x00},
          { 0x229 << 2, 0x00},
          { 0x22A << 2, 0x00},
          { 0x22B << 2, 0x00},
          { 0x22C << 2, 0x00},
          { 0x22D << 2, 0x00},
          { 0x22E << 2, 0x00},
          { 0x22F << 2, 0x00},
          { 0x230 << 2, 0x00},
          { 0x231 << 2, 0x00},
          { 0x232 << 2, 0x00},
          { 0x233 << 2, 0x00},
          { 0x234 << 2, 0x00},
          { 0x235 << 2, 0x00},
          { 0x236 << 2, 0x00},
          { 0x237 << 2, 0x00},
          { 0x238 << 2, 0x00},
          { 0x239 << 2, 0x00},
          { 0x23A << 2, 0x00},
          { 0x23B << 2, 0x00},
          { 0x23C << 2, 0x00},
          { 0x23D << 2, 0x00},
          { 0x23E << 2, 0x00},
          { 0x23F << 2, 0x00},
          { 0x240 << 2, 0x00},
          { 0x241 << 2, 0x00},
          { 0x242 << 2, 0x00},
          { 0x243 << 2, 0x00},
          { 0x244 << 2, 0x00},
          { 0x245 << 2, 0x00},
          { 0x246 << 2, 0x00},
          { 0x247 << 2, 0x00},
          { 0x248 << 2, 0x00},
          { 0x249 << 2, 0x00},
          { 0x24A << 2, 0x00},
          { 0x24B << 2, 0x00},
          { 0x24C << 2, 0x00},
          { 0x24D << 2, 0x00},
          { 0x24E << 2, 0x00},
          { 0x24F << 2, 0x00},
          { 0x250 << 2, 0x60},
          { 0x251 << 2, 0x0f},
          /* Common Lane Block */
          { 0xA00 << 2, 0xc0},
          { 0xA01 << 2, 0x90},
          { 0xA02 << 2, 0x02},
          { 0xA03 << 2, 0x40},
          { 0xA04 << 2, 0x3c},
          { 0xA05 << 2, 0x00},
          { 0xA06 << 2, 0x00},
          { 0xA07 << 2, 0x00},
          { 0xA08 << 2, 0x00},
          { 0xA09 << 2, 0xc3}, //
          { 0xA0A << 2, 0xca}, //
          { 0xA0B << 2, 0xc6},
          { 0xA0C << 2, 0x01},
          { 0xA0D << 2, 0x03},
          { 0xA0E << 2, 0x28},
          { 0xA0F << 2, 0x98},
          { 0xA10 << 2, 0x19},
          { 0xA11 << 2, 0x28},
          { 0xA12 << 2, 0x78},
          { 0xA13 << 2, 0xe1},
          { 0xA14 << 2, 0xf0},
          { 0xA15 << 2, 0x10},
          { 0xA16 << 2, 0xf4},
          { 0xA17 << 2, 0x00},
          { 0xA30 << 2, 0x00},
          { 0xA31 << 2, 0x00},
          { 0xA32 << 2, 0x00},
          { 0xA33 << 2, 0x00},
          { 0xA34 << 2, 0x00},
          { 0xA35 << 2, 0x00},
          { 0xA36 << 2, 0x00},
          { 0xA37 << 2, 0x00},
          { 0xA38 << 2, 0x00},
          { 0xA39 << 2, 0xa0},
          { 0xA3A << 2, 0xa0},
          { 0xA3B << 2, 0xa0},
          { 0xA3C << 2, 0xa0},
          { 0xA3D << 2, 0xa0},
          { 0xA3E << 2, 0xa0},
          { 0xA3F << 2, 0xa0},
          { 0xA40 << 2, 0x6c}, //
          { 0xA41 << 2, 0x00},
          { 0xA42 << 2, 0xc0},
          { 0xA43 << 2, 0x9f},
          { 0xA44 << 2, 0x01},
          { 0xA45 << 2, 0x00},
          { 0xA46 << 2, 0x00},
          { 0xA47 << 2, 0x00},
          { 0xA48 << 2, 0x00},
          { 0xA49 << 2, 0x00},
          { 0xA4A << 2, 0x00},
          { 0xA4B << 2, 0x00},
          { 0xA4C << 2, 0x30},
          { 0xA4D << 2, 0x41},
          { 0xA4E << 2, 0x7e},
          { 0xA4F << 2, 0xd0},
          { 0xA50 << 2, 0xcc},
          { 0xA51 << 2, 0x85},
          { 0xA52 << 2, 0x52},
          { 0xA53 << 2, 0x93},
          { 0xA54 << 2, 0xe0},
          { 0xA55 << 2, 0x49},
          { 0xA56 << 2, 0xdd},
          { 0xA57 << 2, 0xb0},
          { 0xA58 << 2, 0x0b},
          { 0xA59 << 2, 0x02},
          { 0xA5A << 2, 0x00},
          { 0xA5B << 2, 0x00},
          { 0xA5C << 2, 0x00},
          { 0xA5D << 2, 0x00},
          { 0xA5E << 2, 0x00},
          { 0xA5F << 2, 0x00},
          { 0xA60 << 2, 0x00},
          { 0xA61 << 2, 0x00},
          { 0xA62 << 2, 0x00},
          { 0xA63 << 2, 0x00},
          { 0xA64 << 2, 0x00},
          { 0xA65 << 2, 0x00},
          { 0xA66 << 2, 0x00},
          { 0xA67 << 2, 0x00},
          { 0xA68 << 2, 0x00},
          { 0xA69 << 2, 0x00},
          { 0xA6A << 2, 0x00},
          { 0xA6B << 2, 0x00},
          { 0xA6C << 2, 0x00},
          { 0xA6D << 2, 0x00},
          { 0xA6E << 2, 0x00},
          { 0xA6F << 2, 0x00},
          { 0xA70 << 2, 0x00},
          { 0xA71 << 2, 0x00},
          { 0xA72 << 2, 0x00},
          { 0xA73 << 2, 0x00},
          { 0xA74 << 2, 0x00},
          { 0xA75 << 2, 0x00},
          { 0xA76 << 2, 0x00},
          { 0xA77 << 2, 0x00},
          { 0xA78 << 2, 0x00},
          { 0xA79 << 2, 0x00},
          { 0xA7A << 2, 0x00},
          { 0xA7B << 2, 0x00},
          { 0xA7C << 2, 0x00},
          { 0xA7D << 2, 0x00},
          { 0xA7E << 2, 0x00},
          { 0xA7F << 2, 0xd8},
          { 0xA80 << 2, 0x1a},
          { 0xA81 << 2, 0xff},
          { 0xA82 << 2, 0x01},
          { 0xA83 << 2, 0x00},
          { 0xA84 << 2, 0x00},
          { 0xA85 << 2, 0x00},
          { 0xA86 << 2, 0x00},
          { 0xA87 << 2, 0xf0},
          { 0xA88 << 2, 0xff},
          { 0xA89 << 2, 0xff},
          { 0xA8A << 2, 0xff},
          { 0xA8B << 2, 0xff},
          { 0xA8C << 2, 0x1c},
          { 0xA8D << 2, 0xc2},
          { 0xA8E << 2, 0xc3},
          { 0xA8F << 2, 0x3f},
          { 0xA90 << 2, 0x0a},
          { 0xA91 << 2, 0x00},
          { 0xA92 << 2, 0x00},
          { 0xA93 << 2, 0x00},
          { 0xA94 << 2, 0x00},
          { 0xA95 << 2, 0x00},
          { 0xA96 << 2, 0xf8},
          { 0x000 << 2, 0x07}
	};

static struct serdes_regs_s pcie_phy_reg_file_24[] = /*** pcie_phy_reg_file_24[] ***/
	{
	  /* Common CMU block */
  	  { 0x000 << 2, 0x06},
  	  { 0x001 << 2, 0x00},
  	  { 0x002 << 2, 0x09}, //
 	  { 0x003 << 2, 0x00},
 	  { 0x004 << 2, 0x60}, //
 	  { 0x005 << 2, 0x09}, //
 	  { 0x006 << 2, 0x0e}, //
 	  { 0x007 << 2, 0x00},
 	  { 0x008 << 2, 0x00},
 	  { 0x009 << 2, 0x00},
 	  { 0x00A << 2, 0x00},
 	  { 0x00B << 2, 0x00},
 	  { 0x00C << 2, 0x00},
 	  { 0x00D << 2, 0x00},
 	  { 0x00E << 2, 0x00},
 	  { 0x00F << 2, 0x00},
 	  { 0x010 << 2, 0x00},
 	  { 0x011 << 2, 0x00},
 	  { 0x012 << 2, 0x00},
 	  { 0x013 << 2, 0x00},
 	  { 0x014 << 2, 0x00},
 	  { 0x015 << 2, 0x00},
 	  { 0x016 << 2, 0x00},
 	  { 0x017 << 2, 0x00},
 	  { 0x018 << 2, 0x00},
 	  { 0x019 << 2, 0x00},
 	  { 0x01A << 2, 0x00},
 	  { 0x01B << 2, 0x00},
 	  { 0x01C << 2, 0x00},
 	  { 0x01D << 2, 0x00},
 	  { 0x01E << 2, 0x00},
 	  { 0x01F << 2, 0x00},
 	  { 0x020 << 2, 0x00},
 	  { 0x021 << 2, 0x00},
 	  { 0x022 << 2, 0xa0},
 	  { 0x023 << 2, 0x68}, //
 	  { 0x024 << 2, 0x00},
 	  { 0x025 << 2, 0x00},
 	  { 0x026 << 2, 0x00},
 	  { 0x027 << 2, 0x00},
 	  { 0x028 << 2, 0x00},
 	  { 0x029 << 2, 0x00},
 	  { 0x02A << 2, 0x00},
 	  { 0x02B << 2, 0x00},
 	  { 0x02C << 2, 0x00},
 	  { 0x02D << 2, 0x00},
 	  { 0x02E << 2, 0x04},
 	  { 0x02F << 2, 0x50},
 	  { 0x030 << 2, 0x70},
 	  { 0x031 << 2, 0x02},
 	  { 0x032 << 2, 0x25},
 	  { 0x033 << 2, 0x40},
 	  { 0x034 << 2, 0x01},
 	  { 0x035 << 2, 0x40},
 	  { 0x036 << 2, 0x00},
 	  { 0x037 << 2, 0x00},
 	  { 0x038 << 2, 0x00},
 	  { 0x039 << 2, 0x00},
 	  { 0x03A << 2, 0x00},
 	  { 0x03B << 2, 0x00},
 	  { 0x03C << 2, 0x00},
 	  { 0x03D << 2, 0x00},
 	  { 0x03E << 2, 0x00},
 	  { 0x03F << 2, 0x00},
 	  { 0x040 << 2, 0x00},
 	  { 0x041 << 2, 0x00},
 	  { 0x042 << 2, 0x00},
 	  { 0x043 << 2, 0x00},
 	  { 0x044 << 2, 0x00},
 	  { 0x045 << 2, 0x00},
 	  { 0x046 << 2, 0x00},
 	  { 0x047 << 2, 0x00},
 	  { 0x048 << 2, 0x00},
 	  { 0x049 << 2, 0x00},
 	  { 0x04A << 2, 0x00},
 	  { 0x04B << 2, 0x00},
 	  { 0x04C << 2, 0x00},
 	  { 0x04D << 2, 0x00},
 	  { 0x04E << 2, 0x00},
 	  { 0x04F << 2, 0x00},
 	  { 0x050 << 2, 0x00},
 	  { 0x051 << 2, 0x00},
 	  { 0x052 << 2, 0x00},
 	  { 0x053 << 2, 0x00},
 	  { 0x054 << 2, 0x00},
 	  { 0x055 << 2, 0x00},
 	  { 0x056 << 2, 0x00},
 	  { 0x057 << 2, 0x00},
 	  { 0x058 << 2, 0x00},
 	  { 0x059 << 2, 0x00},
 	  { 0x05A << 2, 0x00},
 	  { 0x05B << 2, 0x00},
 	  { 0x05C << 2, 0x00},
 	  { 0x05D << 2, 0x00},
 	  { 0x05E << 2, 0x00},
 	  { 0x05F << 2, 0x00},
 	  { 0x060 << 2, 0x00},
 	  { 0x061 << 2, 0x06}, //for Rev-A0 device: 0x2e, Rev-A1: 0x06
 	  { 0x062 << 2, 0x00}, //
 	  { 0x063 << 2, 0x5e},
 	  { 0x064 << 2, 0x00},
 	  { 0x065 << 2, 0x42},
  	  { 0x066 << 2, 0x91},
 	  { 0x067 << 2, 0x10}, //
 	  { 0x068 << 2, 0x48},
 	  { 0x069 << 2, 0x90}, //
 	  { 0x06A << 2, 0x0c}, //
 	  { 0x06B << 2, 0x4c}, //
 	  { 0x06C << 2, 0x73}, //
 	  { 0x06D << 2, 0x03}, //
 	  { 0x06E << 2, 0x00},
 	  { 0x06F << 2, 0x00},
  	  { 0x070 << 2, 0x00},
 	  { 0x071 << 2, 0x00},
 	  { 0x072 << 2, 0x00},
 	  { 0x073 << 2, 0x00},
 	  { 0x075 << 2, 0x00},
  	  /* Lane0 Block */
  	  { 0x200 << 2, 0x00},
 	  { 0x201 << 2, 0x00},
 	  { 0x202 << 2, 0x00},
 	  { 0x203 << 2, 0x00},
 	  { 0x204 << 2, 0x00},
 	  { 0x205 << 2, 0x10},
 	  { 0x206 << 2, 0x04},
 	  { 0x207 << 2, 0x18},
 	  { 0x208 << 2, 0xe0},
 	  { 0x210 << 2, 0x23},
 	  { 0x211 << 2, 0x00},
 	  { 0x212 << 2, 0x00},
 	  { 0x213 << 2, 0x04}, //
 	  { 0x214 << 2, 0x38}, //
 	  { 0x215 << 2, 0x10}, //
 	  { 0x216 << 2, 0x00},
 	  { 0x217 << 2, 0x68},
 	  { 0x218 << 2, 0xa2},
 	  { 0x219 << 2, 0x1e},
 	  { 0x21A << 2, 0x18},
 	  { 0x21B << 2, 0x0d},
 	  { 0x21C << 2, 0x0c},
 	  { 0x21D << 2, 0x00},
 	  { 0x21E << 2, 0x00},
 	  { 0x21F << 2, 0x00},
 	  { 0x220 << 2, 0x00},
 	  { 0x221 << 2, 0x00},
 	  { 0x222 << 2, 0x00},
 	  { 0x223 << 2, 0x00},
 	  { 0x224 << 2, 0x00},
 	  { 0x225 << 2, 0x00},
 	  { 0x226 << 2, 0x00},
 	  { 0x227 << 2, 0x00},
 	  { 0x228 << 2, 0x00},
 	  { 0x229 << 2, 0x00},
 	  { 0x22A << 2, 0x00},
 	  { 0x22B << 2, 0x00},
 	  { 0x22C << 2, 0x00},
 	  { 0x22D << 2, 0x00},
 	  { 0x22E << 2, 0x00},
 	  { 0x22F << 2, 0x00},
 	  { 0x230 << 2, 0x00},
 	  { 0x231 << 2, 0x00},
 	  { 0x232 << 2, 0x00},
 	  { 0x233 << 2, 0x00},
 	  { 0x234 << 2, 0x00},
 	  { 0x235 << 2, 0x00},
 	  { 0x236 << 2, 0x00},
 	  { 0x237 << 2, 0x00},
 	  { 0x238 << 2, 0x00},
 	  { 0x239 << 2, 0x00},
 	  { 0x23A << 2, 0x00},
 	  { 0x23B << 2, 0x00},
 	  { 0x23C << 2, 0x00},
 	  { 0x23D << 2, 0x00},
 	  { 0x23E << 2, 0x00},
 	  { 0x23F << 2, 0x00},
 	  { 0x240 << 2, 0x00},
 	  { 0x241 << 2, 0x00},
 	  { 0x242 << 2, 0x00},
 	  { 0x243 << 2, 0x00},
 	  { 0x244 << 2, 0x00},
 	  { 0x245 << 2, 0x00},
 	  { 0x246 << 2, 0x00},
 	  { 0x247 << 2, 0x00},
 	  { 0x248 << 2, 0x00},
 	  { 0x249 << 2, 0x00},
 	  { 0x24A << 2, 0x00},
 	  { 0x24B << 2, 0x00},
 	  { 0x24C << 2, 0x00},
 	  { 0x24D << 2, 0x00},
 	  { 0x24E << 2, 0x00},
 	  { 0x24F << 2, 0x00},
 	  { 0x250 << 2, 0xf6},
 	  { 0x251 << 2, 0x03},
	  /* Common Lane Block */
 	  { 0xA00 << 2, 0xc0},
 	  { 0xA01 << 2, 0x90},
 	  { 0xA02 << 2, 0x02},
 	  { 0xA03 << 2, 0x40},
 	  { 0xA04 << 2, 0x3c},
 	  { 0xA05 << 2, 0x00},
 	  { 0xA06 << 2, 0x00},
 	  { 0xA07 << 2, 0x00},
 	  { 0xA08 << 2, 0x00},
 	  { 0xA09 << 2, 0x83}, //
 	  { 0xA0A << 2, 0x8b}, //
 	  { 0xA0B << 2, 0xc6},
 	  { 0xA0C << 2, 0x01},
 	  { 0xA0D << 2, 0x03},
 	  { 0xA0E << 2, 0x28},
 	  { 0xA0F << 2, 0x98},
 	  { 0xA10 << 2, 0x19},
 	  { 0xA11 << 2, 0x28},
 	  { 0xA12 << 2, 0x78},
 	  { 0xA13 << 2, 0xe1},
 	  { 0xA14 << 2, 0xf0},
 	  { 0xA15 << 2, 0x10},
 	  { 0xA16 << 2, 0xf4},
 	  { 0xA17 << 2, 0x00},
  	  { 0xA30 << 2, 0x00},
 	  { 0xA31 << 2, 0x00},
 	  { 0xA32 << 2, 0x00},
 	  { 0xA33 << 2, 0x00},
 	  { 0xA34 << 2, 0x00},
 	  { 0xA35 << 2, 0x00},
 	  { 0xA36 << 2, 0x00},
 	  { 0xA37 << 2, 0x00},
 	  { 0xA38 << 2, 0x00},
 	  { 0xA39 << 2, 0xa0},
 	  { 0xA3A << 2, 0xa0},
 	  { 0xA3B << 2, 0xa0},
 	  { 0xA3C << 2, 0xa0},
 	  { 0xA3D << 2, 0xa0},
 	  { 0xA3E << 2, 0xa0},
 	  { 0xA3F << 2, 0xa0},
 	  { 0xA40 << 2, 0x68}, //
 	  { 0xA41 << 2, 0x00},
 	  { 0xA42 << 2, 0xc0},
 	  { 0xA43 << 2, 0x9f},
 	  { 0xA44 << 2, 0x01},
 	  { 0xA45 << 2, 0x00},
 	  { 0xA46 << 2, 0x00},
 	  { 0xA47 << 2, 0x00},
 	  { 0xA48 << 2, 0x00},
 	  { 0xA49 << 2, 0x00},
 	  { 0xA4A << 2, 0x00},
 	  { 0xA4B << 2, 0x00},
 	  { 0xA4C << 2, 0x30},
 	  { 0xA4D << 2, 0x41},
 	  { 0xA4E << 2, 0x7e},
 	  { 0xA4F << 2, 0xd0},
 	  { 0xA50 << 2, 0xcc},
 	  { 0xA51 << 2, 0x85},
 	  { 0xA52 << 2, 0x52},
 	  { 0xA53 << 2, 0x93},
 	  { 0xA54 << 2, 0xe0},
 	  { 0xA55 << 2, 0x49},
 	  { 0xA56 << 2, 0xdd},
 	  { 0xA57 << 2, 0xb0},
 	  { 0xA58 << 2, 0x0b},
 	  { 0xA59 << 2, 0x02},
 	  { 0xA5A << 2, 0x00},
 	  { 0xA5B << 2, 0x00},
 	  { 0xA5C << 2, 0x00},
 	  { 0xA5D << 2, 0x00},
 	  { 0xA5E << 2, 0x00},
 	  { 0xA5F << 2, 0x00},
 	  { 0xA60 << 2, 0x00},
 	  { 0xA61 << 2, 0x00},
  	  { 0xA62 << 2, 0x00},
 	  { 0xA63 << 2, 0x00},
 	  { 0xA64 << 2, 0x00},
 	  { 0xA65 << 2, 0x00},
 	  { 0xA66 << 2, 0x00},
 	  { 0xA67 << 2, 0x00},
 	  { 0xA68 << 2, 0x00},
 	  { 0xA69 << 2, 0x00},
 	  { 0xA6A << 2, 0x00},
 	  { 0xA6B << 2, 0x00},
 	  { 0xA6C << 2, 0x00},
 	  { 0xA6D << 2, 0x00},
 	  { 0xA6E << 2, 0x00},
 	  { 0xA6F << 2, 0x00},
 	  { 0xA70 << 2, 0x00},
 	  { 0xA71 << 2, 0x00},
 	  { 0xA72 << 2, 0x00},
 	  { 0xA73 << 2, 0x00},
 	  { 0xA74 << 2, 0x00},
 	  { 0xA75 << 2, 0x00},
 	  { 0xA76 << 2, 0x00},
 	  { 0xA77 << 2, 0x00},
 	  { 0xA78 << 2, 0x00},
 	  { 0xA79 << 2, 0x00},
 	  { 0xA7A << 2, 0x00},
 	  { 0xA7B << 2, 0x00},
 	  { 0xA7C << 2, 0x00},
 	  { 0xA7D << 2, 0x00},
 	  { 0xA7E << 2, 0x00},
 	  { 0xA7F << 2, 0xd8},
 	  { 0xA80 << 2, 0x1a},
 	  { 0xA81 << 2, 0xff},
 	  { 0xA82 << 2, 0x01},
 	  { 0xA83 << 2, 0x00},
 	  { 0xA84 << 2, 0x00},
 	  { 0xA85 << 2, 0x00},
 	  { 0xA86 << 2, 0x00},
 	  { 0xA87 << 2, 0xf0},
 	  { 0xA88 << 2, 0xff},
 	  { 0xA89 << 2, 0xff},
 	  { 0xA8A << 2, 0xff},
 	  { 0xA8B << 2, 0xff},
 	  { 0xA8C << 2, 0x1c},
 	  { 0xA8D << 2, 0xc2},
 	  { 0xA8E << 2, 0xc3},
 	  { 0xA8F << 2, 0x3f},
 	  { 0xA90 << 2, 0x0a},
 	  { 0xA91 << 2, 0x00},
 	  { 0xA92 << 2, 0x00},
 	  { 0xA93 << 2, 0x00},
  	  { 0xA94 << 2, 0x00},
 	  { 0xA95 << 2, 0x00},
 	  { 0xA96 << 2, 0xf8},
   	  { 0x000 << 2, 0x07}
	};

#endif
