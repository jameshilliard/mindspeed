
/*
 * diags/test_pcie.c
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

#include <types.h>
#include <common.h>
#include <asm/io.h>
#include <mach/comcerto-2000.h>
#include <mach/pcie.h>
#include <mach/serdes.h>
#include <mach/clkcore.h>
#include <mach/gpio.h>
#include <diags.h>
#include "test_pcie.h"
#include "tests.h"
#include "dump.h"
#include "common_func.h"
#include "serdes_common.h"

#define U8 u8

#define PCI_VENDOR_ID		0x00	/* 16 bits */
#define PCI_DEVICE_ID		0x02	/* 16 bits */
#define PCI_STATUS_CMD		0x04
#define PCI_CLASS_REV		0x08
#define PCI_BIST_HDR_TMR_CSZ	0x0c
#define PCI_SUBSYS_ID		0x2c	/* 16 bits */
#define PCI_BASE_ADDRESS_0	0x10	/* 32 bits */

/*** from kernel: ~/arm/mach-comcerto/include/mach/reset.h   ***/
#define SERDES_PCIE0_RESET_BIT   0x00000003
#define SERDES_PCIE1_RESET_BIT   0x0000000c

/* DWC PCIEe configuration register offsets on APB */
struct pcie_app_reg app_regs[NUM_PCIE_PORTS] = {
	//PCIe0 
	{ 0x00000000,     // cfg0
	  0x00000004,
	  0x00000008,
	  0x0000000C,
	  0x00000010,
	  0x00000014,
	  0x00000018,     
	  0x00000040,     // sts0
	  0x00000044,
	  0x00000048,
	  0x00000058,     
	  0x00000080,     // pwr_cfg_bdgt_data
	  0x00000084,
	  0x000000C0,     // radm_sts
	  0x000000C4,     // pwr_sts_bdgt
	  0x00000100,     // intr_sts
	  0x00000104,     // intr_en
	  0x00000108,     // intr_msi_sts
	  0x0000010C      // intr_msi_en
	},
	//PCIe1
	{ 0x00000020,     // cfg0
	  0x00000024,
	  0x00000028,
	  0x0000002C,
	  0x00000030,
	  0x00000034,
	  0x00000038,
	  0x0000004C,     // sts0
	  0x00000050,
	  0x00000054,
	  0x0000005C,
	  0x00000088,     // pwr_cfg_bdgt_data
	  0x0000008C,
	  0x000000C8,     // radm_sts
	  0x000000CC,     // pwr_stst_bdgt
	  0x00000110,
	  0x00000114,
	  0x00000118,
	  0x0000011C
	}
};

/* Keeping all DDR area of 256MB accesible for inbound transaction */
#define INBOUND_ADDR_MASK	0xFFFFFFF


#define PCIE_SETUP_iATU_IB_ENTRY( _pp, _view_port, _base, _limit, _ctl1, _ctl2, _target ) \
{\
	comcerto_dbi_write_reg(_pp, PCIE_iATU_VIEW_PORT, 4, (u32)(_view_port|iATU_VIEW_PORT_IN_BOUND)); \
	comcerto_dbi_write_reg(_pp, PCIE_iATU_CTRL2, 4, 0); \
	comcerto_dbi_write_reg(_pp, PCIE_iATU_SRC_LOW, 4, (u32)_base); \
	comcerto_dbi_write_reg(_pp, PCIE_iATU_SRC_HIGH, 4, 0); \
	comcerto_dbi_write_reg(_pp, PCIE_iATU_LIMIT, 4, (u32)_limit); \
	comcerto_dbi_write_reg(_pp, PCIE_iATU_TRGT_LOW, 4, (u32)_target); \
	comcerto_dbi_write_reg(_pp, PCIE_iATU_TRGT_HIGH, 4, (u32)0); \
	comcerto_dbi_write_reg(_pp, PCIE_iATU_CTRL1, 4, (u32)_ctl1); \
	comcerto_dbi_write_reg(_pp, PCIE_iATU_CTRL2, 4, (u32)(_ctl2 |iATU_CTRL2_ID_EN) ); \
}

#define PCIE_SETUP_iATU_OB_ENTRY( _pp, _view_port, _base, _limit, _ctl1, _ctl2, _target ) \
{\
	comcerto_dbi_write_reg(_pp, PCIE_iATU_VIEW_PORT, 4, (u32)_view_port); \
	comcerto_dbi_write_reg(_pp, PCIE_iATU_CTRL2, 4, 0); \
	comcerto_dbi_write_reg(_pp, PCIE_iATU_SRC_LOW, 4, (u32)_base); \
	comcerto_dbi_write_reg(_pp, PCIE_iATU_SRC_HIGH, 4, (u32)0); \
	comcerto_dbi_write_reg(_pp, PCIE_iATU_LIMIT, 4, (u32)_limit); \
	comcerto_dbi_write_reg(_pp, PCIE_iATU_TRGT_LOW, 4, (u32)_target); \
	comcerto_dbi_write_reg(_pp, PCIE_iATU_TRGT_HIGH, 4, (u32)0); \
	comcerto_dbi_write_reg(_pp, PCIE_iATU_CTRL1, 4, (u32)_ctl1); \
	comcerto_dbi_write_reg(_pp, PCIE_iATU_CTRL2, 4, (u32)(_ctl2 |iATU_CTRL2_ID_EN) ); \
}

