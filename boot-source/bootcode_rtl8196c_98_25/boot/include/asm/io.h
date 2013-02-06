/* $Id: io.h,v 1.1 2009/11/13 13:22:46 jasonwang Exp $
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1994, 1995 Waldorf GmbH
 * Copyright (C) 1994 - 2000 Ralf Baechle
 * Copyright (C) 1999, 2000 Silicon Graphics, Inc.
 */
#ifndef _ASM_IO_H
#define _ASM_IO_H

/*
 * Slowdown I/O port space accesses for antique hardware.
 *	
 * Notice: just because this is mips you can't assume it has no broken
 * hardware. For example I've just inserted pcmcia ne2k into my 
 * philips velo. Ouch.
 */
#undef CONF_SLOWDOWN_IO

#include <asm/addrspace.h>

/*
 * This file contains the definitions for the MIPS counterpart of the
 * x86 in/out instructions. This heap of macros and C results in much
 * better code than the approach of doing it in plain C.  The macros
 * result in code that is to fast for certain hardware.  On the other
 * side the performance of the string functions should be improved for
 * sake of certain devices like EIDE disks that do highspeed polled I/O.
 *
 *   Ralf
 *
 * This file contains the definitions for the x86 IO instructions
 * inb/inw/inl/outb/outw/outl and the "string versions" of the same
 * (insb/insw/insl/outsb/outsw/outsl). You can also use "pausing"
 * versions of the single-IO instructions (inb_p/inw_p/..).
 *
 * This file is not meant to be obfuscating: it's just complicated
 * to (a) handle it all in a way that makes gcc able to optimize it
 * as well as possible and (b) trying to avoid writing the same thing
 * over and over again with slight variations and possibly making a
 * mistake somewhere.
 */

/*
 * On MIPS I/O ports are memory mapped, so we access them using normal
 * load/store instructions. mips_io_port_base is the virtual address to
 * which all ports are being mapped.  For sake of efficiency some code
 * assumes that this is an address that can be loaded with a single lui
 * instruction, so the lower 16 bits must be zero.  Should be true on
 * on any sane architecture; generic code does not use this assumption.
 */

#include <asm/delay.h>
//#define mips_io_port_base 0xbd010000
#define mips_io_port_base (0xB8000000)
//extern unsigned long mips_io_port_base;
#define __SLOW_DOWN_IO \
	{ int i; for (i=0; i<100; i++) barrier(); }

#ifdef REALLY_SLOW_IO
#define SLOW_DOWN_IO { __SLOW_DOWN_IO; __SLOW_DOWN_IO; __SLOW_DOWN_IO; __SLOW_DOWN_IO; }
#else
#define SLOW_DOWN_IO __SLOW_DOWN_IO
#endif


/*
 * Change virtual addresses to physical addresses and vv.
 * These are trivial on the 1:1 Linux/MIPS mapping
 */
extern inline unsigned long virt_to_phys(volatile void * address)
{
	return PHYSADDR(address);
}

extern inline void * phys_to_virt(unsigned long address)
{
	return (void *)KSEG0ADDR(address);
}

/*
 * IO bus memory addresses are also 1:1 with the physical address
 * Note: These functions are only to be used to translate between
 * bus and vitual for system RAM, not for shared peripheral RAM
 */
extern inline unsigned long virt_to_bus(volatile void * address)
{
	return PHYSADDR(address);
}

extern inline void * bus_to_virt(unsigned long address)
{
	return (void *)KSEG0ADDR(address);
}

/*
 * isa_slot_offset is the address where E(ISA) busaddress 0 is is mapped
 * for the processor.
 */
extern unsigned long isa_slot_offset;

/*
 * On MIPS, we have the whole physical address space mapped at all
 * times, so "ioremap()" and "iounmap()" do not need to do anything.
 * (This isn't true for all machines but we still handle these cases
 * with wired TLB entries anyway ...)
 *
 * We cheat a bit and always return uncachable areas until we've fixed
 * the drivers to handle caching properly.
 *
 * Note that although the return value of these functions looks like a
 * virtual address (and is used as such by read/write[bwl]), it cannot
 * simply be used as such, but must only be used with read/write[bwl]()
 * or mem*io(), below.
 *
 * The offset argument is supposedly a bus address, but we mask off the
 * non-significant bits because sometimes they contain extra information
 * (for systems with multiple PCI busses, at least).
 */
