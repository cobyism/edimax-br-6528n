
/*
* Copyright c                  Realtek Semiconductor Corporation, 2002
* All rights reserved.
*
* Program : The mbuf module implementation
* Abstract :
* Author : David Chun-Feng Liu (cfliu@realtek.com.tw)
*
* -------------------------------------------------------
*/
#include "rtl_types.h"
#include "assert.h"
#include "types.h"
#include "mbuf.h"
#include "rtl_utils.h"
#include "rtl_glue.h"
#if defined(RTL865X_TEST)||defined(RTL865X_MODEL_USER)
#include "ctype.h"
#else
#include "linux/ctype.h"
#endif

#define RTL_STATIC_INLINE   static __inline__

#define MBUF_COMPACT   //many of the routines in this module are work but no users yet. Can be trimmed...

/*TODO: 1. Support not yet provided if users want to allocate their own clusters
  *		2. Check what may happen for zombie clusters.
  *	Support:
  *		Test  mBuf_get/m_getPkthdr/m_freemfor ISR.
  */

/*********************************************************************************
	Section 1:	Exported public global variables
**********************************************************************************/
#define UNINIT_CLUSTER_SIZE (-1)
int32 m_clusterSize=UNINIT_CLUSTER_SIZE;	//Size of each cluster(If clusrers are allocated and managed by mbuf module)
/*********************************************************************************
	Section 2:	Internal structures and variables, see local mbuf_local.h
**********************************************************************************/

#include "mbuf_local.h"

static int32 _mbInited=0;
struct extCluster { //temporily place holder for allocated cluster addresses and their id mapping used in _m_get
	void *buf;
	uint32 id;
};
static struct extCluster *_clusters;
#define DEFAULT_MAX_RXDESC_IDX  (32767)
static int16 _maxRxDescidx= DEFAULT_MAX_RXDESC_IDX;
/*********************************************************************************
	Section 3:	Exported API prototype: Defined in mbuf.h
**********************************************************************************/
/*********************************************************************************
	Section 4:	Internal function prototype
**********************************************************************************/
static int32 _iTryGet(uint32 * piAvailableBuffers, uint32 iRequestedBuffers,  uint32 * piWaitID, int32 iHow, uint32 * piHaveWaited);
static uint32 _iFreeMbufChain(struct rtl_mBuf *pFirstMbuf, int8 flags);
static void _vWakeup(uint32 * piWaitID);
static void _vInitBufferPools(void *pBufPool, int32 iSize, int32 iNum);
RTL_STATIC_INLINE  void _vFreePkthdr(struct rtl_pktHdr *ph);
RTL_STATIC_INLINE uint8 _iGetClusterDataByte(struct rtl_mBuf *m, uint32 offset, uint32 *offsetTbl, uint32 *lenTbl, uint32 mbufs);
RTL_STATIC_INLINE void _vReturnFreeBuffer(void *pvFreeBuffer,  struct poolstat *pBufPoolStat);
RTL_STATIC_INLINE void *_pvGetFreeBuffer(struct poolstat *pBufPoolStat);
static struct rtl_mBuf *_pCopyPacket(struct rtl_mBuf *pMbuf, uint32 iHow);
/*********************************************************************************
	Section 5:	Exported API implementation
**********************************************************************************/
#if defined(RTL865X_TEST)||defined(RTL865X_MODEL_USER)
#define UNCACHED_MALLOC(x)  (void *) ((uint32) rtlglue_malloc(x))
#else
#define UNCACHED_MALLOC(x)  (void *) (UNCACHE((uint32) rtlglue_malloc(x)))
#endif /* RTL865X_TEST */
/* Initialize all mbuf module internal structures and variables. See mbuf.h */
int32 mBuf_init(uint32 nmbufs, uint32 nmbclusters, uint32 nmpkthdrs, uint32 clusterSize, memaddr logId)
{
	uint32 exponent;

	if ( _mbInited!=0)
	{
		return FAILED;
	}

	rtlglue_mbufMutexLock();

	_iTotalMbufs = nmbufs;

	_iTotalPkthdrs = nmpkthdrs;
	_mbufMsgLogId = logId;

#if defined(RTL865X_TEST)||defined(RTL865X_MODEL_USER)
	_MbufPool= rtlglue_malloc(_iTotalMbufs *sizeof(struct rtl_mBuf));	/* mbuf pool */
#else
	_MbufPool= UNCACHED_MALLOC(_iTotalMbufs *sizeof(struct rtl_mBuf));	/* mbuf pool */
#endif

	if(nmbclusters){
		//clusterSize must be power of 2
		if((isPowerOf2(clusterSize, &exponent)==FALSE)||(clusterSize < sizeof(struct mcluster_tag)))
			return FAILED;

		m_clusterSize = clusterSize;
		m_clusterShift = exponent;

		_ClusterDataPool= UNCACHED_MALLOC(nmbclusters * clusterSize); /* cluster data */
		_ClusterTag= (struct mcluster_tag *)rtlglue_malloc(nmbclusters * sizeof(struct mcluster_tag)); /* cluster data */
		_iTotalClusters = nmbclusters;
	}else{
		//cluster size still must be power of 2
		//if(isPowerOf2(clusterSize, &exponent)==FALSE)
		//	return FAILED;
		m_clusterSize = clusterSize;
		m_clusterShift =0;
		 _clusters= (struct extCluster *)rtlglue_malloc(sizeof(struct extCluster)*nmbufs);
		 if(!_clusters)
		 	return FAILED;
		//clusters would be allocated extrnally. 
		memset(_clusters, 0, sizeof(struct extCluster)*nmbufs);
		_iTotalClusters = 0;
		_ClusterDataPool =NULL;
		_ClusterTag=NULL;
	}

	_PkthdrPool= UNCACHED_MALLOC(_iTotalPkthdrs * sizeof(struct rtl_pktHdr)); /* cluster data */
	if(!_MbufPool || (nmbclusters&&(!_ClusterDataPool || !_ClusterDataPool)) || !_PkthdrPool)
		return FAILED;

	if(nmbclusters){
		_vInitBufferPools(_ClusterTag, sizeof(struct mcluster_tag),  _iTotalClusters);
		memset((void *) &_ClStatus, (uint8) 0,   sizeof(struct poolstat));
		/*
		   Initialize Data pool
		 */
		memset((void *) _ClusterDataPool, (uint8) 0,
			   _iTotalClusters * m_clusterSize);
		_ClStatus.pvAddr = _ClStatus.pvFreelist = _ClusterTag;
		_ClStatus.iTotalSize = _ClStatus.iFreebufs = _iTotalClusters;

	}



	/*
	   Clears content and chain neighboring elements. Own bits set to BUF_FREE(0x00) implicitly
	 */
	_vInitBufferPools(_MbufPool, sizeof(struct rtl_mBuf), _iTotalMbufs);
	_vInitBufferPools(_PkthdrPool, sizeof(struct rtl_pktHdr), _iTotalPkthdrs);

	/*
	   Initialize Pool status variables
	 */
	memset((void *) &_MbufStatus, (uint8) 0,   sizeof(struct poolstat));
	memset((void *) &_PHStatus, (uint8) 0,   sizeof(struct poolstat));

	_MbufStatus.pvAddr = _MbufStatus.pvFreelist = _MbufPool;
	_MbufStatus.iTotalSize = _MbufStatus.iFreebufs = _iTotalMbufs;


	_PHStatus.pvAddr = _PHStatus.pvFreelist = _PkthdrPool;
	_PHStatus.iTotalSize = _PHStatus.iFreebufs = _iTotalPkthdrs;


	memset((void *) &_Statistics, (uint8) 0, sizeof(struct rtl_mBufStatus));
	_Statistics.m_msize = sizeof(struct rtl_mBuf);	   /* length of an mbuf */
	_Statistics.m_pkthdrsize = sizeof(struct rtl_pktHdr);
	_Statistics.m_mclbytes=m_clusterSize;
	_mbInited = 1;
	rtlglue_mbufMutexUnlock();
	return SUCCESS;
}
//#ifdef RTL865X_TEST	// chenyl
#if defined(RTL865X_TEST)||defined(RTL865X_MODEL_USER)
/* Re-initialize all mbuf module internal structures and variables. See mbuf.h */
int32 mBuf_Reinit(void)
{

	if ( _mbInited!=1)
		return FAILED;

	rtlglue_mbufMutexLock();

	// reset mbuf pool
	memset(_MbufPool, 0, (_iTotalMbufs*sizeof(struct rtl_mBuf)));

	// reset Cluster
	if (_iTotalClusters)
	{	// internal Cluster
		memset(_ClusterDataPool, 0, (m_clusterSize*_iTotalClusters));
		memset(_ClusterTag, 0, (_iTotalClusters*sizeof(struct mcluster_tag)));
		memset((void *) &_ClStatus, (uint8) 0,   sizeof(struct poolstat));

		_vInitBufferPools(_ClusterTag, sizeof(struct mcluster_tag), _iTotalClusters);

		_ClStatus.pvAddr = _ClStatus.pvFreelist = _ClusterTag;
		_ClStatus.iTotalSize = _ClStatus.iFreebufs = _iTotalClusters;	
	}
	else
	{	// external Cluster
		if (_clusters)
			memset(_clusters, 0, (_iTotalMbufs*sizeof(struct extCluster)));
	}
	// reset pkthdr
	memset(_PkthdrPool, 0 , (_iTotalPkthdrs*sizeof(struct rtl_pktHdr)));

	/*
	   Clears content and chain neighboring elements. Own bits set to BUF_FREE(0x00) implicitly
	 */
	_vInitBufferPools(_MbufPool, sizeof(struct rtl_mBuf), _iTotalMbufs);
	_vInitBufferPools(_PkthdrPool, sizeof(struct rtl_pktHdr), _iTotalPkthdrs);

	// reset mbuf status
	memset((void *) &_MbufStatus, (uint8) 0,   sizeof(struct poolstat));
	_MbufStatus.pvAddr = _MbufStatus.pvFreelist = _MbufPool;
	_MbufStatus.iTotalSize = _MbufStatus.iFreebufs = _iTotalMbufs;

	// reset pktHdr status
	memset((void *) &_PHStatus, (uint8) 0,   sizeof(struct poolstat));
	_PHStatus.pvAddr = _PHStatus.pvFreelist = _PkthdrPool;
	_PHStatus.iTotalSize = _PHStatus.iFreebufs = _iTotalPkthdrs;

	// reset statistics
	memset((void *) &_Statistics, (uint8) 0, sizeof(struct rtl_mBufStatus));
	_Statistics.m_msize = sizeof(struct rtl_mBuf);	   /* length of an mbuf */
	_Statistics.m_pkthdrsize = sizeof(struct rtl_pktHdr);
	_Statistics.m_mclbytes=m_clusterSize;
	_mbInited = 1;
	rtlglue_mbufMutexUnlock();
	return SUCCESS;
}
#endif /* RTL865X_TEST */

/* Get mbuf module buffer pool status , see mbuf.h*/
int32 mBuf_getBufStat(struct rtl_mBufStatus *mbs){
	if (!mbs || _mbInited!=1)
		return FAILED;
	mbs->m_totalmbufs=_iTotalMbufs;
	mbs->m_totalclusters=_iTotalClusters;
	mbs->m_totalpkthdrs=_iTotalPkthdrs;
	mbs->m_freembufs=  _MbufStatus.iFreebufs;	   /* mbufs obtained from page pool */
	mbs->m_freeclusters=_ClStatus.iFreebufs;	   /* clusters obtained from page pool */
	mbs->m_freepkthdrs=_PHStatus.iFreebufs;	   /* number of free pkthdrs */
	mbs->m_msize = _Statistics.m_msize;		   /* length of an mbuf */
	mbs->m_mclbytes=_Statistics.m_mclbytes;	   /* length of an mbuf cluster */
	mbs->m_pkthdrsize= _Statistics.m_pkthdrsize;   /* length of an pkthdr */
	mbs->m_wait= _Statistics.m_wait;	  /* times waited for space, includes mbuf, pkthdr and cluster */
	return SUCCESS;
}

/*
 * Compute the amount of space available
 * before the current start of data in a cluster.
 */
int32 mBuf_leadingSpace(struct rtl_mBuf * m)
{
	int32 retval;

	assert(m &&	ISSET(m->m_flags, MBUF_USED) && (m->m_flags & MBUF_EXT));
	retval = (int32)(m->m_data - m->m_extbuf);

	return retval;
}

int32 mBuf_reserve(struct rtl_mBuf * m, uint16 headroom){
	assert(m);
	assert(headroom);
	if(m->m_len){
		return FAILED;
	}
	if(m->m_extsize<headroom){
		return FAILED;
	}
	m->m_data=m->m_extbuf;
	MBUF_RESERVE(m, headroom);
	return SUCCESS;
}


/*
 * Compute the amount of space available
 * after the end of data in an cluster.
 */
int32 mBuf_trailingSpace(struct rtl_mBuf * m)
{
	int32 retval;

	assert(m && ISSET(m->m_flags, MBUF_USED) &&(m->m_flags & MBUF_EXT));
	retval =  (( (memaddr)m->m_extbuf + m->m_extsize) - ((memaddr)m->m_data + m->m_len));

	return retval;

}

/*
 * Check if we can write to the cluster. Only the owner of this cluster can write.
 */
int32 mBuf_clusterIsWritable(struct rtl_mBuf * m)
{
	int32 retval;
/*	Note: When clusters are allocated externally by OS, mbuf module doesn't know which mbuf
*		is the first referee and owns the write priviledge to cluster. Therefore, write priviledge
*		is granted to ALL cluster referees.
*/	
	if(!_iTotalClusters)
		return TRUE;
	if (CLDataAddr2TagAddr(m->m_extbuf)->cltag_mbuf == m)
		retval = (CLDataAddr2TagAddr(m->m_extbuf)->cltag_refcnt == 1);
	else
		retval = 0;

	return retval;

}

/*Get packet length. see mbuf.h */
static uint32 _m_getPktlen(struct rtl_mBuf *mp){
	struct rtl_mBuf *m=mp;
	uint32 pktlen=0;
	assert(mp);
	assert(ISSET(m->m_flags, MBUF_USED));

	//calculate total pktlen
	if(m->m_pkthdr)	//If there is a packet header, use ph_len directly
		pktlen = m->m_pkthdr->ph_len;
	else{
		pktlen = 0;	//since we don't assume pkthdr exists, we calculate pktlen manually via 'pktlen'
		for (;m;)		//find the last mbuf and pktlen pktlen
		{
			pktlen += m->m_len;
			if (m->m_next == (struct rtl_mBuf *) 0){
				m=mp;
				break;
			}
			m = m->m_next;
		}
	}
	return pktlen;
}

uint32 mBuf_getPktlen(struct rtl_mBuf *mp){
	uint32 ret;
	rtlglue_mbufMutexLock();
	//we need lock becoz we will traverse linked list.
	ret = _m_getPktlen(mp);
	rtlglue_mbufMutexUnlock();
	return ret;
}

