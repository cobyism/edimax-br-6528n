/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : rtl8651_tblAsicDrv_cmd.c
* Abstract : RTL8651 Home gateway controller ASIC Command
* Author : Chun-Feng Liu(cfliu@realtek.com.tw)
* $Id: rtl8651_tblAsicDrv_cmd.c,v 1.1 2007/12/04 11:54:01 joeylin Exp $
*/
#include "rtl_types.h"
#include "rtl_utils.h"
#include "types.h"
#include "rtl_cle.h" 
#include "asicregs.h"
#ifndef CONFIG_RTL865X_LIGHT_ROMEDRV
#include "rtl8651_layer2local.h"
#endif
#ifdef CONFIG_RTL865XC
	#include "rtl865xC_tblAsicDrv.h"
#else
	#include "rtl8651_tblAsicDrv.h"
#endif
#ifdef RTL865X_MODEL_USER
	#include <string.h>
#else
	#include <linux/string.h>
#endif
#ifndef CONFIG_RTL865X_LIGHT_ROMEDRV
#ifdef CONFIG_RTL865XC
	#include "rtl865xc_gpio_cmd.h"
#endif
#endif

#ifdef CONFIG_RTL865X_LIGHT_ROMEDRV
	#include "rtl865x_lightrome.h"
#endif

#define PHY_SPEED_DUPLEX 	5
static int8 _phySpeedDuplex[PHY_SPEED_DUPLEX][5]={
	"100f", "100h", "10f", "10h", "an"
};

static int32	_rtl8651_phyCmd(uint32 userId,  int32 argc,int8 **saved){
	int32 size, result;
	int8 *nextToken;
	uint32 i, port;
	
	cle_getNextCmdToken(&nextToken,&size,saved); 
	if(size == 0)
		port = RTL8651_PORT_NUMBER;
	else
		port=U32_value(nextToken);
	
	cle_getNextCmdToken(&nextToken,&size,saved); 
	if(!strcmp(nextToken, "an")) {
		if(port == RTL8651_PORT_NUMBER) {
			for(i=0; i< RTL8651_PORT_NUMBER; i++)
				rtl8651_setAsicEthernetPHY0( i, TRUE, RTL8651_ETHER_AUTO_100FULL, 100, TRUE);
			result = SUCCESS;
		}
		else
			result = rtl8651_setAsicEthernetPHY0(port, TRUE, RTL8651_ETHER_AUTO_100FULL, 100, TRUE);
	}
	else if(!strcmp(nextToken, "100f")){
		if(port == RTL8651_PORT_NUMBER) {
			for(i=0; i< RTL8651_PORT_NUMBER; i++)
				rtl8651_setAsicEthernetPHY0( i, FALSE, RTL8651_ETHER_AUTO_100FULL, 100, TRUE);
			result = SUCCESS;
		}
		else
			result = rtl8651_setAsicEthernetPHY0(port, FALSE, RTL8651_ETHER_AUTO_100FULL, 100, TRUE);
	}
	else if(!strcmp(nextToken, "100h")){
		if(port == RTL8651_PORT_NUMBER) {
			for(i=0; i< RTL8651_PORT_NUMBER; i++)
				rtl8651_setAsicEthernetPHY0( i, FALSE, RTL8651_ETHER_AUTO_100FULL, 100, FALSE);
			result = SUCCESS;
		}
		else
			result = rtl8651_setAsicEthernetPHY0(port, FALSE, RTL8651_ETHER_AUTO_100FULL, 100, FALSE);
	}
	else if(!strcmp(nextToken, "10f")){
		if(port == RTL8651_PORT_NUMBER) {
			for(i=0; i< RTL8651_PORT_NUMBER; i++)
				rtl8651_setAsicEthernetPHY0( i, FALSE, RTL8651_ETHER_AUTO_100FULL, 10, TRUE);
			result = SUCCESS;
		}
		else
			result = rtl8651_setAsicEthernetPHY0(port, FALSE, RTL8651_ETHER_AUTO_100FULL, 10, TRUE);
	}
	else {
		if(port == RTL8651_PORT_NUMBER) {
			for(i=0; i< RTL8651_PORT_NUMBER; i++)
				rtl8651_setAsicEthernetPHY0( i, FALSE, RTL8651_ETHER_AUTO_100FULL, 10, FALSE);
			result = SUCCESS;
		}
		else
			result = rtl8651_setAsicEthernetPHY0(port, FALSE, RTL8651_ETHER_AUTO_100FULL, 10, FALSE);
	}
	return result;
}

