#ifndef ZEPHYR_INCLUDE_ARCH_ARM_AARCH32_GDBSTUB_H_
#define ZEPHYR_INCLUDE_ARCH_ARM_AARCH32_GDBSTUB_H_

// required structure
struct gdb_ctx {
    // cause of the exception
    unsigned int exception;
};

#endif
