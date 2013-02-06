/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
*
* Abstract: PHY access header file.
*
* ---------------------------------------------------------------
*/

#ifndef _PHY_H_
#define _PHY_H_



#define PHY_CTRL_REG            0
#define PHY_STS_REG             1
#define PHY_ID1_REG             2
#define PHY_ID2_REG             3
#define PHY_ANADV_REG           4
#define PHY_ANLP_REG            5


uint32 phy_readReg(uint32 port, uint32 regnum);
int32 phy_writeReg(uint32 port, uint32 regnum, uint32 value);



#endif   /* _PHY_H_ */