/*
	command to config ASIC registers
*/
static int32 _rtl8651_registerCmd(uint32 userId, int32 argc, int8 **saved)
{
	int32 size;
	char *nextToken;
	char registerName[64];
	uint32 isSet;

	/* set or get ? */
	/* =========================================================== */
	RTL_PROC_CHECK(cle_getNextCmdToken(&nextToken, &size, saved), SUCCESS);

	if ( RTL_STREAM_SAME(nextToken, "set") )
	{
		isSet = TRUE;
	}
	else if ( RTL_STREAM_SAME(nextToken, "get") )
	{
		isSet = FALSE;
	}
	else
	{
		goto errout;
	}

	/* register name */
	/* =========================================================== */
	RTL_PROC_CHECK(cle_getNextCmdToken(&nextToken, &size, saved), SUCCESS);

	memset(registerName, 0, sizeof(registerName));
	strncpy(registerName, nextToken, sizeof(registerName));

	if ( RTL_STREAM_SAME(nextToken, "mmtu") )
	{
		uint32 mmtu;

		if ( isSet == TRUE )
		{
			/* Set MMTU */
			RTL_PROC_CHECK(cle_getNextCmdToken(&nextToken, &size, saved), SUCCESS);
			mmtu = U32_value(nextToken);

			RTL_PROC_CHECK(rtl8651_setAsicMulticastMTU(mmtu), SUCCESS);

			return SUCCESS;
		} else
		{
			/* Get MMTU */
			RTL_PROC_CHECK(rtl8651_getAsicMulticastMTU(&mmtu), SUCCESS);
			rtlglue_printf("%s = %d\n", registerName, mmtu);

			return SUCCESS;
		}
	} else if( RTL_STREAM_SAME(nextToken, "scr") )
	{
#ifdef CONFIG_RTL865XC
		uint32 clock, val;
		
		if ( isSet == TRUE )
		{		
			/* Set CPU SCR */
			RTL_PROC_CHECK(cle_getNextCmdToken(&nextToken, &size, saved), SUCCESS);			
			clock = U32_value(nextToken);			
			REG32(DPLCR0) |= EN_S0DPLL | EN_S1DPLL;		
			REG32(DPLCR0) = 0;
			
			switch(clock)
			{
				case 0: val = S0UPDPLL_VALUE_225MHZ; break;
				case 1: val = S0UPDPLL_VALUE_250MHZ; break;
				case 2: val = S0UPDPLL_VALUE_275MHZ; break;
				case 3: val = S0UPDPLL_VALUE_300MHZ; break;
				case 4: val = S0UPDPLL_VALUE_325MHZ; break;
				case 5: val = S0UPDPLL_VALUE_350MHZ; break;
				case 6: val = S0UPDPLL_VALUE_375MHZ; break;
				case 7: val = S0UPDPLL_VALUE_400MHZ; break;
				default:
					val = S0UPDPLL_VALUE_DEFAULT;
					rtlglue_printf("Return to default cpu clock rate\n");
					break;
			}			
			REG32(DPLCR0) |= EN_S0DPLL | S0DOWNDPLL_VALUE | val;
			goto getscr;
		} else
		{
getscr:
			/* Get CPU SCR */
			REG32(DPLCR0) |= EN_S0DPLL | EN_S1DPLL;						
			uint32 s0updpll = (REG32(DPLCR0)>>S0UPDPLL_OFFSET) & S0UPDPLL_MASK;
			uint32 s0downdpll = (REG32(DPLCR0)>>S0DOWNDPLL_OFFSET) & S0DOWNDPLL_MASK;				
			if(s0updpll || s0downdpll) {
				uint32 s0cr = 125*(s0updpll+1)/(s0downdpll+1);			
				rtlglue_printf("CPU Clock = %dMHz\n", s0cr);
			} else {
				 uint32 scr = (REG32(SCCR)>>SCCR_CPU_OFFSET) & SCCR_STATUS_MASK;
				 char *str;
				 switch( scr )
    				 {
        				case 0: str = "100MHz"; break;
        				case 1: str = "200MHz"; break;
        				case 2: str = "250MHz"; break;
        				case 3: str = "260MHz"; break;
        				case 4: str = "270MHz"; break;
        				case 5: str = "280MHz"; break;
        				case 6: str = "290MHz"; break;
        				case 7: str = "300MHz"; break;
        				default:str = "unknown"; break;
    				}
				rtlglue_printf("CPU Clock = %s\n", str);
			}
			return SUCCESS;
		}
#else
		rtlglue_printf("CLE command for scr control is supported with RTL865xC\n");
#endif
	} else if( RTL_STREAM_SAME(nextToken, "mcr") )
	{
#ifdef CONFIG_RTL865XC
		uint32 clock, val;
		
		if ( isSet == TRUE )
		{		
			/* Set MCR */
			RTL_PROC_CHECK(cle_getNextCmdToken(&nextToken, &size, saved), SUCCESS);
			clock = U32_value(nextToken);
			REG32(DPLCR1) |= EN_MDPLL;		
			REG32(DPLCR1) = 0;
			
			switch(clock)
			{				
				case 0: val = MUPDPLL_VALUE_137MHZ; break;
				case 1: val = MUPDPLL_VALUE_150MHZ; break;
				case 2: val = MUPDPLL_VALUE_162MHZ; break;
				case 3: val = MUPDPLL_VALUE_175MHZ; break;
				case 4: val = MUPDPLL_VALUE_187MHZ; break;
				default:
					val = MUPDPLL_VALUE_DEFAULT;
					rtlglue_printf("Return to default memory clock rate\n");
					break;
			}
			REG32(DPLCR1) |= EN_MDPLL | MDOWNDPLL_VALUE | val;	
			goto getmcr;			
		} else
		{
getmcr:
			/* Get MCR */
			REG32(DPLCR1) |= EN_MDPLL;			
			uint32 mupdpll = (REG32(DPLCR1)>>MUPDPLL_OFFSET) & MUPDPLL_MASK;
			uint32 mdowndpll = (REG32(DPLCR1)>>MDOWNDPLL_OFFSET) & MDOWNDPLL_MASK;			
			if(mupdpll || mdowndpll) {
				uint32 mcr = 125*(mupdpll+1)/(mdowndpll+1);			
				rtlglue_printf("Memory Clock = %dMHz\n", mcr);
			} else {
				 uint32 mcr = REG32(SCCR) & SCCR_STATUS_MASK;
				 char *str;
				 switch( mcr )
    				 {
        				case 0: str = "50MHz"; break;
                    		case 1: str = "120MHz"; break;
                    		case 2: str = "130MHz"; break;
                    		case 3: str = "140MHz"; break;
                    		case 4: str = "150MHz"; break;
                    		case 5: str = "160MHz"; break;
                    		case 6: str = "170MHz"; break;
                    		case 7: str = "180MHz"; break;
                    		default:str = "unknown"; break;
    				}
				rtlglue_printf("Memory Clock = %s\n", str);
			}			
			return SUCCESS;
		}
#else
		rtlglue_printf("CLE command for mcr control is supported with RTL865xC\n");
#endif
	}


errout:
	return FAILED;;
}

