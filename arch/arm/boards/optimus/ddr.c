#include <common.h>
#include <sizes.h>
#include <asm/io.h>
#include <asm/types.h>
#include <mach/ddr.h>
#include <mach/gpio.h>
#include <board_id.h>

static struct ddr_reg_val optimus_ddrc_cfg[] = {
	{DDRC_CTL_00_REG, 0x20410600LL},
	{DDRC_CTL_02_REG, 0x00000006LL},
	{DDRC_CTL_03_REG, 0x0001A07CLL},
	{DDRC_CTL_04_REG, 0x00041127LL},
	{DDRC_CTL_05_REG, 0x04061200LL},
	{DDRC_CTL_06_REG, 0x141B0404LL},
	{DDRC_CTL_07_REG, 0x08040804LL},
	{DDRC_CTL_08_REG, 0x0092190CLL},
	{DDRC_CTL_09_REG, 0x00000504LL},
	{DDRC_CTL_10_REG, 0x08080101LL},
	{DDRC_CTL_11_REG, 0x00020010LL},
	{DDRC_CTL_12_REG, 0x00011803LL},
	{DDRC_CTL_13_REG, 0x00000009LL},
	{DDRC_CTL_14_REG, 0x00BB0100LL},
	{DDRC_CTL_15_REG, 0x00001034LL},
	{DDRC_CTL_16_REG, 0x000D0004LL},
	{DDRC_CTL_17_REG, 0x00060002LL},
	{DDRC_CTL_18_REG, 0x00C00200LL},
	{DDRC_CTL_19_REG, 0x00000000LL},
	{DDRC_CTL_20_REG, 0x00060600LL},
	{DDRC_CTL_21_REG, 0x00000000LL},
	{DDRC_CTL_22_REG, 0x00000000LL},
	{DDRC_CTL_23_REG, 0x00000000LL},
	{DDRC_CTL_24_REG, 0x00000000LL},
	{DDRC_CTL_25_REG, 0x00000000LL},
	{DDRC_CTL_26_REG, 0x00000000LL},
	{DDRC_CTL_27_REG, 0x00085000LL},
	{DDRC_CTL_28_REG, 0x00080006LL},
	{DDRC_CTL_29_REG, 0x00000000LL},
	{DDRC_CTL_30_REG, 0x00060850LL},
	{DDRC_CTL_31_REG, 0x00000008LL},
	{DDRC_CTL_32_REG, 0x00020000LL},
	{DDRC_CTL_33_REG, 0x00000000LL},
	{DDRC_CTL_43_REG, 0x01400200LL},
	{DDRC_CTL_44_REG, 0x02000040LL},
	{DDRC_CTL_45_REG, 0x01010080LL},
	{DDRC_CTL_46_REG, 0xFF0A0101LL},
	{DDRC_CTL_47_REG, 0x010101FFLL},
	{DDRC_CTL_48_REG, 0x00010001LL},
	{DDRC_CTL_49_REG, 0x000C0100LL},
	{DDRC_CTL_50_REG, 0x00010002LL},
	{DDRC_CTL_52_REG, 0x00000000LL},
	{DDRC_CTL_53_REG, 0x007FFFFFLL},
	{DDRC_CTL_59_REG, 0x01000000LL},
	{DDRC_CTL_60_REG, 0x00020100LL},
	{DDRC_CTL_61_REG, 0x02010202LL},
	{DDRC_CTL_62_REG, 0x02000100LL},
	{DDRC_CTL_63_REG, 0x00000000LL},
	{DDRC_CTL_91_REG, 0xFFFF0000LL},
	{DDRC_CTL_92_REG, 0x00000202LL},
	{DDRC_CTL_93_REG, 0x0101FFFFLL},
	{DDRC_CTL_94_REG, 0x03FFFF00LL},
	{DDRC_CTL_95_REG, 0xFFFF0003LL},
	{DDRC_CTL_96_REG, 0x00000303LL},
	{DDRC_CTL_97_REG, 0x01000400LL},
	{DDRC_CTL_98_REG, 0x00016400LL},
	{DDRC_CTL_99_REG, 0x00000100LL},
	{DDRC_CTL_100_REG, 0x00000001LL},
	{DDRC_CTL_102_REG, 0x00000800LL},
	{DDRC_CTL_103_REG, 0x00103300LL},
	{DDRC_CTL_104_REG, 0x02000200LL},
	{DDRC_CTL_105_REG, 0x02000200LL},
	{DDRC_CTL_106_REG, 0x00001033LL},
	{DDRC_CTL_107_REG, 0x000050FFLL},
	{DDRC_CTL_108_REG, 0x00020610LL},
	{DDRC_CTL_109_REG, 0x00000003LL},
	{DDRC_CTL_126_REG, 0x00000000LL},
	{0, 0}
};

