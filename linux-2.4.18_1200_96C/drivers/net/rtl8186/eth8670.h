

#include <asm/rtl8186.h>

/*
For ETH0:
	rx-desc:0x80400400
	tx-desc:0x80400000 (hi priority)
	tx-desc:0x80400800 (lo priority)
	rx-skb: 0x80300000
	tx-skb: 0x80b00000 (hi priority)
	tx-skb: 0x80700000 (lo priority)

For ETH1:
	rx-desc:0x80600400
	tx-desc:0x80600000 (hi priority)
	tx-desc:0x80600800 (lo priority)
	rx-skb: 0x80a00000
	tx-skb: 0x80500000 (hi priority)
	tx-skb: 0x80800000 (lo priority)

*/
// NIC 0 descriptor address
#define TD0HI           0x80400000      // hi priority tx descriptor
#define TD0LO           0x80400800      // lo priority tx descriptor
#define RD0  	        0x80400400      //             rx descirptor
#define TSKB0HI		0x80b00000      // hi priority tx skb start address
#define TSKB0LO		0x80700000      // lo priority tx skb start address
#define RSKB0           0x80300000      //             rx skb start address

// NIC 1 descriptor address
#define TD1HI		0x80600000
#define TD1LO		0x80600800
#define RD1             0x80600400
#define TSKB1HI       	0x80500000 
#define TSKB1LO         0x80800000
#define RSKB1		0x80a00000





#define TX_NUM_DESC	64		// 64 descriptors
#define RX_NUM_DESC	64		// 64 descriptors
#define DESC_SIZE       16		// 16 bytes
#define DCACHEBYTES     16              // 16 bytes per D-cache line
#define MAX_PKTLEN      1514		// Max. ethernet packet size
#define SKB_SIZE        2048            // 2k bytes for SKB
#define RX_BUF_LEN      (1600)	
#define MAXPKTNUM   	500		// max. reserved packet number

enum PKTTYPE {
	IP_PKT_TYPE  = 0x0a0a,
	TCP_PKT_TYPE = 0x0b0b,
	UDP_PKT_TYPE = 0x0c0c
};

enum SPEED {
	HALF_10	  = 0,
	HALF_100  = 1,
	FULL_10   = 2,
	FULL_100  = 3
};



char eth0_mac[6]={0x12, 0xaa, 0xa5, 0x5a, 0x7d, 0xe8};
//char eth0_mac[6]={0x56, 0xaa, 0xa5, 0x5a, 0x7d, 0xe8};
char eth1_mac[6]={0x34, 0xa5, 0x5a, 0x7d, 0x12, 0xc6};
//char eth1_mac[6]={0x66, 0xa5, 0x5a, 0x7d, 0x12, 0xc6};


// For NIC

#define NIC_ID1		0x000
#define NIC_ID2		0x004
#define NIC_MAR1	0x008       
#define NIC_MAR2	0x00C       
#define NIC_TXOK	0x010	// half-word
#define NIC_RXOK	0x012	// half-word
#define NIC_TXERR	0x014	// half-word
#define NIC_RXERR	0x016	// half-word
#define NIC_RXMISS	0x018	// half-word
#define NIC_FAE		0x01A	// half-word
#define NIC_TX1COL	0x01C	// half-word
#define NIC_TXMCOL	0x01E	// half-word
#define NIC_RXPMOK	0x020	// half-word
#define NIC_RXBROK	0x022	// half-word
#define NIC_RXMUOK	0x024	// half-word
#define NIC_TXABT	0x026	// half-word
#define NIC_TXUR	0x028	// half-word
#define NIC_TRSR	0x034
#define NIC_CR		0x03B	// byte    
#define NIC_IMR		0x03C   // half-word  
#define NIC_ISR		0x03E   // half-word 
#define NIC_TCR		0x040     
#define NIC_RCR		0x044     
#define NIC_MSR		0x058	// byte      
#define NIC_MII		0x05C        

#define NIC_TXFDPH	0x1300
#define NIC_TXCDOH	0x1304
#define NIC_TXFDPL      0x1380   
#define NIC_TXCDOL	0x1384    
#define NIC_RXFDP       0x13F0   
#define NIC_RXCDO	0x13F4    
#define NIC_RXRINGNO	0x13F6 	// byte    
#define NIC_RXDESFIN    0x1430  // half-word 
#define NIC_DESDIFF	0x1432  // half-word 
#define NIC_IOCMD	0x1434  // half-word 


