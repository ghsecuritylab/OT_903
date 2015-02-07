

#ifndef _KYRO_H
#define _KYRO_H

struct kyrofb_info {
	void __iomem *regbase;

	u32 palette[16];
	u32 HTot;	/* Hor Total Time    */
	u32 HFP;	/* Hor Front Porch   */
	u32 HST;	/* Hor Sync Time     */
	u32 HBP;	/* Hor Back Porch    */
	s32 HSP;		/* Hor Sync Polarity */
	u32 VTot;	/* Ver Total Time    */
	u32 VFP;	/* Ver Front Porch   */
	u32 VST;	/* Ver Sync Time     */
	u32 VBP;	/* Ver Back Porch    */
	s32 VSP;		/* Ver Sync Polarity */
	u32 XRES;	/* X Resolution      */
	u32 YRES;	/* Y Resolution      */
	u32 VFREQ;	/* Ver Frequency     */
	u32 PIXCLK;	/* Pixel Clock       */
	u32 HCLK;	/* Hor Clock         */

	/* Usefull to hold depth here for Linux */
	u8 PIXDEPTH;

#ifdef CONFIG_MTRR
	int mtrr_handle;
#endif
};

extern int kyro_dev_init(void);
extern void kyro_dev_reset(void);

extern unsigned char *kyro_dev_physical_fb_ptr(void);
extern unsigned char *kyro_dev_virtual_fb_ptr(void);
extern void *kyro_dev_physical_regs_ptr(void);
extern void *kyro_dev_virtual_regs_ptr(void);
extern unsigned int kyro_dev_fb_size(void);
extern unsigned int kyro_dev_regs_size(void);

extern u32 kyro_dev_overlay_offset(void);

#define KYRO_IOC_MAGIC 'k'

#define KYRO_IOCTL_OVERLAY_CREATE       _IO(KYRO_IOC_MAGIC, 0)
#define KYRO_IOCTL_OVERLAY_VIEWPORT_SET _IO(KYRO_IOC_MAGIC, 1)
#define KYRO_IOCTL_SET_VIDEO_MODE       _IO(KYRO_IOC_MAGIC, 2)
#define KYRO_IOCTL_UVSTRIDE             _IO(KYRO_IOC_MAGIC, 3)
#define KYRO_IOCTL_OVERLAY_OFFSET       _IO(KYRO_IOC_MAGIC, 4)
#define KYRO_IOCTL_STRIDE               _IO(KYRO_IOC_MAGIC, 5)

typedef struct _OVERLAY_CREATE {
	u32 ulWidth;
	u32 ulHeight;
	int bLinear;
} overlay_create;

typedef struct _OVERLAY_VIEWPORT_SET {
	u32 xOrgin;
	u32 yOrgin;
	u32 xSize;
	u32 ySize;
} overlay_viewport_set;

typedef struct _SET_VIDEO_MODE {
	u32 ulWidth;
	u32 ulHeight;
	u32 ulScan;
	u8 displayDepth;
	int bLinear;
} set_video_mode;

#endif /* _KYRO_H */
