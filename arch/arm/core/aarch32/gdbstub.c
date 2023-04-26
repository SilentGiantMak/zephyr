#include <zephyr/kernel.h>
#include <kernel_internal.h>
#include <zephyr/arch/arm/aarch32/gdbstub.h>

#define DBGDSCR_MONITOR_MODE_EN 0x8000

#define DBGDBCR_ADDR_MASK_MASK		0x1F
#define DBGDBCR_ADDR_MASK_SHIFT		24
#define DBGDBCR_MEANING_MASK		0x7
#define DBGDBCR_MEANING_SHIFT		20
#define DBGDBCR_MEANING_ADDR_MATCH	0x0
#define DBGDBCR_MEANING_ADDR_MISMATCH	0x4
#define DBGDBCR_LINKED_BRP_MASK		0xF
#define DBGDBCR_LINKED_BRP_SHIFT	16
#define DBGDBCR_SECURE_STATE_MASK	0x3
#define DBGDBCR_SECURE_STATE_SHIFT	14
#define DBGDBCR_BYTE_ADDR_MASK		0xF
#define DBGDBCR_BYTE_ADDR_SHIFT		5
#define DBGDBCR_SUPERVISOR_ACCESS_MASK	0x3
#define DBGDBCR_SUPERVISOR_ACCESS_SHIFT 1
#define DBGDBCR_BRK_EN_MASK		0x1

#define SPSR_REG_IDX	25
#define GDB_PACKET_SIZE (41 * 8 + 8)

/* Position of each register in the packet, see GDB code */
static const int packet_pos[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 41};

/* Required struct */
static struct gdb_ctx ctx;

/* True if entering after a BKPT instruction */
static int first_entry;

static void printch(char p)
{
	// wait for an empty flag in case the buffer is completely full
	while ((sys_read32(0xe000102c) & 0x8) == 0) {
		;
	}
	sys_write32((unsigned int)(p), 0xE0001030);
}

static void print(char *prefix, unsigned num)
{
	for (char *p = prefix; *p; p++) {
		printch(*p);
	}

	char str[12] = {0};
	char hex[] = "0123456789abcdef";
	char *p = str;
	do {
		*p++ = hex[num & 0xf];
		num >>= 4;
	} while (num > 0);
	*p++ = 'x';
	*p = '0';
	while (p >= str) {
		printch(*p--);
	}
	printch('\r');
	printch('\n');
}

/* Wrapper function to save and restore execution context */
void z_gdb_entry(z_arch_esf_t *esf, unsigned int exc_cause)
{
	print("--- Entering stub ", 0);
	/* Disable the hardware breapoint in case it was set */
	__asm__ volatile("mcr p14, 0, %0, c0, c0, 5" ::"r"(0x0) :);

	ctx.exception = exc_cause;
	// save the registers
	ctx.registers[R0] = esf->basic.r0;
	ctx.registers[R1] = esf->basic.r1;
	ctx.registers[R2] = esf->basic.r2;
	ctx.registers[R3] = esf->basic.r3;
	/* We are going to assume, that the EXTRA_EXCEPTION_INFO kernel option is set */
	ctx.registers[R4] = esf->extra_info.callee->v1;
	ctx.registers[R5] = esf->extra_info.callee->v2;
	ctx.registers[R6] = esf->extra_info.callee->v3;
	ctx.registers[R7] = esf->extra_info.callee->v4;
	ctx.registers[R8] = esf->extra_info.callee->v5;
	ctx.registers[R9] = esf->extra_info.callee->v6;
	ctx.registers[R10] = esf->extra_info.callee->v7;
	ctx.registers[R11] = esf->extra_info.callee->v8;
	ctx.registers[R13] = esf->extra_info.callee->psp;
	ctx.registers[R12] = esf->basic.r12;
	ctx.registers[LR] = esf->basic.lr;
	ctx.registers[PC] = esf->basic.pc;
	ctx.registers[SPSR] = esf->basic.xpsr;

	z_gdb_main_loop(&ctx);

	esf->basic.r0 = ctx.registers[R0];
	esf->basic.r1 = ctx.registers[R1];
	esf->basic.r2 = ctx.registers[R2];
	esf->basic.r3 = ctx.registers[R3];
	esf->basic.r12 = ctx.registers[R12];
	esf->basic.lr = ctx.registers[LR];
	/* The registers part of EXTRA_EXCEPTION_INFO are read-only - the excpetion return code
	does not restore them, thus we don't need to do so here */
	if (first_entry) {
		/* The CPU should continue on the next instruction - apply this offset,
		so that it won't be affected by the bkpt instruction */
		esf->basic.pc = ctx.registers[PC] + 0x4;
	} else {
		esf->basic.pc = ctx.registers[PC];
	}
	first_entry = 0;
	esf->basic.xpsr = ctx.registers[SPSR];
}

