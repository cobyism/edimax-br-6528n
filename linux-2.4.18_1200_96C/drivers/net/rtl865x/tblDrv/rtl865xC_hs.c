#include "types.h"
#include "assert.h"
#include "asicregs.h"
#include "rtl_utils.h"
#include "rtl8651_debug.h"
#include "rtl865xC_hs.h"

/* dump all fields to one string */

void hs_displayHsb(hsb_param_t * hsbWatch)
{
	int32	i=0;
	
	rtlglue_printf("---------------------------- BEGIN HSB-----------------------------\n");
	switch(hsbWatch->type) {
		case 0:	rtlglue_printf("Packet Type Ethernet, "); break;
		case 1:	rtlglue_printf("Packet Type PPTP, "); break;
		case 2:	rtlglue_printf("Packet Type IP, "); break;
		case 3:	rtlglue_printf("Packet Type ICMP, "); break;
		case 4:	rtlglue_printf("Packet Type IGMP, "); break;
		case 5:	rtlglue_printf("Packet Type TCP, "); break;
		case 6:	rtlglue_printf("Packet Type UDP, "); break;
		default:	rtlglue_printf("Packet Unknown Type: %d",hsbWatch->type); break;
	}
	if(hsbWatch->spa<7)
		rtlglue_printf("source port: Physical port %d, ",hsbWatch->spa);
	else if(hsbWatch->extspa<3)
		rtlglue_printf("source port: Extension port %d, ",hsbWatch->extspa);
	else
		rtlglue_printf("source port: CPU, ");
	rtlglue_printf("bytecount: %d (0x%x)\n",hsbWatch->len,hsbWatch->len);
	rtlglue_printf("Layer2 Format %s",hsbWatch->vid ?"VLAN":"NoVLAN");
	if(hsbWatch->vid)
		rtlglue_printf(" ID: %d (0x%x)",hsbWatch->vid,hsbWatch->vid);
	rtlglue_printf(", %s, %s", hsbWatch->llcothr?"LLC Other":"NoLLC", hsbWatch->pppoeif?"PPPoE":"NoPPPoE");
//      if(hsbWatch->pppoesid)
//		rtlglue_printf("ID: %d (0x%x), ", hsbWatch->pppoesid, hsbWatch->pppoesid));
	rtlglue_printf("\n");
	
	// Protocol contents
	rtlglue_printf("DMAC: ");
	for (i=0; i<6; i++)
		rtlglue_printf("%02x%s", hsbWatch->da[i], i<5?"-":"");
	rtlglue_printf(", ");
	
	rtlglue_printf("SMAC: ");
	for (i=0; i<6; i++)
		rtlglue_printf("%02x%s", hsbWatch->sa[i], i<5?"-":"");
	rtlglue_printf("\n");
	rtlglue_printf("%s, ",hsbWatch->dirtx?"DirectTX":"FromPHY");
	if(hsbWatch->dirtx) {
		rtlglue_printf("L3 Checksum 0x%x ", hsbWatch->sprt);
		rtlglue_printf("L4 Checksum 0x%x\n", hsbWatch->dprt);
		rtlglue_printf("Reason %x ", hsbWatch->iptos);
		if((hsbWatch->ethtype>>3)&0x1) 
			rtlglue_printf("Add PPPoE Header ID: %d\n", hsbWatch->ethtype&0x7);
#if 0        
		if((hsbWatch->pppoesid>>3)&0x1)
			rtlglue_printf("Add VLAN Header ID: %d\n", hsbWatch->pppoesid&0x7);
		else
			rtlglue_printf("\n");
#endif
		rtlglue_printf("Specify output port: ");
#if 0
		for(i=0;i<6;i++)
			if((hsbWatch->pppoesid>>5)&(1<<i))
				rtlglue_printf("%d ",i);
		if(hsbWatch->pppoesid>>11)	
			rtlglue_printf("CPU\n");
		else
			rtlglue_printf("\n");
			

		if(hsbWatch->spa== 7 && hsbWatch->extspa) {//Only from extension port, the cpu_l2 has meaning
			if(hsbWatch->c)
				rtlglue_printf("CPU Send with Layer2 action");
			else
				rtlglue_printf("CPU Send with Multi-layer action");
		}
#endif
	}
	else {
#if 0		
		rtlglue_printf("ethertype: %d (0x%x)",hsbWatch->ethtype,hsbWatch->ethtype);		
		rtlglue_printf("sip: ");
		for (i=0; i<4; i++)
			rtlglue_printf("%d%s", hsbWatch->sip[i], i<3?".":"");
		rtlglue_printf(", ");
		
		rtlglue_printf("sprt (ICMP ID, L3 Checksum): %d 0x%x\n",(int)hsbWatch->sprt,(int)hsbWatch->sprt);
		
		rtlglue_printf("dip: ");
		for (i=0; i<4; i++)
			rtlglue_printf("%d%s", hsbWatch->dip[i], i<3?".":"");
		rtlglue_printf(", ");
			
		rtlglue_printf("dprt: %d 0x%x \n",hsbWatch->dprt,hsbWatch->dprt);
		
		rtlglue_printf("ip protocol (ICMP, IGMP Type): %d 0x%x, ",(int)hsbWatch->ipptl,(int)hsbWatch->ipptl);
		rtlglue_printf("ip flag: %d 0x%x, ",hsbWatch->ipflg,hsbWatch->ipflg);
		rtlglue_printf("ip tos: %d 0x%x\n",hsbWatch->iptos,hsbWatch->iptos);
		rtlglue_printf("tcp flag: %d 0x%x, ",hsbWatch->tcpflg,hsbWatch->tcpflg);
		
		rtlglue_printf("PatternMatch: %d, ",hsbWatch->prtnmat);
	       
		rtlglue_printf("udp_nocs: %d\n",hsbWatch->udp_nocs);
		if(hsbWatch->ttlst == 2)
			rtlglue_printf("TTL > 1,"));
		else
			rtlglue_printf("TTL %x, ",hsbWatch->ttlst);
		rtlglue_printf("pktend: %d, ",hsbWatch->pktend);

		rtlglue_printf("Packet priority is  %d ",hsbWatch->hiprior);
		rtlglue_printf(" L3CSOK:%d, L4CSOK:%d, IPFRAGIF:%d",hsbWatch->l3crcok,hsbWatch->l4crcok,hsbWatch->ipfragif);
		rtlglue_printf("\n"));

		rtlglue_printf("LLC_Other_IF: %d, ",hsbWatch->llc_other);
		rtlglue_printf("URL_Trap: %d\n",hsbWatch->urlmch);