//#define MAX_LINK_UP_WAIT_JIFFIES	100
#define MAX_LINK_UP_WAIT_JIFFIES	0xfff
#define PCI_SLOT(d)         (((d) >> 3) & 0x1f)
#define PCI_FUNC(d)	(((d) >> 8) & 0x7)

#define GPIO_MISC_PIN_SELECT		(GPIO_BASE + 0x60)
#define GPIO_63_32_PIN_OUTPUT		(GPIO_BASE + 0xd0)
#define GPIO_63_32_PIN_OUTPUT_EN	(GPIO_BASE + 0xd4)

		/*** 0x9800_0000, 0x9900_0000	***/
static unsigned long pcie_cnf_base_addr[NUM_PCIE_PORTS] =
                { COMCERTO_AXI_PCIe0_CFG_BASE, COMCERTO_AXI_PCIe1_CFG_BASE };

                /*** 0xA000_0000, 0xB000_0000 ***/
static unsigned long pcie_remote_base_addr[NUM_PCIE_PORTS] =
                { COMCERTO_AXI_PCIe0_SLV_BASE, COMCERTO_AXI_PCIe1_SLV_BASE };

static struct pcie_port pcie_port[NUM_PCIE_PORTS];

static int initialized[NUM_PCIE_PORTS] = {0, 0};

static void __init pcie_port_init_rc(int port);


void pcie_wEnable (int enable)
{
 int gpio_reg = GPIO_BASE + 0xd0;
 unsigned long gpio_mask = 0x00006000;
 unsigned int rdata, wdata;

    rdata = readl(GPIO_63_32_PIN_OUTPUT_EN);
    wdata = rdata | gpio_mask;
#ifdef PCIE_DEBUG_LOOPBACK
    printf("pcie: enReg:0x%x 0x%x: 0x0000-6000 ! \n", GPIO_63_32_PIN_OUTPUT_EN, wdata);
#endif
    writel(rdata, GPIO_63_32_PIN_OUTPUT_EN);

    rdata = readl(gpio_reg);
    wdata = enable ? rdata | gpio_mask : rdata & ~gpio_mask;
#ifdef PCIE_DEBUG_LOOPBACK
    printf("pcie: W-46 & W-45 Reg:0x%x to get Data=0x%x ! \n", gpio_reg, wdata);
#endif
    writel(wdata, GPIO_63_32_PIN_OUTPUT);

    rdata = readl(GPIO_63_32_PIN_OUTPUT);
#ifdef PCIE_DEBUG_LOOPBACK
    printf("pcie_wEnable GPIO-46-45:0x%x 0x%x:0x0000-6000 atEnd! \n", 
                         GPIO_63_32_PIN_OUTPUT, rdata);
#endif
}

void pcie0_reset (int reset)
{
 int gpio_reg = GPIO_BASE + 0xd0;
 int misc_reg = GPIO_BASE + 0x60;
 unsigned long misc_mask = 0x00000030;
 unsigned long misc_gpio = 0x00000020;
 unsigned long gpio_mask = 0x40000000;
 unsigned int rdata, wdata;

    rdata = readl(misc_reg);
#ifdef PCIE_DEBUG_LOOPBACK
    printf("pcie0: select miscSel62 Reg:0x%x to read Data=0x%x ! \n", misc_reg, rdata);
#endif

    rdata = readl(GPIO_MISC_PIN_SELECT);
    rdata &= ~misc_mask;
    rdata |=  misc_gpio;
#ifdef PCIE_DEBUG_LOOPBACK
    printf("pcie0: set miscSel62 Reg:0x%x to Data=0x%x ! \n", GPIO_MISC_PIN_SELECT, rdata);
#endif
    writel(rdata, GPIO_MISC_PIN_SELECT);

    rdata = readl(GPIO_63_32_PIN_OUTPUT_EN);
    rdata |= gpio_mask;
#ifdef PCIE_DEBUG_LOOPBACK
    printf("pcie0: Enable GPIO62_Reg:0x%x 0x%x ! \n", GPIO_63_32_PIN_OUTPUT_EN, rdata);
#endif
    writel(rdata, GPIO_63_32_PIN_OUTPUT_EN);

    rdata = readl(gpio_reg);
#ifdef PCIE_DEBUG_LOOPBACK
    printf("pcie0: read GPIO-62:0x%x: 0x%x ! \n", GPIO_63_32_PIN_OUTPUT, rdata);
#endif

    if (reset)
    {
    	wdata = rdata & ~gpio_mask;
        writel(wdata, GPIO_63_32_PIN_OUTPUT);
#ifdef PCIE_DEBUG_LOOPBACK
    	printf("pcie0: set GPIO-62:0x%x: 0x%x > 0x%x, LOW Reset ! \n", 
                              GPIO_63_32_PIN_OUTPUT, rdata, wdata);
#endif
    }
    else
    {
    	wdata = rdata | gpio_mask;
#ifdef PCIE_DEBUG_LOOPBACK
    	printf("pcie0: set GPIO-62:0x%x: 0x%x > 0x%x, HIGH Normal! \n", 
                              GPIO_63_32_PIN_OUTPUT, rdata, wdata);
#endif
    }
    writel(wdata, GPIO_63_32_PIN_OUTPUT);

    rdata = readl(GPIO_BASE + 0xd0);
#ifdef PCIE_DEBUG_LOOPBACK
    printf("pcie0: GPIO-62: Data=0x%x, mask:0x4000-0000 at END ! \n", rdata);
#endif
}

