

#ifndef __ASM_PLAT_MAP_S5P_H
#define __ASM_PLAT_MAP_S5P_H __FILE__

#define S5P_VA_CHIPID		S3C_ADDR(0x00700000)
#define S5P_VA_GPIO		S3C_ADDR(0x00500000)
#define S5P_VA_SYSTIMER		S3C_ADDR(0x01200000)
#define S5P_VA_SROMC		S3C_ADDR(0x01100000)

#define S5P_VA_UART0		(S3C_VA_UART + 0x0)
#define S5P_VA_UART1		(S3C_VA_UART + 0x400)
#define S5P_VA_UART2		(S3C_VA_UART + 0x800)
#define S5P_VA_UART3		(S3C_VA_UART + 0xC00)

#define S3C_UART_OFFSET		(0x400)

#define VA_VIC(x)		(S3C_VA_IRQ + ((x) * 0x10000))
#define VA_VIC0			VA_VIC(0)
#define VA_VIC1			VA_VIC(1)
#define VA_VIC2			VA_VIC(2)
#define VA_VIC3			VA_VIC(3)

#endif /* __ASM_PLAT_MAP_S5P_H */