#endif			
	}

		
}


void hs_displayHsa_S(hsa_param_t *hsaWatch)
{
	ipaddr_t addr;
	char addr_s[100];

	rtlglue_printf(("HSA("));
//	rtlglue_printf("\tmac:%02x-%02x-%02x-%02x-%02x-%02x\n",hsaWatch->nhmac[5],hsaWatch->nhmac[4],hsaWatch->nhmac[3],hsaWatch->nhmac[2],hsaWatch->nhmac[1],hsaWatch->nhmac[0]);
	rtlglue_printf("\tmac:%02x-%02x-%02x-%02x-%02x-%02x\n",hsaWatch->nhmac[0],hsaWatch->nhmac[1],hsaWatch->nhmac[2],hsaWatch->nhmac[3],hsaWatch->nhmac[4],hsaWatch->nhmac[5]);

	addr =ntohl( hsaWatch->trip);
	inet_ntoa_r(addr, addr_s);
	rtlglue_printf("\ttrip:%s(hex:%08x)",addr_s,hsaWatch->trip);	
	rtlglue_printf("\tprt:%d\tipmcast:%d\n",hsaWatch->port,hsaWatch->ipmcastr);
	rtlglue_printf("\tl3cs:%d",hsaWatch->l3csdt);
	rtlglue_printf("\tl4cs:%d",hsaWatch->l4csdt);
	rtlglue_printf("\tInternal NETIF:%d",hsaWatch->egif);
	rtlglue_printf("\tl2tr:%d,\n ",hsaWatch->l2tr);
	rtlglue_printf("\tl34tr:%d",hsaWatch->l34tr);
	rtlglue_printf("\tdirtx:%d",hsaWatch->dirtxo);
	rtlglue_printf("\ttype:%d",hsaWatch->typeo);
	rtlglue_printf("\tsnapo:%d",hsaWatch->snapo);
	rtlglue_printf("\twhy2cpu 0x%x (%d)\n",hsaWatch->why2cpu,hsaWatch->why2cpu);
	rtlglue_printf("\tpppif:%d",hsaWatch->pppoeifo);
	rtlglue_printf("\tpppid:%d",hsaWatch->pppidx);
	rtlglue_printf("\tttl_1:0x%x",hsaWatch->ttl_1if);
	rtlglue_printf("\tdpc:%d,",hsaWatch->dpc);

	rtlglue_printf("\tleno:%d(0x%x)\n",hsaWatch->leno,hsaWatch->leno);

	rtlglue_printf("\tl3CrcOk:%d",hsaWatch->l3csoko);
	rtlglue_printf("\tl4CrcOk:%d",hsaWatch->l4csoko);
	rtlglue_printf("\tfrag:%d",hsaWatch->frag);
	rtlglue_printf("\tlastFrag:%d\n",hsaWatch->lastfrag);



	rtlglue_printf("\tsvid:0x%x",hsaWatch->svid);
	rtlglue_printf("\tdvid:%d(0x%x)",hsaWatch->dvid,hsaWatch->dvid);
	rtlglue_printf("\tdestination interface :%d\n",hsaWatch->difid);
	rtlglue_printf("\trxtag:%d",hsaWatch->rxtag);
	rtlglue_printf("\tdvtag:0x%x",hsaWatch->dvtag);
	rtlglue_printf("\tspa:%d",hsaWatch->spao);
	rtlglue_printf("\tdpext:0x%x\thwfwrd:%d\n",hsaWatch->dpext,hsaWatch->hwfwrd);
	rtlglue_printf("\tspcp:%d",hsaWatch->spcp);
	rtlglue_printf("\tpriority:%d",hsaWatch->priority);
	
	rtlglue_printf("\tdp:0x%x\n",hsaWatch->dp);
	rtlglue_printf(")\n");	
}