void arch_gdb_init(void)
{
	print("--- Stub init", 0);
	first_entry = 1;
	uint32_t reg_val;
	/* Enable the monitor debug mode */
	__asm__ volatile("mrc p14, 0, %0, c0, c2, 2" : "=r"(reg_val)::);
	reg_val |= DBGDSCR_MONITOR_MODE_EN;
	__asm__ volatile("mcr p14, 0, %0, c0, c2, 2" ::"r"(reg_val) :);

	/* Generate the Prefetch abort exception */
	//__asm__ volatile("BKPT");
}

void arch_gdb_continue(void)
{
	/* No need to do anything, return to the code. */
}

void arch_gdb_step(void)
{
	/* Set the hardware breakpoint */
	uint32_t reg_val = ctx.registers[PC];
	/* set BVR (Breakpoint value register) to PC, make sure it is word aligned */
	reg_val &= ~(0x3);
	__asm__ volatile("mcr p14, 0, %0, c0, c0, 4" ::"r"(reg_val) :);

	reg_val = 0;
	/* Address mismatch */
	reg_val |= (DBGDBCR_MEANING_ADDR_MISMATCH & DBGDBCR_MEANING_MASK) << DBGDBCR_MEANING_SHIFT;
	/* Match any other instruction */
	reg_val |= (0xF & DBGDBCR_BYTE_ADDR_MASK) << DBGDBCR_BYTE_ADDR_SHIFT;
	/* Breakpoint enable */
	reg_val |= DBGDBCR_BRK_EN_MASK;
	__asm__ volatile("mcr p14, 0, %0, c0, c0, 5" ::"r"(reg_val) :);
}

size_t arch_gdb_reg_readall(struct gdb_ctx *ctx, uint8_t *buf, size_t buflen)
{
	int ret = 0;
	/* All other register are not supported */
	memset(buf, 'x', buflen);
	for (int i = 0; i < GDB_STUB_NUM_REGISTERS; i++) {
		/* offset inside the packet */
		int pos = packet_pos[i] * 8;
		int r = bin2hex((const uint8_t *)(ctx->registers + i), 4, buf + pos, buflen - pos);
		/* remove the newline character placed by the bin2hex function */
		buf[pos + 8] = 'x';
		if (r == 0) {
			ret = 0;
			break;
		}
		ret += r;
	}

	if (ret) {
		/* since we don't support some floating point registers, set the packet size
		 * manually */
		ret = GDB_PACKET_SIZE;
	}
	return ret;
}

size_t arch_gdb_reg_writeall(struct gdb_ctx *ctx, uint8_t *hex, size_t hexlen)
{

	int ret = 0;
	for (unsigned int i = 0; i < hexlen; i += 8) {
		if (hex[i] != 'x') {
			/* check if the stub supports this register */
			for (unsigned int j = 0; j < GDB_STUB_NUM_REGISTERS; j++) {
				if (packet_pos[j] != i) {
					continue;
				}
				int r = hex2bin(hex + i * 8, 8, (uint8_t *)(ctx->registers + j), 4);
				if (r == 0) {
					return 0;
				}
				ret += r;
			}
		}
	}
	return ret;
}

size_t arch_gdb_reg_readone(struct gdb_ctx *ctx, uint8_t *buf, size_t buflen, uint32_t regno)
{

	int ret = 0;
	/* Fill the buffer with 'x' in case the stub does not support the required register */
	memset(buf, "xxxxxxxx", 8);
	ret = 8;
	if (regno == SPSR_REG_IDX) {
		/* The SPSR register is at the end, we have to check separately */
		ret = bin2hex((uint8_t *)(ctx->registers + GDB_STUB_NUM_REGISTERS - 1), 4, buf,
			      buflen);
	} else {
		/* Check which of our registers corresponds to regnum */
		for (int i = 0; i < GDB_STUB_NUM_REGISTERS; i++) {
			if (packet_pos[i] == regno) {
				ret = bin2hex((uint8_t *)(ctx->registers + i), 4, buf, buflen);
				break;
			}
		}
	}

	return ret;
}

size_t arch_gdb_reg_writeone(struct gdb_ctx *ctx, uint8_t *hex, size_t hexlen, uint32_t regno)
{

	int ret = 0;
	/* Set the value of a register */
	if (hexlen != 8) {
		return ret;
	}

	if (regno < (GDB_STUB_NUM_REGISTERS - 1)) {
		/* Again, check the corresponding register index */
		for (int i = 0; i < GDB_STUB_NUM_REGISTERS; i++) {
			if (packet_pos[i] == regno) {
				ret = hex2bin(hex, hexlen, (uint8_t *)(ctx->registers + i), 4);
				break;
			}
		}
	} else if (regno == SPSR_REG_IDX) {
		ret = hex2bin(hex, hexlen, (uint8_t *)(ctx->registers + GDB_STUB_NUM_REGISTERS - 1),
			      4);
	}
	return ret;
}