#ifndef MBUF_COMPACT
// Given data addr(Always in cluster) -->return mbuf addr
struct rtl_mBuf *mBuf_data2Mbuf(int8 * x)
{
	struct rtl_mBuf *retval;
	assert(x);
	assert(_iTotalClusters);//not appliable when clusters allocated externally by OS.
	retval = CLDataAddr2TagAddr(x)->cltag_mbuf;
	return retval;
}
#endif

static struct rtl_mBuf *_m_get(int32 how, uint32 Nbuf)
{
	register int32      iIndex=0, iHow, extClusterAllocated=0;
	register struct rtl_mBuf *pThisMbuf, *pFirstMbuf, *pPrevMbuf;
	register struct mcluster_tag *pThisClusterTag;
	uint32 iMbufWaited;
	uint32 iClusterWaited;
	assert(how);
	assert(Nbuf > 0);

	pThisMbuf = pFirstMbuf = pPrevMbuf = NULL;
	pThisClusterTag = NULL;
	iMbufWaited = iClusterWaited = FALSE;

	iHow = how;

	//see if we can get all mbufs. Max mbuf chain size is 64K.
	//Note: An mbuf chain can NEVER grow up to 65536 bytes, this overflows ph_len.
	if (Nbuf>_MbufStatus.iFreebufs){
		goto out;
	}
	if(Nbuf*m_clusterSize>65536){
		goto out;
	}
	if(_iTryGet(&_MbufStatus.iFreebufs, Nbuf, &_MbufStatus.iWaiting, iHow,&iMbufWaited) == FALSE)
		goto out;

	//Do we need clusters?  if we do,try to get all clusters at once.
	if (ISCLEARED(iHow, MBUF_ONLY))	{
		if(_iTotalClusters==0){
			//allocate all requested clusters one by one
			//MBUF_WAITOK is NOT permitted.
			if(ISSET(iHow, MBUF_WAITOK))
				goto out;
			for(extClusterAllocated=0; extClusterAllocated < Nbuf; extClusterAllocated++){		
				if(rtlglue_mbufAllocCluster(&_clusters[extClusterAllocated].buf,m_clusterSize,&_clusters[extClusterAllocated].id)==FAILED){
					int32 release;
					//can't allocate all requested clusters. Release all already allocated.
					for(release=0; release < extClusterAllocated; release++)
						rtlglue_mbufFreeCluster(_clusters[release].buf,m_clusterSize,_clusters[release].id);
					goto out;
				}
				assert(_clusters[extClusterAllocated].buf);
			}
		}else{
			//see if we can get all clusters
			if ((Nbuf>_iTotalClusters)||
				(_iTryGet(&_ClStatus.iFreebufs, Nbuf, &_ClStatus.iWaiting, iHow,&iClusterWaited) == FALSE))
					goto out;
		}
	}
	CLEARBITS(iHow, MBUF_WAITOK);	   //Clear wait bit.

	/* We now have enough mbufs and clusters(if MBUF_ONLY isn't set)  */

	if (ISSET(iHow, MBUF_ONLY))
	{	//user want mbuf only. Maybe it would be used to reference other allocated clusters or maybe user wishes to manage his own cluster pool
		for (iIndex = 0; iIndex < Nbuf; iIndex++){		   // Initialize all Nbuf mbufs
			assert(_MbufStatus.pvFreelist);
			pThisMbuf = (struct rtl_mBuf *) _pvGetFreeBuffer(&_MbufStatus);
			assert(pThisMbuf);
			memset((void *) pThisMbuf, (uint8) 0, sizeof(struct rtl_mBuf));	//Initialize mbuf to 0
			CLEARBITS(pThisMbuf->m_flags, MBUF_EXT);
			SETBITS(pThisMbuf->m_flags, MBUF_USED);
			pThisMbuf->m_next = NULL;
#ifdef CONFIG_RTL865XC
			pThisMbuf->m_rxDesc = 0;
#endif
			if (iIndex == 0)
				pFirstMbuf = pThisMbuf;	/* Return this pointer to user */
			else
				pPrevMbuf->m_next = pThisMbuf;	/*Chain together */
			pPrevMbuf = pThisMbuf;
		}
	} else
	{
		//Normal case, user wants both an mbuf and a cluster
		for (iIndex = 0; iIndex < Nbuf; iIndex++)
		{							   // Initialize all Nbuf mbufs
			assert(_MbufStatus.pvFreelist);
			pThisMbuf = (struct rtl_mBuf *) _pvGetFreeBuffer(&_MbufStatus);

			assert(pThisMbuf);

			//Initializing mbuf*/
			memset((void *) pThisMbuf, (uint8) 0, sizeof(struct rtl_mBuf));
			SETBITS(pThisMbuf->m_flags, MBUF_EXT | MBUF_USED);
			pThisMbuf->m_next = NULL;
#ifdef CONFIG_RTL865XC
			pThisMbuf->m_rxDesc = 0;
#endif
			if(_iTotalClusters)
			{
				assert(_ClStatus.pvFreelist);
				pThisClusterTag =	(struct mcluster_tag *) _pvGetFreeBuffer(&_ClStatus);
				assert(pThisClusterTag);
				pThisMbuf->m_extbuf = pThisMbuf->m_data =
					(uint8 *) CLTagAddr2DataAddr(pThisClusterTag);
				/*initialize corresponding cluster tag, cluster is NOT cleared to save time*/
				pThisClusterTag->cltag_refcnt = 1;
				SETBITS(pThisClusterTag->cltag_flags, CLUSTER_USED);
				pThisClusterTag->cltag_mbuf = pThisMbuf;	/* points back to pThisMbuf */
			} else
			{
				pThisMbuf->m_extbuf = pThisMbuf->m_data =  (uint8 *)_clusters[iIndex].buf;
				pThisMbuf->m_extClusterId = _clusters[iIndex].id;
			}				
			pThisMbuf->m_len = 0;
			pThisMbuf->m_extsize = m_clusterSize;

			if (iIndex == 0)
				pFirstMbuf = pThisMbuf;	/* Return this pointer to user */
			else
				pPrevMbuf->m_next = pThisMbuf;	/*Chain together */
			pPrevMbuf = pThisMbuf;

		}
	}

	if(_iTotalClusters){
	/*
	   see if we can wakeup another one.
	   **only when clusters are aloocated my mbuf module itself***
	 */
		if (iClusterWaited == TRUE && _ClStatus.pvFreelist)	{
			_Statistics.m_wait++;
			_vWakeup(&_ClStatus.iWaiting); /* wakeup another one */
		}
		else if (iMbufWaited == TRUE && _MbufStatus.pvFreelist){
			_Statistics.m_wait++;
			_vWakeup(&_MbufStatus.iWaiting);	/* wakeup another one */
		}
	}
out:

	if (iIndex == Nbuf)
		return (pFirstMbuf);		   //Return success only when all requested buffers allocated

	return NULL;					   //Return Fail
}

/*Allocate mbuf and clusters. See document in mbuf.h*/
 struct rtl_mBuf *mBuf_get(int32 how, int32 type, uint32 Nbuf){
	//parameter 'type' is ignored. No longer in use. Keep for backward compatiable
	struct rtl_mBuf *m;
	rtlglue_mbufMutexLock();
	m=_m_get( how, Nbuf);
	rtlglue_mbufMutexUnlock();
	return m;
}


//get one mbuf, get a pkthdr and attach it with assigned cluster
struct rtl_mBuf *mBuf_attachHeader(void *buffer, uint32 id, uint32 bufsize,uint32 datalen, uint16 align){
	struct rtl_mBuf *m=NULL;
	struct rtl_pktHdr *ph=NULL;	
	rtlglue_mbufMutexLock();

	//no more free buffers or alignment offset out of range.
	if((!_MbufStatus.iFreebufs)||(!_PHStatus.iFreebufs)||(align &&align>=bufsize)) 
		goto noFreeEntry;
	
	m = (struct rtl_mBuf *) _pvGetFreeBuffer(&_MbufStatus);
	ph = (struct rtl_pktHdr *) _pvGetFreeBuffer(&_PHStatus);

	ph->ph_mbuf = m;
	ph->ph_len = datalen;
	ph->ph_flags= PKTHDR_USED;
	*((uint16 *)((uint8 *)ph)+8)=0;

	ph->ph_iphdrOffset=0;
	ph->ph_pppeTagged = 0;
	ph->ph_LLCTagged = 0;
	ph->ph_vlanTagged = 0;
#ifdef CONFIG_RTL865XC  	
	ph->ph_flags2 = 0;
	ph->ph_vlanIdx = 0;
	ph->ph_portlist =0;
#endif
	
	ph->ph_rxdesc = PH_RXDESC_INDRV;
	ph->ph_rxPkthdrDescIdx = PH_RXPKTHDRDESC_INDRV;
	m->m_next = NULL;
	m->m_pkthdr = ph;
	m->m_len=datalen;
	m->m_flags=MBUF_USED|MBUF_EXT|MBUF_PKTHDR;
	m->m_data = ((uint8 *)buffer)+align;
	m->m_extbuf=buffer;
	m->m_extClusterId = id;
	m->m_extsize= bufsize;
#ifdef CONFIG_RTL865XC
	m->m_rxDesc = 0;
#endif

	rtlglue_mbufMutexUnlock();
	return m;

noFreeEntry:	
	rtlglue_mbufMutexUnlock();	
	return NULL;
 }


//get mbufs, get a pkthdr and attach it with assigned cluster, return first mbuf
struct rtl_mBuf *mBuf_attachHeaderJumbo(void *buffer, uint32 id, uint32 bufsize,uint32 datalen){
	struct rtl_mBuf *m=NULL,*pm=NULL;
	struct rtl_pktHdr *ph=NULL;	
	uint32 nonAttachedSize=datalen;	
	
	rtlglue_mbufMutexLock();

	// no more free pkthdr buffers  
	if(!_PHStatus.iFreebufs) 
	{
		goto noFreePkt;
	}

	ph = (struct rtl_pktHdr *) _pvGetFreeBuffer(&_PHStatus);

	ph->ph_len = datalen;

	ph->ph_flags= PKTHDR_USED;
	*((uint16 *)((uint8 *)ph)+8)=0;

	ph->ph_iphdrOffset=0;
	ph->ph_pppeTagged = 0;
	ph->ph_LLCTagged = 0;
	ph->ph_vlanTagged = 0;
#ifdef CONFIG_RTL865XC
	ph->ph_flags2 = 0;
	ph->ph_vlanIdx = 0;
#endif
	
	ph->ph_rxdesc = PH_RXDESC_INDRV;
	ph->ph_rxPkthdrDescIdx = PH_RXPKTHDRDESC_INDRV;
	
	while(1)
	{
		//no more free mbuf buffers 
		if (!_MbufStatus.iFreebufs) 
			goto noFreeMbuf;
		
		m = (struct rtl_mBuf *) _pvGetFreeBuffer(&_MbufStatus);

		m->m_pkthdr = ph;
#ifdef CONFIG_RTL865XC
		m->m_rxDesc = 0;
#endif

		if(pm!=NULL) 
		{
			m->m_flags=MBUF_USED|MBUF_EXT;		
			pm->m_next=m;
		}
		else
		{
			m->m_flags=MBUF_USED|MBUF_EXT|MBUF_PKTHDR;
			ph->ph_mbuf = m;
		}

		m->m_extbuf = m->m_data = ((uint8 *)buffer+(datalen-nonAttachedSize));
		m->m_extsize= bufsize;
		m->m_extClusterId = id;

		if(nonAttachedSize>=bufsize)
		{
	
			nonAttachedSize-=bufsize;
			m->m_len=bufsize;		
		}
		else
		{
	
			m->m_next = NULL;		
			m->m_len=nonAttachedSize;		
			break;
		}

		pm=m;
		
	}

	rtlglue_mbufMutexUnlock();
	return ph->ph_mbuf ;
	
noFreeMbuf:
	if(m==NULL)
		mBuf_freePkthdr(ph);
	else
		mBuf_freeMbufChain(ph->ph_mbuf);
noFreePkt:	
	rtlglue_mbufMutexUnlock();
	return NULL;
 }


int mBuf_attachHeaderLSO(uint32 skb_len,uint32 skb_data_len,uint8 *skb_data,uint32 skb,uint8 protocol, struct rtl_mBuf *m[32],uint8 isVlanTagged){
	
	struct rtl_pktHdr *ph[32],*phi;
	struct rtl_mBuf *mi;
	int loop=0,size=skb_data_len;
	uint32 header_len=(skb_len-skb_data_len);
	uint32 mss=0;
	
	if(isVlanTagged)
		mss=1518-header_len;
	else
		mss=1514-header_len;
	
	rtlglue_mbufMutexLock();


	if(protocol==0x06) //TCP
	{
		while(1)
		{
			size-=mss;		
			loop++;
			if(size<=0) break;
		}	
		/* 
			if a skb's xmit times not multiple of two, must change mss to be multiple of two.
			else must waiting for a ACK reply about 0.2 sec in windows OS. (for SAMBA...)
		*/
		if((loop>1) && ((loop&1)==1))
		{
			mss=(skb_data_len/(loop+1))+1;		
			/*
				for 2 bytes alignment issue in slowPath
			*/
			if(mss&1) mss++;
		}	
		loop=0;
		size=skb_data_len;
	}

	while(1)
	{
		if (!_MbufStatus.iFreebufs||!_PHStatus.iFreebufs) 
			goto out;

		m[loop] = (struct rtl_mBuf *) _pvGetFreeBuffer(&_MbufStatus);
		ph[loop] = (struct rtl_pktHdr *) _pvGetFreeBuffer(&_PHStatus);

		mi=m[loop];
		phi=ph[loop];

		phi->ph_mbuf = mi;
		phi->ph_flags = PKTHDR_USED;
		phi->ph_iphdrOffset = 0;
		phi->ph_pppeTagged = 0;
		phi->ph_LLCTagged = 0;
		phi->ph_vlanTagged = 0;
#ifdef CONFIG_RTL865XC	
		phi->ph_flags2 = 0;
		phi->ph_vlanIdx = 0;
#endif

		/* Turn on this bit to ask ROMEDRV to calculate ip-in-ip L3 checksum. */
		phi->ph_pkt_property = 0;	// Initialize
		phi->ph_pkt_property |= PH_PKT_PROPER_IPIP_L3CSM;
		phi->ph_pkt_property |= PH_PKT_PROPER_IPIP_L4CSM;

		*((uint16 *)((uint8 *)ph)+8)=0;
		phi->ph_rxdesc = PH_RXDESC_INDRV;
		phi->ph_rxPkthdrDescIdx = PH_RXPKTHDRDESC_INDRV;
		mi->m_next = NULL;
#ifdef CONFIG_RTL865XC
		mi->m_rxDesc = 0;
#endif
		mi->m_pkthdr = phi;		
		mi->m_flags=MBUF_USED|MBUF_EXT|MBUF_PKTHDR;		

		mi->m_len = phi->ph_len = header_len+((size>mss)?mss:size);		
		size-=mss;
		mi->m_extbuf = mi->m_data = (uint8*)UNCACHE(skb_data)+mss*loop;
		
		if(size<=0) 
		{
			mi->m_extClusterId = (uint32)skb;
			break;
		}
		mi->m_extClusterId = 0;
		mi->m_extsize= mi->m_len;
		loop++;		
	}
	
	rtlglue_mbufMutexUnlock();
	return SUCCESS;
	
out:	
	rtlglue_mbufMutexUnlock();
	return FAILED;

 }