void hs_displayHsb_S(hsb_param_t * hsbWatch)
{
	ipaddr_t addr;
	char addr_s[100];

	rtlglue_printf("HSB(");
	rtlglue_printf("\ttype:%d",hsbWatch->type);
	
	rtlglue_printf("\tspa:%d",hsbWatch->spa);
	rtlglue_printf("\tlen:%d",hsbWatch->len);
	rtlglue_printf("\tvid :%d\n",hsbWatch->vid);
	rtlglue_printf("\tpppoe:%d",hsbWatch->pppoeif);
	
	// Protocol contents
	rtlglue_printf("\ttagif:%d\tpppoeId:%d",hsbWatch->tagif,hsbWatch->pppoeid);
	rtlglue_printf("\tethrtype:0x%04x\n",hsbWatch->ethtype);
	rtlglue_printf("\tllc_other:%d\tsnap:%d\n",hsbWatch->llcothr,hsbWatch->snap);
	rtlglue_printf("\tda:%02x-%02x-%02x-%02x-%02x-%02x",hsbWatch->da[0],hsbWatch->da[1],hsbWatch->da[2],hsbWatch->da[3],hsbWatch->da[4],hsbWatch->da[5]);
	rtlglue_printf("\tsa:%02x-%02x-%02x-%02x-%02x-%02x\n",hsbWatch->sa[0],hsbWatch->sa[1],hsbWatch->sa[2],hsbWatch->sa[3],hsbWatch->sa[4],hsbWatch->sa[5]);
	
	//v32 = (hsbWatch->ethrtype15_12 << 12) | hsbWatch->ethrtype11_0;

	addr = ntohl( hsbWatch->sip);
	inet_ntoa_r(addr, addr_s);
	rtlglue_printf("\tsip:%s(hex:%08x)   ",addr_s,hsbWatch->sip);
	rtlglue_printf("\tsprt:%d (hex:%x)\n ",(int)hsbWatch->sprt,hsbWatch->sprt);
	addr  = ntohl(hsbWatch->dip);
	inet_ntoa_r(addr, addr_s);
	rtlglue_printf("\tdip:%s(hex:%08x) ",addr_s,hsbWatch->dip);;		
	rtlglue_printf("\tdprt:%d(hex:%08x)\n",hsbWatch->dprt,hsbWatch->dprt);
	
	rtlglue_printf("\tipptl:%d,",(int)hsbWatch->ipptl);
	rtlglue_printf("\tipflg:%d,",hsbWatch->ipfg);
	rtlglue_printf("\tiptos:%d,",hsbWatch->iptos);
	rtlglue_printf("\ttcpflg:%d\n",hsbWatch->tcpfg);
	
	rtlglue_printf("\tdirtx:%d,",hsbWatch->dirtx);
	rtlglue_printf("\tprtnmat:%d",hsbWatch->patmatch);
       
	rtlglue_printf("\tudp_nocs:%d",hsbWatch->udpnocs);
	rtlglue_printf("\tttlst:0x%x\n",hsbWatch->ttlst);

	
	rtlglue_printf("\thp:%d",hsbWatch->hiprior);
	rtlglue_printf("\tl3csok:%d\tl4csok:%d\tipfragif:%d\n",hsbWatch->l3csok,hsbWatch->l4csok,hsbWatch->ipfo0_n);
	
 	rtlglue_printf("\textspa:%d",hsbWatch->extspa);
	rtlglue_printf("\turlmch:%d\n)\n",hsbWatch->urlmch);
 
}

