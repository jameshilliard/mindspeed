#ifndef __MACH_DEBUG_LL_H__
#define   __MACH_DEBUG_LL_H__

#include <mach/serial.h>
#include <mach/comcerto-2000.h>
#include <asm/io.h>

static inline void __putc(char c)
{
	/* Wait until there is space in the FIFO */
	while ((readl(UART_BASEADDR + UART_LSR) & LSR_THRE) == 0);

	/* Send the character */
	writel(c, UART_BASEADDR + UART_THR);
	/* Wait to make sure it hits the line, in case we die too soon. */
	while ((readl(UART_BASEADDR + UART_LSR) & LSR_THRE) == 0);
}
static inline void putc(char c) {
	if (c == '\n') {
		__putc('\r');
	}
	__putc(c);
}

#endif