#if 0
int mBuf_attachHeaderLSO_2mbuf(uint32 skb_len,uint32 skb_data_len,uint8 *skb_data,uint32 skb,uint8 protocol, struct rtl_mBuf *m[64]){
	
	struct rtl_pktHdr *ph[32],*phi;
	struct rtl_mBuf *mi=NULL,*mih;
	int loop=0,loop2=0,size=skb_len;
	uint32 header_len=(skb_len-skb_data_len);
	uint32 mss=1514-header_len;
	rtlglue_mbufMutexLock();


	if(protocol==0x06) //TCP
	{
		while(1)
		{
			size-=mss;		
			loop++;
			if(size<=0) break;
		}	
		/* 
			if a skb's xmit times not multiple of two, must change mss to be multiple of two.
			else must waiting for a ACK reply about 0.2 sec in windows OS. (for SAMBA...)
		*/
		if((loop>1) && ((loop&1)==1))
		{
			mss=(skb_data_len/(loop+1))+1;		
			/*
				for 2 bytes alignment issue in slowPath
			*/
			if(mss&1) mss++;
		}	
		loop=0;
		size=skb_len;
	}

	while(1)
	{
		if (!_MbufStatus.iFreebufs)
			goto out;		
		m[loop2] = (struct rtl_mBuf *) _pvGetFreeBuffer(&_MbufStatus);
		_MbufStatus.iUsedbufs++;
		mih=m[loop2];	

		if(!_PHStatus.iFreebufs) 
			goto out;
		ph[loop] = (struct rtl_pktHdr *) _pvGetFreeBuffer(&_PHStatus);
		_PHStatus.iUsedbufs++;
		phi=ph[loop];
	
		
		if(loop!=0)
		{
			if (!_MbufStatus.iFreebufs)
				goto out;
			m[loop2+1] = (struct rtl_mBuf *) _pvGetFreeBuffer(&_MbufStatus);
			_MbufStatus.iUsedbufs++;	
			mi=m[loop2+1];
			mih->m_next=mi;
#ifdef CONFIG_RTL865XC
			mih->m_rxDesc = 0;
#endif
			mi->m_next = NULL;			
			mih->m_len=header_len;
			mi->m_len = ((size>mss)?mss:size);	
			phi->ph_len = mi->m_len+mih->m_len;
			mi->m_pkthdr = phi;
			mi->m_flags=MBUF_USED|MBUF_EXT;
			mi->m_extbuf = mi->m_data = (uint8*)UNCACHE(skb_data)+mss*loop;
			mi->m_extClusterId = 0;
			mi->m_extsize= mi->m_len;
		}
		else
		{
			mih->m_next=NULL;
#ifdef CONFIG_RTL865XC
			mih->m_rxDesc = 0;
#endif
			mih->m_len=phi->ph_len =((skb_len>mss)?mss:skb_len);				

		}

		
		mih->m_pkthdr = phi;
		mih->m_flags=MBUF_USED|MBUF_EXT|MBUF_PKTHDR;			
		mih->m_extbuf =( mih->m_data = (uint8*)UNCACHE(skb_data));			


		phi->ph_mbuf = mih;
		phi->ph_flags= PKTHDR_USED;
		phi->ph_iphdrOffset=0;
		phi->ph_pppeTagged = 0;
		phi->ph_LLCTagged = 0;
		phi->ph_vlanTagged = 0;

		*((uint16 *)((uint8 *)ph)+8)=0;
		phi->ph_rxdesc = PH_RXDESC_INDRV;
		phi->ph_rxPkthdrDescIdx = PH_RXPKTHDRDESC_INDRV;
		
		size-=mss;		
		
		if(size<=0) 
		{
			if(loop==0)
				{
				mih->m_extClusterId = (uint32)skb;
				}
			else
				mi->m_extClusterId = (uint32)skb;
			break;
		}
		
		mih->m_extClusterId = 0;										
		mih->m_extsize=mih->m_len;
	
		loop++;		
		loop2+=2;
	}

	
	rtlglue_mbufMutexUnlock();
	return SUCCESS;
	
out:		
	rtlglue_mbufMutexUnlock();
	return FAILED;

 }
#endif

#if 0
struct rtl_mBuf *mBuf_attachHeaderSG(struct sk_buff * skb,uint32 bufsize, uint16 align){
	//void *buffer, uint32 id, uint32 bufsize,uint32 datalen, uint16 align
	struct rtl_mBuf *m=NULL;
	struct rtl_pktHdr *ph=NULL;
	rtlglue_mbufMutexLock();

	//no more free buffers or alignment offset out of range.
	if (!_MbufStatus.iFreebufs||!_PHStatus.iFreebufs||(align &&align>=bufsize)) 
		goto out;

	ph = (struct rtl_pktHdr *) _pvGetFreeBuffer(&_PHStatus);
	_PHStatus.iUsedbufs++;
	//_PHStatus.iFreebufs--;	
	
	m = (struct rtl_mBuf *) _pvGetFreeBuffer(&_MbufStatus);
	_MbufStatus.iUsedbufs++;
	
	//_MbufStatus.iFreebufs--;	
	ph->ph_mbuf = m;
	ph->ph_len = skb->len;
	ph->ph_flags= PKTHDR_USED;
	*((uint16 *)((uint8 *)ph)+8)=0;
	/*
	ph->ph_iphdrOffset=0;
	ph->ph_pppeTagged = 0;
	ph->ph_LLCTagged = 0;
	ph->ph_vlanTagged = 0;
	*/

	ph->ph_rxdesc = PH_RXDESC_INDRV;
	ph->ph_rxPkthdrDescIdx = PH_RXPKTHDRDESC_INDRV;
	
	
	m->m_pkthdr = ph;
#ifdef CONFIG_RTL865XC
	m->m_rxDesc = 0;
#endif
	m->m_len=skb->tail-skb->data;
	m->m_flags=MBUF_USED|MBUF_EXT|MBUF_PKTHDR;
	m->m_data = ((uint8 *)UNCACHE(skb->data))+align;
	m->m_extbuf=(uint8 *)UNCACHE(skb->data);
	m->m_extClusterId = (uint32)skb;
	m->m_extsize= bufsize;

	if(m->m_len<ph->ph_len)
	{
		int i;
		struct skb_shared_info *shinfo;	
		struct rtl_mBuf *old_m=m;
		struct rtl_mBuf *new_m=NULL;
		uint8 *v;
#ifdef SG_DEBUG			
		rtlglue_printf("send it.........................\n");
#endif
		
		shinfo=(struct skb_shared_info *)skb->end;
		for(i=0;i<shinfo->nr_frags;i++)
		{				

			if (!_MbufStatus.iFreebufs)
			{
				old_m->m_next = NULL;
				goto out; //must clear pkt and mbuf first
			}

			new_m = (struct rtl_mBuf *) _pvGetFreeBuffer(&_MbufStatus);
			_MbufStatus.iUsedbufs++;

			//v=(uint8 *)UNCACHE((uint32)(page_address(shinfo->frags[i].page))+shinfo->frags[i].page_offset);
			v=page_address(shinfo->frags[i].page);
			v=(uint8*)UNCACHE(v)+shinfo->frags[i].page_offset;
			
			old_m->m_next=new_m;
		
			new_m->m_pkthdr = ph;
			new_m->m_len=shinfo->frags[i].size;			
			new_m->m_flags=MBUF_USED|MBUF_EXT;
			new_m->m_data = v+align;
			new_m->m_extbuf=v;
			new_m->m_extClusterId = (uint32)(skb);
			new_m->m_extsize=bufsize;
#ifdef CONFIG_RTL865XC
			new_m->m_rxDesc = 0;
#endif
			old_m=new_m;
#ifdef SG_DEBUG			
			rtlglue_printf("nr_frags=%d frags[i].size=%d .page_offset=%d .size=%d \n",i,shinfo->frags[i].size,shinfo->frags[i].page_offset,shinfo->frags[i].size);
			memDump(v,32,"test");
#endif			
		}		
		old_m->m_next=NULL;
		old_m->m_flags|=MBUF_EOR;


#ifdef SG_DEBUG			

		//patch for hardware must padding to > 60 bytes		
		{
			int must_pad=128-(int)m->m_len;
			if(must_pad>0)
			{
				new_m=m->m_next;
				if(new_m->m_len>=must_pad)
				{					
					memcpy(m->m_data+m->m_len,new_m->m_data,must_pad);
					m->m_len+=must_pad;
					new_m->m_data+=must_pad;
					new_m->m_len-=must_pad;
				}
			}
		}
		

		//debug
		rtlglue_printf("--------------------------\n");
		new_m=ph->ph_mbuf;
		while(new_m!=NULL)
		{
			memDump(new_m->m_data,30,"test");
		
			new_m=new_m->m_next;
		}
#endif		
		
		

	}
	else	
		m->m_next = NULL;
out:
	rtlglue_mbufMutexUnlock();
	return m;
 }
#endif


#ifndef MBUF_COMPACT
/*Same as mBuf_get,but initialize all clusters to zero */
struct rtl_mBuf *mBuf_getCleared(int32 how, int32 type, uint32 Nbuf)
{
	register struct rtl_mBuf *m, *n;
	int32     i = 0;

	assert(Nbuf);
	rtlglue_mbufMutexLock();
	//parameter 'type' is ignored. No longer in use. Keep for backward compatiable
	m = _m_get(how, Nbuf);
	if (m){
		assert(ISSET(m->m_flags, MBUF_USED));

		if(ISSET(how, MBUF_ONLY))
			goto out;

		//Clear the content in clusters allocated.
		assert(ISCLEARED(how, MBUF_ONLY));
		for (n = m, i = 0; i < Nbuf; i++, n = n->m_next)
			memset(MBUF2DATAPTR(n, int8 *), 0, m->m_extsize);
	}
out:
	rtlglue_mbufMutexUnlock();
	return (m);
}
#endif

/*Given an mbuf, allocate a pkthdr for it. See document in mbuf.h */
static struct rtl_mBuf *_m_gethdr(struct rtl_mBuf *pMbuf, int32 how){
	int32 iLen = 0;
	struct rtl_mBuf *pThisMbuf = NULL;
	struct rtl_pktHdr *pPkthdr = NULL;
	uint32 PkthdrWaited;

	assert(how &&pMbuf);
	assert(ISCLEARED(pMbuf->m_flags, MBUF_PKTHDR));

	iLen = pMbuf->m_len;
	// Should there be an mbuf in given mbuf chain who holds a pkthdr, rob the pkthdr's ownership and makes pMbuf the owner.
	for (pThisMbuf = pMbuf->m_next; pThisMbuf; pThisMbuf = pThisMbuf->m_next)
	{
		//find a pkthdr owner if there is one
		if (ISSET(pThisMbuf->m_flags, MBUF_PKTHDR)){
			assert(ISSET(pPkthdr->ph_flags, PKTHDR_USED));
			assert(pPkthdr->ph_mbuf == pThisMbuf);
			pPkthdr = pThisMbuf->m_pkthdr;
			pMbuf->m_pkthdr = pPkthdr;
			pPkthdr->ph_mbuf = pMbuf;
			pPkthdr->ph_len += iLen;   //increment packet length
			SETBITS(pMbuf->m_flags, MBUF_PKTHDR);	// make pBuf the pkthdr owner
			for (; pThisMbuf && (pThisMbuf->m_next != NULL); pThisMbuf = pThisMbuf->m_next)
				CLEARBITS(pThisMbuf->m_flags, MBUF_PKTHDR);	//clear the rested mbufs' PKTHDR bit if exist
			return pMbuf;
		}
		iLen += pThisMbuf->m_len;
	}


	/*
	   No mbufs already have a pkthdr, so we need to allocate one.
	 */

	//see if we can get a pkthdr
	if ((!_PHStatus.pvFreelist) &&
		(_iTryGet(&_PHStatus.iFreebufs, 1, &_PHStatus.iWaiting, how, &PkthdrWaited) == FALSE))
			goto out;

	/*
	   Yes we can...
	 */
	assert(_PHStatus.pvFreelist);
	pPkthdr = (struct rtl_pktHdr *) _pvGetFreeBuffer(&_PHStatus);
	assert(pPkthdr);
	memset((void *) pPkthdr, (uint8) 0, sizeof(struct rtl_pktHdr));	//initialize pkthdr to 0
	SETBITS(pPkthdr->ph_flags, PKTHDR_USED);
	pPkthdr->ph_mbuf = pMbuf;
	pPkthdr->ph_len += iLen;
	pMbuf->m_pkthdr = pPkthdr;
	SETBITS(pMbuf->m_flags, MBUF_PKTHDR);	/* Now we are the head of this mbuf chain */

	//change all mbuf's m_pkthdr pointer
	for (pThisMbuf = pMbuf->m_next; pThisMbuf;pThisMbuf = pThisMbuf->m_next)
		pThisMbuf->m_pkthdr = pPkthdr;

	if (PkthdrWaited == TRUE && _PHStatus.pvFreelist){
		_Statistics.m_wait++;
		_vWakeup(&_PHStatus.iWaiting);	/* wakeup another one */
	}

out:
	if (pPkthdr)
		return pMbuf;				   //return success if a pkthdr is allocated
	else
		return NULL;
}


 struct rtl_mBuf *mBuf_getPkthdr(struct rtl_mBuf *pMbuf, int32 how)
 {
 	struct rtl_mBuf *m;
	rtlglue_mbufMutexLock();
	m = _m_gethdr( pMbuf,  how);
	m->m_pkthdr->ph_rxdesc = PH_RXDESC_INDRV;
	m->m_pkthdr->ph_rxPkthdrDescIdx = PH_RXPKTHDRDESC_INDRV;
	m->m_pkthdr->ph_priority = 0;	/* Init priority field in pkthdr. Set default priority level to 0. */
	rtlglue_mbufMutexUnlock();
	return m;
	
 }


/*Free a pkthdr alone. This is usd in TCP, when appending incoming packet to TCP receive buffer and the pkthdr attached should be
removed.  See document in mbuf.h*/
void mBuf_freePkthdr(struct rtl_pktHdr *ph){
	rtlglue_mbufMutexLock();
	_vFreePkthdr(ph);
	rtlglue_mbufMutexUnlock();
}




/* This will allocate len-worth of  mbuf clusters and return a pointer to the top
 * of the allocated chain. See document in mbuf.h
 */
struct rtl_mBuf *mBuf_getm(struct rtl_mBuf *m, uint32 len, int32 how, int32 type){
	register struct rtl_mBuf *pMbuf = m, *mtail = NULL, *p=NULL;
	register int32     trailing_len = 0, Nbuf = 0;

