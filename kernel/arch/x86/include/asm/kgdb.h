
#ifndef _ASM_X86_KGDB_H
#define _ASM_X86_KGDB_H


#define BUFMAX			1024

#ifdef CONFIG_X86_32
enum regnames {
	GDB_AX,			/* 0 */
	GDB_CX,			/* 1 */
	GDB_DX,			/* 2 */
	GDB_BX,			/* 3 */
	GDB_SP,			/* 4 */
	GDB_BP,			/* 5 */
	GDB_SI,			/* 6 */
	GDB_DI,			/* 7 */
	GDB_PC,			/* 8 also known as eip */
	GDB_PS,			/* 9 also known as eflags */
	GDB_CS,			/* 10 */
	GDB_SS,			/* 11 */
	GDB_DS,			/* 12 */
	GDB_ES,			/* 13 */
	GDB_FS,			/* 14 */
	GDB_GS,			/* 15 */
};
#define NUMREGBYTES		((GDB_GS+1)*4)
#else /* ! CONFIG_X86_32 */
enum regnames64 {
	GDB_AX,			/* 0 */
	GDB_BX,			/* 1 */
	GDB_CX,			/* 2 */
	GDB_DX,			/* 3 */
	GDB_SI,			/* 4 */
	GDB_DI,			/* 5 */
	GDB_BP,			/* 6 */
	GDB_SP,			/* 7 */
	GDB_R8,			/* 8 */
	GDB_R9,			/* 9 */
	GDB_R10,		/* 10 */
	GDB_R11,		/* 11 */
	GDB_R12,		/* 12 */
	GDB_R13,		/* 13 */
	GDB_R14,		/* 14 */
	GDB_R15,		/* 15 */
	GDB_PC,			/* 16 */
};

enum regnames32 {
	GDB_PS = 34,
	GDB_CS,
	GDB_SS,
};
#define NUMREGBYTES		((GDB_SS+1)*4)
#endif /* CONFIG_X86_32 */

static inline void arch_kgdb_breakpoint(void)
{
	asm("   int $3");
}
#define BREAK_INSTR_SIZE	1
#define CACHE_FLUSH_IS_SAFE	1

extern int kgdb_ll_trap(int cmd, const char *str,
			struct pt_regs *regs, long err, int trap, int sig);

#endif /* _ASM_X86_KGDB_H */