enum RTL8670_DESC_ATTRIBUTE {
	OWNBYNIC	=(1 << 31),
	ENDOFRING	=(1 << 30),
	FIRSTSEG	=(1 << 29),
	LASTSEG		=(1 << 28),
	RXFAE		=(1 << 27),
	MAR		=(1 << 26),
	PAM		=(1 << 25),
	BAR		=(1 << 24),
	PPPOE           =(1 << 23),
	ETH2            =(1 << 22),
	RXWT            =(1 << 21),
	RXES            =(1 << 20),
        RXUNT           =(1 << 19),
        RXCRC		=(1 << 18),
	RX_PID1		=(1 << 17),
	RX_PID0		=(1 << 16),
	IPFail		=(1 << 15),
	UDPFail		=(1 << 14),
	TCPFail		=(1 << 13),
	IPSEG		=(1 << 12),
	PROTO_IP	=3,
	PROTO_UDP	=2,
	PROTO_TCP	=1,

	WCRC            =(1 << 23),     // Tx CRC appended or not
        	
	IpTxCsumEn	= (1 << 18),
	UdpTxCsumEn	= (1 << 17),
	TcpTxCsumEn	= (1 << 16),
	RXLENMASK	=0x000007ff,	// 11 bits
	TXLENMASK	=0x00000fff,	// 12 bits
	FULLSEG		=(FIRSTSEG | LASTSEG),
	NOVLAN		= 0,
};

struct _test_struct_
{
	unsigned char	test_type;
	unsigned char	txethnr;
	unsigned char	rxethnr;
	unsigned char	txqueue;
	unsigned int	totalruns;
};

struct statistics
{
	unsigned int txhpkt;		// total tx hi queue pkt number
	unsigned int txlpkt;		// total tx lo queue pkt number
	unsigned int rxpkt;		// total rx pkt number
	unsigned int txhbytes;		// total tx hi queue pkt bytes
	unsigned int txlbytes;		// total tx lo queue pkt bytes
	unsigned int rxbytes;		// total rx pkt bytes
	unsigned int txokcnt;		// total tx ok counter
	unsigned int rxokcnt;		// total rx ok counter
	unsigned int txerr;		// total tx error counter
	unsigned int rxerrcnt;		// total rx error counter
	unsigned int rxffovcnt;		// total rx fifo overflow counter (INT)
	unsigned int rxnodescnt;	// total rx descriptor unavailable counter (INT)
	unsigned int rxerruntcnt;	// total rx runt error counter(INT)
	unsigned int tx_irqcnt;		// total tx irq counter
	unsigned int rx_irqcnt;		// total rx irq counter
	unsigned int rxdropcnt;		// total rx dropped packet caused by throttle
	unsigned int lastdroppkt;	// last dropped rx packet
	unsigned int rxcnt1int;		// rx packet number per INT
	unsigned int txhcnt1int;		// released tx packet(descriptor) number per INT
	unsigned int txlcnt1int;		// released tx packet(descriptor) number per INT
	unsigned int txunderrun;	// total tx under run counter
	unsigned int swintcnt;		// total software interrupt counter
	unsigned int rxcrccnt;		// total crc error counter
	unsigned int ipcnt;
	unsigned int tcpcnt;
	unsigned int udpcnt;
	unsigned int ipcrccnt;		// total ip checksum error counter
	unsigned int tcpcrccnt;		// total tcp checksum error counter
	unsigned int udpcrccnt;		// total udp checksum error counter
	unsigned int brdcastcnt;	// total broadcast packet counter
	unsigned int mulcastcnt;	// total multicast packet counter
	unsigned int eth2cnt;		// total ethernet 2 packet counter
	unsigned int rxuntcnt;
	unsigned int rxwtoutcnt;	// total rx watch dog timer timeout counter
	unsigned int rxfaecnt;		// total rx FAE packet counter
	unsigned int ipseccnt; 		// total IP security packet counter
	unsigned int vlancnt;		// total VLAN packet counter
	unsigned int linkchcnt;		// total link change counter
	unsigned int txhpktcounter;	// a counter for counting Txed Hi queue packet
	unsigned int txlpktcounter;	// a counter for counting Txed Lo queue packet
	unsigned int rxpktcounter;	// a counter for counting Rxed packet
};