	assert(how);
	assert(len);

	//Exceed maximum mbuf chain limit?
	if(len> 65535)
		return pMbuf;

	rtlglue_mbufMutexLock();

	if (m != NULL){	   //we are given an mbuf chain,  so we will attach new mbufs after given mbuf chain.
		assert(ISSET(m->m_flags, MBUF_USED));
		for (mtail = m; mtail->m_next != NULL; mtail = mtail->m_next) ;
		trailing_len = mBuf_trailingSpace(mtail);
		//mtail would be used later to concatenate old mbuf chain and new mbuf chain
		if (trailing_len >= len)	   // Trailing space is large enough, No need to allocate new mbufs
			goto out;
		len -= trailing_len;
	}

	//Calculate how many mbufs we need
	Nbuf = ((len - 1) / m_clusterSize) + 1;
	assert(Nbuf > 0);

	CLEARBITS(how, MBUF_ONLY);

	//Caution:pMbuf is reused here
	pMbuf = _m_get(how, Nbuf);

	if (!pMbuf)
		goto out;

	if (m != NULL)	{
		if(m->m_pkthdr){//if m has a pkthdr, so do all new mbufs
			for (p = pMbuf; p; p = p->m_next)
				p->m_pkthdr = m->m_pkthdr;
		}
		mtail->m_next = pMbuf;
		pMbuf = m;
	}else{
		//We only allocate a pkthdr when user says he wants one and
		//user didn't give us an mbuf to chain with m
		if (ISSET(how, MBUF_ALLOCPKTHDR) && (pMbuf != _m_gethdr(pMbuf, how)))
		{
			_iFreeMbufChain(m, MBUF_FREEALL);
			pMbuf = NULL;
			goto out;
		}
		pMbuf->m_pkthdr->ph_rxdesc = PH_RXDESC_INDRV;
		pMbuf->m_pkthdr->ph_rxPkthdrDescIdx = PH_RXPKTHDRDESC_INDRV;

		for (p = pMbuf; p; p = p->m_next)
			p->m_pkthdr = pMbuf->m_pkthdr;
	}
out:
	rtlglue_mbufMutexUnlock();
	return pMbuf;

}

/* Arrange to prepend space of size plen to mbuf m and adjust data pointers
 * If a new mbuf must be allocated, how specifies whether to wait.
 *  See document in mbuf.h
 */
struct rtl_mBuf *mBuf_prepend(struct rtl_mBuf *m, uint32 plen, int32 how){
	struct rtl_mBuf *pMbuf=m, *mm;
	int32     leading_len, Nbuf, start;
	int32	count=0;


	assert(m);
	assert(ISSET(m->m_flags, MBUF_USED));

	rtlglue_mbufMutexLock();
	//By default, m_prepedn uses any leading free space before allocating any new clusters.
	//But if M_GETNEWMBUF is set, mBuf_prepend allocates new mbufs directly.
	if (ISSET(how, MBUF_GETNEWMBUF))
		leading_len = 0;
	else
		leading_len = mBuf_leadingSpace(m);

	//Does leading free space size satisfies user's request?
	if (leading_len >= plen){
		m->m_data -= plen;
		m->m_len += plen;
		if(m->m_pkthdr)
			m->m_pkthdr->ph_len +=plen;
		assert(pMbuf==m);
		goto out;
	}
	if(leading_len){//Bug!! FIXME: 2003.4.1 (cfliu) if we remove this if(leading_len).... on QA board, 
				// then m->m_data would be set as 0x1!!! strange, strange, strange...
		//this mbuf is not large enough to hold all requesting 'plen' bytes...
		m->m_data -= leading_len;	//Move data pointer of 'm' to the first byte
		m->m_len += leading_len;	//Use up all available space in this cluster
		if(m->m_pkthdr)
			m->m_pkthdr->ph_len += leading_len;
		plen -= leading_len;
	}
	count = leading_len;


	//now allocate new clusters and mbufs
	Nbuf = ((plen - 1) / m_clusterSize) + 1;
	start = (Nbuf*m_clusterSize)-plen;

	CLEARBITS(how, MBUF_ONLY);

	//Caution: pMbuf is reused here..
	pMbuf = _m_get(how, Nbuf);
	if (!pMbuf)
		goto out;

	assert(pMbuf->m_data == pMbuf->m_extbuf);

	//find the last new mbuf and set pkthdr fields for all new mbufs if required
	for (mm = pMbuf; mm; mm = mm->m_next){
		//for all mbufs, use up all cluster space.
		assert(mm->m_len==0);
		if(mm==pMbuf){
			pMbuf->m_data += start;
			pMbuf->m_len = m->m_extsize - start; 	//Move data pointer of the first new mbuf
			count+= pMbuf->m_len;
		}else{
			mm->m_len = mm->m_extsize;//each mbuf except the first one is full-sized
			count+=mm->m_extsize;
		}
		if(m->m_pkthdr){	//if old mbuf chain has a pkthdr, all new mbufs aware it.
			mm->m_pkthdr = m->m_pkthdr;
			m->m_pkthdr->ph_len+=mm->m_len;
		}
		if(mm->m_next==NULL){
			mm->m_next = m;			//chain last new mbuf with first old mbuf
			if(m->m_pkthdr){
				SETBITS(pMbuf->m_flags, MBUF_PKTHDR);
				m->m_pkthdr->ph_mbuf = pMbuf;		//pkthdr now points to first new mbuf
				CLEARBITS(m->m_flags, MBUF_PKTHDR);
				assert(count==plen+leading_len);
				//m->m_pkthdr->ph_len += plen;	//adjust packet length
			}
			break;
		}
	}
out:
	rtlglue_mbufMutexUnlock();
	return pMbuf;
}


/*
 * Arrange to append space of size plen to mbuf m
 * See document in mbuf.h
 */
struct rtl_mBuf *mBuf_padding(struct rtl_mBuf *m, uint32 plen, int32 how){
	struct rtl_mBuf *pLastMbuf=NULL, *pLastMbufNext, *pNewMbuf, *mm;
	int32     trailing_len, Nbuf, end;
	int32	count=0;


	assert(m);
	assert(ISSET(m->m_flags, MBUF_USED));

	rtlglue_mbufMutexLock();
	// find the last mbuf in the list
	for(pLastMbuf=m, pLastMbufNext=m->m_next; pLastMbufNext; pLastMbuf=pLastMbuf->m_next, pLastMbufNext=pLastMbufNext->m_next); //No problem for ;

	trailing_len = mBuf_trailingSpace(pLastMbuf);

	//Does the last mbuf satisfies request?
	if (trailing_len >= plen){
		//memset((void *)(pLastMbuf->m_data+pLastMbuf->m_len), 0, plen); // clear padding bytes
		pLastMbuf->m_len += plen;
		if(m->m_pkthdr)
			m->m_pkthdr->ph_len +=plen;
		goto out;
	}

	//this mbuf is not large enough to hold all requested 'plen' bytes
	//memset((void *)(pLastMbuf->m_data+pLastMbuf->m_len), 0, trailing_len); // clear padding bytes
	pLastMbuf->m_len += trailing_len;	//Use up all available trailing space in this cluster
	if(m->m_pkthdr)
		m->m_pkthdr->ph_len += plen;
	count = trailing_len;

	plen -= trailing_len;

	//now allocate new clusters and mbufs
	Nbuf = ((plen - 1) / m_clusterSize) + 1;
	end = (Nbuf*m_clusterSize)-plen;

	CLEARBITS(how, MBUF_ONLY);
	pNewMbuf = _m_get(how, Nbuf);
	if (!pNewMbuf){
		rtlglue_mbufMutexUnlock();
		return NULL;
	}

	assert(pNewMbuf->m_data == pNewMbuf->m_extbuf);
	// append the newly allocated mbuf to the original list
	pLastMbuf->m_next = pNewMbuf;

	//find the last new mbuf and set pkthdr fields for all new mbufs if required
	for (mm = pNewMbuf; mm; mm = mm->m_next)
	{
		//for all mbufs, use up all cluster space.
		assert(mm->m_len==0);

		if(m->m_pkthdr){	//if old mbuf chain has a pkthdr, all new mbufs aware it.
			mm->m_pkthdr = m->m_pkthdr;
		}
		if(mm->m_next != NULL) {
			mm->m_len = mm->m_extsize;//each mbuf except the first one is full-sized
			memset((void *)mm->m_data, 0, mm->m_len); // clear padding bytes
			count+=mm->m_len;
		} else {/* last mbuf */
			mm->m_len = mm->m_extsize-end;
			memset((void *)mm->m_data, 0, mm->m_len); // clear padding bytes
			count+=mm->m_len;
			assert(count==plen);
			pLastMbuf = mm;
			break;
		}
	}
out:
	rtlglue_mbufMutexUnlock();
	return pLastMbuf;
}

/* Free a single mbuf, leave associated cluster intact. See document in mbuf.h*/
int32 mBuf_freeOneMbufPkthdr(struct rtl_mBuf *m, void **buffer, uint32 *id, uint16 *size){
	uint32 ret;
	assert(m);
	rtlglue_mbufMutexLock();
	if(buffer)
		*buffer =(void *)m->m_extbuf;
	if(id)
		*id =m->m_extClusterId;
	if(size)		
		*size = m->m_extsize;

	ret=_iFreeMbufChain(m, MBUF_KEEPCLUSTER | MBUF_FREEONE);
	rtlglue_mbufMutexUnlock();
	return ret;
}

/* Attach a cluster to an mbuf */
int32 mBuf_attachCluster(struct rtl_mBuf *m, void *buffer, uint32 id, uint32 size, uint16 datalen, uint16 align){
	int32 retval = FAILED;
	struct rtl_pktHdr *ph;
	ph=m->m_pkthdr;
	rtlglue_mbufMutexLock();

	if(ISCLEARED(m->m_flags, MBUF_USED|MBUF_PKTHDR))
		goto out;
	SETBITS(m->m_flags, MBUF_EXT);
	
	m->m_extbuf=buffer;
	m->m_extClusterId = id;
	m->m_extsize= size;
	
	if(datalen){
		if(datalen+align>size)
			goto out;
		m->m_len=datalen;
		m->m_data = ((uint8 *)buffer)+align;
	}else{
		m->m_len=0;
		m->m_data = ((uint8 *)buffer);
	}
	
	retval = SUCCESS;
out:
	rtlglue_mbufMutexUnlock();
	return retval;
}


/* Free a single mbuf and associated external storage. Place the successor, if any, in n.  See document in mbuf.h*/

struct rtl_mBuf *mBuf_freeOne(struct rtl_mBuf *m)
{

	struct rtl_mBuf *n = NULL;
	rtlglue_mbufMutexLock();
	assert(m);
	assert(ISSET(m->m_flags, MBUF_USED));
	if (m->m_next)
		n = m->m_next;

	 _iFreeMbufChain(m, MBUF_FREEONE);
	rtlglue_mbufMutexUnlock();
	return n;
}


/* Rewritten.   mBuf_freeMbufChain: Free the whole mbuf chain. Notice user may pass only a part of the mbuf chain to us */
uint32 mBuf_freeMbufChain(register struct rtl_mBuf * m){
	int32     i = 0;

	assert(m);
	assert(ISSET(m->m_flags, MBUF_USED));

	rtlglue_mbufMutexLock();
	i = _iFreeMbufChain(m, MBUF_FREEALL);
	rtlglue_mbufMutexUnlock();
	if (i > 0)
		return i;
	else
		return 0;

}


///////////////////////////////////////////////////////

/*
 * Mbuffer utility routines.
 */

/*
 * Copy data from a buffer back into the indicated mbuf chain,
 * starting "off" bytes in mbuf chain from the beginning, extending the mbuf
 * chain if necessary.
 */
int32 mBuf_copyToMbuf(struct rtl_mBuf * m0,  uint32 offset, register uint32 len,  int8 * cp)
{
	 int32 mlen;
	 struct rtl_mBuf *m = m0, *n;
	 uint32 off = offset;
	int32     totlen = 0;


	assert(m);
	rtlglue_mbufMutexLock();
	if(mBuf_clusterIsWritable(m)==0){	//cluster is allocated by OS or We are not the owner or the cluster is cloned by other mbuf
		rtlglue_mbufMutexUnlock();
		return -1;
	}

	assert(ISSET(m->m_flags, MBUF_USED));
	mlen = m->m_len;
	while (off > mlen){/*Find from which mbuf we start copy */
		off -= mlen;
		totlen += mlen;
		if (m->m_next == 0){
			if(totlen < offset) //We have reached the end f mbuf chain but  we don't have that much data in mbuf chain!!!
			{
				rtlglue_mbufMutexUnlock();
				return -1;
			}
		}
		m = m->m_next;
	}
 	while (len > 0){
 		int32 maxLen;
		assert(ISSET(m->m_flags, MBUF_EXT));
		if (m->m_len)
			mlen= min(mBuf_trailingSpace(m)+m->m_len - off, len);
		else  //in this case, off= m_len=0
			mlen = min((uint32)m->m_extsize,(uint32) len);

		//Enforce max mbuf chain size limit.
		maxLen = totlen + m->m_len + mlen;
		if(maxLen > 65535)
			mlen= 65535-totlen-m->m_len;

		rtl8651_memcpy( (void *) (off + MBUF2DATAPTR(m, int8 *)),(void *) cp,(uint32) mlen);
		cp += mlen;
		len -= mlen;
		mlen += off;
		off = 0;
		totlen += mlen;
		if(m->m_len < mlen+off)
			m->m_len = mlen+off;

		if (len == 0 || maxLen>65535)
			break;
		if (m->m_next == 0){
			n = _m_get(MBUF_DONTWAIT,  1);
			if (n == 0)
				break;
			n->m_len = min(m_clusterSize, (int32)len);
			m->m_next = n;
		}
		m = m->m_next;
	}

  	if (((m = m0)->m_flags & MBUF_PKTHDR)&& (m->m_pkthdr->ph_len < totlen))
		m->m_pkthdr->ph_len = totlen;
	rtlglue_mbufMutexUnlock();
	return totlen;
}


//Jump table used by Boyer-Moore pattern search algorithm
void mBuf_getBMjmpTable(uint8 *pat,  uint16 *jump_tbl,uint16 patLen, uint8 caseSensitive) {
	uint32 i;
	rtlglue_mbufMutexLock();
	for(i=0; i<256; i++)
		jump_tbl[i]= patLen;

	if (caseSensitive == FALSE)
	{
		for(i = 0; i < patLen - 1; i++)
		{
			jump_tbl[(uint16) tolower(pat[i])] = patLen -i -1;
			jump_tbl[(uint16) toupper(pat[i])] = patLen -i -1;
		}
	}
	else
	{
		for(i=0; i< patLen-1; i++)
			jump_tbl[(uint16) pat[i]]= patLen -i -1;
	}
	rtlglue_mbufMutexUnlock();
}