void pcie1_reset (int reset)
{
 int gpio_reg = GPIO_BASE + 0x00;
 unsigned long gpio_mask = 0x08000000;
 unsigned int rdata, wdata;

    rdata = readl(gpio_reg);
#ifdef PCIE_DEBUG_LOOPBACK
    printf("pcie1: read GPIO-27: 0x%x, Data=0x%x ! \n", gpio_reg, rdata);
#endif

//  output enable
    wdata = readl(gpio_reg + 4) | gpio_mask;
    writel(wdata, gpio_reg + 4);

    if (reset)
    {
    	reg_rmw((GPIO_BASE+ 0x0),0x08000000, 0x0);
    }
    else
    {
    	reg_rmw((GPIO_BASE+ 0x0),0x08000000, 0x08000000);
    }

    rdata = readl(gpio_reg);
#ifdef PCIE_DEBUG_LOOPBACK
    printf("pcie1: GPIO-27: Data=0x%x, mask:0x0800-0000 at END! \n", rdata);
#endif
}

#define PCIE_PL_REG     0x700
#define PCIE_AFL0L1_REG (PCIE_PL_REG + 0xc)
#define PCIE_PLCTL_REG  (PCIE_PL_REG + 0x10)
#define PCIE_SYMNUM     (PCIE_PL_REG + 0x18)
#define PCIE_G2CTRL_REG (PCIE_PL_REG + 0x10c)

#define PCIE_CAP_BASE           0x70
#define PCIE_LCAP_REG           (PCIE_CAP_BASE + 0x0C)
#define PCIE_LCNT2_REG          (PCIE_CAP_BASE + 0x30)

#define PCIE_LOOP_DEST 0x70000000

static void pcie_loopback(int port)
{
        struct pcie_port *pp = &pcie_port[port];
        writel((readl(pp->base + PCIE_PLCTL_REG)) | 0x4,  pp->base + PCIE_PLCTL_REG);
        printf("Enabling Loopback: PCIE_PLCTL_REG 0x%x\n",readl(pp->base + PCIE_PLCTL_REG));
}


static void comcerto_pcie_LTSon (int port)
{
 struct pcie_port *pp = &pcie_port[port];
 int app_cfg5 = pp->app_base + pp->app_regs->cfg5;
 unsigned int reg_data;

        reg_data = readl(app_cfg5) & ~CFG5_LTSSM_ENABLE;
        writel(reg_data, app_cfg5);
        printf("%s : PCIe-%d app_cfg5 0x%x data=0x%x, CFG5_LTSSM: held ! \n",
                __func__, port, app_cfg5, reg_data);

        pcie_port_init_rc(port);

	pcie_loopback(port);

        reg_data = readl(app_cfg5) | (CFG5_APP_INIT_RST | CFG5_LTSSM_ENABLE);
        writel(reg_data, app_cfg5);
        printf("%s : PCIe-%d app_cfg5:0x%x data=0x%x LTSSM Enabling! \n",
                __func__, port, app_cfg5, reg_data);
}

static void comcerto_serdes_set_polarity(int current_polarity)
{
	struct serdes_regs_s *p_pcie_phy_reg_file_24 = &pcie_phy_reg_file_24[0];

printf ("%s: polarity=%d \n", __func__, current_polarity);


        switch(current_polarity)
        {
                case 0:
                        p_pcie_phy_reg_file_24[0x73].val = 0x0;
                        p_pcie_phy_reg_file_24[0x75].val = 0x0;
                        break;
                case 1:
                        p_pcie_phy_reg_file_24[0x73].val = 0x8;
                        p_pcie_phy_reg_file_24[0x75].val = 0x2;
                        break;
                case 2:
                        p_pcie_phy_reg_file_24[0x73].val = 0x0;
                        p_pcie_phy_reg_file_24[0x75].val = 0x2;
                        break;
                case 3:
                        p_pcie_phy_reg_file_24[0x73].val = 0x8;
                        p_pcie_phy_reg_file_24[0x75].val = 0x0;
                        break;
        }

}

/**
 * This function checks whether link is up or not.
 * Returns true if link is up otherwise returns false.
 * @param pp	Pointer to PCIe Port control block.
 */