struct eth_private
{
	unsigned int 		nr;
	unsigned int 		io_addr;
	unsigned int 		irq;
	unsigned int 		txl_num_desc;
	unsigned int 		txh_num_desc;
	unsigned int 		rx_num_desc;
	unsigned int 		rx_descaddr;
	unsigned int 		txl_descaddr;
	unsigned int 		txh_descaddr;
	unsigned int 		txl_skbaddr[TX_NUM_DESC];	
	unsigned int 		txh_skbaddr[TX_NUM_DESC];	
	unsigned int 		rx_skbaddr[RX_NUM_DESC];
	unsigned int 		cur_rx;
	unsigned int 		cur_txl;
	unsigned int 		cur_txh;
	unsigned int		last_txl;
	unsigned int		last_txh;
	unsigned int 		phy_stat;	
	unsigned char 		name[16];
	unsigned int 		rx_nr;		// rx number
	struct statistics  	res;	
	unsigned short		ip_id;
	unsigned short		udp_sport;
	unsigned short 		udp_dport;
	unsigned short		tcp_sport;
	unsigned short		tcp_dport;
	unsigned int		seq_num;
	unsigned int		ack;
	unsigned int		test_mode;	//0 ==> PHY-loopback	
	                                        //1 ==> DMA-loopback  
	                                        //2==>cross-loopback	
};


struct nic_default
{
	char 		*regname;	// register name
	unsigned long 	offset;		// IO address offset
	unsigned int 	val;		// register content	
	unsigned char   access;		// access type: byte, half word, word
};


/* Interrupt register bits, using my own meaningful names. */
enum NIC_INTERRUPT_REGISTER {
	SWInt		= BIT(10),
	TxDescUn        = BIT(9),   
	LinkCh		= BIT(8),
	TxErr		= BIT(7),
	TxOk		= BIT(6),
	RxDescUn	= BIT(5),
	RxFIFOFULL	= BIT(4),
	RxERRUNT	= BIT(2),
	RxOk		= BIT(0),
};

enum NIC_INTERRUPT_MASK {

	SWIntMask	= BIT(10),
	TxDescUnMask    = BIT(9),   
	LinkChMask	= BIT(8),
	TxErrMask	= BIT(7),
	TxOkMask	= BIT(6),
	RxDescUnMask	= BIT(5),
	RxFIFOFULLMask	= BIT(4),
	RERUNTMask	= BIT(2),
	RxOkMask	= BIT(0),
	WITHALLINT	= BIT(10)|BIT(9)|BIT(8)|BIT(7)|BIT(6)|BIT(5)|BIT(4)|BIT(2)|BIT(0),
	NOTXINT		= BIT(10)|BIT(8)|BIT(5)|BIT(4)|BIT(3)|BIT(2)|BIT(0),
	NORXINT		= BIT(10)|BIT(9)|BIT(8)|BIT(7)|BIT(6),
	NOSWINT		= BIT(9)|BIT(8)|BIT(7)|BIT(6)|BIT(5)|BIT(4)|BIT(2)|BIT(0),
	NOLNKCHGINT	= BIT(10)|BIT(9)|BIT(7)|BIT(6)|BIT(5)|BIT(4)|BIT(2)|BIT(0),
	NOINT		= 0,
};

enum NIC_TRSR_REGISTERS {
	TXOK		= BIT(3),
	TXUN		= BIT(2),
	RXFIFOEMPTY	= BIT(1),
};
enum NIC_CMD_REGISTERS {
	RxVLAN 		= BIT(2),
	RxChkSum	= BIT(1),
	NIC_Reset	= BIT(0),
}; 


/* Transmit Configuration Register */
enum NIC_TXCONFIG_REGISTER   {
	TCRIFG_CLEAR	= ~(BIT(12) | BIT(11) | BIT(10)),
	TCRIFG_960	= BIT(11) | BIT(10),
	TCRIFG_1040	= BIT(12),
	TCRIFG_1120	= BIT(12) | BIT(10),
	TCRIFG_1200	= BIT(12) | BIT(11),
	TCRIFG_1280	= BIT(12) | BIT(11) | BIT(10),
	TCRIFG_1360	= 0,
	TCRIFG_1440	= BIT(10),
	TCRIFG_1920	= BIT(11),
	TCRLBK_CLEAR	= ~(BIT(9) | BIT(8)),
	TCRLBK		= BIT(9) | BIT(8),
	TXNORMAL	= TCRIFG_960,
	TXLOOPBACK	= TXNORMAL | TCRLBK,

};	    