static int32 convertHsbToSoftware( hsb_t* rawHsb, hsb_param_t* hsb )
{
	/* bit-to-bit mapping */
	hsb->spa = rawHsb->spa;
	hsb->trigpkt = rawHsb->trigpkt;
 	hsb->len = rawHsb->len;
	hsb->vid = rawHsb->vid;
	hsb->tagif = rawHsb->tagif;
	hsb->pppoeif = rawHsb->pppoeif;
	hsb->sip = rawHsb->sip29_0 | (rawHsb->sip31_30<<30);
	hsb->sprt = rawHsb->sprt;
	hsb->dip = rawHsb->dip13_0 | (rawHsb->dip31_14<<14);
	hsb->dprt = rawHsb->dprt13_0 | (rawHsb->dprt15_14<<14);
	hsb->ipptl = rawHsb->ipptl;
	hsb->ipfg = rawHsb->ipfg;
	hsb->iptos = rawHsb->iptos;
	hsb->tcpfg = rawHsb->tcpfg;
	hsb->type = rawHsb->type;
	hsb->patmatch = rawHsb->patmatch;
	hsb->ethtype = rawHsb->ethtype;
#if 1 /* Since the endian is reversed, we must translate it. */
	hsb->da[5] = rawHsb->da14_0;
	hsb->da[4] = (rawHsb->da14_0>>8) | (rawHsb->da46_15<<7);
	hsb->da[3] = rawHsb->da46_15>>1;
	hsb->da[2] = rawHsb->da46_15>>9;
	hsb->da[1] = rawHsb->da46_15>>17;
	hsb->da[0] = (rawHsb->da46_15>>25) | (rawHsb->da47_47<<7);
	hsb->sa[5] = rawHsb->sa30_0;
	hsb->sa[4] = rawHsb->sa30_0>>8;
	hsb->sa[3] = rawHsb->sa30_0>>16;
	hsb->sa[2] = (rawHsb->sa30_0>>24) | (rawHsb->sa47_31<<7);
	hsb->sa[1] = rawHsb->sa47_31>>1;
	hsb->sa[0] = rawHsb->sa47_31>>9;
#else
	hsb->da[0] = rawHsb->da14_0;
	hsb->da[1] = (rawHsb->da14_0>>8) | (rawHsb->da46_15<<7);
	hsb->da[2] = rawHsb->da46_15>>1;
	hsb->da[3] = rawHsb->da46_15>>9;
	hsb->da[4] = rawHsb->da46_15>>17;
	hsb->da[5] = (rawHsb->da46_15>>25) | (rawHsb->da47_47<<7);
	hsb->sa[0] = rawHsb->sa30_0;
	hsb->sa[1] = rawHsb->sa30_0>>8;
	hsb->sa[2] = rawHsb->sa30_0>>16;
	hsb->sa[3] = (rawHsb->sa30_0>>24) | (rawHsb->sa47_31<<7);
	hsb->sa[4] = rawHsb->sa47_31>>1;
	hsb->sa[5] = rawHsb->sa47_31>>9;
#endif
	hsb->hiprior = rawHsb->hiprior;
	hsb->snap = rawHsb->snap;
	hsb->udpnocs = rawHsb->udpnocs;
	hsb->ttlst = rawHsb->ttlst;
	hsb->dirtx = rawHsb->dirtx;
	hsb->l3csok = rawHsb->l3csok;
	hsb->l4csok = rawHsb->l4csok;
	hsb->ipfo0_n = rawHsb->ipfo0_n;
	hsb->llcothr = rawHsb->llcothr;
	hsb->urlmch = rawHsb->urlmch;
	hsb->extspa = rawHsb->extspa;
	hsb->extl2 = rawHsb->extl2;
	hsb->linkid = rawHsb->linkid;
	hsb->pppoeid = rawHsb->pppoeid;
	return SUCCESS;
}


static int32 virtualMacGetHsb( hsb_param_t* hsb )
{
	hsb_t rawHsb;
	int32 ret = SUCCESS;

	{ /* Word-Access */
		uint32 *pSrc, *pDst;
		uint32 i;

		/* We must assert structure size is the times of 4-bytes. */
		if ( (sizeof(rawHsb)%4) != 0 ) RTL_BUG( "sizeof(rawHsb) is not the times of 4-bytes." );

		pSrc = (uint32*)HSB_BASE;
		pDst = (uint32*)&rawHsb;
		for( i = 0; i < sizeof(rawHsb); i+=4 )
		{
			*pDst = READ_MEM32((uint32)pSrc);
			pSrc++;
			pDst++;
		}
	}

	convertHsbToSoftware( &rawHsb, hsb );
	return ret;
}

