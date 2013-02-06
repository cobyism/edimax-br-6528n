/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /cvs/AP/rtl865x/linux-2.4.18/drivers/net/rtl865x/phy.h,v 1.1.1.1 2007/08/06 10:04:52 root Exp $
*
* Abstract: PHY access header file.
*
* $Author: root $
*
* $Log: phy.h,v $
* Revision 1.1.1.1  2007/08/06 10:04:52  root
* Initial import source to CVS
*
* Revision 1.3  2005/09/22 05:22:31  bo_zhao
* *** empty log message ***
*
* Revision 1.1.1.1  2005/09/05 12:38:24  alva
* initial import for add TFTP server
*
* Revision 1.2  2004/03/31 01:49:20  yjlou
* *: all text files are converted to UNIX format.
*
* Revision 1.1  2004/03/16 06:36:13  yjlou
* *** empty log message ***
*
* Revision 1.1.1.1  2003/09/25 08:16:56  tony
*  initial loader tree 
*
* Revision 1.1.1.1  2003/05/07 08:16:07  danwu
* no message
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