/* Receive Configuration Register */
enum NIC_RXCONFIG_REGISTER   {
	RCRAFLO		= BIT(6),
	RCRAER		= BIT(5),
	RCRAR		= BIT(4),
	RCRAB		= BIT(3),
	RCRAM		= BIT(2),
	RCRAPM		= BIT(1),
	RCRAAP		= BIT(0),
	RXALL		= RCRAFLO|RCRAER|RCRAR|RCRAB|RCRAM|RCRAPM|RCRAAP,
	RXNORMAL	= RCRAB|RCRAPM|RCRAM|RCRAAP,
	RXFLOWCONTROL	= RXAFLO|RXNORMAL,
	RXERROR		= RXAER|RCRAR|RCRAB|RCRAM|RCRAPM|RCRAAP,
	RXNO		= 0,
};	    

enum NIC_MEDIASTATUS_REGISTER 	{
	FRCTXFCE	= BIT(7),
	RXFCE		= BIT(6),
	TXFCE		= BIT(5),
	MEDIASPEED	= BIT(3),
	LNKSTATUS	= BIT(2),
	TXPAUSEFRAME	= BIT(1),
	RXPAUSEFRAME	= BIT(0),
};	

enum NIC_MII_REGISTER {
	MIIW		= BIT(31),
	MIIPHY1		= BIT(26);
	MIIPHY2		= BIT(27);
	MIIPHY3		= BIT(27)|BIT(26);
	MIIPHY4		= BIT(28);
	MIIPHYREGOFFSET	= 16;
	MIIPHYREG0	= 0;
	MIIPHYREG1	= BIT(16);
	MIIPHYREG2	= BIT(17);
	MIIPHYREG3	= BIT(17)|BIT(16);
	MIIPHYREG4 	= BIT(18);
	MIIPHYREG5	= BIT(18)|BIT(16);
};
/* IO Command Register */
enum NIC_IOCMD_REGISTERS    {
	TXTH_CLEAR	= ~(BIT(20) | BIT(19)),
	TXTH_64		= 0,
	TXTH_128	= BIT(19),
	TXTH_256	= BIT(20),
	
	TXINT_CLEAR	= ~(BIT(18) | BIT(17) | BIT(16)),
	TXINT_1		= 0,   		 
	TXINT_2		= BIT(16),   		 
	TXINT_3		= BIT(17),   		 
	TXINT_4		= BIT(17) | BIT(16),   		 
	TXINT_5		= BIT(18),   		 
	TXINT_6		= BIT(18) | BIT(16),   		 
	TXINT_7		= BIT(18) | BIT(17),   		 
	TXINT_8		= BIT(18) | BIT(17) | BIT(16),   		 

	RXTO_CLEAR	= ~(BIT(15) | BIT(14) | BIT(13)),
	RXTO_NO		= 0,   		 
	RXTO_82		= BIT(13),   		 
	RXTO_164	= BIT(14),   		 
	RXTO_246	= BIT(14) | BIT(13),   		 
	RXTO_328	= BIT(15),   		 
	RXTO_410	= BIT(15) | BIT(13),   		 
	RXTO_492	= BIT(15) | BIT(14),   		 
	RXTO_574	= BIT(15) | BIT(14) | BIT(13),   		 
	
	RXTH_CLEAR	= ~(BIT(12) | BIT(11)),
	RXTH_NO		= 0,   		 
	RXTH_32		= BIT(11),   		 
	RXTH_64		= BIT(12),   		 
	RXTH_128	= BIT(12) | BIT(11),   		 
	
	RXINT_CLEAR	= ~(BIT(10) | BIT(9) | BIT(8)),
	RXINT_1		= 0,   		 
	RXINT_2		= BIT(8),   		 
	RXINT_3		= BIT(9),   		 
	RXINT_4		= BIT(9) | BIT(8),   		 
	RXINT_5		= BIT(10),   		 
	RXINT_6		= BIT(10) | BIT(8),   		 
	RXINT_7		= BIT(10) | BIT(9),   		 
	RXINT_8		= BIT(10) | BIT(9) | BIT(8),   		 

	RXENABLE	= BIT(3),   		 
	TXENABLE	= BIT(2),   		 
	TXFNL		= BIT(1),   		 
	TXFNH		= BIT(0),   		 
};   

enum PHYSETTINGS {
	PHYRESET	= BIT(15),
	PHYLPBK		= BIT(14),
	PHY10M		= BIT(13),
	PHY100M		= ~BIT(13),
	PHYAUTONE	= BIT(12),
	PHYPWDN		= BIT(11),
	PHYISO		= BIT(10),
	PHYRAUTO	= BIT(9),
	PHYFULL		= BIT(8),
	PHYHALFn	= ~BIT(8),
};

