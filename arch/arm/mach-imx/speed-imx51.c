#include <common.h>
#include <asm/io.h>
#include <asm-generic/div64.h>
#include <mach/imx51-regs.h>
#include "mach/clock-imx51.h"

static u32 ccm_readl(u32 ofs)
{
	return readl(MX51_CCM_BASE_ADDR + ofs);
}

static unsigned long ckil_get_rate(void)
{
	return 32768;
}

static unsigned long osc_get_rate(void)
{
	return 24000000;
}

static unsigned long fpm_get_rate(void)
{
	return ckil_get_rate() * 512;
}

static unsigned long pll_get_rate(void __iomem *pllbase)
{
	long mfi, mfn, mfd, pdf, ref_clk, mfn_abs;
	unsigned long dp_op, dp_mfd, dp_mfn, dp_ctl, pll_hfsm, dbl;
	u64 temp;
	unsigned long parent_rate;

	dp_ctl = readl(pllbase + MX51_PLL_DP_CTL);

	if ((dp_ctl & MX51_PLL_DP_CTL_REF_CLK_SEL_MASK) == 0)
		parent_rate = fpm_get_rate();
	else
		parent_rate = osc_get_rate();

	pll_hfsm = dp_ctl & MX51_PLL_DP_CTL_HFSM;
	dbl = dp_ctl & MX51_PLL_DP_CTL_DPDCK0_2_EN;

	if (pll_hfsm == 0) {
		dp_op = readl(pllbase + MX51_PLL_DP_OP);
		dp_mfd = readl(pllbase + MX51_PLL_DP_MFD);
		dp_mfn = readl(pllbase + MX51_PLL_DP_MFN);
	} else {
		dp_op = readl(pllbase + MX51_PLL_DP_HFS_OP);
		dp_mfd = readl(pllbase + MX51_PLL_DP_HFS_MFD);
		dp_mfn = readl(pllbase + MX51_PLL_DP_HFS_MFN);
	}
	pdf = dp_op & MX51_PLL_DP_OP_PDF_MASK;
	mfi = (dp_op & MX51_PLL_DP_OP_MFI_MASK) >> MX51_PLL_DP_OP_MFI_OFFSET;
	mfi = (mfi <= 5) ? 5 : mfi;
	mfd = dp_mfd & MX51_PLL_DP_MFD_MASK;
	mfn = mfn_abs = dp_mfn & MX51_PLL_DP_MFN_MASK;
	/* Sign extend to 32-bits */
	if (mfn >= 0x04000000) {
		mfn |= 0xFC000000;
		mfn_abs = -mfn;
	}

	ref_clk = 2 * parent_rate;
	if (dbl != 0)
		ref_clk *= 2;

	ref_clk /= (pdf + 1);
	temp = (u64)ref_clk * mfn_abs;
	do_div(temp, mfd + 1);
	if (mfn < 0)
		temp = -temp;
	temp = (ref_clk * mfi) + temp;

	return temp;
}

static unsigned long pll1_main_get_rate(void)
{
	return pll_get_rate((void __iomem *)MX51_PLL1_BASE_ADDR);
}

static unsigned long pll2_sw_get_rate(void)
{
	return pll_get_rate((void __iomem *)MX51_PLL2_BASE_ADDR);
}

static unsigned long pll3_sw_get_rate(void)
{
	return pll_get_rate((void __iomem *)MX51_PLL3_BASE_ADDR);
}

static unsigned long get_rate_select(int select,
	unsigned long (* get_rate1)(void),
	unsigned long (* get_rate2)(void),
	unsigned long (* get_rate3)(void),
	unsigned long (* get_rate4)(void))
{
	switch (select) {
	case 0:
		return get_rate1() ? get_rate1() : 0;
	case 1:
		return get_rate2() ? get_rate2() : 0;
	case 2:
		return get_rate3 ? get_rate3() : 0;
	case 3:
		return get_rate4 ? get_rate4() : 0;
	}

	return 0;
}

unsigned long imx_get_uartclk(void)
{
	u32 reg, prediv, podf;
	unsigned long parent_rate;

	parent_rate = pll2_sw_get_rate();

	reg = ccm_readl(MX51_CCM_CSCDR1);
	prediv = ((reg & MX51_CCM_CSCDR1_UART_CLK_PRED_MASK) >>
		  MX51_CCM_CSCDR1_UART_CLK_PRED_OFFSET) + 1;
	podf = ((reg & MX51_CCM_CSCDR1_UART_CLK_PODF_MASK) >>
		MX51_CCM_CSCDR1_UART_CLK_PODF_OFFSET) + 1;

	return parent_rate / (prediv * podf);
}

static unsigned long imx_get_ahbclk(void)
{
	u32 reg, div;

	reg = ccm_readl(MX51_CCM_CBCDR);
	div = ((reg >> 10) & 0x7) + 1;

	return pll2_sw_get_rate() / div;
}

unsigned long imx_get_ipgclk(void)
{
	u32 reg, div;

	reg = ccm_readl(MX51_CCM_CBCDR);
	div = ((reg >> 8) & 0x3) + 1;

	return imx_get_ahbclk() / div;
}

unsigned long imx_get_gptclk(void)
{
	return imx_get_ipgclk();
}

unsigned long imx_get_fecclk(void)
{
	return imx_get_ipgclk();
}

unsigned long imx_get_mmcclk(void)
{
	u32 reg, prediv, podf, rate;

	reg = ccm_readl(MX51_CCM_CSCMR1);
	reg &= MX51_CCM_CSCMR1_ESDHC1_MSHC1_CLK_SEL_MASK;
	reg >>= MX51_CCM_CSCMR1_ESDHC1_MSHC1_CLK_SEL_OFFSET;
	rate = get_rate_select(reg,
			pll1_main_get_rate,
			pll2_sw_get_rate,
			pll3_sw_get_rate,
			NULL);

	reg = ccm_readl(MX51_CCM_CSCDR1);
	prediv = ((reg & MX51_CCM_CSCDR1_ESDHC1_MSHC1_CLK_PRED_MASK) >>
			MX51_CCM_CSCDR1_ESDHC1_MSHC1_CLK_PRED_OFFSET) + 1;
	podf = ((reg & MX51_CCM_CSCDR1_ESDHC1_MSHC1_CLK_PODF_MASK) >>
			MX51_CCM_CSCDR1_ESDHC1_MSHC1_CLK_PODF_OFFSET) + 1;

	return rate / (prediv * podf);
}

void imx_dump_clocks(void)
{
	printf("pll1: %ld\n", pll1_main_get_rate());
	printf("pll2: %ld\n", pll2_sw_get_rate());
	printf("pll3: %ld\n", pll3_sw_get_rate());
	printf("uart: %ld\n", imx_get_uartclk());
	printf("ipg:  %ld\n", imx_get_ipgclk());
	printf("fec:  %ld\n", imx_get_fecclk());
	printf("gpt:  %ld\n", imx_get_gptclk());
}
