/*
 * Copyright (C) 2001 Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <linux/kernel.h>
#include <asm/mipsregs.h>

/* SB1 definitions */
#define CP0_ERRCTL_RECOVERABLE (1 << 31)
#define CP0_ERRCTL_DCACHE      (1 << 30)
#define CP0_ERRCTL_ICACHE      (1 << 29)
#define CP0_ERRCTL_MULTIBUS    (1 << 23)
#define CP0_ERRCTL_MC_TLB      (1 << 15)
#define CP0_ERRCTL_MC_TIMEOUT  (1 << 14)

#define CP0_CERRI_TAG_PARITY   (1 << 29)
#define CP0_CERRI_DATA_PARITY  (1 << 28)
#define CP0_CERRI_EXTERNAL     (1 << 26)
#define CP0_CERRI_CACHE_IDX    (0xff << 5)

#define CP0_CERRD_MULTIPLE     (1 << 31)
#define CP0_CERRD_TAG_STATE    (1 << 30)
#define CP0_CERRD_TAG_ADDRESS  (1 << 29)
#define CP0_CERRD_DATA_SBE     (1 << 28)
#define CP0_CERRD_DATA_DBE     (1 << 27)
#define CP0_CERRD_EXTERNAL     (1 << 26)
#define CP0_CERRD_LOAD         (1 << 25)
#define CP0_CERRD_STORE        (1 << 24)
#define CP0_CERRD_FILLWB       (1 << 23)
#define CP0_CERRD_COHERENCY    (1 << 22)
#define CP0_CERRD_DUPTAG       (1 << 21)
#define CP0_CERRD_CACHE_IDX    (0xff << 5)

asmlinkage void sb1_cache_error(void)
{
	unsigned int errctl, cerr_i, cerr_d, cerr_dpa;
	unsigned long eepc;

	eepc = get_errorepc();

	__asm__ __volatile__ (
		".set push\n"
		"#.set mips64\n"
		".set mips4\n"
		".word 0x4001D000; move %0, $1; # mfc0 %0, $26, 0\n"
		".word 0x4001D800; move %1, $1; # mfc0 %1, $27, 0\n"
		".word 0x4001D801; move %2, $1; # mfc0 %2, $27, 1\n"
		".word 0x4001D803; move %3, $1; # mfc0 %3, $27, 3\n"
		".set pop\n"
		: "=r" (errctl), "=r" (cerr_i),
		  "=r" (cerr_d), "=r" (cerr_dpa));

	printk("Cache error exception:\n");
	printk(" cp0_errorepc == 0x%0lx\n", eepc);
	printk(" cp0_errctl   == 0x%08x\n", errctl);

	if (errctl & CP0_ERRCTL_DCACHE) {
		printk(" cp0_cerr_d   == 0x%08x\n", cerr_d);
		printk(" cp0_cerr_dpa == 0x%08x\n", cerr_dpa);
	}
	if (errctl & CP0_ERRCTL_ICACHE) {
		printk(" cp0_cerr_i   == 0x%08x\n", cerr_i);
	}

	panic("Can't handle the cache error!");
}
