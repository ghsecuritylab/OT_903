

#ifndef QEMU_SOFTMMU_OUTSIDE_JIT_H
#define QEMU_SOFTMMU_OUTSIDE_JIT_H

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////
// Declares routines implemented in softmmu_outside_jit.c, that are used in
// this macros expansion. Note that MMUSUFFIX _outside_jit is enforced in
// softmmu_header.h by defining OUTSIDE_JIT macro.
////////////////////////////////////////////////////////////////////////////////

uint8_t REGPARM __ldb_outside_jit(target_ulong addr, int mmu_idx);
void REGPARM __stb_outside_jit(target_ulong addr, uint8_t val, int mmu_idx);
uint16_t REGPARM __ldw_outside_jit(target_ulong addr, int mmu_idx);
void REGPARM __stw_outside_jit(target_ulong addr, uint16_t val, int mmu_idx);
uint32_t REGPARM __ldl_outside_jit(target_ulong addr, int mmu_idx);
void REGPARM __stl_outside_jit(target_ulong addr, uint32_t val, int mmu_idx);
uint64_t REGPARM __ldq_outside_jit(target_ulong addr, int mmu_idx);
void REGPARM __stq_outside_jit(target_ulong addr, uint64_t val, int mmu_idx);

// Enforces MMUSUFFIX to be set to _outside_jit in softmmu_header.h
#define OUTSIDE_JIT
// Enforces use of cpu_single_env for CPU environment.
#define env cpu_single_env

// =============================================================================
// Generate ld/stx_user
// =============================================================================
#define MEMSUFFIX MMU_MODE1_SUFFIX
#define ACCESS_TYPE 1

#define DATA_SIZE 1
#include "softmmu_header.h"

#define DATA_SIZE 2
#include "softmmu_header.h"

#define DATA_SIZE 4
#include "softmmu_header.h"

#define DATA_SIZE 8
#include "softmmu_header.h"

#undef MEMSUFFIX
#undef ACCESS_TYPE

// =============================================================================
// Generate ld/stx_kernel
// =============================================================================
#define MEMSUFFIX MMU_MODE0_SUFFIX
#define ACCESS_TYPE 0

#define DATA_SIZE 1
#include "softmmu_header.h"

#define DATA_SIZE 2
#include "softmmu_header.h"

#define DATA_SIZE 4
#include "softmmu_header.h"

#define DATA_SIZE 8
#include "softmmu_header.h"

#undef MEMSUFFIX
#undef ACCESS_TYPE

#undef env
#undef OUTSIDE_JIT

#ifdef __cplusplus
};  /* end of extern "C" */
#endif

#endif  // QEMU_SOFTMMU_OUTSIDE_JIT_H
