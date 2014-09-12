#include <antirebootloop.h>
#include <common.h>
#include <environment.h>
#include <init.h>

#define FAILOVER_THRESHOLD 3
#define FAILOVER_THRESHOLD_NO_KERNEL_SUPPORT 6

static void *antirebootloop_scratchspace = (void *) 0x1000;

static int write_marker(struct arl_marker *m) {
	int i;
	memset(m, 0, sizeof(*m));
	for (i=0; i < ARRAY_SIZE(m->magic); i++) {
		m->magic[i] = ARL_MAGIC + i;
	}
	m->kernel_version = 0;
	/* m->bootloader_version is populated in antirebootloop_init() */
	return 0;
}

static int verify_marker(struct arl_marker *m) {
	int i;
	for (i=0; i < ARRAY_SIZE(m->magic); i++) {
		if (m->magic[i] != ARL_MAGIC + i) return -1;
	}
	return 0;
}

static int antirebootloop_init(void) {
	struct arl_marker *m = antirebootloop_scratchspace;
	if (verify_marker(m)) {
		printf("Cannot find antirebootloop marker\n");
		write_marker(m);
		m->counter = 0;
	} else {
		printf("Found antirebootloop marker. Counter %u."
			       " Kernel support %u\n",
			       m->counter, m->kernel_version);
	}
	m->bootloader_version = ARL_BOOTLOADER_VERSION;
	if ( (m->kernel_version == 1 && m->counter >= FAILOVER_THRESHOLD) ||
		(m->kernel_version == 0 &&
			m->counter >= FAILOVER_THRESHOLD_NO_KERNEL_SUPPORT) ) {
		printf("Antirebootloop threshold reached\n");
		setenv("arl", "failover");
	} else {
		setenv("arl", "ok");
	}
	return 0;
}

void antirebootloop_preboot_hook(void) {
	struct arl_marker *m = antirebootloop_scratchspace;
	m->counter++;
	/* Reset kernel_version to ensure that, on the next boot, we don't read
	 * some stale value from a previously booted kernel. */
	m->kernel_version = 0;
}
EXPORT_SYMBOL(antirebootloop_preboot_hook)

late_initcall(antirebootloop_init);
