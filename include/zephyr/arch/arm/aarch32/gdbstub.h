/*
 * Copyright (c) 2023 Marek Vedral <vedrama5@fel.cvut.cz>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_ARCH_ARM_AARCH32_GDBSTUB_H_
#define ZEPHYR_INCLUDE_ARCH_ARM_AARCH32_GDBSTUB_H_

#include <zephyr/arch/arm/aarch32/exc.h>

#ifndef _ASMLANGUAGE

#define IFSR_DEBUG_EVENT 0x2

enum AARCH32_GDB_REG {
	R0 = 0,
	R1,
	R2,
	R3,
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
	LR,
	PC,
	/* Saved program status register */
	SPSR,
	GDB_NUM_REGS
};

/* required structure */ 
struct gdb_ctx {
	/* cause of the exception */
	unsigned int exception;
	unsigned int registers[GDB_NUM_REGS];
};

void z_gdb_entry(z_arch_esf_t *esf, unsigned int exc_cause);

#endif

#endif
