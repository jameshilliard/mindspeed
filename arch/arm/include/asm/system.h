#ifndef __ASM_ARM_SYSTEM_H
#define __ASM_ARM_SYSTEM_H

#if __LINUX_ARM_ARCH__ >= 7
#define isb() __asm__ __volatile__ ("isb" : : : "memory")
#define dsb() __asm__ __volatile__ ("dsb" : : : "memory")
#define dmb() __asm__ __volatile__ ("dmb" : : : "memory")
#elif defined(CONFIG_CPU_XSC3) || __LINUX_ARM_ARCH__ == 6
#define isb() __asm__ __volatile__ ("mcr p15, 0, %0, c7, c5, 4" \
                                    : : "r" (0) : "memory")
#define dsb() __asm__ __volatile__ ("mcr p15, 0, %0, c7, c10, 4" \
                                    : : "r" (0) : "memory")
#define dmb() __asm__ __volatile__ ("mcr p15, 0, %0, c7, c10, 5" \
                                    : : "r" (0) : "memory")
#elif defined(CONFIG_CPU_FA526)
#define isb() __asm__ __volatile__ ("mcr p15, 0, %0, c7, c5, 4" \
                                    : : "r" (0) : "memory")
#define dsb() __asm__ __volatile__ ("mcr p15, 0, %0, c7, c10, 4" \
                                    : : "r" (0) : "memory")
#define dmb() __asm__ __volatile__ ("" : : : "memory")
#else
#define isb() __asm__ __volatile__ ("" : : : "memory")
#define dsb() __asm__ __volatile__ ("mcr p15, 0, %0, c7, c10, 4" \
                                    : : "r" (0) : "memory")
#define dmb() __asm__ __volatile__ ("" : : : "memory")
#endif

/*
 * CR1 bits (CP#15 CR1)
 */
#define CR_M    (1 << 0)	/* MMU enable				*/
#define CR_A    (1 << 1)	/* Alignment abort enable		*/
#define CR_C    (1 << 2)	/* Dcache enable			*/
#define CR_W    (1 << 3)	/* Write buffer enable			*/
#define CR_P    (1 << 4)	/* 32-bit exception handler		*/
#define CR_D    (1 << 5)	/* 32-bit data address range		*/
#define CR_L    (1 << 6)	/* Implementation defined		*/
#define CR_B    (1 << 7)	/* Big endian				*/
#define CR_S    (1 << 8)	/* System MMU protection		*/
#define CR_R    (1 << 9)	/* ROM MMU protection			*/
#define CR_F    (1 << 10)	/* Implementation defined		*/
#define CR_Z    (1 << 11)	/* Implementation defined		*/
#define CR_I    (1 << 12)	/* Icache enable			*/
#define CR_V    (1 << 13)	/* Vectors relocated to 0xffff0000	*/
#define CR_RR   (1 << 14)	/* Round Robin cache replacement	*/
#define CR_L4   (1 << 15)	/* LDR pc can set T bit			*/
#define CR_DT   (1 << 16)
#define CR_IT   (1 << 18)
#define CR_ST   (1 << 19)
#define CR_FI   (1 << 21)	/* Fast interrupt (lower latency mode)	*/
#define CR_U    (1 << 22)	/* Unaligned access operation		*/
#define CR_XP   (1 << 23)	/* Extended page tables			*/
#define CR_VE   (1 << 24)	/* Vectored interrupts			*/
#define CR_EE   (1 << 25)	/* Exception (Big) Endian		*/
#define CR_TRE  (1 << 28)	/* TEX remap enable			*/
#define CR_AFE  (1 << 29)	/* Access flag enable			*/
#define CR_TE   (1 << 30)	/* Thumb exception enable		*/

static inline unsigned int get_cr(void)
{
	unsigned int val;
	asm("mrc p15, 0, %0, c1, c0, 0  @ get CR" : "=r" (val) : : "cc");
	return val;
}

static inline void set_cr(unsigned int val)
{
	asm volatile("mcr p15, 0, %0, c1, c0, 0 @ set CR"
	  : : "r" (val) : "cc");
	isb();
}