enum PHYSTATUS {
	S100T4		= BIT(15),
	S100TXFULL	= BIT(14),
	S100TXHALF	= BIT(13),
	S10TFULL	= BIT(12),
	S10THALF	= BIT(11),
	AUTONEGOK	= BIT(5),
	REFAULT		= BIT(4),
	PHYLNKSTATUS	= BIT(2),
	JABBER		= BIT(1),
};

enum AUTONEGOADV {
	IAMS100T4	= BIT(9),
	IAMS100TXFULL	= BIT(8),
	IAMS100TXHALF	= BIT(7),
	IAMS10XFULL	= BIT(6),
	IAMS10THALF	= BIT(5),
};

enum AUTONEGOPAR {
	YOUS100T4	= BIT(9),
	YOUS100TXFULL	= BIT(8),
	YOUS100TXHALF	= BIT(7),
	YOUS10XFULL	= BIT(6),
	YOUS10THALF	= BIT(5),
};	

	
enum accesssize {
	byte,
	hword,
	word,
};

struct nic_default nicreg[] =
{

	{
                regname : "NIC_ID1",        
		offset	:  NIC_ID1,
		val	:  0x00000000,
		access	:  word
	},
	{
                regname : "NIC_ID2",        
		offset	:  NIC_ID2,
		val	:  0x00000000,
		access	:  word
	},
	{
                regname : "NIC_MAR1",        
		offset	:  NIC_MAR1,
		val	:  0x00000000,
		access	:  word
	},
	{
                regname : "NIC_MAR2",        
		offset	:  NIC_MAR2,
		val	:  0x00000000,
		access	:  word
	},
	{
                regname : "NIC_TXOK",        
		offset	:  NIC_TXOK,
		val	:  0x0000,
		access	:  hword
	},
	{
                regname : "NIC_RXOK",        
		offset	:  NIC_RXOK,
		val	:  0x0000,
		access	:  hword
	},
	{
                regname : "NIC_TXERR",        
		offset	:  NIC_TXERR,
		val	:  0x0000,
		access	:  hword
	},
	{
                regname : "NIC_RXERR",        
		offset	:  NIC_RXERR,
		val	:  0x0000,
		access	:  hword
	},
	{
                regname : "NIC_RXMISS",        
		offset	:  NIC_RXMISS,
		val	:  0x0000,
		access	:  hword
	},
	{
                regname : "NIC_FAE",        
		offset	:  NIC_FAE,
		val	:  0x0000,
		access	:  hword
	},
	{
                regname : "NIC_TX1COL",        
		offset	:  NIC_TX1COL,
		val	:  0x0000,
		access	:  hword
	},
	{
                regname : "NIC_TXMCOL",        
		offset	:  NIC_TXMCOL,
		val	:  0x0000,
		access	:  hword
	},
	{
                regname : "NIC_RXPMOK",        
		offset	:  NIC_RXPMOK,
		val	:  0x0000,
		access	:  hword
	},
	{
                regname : "NIC_RXBROK",        
		offset	:  NIC_RXBROK,
		val	:  0x0000,
		access	:  hword
	},
	{
                regname : "NIC_RXMUOK",        
		offset	:  NIC_RXMUOK,
		val	:  0x0000,
		access	:  hword
	},
	{
                regname : "NIC_TXABT",        
		offset	:  NIC_TXABT,
		val	:  0x0000,
		access	:  hword
	},
	{
                regname : "NIC_TXUR",        
		offset	:  NIC_TXUR,
		val	:  0x0000,
		access	:  hword
	},
	
	{
		regname : "NIC_TRSR",
		offset	: NIC_TRSR,
		val	: 0x02,
		access	: word 
	},
	{
		regname : "NIC_CR",
		offset	: NIC_CR,
		val	: 0x00,
		access	: byte 
	},
	{
		regname : "NIC_IMR",
		offset	: NIC_IMR,
		val	: 0x0000,
		access	: hword 
	},
	{
		regname : "NIC_ISR",
		offset	: NIC_ISR,
		val	: 0x0000,
		access	: hword 
	},

	{
		regname : "NIC_TCR",
		offset	: NIC_TCR,
		val	: 0x00000000,
		access	: word
	},
		
	{
		regname : "NIC_RCR",
		offset	: NIC_RCR,
		val	: 0x00000000,
		access	: word
	},

	{
		regname : "NIC_MSR",
		offset	: NIC_MSR,
		val	: 0x00,
		access	: byte 
	},