//Find in the first 'len' bytes of mbuf chain 'm' (mat cross cluster), find first occurance of
//pattern 'delimiter', which is 'delimitLen' bytes long.
//If pattern matched, return the position found(start from 0), otherwise return -1.
//Assumption: Assume there are 'len' bytes on chain. Don't check it.
//len: Lenght of 'mem'
//delimiter: pattern to be matched
//delimitLen: length of 'pat' and 'jmp_tbl'
//jmp_tbl: jump table buffer, should be 256 entries long
/* caseSensitive: match the pattern case-sensitively or not */
int32 mBuf_BMpatternMatch(struct rtl_mBuf *m, uint32 len, uint8 *delimiter, uint32 delimitLen, uint16 *jmp_tbl, uint8 caseSensitive) {
	int32 retval= -1;
	//Boyer-Moore string searching algorithm.

	assert(m && len && delimiter && delimitLen && jmp_tbl);
	rtlglue_mbufMutexLock();
	if(m->m_data + len <= m->m_extbuf + m->m_extsize){
		//Fast path, all 'len' bytes within a single cluster.
		int32 i,j,k;
		uint8 *mem= (uint8 *)m->m_data;

		if (caseSensitive == FALSE)
		{
			for(i = delimitLen -1; i < len; ){  //compute downward
				for(j = delimitLen - 1, k = i; j >= 0 && (tolower(mem[k]) == delimiter[j]); k--, j--); //no problem for ;
				if (j < 0) {	//delimiter exhausted?
					retval = k+1;	//yes, pattern found at (k+2)-th byte
					goto out;
				}else
					i+=jmp_tbl[(uint16)mem[i]];	//no, update using jmp_tbl
			}
		}
		else 
		{	/* case-sensitively match: original BM algo */
			for(i=delimitLen -1; i < len; ){  //compute downward
				for(j=delimitLen-1, k=i; j>=0 && (mem[k]==delimiter[j]); k--, j--); //no problem for ;
				if (j<0){	//delimiter exhausted?
					retval = k+1;	//yes, pattern found at (k+2)-th byte
					goto out;
				}else
					i+=jmp_tbl[(uint16)mem[i]];	//no, update using jmp_tbl
			}
		}
	}else{

		//Slow path...may cross cluster
		struct rtl_mBuf *p;
		int32 i,j,k;
		uint32 offsetTbl[16], lenTbl[16],mbufs, num=0;

		//Remember last byte data offset of each cluster.
		for(mbufs=0, p=m; p; p=p->m_next, mbufs++){
			offsetTbl[mbufs]=num+p->m_len;
			lenTbl[mbufs]=p->m_len;
			num +=p->m_len;
		}

		if (caseSensitive == FALSE)
		{
			for(i=delimitLen -1; i < len; ){  //compute downward
				for(j=delimitLen-1, k=i; j>=0 && tolower(_iGetClusterDataByte(m, k,offsetTbl,lenTbl,mbufs)) ==delimiter[j]; k--, j--); //no problem for ;
				if (j<0){	//delimiter exhausted?
					retval = k+1;	//yes, pattern found at (k+1)-th byte
					goto out;
				}else
					i+=jmp_tbl[(uint16)_iGetClusterDataByte(m, i, offsetTbl,lenTbl,mbufs)];	//no, update using jmp_tbl
			}
		}
		else
		{	/* case-sensitively match: original BM algo */
			for(i=delimitLen -1; i < len; ){  //compute downward
				for(j=delimitLen-1, k=i; j>=0 && _iGetClusterDataByte(m, k,offsetTbl,lenTbl,mbufs) ==delimiter[j]; k--, j--); //no problem for ;
				if (j<0){	//delimiter exhausted?
					retval = k+1;	//yes, pattern found at (k+1)-th byte
					goto out;
				}else
					i+=jmp_tbl[(uint16)_iGetClusterDataByte(m, i, offsetTbl,lenTbl,mbufs)];	//no, update using jmp_tbl
			}
		}
	}
out:
	rtlglue_mbufMutexUnlock();
	return retval;
}


/* Not modified...Minor revision only.
 * Copy data from an mbuf chain starting "off" (Offset starts from 0)bytes from the beginning,
 * continuing for "len" bytes, into the indicated buffer.
 */
int32 mBuf_copyToUserBuffer(struct rtl_mBuf * m, uint32 off, uint32 len, int8 * cp)
{
	register uint32 count;
	register uint32 copied = 0;


	rtlglue_mbufMutexLock();
	while (off > 0)
	{
		assert(m != NULL);
		assert(ISSET(m->m_flags, MBUF_USED));
		if (off < m->m_len)
			break;
		off -= m->m_len;
		m = m->m_next;
	}
	while (len > 0)
	{
		assert(m != NULL);
		assert(ISSET(m->m_flags, MBUF_USED));
		count = min(m->m_len - off, len);
		rtl8651_memcpy( cp,MBUF2DATAPTR(m, int8 *) + off, count);
		len -= count;
		copied += count;
		cp += count;
		off = 0;
		m = m->m_next;
	}
	rtlglue_mbufMutexUnlock();
	return copied;
}

#ifndef MBUF_COMPACT
/* Rewritten
 * Make a clone of an mbuf chain starting "iOffset"(begin from 0) bytes from the beginning,
 * continuing for "iLength" bytes.  The wait parameter is a choice of MBUF_WAITOK/MBUF_DONTWAIT from caller.
 * If len is M_COPYALL, clone to end of mbuf chain. (In this case, you can use mBuf_clonePacket() instead)
 *
 * Note that the copy is read-only, because clusters are not copied,
 * only their reference counts are incremented.
 */
struct rtl_mBuf *mBuf_cloneMbufChain(struct rtl_mBuf *pMbufChain, int32 iOffset, int32 iLength,int32 flag){
	struct rtl_mBuf *m;
	rtlglue_mbufMutexLock();
	m =  _pCopyMbufChain(pMbufChain,  iOffset,  iLength, flag | MBUF_ONLY);
	rtlglue_mbufMutexUnlock();
	return m;
}

/* Rewritten
 * Duplicate an mbuf chain starting "iOffset"(begin from 0) bytes from the beginning,
 * continuing for "iLength" bytes.  The wait parameter is a choice of MBUF_WAITOK/MBUF_DONTWAIT from caller.
 * If len is M_COPYALL, duplicate to end of mbuf chain. (In this case, you can use mBuf_dupPacket() instead)
 *
 * Returned mubf chain is writable.
 */

struct rtl_mBuf *mBuf_dupMbufChain(struct rtl_mBuf *pMbufChain, int32 iOffset, int32 iLength,  int32 flag){
	struct rtl_mBuf *m;
	rtlglue_mbufMutexLock();
	m =  _pCopyMbufChain( pMbufChain,  iOffset,  iLength, flag);
	rtlglue_mbufMutexUnlock();
	return m;
}

/* Rewritten
 * "CLONE"  ENTIRE packet, including header (which must be present).
 * An optimization of the common case `mBuf_cloneMbufChain(m, 0, M_COPYALL, how)'.
 * Note that the copy is read-only, because clusters are not copied,
 * only their reference counts are incremented.
 */
struct rtl_mBuf *mBuf_clonePacket(struct rtl_mBuf *pMbuf, int32 iHow){
	struct rtl_mBuf *m;
	rtlglue_mbufMutexLock();
	m =  _pCopyPacket(pMbuf, iHow | MBUF_ONLY);
	rtlglue_mbufMutexUnlock();
	return m;
}
#endif

/* Rewritten
 * "DUPLICATE" entire packet into a completely new chain, including
 * copying any mbufs, pkthdrs, and clusters.  An optimization of the common case
 * mBuf_dupMbufChain(). Use this instead of mBuf_clonePacket() when
 * you need a writable copy of an mbuf chain.
 */
struct rtl_mBuf *mBuf_dupPacket(struct rtl_mBuf *pMbuf, int32 iHow){
	struct rtl_mBuf *m;
	rtlglue_mbufMutexLock();
	m =  _pCopyPacket(pMbuf, iHow);
	rtlglue_mbufMutexUnlock();
	return m;
}

#ifndef MBUF_COMPACT
/*
 * Concatenate mbuf chain pTailMbuf to pHeadMbuf. If pTailMbuf has a pkthdr, it would be freed.
 * Both chains must be of the same type (e.g. MT_DATA).
 */

struct rtl_mBuf *mBuf_cat(struct rtl_mBuf *pHeadMbuf, struct rtl_mBuf *pTailMbuf)
{
	struct rtl_mBuf *pHeadLastMbuf = pHeadMbuf;
	struct rtl_mBuf *pMbuf;
	uint32    iTailLength = 0, pktlen=0;

	assert(pTailMbuf && pHeadMbuf);
	assert(ISSET(pHeadMbuf->m_flags, MBUF_USED));
	assert(ISSET(pTailMbuf->m_flags, MBUF_USED));

	rtlglue_mbufMutexLock();

	while (pHeadLastMbuf->m_next){
		pktlen+= pHeadLastMbuf->m_len;
		pHeadLastMbuf = pHeadLastMbuf->m_next;
	}
	pktlen+=pHeadLastMbuf->m_len;
	
	//Exceed max mbuf chain size.
	if((_m_getPktlen(pTailMbuf)+pktlen)>=65536){
		pHeadMbuf=NULL;
		goto out;
	}

	pHeadLastMbuf->m_next = pTailMbuf;

	//free pTailMbuf's pkthdr if there is one
	if ISSET(pTailMbuf->m_flags, MBUF_PKTHDR){
		assert(pTailMbuf->m_pkthdr && ISSET(pTailMbuf->m_pkthdr->ph_flags, PKTHDR_USED));
		iTailLength = pTailMbuf->m_pkthdr->ph_len;
		_vFreePkthdr(pTailMbuf->m_pkthdr);
		pTailMbuf->m_pkthdr=0;
		CLEARBITS(pTailMbuf->m_flags, MBUF_PKTHDR);
	}

	if ISSET(pHeadMbuf->m_flags, MBUF_PKTHDR){
		assert(pHeadMbuf->m_pkthdr
			   && ISSET(pHeadMbuf->m_pkthdr->ph_flags, PKTHDR_USED));
		if (iTailLength > 0)
			pHeadMbuf->m_pkthdr->ph_len += iTailLength;
		pMbuf = pTailMbuf;
		while (pMbuf)	{
			CLEARBITS(pMbuf->m_flags, MBUF_PKTHDR);
			pMbuf->m_pkthdr = pHeadMbuf->m_pkthdr;	//all trailing mbuf's m_pkthdr points to pHeadMbuf's pkthdr
			pMbuf = pMbuf->m_next;
		}
	}

out:
	rtlglue_mbufMutexUnlock();
	return pHeadMbuf;
}
#endif
struct rtl_mBuf *mBuf_adjHead(struct rtl_mBuf *mp, uint32 req_len){
	 uint32 len = req_len;
	 struct rtl_mBuf *m=mp;
	 uint32 adjusted=0, toomuch=0;


	assert(mp);
	assert(ISSET(m->m_flags, MBUF_USED));

	if(ISCLEARED(m->m_flags, MBUF_EXT))
		return NULL;

	rtlglue_mbufMutexLock();

	while (m != NULL && len > 0)	{
		assert(m && (m->m_flags & MBUF_EXT));
		if (m->m_len <= len){		//The whole mbuf should be adjusted or trimmed
			len -= m->m_len;
			adjusted+= m->m_len;
			m->m_len = 0;		   //clusters are not freed even it contains no data
			m->m_data = m->m_extbuf; //reset the m_data pointer
			m = m->m_next;
		}else{
			m->m_len -= len;
			adjusted+= len;

			if(m->m_len==0)
			{
				m->m_data = m->m_extbuf;
			}
			else
				m->m_data += len;
			len = 0;
			assert(m->m_len>0 );
			break;
		}
	}
	if(len>0)
		toomuch=1;
	if (mp->m_pkthdr)
		mp->m_pkthdr->ph_len -= adjusted;
	rtlglue_mbufMutexUnlock();
	if(toomuch)
		return mp;
	else
		return m;
}

struct rtl_mBuf *mBuf_adjTail(struct rtl_mBuf *mp, uint32 req_len){
	 uint32 len = req_len;
	 struct rtl_mBuf *m=mp, *n=NULL;
	 uint32 pktlen, drop;
	 uint32 adjusted=0;


	assert(mp);
	assert(req_len);
	assert(ISSET(m->m_flags, MBUF_USED));

	if(ISCLEARED(m->m_flags, MBUF_EXT)){
		return NULL;
	}

	rtlglue_mbufMutexLock();
	//calculate total pktlen
	pktlen = _m_getPktlen(mp);

	if(len>pktlen)
		len = pktlen;
	drop = pktlen - len;	//we will start dropping data from the #drop -th byte


	// 'pktlen' is the length of packet.
	// 'len' is the number of daya bytes to be dropped
	// 'drop' downcounts how far is the first byte to drop
	// 'm' is the first mbuf in packet,

	/*
	 * Find the mbuf with first byte of data to be trimmed, adjust its length,
	 * and toss data from remaining mbufs on chain.
	 */

	m = mp;		//start from first mbuf
	n = NULL;
	adjusted=0;
	for (; m; n=m, m = m->m_next)
	{
		assert(m && (m->m_flags & MBUF_EXT));
		if (m->m_len >= drop)			//the first mbuf to trim.
		{
			adjusted+= (m->m_len - drop);
			if (drop==0){
				assert(m->m_extbuf);
				m->m_data = m->m_extbuf;
			}
			m->m_len = drop;		//m->m_data + m->m_len is the last used byte
			break;
		}
		drop -= m->m_len;			//The first byte to trim is 'drop+1' byte away from here.
	}

	// 'm' is now the first mbuf which contains first data byte.
	//Now we clear all mbufs after 'm'  starting from 'n'

	n = m->m_next;
	if(!n && (m->m_len==0)){	//No more mbufs after m and m is empty. Return first mbuf to user
		m = mp;
		goto done;
	}

	while (n){
		adjusted+= n->m_len;
		n->m_len = 0;
		n->m_data = n->m_extbuf;
		n = n->m_next;
	}

done:
	if (mp->m_pkthdr)
		mp->m_pkthdr->ph_len -= adjusted;

	rtlglue_mbufMutexUnlock();
	return m;
}



#ifndef MBUF_COMPACT

/*
 * Partition an mbuf chain in two pieces, returning the tail --
 * all but the first len0 bytes.  In case of failure, it returns NULL and
 * attempts to restore the chain to its original state.
 *  The chain is NOT copied, but cloned.
 */
static struct rtl_mBuf *_m_split(register struct rtl_mBuf *m0, uint32 len0, int32 wait){

	register struct rtl_mBuf *m = NULL, *n = NULL;
	uint32  len = len0, remain = 0;


	assert(len);
	assert(m0);
	assert(ISSET(m0->m_flags, MBUF_USED));

	rtlglue_mbufMutexLock();
	for (m = m0; m && len > m->m_len; m = m->m_next){  //find from which mbuf to split
		assert(ISSET(m->m_flags, MBUF_EXT));
		len -= m->m_len;
	}
	if (m == 0)	   //len0 is greater than the number of all data we have, can't split....
		goto out;