static int comcerto_pcie_link_up( struct pcie_port *pp  )
{
 int abase = pp->app_base;
 int sts0  = pp->app_regs->sts0;
 int link_reg = abase + sts0;
 int link_stat;

	unsigned long timeout = MAX_LINK_UP_WAIT_JIFFIES;

printf("comcerto_pcie_link_up: pcie_port->0x%x, abase=0x%x sts0=0x%x \n", link_reg, abase, sts0);

	do {
		if (readl( pp->app_base + pp->app_regs->sts0 ) & STS0_RDLH_LINK_UP) {
			printf("PCIe Link up: Ok\n");
			return 1;
		}

		mdelay(1);
	} while (timeout--);

	link_stat =  readl( pp->app_base + pp->app_regs->sts0 );
	printf("PCIe Link up: failed link_stat= 0x%x, STS0_RDLH_LINK_UP=0x%x \n", 
                              link_stat, STS0_RDLH_LINK_UP);

	return 0;
}

/**
 * This function is used to read DBI registers.
 */

static void comcerto_dbi_read_reg(struct pcie_port *pp, int where, int size,
		u32 *val)
{
	u32 address;

	address = (u32)pp->base + (where & ~0x3);

	*val = readl(address);

	if (size == 1)
		*val = (*val >> (8 * (where & 3))) & 0xff;
	else if (size == 2)
		*val = (*val >> (8 * (where & 3))) & 0xffff;
}

/**
 * This function is used to write into DBI registers.
 */
static void comcerto_dbi_write_reg(struct pcie_port *pp, int where, int size,
		u32 val)
{
	u32 address;
	int pos, val1, mask = 0;

	//printk("%s: start\n", __func__);

	address = (u32)pp->base + (where & ~0x3);

	pos = (where & 0x3) << 3;

	if (size == 4)
		val1 = val;
	else
	{
		if (size == 2)
			mask = 0xffff;
		else if (size == 1)
			mask = 0xff;

		val1 = readl(address);
		val1 = ( val1 & ~( mask  << pos ) ) | ( (val & mask) << pos );
	}

#ifdef PCIE_DEBUG_LOOPBACK
	printk("%s: address: %08x, val: %08x\n", __func__, address, val1);
#endif

	writel(val1, address);

	//printk("%s: done\n", __func__);
}


static int comcerto_pcie_rd_conf(struct pcie_port *pp, int bus_nr,
		u32 devfn, int where, int size, u32 *val)
{
 	u32 address;
	u32 target_address = (u32)(bus_nr << 24) | (PCI_SLOT(devfn) << 19) | (PCI_FUNC(devfn) << 16);

	//Initialize iATU
	if (bus_nr != pp->root_bus_nr) {
		/* Type1 configuration request */
		PCIE_SETUP_iATU_OB_ENTRY( pp, iATU_ENTRY_CNF1, (u32)pp->cfg1_base,
				((u32)(pp->cfg1_base + (iATU_CFG1_SIZE - 1))),
				(AXI_OP_TYPE_CONFIG_RDRW_TYPE1 & iATU_CTRL1_TYPE_MASK),
				0, target_address );

		address = (u32)pp->cfg1_base |(where & 0xFFFC);
	} else {
		/* +0x0904: AXI_OP_TYPE_CONFIG_RDRW_TYPE0 = 4 */
		PCIE_SETUP_iATU_OB_ENTRY( pp, iATU_ENTRY_CNF0, (u32)pp->cfg0_base,
				((u32)(pp->cfg0_base + (iATU_CFG0_SIZE - 1))),
				(AXI_OP_TYPE_CONFIG_RDRW_TYPE0 & iATU_CTRL1_TYPE_MASK),
				0, target_address );

		address = (u32)((pp->cfg0_base)|(where & 0xFFFC));
	}

	*val = readl(address);

        printf("%s: root_bus-%d readl<address->%08x> = 0x%08x devfn=%d iATUtarget=0x%08x \n", 
		__func__, pp->root_bus_nr, address, *val, devfn, target_address);

	if (size == 1)
		*val = (*val >> (8 * (where & 3))) & 0xff;
	else if (size == 2)
		*val = (*val >> (8 * (where & 3))) & 0xffff;

	return 0;
}

static int pcie_read_conf(struct pcie_port *pp, int bus_nr, u32 devfn, int where, int size, u32 *val)
{
	int ret;

 	/* Make sure that link is up.
	 * Filter device numbers, unless it's a type1 access
	 */
	if ( (!pp->link_state)||
			((bus_nr == pp->root_bus_nr) && (PCI_SLOT(devfn) > 0)) ) {
		*val = 0xffffffff;
		return -1;
	}

	BUG_ON (((where & 0x3) + size) > 4);

	ret = comcerto_pcie_rd_conf(pp, bus_nr, devfn, where, size, val);

	return ret;
}

