

#ifndef __ASM_ARM_ARCH_IO_H
#define __ASM_ARM_ARCH_IO_H

#include <mach/hardware.h>

#define IO_SPACE_LIMIT		0xffffffff
#define __mem_pci(a)		(a)

#ifdef CONFIG_IXP2000_SUPPORT_BROKEN_PCI_IO
#define ___io(p)		((void __iomem *)((p)+IXP2000_PCI_IO_VIRT_BASE))

#define alignb(addr)		(void __iomem *)((unsigned long)(addr) ^ 3)
#define alignw(addr)		(void __iomem *)((unsigned long)(addr) ^ 2)

#define outb(v,p)		__raw_writeb((v),alignb(___io(p)))
#define outw(v,p)		__raw_writew((v),alignw(___io(p)))
#define outl(v,p)		__raw_writel((v),___io(p))

#define inb(p)		({ unsigned int __v = __raw_readb(alignb(___io(p))); __v; })
#define inw(p)		\
	({ unsigned int __v = (__raw_readw(alignw(___io(p)))); __v; })
#define inl(p)		\
	({ unsigned int __v = (__raw_readl(___io(p))); __v; })

#define outsb(p,d,l)		__raw_writesb(alignb(___io(p)),d,l)
#define outsw(p,d,l)		__raw_writesw(alignw(___io(p)),d,l)
#define outsl(p,d,l)		__raw_writesl(___io(p),d,l)

#define insb(p,d,l)		__raw_readsb(alignb(___io(p)),d,l)
#define insw(p,d,l)		__raw_readsw(alignw(___io(p)),d,l)
#define insl(p,d,l)		__raw_readsl(___io(p),d,l)

#define __is_io_address(p)	((((unsigned long)(p)) & ~(IXP2000_PCI_IO_SIZE - 1)) == IXP2000_PCI_IO_VIRT_BASE)

#define ioread8(p)						\
	({							\
		unsigned int __v;				\
								\
		if (__is_io_address(p)) {			\
			__v = __raw_readb(alignb(p));		\
		} else {					\
			__v = __raw_readb(p);			\
		}						\
								\
		__v;						\
	})							\

#define ioread16(p)						\
	({							\
		unsigned int __v;				\
								\
		if (__is_io_address(p)) {			\
			__v = __raw_readw(alignw(p));		\
		} else {					\
			__v = le16_to_cpu(__raw_readw(p));	\
		}						\
								\
		__v;						\
	})

#define ioread32(p)						\
	({							\
		unsigned int __v;				\
								\
		if (__is_io_address(p)) {			\
			__v = __raw_readl(p);			\
		} else {					\
			__v = le32_to_cpu(__raw_readl(p));	\
		}						\
								\
		 __v;						\
	})

#define iowrite8(v,p)						\
	({							\
		if (__is_io_address(p)) {			\
			__raw_writeb((v), alignb(p));		\
		} else {					\
			__raw_writeb((v), p);			\
		}						\
	})

#define iowrite16(v,p)						\
	({							\
		if (__is_io_address(p)) {			\
			__raw_writew((v), alignw(p));		\
		} else {					\
			__raw_writew(cpu_to_le16(v), p);	\
		}						\
	})

#define iowrite32(v,p)						\
	({							\
		if (__is_io_address(p)) {			\
			__raw_writel((v), p);			\
		} else {					\
			__raw_writel(cpu_to_le32(v), p);	\
		}						\
	})

#define ioport_map(port, nr)	___io(port)

#define ioport_unmap(addr)
#else
#define __io(p)			((void __iomem *)((p)+IXP2000_PCI_IO_VIRT_BASE))
#endif


#endif