extern inline void * ioremap(unsigned long offset, unsigned long size)
{
#ifdef CONFIG_CPU_VR4122
	return (void *) KSEG1ADDR(offset);
#else
#ifdef CONFIG_ISA
	return (void *)(isa_slot_offset + (offset & 0x03ffffff));
#ifdef CONFIG_PCI
#error "using ISA mapping for PCI bus addresses, that's just wrong"
#endif
#else
	return (void *) KSEG1ADDR(offset);
#if 0 // I'm silencing this warning, because it makes r39xx compiles annoying
#warning "using 1:1 bus to pyhsical mapping, not sure if that's right"
#endif
#endif
#endif
}

/*
 * This one maps high address device memory and turns off caching for that area.
 * it's useful if some control registers are in such an area and write combining
 * or read caching is not desirable:
 */
extern inline void * ioremap_nocache (unsigned long offset, unsigned long size)
{
#ifdef CONFIG_CPU_VR4122
	return (void *) KSEG1ADDR(offset);
#else
#ifdef CONFIG_ISA
	return (void *) KSEG1ADDR(isa_slot_offset + (offset & 0x03ffffff));
#ifdef CONFIG_PCI
#error "using ISA mapping for PCI bus addresses, that's just wrong"
#endif
#else
	return (void *) KSEG1ADDR(offset);
#if 0 // this one too - note that these do need to be addressed for non-isa
#warning "using 1:1 bus to pyhsical mapping, not sure if that's right"
#endif
#endif
#endif
}

extern inline void iounmap(void *addr)
{
}

/*
 * readX/writeX() are used to access memory mapped devices. Some architectures
 * require special handling for this, but MIPS can just read/write the memory
 * location directly.
 *
 * Note that the argument to these is niether a virtual addresses, a physical
 * addresses, nor a bus addresses.  The address argument is a "ioremap cookie",
 * (cleverly disguised as a virtual address) as returned by ioremap().  The
 * return value of ioremap() should never be used directly and nothing but the
 * return value of ioremap() should be used with readX/writeX (or memXio).
 */

#define readb(addr) (*(volatile unsigned char *)(addr))
#define readw(addr) (*(volatile unsigned short *)(addr))
#define readl(addr) (*(volatile unsigned int *)(addr))
#define __raw_readb readb
#define __raw_readw readw
#define __raw_readl readl

#define writeb(b,addr) (*(volatile unsigned char *)(addr)) = (b)
#define writew(b,addr) (*(volatile unsigned short *)(addr)) = (b)
#define writel(b,addr) (*(volatile unsigned int *)(addr)) = (b)
#define __raw_writeb writeb
#define __raw_writew writew
#define __raw_writel writel

#define memset_io(a,b,c)	memset((void *)(a),(b),(c))
#define memcpy_fromio(a,b,c)	memcpy((a),(void *)(b),(c))
#define memcpy_toio(a,b,c)	memcpy((void *)(a),(b),(c))

/*
 * These are for convenience porting old ISA drivers, as an alternative to
 * adding a call to ioremap().  The proper thing to do is use ioremap(),
 * these are here mostly becuase they're easy to implement.
 */

#define isa_readb(a) readb(isa_slot_offset + (a))
#define isa_readw(a) readw(isa_slot_offset + (a))
#define isa_readl(a) readl(isa_slot_offset + (a))
#define isa_writeb(b,a) writeb(b,isa_slot_offset + (a))
#define isa_writew(w,a) writew(w,isa_slot_offset + (a))
#define isa_writel(l,a) writel(l,isa_slot_offset + (a))
#define isa_memset_io(a,b,c)		memset_io(isa_slot_offset + (a),(b),(c))
#define isa_memcpy_fromio(a,b,c)	memcpy_fromio((a),isa_slot_offset + (b),(c))
#define isa_memcpy_toio(a,b,c)		memcpy_toio(isa_slot_offset + (a),(b),(c)

/*
 * We don't have csum_partial_copy_fromio() yet, so we cheat here and
 * just copy it. The net code will then do the checksum later.
 */