static int comcerto_pcie_wr_conf(struct pcie_port *pp, int bus_nr,
		u32 devfn, int where, int size, u32 val)
{
	int ret = 0;
	u32 address;
	u32 target_address = (u32)(bus_nr << 24) | (PCI_SLOT(devfn) << 19) | (PCI_FUNC(devfn) << 16);

	//Initialize iATU
	if (bus_nr != pp->root_bus_nr) {
		/* Type1 configuration request */
		PCIE_SETUP_iATU_OB_ENTRY( pp, iATU_ENTRY_CNF1, (u32)pp->cfg1_base,
				((u32)(pp->cfg1_base + (iATU_CFG1_SIZE - 1))),
				(AXI_OP_TYPE_CONFIG_RDRW_TYPE1 & iATU_CTRL1_TYPE_MASK),
				0, target_address );

		address = (u32)pp->cfg1_base |(where & 0xFFFC);
	} else {
		/* Type0 configuration request */
		PCIE_SETUP_iATU_OB_ENTRY( pp, iATU_ENTRY_CNF0, (u32)pp->cfg0_base,
				((u32)(pp->cfg0_base + (iATU_CFG0_SIZE - 1))),
				(AXI_OP_TYPE_CONFIG_RDRW_TYPE0 & iATU_CTRL1_TYPE_MASK),
				0, target_address );
		address = (u32)pp->cfg0_base |(where & 0xFFFC);
	}


	if (size == 4)
		writel(val, address);
	else if (size == 2)
		writew(val, address + (where & 2));
	else if (size == 1)
		writeb(val, address + (where & 3));
	else
		ret = -1;

	return ret;
}

static int pcie_write_conf(struct pcie_port *pp, int bus_nr, u32 devfn, int where, int size, u32 val)
{
	int ret;

 	/* Make sure that link is up.
	 * Filter device numbers, unless it's a type1 access
	 */
	if ( (!pp->link_state)||
			((bus_nr == pp->root_bus_nr) && (PCI_SLOT(devfn) > 0)) ) {
		return -1;
	}

	BUG_ON (((where & 0x3) + size) > 4);

	ret = comcerto_pcie_wr_conf(pp, bus_nr, devfn, where, size, val);

	return ret;
}

static int pcie_read_subsysid(struct pcie_port *pp, u32 devfn, u32 *val)
{
 unsigned int subid;

	(void)comcerto_pcie_rd_conf (pp, pp->root_bus_nr, devfn, PCI_SUBSYS_ID, 4, val);
        if(*val != 0xffffffff)
        {
		subid = *val;

                printf("PCIe-%d: subSYS_VENDOR: 0x%04X subSYS_ID: 0x%04X !\n",
                pp->root_bus_nr, (subid & 0x0000FFFF), ((subid >> 16) & 0xFFFF));
        }
        else
        {
                printf("PCIe-%d: SubSytem & Vendor IDs failed to access ? \n", pp->root_bus_nr);
                return 1;
        }

	return 0;
}

/**********************************************************************************************/