static struct ddr_reg_val spacecast_ddrc_cfg[] = {
	{DDRC_CTL_00_REG, 0x20410600LL},
	{DDRC_CTL_02_REG, 0x00000006LL},
	{DDRC_CTL_03_REG, 0x0001A07CLL},
	{DDRC_CTL_04_REG, 0x00041127LL},
	{DDRC_CTL_05_REG, 0x04061200LL},
	{DDRC_CTL_06_REG, 0x141B0404LL},
	{DDRC_CTL_07_REG, 0x08040804LL},
	{DDRC_CTL_08_REG, 0x0092190CLL},
	{DDRC_CTL_09_REG, 0x00000504LL},
	{DDRC_CTL_10_REG, 0x08080101LL},
	{DDRC_CTL_11_REG, 0x00020010LL},
	{DDRC_CTL_12_REG, 0x00011803LL},
	{DDRC_CTL_13_REG, 0x00000009LL},
	{DDRC_CTL_14_REG, 0x00BB0100LL},
	{DDRC_CTL_15_REG, 0x00001034LL},
	{DDRC_CTL_16_REG, 0x000D0004LL},
	{DDRC_CTL_17_REG, 0x00060002LL},
	{DDRC_CTL_18_REG, 0x00C00200LL},
	{DDRC_CTL_19_REG, 0x00000000LL},
	{DDRC_CTL_20_REG, 0x00060600LL},
	{DDRC_CTL_21_REG, 0x00000000LL},
	{DDRC_CTL_22_REG, 0x00000000LL},
	{DDRC_CTL_23_REG, 0x00000000LL},
	{DDRC_CTL_24_REG, 0x00000000LL},
	{DDRC_CTL_25_REG, 0x00000000LL},
	{DDRC_CTL_26_REG, 0x00000000LL},
	{DDRC_CTL_27_REG, 0x00085000LL},
	{DDRC_CTL_28_REG, 0x00080006LL},
	{DDRC_CTL_29_REG, 0x00000000LL},
	{DDRC_CTL_30_REG, 0x00060850LL},
	{DDRC_CTL_31_REG, 0x00000008LL},
	{DDRC_CTL_32_REG, 0x00020000LL},
	{DDRC_CTL_33_REG, 0x00000000LL},
	{DDRC_CTL_43_REG, 0x01400200LL},
	{DDRC_CTL_44_REG, 0x02000040LL},
	{DDRC_CTL_45_REG, 0x01010080LL},
	{DDRC_CTL_46_REG, 0xFF0A0101LL},
	{DDRC_CTL_47_REG, 0x010101FFLL},
	{DDRC_CTL_48_REG, 0x00010001LL},
	{DDRC_CTL_49_REG, 0x000C0100LL},
	{DDRC_CTL_50_REG, 0x00010002LL},
	{DDRC_CTL_52_REG, 0x00000000LL},
	{DDRC_CTL_53_REG, 0x007FFFFFLL},
	{DDRC_CTL_59_REG, 0x01000000LL},
	{DDRC_CTL_60_REG, 0x00020100LL},
	{DDRC_CTL_61_REG, 0x02010202LL},
	{DDRC_CTL_62_REG, 0x02000100LL},
	{DDRC_CTL_63_REG, 0x00000000LL},
	{DDRC_CTL_91_REG, 0xFFFF0000LL},
	{DDRC_CTL_92_REG, 0x00000202LL},
	{DDRC_CTL_93_REG, 0x0303FFFFLL},
	{DDRC_CTL_94_REG, 0x03FFFF00LL},
	{DDRC_CTL_95_REG, 0xFFFF0003LL},
	{DDRC_CTL_96_REG, 0x00000101LL},
	{DDRC_CTL_97_REG, 0x01000400LL},
	{DDRC_CTL_98_REG, 0x00012100LL},
	{DDRC_CTL_99_REG, 0x0500011aLL},
	{DDRC_CTL_100_REG, 0x00000001LL},
	{DDRC_CTL_102_REG, 0x00000800LL},
	{DDRC_CTL_103_REG, 0x00103300LL},
	{DDRC_CTL_104_REG, 0x02000200LL},
	{DDRC_CTL_105_REG, 0x02000200LL},
	{DDRC_CTL_106_REG, 0x00001033LL},
	{DDRC_CTL_107_REG, 0x000050FFLL},
	{DDRC_CTL_108_REG, 0x00020610LL},
	{DDRC_CTL_109_REG, 0x00000003LL},
	{DDRC_CTL_126_REG, 0x00000000LL},
	{0, 0}
};