	assert(m->m_len >= len);
	remain = m->m_len - len;		   // "remain" bytes in this mbuf are to be copied to new mbuf


	if (ISSET(m0->m_flags, MBUF_PKTHDR))	{
		if (remain == 0)	{							   //we split exactly at the cluster boundary
			if (!m->m_next)
				goto out;//can't split at the last byte
			n = m->m_next;
			n->m_pkthdr = NULL;
		}else if ((n = _m_get(wait | MBUF_ONLY,  1)) == NULL)	//the split point is not at the boundary of cluster
			goto out;

		if (!_m_gethdr(n, wait)){
			_iFreeMbufChain(n, MBUF_FREEALL);
			n=NULL;
			goto out;
		}


		//Copy pkthdr content. skip  first 4-bytes to avoid overwriting ph_mbuf pointer */
		rtl8651_memcpy((void *) (((int32 *) n->m_pkthdr) + 1), (void *) (((int32 *) m0->m_pkthdr) + 1),sizeof(struct rtl_pktHdr) - 4);

		n->m_pkthdr->ph_len = m0->m_pkthdr->ph_len - len0;	//adjust mbuf chain length.
		n->m_pkthdr->ph_rxdesc = PH_RXDESC_INDRV;
		n->m_pkthdr->ph_rxPkthdrDescIdx = PH_RXPKTHDRDESC_INDRV;

		m0->m_pkthdr->ph_len = len0;
		assert(ISSET(m->m_flags, MBUF_EXT));
		goto extpacket;

	}else if (remain == 0){	 
		n = m->m_next;
		m->m_next = 0;				   //terminate the m0 mbuf chain
		goto out;
	}else{	   //no pkthdr, get a new mbuf to store trailing data
		m->m_len -= remain;
		if ((n = _m_get(wait | MBUF_ONLY,  1)) == NULL)
			goto out;
	}

  extpacket:
	SETBITS(n->m_flags, MBUF_EXT);
	if (remain != 0)	{
		n->m_extbuf = m->m_extbuf;
		n->m_extsize = m->m_extsize;
		if (_iTotalClusters)
			CLDataAddr2TagAddr(m->m_extbuf)->cltag_refcnt++;
		else{
			n->m_extClusterId= m->m_extClusterId;
			rtlglue_mbufClusterRefcnt(m->m_extbuf, m->m_extClusterId, NULL, 1);
		}
		n->m_data = m->m_data + len;
		n->m_len = remain;
		n->m_next = m->m_next;
		m->m_len = len;
	}
	m->m_next = 0;
out:
	rtlglue_mbufMutexUnlock();
	return n;
}


struct rtl_mBuf *mBuf_split(register struct rtl_mBuf *m0, uint32 len0, int32 wait){
	struct rtl_mBuf *m;
	rtlglue_mbufMutexLock();
	m =  _m_split( m0, len0, wait);
	rtlglue_mbufMutexUnlock();
	return m;
}

//although using mBuf_split has some overhead and side effect (one extra mbuf and pkthdr may be allocated
//and freed immediately), and is less efficient, however, we don't need extra code here.
extern struct rtl_mBuf *mBuf_trimHead(struct rtl_mBuf *mp, uint32 req_len){
	struct rtl_mBuf *n=NULL;

	assert(mp);
	assert(req_len);
	assert(ISSET(mp->m_flags, MBUF_USED));
	rtlglue_mbufMutexLock();
	if(_m_getPktlen(mp)<= req_len){
		_iFreeMbufChain(mp,MBUF_FREEALL);
	}else{
		n = _m_split(mp, req_len, MBUF_DONTWAIT);
		if(_iFreeMbufChain(mp,MBUF_FREEALL)<1)
			;//DEBUGMSG(( "mBuf_trimHead failed"));
	}
	rtlglue_mbufMutexUnlock();
	return n;

}

//although using mBuf_split has some overhead and side effect (one extra mbuf and pkthdr may be allocated
//and freed immediately), and is less efficient, however, we don't need extra code here.
struct rtl_mBuf *mBuf_trimTail(struct rtl_mBuf *mp, uint32 req_len){
	struct rtl_mBuf *n;
	int32 len;

	assert(mp);
	assert(req_len);
	assert(ISSET(mp->m_flags, MBUF_USED));

	rtlglue_mbufMutexLock();
	len = _m_getPktlen(mp);

	if(len==0){
		mp=NULL;
		goto out;
	}else
		len -= req_len;

	if(len <= 0){ //trim too much
		_iFreeMbufChain(mp,MBUF_FREEALL);
		mp=NULL;
		goto out;

	}

	n = _m_split(mp, len, MBUF_DONTWAIT);
	assert(n);

	if(_iFreeMbufChain(n, MBUF_FREEALL)<1){
		;//DEBUGMSG(( "mBuf_trimTail failed2"));
		mp=NULL;
	}
	//find the last mbuf which contains last data byte.
out:
	rtlglue_mbufMutexUnlock();
	return mp;
}



/*
 *	cfliu: This function is important in traditional BSD networking stack because
 *		 original mbuf can't do dtom() if data is saved in cluster. This is not the
 *		case here since we offer a back pointer from cluster to mbuf via cltag_mbuf field.
 *		However, this function is still useful if we want to collect continous databytes from later clusters
 *		to the specified mbuf's cluster.
 *
 * 		We won't pullup more data bytes than a cluster can hold (ie. iLen <=m_clusterSize)
 */
int32 mBuf_pullup(register struct rtl_mBuf * pMbuf, int32 iLen)
{
	int32     iBytesLeft = 0, iRestSpace = 0, iCount = 0;
	uint8     *pChar = NULL;

	struct rtl_mBuf *pThisMbuf = NULL;

	assert(pMbuf && (iLen > 0));
	assert(ISSET(pMbuf->m_flags, MBUF_USED));

	rtlglue_mbufMutexLock();

	if (ISCLEARED(pMbuf->m_flags, MBUF_EXT) || (!pMbuf->m_data))	//may be just a mbuf without clusters
		goto quit;

	if ((iBytesLeft = iLen) <= pMbuf->m_len)	// no need to pull up.
		goto quit;

	if ((pMbuf->m_pkthdr) && (pMbuf->m_pkthdr->ph_len < iBytesLeft))	//not enough data to be pulled up
		goto quit;

	//mbuf chain has enough data for us to pull up.

	iRestSpace = mBuf_trailingSpace(pMbuf);

	iBytesLeft -= pMbuf->m_len;

	if (iRestSpace >= iBytesLeft)
	{								   // Cluster is large enough
		pThisMbuf = pMbuf->m_next;	   //first mbuf to check
		assert(pThisMbuf != NULL);
		pChar = pMbuf->m_data + pMbuf->m_len;	//where first byte of data should be copied to
		while (iBytesLeft > 0)
		{
			while (pThisMbuf->m_len == 0)
				pThisMbuf = pThisMbuf->m_next;
			assert(pThisMbuf != NULL);
			iCount = min(pThisMbuf->m_len, iBytesLeft);
			if (!rtl8651_memcpy( pChar, pThisMbuf->m_data,iCount))
				goto quit;

			pThisMbuf->m_len -= iCount;	//length shrinks
			pThisMbuf->m_data += iCount;	//starting address goes down

			pMbuf->m_len += iCount;
			pChar += iCount;

			iBytesLeft -= iCount;
		}
	}
	else {
	////
	/* hiwu : pullup based on leading space*/
		uint8 *cp1, *cp2;
		int32 ii = 0;
		iRestSpace = mBuf_leadingSpace(pMbuf);
		if (iRestSpace >= iBytesLeft){ // Cluster is large enough
		pThisMbuf = pMbuf->m_next;	   //first mbuf to check
		assert(pThisMbuf != NULL);

		cp1 = pMbuf->m_data;
		pMbuf->m_data -= iBytesLeft;
		cp2 = pMbuf->m_data;
		for(ii = 0 ; ii < pMbuf->m_len ; ii++){
			*cp2 = *cp1;
			 cp2++;
			 cp1++;
		}

		pChar = pMbuf->m_data + pMbuf->m_len;	//where first byte of data should be copied to


		while (iBytesLeft > 0)
		{
			while (pThisMbuf->m_len == 0)
				pThisMbuf = pThisMbuf->m_next;
			assert(pThisMbuf != NULL);
			iCount = min(pThisMbuf->m_len, iBytesLeft);
			if (!rtl8651_memcpy( pChar, pThisMbuf->m_data,iCount))
				goto quit;

			pThisMbuf->m_len -= iCount;	//length shrinks
			pThisMbuf->m_data += iCount;	//starting address goes down

			pMbuf->m_len += iCount;
			pChar += iCount;

			iBytesLeft -= iCount;
		}
		}

	}

  quit:
	rtlglue_mbufMutexUnlock();
	return pMbuf->m_len;
}
#endif


//////// mbuf APIs dedicated for drivers //////

//user want allocate pkthdrs without mbufs.
//mBuf_driverGetPkthdr returns success only when all requested pkthdr could be get.
//User should be responsible for managing the pointers linking pkthdr and mbufs. Otherwise,
//orphan pkthdrs may never be freed.
//Assumption: Caller should be responsible that pHeadPkthdr and pTailPkthdr are always non-NULL!!
uint32 mBuf_driverGetPkthdr(uint32 Npkthdr, struct rtl_pktHdr **ppFirstPkthdr, struct rtl_pktHdr **ppTailPkthdr){
	//see if we can get all mbufs
	 int32 iIndex;
	 struct rtl_pktHdr *pThisPkthdr=NULL,*pPrevPkthdr=NULL;
	rtlglue_mbufMutexLock();

	Npkthdr = min(Npkthdr, _PHStatus.iFreebufs);
	if (Npkthdr==0){
		rtlglue_mbufMutexUnlock();
		return 0;
	}

	/*Yes, we have enough free pkthdrs... */
	for (iIndex = 0; iIndex < Npkthdr; iIndex++){	// Initialize all Npkthdr pkthdrs
		assert(_PHStatus.pvFreelist);
		pThisPkthdr = _PHStatus.pvFreelist;
		_PHStatus.iFreebufs--;

		//Assumption: Pointers are 32-bits, 
		// let pvFreelist pointer to next free ptr.
		_PHStatus.pvFreelist = (uint32 *) (*((uint32 *) pThisPkthdr));

		//assert(pThisPkthdr);
		//memset((void *) pThisPkthdr, (uint8) 0, sizeof(struct rtl_pktHdr));	//Do we really need to Initialize pkthdr to 0 at driver???
		pThisPkthdr->ph_flags= PKTHDR_USED|PKTHDR_DRIVERHOLD;
		pThisPkthdr->ph_nextHdr=NULL;
		pThisPkthdr->ph_len=0;
		pThisPkthdr->ph_rxdesc = PH_RXDESC_INDRV;
		pThisPkthdr->ph_rxPkthdrDescIdx = PH_RXPKTHDRDESC_INDRV;	/* Invalid RX ring : means this packet header is NOT from RX ring */

		if (iIndex != 0)
			pPrevPkthdr->ph_nextHdr = pThisPkthdr;	/*Chain together */
		else
			*ppFirstPkthdr = pThisPkthdr;	/* Return this pointer to user */
		pPrevPkthdr = pThisPkthdr;
	}
	assert(iIndex==Npkthdr);
	rtlglue_mbufMutexUnlock();
	*ppTailPkthdr = pThisPkthdr;
	return (iIndex);		   //Return number of pkthdr successfully allocated
}



/*Free a pkthdr alone. Callers should be aware that this is NOT the normal way to free a pkthdr with
mbufs attached. Callers should use  mBuf_freeOne()or  mBuf_freeMbufChain() to free pkthdrs attached with mbuf chains.
Caller of this function should remove the links between mbufs and pkthdrs on their own before
mBuf_freePkthdr() is called.*/
uint32 mBuf_driverFreePkthdr(struct rtl_pktHdr *ph, uint32 Npkthdr, struct rtl_pktHdr **ppHeadPkthdr)
{
	register struct rtl_pktHdr *pThisPkthdr = ph, *pNextPkthdr = NULL;
	register uint32     iFreed = 0;
	rtlglue_mbufMutexLock();
	while (iFreed<Npkthdr && pThisPkthdr && ISSET(pThisPkthdr->ph_flags, PKTHDR_USED))
	{
		//assert(ph->ph_flags & PKTHDR_USED);
		CLEARBITS(ph->ph_flags, PKTHDR_USED);
		//Assumption: 1. Pointers are 32-bits,   2.1st field of pBufPoolStat points to next free Buffer as defined in mbuf.h
		*((uint32 *) pThisPkthdr) = (uint32) _PHStatus.pvFreelist;	/* i.e.   p->next = Freelist */
		_PHStatus.pvFreelist = pThisPkthdr;	/*i.e.  Freelist = p */
		_PHStatus.iFreebufs++;
		pNextPkthdr = pThisPkthdr->ph_nextHdr;
		iFreed++;
		pThisPkthdr = pNextPkthdr;
	}
	if(ppHeadPkthdr)
		*ppHeadPkthdr = pNextPkthdr;
	rtlglue_mbufMutexUnlock();
	return iFreed;
}


