#ifndef ZEPHYR_INCLUDE_ARCH_ARM_AARCH32_GDBSTUB_H_
#define ZEPHYR_INCLUDE_ARCH_ARM_AARCH32_GDBSTUB_H_

#include <zephyr/arch/arm/aarch32/exc.h>

#ifndef _ASMLANGUAGE

#define GDB_STUB_NUM_REGISTERS 8

enum AARCH32_GDB_REG {
	R0 = 0,
	R1,
	R2,
	R3,
	R12,
	LR,
	PC,
	/* Saved program status register */
	SPSR
};

// required structure
struct gdb_ctx {
	// cause of the exception
	unsigned int exception;
	unsigned int registers[GDB_STUB_NUM_REGISTERS];
};

void z_gdb_entry(z_arch_esf_t *esf);

#endif

#endif