/* This array is called optimus_ddr_phy_cfg for historic reasons. */
/* It applies to both Optimus and SpaceCast. */
static struct ddr_reg_val optimus_ddr_phy_cfg[] = {
	{DDR_PHY_CTL_00_REG, 0x000F1023LL},
	{DDR_PHY_CTL_01_REG, 0x18201010LL},
	{DDR_PHY_CTL_02_REG, 0x00000006LL},
	{DDR_PHY_CTL_03_REG, 0x09090909LL},
	{DDR_PHY_CTL_04_REG, 0x00000009LL},
	{DDR_PHY_CTL_05_REG, 0x00000000LL},
	{DDR_PHY_CTL_06_REG, 0x04300623LL},
	{DDR_PHY_CTL_07_REG, 0x00000000LL},
	{0, 0}
};

struct ddr_reg_val sideswipe_ddrc_cfg[] = {
	/* DDR3 Controller @533MHz */
	{DDRC_CTL_00_REG, 0x20410600LL},
	{DDRC_CTL_02_REG, 0x00000006LL},
	{DDRC_CTL_03_REG, 0x0001A07CLL},
	{DDRC_CTL_04_REG, 0x00041127LL},
	{DDRC_CTL_05_REG, 0x04060E00LL},
	{DDRC_CTL_06_REG, 0x141B0604LL},
	{DDRC_CTL_07_REG, 0x08040704LL},
	{DDRC_CTL_08_REG, 0x0092190CLL},
	{DDRC_CTL_09_REG, 0x00000504LL},
	{DDRC_CTL_10_REG, 0x08070101LL},
	{DDRC_CTL_11_REG, 0x0002000FLL},
	{DDRC_CTL_12_REG, 0x00011B03LL},
	{DDRC_CTL_13_REG, 0x00000008LL},
	{DDRC_CTL_14_REG, 0x00560100LL},
	{DDRC_CTL_15_REG, 0x00001034LL},
	{DDRC_CTL_16_REG, 0x000D0004LL},
	{DDRC_CTL_17_REG, 0x00060002LL},
	{DDRC_CTL_18_REG, 0x005B0200LL},
	{DDRC_CTL_19_REG, 0x00000000LL},
	{DDRC_CTL_20_REG, 0x00060600LL},
	{DDRC_CTL_21_REG, 0x00000000LL},
	{DDRC_CTL_22_REG, 0x00000000LL},
	{DDRC_CTL_23_REG, 0x00000000LL},
	{DDRC_CTL_24_REG, 0x00000000LL},
	{DDRC_CTL_25_REG, 0x00000000LL},
	{DDRC_CTL_26_REG, 0x00000000LL},
	{DDRC_CTL_27_REG, 0x00083000LL},
	{DDRC_CTL_28_REG, 0x00080006LL},
	{DDRC_CTL_29_REG, 0x00000000LL},
	{DDRC_CTL_30_REG, 0x00060830LL},
	{DDRC_CTL_31_REG, 0x00000008LL},
	{DDRC_CTL_32_REG, 0x00020000LL},
	{DDRC_CTL_33_REG, 0x00000000LL},
	{DDRC_CTL_43_REG, 0x01400200LL},
	{DDRC_CTL_44_REG, 0x02000040LL},
	{DDRC_CTL_45_REG, 0x01010080LL},
	{DDRC_CTL_46_REG, 0xFF0A0102LL},
	{DDRC_CTL_47_REG, 0x010101FFLL},
	{DDRC_CTL_48_REG, 0x00010001LL},
	{DDRC_CTL_49_REG, 0x000C0100LL},
	{DDRC_CTL_50_REG, 0x00010002LL},
	{DDRC_CTL_52_REG, 0x00000000LL},
	{DDRC_CTL_53_REG, 0x007FFFFFLL},
	{DDRC_CTL_59_REG, 0x01000000LL},
	{DDRC_CTL_60_REG, 0x00020100LL},
	{DDRC_CTL_61_REG, 0x02010202LL},
	{DDRC_CTL_62_REG, 0x02000101LL},
	{DDRC_CTL_63_REG, 0x00000000LL},
	{DDRC_CTL_91_REG, 0xFFFF0000LL},
	{DDRC_CTL_92_REG, 0x00000202LL},
	{DDRC_CTL_93_REG, 0x0101FFFFLL},
	{DDRC_CTL_94_REG, 0x03FFFF00LL},
	{DDRC_CTL_95_REG, 0xFFFF0003LL},
	{DDRC_CTL_96_REG, 0x00000303LL},
	{DDRC_CTL_97_REG, 0x01000400LL},
	{DDRC_CTL_98_REG, 0x00016400LL},
	{DDRC_CTL_99_REG, 0x00000100LL},
	{DDRC_CTL_100_REG, 0x00000001LL},
	{DDRC_CTL_102_REG, 0x00000800LL},
	{DDRC_CTL_103_REG, 0x00103300LL},
	{DDRC_CTL_104_REG, 0x02000200LL},
	{DDRC_CTL_105_REG, 0x02000200LL},
	{DDRC_CTL_106_REG, 0x00001033LL},
	{DDRC_CTL_107_REG, 0x000050FFLL},
	{DDRC_CTL_108_REG, 0x0002060CLL},
	{DDRC_CTL_109_REG, 0x00000003LL},
	{DDRC_CTL_126_REG, 0x00000000LL},
	{0, 0}
};

