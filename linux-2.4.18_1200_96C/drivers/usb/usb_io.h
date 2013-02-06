/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 */
#ifndef _USB_IO_H
#define _USB_IO_H

/* JoeyLin_2007-06-25, refer to \uClinux-dist\linux-2.4.x\include\asm-mips\io.h */
#if 1

u16 rtl865x_usb_ioread16(u32 addr);
extern u32 rtl865x_usb_ioread32(u32 addr);
extern void rtl865x_usb_iowrite16(u32 addr, u16 val);
extern void rtl865x_usb_iowrite32(u32 addr, u32 val);

static inline u16 rtl865x_ioread16(u32 addr)
{
	return rtl865x_usb_ioread16(addr);
}

static inline u32 rtl865x_ioread32(u32 addr)
{
	return rtl865x_usb_ioread32(addr);
}

static inline void rtl865x_iowrite16(u32 addr, u16 val)
{
	rtl865x_usb_iowrite16(addr,val);
}

static inline void rtl865x_iowrite32(u32 addr, u32 val)
{
	rtl865x_usb_iowrite32(addr,val);
}

#undef readw
#undef readl
#undef writew
#undef writel

#define readw(addr) (rtl865x_ioread16((u32)addr))
#define readl(addr) (rtl865x_ioread32((u32)addr))
#define writew(val,addr) (rtl865x_iowrite16((u32)addr,val))
#define writel(val,addr) (rtl865x_iowrite32((u32)addr,val))

#endif

#endif /* _USB_IO_H */