#ifdef CONFIG_RTL865XC
static int32	_rtl8651_qosCmd(uint32 userId,  int32 argc,int8 **saved){
	int32 size, result;
	int8 *nextToken;

	cle_getNextCmdToken(&nextToken,&size,saved); 
	if (!strcmp(nextToken, "set"))
	{
		cle_getNextCmdToken(&nextToken,&size,saved); 
		if (!strcmp(nextToken, "lb"))
		{
			uint32 token, tick, hiThreshold;

			cle_getNextCmdToken(&nextToken, &size, saved); 
			token = U32_value(nextToken);
			cle_getNextCmdToken(&nextToken, &size, saved); 
			tick = U32_value(nextToken);
			cle_getNextCmdToken(&nextToken, &size, saved); 
			hiThreshold = U32_value(nextToken);

			result = rtl8651_setAsicLBParameter( token, tick, hiThreshold );
			return result;
		}
		else if (!strcmp(nextToken, "qrate"))
		{
			enum PORTID port;
			enum QUEUEID queueid;
			uint32 pprTime, aprBurstSize, apr;
			
			cle_getNextCmdToken(&nextToken, &size, saved); 
			port = U32_value(nextToken);
			cle_getNextCmdToken(&nextToken, &size, saved); 
			queueid = U32_value(nextToken);
			cle_getNextCmdToken(&nextToken, &size, saved); 
			pprTime = U32_value(nextToken);
			cle_getNextCmdToken(&nextToken, &size, saved); 
			aprBurstSize = U32_value(nextToken);
			cle_getNextCmdToken(&nextToken, &size, saved); 
			apr = U32_value(nextToken);
		
			result = rtl8651_setAsicQueueRate( port, queueid, pprTime, aprBurstSize, apr );
			return result;
		}
		else if (!strcmp(nextToken, "pingressbandwidth"))
		{
			enum PORTID port;
			uint32 bandwidth;
			
			cle_getNextCmdToken(&nextToken, &size, saved); 
			port = U32_value(nextToken);
			cle_getNextCmdToken(&nextToken, &size, saved); 
			bandwidth = U32_value(nextToken);
		
			result = rtl8651_setAsicPortIngressBandwidth( port, bandwidth);
			return result;
		}		
		else if (!strcmp(nextToken, "pegressbandwidth"))
		{
			enum PORTID port;
			uint32 bandwidth;
			
			cle_getNextCmdToken(&nextToken, &size, saved); 
			port = U32_value(nextToken);
			cle_getNextCmdToken(&nextToken, &size, saved); 
			bandwidth = U32_value(nextToken);
		
			result = rtl8651_setAsicPortEgressBandwidth( port, bandwidth );
			return result;
		}
		else if (!strcmp(nextToken, "qweight"))
		{
			enum PORTID port;
			enum QUEUEID queueid;
			enum QUEUETYPE queueType;
			uint32 weight;
			
			cle_getNextCmdToken(&nextToken, &size, saved); 
			port = U32_value(nextToken);
			cle_getNextCmdToken(&nextToken, &size, saved); 
			queueid = U32_value(nextToken);
			cle_getNextCmdToken(&nextToken, &size, saved); 
			queueType = U32_value(nextToken);
			cle_getNextCmdToken(&nextToken, &size, saved); 
			weight = U32_value(nextToken);

			if (weight !=0)
				weight -= 1;
			result = rtl8651_setAsicQueueWeight( port, queueid, queueType, weight );
			return result;
		}
		else if (!strcmp(nextToken, "qnum"))
		{
			enum PORTID port;
			enum QUEUENUM qnum;
	
			cle_getNextCmdToken(&nextToken, &size, saved); 
			port = U32_value(nextToken);
			cle_getNextCmdToken(&nextToken, &size, saved); 
			qnum = U32_value(nextToken);

			result = rtl8651_setAsicOutputQueueNumber(port, qnum);
			return result;
		}
		else if (!strcmp(nextToken, "1prmEN"))
		{
			enum PORTID port;
			uint32 isEnable;
			
			cle_getNextCmdToken(&nextToken, &size, saved); 
			port = U32_value(nextToken);
			cle_getNextCmdToken(&nextToken, &size, saved); 
			isEnable = U32_value(nextToken);
		
			result = rtl8651_setAsicDot1pRemarkingAbility(port, isEnable);
			return result;
		}
		else if (!strcmp(nextToken, "1prm"))
		{
			enum PRIORITYVALUE priority, newpriority;
			
			cle_getNextCmdToken(&nextToken, &size, saved); 
			priority = U32_value(nextToken);
			cle_getNextCmdToken(&nextToken, &size, saved); 
			newpriority = U32_value(nextToken);
		
			result = rtl8651_setAsicDot1pRemarkingParameter(priority, newpriority);
			return result;
		}
		else if (!strcmp(nextToken, "dscprmEN"))
		{
			enum PORTID port;
			uint32 isEnable;
			
			cle_getNextCmdToken(&nextToken, &size, saved); 
			port = U32_value(nextToken);
			cle_getNextCmdToken(&nextToken, &size, saved); 
			isEnable = U32_value(nextToken);
		
			result = rtl8651_setAsicDscpRemarkingAbility(port, isEnable);
			return result;
		}
		else if (!strcmp(nextToken, "dscprm"))
		{
			enum PRIORITYVALUE priority;
			uint32 newdscp;
			
			cle_getNextCmdToken(&nextToken, &size, saved); 
			priority = U32_value(nextToken);
			cle_getNextCmdToken(&nextToken, &size, saved); 
			newdscp = U32_value(nextToken);
		
			result = rtl8651_setAsicDscpRemarkingParameter(priority, newdscp);
			return result;
		}
		else if (!strcmp(nextToken, "pridecision"))
		{
			uint32 portpri, dot1qpri, dscppri, aclpri, natpri;

			cle_getNextCmdToken(&nextToken, &size, saved); 
			portpri = U32_value(nextToken);
			cle_getNextCmdToken(&nextToken, &size, saved); 
			dot1qpri = U32_value(nextToken);
			cle_getNextCmdToken(&nextToken, &size, saved); 
			dscppri = U32_value(nextToken);
			cle_getNextCmdToken(&nextToken, &size, saved); 
			aclpri = U32_value(nextToken);
			cle_getNextCmdToken(&nextToken, &size, saved); 
			natpri = U32_value(nextToken);
		
			result = rtl8651_setAsicPriorityDecision(portpri, dot1qpri, dscppri, aclpri, natpri);
			return result;
		}
		else if (!strcmp(nextToken, "portpri"))
		{
			enum PORTID port;
			enum PRIORITYVALUE priority;
			
			cle_getNextCmdToken(&nextToken, &size, saved); 
			port = U32_value(nextToken);
			cle_getNextCmdToken(&nextToken, &size, saved); 
			priority = U32_value(nextToken);
		
			result = rtl8651_setAsicPortPriority(port, priority);   
			return result;
		}
		else if (!strcmp(nextToken, "1qpri"))
		{
			enum PRIORITYVALUE srcpriority;
			enum PRIORITYVALUE priority;
			
			cle_getNextCmdToken(&nextToken, &size, saved); 
			srcpriority = U32_value(nextToken);
			cle_getNextCmdToken(&nextToken, &size, saved); 
			priority = U32_value(nextToken);
		
			result = rtl8651_setAsicDot1qAbsolutelyPriority(srcpriority, priority);
			return result;
		}
		else if (!strcmp(nextToken, "dscppri"))
		{
			enum PRIORITYVALUE priority;
			uint32 dscp;
			
			cle_getNextCmdToken(&nextToken, &size, saved); 
			dscp = U32_value(nextToken);
			cle_getNextCmdToken(&nextToken, &size, saved); 
			priority = U32_value(nextToken);
		
			result = rtl8651_setAsicDscpPriority(dscp, priority);
			return result;
		}
		else if (!strcmp(nextToken, "pritoqid"))
		{
			enum PRIORITYVALUE priority;
			enum QUEUEID qid;
			uint32 qnum;
			
			cle_getNextCmdToken(&nextToken, &size, saved); 
			qnum = U32_value(nextToken);
			cle_getNextCmdToken(&nextToken, &size, saved); 
			priority = U32_value(nextToken);
			cle_getNextCmdToken(&nextToken, &size, saved); 
			qid = U32_value(nextToken);

			result = rtl8651_setAsicPriorityToQIDMappingTable(qnum, priority, qid);
			return result;
		}
#if 1 /* @	The command will be removed after verifying 865xC QoS function. */
		else if (!strcmp(nextToken, "init"))
		{
			result = rtl8651_initQoSParameter();
			return result;
		}
#endif		
		else
			return FAILED;

	}
	else if (!strcmp(nextToken, "get"))
	{
		cle_getNextCmdToken(&nextToken, &size, saved); 
		if (!strcmp(nextToken, "lb"))
		{
			uint32 token, tick, hiThreshold;
			int32 bandwidth;
			int32 err;
			result = rtl8651_getAsicLBParameter( &token, &tick, &hiThreshold );

			if ( result==SUCCESS )
			{
				rtlglue_printf( "\nLeacky Bucket Parameter\n\n" );
				rtlglue_printf( "Refill cycle: %d, refill token: %d, hiThreshold: %d\n", tick, token, hiThreshold );
				rtlglue_printf( "\nAssume SWCORE clock is 100MHz, the refill rate:\n" );

				/* compute error for 1Gbps */
				bandwidth = (100000000/tick)/(1<<14)*(1000000000/64000)*token*8;
				err = (bandwidth-1000000000)*10000/1000000000;
				rtlglue_printf( "  for 1Gbps is %d (error:%2d.%02d%%)\n", bandwidth, err/100, abs(err%100) );
				
				/* compute error for 1Mbps */
				bandwidth = (100000000/tick)/(1<<14)*(1000000/64000)*token*8;
				err = (bandwidth-1000000)*10000/1000000;
				rtlglue_printf( "  for 1Mbps is %d (error:%2d.%02d%%)\n", bandwidth, err/100, abs(err%100) );
				
				/* compute error for 64Kbps */
				bandwidth = (100000000/tick)/(1<<14)*(64000/64000)*token*8;
				err = (bandwidth-64000)*10000/64000;
				rtlglue_printf( "  for 64Kbps is %d (error:%2d.%02d%%)\n", bandwidth, err/100, abs(err%100) );
			}
			return result;
		}
		else if (!strcmp(nextToken, "qrate"))
		{
			enum PORTID port;
			enum QUEUEID queueid;
			uint32 pprTime, aprBurstSize, apr;
			uint32 ppr;
			
			cle_getNextCmdToken(&nextToken, &size, saved); 
			port = U32_value(nextToken);
			cle_getNextCmdToken(&nextToken, &size, saved); 
			queueid = U32_value(nextToken);
		
			result = rtl8651_getAsicQueueRate( port, queueid, &pprTime, &aprBurstSize, &apr );

			if ( result==SUCCESS )
			{
				int	i=0;

				ppr = 1;
				for(i=0;i<pprTime;i++)
				{
					ppr *= 2;
				}
				ppr = apr*ppr*64;
				rtlglue_printf( "\n[Port=%d, Queue=%d] ", port, queueid );
				
				if ( aprBurstSize==0xff )
					rtlglue_printf( "ARP Burst is disabled " );
				else
					rtlglue_printf( "ARP Burst=%dKBytes ", aprBurstSize );
					
				if ( apr==0x3fff )
					rtlglue_printf( "ARP is unlimited " );
				else
				{
					if ( apr*64000 > 1000000 )
						rtlglue_printf( "APR=%d.%dMbps ", apr*64/1000, (apr*64)%1000 );
					else
						rtlglue_printf( "APR=%dKbps ", apr*64 );

					if ( pprTime==7 )
						rtlglue_printf( "PPR is disabled " );
					else
						rtlglue_printf( "PPR=%dKBytes ", ppr );
				}
				rtlglue_printf( "\n\n" );
			}
			return result;
		}
		else if (!strcmp(nextToken, "pingressbandwidth"))
		{
			enum PORTID port;
			uint32 bandwidth;
			uint32 rate;
            
			cle_getNextCmdToken(&nextToken, &size, saved); 
			port = U32_value(nextToken);
		
			result = rtl8651_getAsicPortIngressBandwidth( port, &bandwidth );
			if ( result==SUCCESS )
			{
				rtlglue_printf( "\n[Port=%d] ", port );
                            #if 1
                            rate = bandwidth*16;
				if (rate > 1024)
					rtlglue_printf( "Bandwidth=%d.%dMbps ", rate/1024, (rate%1024) * 100 /1024);
				else
					rtlglue_printf( "Bandwidth=%dKbps ", rate);
                            #else
				if ( bandwidth*64000 > 1000000 )
					rtlglue_printf( "Bandwidth=%d.%dMbps ", bandwidth*64/1000, (bandwidth*64)%1000 );
				else
					rtlglue_printf( "Bandwidth=%dKbps ", bandwidth*64 );
                            #endif
				rtlglue_printf( "\n\n" );
			}
			return result;
		}
		else if (!strcmp(nextToken, "pegressbandwidth"))
		{
			enum PORTID port;
			uint32 bandwidth;
                     uint32 rate;
			cle_getNextCmdToken(&nextToken, &size, saved); 
			port = U32_value(nextToken);
		
			result = rtl8651_getAsicPortEgressBandwidth( port, &bandwidth );
			if ( result==SUCCESS )
			{
				rtlglue_printf( "\n[Port=%d] ", port );
                            rate = bandwidth*64000/1024;
				if (rate > 1024)
					rtlglue_printf( "Bandwidth=%d.%dMbps ", rate/1024, (rate%1024) *100 /1024);
				else
					rtlglue_printf( "Bandwidth=%dKbps ", rate );
				rtlglue_printf( "\n\n" );
			}
			return result;
		}

		else if (!strcmp(nextToken, "qweight"))
		{
			enum PORTID port;
			enum QUEUEID queueid;
			enum QUEUETYPE queueType;
			uint32 weight;
			
			cle_getNextCmdToken(&nextToken, &size, saved); 
			port = U32_value(nextToken);
			cle_getNextCmdToken(&nextToken, &size, saved); 
			queueid = U32_value(nextToken);
			cle_getNextCmdToken(&nextToken, &size, saved); 
		
			result = rtl8651_getAsicQueueWeight( port, queueid, &queueType, &weight );
			if ( result==SUCCESS )
			{
				rtlglue_printf( "\n[Port=%d, Queue=%d] ", port, queueid );
				if ( queueType==STR_PRIO )
					rtlglue_printf( "Strict Priority " );
				else
					rtlglue_printf( "Weight=%d ", weight+1 );
				rtlglue_printf( "\n\n" );
			}
			return result;
		}
		else if (!strcmp(nextToken, "qnum"))
		{
			enum PORTID port;
			enum QUEUENUM qnum;
			
			cle_getNextCmdToken(&nextToken, &size, saved); 
			port = U32_value(nextToken);
		
			result = rtl8651_getAsicOutputQueueNumber(port, &qnum);
			if ( result==SUCCESS )
			{
				rtlglue_printf( "\n[Port=%d] Queue Number=%d ", port, qnum );
				rtlglue_printf( "\n\n" );
			}
			return result;
		}
		else if (!strcmp(nextToken, "1prmEN"))
		{
			enum PORTID port;
			uint32 isEnable;
			
			cle_getNextCmdToken(&nextToken, &size, saved); 
			port = U32_value(nextToken);
		
			result = rtl8651_getAsicDot1pRemarkingAbility(port, &isEnable);
			if ( result==SUCCESS )
			{
				rtlglue_printf( "\n[Port=%d] ", port);
				if ( isEnable==TRUE )
					rtlglue_printf( "802.1p Remarking Enable " );
				else
					rtlglue_printf( "802.1p Remarking Disable " );
				rtlglue_printf( "\n\n" );
			}
			return result;
		}
		else if (!strcmp(nextToken, "1prm"))
		{
			enum PRIORITYVALUE priority, pNewpriority;
			
			cle_getNextCmdToken(&nextToken, &size, saved); 
			priority = U32_value(nextToken);
		
			result = rtl8651_getAsicDot1pRemarkingParameter(priority, &pNewpriority);
			if ( result==SUCCESS )
			{
				rtlglue_printf( "\n[priority=%d] new priority=%d ", priority, pNewpriority);
				rtlglue_printf( "\n\n" );
			}
			return result;
		}
		else if (!strcmp(nextToken, "dscprmEN"))
		{
			enum PORTID port;
			uint32 isEnable;
			
			cle_getNextCmdToken(&nextToken, &size, saved); 
			port = U32_value(nextToken);
		
			result = rtl8651_getAsicDscpRemarkingAbility( port, &isEnable);
			if ( result==SUCCESS )
			{
				rtlglue_printf( "\n[Port=%d] ", port);
				if ( isEnable==TRUE )
					rtlglue_printf( "DSCP Remarking Enable " );
				else
					rtlglue_printf( "DSCP Remarking Disable " );
				rtlglue_printf( "\n\n" );
			}
			return result;
		}
		else if (!strcmp(nextToken, "dscprm"))
		{
			enum PRIORITYVALUE priority;
			uint32 pNewdscp;
			
			cle_getNextCmdToken(&nextToken, &size, saved); 
			priority = U32_value(nextToken);
		
			result = rtl8651_getAsicDscpRemarkingParameter(priority, &pNewdscp);
			if ( result==SUCCESS )
			{
				rtlglue_printf( "\n[priority=%d] new DSCP=%d ", priority, pNewdscp);
				rtlglue_printf( "\n\n" );
			}
			return result;
		}
		else if (!strcmp(nextToken, "pridecision"))
		{
			uint32 pPortpri, pDot1qpri, pDscppri, pAclpri, pNatpri;
		
			result = rtl8651_getAsicPriorityDecision(&pPortpri, &pDot1qpri, &pDscppri, &pAclpri, &pNatpri);
			if ( result==SUCCESS )
			{
				rtlglue_printf( "\n[Port based]= %d %d %d %d", (pPortpri >> 3) & 1, (pPortpri >> 2) & 1, (pPortpri >> 1) & 1, pPortpri % 2);
				rtlglue_printf( "\n[  1Q based]= %d %d %d %d", (pDot1qpri >> 3) & 1, (pDot1qpri >> 2) & 1, (pDot1qpri >> 1) & 1, pDot1qpri % 2);
				rtlglue_printf( "\n[DSCP based]= %d %d %d %d", (pDscppri >> 3) & 1, (pDscppri >> 2) & 1, (pDscppri >> 1) & 1, pDscppri % 2);
				rtlglue_printf( "\n[ ACL based]= %d %d %d %d", (pAclpri >> 3) & 1, (pAclpri >> 2) & 1, (pAclpri >> 1) & 1, pAclpri % 2);
				rtlglue_printf( "\n[ NAT based]= %d %d %d %d", (pNatpri >> 3) & 1, (pNatpri >> 2) & 1, (pNatpri >> 1) & 1, pNatpri % 2);
				
				rtlglue_printf( "\n\n" );
			}
			return result;
		}
		else if (!strcmp(nextToken, "portpri"))
		{
			enum PORTID port;
			enum PRIORITYVALUE pPriority;		

			cle_getNextCmdToken(&nextToken, &size, saved); 
			port = U32_value(nextToken);

			result = rtl8651_getAsicPortPriority(port, &pPriority);
			if ( result==SUCCESS )
			{
				rtlglue_printf( "\n[Port=%d] priority=%d ", port, pPriority);
				rtlglue_printf( "\n\n" );
			}
			return result;
		}
		else if (!strcmp(nextToken, "1qpri"))
		{
			enum PRIORITYVALUE srcpriority, pPriority;

			cle_getNextCmdToken(&nextToken, &size, saved); 
			srcpriority = U32_value(nextToken);
			
			result = rtl8651_getAsicDot1qAbsolutelyPriority(srcpriority, &pPriority);
			if ( result==SUCCESS )
			{
				rtlglue_printf( "\n[priority=%d] absolutely priority=%d ", srcpriority, pPriority);
				rtlglue_printf( "\n\n" );
			}
			return result;
		}
		else if (!strcmp(nextToken, "dscppri"))
		{
			uint32 dscp;
			enum PRIORITYVALUE pPriority;

			cle_getNextCmdToken(&nextToken, &size, saved); 
			dscp = U32_value(nextToken);
			
			result = rtl8651_getAsicDscpPriority(dscp, &pPriority);
			if ( result==SUCCESS )
			{
				rtlglue_printf( "\n[DSCP=%d] priority=%d ", dscp, pPriority);
				rtlglue_printf( "\n\n" );
			}
			return result;
		}
		else if (!strcmp(nextToken, "pritoqid"))
		{
			enum QUEUENUM qnum;
			enum PRIORITYVALUE priority;
			enum QUEUEID pQid;			

			cle_getNextCmdToken(&nextToken, &size, saved); 
			qnum = U32_value(nextToken);
			cle_getNextCmdToken(&nextToken, &size, saved); 
			priority = U32_value(nextToken);
			
			result = rtl8651_getAsicPriorityToQIDMappingTable(qnum, priority, &pQid);
			if ( result==SUCCESS )
			{
				rtlglue_printf( "\n[Queue Number=%d  Priority=%d]  Queue ID=%d ", qnum, priority, pQid);
				rtlglue_printf( "\n\n" );
			}
			return result;
		}
		else
			return FAILED;	
	}
	else if (!strcmp(nextToken, "dump"))
	{
		return FAILED;
	}
	else if (!strcmp(nextToken, "chariot"))
	{
		int32 retval;
		rtl865x_tblAsicDrv_vlanParam_t vlan;
		rtl865x_tblAsicDrv_intfParam_t intf;

		/* SWCORE */
		WRITE_MEM32(SIRR, READ_MEM32(SIRR)|FULL_RST|SEMI_RST);
		rtl8651_clearAsicAllTable();
		rtl8651_clearRegister();
		WRITE_MEM32(TMCR,~ENHSBTESTMODE&READ_MEM32(TMCR));
		rtl8651_setAsicOperationLayer(2);
		
		/**********************************************************************
		 * set ASIC registers
		 **********************************************************************/
		WRITE_MEM32(MSCR, READ_MEM32(MSCR)& ~(EN_IN_ACL|EN_OUT_ACL));
		WRITE_MEM32(QNUMCR, P0QNum_1|P1QNum_1|P2QNum_1|P3QNum_1|P4QNum_1|P5QNum_1|P6QNum_1 );
		WRITE_MEM32(QRR,READ_MEM32(QRR)|QRST );
		WRITE_MEM32(SIRR, READ_MEM32(SIRR)|TRXRDY );
		WRITE_MEM32(CCR, READ_MEM32(CCR)|L2CRCErrAllow );
		
		/**********************************************************************
		 * Set Net Interface 0
		 *---------------------------------------------------------------------*/
		bzero((void*) &intf, sizeof(intf));
		strtomac(&intf.macAddr, "00-00-10-11-12-20" );
		intf.macAddrNumber = 1;
		intf.vid = 8;
		intf.mtu = 1500;
		intf.inAclStart = 0;
		intf.inAclEnd = 0;
		intf.outAclStart = 0;
		intf.outAclEnd = 0;
		intf.enableRoute = 0;
		intf.valid = 1;
		retval = rtl8651_setAsicNetInterface( RTL865XC_NETIFTBL_SIZE, &intf );
		if( retval != SUCCESS ) return retval;

		/**********************************************************************
		 * set VLAN 8 member port 0x3f		
		 **********************************************************************/
		memset( &vlan, 0x0, sizeof(vlan) );
		vlan.memberPortMask = 0x3;; /* Port 0&1 */
		vlan.untagPortMask = 0x3; /* Port 0&1 */
		retval = rtl8651_setAsicVlan(8,&vlan);
		if( retval != SUCCESS ) return retval;

		/**********************************************************************
		 * Port Based VLAN Control Register
		 **********************************************************************/
		WRITE_MEM32( PVCR0, (READ_MEM32(PVCR0)&~(0xffff<< 0))|(0/*priority*/<<12)|(0x008/*PVID*/<< 0) ); /* Port 0 */
		WRITE_MEM32( PVCR0, (READ_MEM32(PVCR0)&~(0xffff<<16))|(0/*priority*/<<28)|(0x008/*PVID*/<<16) ); /* Port 1 */
		WRITE_MEM32( PVCR1, (READ_MEM32(PVCR1)&~(0xffff<< 0))|(0/*priority*/<<12)|(0x008/*PVID*/<< 0) ); /* Port 2 */
		WRITE_MEM32( PVCR1, (READ_MEM32(PVCR1)&~(0xffff<<16))|(0/*priority*/<<28)|(0x008/*PVID*/<<16) ); /* Port 3 */
		WRITE_MEM32( PVCR2, (READ_MEM32(PVCR2)&~(0xffff<< 0))|(0/*priority*/<<12)|(0x008/*PVID*/<< 0) ); /* Port 4 */
		WRITE_MEM32( PVCR2, (READ_MEM32(PVCR2)&~(0xffff<<16))|(0/*priority*/<<28)|(0x008/*PVID*/<<16) ); /* Port 5 */
		WRITE_MEM32( PVCR3, (READ_MEM32(PVCR3)&~(0xffff<< 0))|(0/*priority*/<<12)|(0x008/*PVID*/<< 0) ); /* Port ext 1 */
		WRITE_MEM32( PVCR3, (READ_MEM32(PVCR3)&~(0xffff<<16))|(0/*priority*/<<28)|(0x008/*PVID*/<<16) ); /* Port ext 2 */
		WRITE_MEM32( PVCR4, (READ_MEM32(PVCR4)&~(0xffff<< 0))|(0/*priority*/<<12)|(0x008/*PVID*/<< 0) ); /* Port ext 3 */
		
		/**********************************************************************
		 * Port Conf Registers 
		 **********************************************************************/
		WRITE_MEM32( PCRP0, (0x08<<ExtPHYID_OFFSET)|EnForceMode|ForceLink|ForceSpeed10M|ForceDuplex|EnablePHYIf|STP_PortST_FORWARDING );
		WRITE_MEM32( PCRP1, (0x09<<ExtPHYID_OFFSET)|EnForceMode|ForceLink|ForceSpeed10M|ForceDuplex|EnablePHYIf|STP_PortST_FORWARDING );
		WRITE_MEM32( PCRP2, (0x0A<<ExtPHYID_OFFSET)|EnForceMode|ForceLink|ForceSpeed10M|ForceDuplex|EnablePHYIf|STP_PortST_FORWARDING );
		WRITE_MEM32( PCRP3, (0x0B<<ExtPHYID_OFFSET)|EnForceMode|ForceLink|ForceSpeed10M|ForceDuplex|EnablePHYIf|STP_PortST_FORWARDING );
		WRITE_MEM32( PCRP4, (0x0C<<ExtPHYID_OFFSET)|EnForceMode|ForceLink|ForceSpeed10M|ForceDuplex|EnablePHYIf|STP_PortST_FORWARDING );

		/**********************************************************************
		 * Port Based Priority
		 **********************************************************************/
		
		/**********************************************************************
		 * MDC/MDIO Control Register
		 **********************************************************************/
		WRITE_MEM32(MDCIOCR, COMMAND_WRITE|(0x01<<PHYADD_OFFSET)|(4<<REGADD_OFFSET)|(0x05E1<<WRDATA_OFFSET) );
		WRITE_MEM32(MDCIOCR, COMMAND_WRITE|(0x02<<PHYADD_OFFSET)|(4<<REGADD_OFFSET)|(0x05E1<<WRDATA_OFFSET) );
		WRITE_MEM32(MDCIOCR, COMMAND_WRITE|(0x04<<PHYADD_OFFSET)|(4<<REGADD_OFFSET)|(0x05E1<<WRDATA_OFFSET) );
		WRITE_MEM32(MDCIOCR, COMMAND_WRITE|(0x08<<PHYADD_OFFSET)|(4<<REGADD_OFFSET)|(0x05E1<<WRDATA_OFFSET) );
		WRITE_MEM32(MDCIOCR, COMMAND_WRITE|(0x10<<PHYADD_OFFSET)|(4<<REGADD_OFFSET)|(0x05E1<<WRDATA_OFFSET) );

		WRITE_MEM32(MDCIOCR, COMMAND_WRITE|(0x01<<PHYADD_OFFSET)|(0<<REGADD_OFFSET)|(0x1200<<WRDATA_OFFSET) );
		WRITE_MEM32(MDCIOCR, COMMAND_WRITE|(0x02<<PHYADD_OFFSET)|(0<<REGADD_OFFSET)|(0x1200<<WRDATA_OFFSET) );
		WRITE_MEM32(MDCIOCR, COMMAND_WRITE|(0x04<<PHYADD_OFFSET)|(0<<REGADD_OFFSET)|(0x1200<<WRDATA_OFFSET) );
		WRITE_MEM32(MDCIOCR, COMMAND_WRITE|(0x08<<PHYADD_OFFSET)|(0<<REGADD_OFFSET)|(0x1200<<WRDATA_OFFSET) );
		WRITE_MEM32(MDCIOCR, COMMAND_WRITE|(0x10<<PHYADD_OFFSET)|(0<<REGADD_OFFSET)|(0x1200<<WRDATA_OFFSET) );
	
		return SUCCESS;
	}
	else
		return FAILED;	
}
#endif