static void __init pcie_port_init_rc(int port)
{
	struct pcie_port *pp = &pcie_port[port];

#ifdef PCIE_DEBUG_LOOPBACK
	printf("\n*>> %s: start pcie_port-%d base=0x%x \n", __func__, port, (unsigned int)pp->base);

	printf(" >> pcie_port-%d base+cfg0=0x%x \n", port, pp->app_regs->cfg0);
#endif
	writel(readl(pp->app_base + pp->app_regs->cfg0) | 0x08007FF0,
					pp->app_base + pp->app_regs->cfg0);

#ifdef PCIE_DEBUG_LOOPBACK
	printf(" >> pcie_port-%d base+cfg4=0x%x \n", port, pp->app_regs->cfg4);
#endif
	writel(readl(pp->app_base + pp->app_regs->cfg4) | 0x00008000,
					pp->app_base + pp->app_regs->cfg4);

#ifdef PCIE_DEBUG_LOOPBACK
	printf(" >> pcie_port-%d base+PCIE_AFL0L1_REG at 0x70C synopsis dbi access \n", port);
#endif
	writel((readl(pp->base + 0x70C) & ~(0x00ffff00))| 0x00F1F100, pp->base + 0x70C);

#ifdef PCIE_DEBUG_LOOPBACK
	printf(" >> pcie_port-%d base+PCIE_G2CTRL_REG at 0x80C synopsis dbi access \n", port);
#endif
	writel(readl(pp->base + 0x80C) & ~(0xff) | 0xF1,  pp->base + 0x80C);
	writel(readl(pp->base + 0x80C) | (1 << 17), pp->base + 0x80C);

#if 0
	printf(" >> pcie_port-%d: iATU_IB_ENTRY View-0 MEM base=0 DDR *->0x%x mskSize=0x%x \n", 
				port, COMCERTO_AXI_DDR_BASE, INBOUND_ADDR_MASK);
        {
        	PCIE_SETUP_iATU_IB_ENTRY( pp, 0, 
			0, INBOUND_ADDR_MASK,                   /* 0xfff_ffff */
				0, 0, COMCERTO_AXI_DDR_BASE);   /* 0x0000_0000 */
	}

	printf(" >> pcie_port-%d: iATU_OB_ENTRY View-0 MEM base=0x%x *->0x%x mskSize=0x%x \n", 
			port, iATU_GET_MEM_BASE(pp->remote_mem_baseaddr), 
					pp->remote_mem_baseaddr, iATU_MEM_SIZE - 1);
	{
        	PCIE_SETUP_iATU_OB_ENTRY( pp, iATU_ENTRY_MEM, 
			iATU_GET_MEM_BASE(pp->remote_mem_baseaddr), iATU_MEM_SIZE - 1, 
				0, 0, pp->remote_mem_baseaddr );
	}

	printf(" >> pcie_port-%d: iATU_OB_ENTRY View-1 IO base=0x%x *->0x%x mskSize=0x%x \n", 
			port, iATU_GET_IO_BASE(pp->remote_mem_baseaddr), 
					iATU_GET_IO_BASE(pp->remote_mem_baseaddr), iATU_IO_SIZE - 1);
        {
        	PCIE_SETUP_iATU_OB_ENTRY( pp, iATU_ENTRY_IO, 
			iATU_GET_IO_BASE(pp->remote_mem_baseaddr), iATU_IO_SIZE - 1, 
				(AXI_OP_TYPE_IO_RDRW & iATU_CTRL1_TYPE_MASK), 0, 
						iATU_GET_IO_BASE(pp->remote_mem_baseaddr) );
	}

	printf(" >> pcie_port-%d: iATU_MSG_ENTRY View-4 MSG base=0x%x *->0x%x mskSize=0x%x \n", 
			port, iATU_GET_MSG_BASE(pp->remote_mem_baseaddr), 
					iATU_GET_MSG_BASE(pp->remote_mem_baseaddr), iATU_MSG_SIZE - 1);
	{
        	PCIE_SETUP_iATU_OB_ENTRY( pp, iATU_ENTRY_MSG, 
			iATU_GET_MSG_BASE(pp->remote_mem_baseaddr), iATU_MSG_SIZE - 1, 
				(AXI_OP_TYPE_MSG_REQ & iATU_CTRL1_TYPE_MASK), 0, 
						iATU_GET_MSG_BASE(pp->remote_mem_baseaddr) );
	}
#else
        //Register Initialization (MEM_SPACE_EN_Enable + BUS_MASTER_EN_Enable + CAP_LIST_Enable)
        writel(0x00100006,  pp->base + 0x4);

        //Set iATU Registers (Outbound)
        comcerto_dbi_write_reg(pp, PCIE_iATU_VIEW_PORT, 4, port);
        comcerto_dbi_write_reg(pp, PCIE_iATU_SRC_LOW, 4, pcie_remote_base_addr[port]);
        comcerto_dbi_write_reg(pp, PCIE_iATU_LIMIT, 4, pcie_remote_base_addr[port] + 0xFFFF);
        comcerto_dbi_write_reg(pp, PCIE_iATU_TRGT_LOW, 4, pcie_remote_base_addr[port]);
        comcerto_dbi_write_reg(pp, PCIE_iATU_CTRL1, 4, 0x0);
        comcerto_dbi_write_reg(pp, PCIE_iATU_CTRL2, 4, 0x80000000);

        //Set iATU Registers (Inbound))
        comcerto_dbi_write_reg(pp, PCIE_iATU_VIEW_PORT, 4, 0x80000000 + port);
        comcerto_dbi_write_reg(pp, PCIE_iATU_TRGT_LOW, 4, PCIE_LOOP_DEST);
        comcerto_dbi_write_reg(pp, PCIE_iATU_CTRL1, 4, 0x0);
        comcerto_dbi_write_reg(pp, PCIE_iATU_CTRL2, 4, 0xc0000000);

        //Register Initialization (MEM_SPACE_EN_Enable + BUS_MASTER_EN_Enable + CAP_LIST_Enable)
        writel(0x00100006,  pp->base + 0x4);
        printf("%s: MEM_SPACE_EN_Enable + BUS_MASTER_EN_Enable + CAP_LIST_Enable\n",__func__);
        //writel(0xa0000000,  pp->base + 0x10);
        writel(pcie_remote_base_addr[port],  pp->base + 0x10);

#endif
}