int32 convertHsaToSoftware( hsa_t* rawHsa, hsa_param_t* hsa )
{
	/* bit-to-bit mapping */
#if 1 /* Since the endian is reversed, we must translate it. */
	hsa->nhmac[5] = rawHsa->nhmac0;
	hsa->nhmac[4] = rawHsa->nhmac1;
	hsa->nhmac[3] = rawHsa->nhmac2;
	hsa->nhmac[2] = rawHsa->nhmac3;
	hsa->nhmac[1] = rawHsa->nhmac4;
	hsa->nhmac[0] = rawHsa->nhmac5;
#else
	hsa->nhmac[0] = rawHsa->nhmac0;
	hsa->nhmac[1] = rawHsa->nhmac1;
	hsa->nhmac[2] = rawHsa->nhmac2;
	hsa->nhmac[3] = rawHsa->nhmac3;
	hsa->nhmac[4] = rawHsa->nhmac4;
	hsa->nhmac[5] = rawHsa->nhmac5;
#endif
	hsa->trip = rawHsa->trip15_0 | (rawHsa->trip31_16<<16);
	hsa->port = rawHsa->port;
	hsa->l3csdt = rawHsa->l3csdt;
	hsa->l4csdt = rawHsa->l4csdt;
	hsa->egif = rawHsa->egif;
	hsa->l2tr = rawHsa->l2tr;
	hsa->l34tr = rawHsa->l34tr;
	hsa->dirtxo = rawHsa->dirtxo;
	hsa->typeo = rawHsa->typeo;
	hsa->snapo = rawHsa->snapo;
	hsa->rxtag = rawHsa->rxtag;
	hsa->dvid = rawHsa->dvid;
	hsa->pppoeifo = rawHsa->pppoeifo;
	hsa->pppidx = rawHsa->pppidx;
	hsa->leno = rawHsa->leno5_0|(rawHsa->leno14_6<<6);
	hsa->l3csoko = rawHsa->l3csoko;
	hsa->l4csoko = rawHsa->l4csoko;
	hsa->frag = rawHsa->frag;
	hsa->lastfrag = rawHsa->lastfrag;
	hsa->ipmcastr = rawHsa->ipmcastr;
	hsa->svid = rawHsa->svid;
	hsa->fragpkt = rawHsa->fragpkt;
	hsa->ttl_1if = rawHsa->ttl_1if4_0|(rawHsa->ttl_1if5_5<<5)|(rawHsa->ttl_1if8_6<<6);
	hsa->dpc = rawHsa->dpc;
	hsa->spao = rawHsa->spao;
	hsa->hwfwrd = rawHsa->hwfwrd;
	hsa->dpext = rawHsa->dpext;
	hsa->spaext = rawHsa->spaext;
	hsa->why2cpu = rawHsa->why2cpu13_0|(rawHsa->why2cpu15_14<<14);
	hsa->spcp = rawHsa->spcp;
	hsa->dvtag = rawHsa->dvtag;
	hsa->difid = rawHsa->difid;
	hsa->linkid = rawHsa->linkid;
	hsa->siptos = rawHsa->siptos;
	hsa->dp = rawHsa->dp6_0;
	hsa->priority = rawHsa->priority;
	return SUCCESS;
}

static int32 virtualMacGetHsa( hsa_param_t* hsa )
{
	hsa_t rawHsa;
	int32 ret = SUCCESS;

	{ /* Word-Access */
		uint32 *pSrc, *pDst;
		uint32 i;

		/* We must assert structure size is the times of 4-bytes. */
		if ( (sizeof(rawHsa)%4) != 0 ) RTL_BUG( "sizeof(rawHsa) is not the times of 4-bytes." );

		pSrc = (uint32*)HSA_BASE;
		pDst = (uint32*)&rawHsa;
		for( i = 0; i < sizeof(rawHsa); i+=4 )
		{
			*pDst = READ_MEM32((uint32)pSrc);
			pSrc++;
			pDst++;
		}
	}

	convertHsaToSoftware( &rawHsa, hsa );
	return ret;
}


void dump_hs(void)
{
	hsb_param_t hsb_r;
	hsa_param_t hsa_r;
	memset((void*)&hsb_r,0,sizeof(hsb_r));
	memset((void*)&hsa_r,0,sizeof(hsa_r));
	virtualMacGetHsb( &hsb_r );	
	hs_displayHsb_S(&hsb_r);
	virtualMacGetHsa( &hsa_r );	
	hs_displayHsa_S(&hsa_r);

}
