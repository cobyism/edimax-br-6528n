 /*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 */

#ifndef	NIC_H
#define NIC_H

/*
 *	Structure returned from eth_probe and passed to other driver
 *	functions.
 */

struct nic
{
	char		*packet;
	unsigned int	packetlen;
};

#endif	/* NIC_H */