static int32 _rtl8651_flowControlCmd(uint32 userId,  int32 argc,int8 **saved)
{
	int32 size, retval=FAILED;
	uint32 port;
	int8 *nextToken;

	cle_getNextCmdToken(&nextToken,&size,saved);
	if (strcmp(nextToken, "threshold") == 0) {
		uint32 threshold;
		cle_getNextCmdToken(&nextToken,&size,saved);
		if (strcmp(nextToken, "inq-high") == 0) {
			cle_getNextCmdToken(&nextToken,&size,saved);
			threshold = U32_value(nextToken);
			return rtl8651_flowContrlThreshold(3, threshold);
		}
		else if (strcmp(nextToken, "outq-high") == 0) {
			cle_getNextCmdToken(&nextToken,&size,saved);
			threshold = U32_value(nextToken);
			return rtl8651_flowContrlThreshold(1, threshold);
		}
		else if (strcmp(nextToken, "inq-low") == 0) {
			cle_getNextCmdToken(&nextToken,&size,saved);
			threshold = U32_value(nextToken);
			return rtl8651_flowContrlThreshold(2, threshold);
		}
		else if (strcmp(nextToken, "outq-low") == 0) {
			cle_getNextCmdToken(&nextToken,&size,saved);
			threshold = U32_value(nextToken);
			return rtl8651_flowContrlThreshold(0, threshold);
		}
		else if (strcmp(nextToken, "inopt") == 0) {
			cle_getNextCmdToken(&nextToken,&size,saved);
			threshold = U32_value(nextToken);
			return rtl8651_flowContrlPrimeThreshold(1, threshold);
		}
		else if (strcmp(nextToken, "outopt") == 0) {
			cle_getNextCmdToken(&nextToken,&size,saved);
			threshold = U32_value(nextToken);
			return rtl8651_flowContrlPrimeThreshold(0, threshold);
		}
		return FAILED;
	}
	else {
		port = U32_value(nextToken);
		
		cle_getNextCmdToken(&nextToken,&size,saved);
		if (strcmp(nextToken, "enable") == 0) 
			retval = rtl8651_setFlowControl(port, TRUE);
		else if (strcmp(nextToken, "disable") == 0)
			retval = rtl8651_setFlowControl(port, FALSE);
		else if (strcmp(nextToken, "auto") == 0)
			retval = rtl8651_setFlowControl(port, 2);
		return retval;
	}
}

