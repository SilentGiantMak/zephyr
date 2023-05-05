#ifndef ZEPHYR_INCLUDE_ARCH_ARM_AARCH32_GDBSTUB_H_
#define ZEPHYR_INCLUDE_ARCH_ARM_AARCH32_GDBSTUB_H_

#include <zephyr/arch/arm/aarch32/exc.h>

#ifndef _ASMLANGUAGE

#if defined(CONFIG_EXTRA_EXCEPTION_INFO)
#define GDB_STUB_NUM_REGISTERS 17
#else
#define GDB_STUB_NUM_REGISTERS 8
#endif

enum AARCH32_GDB_REG {
	R0 = 0,
	R1,
	R2,
	R3,
#if defined(CONFIG_EXTRA_EXCEPTION_INFO)
	/* READONLY registers (R4 - R13) except R12 */
	R4,
	R5,
	R6,
	R7,
	R8,
	R9,
	R10,
	R11,
	R12,
	/* Stack pointer - READONLY */
	R13,
#else
	R12,
#endif
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

void z_gdb_entry(z_arch_esf_t *esf, unsigned int exc_cause);

#endif

#endif
