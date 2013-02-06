/*
 * ISP116x HCD (Host Controller Driver) for USB.
 *
 * (C) Copyright 2001 Roman Weissgaerber <weissg@vienna.at>
 *
 *
 */

#define GET_FRAME_NUMBER(hci) 	READ_REG32 (hci, HcFmNumber)

/*
 * Maximum number of root hub ports.
 */
#define MAX_ROOT_PORTS	15	/* maximum OHCI root hub ports */


/* control and status registers */
#define HcRevision		0x00
#define HcControl 		0x01
#define HcCommandStatus		0x02
#define HcInterruptStatus	0x03
#define HcInterruptEnable	0x04
#define HcInterruptDisable	0x05
#define HcFmInterval		0x0D
#define HcFmRemaining		0x0E
#define HcFmNumber		0x0F
#define HcLSThreshold		0x11
#define HcRhDescriptorA		0x12
#define HcRhDescriptorB		0x13
#define HcRhStatus		0x14
#define HcRhPortStatus		0x15

#define HcHardwareConfiguration 0x20
#define HcDMAConfiguration	0x21
#define HcTransferCounter	0x22
#define HcuPInterrupt		0x24
#define HcuPInterruptEnable	0x25
#define HcChipID		0x27
#define HcScratch		0x28
#define HcSoftwareReset		0x29
#define HcITLBufferLength	0x2A
#define HcATLBufferLength	0x2B
#define HcBufferStatus		0x2C
#define HcReadBackITL0Length	0x2D
#define HcReadBackITL1Length	0x2E
#define HcITLBufferPort		0x40
#define HcATLBufferPort		0x41

/* OHCI CONTROL AND STATUS REGISTER MASKS */

/*
 * HcControl (control) register masks
 */
#define OHCI_CTRL_HCFS	(3 << 6)	/* BUS state mask */
#define OHCI_CTRL_RWC	(1 << 9)	/* remote wakeup connected */
#define OHCI_CTRL_RWE	(1 << 10)	/* remote wakeup enable */

/* pre-shifted values for HCFS */
#define OHCI_USB_RESET	(0 << 6)
#define OHCI_USB_RESUME	(1 << 6)
#define OHCI_USB_OPER	(2 << 6)
#define OHCI_USB_SUSPEND	(3 << 6)

/*
 * HcCommandStatus (cmdstatus) register masks
 */
#define OHCI_HCR	(1 << 0)	/* host controller reset */
#define OHCI_SOC  	(3 << 16)	/* scheduling overrun count */

/*
 * masks used with interrupt registers:
 * HcInterruptStatus (intrstatus)
 * HcInterruptEnable (intrenable)
 * HcInterruptDisable (intrdisable)
 */
#define OHCI_INTR_SO	(1 << 0)	/* scheduling overrun */

#define OHCI_INTR_SF	(1 << 2)	/* start frame */
#define OHCI_INTR_RD	(1 << 3)	/* resume detect */
#define OHCI_INTR_UE	(1 << 4)	/* unrecoverable error */
#define OHCI_INTR_FNO	(1 << 5)	/* frame number overflow */
#define OHCI_INTR_RHSC	(1 << 6)	/* root hub status change */
#define OHCI_INTR_ATD	(1 << 7)	/* scheduling overrun */

#define OHCI_INTR_MIE	(1 << 31)	/* master interrupt enable */

/*
 * HcHardwareConfiguration
 */
#define InterruptPinEnable 	(1 << 0)
#define InterruptPinTrigger 	(1 << 1)
#define InterruptOutputPolarity	(1 << 2)
#define DataBusWidth16		(1 << 3)
#define DREQOutputPolarity	(1 << 5)
#define DACKInputPolarity	(1 << 6)
#define EOTInputPolarity	(1 << 7)
#define DACKMode		(1 << 8)
#define AnalogOCEnable		(1 << 10)
#define SuspendClkNotStop	(1 << 11)
#define DownstreamPort15KRSel	(1 << 12)

/*
 * HcDMAConfiguration
 */
#define DMAReadWriteSelect 	(1 << 0)
#define ITL_ATL_DataSelect	(1 << 1)
#define DMACounterSelect	(1 << 2)
#define DMAEnable		(1 << 4)
#define BurstLen_1		0
#define BurstLen_4		(1 << 5)
#define BurstLen_8		(2 << 5)