static  int32 _checkPhySpeedDuplexState(void **token, int8 *input, int32 size, cle_exec_t **nextCmd){
	int32 i,numsize =cle_checkDynCmdParam_String(token, input, size, nextCmd);
	if(numsize)
		return FAILED;
	for(i=0;i < PHY_SPEED_DUPLEX;i++)
		if(strcmp(input, _phySpeedDuplex[i])==0)
			break;
	if(i==PHY_SPEED_DUPLEX)
		return FAILED;
	 *((int32 *) *((memaddr *)token))=i;
	return 4;

}

static int32 _enumPhySpeedDuplexState(int32 entries, struct enumList *list){
	int32 i;

	if (list && entries < PHY_SPEED_DUPLEX)
		return FAILED; //No enough buffer space
	else if(list){
		for(i=0; i < PHY_SPEED_DUPLEX; i++){
			list[i].tokenName =(int8 *)_phySpeedDuplex[i];
			list[i].helpString= "";
		}
	}
	return PHY_SPEED_DUPLEX;
}

static cle_paramType_t _tblAsicDrvPhySpeedDuplexCheckFunc[] ={
	{"%phySpeedDuplexState", 4, _checkPhySpeedDuplexState, 	_enumPhySpeedDuplexState,	"Physical layer speed and duplex state"},
};


