/* =============================================================

	Header file of GLUE-device implementation.

   ============================================================= */
#ifndef RTL865X_GLUEDEV_H
#define RTL865X_GLUEDEV_H

/* <---- Configuration varables ----> */
/* String stored in CB[] of skbuff to indicate this packet is from Rome river */
#define RTL865XB_GLUEDEV_ROMEDRVSKBSTR	"FROM_ROMEDRV"
#define RTL865XB_GLUEDEV_MAXDEVNAMELEN	8
#define RTL865XB_GLUEDEV_ROMEDRVWAN	"FROM_WAN"

#define RTL865XB_GLUEDEV_SYS_STATISTIC	/* Statistics of system counter */

/* <---- device info ----> */

/* parameter used in GLUE device initiation */
typedef struct rtl865x_gluedev_para_s
{
	uint32	valid;										/* entry is valid of invalid */
	uint32	glueDevId;									/* Control ID for GLUE device */

	/* variables for Rome Driver communication */
	uint32	defaultVid;
	uint32	extPortNumber;

	/* variables for further device information */
	char			devName[RTL865XB_GLUEDEV_MAXDEVNAMELEN];	/* device name of this glue device */
	ether_addr_t	macAddr;

} rtl865x_gluedev_para_t;


/* <---- debug message ----> */
#define GLUEDEV_DEBUG
#define GLUEDEV_MSG_MASK			0xffffffff	/* Tune ON all debug messages besides INFO */
#define GLUEDEV_MSG_INFO			(1<<0)
#define GLUEDEV_MSG_WARN			(1<<1)
#define GLUEDEV_MSG_ERR				(1<<2)

#ifdef GLUEDEV_DEBUG

#define GLUEDEV_DEBUG_PREFIX		"<GLUE Dev> "

#if (GLUEDEV_MSG_MASK&GLUEDEV_MSG_INFO)
#define GLUEDEV_INFO(fmt, args...) \
	do {rtlglue_printf(GLUEDEV_DEBUG_PREFIX "[%s-%d]-info-: " fmt "\n", __FUNCTION__, __LINE__, ## args);} while (0);
#else
#define GLUEDEV_INFO(fmt, args...)
#endif

#if (GLUEDEV_MSG_MASK&GLUEDEV_MSG_WARN)
#define CFGMGR_WARN(fmt, args...) \
	do {rtlglue_printf(GLUEDEV_DEBUG_PREFIX "[%s-%d]-warning-: " fmt "\n", __FUNCTION__, __LINE__, ## args);} while (0);
#else
#define GLUEDEV_WARN(fmt, args...)
#endif

#if (GLUEDEV_MSG_MASK&GLUEDEV_MSG_ERR)
#define GLUEDEV_ERR(fmt, args...) \
	do {rtlglue_printf(GLUEDEV_DEBUG_PREFIX "[%s-%d]-error-: " fmt "\n", __FUNCTION__, __LINE__, ## args);} while (0);
#else
#define GLUEDEV_ERR(fmt, args...)
#endif

#define GLUEDEV_PROC_CHECK(expr, success) \
	do {\
			int __retval; \
			if ((__retval = (expr)) != (success))\
			{\
				rtlglue_printf("ERROR >>> [%s]:[%d] failed -- return value: %d\n", __FUNCTION__,__LINE__, __retval);\
				return __retval; \
			}\
		}while(0)


#else

#define GLUEDEV_PROC_CHECK(expr, success)

#endif	/* GLUEDEV_DEBUG */

int32 rtl865x_glueDev_init(rtl865x_gluedev_para_t *para);
int32 rtl865x_glueDev_enable(uint32 glueDevId);
int32 rtl865x_glueDev_disable(uint32 glueDevId);
void rtl865x_glueDev_exit(void);

#endif /* RTL865X_GLUEDEV_H */