struct ddr_reg_val sideswipe_ddr_phy_cfg[] = {
	/* DDR3 PHY @533 */
	{DDR_PHY_CTL_00_REG, 0x000F1023LL},
	{DDR_PHY_CTL_01_REG, 0x18201010LL},
	{DDR_PHY_CTL_02_REG, 0x00000006LL},
	{DDR_PHY_CTL_03_REG, 0x09090909LL},
	{DDR_PHY_CTL_04_REG, 0x00000009LL},
	{DDR_PHY_CTL_05_REG, 0x00000000LL},
	{DDR_PHY_CTL_06_REG, 0x04300623LL},
	{DDR_PHY_CTL_07_REG, 0x00000000LL},
	{0, 0}
};

static struct ddr_config board_id_to_ddr_config[] = {
	{optimus_ddr_phy_cfg, optimus_ddrc_cfg, SZ_1G, "Optimus"},
	/* Nanya NT5CB128M16FP-DI, 533MHz, 32bit, NoECC */
	{sideswipe_ddr_phy_cfg, sideswipe_ddrc_cfg, SZ_512M, "Sideswipe"},
	{optimus_ddr_phy_cfg, spacecast_ddrc_cfg, SZ_1G, "SpaceCast"},
	{0, 0, 0, "Unknown 3"},
	{optimus_ddr_phy_cfg, optimus_ddrc_cfg, SZ_1G, "Optimus Prime"},
	{sideswipe_ddr_phy_cfg, sideswipe_ddrc_cfg, SZ_512M, "Sideswipe Prime"},
};

static struct ddr_config bad_board_id_ddr_config = {0, 0, 0, "Unknown"};

const char *get_ddr_config_description(void) {
	return get_ddr_config().description;
}

resource_size_t get_ddr_config_size(void) {
	return get_ddr_config().size;
}

struct ddr_config get_ddr_config(void) {
	int board_id = get_board_id_gpio();
	if (board_id < ARRAY_SIZE(board_id_to_ddr_config))
		return board_id_to_ddr_config[board_id];
	else
		return bad_board_id_ddr_config;
}
