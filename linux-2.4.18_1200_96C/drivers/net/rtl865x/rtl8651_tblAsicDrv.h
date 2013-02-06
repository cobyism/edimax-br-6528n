#ifndef RTL8651_TBLASICDRV_H
#define RTL8651_TBLASICDRV_H


#if 0
#ifndef CONFIG_RTL865X_LIGHT_ROMEDRV
#include "types.h"
#include "rtl8651_layer2.h"
#include "rtl8651_tblDrv.h"
#endif
#endif


#ifdef CONFIG_RTL865XC
#include "rtl865xC_tblAsicDrv.h"
#elif defined CONFIG_RTL865XB
#include "rtl865xB_tblAsicDrv.h"
#else
#error "rtl8651_tblAsicDrv.h : Unknown Platform"
#endif

int32 rtl8651_flowContrlThreshold(uint32 a, uint32 value);
int32 rtl8651_flowContrlPrimeThreshold(uint32 a, uint32 value);
int32 rtl8651_resetSwitchCoreStore(void);
int32 rtl8651_resetSwitchCoreActionAndConfigure(void);

#endif