#define eth_io_copy_and_sum(skb,src,len,unused) memcpy_fromio((skb)->data,(src),(len))
#define isa_eth_io_copy_and_sum(a,b,c,d) eth_copy_and_sum((a),(b),(c),(d))

static inline int check_signature(unsigned long io_addr,
                                  const unsigned char *signature, int length)
{
	int retval = 0;
	do {
		if (readb(io_addr) != *signature)
			goto out;
		io_addr++;
		signature++;
		length--;
	} while (length);
	retval = 1;
out:
	return retval;
}
#define isa_check_signature(io, s, l) check_signature(i,s,l)

/*
 * Talk about misusing macros..
 */

#define __OUT1(s) \
extern inline void __out##s(unsigned int value, unsigned int port) {

#define __OUT2(m) \
__asm__ __volatile__ ("s" #m "\t%0,%1(%2)"

#define __OUT(m,s) \
__OUT1(s) __OUT2(m) : : "r" (value), "i" (0), "r" (mips_io_port_base+port)); } \
__OUT1(s##c) __OUT2(m) : : "r" (value), "ir" (port), "r" (mips_io_port_base)); } \
__OUT1(s##_p) __OUT2(m) : : "r" (value), "i" (0), "r" (mips_io_port_base+port)); \
	SLOW_DOWN_IO; } \
__OUT1(s##c_p) __OUT2(m) : : "r" (value), "ir" (port), "r" (mips_io_port_base)); \
	SLOW_DOWN_IO; }

#define __IN1(t,s) \
extern __inline__ t __in##s(unsigned int port) { t _v;

/*
 * Required nops will be inserted by the assembler
 */
#define __IN2(m) \
__asm__ __volatile__ ("l" #m "\t%0,%1(%2)"

#define __IN(t,m,s) \
__IN1(t,s) __IN2(m) : "=r" (_v) : "i" (0), "r" (mips_io_port_base+port)); return _v; } \
__IN1(t,s##c) __IN2(m) : "=r" (_v) : "ir" (port), "r" (mips_io_port_base)); return _v; } \
__IN1(t,s##_p) __IN2(m) : "=r" (_v) : "i" (0), "r" (mips_io_port_base+port)); SLOW_DOWN_IO; return _v; } \
__IN1(t,s##c_p) __IN2(m) : "=r" (_v) : "ir" (port), "r" (mips_io_port_base)); SLOW_DOWN_IO; return _v; }

#define __INS1(s) \
extern inline void __ins##s(unsigned int port, void * addr, unsigned long count) {

#define __INS2(m) \
if (count) \
__asm__ __volatile__ ( \
	".set\tnoreorder\n\t" \
	".set\tnoat\n" \
	"1:\tl" #m "\t$1,%4(%5)\n\t" \
	"subu\t%1,1\n\t" \
	"s" #m "\t$1,(%0)\n\t" \
	"bne\t$0,%1,1b\n\t" \
	"addiu\t%0,%6\n\t" \
	".set\tat\n\t" \
	".set\treorder"

#define __INS(m,s,i) \
__INS1(s) __INS2(m) \
	: "=r" (addr), "=r" (count) \
	: "0" (addr), "1" (count), "i" (0), "r" (mips_io_port_base+port), "I" (i) \
	: "$1");} \
__INS1(s##c) __INS2(m) \
	: "=r" (addr), "=r" (count) \
	: "0" (addr), "1" (count), "ir" (port), "r" (mips_io_port_base), "I" (i) \
	: "$1");}

#define __OUTS1(s) \
extern inline void __outs##s(unsigned int port, const void * addr, unsigned long count) {

#define __OUTS2(m) \
if (count) \
__asm__ __volatile__ ( \
        ".set\tnoreorder\n\t" \
        ".set\tnoat\n" \
        "1:\tl" #m "\t$1,(%0)\n\t" \
        "subu\t%1,1\n\t" \
        "s" #m "\t$1,%4(%5)\n\t" \
        "bne\t$0,%1,1b\n\t" \
        "addiu\t%0,%6\n\t" \
        ".set\tat\n\t" \
        ".set\treorder"

#define __OUTS(m,s,i) \
__OUTS1(s) __OUTS2(m) \
	: "=r" (addr), "=r" (count) \
	: "0" (addr), "1" (count), "i" (0), "r" (mips_io_port_base+port), "I" (i) \
	: "$1");} \