static int comcerto_pcie_init( struct pcie_port *pp, int nr)
{
 int ctrl_reg;

	if (nr >= NUM_PCIE_PORTS) {
		printf("%s : Invalid PCIe port number\n", __func__);
		return 1;
	}

	memset(pp, 0, sizeof(struct pcie_port));

	pp->app_regs = &app_regs[nr];
	pp->root_bus_nr = nr;
	pp->base = pcie_cnf_base_addr[nr];
	pp->app_base = (unsigned long)COMCERTO_PCIE_SATA_USB_CTRL_BASE;
	pp->remote_mem_baseaddr = pcie_remote_base_addr[nr];
	pp->cfg0_base = iATU_GET_CFG0_BASE(pp->remote_mem_baseaddr);
	pp->cfg1_base = iATU_GET_CFG1_BASE(pp->remote_mem_baseaddr);

	printf("%s : PCIe-%d remote_mem->0x%08x, cfg0_base->0x%08x cfg1_base->0x%08x \n", 
		__func__, nr, pp->remote_mem_baseaddr, pp->cfg0_base, pp->cfg1_base);

        comcerto_serdes_set_polarity(0);

	if (!nr) {
#ifdef PCIE_DEBUG_LOOPBACK
	printf("%s PCIe-%d: releaseing AXI_RESET_2 & SERDES_RST_CNTRL\n", __func__, nr);
#endif
 
     		writel( readl( AXI_RESET_2 ) & ~PCIE0_AXI_RST, AXI_RESET_2 );
     		writel( readl( SERDES_RST_CNTRL ) & ~(SERDES0_RST), SERDES_RST_CNTRL );

		/* Configure PCIe0 to Root complex mode */
		writel( (readl(pp->app_base + pp->app_regs->cfg0) &
					~DWC_CFG0_DEV_TYPE_MASK) | PCIE_PORT_MODE_RC,
					pp->app_base + pp->app_regs->cfg0);

		/* Enable clocks */
                ctrl_reg = COMCERTO_SERDES_DWC_CFG_REG( 0, SD_PHY_CTRL3_REG_OFST );
#ifdef PCIE_DEBUG_LOOPBACK
                printf("%s: PCIe0 CTRL3_reg->0x%x \n",__func__, ctrl_reg);
#endif
                writel(0xFF3C, ctrl_reg);

                ctrl_reg = COMCERTO_SERDES_DWC_CFG_REG( 1, SD_PHY_CTRL2_REG_OFST );
                writel(readl(ctrl_reg) & ~0x3, ctrl_reg);

		writel( readl( AXI_CLK_CNTRL_2 )| AXI_PCIE0_CLK_EN,  AXI_CLK_CNTRL_2);

                /* Put COMPONENT_SERDES_PCIe0: the PCIe0 SERDES controller in Normal state */
#ifdef PCIE_DEBUG_LOOPBACK
		printf("%s PCIe-0: PCIe_SATA_RST_CNTRL->0x%x, mask=0x0000-0003 \n",
						__func__, PCIe_SATA_RST_CNTRL);
#endif
                writel(readl(PCIe_SATA_RST_CNTRL) & ~SERDES_PCIE0_RESET_BIT, PCIe_SATA_RST_CNTRL);

		pp->port_mode = PCIE_PORT_MODE_RC;

                printf("%s: PCIe-%d Set to PCIE_PORT_MODE_RC \n",__func__, nr);
	}
        else if (nr == (int)1)
	{
#ifdef PCIE_DEBUG_LOOPBACK
        printf("%s PCIe-%d: releasing AXI_RESET_2 & SERDES_RST_CNTRL\n", __func__, nr);
#endif

                writel( readl( AXI_RESET_2 ) & ~PCIE1_AXI_RST, AXI_RESET_2 );
                writel( readl( SERDES_RST_CNTRL ) & ~(SERDES1_RST), SERDES_RST_CNTRL );

                /* Configure PCIe1 to Root complex mode */
                writel( (readl(pp->app_base + pp->app_regs->cfg0) &
                                        ~DWC_CFG0_DEV_TYPE_MASK) | PCIE_PORT_MODE_RC,
                                        pp->app_base + pp->app_regs->cfg0);

		ctrl_reg = COMCERTO_SERDES_DWC_CFG_REG( 1, SD_PHY_CTRL3_REG_OFST );
#ifdef PCIE_DEBUG_LOOPBACK
		printf("%s: PCIe1 CTRL3_reg->0x%x \n",__func__, ctrl_reg);
#endif
		writel(0xFF3C, ctrl_reg);

		ctrl_reg = COMCERTO_SERDES_DWC_CFG_REG( 1, SD_PHY_CTRL2_REG_OFST );
		writel(readl(ctrl_reg) & ~0x3, ctrl_reg);
		writel( readl( AXI_CLK_CNTRL_2 )| AXI_PCIE1_CLK_EN,  AXI_CLK_CNTRL_2);

                /* Put COMPONENT_SERDES_PCIe1: the PCIe1 SERDES controller in Normal state */
#ifdef PCIE_DEBUG_LOOPBACK
		printf("%s PCIe1: PCIe_SATA_RST_CNTRL->0x%x, mask=0x0000-000c \n",
						__func__, PCIe_SATA_RST_CNTRL);
#endif
                writel(readl(PCIe_SATA_RST_CNTRL) & ~SERDES_PCIE1_RESET_BIT, PCIe_SATA_RST_CNTRL);

		pp->port_mode = PCIE_PORT_MODE_RC;

                printf("%s: PCIe-%d Set to PCIE_PORT_MODE_RC \n",__func__, nr);
	}
	else {
		pp->port_mode = PCIE_PORT_MODE_NONE;
		return 1;
	     }

	//Serdes Initialization.
	if( serdes_phy_init(nr,  pcie_phy_reg_file_24,
			sizeof(pcie_phy_reg_file_24) / sizeof(serdes_regs_t),
						SD_DEV_TYPE_PCIE) )
	{
		pp->port_mode = PCIE_PORT_MODE_NONE;
		pr_err("%s: Failed to initialize serdes (%d), set back to MODE_NONE ?\n", __func__, nr );
		return 1;
	}
	printf("%s : PCIe-%d completed serdes_phy_init! \n", __func__, nr);

        mdelay(3);

	comcerto_pcie_LTSon (nr);

	pp->link_state = comcerto_pcie_link_up( &pcie_port[nr] );

        if (pp->link_state)
        {
#ifdef PCIE_DEBUG_LOOPBACK
	        printf("%%%%%%%%%%%% %s : PCIe-%d LINKUP PASS %%%%%%%%%%%%%%\n", __func__, nr);
#endif
        }
        else
	{
		printf("!!!!!!!!!!! %s : PCIe-%d LINKUP FAIL !!!!!!!!!!!!!\n", __func__, nr);
		return -1;
	}

	return 0;
}

