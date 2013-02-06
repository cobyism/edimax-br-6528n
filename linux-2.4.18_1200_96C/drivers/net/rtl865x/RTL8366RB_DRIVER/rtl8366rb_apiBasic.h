#ifndef _RTL8366RB_APIBASIC_H_
#define _RTL8366RB_APIBASIC_H_

#define RTL8366RB_PORTMASK                  0x3F

typedef struct 
{
	uint32 vid;
	uint32 mbrmsk;
	uint32 untagmsk;
	uint32 fid;
} rtl8366rb_vlanConfig_t;

typedef struct rtl8366rb_phyAbility_s
{
	uint16  AutoNegotiation:1;/*PHY register 0.12 setting for auto-negotiation process*/
	uint16  Half_10:1;      /*PHY register 4.5 setting for 10BASE-TX half duplex capable*/
	uint16  Full_10:1;      /*PHY register 4.6 setting for 10BASE-TX full duplex capable*/
	uint16  Half_100:1;     /*PHY register 4.7 setting for 100BASE-TX half duplex capable*/
	uint16  Full_100:1;     /*PHY register 4.8 setting for 100BASE-TX full duplex capable*/
	uint16  Full_1000:1;        /*PHY register 9.9 setting for 1000BASE-T full duplex capable*/
	uint16  FC:1;           /*PHY register 4.10 setting for flow control capability*/
	uint16  AsyFC:1;        /*PHY register 4.11 setting for  asymmetric flow control capability*/
} rtl8366rb_phyAbility_t;

extern int32 rtl8366rb_initChip(void);
extern int32 rtl8366rb_initAcl(void);
extern int32 rtl8366rb_setVlan(rtl8366rb_vlanConfig_t *ptr_vlancfg);

#endif

