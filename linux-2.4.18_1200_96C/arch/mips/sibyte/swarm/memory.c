/*
 * Copyright (C) 2000, 2001 Broadcom Corporation
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

/*
 * Memory related routines 
 */

#include <linux/config.h>
#include <linux/types.h>
#include <asm/page.h>

extern phys_t swarm_mem_region_addrs[];
extern phys_t swarm_mem_region_sizes[];
extern unsigned int swarm_mem_region_count;

int page_is_ram(unsigned long pagenr)
{
	phys_t addr = pagenr << PAGE_SHIFT;
#ifdef CONFIG_SWARM_STANDALONE
	if (addr < (CONFIG_SIBYTE_SWARM_RAM_SIZE * 1024 * 1024)) {
		return 1;
	}
#else
	int i;
	for (i = 0; i < swarm_mem_region_count; i++) {
		if ((addr >= swarm_mem_region_addrs[i]) 
		    && (addr < (swarm_mem_region_addrs[i] + swarm_mem_region_sizes[i]))) {
			return 1;
		}
	}
#endif
	return 0;
}