/*
 * HcuPInterrupt
 */
#define SOFITLInt		(1 << 0)
#define ATLInt			(1 << 1)
#define AllEOTInterrupt		(1 << 2)
#define OPR_Reg			(1 << 4)
#define HCSuspended		(1 << 5)
#define ClkReady		(1 << 6)

/*
 * HcBufferStatus
 */
#define ITL0BufferFull		(1 << 0)
#define ITL1BufferFull		(1 << 1)
#define ATLBufferFull		(1 << 2)
#define ITL0BufferDone		(1 << 3)
#define ITL1BufferDone		(1 << 4)
#define ATLBufferDone		(1 << 5)



/* OHCI ROOT HUB REGISTER MASKS */

/* roothub.portstatus [i] bits */
#define RH_PS_CCS            0x00000001   	/* current connect status */
#define RH_PS_PES            0x00000002   	/* port enable status*/
#define RH_PS_PSS            0x00000004   	/* port suspend status */
#define RH_PS_POCI           0x00000008   	/* port over current indicator */
#define RH_PS_PRS            0x00000010  	/* port reset status */
#define RH_PS_PPS            0x00000100   	/* port power status */
#define RH_PS_LSDA           0x00000200    	/* low speed device attached */
#define RH_PS_CSC            0x00010000 	/* connect status change */
#define RH_PS_PESC           0x00020000   	/* port enable status change */
#define RH_PS_PSSC           0x00040000    	/* port suspend status change */
#define RH_PS_OCIC           0x00080000    	/* over current indicator change */
#define RH_PS_PRSC           0x00100000   	/* port reset status change */

/* roothub.status bits */
#define RH_HS_LPS	     0x00000001		/* local power status */
#define RH_HS_OCI	     0x00000002		/* over current indicator */
#define RH_HS_DRWE	     0x00008000		/* device remote wakeup enable */
#define RH_HS_LPSC	     0x00010000		/* local power status change */
#define RH_HS_OCIC	     0x00020000		/* over current indicator change */
#define RH_HS_CRWE	     0x80000000		/* clear remote wakeup enable */

/* roothub.b masks */
#define RH_B_DR		0x0000ffff		/* device removable flags */
#define RH_B_PPCM	0xffff0000		/* port power control mask */

/* roothub.a masks */
#define	RH_A_NDP	(0xff << 0)		/* number of downstream ports */
#define	RH_A_PSM	(1 << 8)		/* power switching mode */
#define	RH_A_NPS	(1 << 9)		/* no power switching */
#define	RH_A_DT		(1 << 10)		/* device type (mbz) */
#define	RH_A_OCPM	(1 << 11)		/* over current protection mode */
#define	RH_A_NOCP	(1 << 12)		/* no over current protection */
#define	RH_A_POTPGT	(0xff << 24)		/* power on to power good time */

#define min(a,b) (((a)<(b))?(a):(b))


#define URB_DEL 1


typedef struct hcipriv {
	int irq;
	int disabled;			/* e.g. got a UE, we're hung */
	atomic_t resume_count;		/* defending against multiple resumes */
	struct ohci_regs * regs;	/* OHCI controller's memory */
	int hcport;
	int wuport;

	int intrstatus;
	__u32 hc_control;		/* copy of the hc control reg */

	int frame;

	__u8 * tl;
	int tlp;
	int atl_len;
	int atl_buffer_len;
	int itl0_len;
	int itl1_len;
	int itl_buffer_len;
	int itl_index;
	int tl_last;
	int units_left;

} hcipriv_t;
struct hci;

static inline int READ_REG32 (struct hci * hci, int regindex);
static inline int READ_REG16 (struct hci * hci, int regindex);
static inline void READ_REGn16 (struct hci * hci, int regindex, int length, __u8 * buffer);
static inline void WRITE_REG32 (struct hci * hci, unsigned int value, int regindex);
static inline void WRITE_REG16 (struct hci * hci, unsigned int value, int regindex);
static inline void WRITE_REG0 (struct hci * hci, int regindex);
static inline void WRITE_REGn16 (struct hci * hci, int regindex, int length, __u8 * buffer);

/*-------------------------------------------------------------------------*/