#define GEN_CP15_REG_FUNCS(reg, op1, c1, c2, op2) \
static inline unsigned int arm_read_##reg(void) { \
	unsigned int val; \
	__asm__ volatile("mrc p15, " #op1 ", %0, " #c1 "," #c2 "," #op2 : "=r" (val)); \
	return val; \
} \
\
static inline void arm_write_##reg(unsigned int val) { \
	__asm__ volatile("mcr p15, " #op1 ", %0, " #c1 "," #c2 "," #op2 :: "r" (val)); \
	isb(); \
}

/* armv6+ control regs */
GEN_CP15_REG_FUNCS(sctlr, 0, c1, c0, 0);
GEN_CP15_REG_FUNCS(actlr, 0, c1, c0, 1);
GEN_CP15_REG_FUNCS(cpacr, 0, c1, c0, 2);

GEN_CP15_REG_FUNCS(scr, 0, c1, c1, 0);
GEN_CP15_REG_FUNCS(sder, 0, c1, c1, 1);
GEN_CP15_REG_FUNCS(nsacr, 0, c1, c1, 2);
GEN_CP15_REG_FUNCS(vcr, 0, c1, c1, 3);

GEN_CP15_REG_FUNCS(ttbr, 0, c2, c0, 0);
GEN_CP15_REG_FUNCS(ttbr0, 0, c2, c0, 0);
GEN_CP15_REG_FUNCS(ttbr1, 0, c2, c0, 1);
GEN_CP15_REG_FUNCS(ttbcr, 0, c2, c0, 2);
GEN_CP15_REG_FUNCS(dacr, 0, c3, c0, 0);
GEN_CP15_REG_FUNCS(dfsr, 0, c5, c0, 0);
GEN_CP15_REG_FUNCS(ifsr, 0, c5, c0, 1);
GEN_CP15_REG_FUNCS(dfar, 0, c6, c0, 0);
GEN_CP15_REG_FUNCS(wfar, 0, c6, c0, 1);
GEN_CP15_REG_FUNCS(ifar, 0, c6, c0, 2);

GEN_CP15_REG_FUNCS(fcseidr, 0, c13, c0, 0);
GEN_CP15_REG_FUNCS(contextidr, 0, c13, c0, 1);
GEN_CP15_REG_FUNCS(tpidrurw, 0, c13, c0, 2);
GEN_CP15_REG_FUNCS(tpidruro, 0, c13, c0, 3);
GEN_CP15_REG_FUNCS(tpidrprw, 0, c13, c0, 4);

/* armv7+ */
GEN_CP15_REG_FUNCS(midr, 0, c0, c0, 0);
GEN_CP15_REG_FUNCS(mpidr, 0, c0, c0, 5);
GEN_CP15_REG_FUNCS(vbar, 0, c12, c0, 0);
GEN_CP15_REG_FUNCS(mvbar, 0, c12, c0, 1);
GEN_CP15_REG_FUNCS(cbar, 4, c15, c0, 0);

GEN_CP15_REG_FUNCS(ats1cpr, 0, c7, c8, 0);
GEN_CP15_REG_FUNCS(ats1cpw, 0, c7, c8, 1);
GEN_CP15_REG_FUNCS(ats1cur, 0, c7, c8, 2);
GEN_CP15_REG_FUNCS(ats1cuw, 0, c7, c8, 3);
GEN_CP15_REG_FUNCS(ats12nsopr, 0, c7, c8, 4);
GEN_CP15_REG_FUNCS(ats12nsopw, 0, c7, c8, 5);
GEN_CP15_REG_FUNCS(ats12nsour, 0, c7, c8, 6);
GEN_CP15_REG_FUNCS(ats12nsouw, 0, c7, c8, 7);
GEN_CP15_REG_FUNCS(par, 0, c7, c4, 0);

/*
 * This is used to ensure the compiler did actually allocate the register we
 * asked it for some inline assembly sequences.  Apparently we can't trust
 * the compiler from one version to another so a bit of paranoia won't hurt.
 * This string is meant to be concatenated with the inline asm string and
 * will cause compilation to stop on mismatch.
 * (for details, see gcc PR 15089)
 */
#define __asmeq(x, y)  ".ifnc " x "," y " ; .err ; .endif\n\t"

#endif /* __ASM_ARM_SYSTEM_H */
