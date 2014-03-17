#include <common.h>
#include <init.h>
#include <asm/io.h>
#include <asm/types.h>
#include <mach/clkcore.h>
#include <mach/ddr.h>

void SoC_DDR_init(void)
{
	int i;
        volatile u32 delay_count;
	struct ddr_config dc;

	dc = get_ddr_config();

	/* If we did not get a proper DDRC configuration, return. */
	if (!dc.ddr_phy_cfg) return;

	writel(0x1, DDR_RESET); /* DDR controller out of reset and PHY is put into reset */

	for(i = 0; dc.ddr_phy_cfg[i].reg; i++)
	{
		writel(dc.ddr_phy_cfg[i].val, dc.ddr_phy_cfg[i].reg);

		//wait for few us. VLSI has 5 NOPs here.
		delay_count = 0x1;
		while (delay_count--);
	}

	writel(0, DDR_RESET); /* DDR PHY out of reset */

	for(i = 0; dc.ddrc_cfg[i].reg; i++)
	{
		writel(dc.ddrc_cfg[i].val, dc.ddrc_cfg[i].reg);
	}

	/* start the DDR Memory Controller */
	writel((MC_START | readl(DDRC_CTL_00_REG)), DDRC_CTL_00_REG);

	/* Wait for the Memory Controller to complete initialization */
	while (!(readl(DDRC_CTL_51_REG) & MC_INIT_STAT_MASK))
		;
}
