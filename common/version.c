#include <common.h>
#include <mach/ddr.h>
#include <reloc.h>
#include <generated/utsrelease.h>

#ifdef	CONFIG_COMCERTO_BOOTLOADER
const char version_string[] =
	"barebox " UTS_RELEASE " (" __DATE__ " - " __TIME__ ")";
#else
const char version_string[] =
	"uloader " UTS_RELEASE " (" __DATE__ " - " __TIME__ ")";
#endif

#ifdef CONFIG_HAS_EARLY_INIT
#error We do not support CONFIG_HAS_EARLY_INIT
#endif

void barebox_banner (void)
{
	printf("\n\n%s\n\n", version_string);
	printf("SoC: " CONFIG_BOARDINFO "\n");
	printf("Board: %s\n", get_ddr_config_description());
}

