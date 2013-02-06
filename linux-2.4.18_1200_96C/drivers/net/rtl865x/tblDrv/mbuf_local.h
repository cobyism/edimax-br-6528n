#ifndef MBUF_LOCAL_H
#define MBUF_LOCAL_H

#define MBUF_FREEALL		1
#define MBUF_FREEONE		2
#define MBUF_KEEPCLUSTER	4
#define CLUSTERTAG_SHIFT	3
struct mcluster_tag
{
	//THis structure is aligned to 8 bytes for the convenience of converting addresses between
	//cluster tag address and cluster data address. Therefore CLUSTERTAG_SHIFT is 3.

	/*
	   This union should be at the first element. Many internal functions rely on this
	 */
	union
	{
		struct mcluster_tag *mclustertag_next;
		struct rtl_mBuf *mbuf_back;
	}
	MCLNXT;
#define cltag_mbuf		MCLNXT.mbuf_back
#define cltag_nextfreetag	MCLNXT.mclustertag_next

	uint8     cltag_flags;
	/*
	   own bit of this cluster
	 */
#define CLUSTER_FREE			BUF_FREE	/* Free  */
#define CLUSTER_USED			BUF_USED
#define CLUSTER_ASICHOLD		BUF_ASICHOLD	/* Hold by ASIC */
#define CLUSTER_DRIVERHOLD 	BUF_DRIVERHOLD	/* Hold by driver */

	uint8     cltag_refcnt;			   /* Reference count of this cluster */
	uint8     cltag_reserved[2];

};									   /* cluster Tags */

/* poolstat Should be 4-byte aligned */
struct poolstat
{
	uint32     iTotalSize;			   /* Allocated buffers in this pool */
	void     *pvAddr;				   /* starting address of pool */
	void     *pvFreelist;			   /* Freelist pointer */
	uint32 	iFreebufs;			 /* Total number of free buffers in pool */
/*	uint32 	iUsedbufs;			*/ /* Total number of inuse buffers */
	uint32     iWaiting;				   /*Number of waiting processes */
};


#define MBUF_TEST

#ifdef MBUF_TEST
	#ifdef MBUFTEST_MODULE
		#define MBUF_VAR   extern
	#else
		#define MBUF_VAR
	#endif
#else
	#define MBUF_VAR   static
#endif

MBUF_VAR memaddr _mbufMsgLogId;
MBUF_VAR uint32 _iTotalMbufs;					/*Number of toal mbufs */
MBUF_VAR uint32 _iTotalClusters;		/*Number of toal clusters */
MBUF_VAR uint32 _iTotalPkthdrs;		/*Number of toal packetheaders */
MBUF_VAR struct rtl_mBufStatus 	_Statistics; /* mbuf related statistics */
MBUF_VAR struct poolstat 	_MbufStatus, 	// _MbufPool status
						_ClStatus, 		// _ClusterTag status
						_PHStatus;		// _PkthdrPool status
#define _pFreeMbuf		((struct rtl_mBuf *) 		_MbufStatus.pvFreelist)
#define _pFreeCluster		((uint8 *) 	_ClStatus.pvFreelist)
#define _pFreePkthdr		((struct rtl_pktHdr *)		_PHStatus.pvFreelist)
MBUF_VAR struct rtl_mBuf *_MbufPool;	/* mbuf pool, initialized in mBuf_init */
MBUF_VAR uint8 *_ClusterDataPool;	/* cluster data, initialized in mBuf_init */
MBUF_VAR struct rtl_pktHdr *_PkthdrPool;	/* packet header pool, initialized in mBuf_init*/
MBUF_VAR struct mcluster_tag *_ClusterTag; /* cluster control data, initialized in mBuf_init */
MBUF_VAR uint32 m_clusterShift;
#define CLTagAddr2DataAddr(taddr)	(&_ClusterDataPool[ ((uint32)taddr - (uint32)_ClusterTag)<< (m_clusterShift - CLUSTERTAG_SHIFT) ])
#define CLDataAddr2TagAddr(daddr)	(&_ClusterTag[( (uint32)daddr - (uint32)_ClusterDataPool)>> m_clusterShift ])


#endif
