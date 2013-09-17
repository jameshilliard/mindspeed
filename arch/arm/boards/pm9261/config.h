#ifndef __CONFIG_H
#define __CONFIG_H

#define AT91_MAIN_CLOCK		18432000	/* 18.432 MHz crystal */

#define MASTER_PLL_DIV		15
#define MASTER_PLL_MUL		162
#define MAIN_PLL_DIV		2

/* clocks */
#define CONFIG_SYS_MOR_VAL						\
		(AT91_PMC_MOSCEN |					\
		 (255 << 8))		/* Main Oscillator Start-up Time */
#define CONFIG_SYS_PLLAR_VAL						\
		(AT91_PMC_PLLA_WR_ERRATA | /* Bit 29 must be 1 when prog */ \
		 AT91_PMC_OUT |						\
		 ((MASTER_PLL_MUL - 1) << 16) | (MASTER_PLL_DIV))

/* PCK/2 = MCK Master Clock from PLLA */
#define	CONFIG_SYS_MCKR1_VAL		\
		(AT91_PMC_CSS_SLOW |	\
		 AT91_PMC_PRES_1 |	\
		 AT91SAM9_PMC_MDIV_2 |	\
		 AT91_PMC_PDIV_1)

/* PCK/2 = MCK Master Clock from PLLA */
#define	CONFIG_SYS_MCKR2_VAL		\
		(AT91_PMC_CSS_PLLA |	\
		 AT91_PMC_PRES_1 |	\
		 AT91SAM9_PMC_MDIV_2 |	\
		 AT91_PMC_PDIV_1)

/* define PDC[31:16] as DATA[31:16] */
#define CONFIG_SYS_PIOC_PDR_VAL1	0xFFFF0000
/* no pull-up for D[31:16] */
#define CONFIG_SYS_PIOC_PPUDR_VAL	0xFFFF0000

/* EBI_CSA, no pull-ups for D[15:0], CS1 SDRAM, CS3 NAND Flash */
#define CONFIG_SYS_MATRIX_EBICSA_VAL		\
       (AT91_MATRIX_DBPUC | AT91_MATRIX_CS1A_SDRAMC)

/* SDRAM */
/* SDRAMC_MR Mode register */
#define CONFIG_SYS_SDRC_MR_VAL1		AT91_SDRAMC_MODE_NORMAL
/* SDRAMC_TR - Refresh Timer register */
#define CONFIG_SYS_SDRC_TR_VAL1		0x13C
/* SDRAMC_CR - Configuration register*/
#define CONFIG_SYS_SDRC_CR_VAL							\
		(AT91_SDRAMC_NC_9 |						\
		 AT91_SDRAMC_NR_13 |						\
		 AT91_SDRAMC_NB_4 |						\
		 AT91_SDRAMC_CAS_3 |						\
		 AT91_SDRAMC_DBW_32 |						\
		 (1 <<  8) |		/* Write Recovery Delay */		\
		 (7 << 12) |		/* Row Cycle Delay */			\
		 (3 << 16) |		/* Row Precharge Delay */		\
		 (2 << 20) |		/* Row to Column Delay */		\
		 (5 << 24) |		/* Active to Precharge Delay */		\
		 (1 << 28))		/* Exit Self Refresh to Active Delay */

/* Memory Device Register -> SDRAM */
#define CONFIG_SYS_SDRC_MDR_VAL		AT91_SDRAMC_MD_SDRAM
#define CONFIG_SYS_SDRC_MR_VAL2		AT91_SDRAMC_MODE_PRECHARGE
#define CONFIG_SYS_SDRAM_VAL1		0		/* SDRAM_BASE */
#define CONFIG_SYS_SDRC_MR_VAL3		AT91_SDRAMC_MODE_REFRESH
#define CONFIG_SYS_SDRAM_VAL2		0		/* SDRAM_BASE */
#define CONFIG_SYS_SDRAM_VAL3		0		/* SDRAM_BASE */
#define CONFIG_SYS_SDRAM_VAL4		0		/* SDRAM_BASE */
#define CONFIG_SYS_SDRAM_VAL5		0		/* SDRAM_BASE */
#define CONFIG_SYS_SDRAM_VAL6		0		/* SDRAM_BASE */
#define CONFIG_SYS_SDRAM_VAL7		0		/* SDRAM_BASE */
#define CONFIG_SYS_SDRAM_VAL8		0		/* SDRAM_BASE */
#define CONFIG_SYS_SDRAM_VAL9		0		/* SDRAM_BASE */
#define CONFIG_SYS_SDRC_MR_VAL4		AT91_SDRAMC_MODE_LMR
#define CONFIG_SYS_SDRAM_VAL10		0		/* SDRAM_BASE */
#define CONFIG_SYS_SDRC_MR_VAL5		AT91_SDRAMC_MODE_NORMAL
#define CONFIG_SYS_SDRAM_VAL11		0		/* SDRAM_BASE */
#define CONFIG_SYS_SDRC_TR_VAL2		1200		/* SDRAM_TR */
#define CONFIG_SYS_SDRAM_VAL12		0		/* SDRAM_BASE */

/* setup SMC0, CS0 (NOR Flash) - 16-bit, 15 WS */
#define CONFIG_SYS_SMC0_SETUP0_VAL					\
		(AT91_SMC_NWESETUP_(10) | AT91_SMC_NCS_WRSETUP_(10) |	\
		 AT91_SMC_NRDSETUP_(10) | AT91_SMC_NCS_RDSETUP_(10))
#define CONFIG_SYS_SMC0_PULSE0_VAL					\
		(AT91_SMC_NWEPULSE_(11) | AT91_SMC_NCS_WRPULSE_(11) |	\
		 AT91_SMC_NRDPULSE_(11) | AT91_SMC_NCS_RDPULSE_(11))
#define CONFIG_SYS_SMC0_CYCLE0_VAL	\
		(AT91_SMC_NWECYCLE_(22) | AT91_SMC_NRDCYCLE_(22))
#define CONFIG_SYS_SMC0_MODE0_VAL				\
		(AT91_SMC_READMODE | AT91_SMC_WRITEMODE |	\
		 AT91_SMC_DBW_16 |				\
		 AT91_SMC_TDFMODE |				\
		 AT91_SMC_TDF_(6))

/* user reset enable */
#define CONFIG_SYS_RSTC_RMR_VAL			\
		(AT91_RSTC_KEY |		\
		AT91_RSTC_PROCRST |		\
		AT91_RSTC_RSTTYP_WAKEUP |	\
		AT91_RSTC_RSTTYP_WATCHDOG)

/* Disable Watchdog */
#define CONFIG_SYS_WDTC_WDMR_VAL				\
		(AT91_WDT_WDIDLEHLT | AT91_WDT_WDDBGHLT |	\
		 AT91_WDT_WDV |					\
		 AT91_WDT_WDDIS |				\
		 AT91_WDT_WDD)

#endif	/* __CONFIG_H */