//user want get 'Nmbuf' pair of mbuf-cluster.
//mbuf and cluster not initialized to zero.
//Assumption: Caller should be responsible that pFirstMbuf and pTailMbuf are always non-NULL!!
uint32 mBuf_driverGet(uint32 Nmbuf,struct rtl_mBuf **ppFirstMbuf, struct rtl_mBuf **ppTailMbuf)
{
	//see if we can get all mbufs
	register uint32 iIndex, extClusterAllocated=0;
	register struct rtl_mBuf *pThisMbuf=NULL,*pPrevMbuf=NULL;
	register struct mcluster_tag *pThisClusterTag=NULL;

	rtlglue_mbufMutexLock();

	iIndex = 0;

	Nmbuf = min(Nmbuf, _MbufStatus.iFreebufs);

	if (Nmbuf == 0)
	{
		goto out;
	}

	if(_iTotalClusters)
	{
		Nmbuf = min(Nmbuf, _ClStatus.iFreebufs);

		if (Nmbuf == 0)
		{
			goto out;
		}
	} else
	{
		for(extClusterAllocated=0; extClusterAllocated < Nmbuf; extClusterAllocated++)
		{
			if(rtlglue_mbufAllocCluster(&_clusters[extClusterAllocated].buf,m_clusterSize,&_clusters[extClusterAllocated].id)==FAILED)
			{
				int32 release;
				//can't allocate all requested clusters. Release all already allocated.
				for(release=0; release < extClusterAllocated; release++)
				{
					rtlglue_mbufFreeCluster(_clusters[extClusterAllocated].buf,m_clusterSize,_clusters[release].id);
				}
				goto out;
			}
		}
	}

	/*Yes, we have enough free mbuf and cluster pair... */
	for (iIndex = 0; iIndex < Nmbuf; iIndex++)
	{	// Initialize all Npkthdr pkthdrs
		//Assumption: Pointers are 32-bits,

		/*Get an mbuf*/
		assert(_MbufStatus.pvFreelist);

		pThisMbuf = _MbufStatus.pvFreelist;
		_MbufStatus.iFreebufs--;
		_MbufStatus.pvFreelist = (uint32 *) (*((uint32 *) pThisMbuf));

		if(_iTotalClusters)
		{
			/*Get a cluster*/
			assert(_ClStatus.pvFreelist);
			pThisClusterTag = _ClStatus.pvFreelist;
			_ClStatus.iFreebufs--;
			_ClStatus.pvFreelist = (uint32 *) (*((uint32 *) pThisClusterTag));
		}
		pThisMbuf->m_next = NULL;
		pThisMbuf->m_pkthdr= 0;		
		pThisMbuf->m_len = 0;
		pThisMbuf->m_flags =  MBUF_EXT | MBUF_USED;

		if(_iTotalClusters)
		{
			pThisMbuf->m_extbuf = pThisMbuf->m_data =
				(uint8 *) CLTagAddr2DataAddr(pThisClusterTag);
			pThisClusterTag->cltag_mbuf = pThisMbuf;	/* points back to pThisMbuf */
			pThisClusterTag->cltag_flags =  CLUSTER_USED;
			pThisClusterTag->cltag_refcnt = 1;
		}else
		{
			pThisMbuf->m_extbuf = pThisMbuf->m_data =  (uint8 *)_clusters[iIndex].buf;	
			pThisMbuf->m_extClusterId = _clusters[iIndex].id;
		}
		pThisMbuf->m_extsize = _Statistics.m_mclbytes;

		if (iIndex != 0)
			pPrevMbuf->m_next = pThisMbuf;	/*Chain together */
		else
			*ppFirstMbuf = pThisMbuf;	/* Return this pointer to user */
		pPrevMbuf = pThisMbuf;
	}

	assert(iIndex==Nmbuf);
	*ppTailMbuf = pThisMbuf;

out:
	rtlglue_mbufMutexUnlock();
	return iIndex;	/* Return success only when all requested buffers allocated */
}


/*Free the whole mbuf chain*/
//user want free 'Nmbuf' pairs of mbuf-clusters started from 'pFirstMbuf',
//Returns the number of pairs of mbuf-cluster freed and if we haven't free the whole chain, return
//Next usable mbuf via 'pHeadMbuf'
uint32 mBuf_driverFreeMbufChain(struct rtl_mBuf *pFirstMbuf)
{
	register struct rtl_mBuf *pThisMbuf = pFirstMbuf, *pNextMbuf = NULL;
	register struct mcluster_tag *pThisClusterTag = NULL;
	struct rtl_pktHdr * pThisPkthdr=NULL;
	register uint32     iFreed = 0;

	rtlglue_mbufMutexLock();
	if(pThisMbuf->m_flags & MBUF_PKTHDR){

		int32 rxDescIdx;

		pThisPkthdr = pThisMbuf->m_pkthdr;
		rxDescIdx = pThisPkthdr->ph_rxdesc;

		if (pThisPkthdr->ph_rxPkthdrDescIdx >= PH_RXPKTHDRDESC_MINIDX)
		{
			if(rtlglue_reclaimRxBD(rxDescIdx, pThisPkthdr,  pThisMbuf))
				goto free;
			iFreed=1;
			goto out;
		}
	}

free:	
	
	while (pThisMbuf && ISSET(pThisMbuf->m_flags, MBUF_USED))
	{
		//Free the cluster */
		if (pThisMbuf->m_flags & MBUF_EXT)
		{
			if(_iTotalClusters){
				pThisClusterTag = CLDataAddr2TagAddr(pThisMbuf->m_data);

				pThisClusterTag->cltag_refcnt--;
				if (pThisClusterTag->cltag_refcnt == 0)
				{					   // The last referee should  free the cluster
					CLEARBITS(pThisClusterTag->cltag_flags, CLUSTER_USED);

					//Assumption: 1. Pointers are 32-bits,   2.1st field of pBufPoolStat points to next free Buffer as defined in mbuf.h
					*((uint32 *) pThisClusterTag) = (uint32) _ClStatus.pvFreelist;	/* i.e.   p->next = Freelist */
					_ClStatus.pvFreelist = pThisClusterTag;	/*i.e.  Freelist = p */
					_ClStatus.iFreebufs++;
				}
				else if (pThisClusterTag->cltag_mbuf == pThisMbuf)
				{
					//The owner is quiting and cluster becomes zombie.
					//TODO: Checks what may happen if zombie cluster happens.....
					// 1. m_dtom() won't work for zombie clusters
					pThisClusterTag->cltag_mbuf = NULL;
				}
				// All other referees just silently quit.
			}else{
				uint32 count;
				if(pThisMbuf->m_extClusterId){
					rtlglue_mbufClusterRefcnt(pThisMbuf->m_extbuf, pThisMbuf->m_extClusterId, &count, 0);
					if(count==1){
						//Free the cluster when reference count =1, refcnt would be decremented inside rtlglue_mbufFreeCluster()
						rtlglue_mbufFreeCluster(pThisMbuf->m_extbuf, m_clusterSize, pThisMbuf->m_extClusterId);
					}else //we are not the last referee, just decrement the reference count.
						rtlglue_mbufClusterRefcnt(pThisMbuf->m_extbuf, pThisMbuf->m_extClusterId, &count, 2);
				}

			}
		}

		if (pThisMbuf->m_pkthdr)
		{
			if (pThisMbuf->m_flags & MBUF_PKTHDR)
			{
				CLEARBITS(pThisMbuf->m_pkthdr->ph_flags, PKTHDR_USED);
				*((uint32 *) pThisMbuf->m_pkthdr) = (uint32) _PHStatus.pvFreelist;	/* i.e.   p->next = Freelist */
				_PHStatus.pvFreelist = pThisMbuf->m_pkthdr;	/*i.e.  Freelist = p */
				_PHStatus.iFreebufs++;

				pThisMbuf->m_pkthdr = NULL;
				CLEARBITS(pThisMbuf->m_flags, MBUF_PKTHDR);
			}
			else					   /* user wanna free all buffers, so don't worry about pkt length... */
				pThisMbuf->m_pkthdr = NULL;
		}

		/*
		   Free the mbuf itself
		 */
		pThisMbuf->m_flags = MBUF_FREE;
		pNextMbuf = pThisMbuf->m_next;
		*((uint32 *) pThisMbuf) = (uint32) _MbufStatus.pvFreelist;	/* i.e.   p->next = Freelist */
		_MbufStatus.pvFreelist = pThisMbuf;	/*i.e.  Freelist = p */

		_MbufStatus.iFreebufs++;
		iFreed++;
		pThisMbuf = pNextMbuf;
	}
out:
	rtlglue_mbufMutexUnlock();
	return iFreed;
}

int32 mBuf_setNICRxRingSize(uint32 size){
	if ( _mbInited==0)
		return FAILED; //mbuf not yet intialized, can't continue.
	if (size>_iTotalPkthdrs)
		return FAILED;//too many descriptors. 
	_maxRxDescidx=size;
	return SUCCESS;
}

/*********************************************************************************
	Section 6:	Internal function implementation
**********************************************************************************/


/*
 * 	Function:	_vWakeup
 *	Input:		piWaitID		waiting channel to wakeup
 *	Returns:	None
 *	Note:
 * 		Wake up the next instance (if any)  which is waiting in the specified wait channel.
 * 		This should be called with mbuf mutex.
 *
 * 		m_{get | gethdr } routines would call is in order to wake another sleep
 * 		instance faster.
 */

static void _vWakeup(uint32 * piWaitID)
{
	//Assume mbuf mutex already accquired.
	assert(piWaitID);

	if (piWaitID){
		(*piWaitID)--;
		rtlglue_mbufTwakeup((uint32)piWaitID);
	}
	return;
}

RTL_STATIC_INLINE  void _vFreePkthdr(struct rtl_pktHdr *ph){
		assert(ph->ph_flags & PKTHDR_USED);
		CLEARBITS(ph->ph_flags, PKTHDR_USED);
		_vReturnFreeBuffer(ph, &_PHStatus);	/* free the pkthdr */
}

//In mbuf chain 'm', get the 'offset'-th data byte.
//'offsetTbl' stores the accumulated offset of each cluster in chain. 'mbufs' is the total number of mbufs in chain
//For example, m0-m3 are chained together, m0-m3 has 100,200,300,400 databyte bytes, so 'mbufs'=4
//and offsetTbl={100,300,600,1000}
RTL_STATIC_INLINE uint8 _iGetClusterDataByte(struct rtl_mBuf *m, uint32 offset, uint32 *offsetTbl, uint32 *lenTbl, uint32 mbufs){
	uint32 i, len=offset;
	struct rtl_mBuf *p;
	for(i=0, p=m; i<mbufs && offset>offsetTbl[i]; len-=lenTbl[i], p=p->m_next, i++);  //no problem for ;
	return (uint8)p->m_data[len];
}

/*
 * 	Function:	_iTryGet
 *	Input:		piAvailableBuffers		How many resource instances still available
 *				iRequestedBuffers	How many resource instances requested
 *				piWaitID		The wait channel
 *				iHow 		If resources aren't available, is it ok to wait?
 *				have_Waited	When returns, tell caller whether we have waited via tsleep
 *	Returns: 		Whether requested resources are avilable
 *	Note:
 * 			Should be called with mbuf mutex
 */

static int32 _iTryGet(register uint32 * piAvailableBuffers, register uint32 iRequestedBuffers,
					register uint32 * piWaitID, register int32 iHow, register uint32 * piHaveWaited)
{
	//Assume mbuf mutex already accquired.
	int32     iWaited = FALSE;

	assert(piAvailableBuffers && (iRequestedBuffers > 0) && (piWaitID)
		   && (iHow) && (piHaveWaited));

	while (*piAvailableBuffers < iRequestedBuffers)	{

		//no enough buffers now and user doesn't want to wait, return fail
		if (ISSET(iHow , MBUF_DONTWAIT)){
			*piHaveWaited = iWaited;
			return FALSE;   // we don't have enough buffer and user doesn't want to wait
		}

		//User can wait. Wait until we are satisfied.
		(*piWaitID)++;
		rtlglue_mbufMutexLock();
		//wait forever. spinlock must be freed, after tsleep, spinlock must be reaccquired.
		rtlglue_mbufTsleep((uint32)piWaitID);
		(*piWaitID)--;
		iWaited = TRUE;
		rtlglue_mbufMutexUnlock();
	}

	*piHaveWaited = iWaited;
	return TRUE;
}



/*
 * 	Function:	_vInitBufferPools
 *	Input:		p		pointer to starting address of pool
 *				size		size of each element in bytes
 *				num		Number of elements in this pool
 *	Returns: 		NULL
 *	Description:
 *		Clear memory content to 0 and chain all elements together. We assume that the first field of each pool
 *		 is a 32-bit pointer to point to next element in the pool.
 */

static void _vInitBufferPools(void *pBufPool, int32 iSize, int32 iNum)
{
	//Assume mbuf mutex already accquired.
	int32     i;
	int8     *pBuf;

	assert(pBufPool);
	assert(iSize && iNum);
	pBuf = (int8 *) pBufPool;

	memset((void *) pBuf, (uint8) 0, iSize * iNum);	//Initialize to 0

	//Assumption: Pointers are 32-bits,
	for (i = 0; i < (iNum - 1); i++, pBuf += iSize)
		*((uint32 *) pBuf) = (uint32) (pBuf + iSize);	//chain neighboring bufers together via next pointer aligned to 1st field
}


/*
 * 	Function:	_pvGetFreeBuffer
 *	Input:
 *				pool		from which pool
 *	Returns: 		p = the first free buffer address
 *	Description:
 *		Get the first free buffer from specified pool and adjust freebuf count variable.
 *		Should be invoked with mbuf mutex hold
 */

RTL_STATIC_INLINE void *_pvGetFreeBuffer(struct poolstat *pBufPoolStat)
{
	//Assume mbuf mutex already accquired.
	void     *pvFreeBuffer;
	//assert(pBufPoolStat);

	pvFreeBuffer = pBufPoolStat->pvFreelist;
	pBufPoolStat->iFreebufs--;

	//Assumption: Pointers are 32-bits,
	pBufPoolStat->pvFreelist = (uint32 *) (*((uint32 *) pvFreeBuffer));
	return pvFreeBuffer;
};

/*
 * 	Function:	_vReturnFreeBuffer
 *	Input:		pvFreeBuffer		pointer to freed buffer
 *				pBufPoolStat		to which pool
 *	Returns: 		None
 *	Description:
 *		Return buffer p to pool
 */

RTL_STATIC_INLINE void _vReturnFreeBuffer(register void *pvFreeBuffer,
										  register struct poolstat *pBufPoolStat)
{
	//Assume mbuf mutex already accquired.

	//Assumption: 1. Pointers are 32-bits,   2.1st field of pBufPoolStat points to next free Buffer as defined in mbuf.h
	*((uint32 *) pvFreeBuffer) = (uint32) pBufPoolStat->pvFreelist;	/* i.e.   p->next = Freelist */
	pBufPoolStat->pvFreelist = pvFreeBuffer;	/*i.e.  Freelist = p */
	pBufPoolStat->iFreebufs++;
}


