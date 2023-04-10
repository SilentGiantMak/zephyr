#include <zephyr/kernel.h>
#include <kernel_internal.h>
#include <zephyr/arch/arm/aarch32/gdbstub.h>

/* Required struct */
static struct gdb_ctx ctx;

/* Wrapper function to save and restore execution context */
static void z_gdb_entry()
{
    // TODO
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
