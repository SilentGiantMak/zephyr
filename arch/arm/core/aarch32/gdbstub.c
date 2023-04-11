#include <zephyr/kernel.h>
#include <kernel_internal.h>
#include <zephyr/arch/arm/aarch32/gdbstub.h>

#define DBGDSCR_MONITOR_MODE_EN 0x8000

#define DBGDBCR_ADDR_MASK_MASK		0x1F
#define DBGDBCR_ADDR_MASK_SHIFT		24
#define DBGDBCR_MEANING_MASK		0x7
#define DBGDBCR_MEANING_SHIFT		20
#define DBGDBCR_MEANING_ADDR_MATCH	0x0
#define DBGDBCR_LINKED_BRP_MASK		0xF
#define DBGDBCR_LINKED_BRP_SHIFT	16
#define DBGDBCR_SECURE_STATE_MASK	0x3
#define DBGDBCR_SECURE_STATE_SHIFT	14
#define DBGDBCR_BYTE_ADDR_MASK		0xF
#define DBGDBCR_BYTE_ADDR_SHIFT		5
#define DBGDBCR_SUPERVISOR_ACCESS_MASK	0x3
#define DBGDBCR_SUPERVISOR_ACCESS_SHIFT 1
#define DBGDBCR_BRK_EN_MASK		0x1

/* Required struct */
static struct gdb_ctx ctx;

/* Wrapper function to save and restore execution context */
static void z_gdb_entry(z_arch_esf_t *esf);
{
	// TODO add more exception causes - the stub supports just the debug event (0x2)
	ctx.exception = 0x2;
	// save the registers
	ctx.registers[R0] = esf->basic.r0;
	ctx.registers[R1] = esf->basic.r1;
	ctx.registers[R2] = esf->basic.r2;
	ctx.registers[R3] = esf->basic.r3;
	ctx.registers[R12] = esf->basic.r12;
	ctx.registers[LR] = esf->basic.lr;
	ctx.registers[PC] = esf->basic.pc;
	ctx.registers[SPSR] = esf->xpsr;

	z_gdb_main_loop(&ctx);

	esf->basic.r0 = ctx.registers[R0];
	esf->basic.r1 = ctx.registers[R1];
	esf->basic.r2 = ctx.registers[R2];
	esf->basic.r3 = ctx.registers[R3];
	esf->basic.r12 = ctx.registers[R12];
	esf->basic.lr = ctx.registers[LR];
	esf->basic.pc = ctx.registers[PC];
	esf->xpsr = ctx.registers[SPSR];
}

void arch_gdb_init(void)
{
	/* Enable the monitor debug mode */
	uint32_t reg_val;
	__asm__ volatile("mrc p14, 0, %0, c0, c2, 2" : "=r"(reg_val)::);
	reg_val |= DBGDSCR_MONITOR_MODE_EN;
	__asm__ volatile("mcr p14, 0, %0, c0, c2, 2" ::"r"(reg_val) :);
	/* Generate the Prefetch abort exception */
	__asm__ volatile("BKPT");
}

void arch_gdb_continue(void)
{
	/* No need to do anything, return to the code. */
}

void arch_gdb_step(void)
{
	/* Set the hardware breakpoint */
	uint32_t reg_val = ctx.registers[LR] + 0x4;
	/* set BVR to LR + 0x4, make sure it is word aligned */
	reg_val &= ~(0x3);
	__asm__ volatile("mcr p14, 0, %0, c0, c0, 4" ::"r"(reg_val) :);

	/* Program the BCR */
	reg_val = 0;
	/* no addr mask, instruction address match mode */
	reg_val |= (0x0 & DBGDBCR_ADDR_MASK_MASK) << DBGDBCR_ADDR_MASK_SHIFT;
	reg_val |= (0x0 & DBGDBCR_MEANING_MASK) << DBGDBCR_MEANING_SHIFT;
	/* no linked BRP, match in both secure and non-secure state, match all bytes */
	reg_val |= (0x0 & DBGDBCR_LINKED_BRP_MASK) << DBGDBCR_LINKED_BRP_SHIFT;
	reg_val |= (0x0 & DBGDBCR_SECURE_STATE_MASK) << DBGDBCR_SECURE_STATE_SHIFT;
	reg_val |= (0xF & DBGDBCR_BYTE_ADDR_MASK) << DBGDBCR_BYTE_ADDR_SHIFT;
	/* any access control for now, breakpoint enable */
	reg_val |= (0x3 & DBGDBCR_SUPERVISOR_ACCESS_MASK) << DBGDBCR_SUPERVISOR_ACCESS_SHIFT;
	reg_val |= DBGDBCR_BRK_EN_MASK;
    __asm__ volatile("mcr p14, 0, %0, c0, c0, 5" ::"r"(reg_val) :);
}

size_t arch_gdb_reg_readall(struct gdb_ctx *ctx, uint8_t *buf, size_t buflen)
{
	// TODO
}

size_t arch_gdb_reg_writeall(struct gdb_ctx *ctx, uint8_t *hex, size_t hexlen)
{
	// TODO
}

size_t arch_gdb_reg_readone(struct gdb_ctx *ctx, uint8_t *buf, size_t buflen, uint32_t regno)
{
	// TODO
}

size_t arch_gdb_reg_writeone(struct gdb_ctx *ctx, uint8_t *hex, size_t hexlen, uint32_t regno)
{
	// TODO
}