/*Free iNum mbufs from mbuf chain m*/
uint32 _iFreeMbufChain(struct rtl_mBuf *pFirstMbuf, int8 flags)
{
	register struct rtl_mBuf *pThisMbuf = pFirstMbuf, *pNextMbuf = NULL;
	register struct mcluster_tag *pThisClusterTag = NULL;
	register uint32     iFreed = 0;
	register uint32     iFreedPkthdr = 0, iFreedCLusters = 0;

	assert(pFirstMbuf && ISCLEARED(pFirstMbuf->m_flags, MBUF_FREE));

	if (pThisMbuf->m_flags & MBUF_PKTHDR)
	{
		struct rtl_pktHdr *pThisPkthdr;
		int32 rxDescIdx;

		pThisPkthdr = pThisMbuf->m_pkthdr;
		rxDescIdx = pThisPkthdr->ph_rxdesc;

		if(pThisPkthdr->ph_rxPkthdrDescIdx >= PH_RXPKTHDRDESC_MINIDX)
		{
			if(rtlglue_reclaimRxBD(rxDescIdx, pThisPkthdr,  pThisMbuf))
				goto free;
			iFreed=1;
			goto out;
		}
	}
free:

	while (pThisMbuf && ISSET(pThisMbuf->m_flags, MBUF_USED))
	{
		//Free the cluster */
		if (pThisMbuf->m_flags & MBUF_EXT&& ISCLEARED(flags,MBUF_KEEPCLUSTER)){
			if(_iTotalClusters){
				
				pThisClusterTag = CLDataAddr2TagAddr(pThisMbuf->m_data);
				assert(pThisClusterTag);
				assert(pThisClusterTag->cltag_flags & CLUSTER_USED);
				pThisClusterTag->cltag_refcnt--;
				if (pThisClusterTag->cltag_refcnt == 0){   // The last referee should  free the cluster
					CLEARBITS(pThisClusterTag->cltag_flags, CLUSTER_USED);
					_vReturnFreeBuffer(pThisClusterTag, &_ClStatus);
					iFreedCLusters++;
				}else if (pThisClusterTag->cltag_mbuf == pThisMbuf){
					//The owner is quiting and cluster becomes zombie.
					//TODO: Checks what may happen if zombie cluster happens.....
					// 1. m_dtom() won't work for zombie clusters
					pThisClusterTag->cltag_mbuf = NULL;
				}
				// All other referees just silently quit.
			}else{
				uint32 count;
				if(pThisMbuf->m_extClusterId){
					rtlglue_mbufClusterRefcnt(pThisMbuf->m_extbuf, pThisMbuf->m_extClusterId, &count, 0);
					if(count==1){
						//Free the cluster when reference count =1, refcnt would be decremented inside rtlglue_mbufFreeCluster()
						rtlglue_mbufFreeCluster(pThisMbuf->m_extbuf, m_clusterSize, pThisMbuf->m_extClusterId);
					}else //we are not the last referee, just decrement the reference count.
						rtlglue_mbufClusterRefcnt(pThisMbuf->m_extbuf, pThisMbuf->m_extClusterId, &count, 2);
				}
			}	
		}
		/*
		   Free the pkthdr if exists
		 */
		if (pThisMbuf->m_pkthdr){
			if (pThisMbuf->m_flags & MBUF_PKTHDR){
				assert(pThisMbuf->m_pkthdr->ph_mbuf == pThisMbuf);
				/*
				   We are freeing the head of mbuf chain, so we need to either
				   adjust the pointer or free the pkthdr
				 */

				if ((pThisMbuf->m_next) && (flags&MBUF_FREEONE))	{
					pThisMbuf->m_pkthdr->ph_mbuf = pThisMbuf->m_next;
					assert(pThisMbuf->m_len);
					pThisMbuf->m_pkthdr->ph_len -= pThisMbuf->m_len;	/*decrease packet length */
					SETBITS(pThisMbuf->m_next->m_flags, MBUF_PKTHDR);	/* next mbuf is the head now */
				}else{
					//assert(pThisMbuf->m_pkthdr->ph_flags & PKTHDR_USED);
					CLEARBITS(pThisMbuf->m_pkthdr->ph_flags, PKTHDR_USED);
					_vReturnFreeBuffer(pThisMbuf->m_pkthdr, &_PHStatus);	/* free the pkthdr */
					iFreedPkthdr++;
				}
				pThisMbuf->m_pkthdr = NULL;
				CLEARBITS(pThisMbuf->m_flags, MBUF_PKTHDR);
			}else if (ISSET(flags, MBUF_FREEONE))		   //Free this mbuf only
				pThisMbuf->m_pkthdr->ph_len -= pThisMbuf->m_len;	/*decrease packet length */
			else					   /* user wanna free all buffers, so don't worry about pkt length... */
				pThisMbuf->m_pkthdr = NULL;
		}

		/*
		   Free the mbuf itself
		 */

		pThisMbuf->m_flags = MBUF_FREE;
		pNextMbuf = pThisMbuf->m_next;
		CLEARBITS(pThisMbuf->m_flags, MBUF_USED);
		_vReturnFreeBuffer(pThisMbuf, &_MbufStatus);	/* free the mbuf itself */

		iFreed++;

		if (ISSET(flags, MBUF_FREEALL))
			pThisMbuf = pNextMbuf;
		else
			break;
	}

	if (_iTotalClusters&&iFreedCLusters&&_ClStatus.iWaiting)
		_vWakeup(&_ClStatus.iWaiting);
	if (iFreedPkthdr&&_PHStatus.iWaiting)
		_vWakeup(&_PHStatus.iWaiting);
	if(_MbufStatus.iWaiting)
		_vWakeup(&_MbufStatus.iWaiting);

	pFirstMbuf = pNextMbuf;
out:
	return iFreed;
}


/* Duplicate or Clone a packet.
 *  Depends on the MBUF_ONLY bit. If it is set, we clone. otherwise, we duplicate
 */
static struct rtl_mBuf *_pCopyPacket(struct rtl_mBuf *pMbuf, uint32 iHow)
{
	struct rtl_mBuf *pFirstMbuf = NULL, *pThisMbuf = NULL, *pNewMbuf =
		NULL, *pPrevNewMbuf = NULL;
	struct rtl_pktHdr *pNewPkthdr = NULL;
	uint32    iCopied = 0;

	assert(pMbuf);
	assert(iHow);
	pThisMbuf = pMbuf;

	while (pThisMbuf)
	{
		assert(ISSET(pThisMbuf->m_flags, MBUF_USED));
		pNewMbuf = _m_get(iHow,  1);	// get one buffer pNewMbuf
		

		if (!pNewMbuf)
			goto nospace;

		if (pPrevNewMbuf)
			pPrevNewMbuf->m_next = pNewMbuf;	//chain new mbuf onto the chain

		pPrevNewMbuf = pNewMbuf;	   //save the pointer so we can chain it later

		if (!pFirstMbuf && ISSET(pThisMbuf->m_flags, MBUF_PKTHDR))
		{							   //for the first mbuf)
			pFirstMbuf = pNewMbuf;
			if (!_m_gethdr(pFirstMbuf, iHow))	//get a pkthdr
				goto nospace;
			assert(pFirstMbuf->m_pkthdr);
			pNewPkthdr = pFirstMbuf->m_pkthdr;
			assert(pNewPkthdr->ph_mbuf == pFirstMbuf);

			//Copy pkthdr content. skip  first 4-bytes to avoid overwriting ph_mbuf pointer */
			rtl8651_memcpy
				((void *) (((int32 *) pNewPkthdr) + 1),
				  (void *) (((int32 *) pThisMbuf->m_pkthdr) + 1),
				 sizeof(struct rtl_pktHdr) - 4);

			pNewPkthdr->ph_rxdesc = PH_RXDESC_INDRV;
			pNewPkthdr->ph_rxPkthdrDescIdx = PH_RXPKTHDRDESC_INDRV;
			assert(pFirstMbuf->m_pkthdr);
			pNewPkthdr = pFirstMbuf->m_pkthdr;
			assert(pNewPkthdr->ph_mbuf == pFirstMbuf);
			assert(pThisMbuf->m_pkthdr->ph_mbuf == pMbuf);
		}

		/*
		   Copy the mbuf content
		 */
		if ISSET
			(iHow, MBUF_ONLY)
		{							   /* Clusters would be cloned */
			rtl8651_memcpy
				((void *) (((int32 *) pNewMbuf) + 1),
				  (void *) (((int32 *) pThisMbuf) + 1),
				  sizeof(struct rtl_mBuf) - 4);
		}
		else
		{							   /* Have its own cluster */
			//be careful don't destroy m_extbuf, m_data,etc.
			pNewMbuf->m_len = pThisMbuf->m_len;
			pNewMbuf->m_flags = pThisMbuf->m_flags;
		}

		// Restore new pkthdr pointer
		if (pNewPkthdr)
			pNewMbuf->m_pkthdr = pNewPkthdr;

		assert(ISSET(pThisMbuf->m_flags, MBUF_EXT));
		assert(ISSET(pNewMbuf->m_flags, MBUF_EXT));

		if ISSET
			(iHow, MBUF_ONLY)
		{							   /* Cloning */
			/*
			   Increase reference count
			 */
			if (_iTotalClusters)
			{
				//TODO: Be careful to check  _iDataAddrToClusterIndex may be null if cluster is zombie				
				CLDataAddr2TagAddr(pThisMbuf->m_extbuf)->cltag_refcnt++;
			}else{
				pNewMbuf->m_extClusterId= pThisMbuf->m_extClusterId;				
				rtlglue_mbufClusterRefcnt(pThisMbuf->m_extbuf, pThisMbuf->m_extClusterId, NULL, 1);
			}
	
		}
		else
		{							   //Copy cluster content to new buffer
			int32 align=UNCACHE(pThisMbuf->m_data)-UNCACHE(pThisMbuf->m_extbuf);
			if(align){				
				assert(align>=0&&align<pNewMbuf->m_extsize);
				pNewMbuf->m_data+=align;
			}
			rtl8651_memcpy
				( (void *) CACHED(pNewMbuf->m_data),(void *) pThisMbuf->m_data,
				 pThisMbuf->m_len);
			pNewMbuf->m_data=(uint8 *)CACHED(pNewMbuf->m_data);
			pNewMbuf->m_extbuf=(uint8 *)CACHED(pNewMbuf->m_extbuf);
			assert(CACHED(pNewMbuf->m_data)==(uint32)pNewMbuf->m_data);
		}

		iCopied++;
		pThisMbuf = pThisMbuf->m_next;
	}
	assert(pFirstMbuf);

	return pFirstMbuf;

  nospace:
  	if(pFirstMbuf)
		_iFreeMbufChain(pFirstMbuf, MBUF_FREEALL);

	return NULL;
}

#ifndef MBUF_COMPACT
/* Duplicate or Clone an mbuf chain.
 *  Depends on the MBUF_ONLY bit. If it is set, we clone. otherwise, we duplicate
 */
static struct rtl_mBuf *_pCopyMbufChain(struct rtl_mBuf *pMbufChain, int32 iOffset, int32 iLength,	 int32 flag)
{
	register struct rtl_mBuf *pNewMbuf, *pThisMbuf, **ppNewMbuf;
	int32     off = iOffset;
	struct rtl_mBuf *pTopMbuf;
	struct rtl_pktHdr *copiedNewHdr=NULL;
	int32     copyhdr = 0;

	pThisMbuf = pMbufChain;

	assert(pThisMbuf && ISSET(pThisMbuf->m_flags, MBUF_USED));
	assert(iLength > 0 && off >= 0);

	if (ISSET(flag, MBUF_ALLOCPKTHDR))
		copyhdr = 1;				   //so we will clone a mbuf chain with its pkthdr

	if (pThisMbuf->m_len == 0){		   //User gave us a mbuf chain without any data.....
		return NULL;
	}

	while (off > 0)
	{								   /* Find from which mbuf we start cloning */
		assert(pThisMbuf && pThisMbuf->m_len);
		if (off < pThisMbuf->m_len)
			break;
		off -= pThisMbuf->m_len;
		pThisMbuf = pThisMbuf->m_next;
	}

	//We don't have so many databytes in chain now. Can't clone from indicated offset
	if (pThisMbuf->m_len < off){
		return NULL;
	}

	ppNewMbuf = &pTopMbuf;
	pTopMbuf = 0;

	while ((iLength > 0) && (pThisMbuf))
	{
		if(ISSET(flag, MBUF_ONLY))  //clone
			pNewMbuf = _m_get(MBUF_ONLY | flag,  1);	// get one buffer pNewMbuf
		else  //duplicate
			pNewMbuf = _m_get(flag,  1);	// get one buffer pNewMbuf and a cluster

		if (pNewMbuf == 0)
			goto nospace;

		*ppNewMbuf = pNewMbuf;

		if (copyhdr){	 //copy pkthdr only for the first mbuf
			if (!_m_gethdr(pNewMbuf, flag))	//get a pkthdr
				goto nospace;
			//copy the whole pkthdr header except the 1st word(4 bytes) to avoid destroy ph_mbuf pointer
			rtl8651_memcpy
				((void *) (((int32 *) pNewMbuf->m_pkthdr) + 1),
				  (void *) (((int32 *) pThisMbuf->m_pkthdr) + 1),
				 sizeof(struct rtl_pktHdr) - 4);

			pNewMbuf->m_pkthdr->ph_rxdesc = PH_RXDESC_INDRV;
			pNewMbuf->m_pkthdr->ph_rxPkthdrDescIdx = PH_RXPKTHDRDESC_INDRV;

			pNewMbuf->m_flags = pThisMbuf->m_flags;	//copy flags
			SETBITS(pNewMbuf->m_flags, MBUF_PKTHDR);
			if (iLength == MBUF_COPYALL)
				pNewMbuf->m_pkthdr->ph_len -= iOffset;
			else
				pNewMbuf->m_pkthdr->ph_len = iLength;
			copiedNewHdr = pNewMbuf->m_pkthdr;
			copyhdr = 0;
		}

		pNewMbuf->m_len = min(iLength, pThisMbuf->m_len - off);

		if(ISSET(flag, MBUF_ONLY)){
			//Cloning...
			assert(ISSET(pThisMbuf->m_flags, MBUF_EXT));
			pNewMbuf->m_data = pThisMbuf->m_data + off;

			if (_iTotalClusters)
				CLDataAddr2TagAddr(pThisMbuf->m_extbuf)->cltag_refcnt++;
			else{
				pNewMbuf->m_extClusterId= pThisMbuf->m_extClusterId;
				rtlglue_mbufClusterRefcnt(pThisMbuf->m_extbuf, pThisMbuf->m_extClusterId, NULL, 1);
			}
			pNewMbuf->m_extbuf = pThisMbuf->m_extbuf;
			pNewMbuf->m_extsize = pThisMbuf->m_extsize;
			SETBITS(pNewMbuf->m_flags, MBUF_EXT);
		}else{
			int32 align;
			//Duplicate...
			assert(ISSET(pNewMbuf->m_flags, MBUF_EXT));

			if(_iTotalClusters){
#ifdef RTL865X_DEBUG
				assert(CLDataAddr2TagAddr(pThisMbuf->m_extbuf)->cltag_refcnt==1);
#endif
			}else{
				int32 count;
				rtlglue_mbufClusterRefcnt(pThisMbuf->m_extbuf, pThisMbuf->m_extClusterId, &count, 0);
#ifdef RTL865X_DEBUG
				assert(count==1);
#endif
			}

			align=UNCACHE(pThisMbuf->m_data)-UNCACHE(pThisMbuf->m_extbuf);
			if(align){				
				assert(align>=0&&align<pNewMbuf->m_extsize);
				pNewMbuf->m_data+=align;
			}
			rtl8651_memcpy
				( (void *) pNewMbuf->m_data,(void *) pThisMbuf->m_data+off,
				 pThisMbuf->m_len);
			//pNewMbuf->m_data=(uint8 *)CACHED(pNewMbuf->m_data);
			//pNewMbuf->m_extbuf=(uint8 *)CACHED(pNewMbuf->m_extbuf);
			//assert(CACHED(pNewMbuf->m_data)==(uint32)pNewMbuf->m_data);
		}
		if(copiedNewHdr)
			pNewMbuf->m_pkthdr = copiedNewHdr;

		iLength -= pNewMbuf->m_len;	   /*decrement data bytes to be copied */
		off = 0;
		pThisMbuf = pThisMbuf->m_next;
		ppNewMbuf = &pNewMbuf->m_next;
	}

	return (pTopMbuf);
  nospace:
	if (pTopMbuf)
		_iFreeMbufChain(pTopMbuf, MBUF_FREEALL);
	return (0);
}
#endif