__OUTS1(s##c) __OUTS2(m) \
	: "=r" (addr), "=r" (count) \
	: "0" (addr), "1" (count), "ir" (port), "r" (mips_io_port_base), "I" (i) \
	: "$1");}

__IN(unsigned char,b,b)
__IN(unsigned short,h,w)
__IN(unsigned int,w,l)

__OUT(b,b)
__OUT(h,w)
__OUT(w,l)

__INS(b,b,1)
__INS(h,w,2)
__INS(w,l,4)

__OUTS(b,b,1)
__OUTS(h,w,2)
__OUTS(w,l,4)


#define WP_PORT_BASE    0xBBF00000

#define wpreg_bread(buf, addr)  \
	do {  \
   	buf = (*(volatile unsigned char *)((addr)+WP_PORT_BASE));\
   	}  \
    while (0)
#define wpreg_hread(buf, addr)  \
    do {  \
    buf = (*(volatile unsigned short *)((addr)+WP_PORT_BASE));\
    }  \
    while (0)

#define wpreg_wread(buf, addr)  \
	do {  \
    buf = (*(volatile unsigned long *)((addr)+WP_PORT_BASE));\
    }  \
    while (0)
		
#define wpreg_bwrite(val, addr)  \
    do {  \
    (*(volatile unsigned char *)((addr)+WP_PORT_BASE))=(unsigned char)(val);\
    }  \
    while (0)

#define wpreg_hwrite(val, addr)  \
   do {  \
   (*(volatile unsigned short *)((addr)+WP_PORT_BASE))=(unsigned short)(val);\
   }  \
   while (0)

#define wpreg_wwrite(val, addr)  \
   do {  \
   (*(volatile unsigned long *)((addr)+WP_PORT_BASE))=(unsigned long)(val);\
   }  \
   while (0)
		
#ifdef CONFIG_MIPS_EV96100

#include <asm/byteorder.h>

#define inb(port)		in_8((u8 *)((port)+mips_io_port_base))
#define outb(val, port)		out_8((u8 *)((port)+mips_io_port_base), (val))
#define inw(port)		in_16((u16 *)((port)+mips_io_port_base))
#define outw(val, port)		out_16((u16 *)((port)+mips_io_port_base), (val))
#define inl(port)		in_32((u32 *)((port)+mips_io_port_base))
#define outl(val, port)		out_32((u32 *)((port)+mips_io_port_base), (val))

#define inb_p(port)		inb((port))
#define outb_p(val, port)	outb((val), (port))
#define inw_p(port)		inw((port))
#define outw_p(val, port)	outw((val), (port))
#define inl_p(port)		inl((port))
#define outl_p(val, port)	outl((val), (port))

extern inline unsigned char in_8(const unsigned char *addr)
{
    return *KSEG1ADDR(addr);
}

extern inline void out_8(unsigned char *addr, unsigned int val)
{
    *KSEG1ADDR(addr) = (unsigned char)val;
}

extern inline unsigned short in_16(const unsigned short *addr)
{
    return (le16_to_cpu(*KSEG1ADDR(addr)));
}

extern inline void out_16(unsigned short *addr, unsigned int val)
{
    *KSEG1ADDR(addr) = cpu_to_le16((unsigned short)val);
}

extern inline unsigned int in_32(const unsigned int *addr)
{
    return (le32_to_cpu(*KSEG1ADDR(addr)));
}

extern inline void out_32(unsigned int *addr, unsigned int val)
{
    *KSEG1ADDR(addr) = cpu_to_le32((unsigned int)val);
}

#else

/*
 * Note that due to the way __builtin_constant_p() works, you
 *  - can't use it inside an inline function (it will never be true)
 *  - you don't have to worry about side effects within the __builtin..
 */
#define outb(val,port) \
((__builtin_constant_p((port)) && (port) < 32768) ? \
	__outbc((val),(port)) : \
	__outb((val),(port)))

#define inb(port) \
((__builtin_constant_p((port)) && (port) < 32768) ? \
	__inbc(port) : \
	__inb(port))


#define outb_p(val,port) \
((__builtin_constant_p((port)) && (port) < 32768) ? \
	__outbc_p((val),(port)) : \
	__outb_p((val),(port)))

