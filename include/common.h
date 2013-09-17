/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __COMMON_H_
#define __COMMON_H_	1

#include <stdio.h>
#include <module.h>
#include <config.h>
#include <linux/bitops.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/stddef.h>
#include <asm/common.h>

#define pr_info(fmt, arg...)	printf(fmt, ##arg)
#define pr_notice(fmt, arg...)	printf(fmt, ##arg)
#define pr_err(fmt, arg...)	printf(fmt, ##arg)
#define pr_warning(fmt, arg...)	printf(fmt, ##arg)
#define pr_crit(fmt, arg...)	printf(fmt, ##arg)
#define pr_alert(fmt, arg...)	printf(fmt, ##arg)
#define pr_emerg(fmt, arg...)	printf(fmt, ##arg)

#ifdef DEBUG
#define pr_debug(fmt, arg...)	printf(fmt, ##arg)
#else
#define pr_debug(fmt, arg...)	do {} while(0)
#endif

#define debug(fmt, arg...)	pr_debug(fmt, ##arg)

#define BUG() do { \
	printf("BUG: failure at %s:%d/%s()!\n", __FILE__, __LINE__, __FUNCTION__); \
	panic("BUG!"); \
} while (0)
#define BUG_ON(condition) do { if (unlikely((condition)!=0)) BUG(); } while(0)


#define __WARN() do { 								\
	printf("WARNING: at %s:%d/%s()!\n", __FILE__, __LINE__, __FUNCTION__);	\
} while (0)

#ifndef WARN_ON
#define WARN_ON(condition) ({						\
	int __ret_warn_on = !!(condition);				\
	if (unlikely(__ret_warn_on))					\
		__WARN();						\
	unlikely(__ret_warn_on);					\
})
#endif

#ifndef WARN
#define WARN(condition, format...) ({					\
	int __ret_warn_on = !!(condition);				\
	if (unlikely(__ret_warn_on))					\
		__WARN();						\
		puts("WARNING: ");					\
		printf(format);						\
	unlikely(__ret_warn_on);					\
})
#endif

typedef void (interrupt_handler_t)(void *);

#include <asm/barebox.h> /* boot information for Linux kernel */

/*
 * Function Prototypes
 */
void reginfo(void);

void __noreturn hang (void);
void __noreturn panic(const char *fmt, ...);

/* */
long int initdram (int);
char *size_human_readable(ulong size);

/* common/main.c */
int	run_command	(const char *cmd, int flag);
int	readline	(const char *prompt, char *buf, int len);

/* common/memsize.c */
long	get_ram_size  (volatile long *, long);

/* $(CPU)/cpu.c */
void __noreturn reset_cpu(unsigned long addr);

/* $(CPU)/interrupts.c */
//void	timer_interrupt	   (struct pt_regs *);
//void	external_interrupt (struct pt_regs *);
void	irq_install_handler(int, interrupt_handler_t *, void *);
void	irq_free_handler   (int);
#ifdef CONFIG_USE_IRQ
void	enable_interrupts  (void);
int	disable_interrupts (void);
#else
#define enable_interrupts() do {} while (0)
#define disable_interrupts() 0
#endif

/* lib_$(ARCH)/time.c */
void	udelay (unsigned long);
void	mdelay (unsigned long);

int gunzip(void *dst, int dstlen, unsigned char *src, unsigned long *lenp);

/* lib_generic/vsprintf.c */
ulong	simple_strtoul(const char *cp,char **endp,unsigned int base);
#ifdef CFG_64BIT_VSPRINTF
unsigned long long	simple_strtoull(const char *cp,char **endp,unsigned int base);
#endif
long	simple_strtol(const char *cp,char **endp,unsigned int base);

/* lib_generic/crc32.c */
uint32_t crc32(uint32_t, const void*, unsigned int);
uint32_t crc32_no_comp(uint32_t, const void*, unsigned int);

/* common/console.c */
int	ctrlc (void);

#define MEMAREA_SIZE_SPECIFIED 1

struct memarea_info {
        struct device_d *device;
	unsigned long start;
	unsigned long end;
	unsigned long size;
        unsigned long flags;
};

int spec_str_to_info(const char *str, struct memarea_info *info);
int parse_area_spec(const char *str, ulong *start, ulong *size);

/* Just like simple_strtoul(), but this one honors a K/M/G suffix */
unsigned long strtoul_suffix(const char *str, char **endp, int base);

void start_barebox(void);
void shutdown_barebox(void);

void arch_shutdown(void);

int run_shell(void);

/* Force a compilation error if condition is true */
#define BUILD_BUG_ON(condition) ((void)BUILD_BUG_ON_ZERO(condition))

/* Force a compilation error if condition is constant and true */
#define MAYBE_BUILD_BUG_ON(cond) ((void)sizeof(char[1 - 2 * !!(cond)]))

/* Force a compilation error if a constant expression is not a power of 2 */
#define BUILD_BUG_ON_NOT_POWER_OF_2(n)			\
	BUILD_BUG_ON((n) == 0 || (((n) & ((n) - 1)) != 0))

/*
 * Force a compilation error if condition is true, but also produce a
 * result (of value 0 and type size_t), so the expression can be used
 * e.g. in a structure initializer (or where-ever else comma
 * expressions aren't permitted).
 */
#define BUILD_BUG_ON_ZERO(e) (sizeof(struct { int:-!!(e); }))
#define BUILD_BUG_ON_NULL(e) ((void *)sizeof(struct { int:-!!(e); }))

#define ALIGN(x,a)		__ALIGN_MASK(x,(typeof(x))(a)-1)
#define __ALIGN_MASK(x,mask)	(((x)+(mask))&~(mask))
#define PTR_ALIGN(p, a)		((typeof(p))ALIGN((unsigned long)(p), (a)))
#define IS_ALIGNED(x, a)		(((x) & ((typeof(x))(a) - 1)) == 0)

#define ARRAY_SIZE(arr)		(sizeof(arr) / sizeof((arr)[0]) + __must_be_array(arr))
#define ARRAY_AND_SIZE(x)	(x), ARRAY_SIZE(x)

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})

#define USHORT_MAX	((u16)(~0U))
#define SHORT_MAX	((s16)(USHORT_MAX>>1))
#define SHORT_MIN	(-SHORT_MAX - 1)
#define INT_MAX		((int)(~0U>>1))
#define INT_MIN		(-INT_MAX - 1)
#define UINT_MAX	(~0U)
#define LONG_MAX	((long)(~0UL>>1))
#define LONG_MIN	(-LONG_MAX - 1)
#define ULONG_MAX	(~0UL)
#define LLONG_MAX	((long long)(~0ULL>>1))
#define LLONG_MIN	(-LLONG_MAX - 1)
#define ULLONG_MAX	(~0ULL)

#define PAGE_SIZE	4096

int memory_display(char *addr, ulong offs, ulong nbytes, int size);

extern const char version_string[];
void barebox_banner(void);

#define IOMEM(addr)	((void __force __iomem *)(addr))

#define DIV_ROUND_UP(n,d)	(((n) + (d) - 1) / (d))

#define DIV_ROUND_CLOSEST(x, divisor)(			\
{							\
	typeof(divisor) __divisor = divisor;		\
	(((x) + ((__divisor) / 2)) / (__divisor));	\
}							\
)

#define abs(x) ({                               \
		long __x = (x);                 \
		(__x < 0) ? -__x : __x;         \
	})

#define abs64(x) ({                             \
		s64 __x = (x);                  \
		(__x < 0) ? -__x : __x;         \
	})

#endif	/* __COMMON_H_ */