cle_exec_t rtl8651_tblAsicDrv_cmds[] = {

	{	"phy",
		"Configure physical ports",
		"{ %d'port number' | all'All ports' } { 100f | 100h | 10f | 10h | an }",
		_rtl8651_phyCmd,
		CLE_USECISCOCMDPARSER,	
		sizeof(_tblAsicDrvPhySpeedDuplexCheckFunc)/sizeof(cle_paramType_t),
		(cle_paramType_t *)&_tblAsicDrvPhySpeedDuplexCheckFunc
	},
	{
		"register",
		"Configure ASIC register parameters",
		"{ set'Set register' { "
		"{ mmtu'Multicast MTU' %d'(bytes)' } | "
		"{ scr'CPU Clock Control' %d'(0:225MHz, 1:250MHz, 2:275MHz, 3:300MHz... 7:400MHz)' } | "			
		"{ mcr'Memory Clock Control' %d'(0:137MHz, 1:150MHz, 2:162MHz, 3:175MHz, 4:187MHz)' } "
		"} | "
		"get'Get register' { "
		"{ mmtu'Multicst MTU' } | "
		"{ scr'CPU Clock Control' } | "
		"{ mcr'Memory Clock Control' } "
		"} "		
		"}",
		_rtl8651_registerCmd,
		CLE_USECISCOCMDPARSER,	
		0,
		NULL
	},
#ifdef CONFIG_RTL865XC
	{	"qos",
		"Configure QoS - Packet Scheduling",
		"{ set'Set API' { { init' Init Qos Parameters' } | "
		"                 { lb'Leaky bucket parameters ' %d'Token' %d'Tick' %d'High threshold' } | "
		"                 { pingressbandwidth' Port ingress bandwidth ' %d'Port number' %d'Bandwidth (0x3FFF: disable)' } | "
		"                 { pegressbandwidth' Port egress bandwidth ' %d'Port number' %d'Bandwidth (0x3FFF: disable)' } | "
		"                 { qrate'Queue rate ' %d'Port number' %d'Queue ID' %d'Peak rate(0x7: disable)' %d'Burst size of average rate(0xFF: disable)' %d'Average rate(0x3FFF: unlimited rate)' } | "
		"                 { qweight'Queue weight ' %d'Port number' %d'Queue ID' %d'Queue type(0:Strict priority   1:WFQ)' %d'Weight(valid:0~127)' } | "
		"                 { qnum'Queue number ' %d'Port number' %d'Queue number' } | "
		"                 { 1prmEN'802.1p Remarking ability ' %d'Port number' %d'Enable(0:Disable   1:Enable)' } | "
		"                 { 1prm'802.1p Remarking Parameters ' %d'Priority(valid:0~7)' %d'New Priority(valid:0~7)' } | "
		"                 { dscprmEN'DSCP Reamrking ability ' %d'Port number' %d'Enable(0:Disable   1:Enable)' } | "
		"                 { dscprm'DSCP Reamrking Parameters ' %d'Priority(valid:0~7)' %d'New DSCP value(valid:0~63)' } | "
		"                 { pridecision'Priority Descision table ' %d'Port Priority(valid:0~15)' %d'802.1Q Priority(valid:0~15)' %d'DSCP Priority(valid:0~15)' %d'ACL Priority(valid:0~15)' %d'NAT Priority(valid:0~15)' } | "
		"                 { portpri'Port Priority ' %d'Port number' %d'Priority(valid:0~7)' } | "
		"                 { 1qpri'802.1Q Absolutely Priority ' %d'Priority(valid:0~7)' %d'New Priority(valid:0~7)' } | "
		"                 { dscppri'DSCP Priority ' %d'DSCP(valid:0~63)' %d'Priority(valid:0~7)' } | "
		"                 { pritoqid'Priority to QID Mapping table ' %d'Queue Number(valid:1~6)' %d'Priority(valid:0~7)' %d'Queue ID(valid:0~5)' } } | "
		"  get'Get API' { { lb'Leaky bucket parameters' } | "
		"                 { pingressbandwidth'Port ingress bandwidth' %d'Port number' } | "
		"                 { pegressbandwidth'Port egress bandwidth' %d'Port number' } | "
		"                 { qrate'Queue rate' %d'Port number' %d'Queue ID' } | "
		"                 { qweight'Queue weight' %d'Port number' %d'Queue ID' } | "
		"                 { qnum'Queue number' %d'Port number' } | "
		"                 { 1prmEN'802.1p Remarking ability ' %d'Port number' } | "
		"                 { 1prm'802.1p Remarking Parameters ' %d'Priority(valid:0~7)' } | "
		"                 { dscprmEN'DSCP Reamrking ability ' %d'Port number' } | "
		"                 { dscprm'DSCP Reamrking Parameters ' %d'Priority(valid:0~7)' } | "
		"                 { pridecision'Priority Descision table ' } | "
		"                 { portpri'Port Priority ' %d'Port number' } | "
		"                 { 1qpri'802.1Q Absolutely Priority ' %d'Priority(valid:0~7)' } | "
		"                 { dscppri'DSCP Priority ' %d'DSCP(valid:0~63)' } | "
		"                 { pritoqid'Priority to QID Mapping table ' %d'Queue Number(valid:1~6)' %d'Priority(valid:0~7)' } } | "
		"  dump'Dump all setting' | "
		"  chariot'Setting parameters for Chariot test' } ",
		_rtl8651_qosCmd,
		CLE_USECISCOCMDPARSER,	
		0,
		NULL
	},
#endif
	{
		"flow-control",
		"Configure Flow Control",
		"{ %d'Port number' { enable'Enable the specified port Flow Control' | disable'Disable the specified port Flow Control' | auto'Auto' } | threshold { [ inq-high | inq-low | outq-high | outq-low | inopt | outopt ]  } %d'threshold value' } ",
		_rtl8651_flowControlCmd,
		CLE_USECISCOCMDPARSER,	
		0,
		NULL
	},

};

cle_grp_t rtl8651_tblAsicDrv_grps[] =
{
#ifndef CONFIG_RTL865X_LIGHT_ROMEDRV
#ifdef CONFIG_RTL865XC
    {
        "gpio",                             //cmdStr
        "GPIO commands",           //cmdDesc
        NULL,                           //cmdPrompt
        rtl865xc_gpio_cmds,                 //exec array
        NULL,                               //group array
        CMD_RTL865XC_GPIO_CMD_NUM,     //exec number
        0,                                      //group number
        0,                                      //access level
    },
#endif
#endif
};
