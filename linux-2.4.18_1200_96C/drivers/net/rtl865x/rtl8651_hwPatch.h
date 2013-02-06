/*
 *
 *	The header file collects the information about the IC's different Version/Sub-version.
 *		=> It's used by DRIVER to determine how to implement features for different ICs.
 *
 */

#ifndef RTL8651_hwPatch_H
#define RTL8651_hwPatch_H

/* Chip Version */
#define RTL865X_CHIP_VER_RTL865XB	0x01
#define RTL865X_CHIP_VER_RTL865XC	0x02
#define RTL865X_CHIP_VER_RTL8196B	0x03

/* Revision ID of each chip */
#define RTL865X_CHIP_REV_A		0x00
#define RTL865X_CHIP_REV_B		0x01
#define RTL865X_CHIP_REV_C		0x02
#define RTL865X_CHIP_REV_D		0x03
#define RTL865X_CHIP_REV_E		0x04

/* features' and patchs' related definitions */

/* =====================================================================
    =
    =				ADD feature-related MACROs HERE
    =
    =					=> Please ALWAYS add "CHIP" and "REV" for MACROs
    = 
    ===================================================================== */

/* RTL865xB */
/* ==================================  */
/* Rev-A/B/C: Their Refill token interval of Rate Limit table is large */
#define RTL865X_LARGE_RATELIMIT_REFILLINTERVAL	(	(( RtkHomeGatewayChipNameID == RTL865X_CHIP_VER_RTL865XB	/* CHIP */ ) && \
														( RtkHomeGatewayChipRevisionID <= RTL865X_CHIP_REV_C		/* REV */ )) || \
														( RtkHomeGatewayChipNameID == RTL865X_CHIP_VER_RTL865XC		/* CHIP */ )	|| \
														( RtkHomeGatewayChipNameID == RTL865X_CHIP_VER_RTL8196B		/* CHIP */ ))


/* RTL865xC */
/* ================================== */
/* Rev-A: It would cause HW process when dynamically stop HW table lookup. */
#define  RTL865X_TLU_BUG_FIXED		(	(( RtkHomeGatewayChipNameID == RTL865X_CHIP_VER_RTL865XC	/* CHIP */ ) && \
										( RtkHomeGatewayChipRevisionID >= RTL865X_CHIP_REV_B		/* REV */ ))	|| \
										( RtkHomeGatewayChipNameID == RTL865X_CHIP_VER_RTL8196B		/* CHIP */ ))

/* Rev-A: The PHY-ID of PHY6 is set to 0 and conflicts to PHY0 */
#define  RTL865X_PHY6_DSP_BUG		(	( RtkHomeGatewayChipNameID == RTL865X_CHIP_VER_RTL865XC	/* CHIP */ ) && \
										(RtkHomeGatewayChipRevisionID == RTL865X_CHIP_REV_A		/* REV */ )	)

/* 
	Patch for A-CUT & B-Cut RTL865xC hardware bug:
		if A2EO = (2*n) 8 byte		cipher OK
		if A2EO = (2*n+1) 8 byte	the sequence of cipher would be reversal in 8-byte units
	this bug would be fixed in C-Cut and later version
*/
#define  RTL865X_AES_BUG (( RtkHomeGatewayChipNameID == RTL865X_CHIP_VER_RTL865XC	/*CHIP*/)&& (RtkHomeGatewayChipRevisionID < RTL865X_CHIP_REV_C	/*REV*/))

/*
 *	Patch for All RTL865xC default value setting problem.
 *		The default value of IQFCTCR in ASIC is opposite to correct one.
 *
 */
#define	 RTL865X_IQFCTCR_DEFAULT_VALUE_BUG (	(RtkHomeGatewayChipNameID == RTL865X_CHIP_VER_RTL865XC	/* CHIP*/) && \
						( 1 /* ALL Revisions */ )	)

/*
 *	Patch for All RTL865xC NAT-base Priority bit3 auto-clean problem.
 *		The nat base priority should be always<4.
 *
 */
#define	 RTL865X_NATBASE_PRIOTIY_AUTOCLEAN_BUG (	(RtkHomeGatewayChipNameID == RTL865X_CHIP_VER_RTL865XC	/* CHIP*/) && \
						( RtkHomeGatewayChipRevisionID < RTL865X_CHIP_REV_D /* ALL Revisions */ )	)

/*
 *	Patch for All RTL865xC TX Link Down problem.
 *		mantis bug 2570
 *
 */
#define	 RTL865X_TX_LINKDOWN_BUG (	(RtkHomeGatewayChipNameID == RTL865X_CHIP_VER_RTL865XC	/* CHIP*/) && \
						( RtkHomeGatewayChipRevisionID < RTL865X_CHIP_REV_D /* ALL Revisions */ )	)

/*
 *Patch for RTL865xC D-cut new ip multicast hash algorithm.
 *	
 */
#define	RTL865X_IP_MCAST_NEW_HASH (	((RtkHomeGatewayChipNameID == RTL865X_CHIP_VER_RTL865XC	/* CHIP*/) && \
						( RtkHomeGatewayChipRevisionID >= RTL865X_CHIP_REV_D /*REV */ ))	 || \
						( RtkHomeGatewayChipNameID == RTL865X_CHIP_VER_RTL8196B		/* CHIP */ ))

/* =============================================================================== */

#endif	

