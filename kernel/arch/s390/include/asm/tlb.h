
#ifndef _S390_TLB_H
#define _S390_TLB_H


#include <linux/mm.h>
#include <linux/swap.h>
#include <asm/processor.h>
#include <asm/pgalloc.h>
#include <asm/smp.h>
#include <asm/tlbflush.h>

#ifndef CONFIG_SMP
#define TLB_NR_PTRS	1
#else
#define TLB_NR_PTRS	508
#endif

struct mmu_gather {
	struct mm_struct *mm;
	unsigned int fullmm;
	unsigned int nr_ptes;
	unsigned int nr_pxds;
	void *array[TLB_NR_PTRS];
};

DECLARE_PER_CPU(struct mmu_gather, mmu_gathers);

static inline struct mmu_gather *tlb_gather_mmu(struct mm_struct *mm,
						unsigned int full_mm_flush)
{
	struct mmu_gather *tlb = &get_cpu_var(mmu_gathers);

	tlb->mm = mm;
	tlb->fullmm = full_mm_flush || (num_online_cpus() == 1) ||
		(atomic_read(&mm->mm_users) <= 1 && mm == current->active_mm);
	tlb->nr_ptes = 0;
	tlb->nr_pxds = TLB_NR_PTRS;
	if (tlb->fullmm)
		__tlb_flush_mm(mm);
	return tlb;
}

static inline void tlb_flush_mmu(struct mmu_gather *tlb,
				 unsigned long start, unsigned long end)
{
	if (!tlb->fullmm && (tlb->nr_ptes > 0 || tlb->nr_pxds < TLB_NR_PTRS))
		__tlb_flush_mm(tlb->mm);
	while (tlb->nr_ptes > 0)
		pte_free(tlb->mm, tlb->array[--tlb->nr_ptes]);
	while (tlb->nr_pxds < TLB_NR_PTRS)
		/* pgd_free frees the pointer as region or segment table */
		pgd_free(tlb->mm, tlb->array[tlb->nr_pxds++]);
}

static inline void tlb_finish_mmu(struct mmu_gather *tlb,
				  unsigned long start, unsigned long end)
{
	tlb_flush_mmu(tlb, start, end);

	/* keep the page table cache within bounds */
	check_pgt_cache();

	put_cpu_var(mmu_gathers);
}

static inline void tlb_remove_page(struct mmu_gather *tlb, struct page *page)
{
	free_page_and_swap_cache(page);
}

static inline void pte_free_tlb(struct mmu_gather *tlb, pgtable_t pte,
				unsigned long address)
{
	if (!tlb->fullmm) {
		tlb->array[tlb->nr_ptes++] = pte;
		if (tlb->nr_ptes >= tlb->nr_pxds)
			tlb_flush_mmu(tlb, 0, 0);
	} else
		pte_free(tlb->mm, pte);
}

static inline void pmd_free_tlb(struct mmu_gather *tlb, pmd_t *pmd,
				unsigned long address)
{
#ifdef __s390x__
	if (tlb->mm->context.asce_limit <= (1UL << 31))
		return;
	if (!tlb->fullmm) {
		tlb->array[--tlb->nr_pxds] = pmd;
		if (tlb->nr_ptes >= tlb->nr_pxds)
			tlb_flush_mmu(tlb, 0, 0);
	} else
		pmd_free(tlb->mm, pmd);
#endif
}

static inline void pud_free_tlb(struct mmu_gather *tlb, pud_t *pud,
				unsigned long address)
{
#ifdef __s390x__
	if (tlb->mm->context.asce_limit <= (1UL << 42))
		return;
	if (!tlb->fullmm) {
		tlb->array[--tlb->nr_pxds] = pud;
		if (tlb->nr_ptes >= tlb->nr_pxds)
			tlb_flush_mmu(tlb, 0, 0);
	} else
		pud_free(tlb->mm, pud);
#endif
}

#define tlb_start_vma(tlb, vma)			do { } while (0)
#define tlb_end_vma(tlb, vma)			do { } while (0)
#define tlb_remove_tlb_entry(tlb, ptep, addr)	do { } while (0)
#define tlb_migrate_finish(mm)			do { } while (0)

#endif /* _S390_TLB_H */
