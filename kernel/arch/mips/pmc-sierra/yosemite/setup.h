
#ifndef __SETUP_H__
#define __SETUP_H__

/* M48T37 RTC + NVRAM */
#define	YOSEMITE_RTC_BASE		0xfc800000
#define	YOSEMITE_RTC_SIZE		0x00800000

#define HYPERTRANSPORT_BAR0_ADDR        0x00000006
#define HYPERTRANSPORT_SIZE0            0x0fffffff
#define HYPERTRANSPORT_BAR0_ATTR        0x00002000

#define HYPERTRANSPORT_ENABLE           0x6

#define	TITAN_ATMEL_24C32_SIZE		32768
#define	TITAN_ATMEL_24C64_SIZE		65536

#endif /* __SETUP_H__ */