	{
		regname : "NIC_TXFDPH",
		offset	: NIC_TXFDPH,
		val	: 0x00000000,
		access	: word
	},
	{
		regname : "NIC_TXCDOH",
		offset	: NIC_TXCDOH,
		val	: 0x00000000,
		access	: word
	},
	{
		regname : "NIC_TXFDPL",
		offset	: NIC_TXFDPL,
		val	: 0x00000000,
		access	: word
	},
	{
		regname : "NIC_TXCDOL",
		offset	: NIC_TXCDOL,
		val	: 0x00000000,
		access	: word
	},
	{
		regname : "NIC_RXFDP",
		offset	: NIC_RXFDP,
		val	: 0x00000000,
		access	: word
	},
	{
		regname : "NIC_RXCDO",
		offset	: NIC_RXCDO,
		val	: 0x00000000,
		access	: word
	},
	{
		regname : "NIC_RXRINGNO",
		offset	: NIC_RXRINGNO,
		val	: 0x00000000,
		access	: word
	},
	{
		regname : "NIC_RXDESFIN",
		offset	: NIC_RXDESFIN,
		val	: 0x00000000,
		access	: word
	},
	{
		regname : "NIC_DESDIFF",
		offset	: NIC_DESDIFF,
		val	: 0x00000000,
		access	: word
	},
	{
		regname : "NIC_IOCMD",
		offset	: NIC_IOCMD,
		val	: 0xff010100,
		access	: word
	},
};


#define ETHERNET_HLEN	14
#define IP_HLEN		20
// IP to Little Endian
#define IP2L(a,b,c,d)		((a << 24) + (b << 16) + (c << 8) + d)

#define TCP_SEED 	0x38
#define UDP_SEED	0x5a 

enum DLCTYPE {
	DLC_IP  = 0x0800,
	DLC_ARP = 0x0806,
	DLC_HDR = 0x0002,
};

enum PKTTYPE {
	UDP_PKT,	// UDP packet type
	TCP_PKT,	// TCP packet type
	IP_PKT,		// IP packet type
	RAW_PKT,	// RAW packet type
};

enum PATTERNTYPE {
	ALLONE,		// all 1
	ALLZERO,	// all 0
	ALL0XA5,	// all 0xa5
	ALL0XCC,	// all 0xcc
	INCONE,		// increase 1 per byte
	INCTWO,		// increase 2 per byte
	DESCONE,	// descrease 1 per byte
	DESCTWO,	// descrease 2 per byte
	XORBUF,		// xor with buffer address
	RANDOM,		// random type packet
};



struct ip_hdr {
	unsigned char vhlentos[2];
	unsigned char length[2];
	unsigned char auth[2];
	unsigned char offset[2];
	unsigned char ttlproto[2];
	unsigned char chksum[2];
};
struct udp_hdr {
	unsigned char srcport[2];
	unsigned char desport[2];
	unsigned char length[2];
	unsigned char chksum[2];
	unsigned int  pktcount;
	unsigned char txdescno;
	unsigned char rxdescno;
	unsigned char pkttype;
	unsigned char pattern;
	unsigned int  txtimestamp;
};

struct tcp_hdr {
	unsigned char srcport[2];
	unsigned char desport[2];
	unsigned char synno[4];
	unsigned char ackno[4];
	unsigned char flags[2];
	unsigned char winsize[2];
	unsigned char chksum[2];
	unsigned char xindex[2];
};

struct hiddeninfo {
	unsigned int  pktcount;		// total tx packet count
	unsigned char txdescno;		// which Tx descriptor sends this
	unsigned char pattern;		// what pattern inside the payload
	unsigned char rxdescno;		// which Rx descriptor gets this
	unsigned char pkttype;		// what packet type this is
	unsigned int  txtimestamp;	// Tx time stamp
	unsigned int  rxtimestamp;	// Rx time stamp
	unsigned int  flags;		// bitmap flags (Not yet defined)
};
struct ethernet_pkt {

	unsigned char smac[6];
	unsigned char dmac[6];
	unsigned char dlc_type[2];
	struct ip_hdr iphdr;		// 12 bytes
	unsigned char sip[4];
	unsigned char dip[4];
	union {
		struct tcp_hdr tcphdr;	// TCP needs 20 bytes
		struct udp_hdr udphdr;	// UDP needs 8 bytes, 12 byte extra
		struct hiddeninfo rawinfo;
		unsigned char raw_data[20];	// RAW data
	};
	struct hiddeninfo myinfo;		// my information
	unsigned char payload[1500];

};