#define inb_p(port) \
((__builtin_constant_p((port)) && (port) < 32768) ? \
	__inbc_p(port) : \
	__inb_p(port))

#define outw(val,port) \
((__builtin_constant_p((port)) && (port) < 32768) ? \
	__outwc((val),(port)) : \
	__outw((val),(port)))

#define inw(port) \
((__builtin_constant_p((port)) && (port) < 32768) ? \
	__inwc(port) : \
	__inw(port))

#define outw_p(val,port) \
((__builtin_constant_p((port)) && (port) < 32768) ? \
	__outwc_p((val),(port)) : \
	__outw_p((val),(port)))

#define inw_p(port) \
((__builtin_constant_p((port)) && (port) < 32768) ? \
	__inwc_p(port) : \
	__inw_p(port))

#define outl(val,port) \
((__builtin_constant_p((port)) && (port) < 32768) ? \
	__outlc((val),(port)) : \
	__outl((val),(port)))

#define inl(port) \
((__builtin_constant_p((port)) && (port) < 32768) ? \
	__inlc(port) : \
	__inl(port))

#define outl_p(val,port) \
((__builtin_constant_p((port)) && (port) < 32768) ? \
	__outlc_p((val),(port)) : \
	__outl_p((val),(port)))

#define inl_p(port) \
((__builtin_constant_p((port)) && (port) < 32768) ? \
	__inlc_p(port) : \
	__inl_p(port))


#define outsb(port,addr,count) \
((__builtin_constant_p((port)) && (port) < 32768) ? \
	__outsbc((port),(addr),(count)) : \
	__outsb ((port),(addr),(count)))

#define insb(port,addr,count) \
((__builtin_constant_p((port)) && (port) < 32768) ? \
	__insbc((port),(addr),(count)) : \
	__insb((port),(addr),(count)))

#define outsw(port,addr,count) \
((__builtin_constant_p((port)) && (port) < 32768) ? \
	__outswc((port),(addr),(count)) : \
	__outsw ((port),(addr),(count)))

#define insw(port,addr,count) \
((__builtin_constant_p((port)) && (port) < 32768) ? \
	__inswc((port),(addr),(count)) : \
	__insw((port),(addr),(count)))

#define outsl(port,addr,count) \
((__builtin_constant_p((port)) && (port) < 32768) ? \
	__outslc((port),(addr),(count)) : \
	__outsl ((port),(addr),(count)))

#define insl(port,addr,count) \
((__builtin_constant_p((port)) && (port) < 32768) ? \
	__inslc((port),(addr),(count)) : \
	__insl((port),(addr),(count)))
#endif

#ifdef CONFIG_CPU_VR4122
#define IO_SPACE_LIMIT 0xffffffff
#else
#define IO_SPACE_LIMIT 0xffff
#endif

/*
 * The caches on some architectures aren't dma-coherent and have need to
 * handle this in software.  There are three types of operations that
 * can be applied to dma buffers.
 *
 *  - dma_cache_wback_inv(start, size) makes caches and coherent by
 *    writing the content of the caches back to memory, if necessary.
 *    The function also invalidates the affected part of the caches as
 *    necessary before DMA transfers from outside to memory.
 *  - dma_cache_wback(start, size) makes caches and coherent by
 *    writing the content of the caches back to memory, if necessary.
 *    The function also invalidates the affected part of the caches as
 *    necessary before DMA transfers from outside to memory.
 *  - dma_cache_inv(start, size) invalidates the affected parts of the
 *    caches.  Dirty lines of the caches may be written back or simply
 *    be discarded.  This operation is necessary before dma operations
 *    to the memory.
 */
extern void (*_dma_cache_wback_inv)(unsigned long start, unsigned long size);
extern void (*_dma_cache_wback)(unsigned long start, unsigned long size);
extern void (*_dma_cache_inv)(unsigned long start, unsigned long size);

#define dma_cache_wback_inv(start,size)	_dma_cache_wback_inv(start,size)
#define dma_cache_wback(start,size)	_dma_cache_wback(start,size)
#define dma_cache_inv(start,size)	_dma_cache_inv(start,size)

#endif /* _ASM_IO_H */
