#include <zephyr/kernel.h>
#include <kernel_internal.h>
#include <zephyr/arch/arm/aarch32/gdbstub.h>

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
	// TODO
}

void arch_gdb_continue(void)
{
	// TODO
}

void arch_gdb_step(void)
{
	// TODO
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