u32 data_patterns[] = {0x11111111,0x22222222,0x33333333,0x44444444,0x55555555};


int comcerto_pcie_port_test (unsigned int pcie_host_nr)
{
	u32 value;
        struct pcie_port *pp;
        unsigned int addr = PCI_VENDOR_ID;
        unsigned int devfn = 0;
        int loopback_stat = 0;
	int i;


	if (pcie_host_nr >= NUM_PCIE_PORTS) {
		printf("%s : Invalid port number (%d)\n", __func__, pcie_host_nr);
		return 1;
	}

        value = 0xffffffff;

	printf("\nPCIe%d Root complex test:\n", pcie_host_nr);

	pp = &pcie_port[pcie_host_nr];

		if ( comcerto_pcie_init(pp, pcie_host_nr) ) {
			printf("comcerto_pcie_init failed\n");
			return 1;
                }

/***
	if (!initialized[pcie_host_nr])
	{
		if ( comcerto_pcie_init(pp, pcie_host_nr) ) {
			printf("comcerto_pcie_init failed\n");
			return 1;
		}

		initialized[pcie_host_nr] = 1;
	}
***/


	if ( pp->port_mode == PCIE_PORT_MODE_NONE ) {
		printf("PCIe%d controller is not enabled.........!!!\n", pcie_host_nr);
		return 1;
	}

#if 0
	printf(">>> PCIe-%d RComplex is enabled to read Device, devfn=%d  ... \n", pcie_host_nr, devfn);

	if ( ( pp->port_mode == PCIE_PORT_MODE_RC ) && ( pp->link_state ) )
	{
		pcie_read_conf(pp, pcie_host_nr, devfn, addr, 0x4, &value);

		if(value != 0xffffffff)
               	{
                       	printf("PCIe-%d: Device Vendor_ID: 0x%04X Device_ID: 0x%04X, PASSed! \n", 
				pcie_host_nr, (value & 0xFFFF), ((value >> 16) & 0xFFFF));

               	}
		else
		{
                       	printf("PCIe Lane %d: Vendor Device Id not found for the device...TEST FAIL\n", 
					pcie_host_nr);
			return 1;
               	}

		unsigned int subsys_id;
                int retcode = pcie_read_subsysid (pp, 0, &subsys_id);

		if(retcode)
		{
                       	printf("PCIe-%d: SubSytem & Vendor IDs failed to access ? \n", pcie_host_nr);
			return 1;
               	}
       	}
	else
	{
		printf("PCIe-%d: RComplex and LINKDOWN !... TEST FAIL\n", pcie_host_nr);
		return 1;
       	}
#else
        //////Test the Loopback////////

        //set the dest addr to 0
        for (i = 0 ; i < ARRAY_SIZE(data_patterns) ; i++)
        {
                writel(0, PCIE_LOOP_DEST + i * 4);
        }

        //write the data to the PCIe mem space
        for (i = 0 ; i < ARRAY_SIZE(data_patterns) ; i++)
        {
                writel(data_patterns[i], pp->remote_mem_baseaddr + i * 4);
        }

        //Verify the data in dest addr if it gets loopbacked
        for (i = 0 ; i < ARRAY_SIZE(data_patterns) ; i++)
        {
                value = readl(PCIE_LOOP_DEST + i * 4);
                if(value != data_patterns[i])
                {
                        loopback_stat = -1;
                        printf("Expected value at addr 0x%x: 0x%x, but got value 0x%x\n",PCIE_LOOP_DEST + i * 4, data_patterns[i], value);
                }
        }

	printf("PCIE%d Loopback Test Result : ", pcie_host_nr);
        if(loopback_stat == -1)
                printf("Loopback Failed\n");
        else
                printf("Loopback Passed\n");

#endif


	return 0;
}

static int wEnable = 0;

int comcerto_pcie_rc_test (struct diags_test_param *p)
{
	int rc0, rc1;

     wEnable = 1;

     pcie0_reset (wEnable);
     pcie1_reset (wEnable);

     printf("\n>>> PCIe toggle Reset <<<\n");

     pcie0_reset ((int)0);
     pcie1_reset ((int)0);

     printf("\n>>> PCIe Normal & Wifi Enable = %d <<<\n", wEnable);
     pcie_wEnable (wEnable);

	rc0 = comcerto_pcie_port_test (0);
	rc1 = comcerto_pcie_port_test (1);

	return rc0 | rc1;
}

/*********************************************************************************/

int comcerto_pcie_ep_test (struct diags_test_param *p)
{
        printf("\n PCIE endpoint diags is not yet supported ....\n");
        return 0;
}

int comcerto_pcie_perf_test (struct diags_test_param *p)
{
        printf("\n PCIE perf diags is not yet supported ....\n");
        return 0;
}
