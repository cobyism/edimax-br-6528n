#include <sys/types.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "strerr.h"
#include "ip.h"
#include "str.h"
#include "byte.h"
#include "substdio.h"
#include "readwrite.h"
#include "select.h"
#include "scan.h"
#include "leapsecs.h"
#include "tai.h"
#include "taia.h"

struct tm {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
};

#define NTP_OFFSET 2208988790UL /* TAI64 baseline - NTP epoch */
/* TAI64 baseline is 1970-01-01 00:00:00 = 4000000000000000 TAI64 */
/* NTP epoch is 1900-01-01 00:00:10 = 3fffffff7c55818a TAI64 */

time_t when=0;
unsigned long nano=0;
unsigned long atto=0;
int test_timezone;

int timezone_offset[]={ -12, -11, -10, -9, -8, -7, -7, -6, -6, -6,
						 -5,  -5,  -5, -4, -4, -4, -3, -3, -3, -2,
						 -1,   0,   0,  1,  1,  1,  1,  1,  1,  2,
						  2,   2,   2,  2,  2,  3,  3,  3,  3,  4,
						  4,   4,   5,  5,  5,  6,  6,  7,  8,  8,
						  8,   8,   9,  9,  9,  9,  9, 10, 10, 10,
						 10,  10,  11, 12, 12 };

void test( ntp, flagleap )
unsigned char* ntp;
int flagleap;
{
	unsigned char buf[16]={0};
	struct tai t;
	unsigned long u;
	double z;
	
	when = 0;
	nano = 0;
	atto = 0;

	u = (unsigned long) ntp[0];
	u <<= 8; u += (unsigned long) ntp[1];
	u <<= 8; u += (unsigned long) ntp[2];
	u <<= 8; u += (unsigned long) ntp[3];
	u -= NTP_OFFSET;

	/* safe to assume that now is past 1970 */

	buf[0] = 64;
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = 0;
	buf[7] = u; u >>= 8;
	buf[6] = u; u >>= 8;
	buf[5] = u; u >>= 8;
	buf[4] = u;

	tai_unpack(buf,&t);
	leapsecs_add(&t,flagleap);
	tai_pack(buf,&t);

	u = (unsigned long) ntp[4];
	u <<= 8; u += (unsigned long) ntp[5];
	u <<= 8; u += (unsigned long) ntp[6];
	u <<= 8; u += (unsigned long) ntp[7];
	z = u / 4294967296.0;

	z *= 1000000000.0;
	u = z;
	if (u > z) 
		--u;
	if (u > 999999999) 
		u = 999999999;
	z -= u;
	buf[11] = u; u >>= 8;
	buf[10] = u; u >>= 8;
	buf[9] = u; u >>= 8;
	buf[8] = u;

	z *= 1000000000.0;
	u = z;
	if (u > z) 
		--u;
	if (u > 999999999) 
		u = 999999999;
	buf[15] = u; u >>= 8;
	buf[14] = u; u >>= 8;
	buf[13] = u; u >>= 8;
	buf[12] = u;

	//
	u = buf[12];
	u <<= 8; u += buf[13];
	u <<= 8; u += buf[14];
	u <<= 8; u += buf[15];

	atto += u;
	if (atto > 999999999) 
	{
		atto -= 1000000000;
	    ++nano;
	}

	u = buf[8];
	u <<= 8; u += buf[9];
	u <<= 8; u += buf[10];
	u <<= 8; u += buf[11];

	nano += u;
	if (nano > 999999999) 
	{
	   nano -= 1000000000;
	   ++when;
	}

	u = buf[4];
	u <<= 8; u += buf[5];
	u <<= 8; u += buf[6];
	u <<= 8; u += buf[7];
	
	if (u < 2147483648UL)
		when += u;
    else
		when -= (4294967295UL + 1 - u);
	
	print();
}

int print(void)
{ 			    
	char buffer[32];
	struct tm *tmp;
	tmp = localtime((time_t*)&when);

    printf("Get date = %d.%d.%d-%d:%d:%d, offset= %d\n",
	                    1900 + tmp->tm_year,
	                    1 + tmp->tm_mon,
	                    tmp->tm_mday,
	                    tmp->tm_hour,
	                    tmp->tm_min,
	                    tmp->tm_sec,
						timezone_offset[test_timezone]);
	
	if( (tmp->tm_hour + timezone_offset[test_timezone]) < 0 )
	{
//		tmp->tm_hour = tmp->tm_hour - timezone_offset[test_timezone];
		tmp->tm_hour = tmp->tm_hour + timezone_offset[test_timezone];	//20060414 pippen
		//tmp->tm_mday--;	//20060414 pippen 
		if( tmp->tm_mday < 1 )
		{
			tmp->tm_mon--;
			if( tmp->tm_mon < 0 )
			{
				tmp->tm_year--;
				tmp->tm_mon = 11;
			}
			switch( (tmp->tm_mon + 1) )
			{
			case 1:
			case 3:
			case 5:
			case 7:
			case 8:
			case 10:
			case 12:
				tmp->tm_mday = 31;
				break;
			case 2:
				tmp->tm_mday = 28;
				break;
			case 4:
			case 6:
			case 9:
			case 11:
				tmp->tm_mday = 30;
				break;
			}
		}
	}
	else
	{
		if ( (tmp->tm_hour + timezone_offset[test_timezone]) > 24 )
		{
			tmp->tm_hour = tmp->tm_hour + timezone_offset[test_timezone] - 24;
			tmp->tm_mday++;
			switch( (tmp->tm_mon + 1) )
			{
			case 1:
			case 3:
			case 5:
			case 7:
			case 8:
			case 10:
				if( tmp->tm_mday > 31 )
				{
					tmp->tm_mday = 1;
					tmp->tm_mon++;
				}
				break;
			case 12:
				if( tmp->tm_mday > 31 )
				{
					tmp->tm_mday = 1;
					tmp->tm_mon = 0;
					tmp->tm_year++;
				}
				break;
			case 2:
				if( tmp->tm_mday > 28 )
				{
					tmp->tm_mday = 1;
					tmp->tm_mon++;
				}
				break;
			case 4:
			case 6:
			case 9:
			case 11:
				if( tmp->tm_mday > 30 )
				{
					tmp->tm_mday = 1;
					tmp->tm_mon++;
				}
				break;
			}
		}
		else
			tmp->tm_hour = tmp->tm_hour + timezone_offset[test_timezone];
	}
	//EDX Steven
	if(test_timezone==16)
	{
		sprintf(buffer,"date %d.%d.%d-%d:%d:%d > /dev/null",
					1900 + tmp->tm_year,
					1 + tmp->tm_mon,
					tmp->tm_mday,
					tmp->tm_hour,
					-30+tmp->tm_min,
					tmp->tm_sec);
	}
	else if(test_timezone==38||test_timezone==41||test_timezone==44||test_timezone==55||test_timezone==56)
	{
		sprintf(buffer,"date %d.%d.%d-%d:%d:%d > /dev/null",
					1900 + tmp->tm_year,
					1 + tmp->tm_mon,
					tmp->tm_mday,
					tmp->tm_hour,
					30+tmp->tm_min,
					tmp->tm_sec);
	}
	else //EDX Steven
	sprintf(buffer,"date %d.%d.%d-%d:%d:%d > /dev/null",
					1900 + tmp->tm_year,
					1 + tmp->tm_mon,
					tmp->tm_mday,
					tmp->tm_hour,
					tmp->tm_min,
					tmp->tm_sec);

	printf("use command: [%s]\n",buffer);
	
	system(buffer);
}

/*
void ntp_taia(ntp,ta,flagleap)
unsigned char *ntp;
struct taia *ta;
int flagleap;
{
  unsigned char buf[16];
  struct tai t;
  unsigned long u;
  double z;

  u = (unsigned long) ntp[0];
  u <<= 8; u += (unsigned long) ntp[1];
  u <<= 8; u += (unsigned long) ntp[2];
  u <<= 8; u += (unsigned long) ntp[3];
  u -= NTP_OFFSET;

  // safe to assume that now is past 1970

  buf[0] = 64;
  buf[1] = 0;
  buf[2] = 0;
  buf[3] = 0;
  buf[7] = u; u >>= 8;
  buf[6] = u; u >>= 8;
  buf[5] = u; u >>= 8;
  buf[4] = u;

  tai_unpack(buf,&t);
  leapsecs_add(&t,flagleap);
  tai_pack(buf,&t);

  u = (unsigned long) ntp[4];
  u <<= 8; u += (unsigned long) ntp[5];
  u <<= 8; u += (unsigned long) ntp[6];
  u <<= 8; u += (unsigned long) ntp[7];
  z = u / 4294967296.0;

  z *= 1000000000.0;
  u = z;
  if (u > z) --u;
  if (u > 999999999) u = 999999999;
  z -= u;
  buf[11] = u; u >>= 8;
  buf[10] = u; u >>= 8;
  buf[9] = u; u >>= 8;
  buf[8] = u;

  z *= 1000000000.0;
  u = z;
  if (u > z) --u;
  if (u > 999999999) u = 999999999;
  buf[15] = u; u >>= 8;
  buf[14] = u; u >>= 8;
  buf[13] = u; u >>= 8;
  buf[12] = u;

  taia_unpack(buf,ta);
}
*/

//char outbuf[16];
//substdio ssout = SUBSTDIO_FDBUF(write,1,outbuf,sizeof outbuf);

//#define FATAL "sntpclock: fatal: "
//#define WARNING "sntpclock: warning: "

void die_usage()
{
//	strerr_die1x(100,"sntpclock: usage: sntpclock ip.ad.dr.ess [timezone]");
	printf("sntpclock: usage: sntpclock ip.ad.dr.ess [timezone]\n");
	exit(1);
}

char *host;
struct ip_address ipremote;
struct sockaddr_in sa;
int s;

unsigned char query[48];
unsigned char response[128];

//char initdeltaoffset[] = {0,0,0,0,0,2,163,0,0,0,0,0,0,0,0,0};
//char initdeltamin[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//char initdeltamax[] = {0,0,0,0,0,5,70,0,0,0,0,0,0,0,0,0};
//char initerrmin[] = {255,255,255,255,255,255,255,254,0,0,0,0,0,0,0,0};
//char initerrmax[] = {0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0};
//struct taia deltaoffset;
//struct taia deltamin;
//struct taia deltamax;
//struct taia errmin;
//struct taia errmax;

//struct taia ta0;
//struct taia ta1;
//struct taia taremote;

//struct taia temp1;
//struct taia temp2;

//unsigned char adj[16];

unsigned char test_time[16];
int test_flagleap;

void main(argc,argv)
int argc;
char **argv;
{
	struct timeval tvselect;
	fd_set rfds;
	char *x;
	unsigned long u;
	int r;
	int loop;
	struct timeval tvcookie;
	int flagleap;
	
	int GetOK=0;

	if (leapsecs_init() == -1)
	{
//    	strerr_die2sys(111,FATAL,"unable to initialize leap seconds: ");
		printf("SNTP: ERROR - unable to initialize leap seconds\n");
		exit(1);
	}

	if( argc != 3 )
		die_usage();
	else
		printf("SNTP: start...argv[1]= %s argv[2]= %s\n",argv[1],argv[2]);
	
	host = argv[1];
	if (!host) 
		die_usage();
	if (!str_diff(host,"0")) 
		host = "127.0.0.1";
	if (host[ip_scan(host,&ipremote)]) 
		die_usage();

	test_timezone = atoi(argv[2]);
	printf("time zone= %d [ GMT %s %d]\n",test_timezone,
					                     (timezone_offset[test_timezone] < 0 )?"":"+",
					                     timezone_offset[test_timezone]);
	
	s = socket(AF_INET,SOCK_DGRAM,0);
	if (s == -1)
	{
//    	strerr_die2sys(111,FATAL,"unable to create socket: ");
		printf("SNTP: ERROR - unable to create socket\n");
		exit(1);
	}

	byte_zero(&sa,sizeof(sa));
	byte_copy(&sa.sin_addr,4,&ipremote);
	x = (char *) &sa.sin_port;
	x[0] = 0;
	x[1] = 123; /* NTP */
	sa.sin_family = AF_INET;

	for (loop = 0;loop < 10;++loop) 
	{
    	byte_zero(query,sizeof query);
    	query[0] = 27; /* client, NTP version 3 */
    	query[2] = 8;
  
    	gettimeofday(&tvcookie,(struct timezone *) 0);
    	u = tvcookie.tv_sec + NTP_OFFSET;
    	query[43] = u; u >>= 8;
    	query[42] = u; u >>= 8;
    	query[41] = u; u >>= 8;
	    query[40] = u;
    	u = tvcookie.tv_usec;
	    query[45] = u; u >>= 8; /* deliberately inaccurate; this is a cookie */
    	query[44] = u;
	    u = getpid();
    	query[47] = u; u >>= 8;
	    query[46] = u;
  
	    if (sendto(s,query,sizeof query,0,(struct sockaddr *) &sa,sizeof sa) == -1)
		{
//    		strerr_die2sys(111,FATAL,"unable to send request: ");
			printf("SNTP loop %d : ERROR - unable to send request\n",loop);
			exit(1);
		}
		
	    FD_ZERO(&rfds);
    	FD_SET(s,&rfds);
	    tvselect.tv_sec = 1;
    	tvselect.tv_usec = 0;
	    if (select(s + 1,&rfds,(fd_set *) 0,(fd_set *) 0,&tvselect) != 1) 
		{
//    		strerr_warn2(WARNING,"unable to read clock: timed out",0);
    		printf("SNTP loop %d : unable to read clock timed out\n",loop);
			continue;
	    }
		
	    r = recv(s,response,sizeof response,0);
    	if (r == -1) 
		{
//			strerr_warn2(WARNING,"unable to read clock: ",&strerr_sys);
			printf("SNTP loop %d : unable to read clock ",loop);
			continue;
	    }
		
	    if( (r < 48)
			|| (r >= sizeof response)
			|| (((response[0] & 7) != 2) && ((response[0] & 7) != 4))
			|| !(response[0] & 56)
			|| byte_diff(query + 40,8,response + 24) ) 
		{
//			strerr_warn2(WARNING,"unable to read clock: bad response format",0);
			printf("SNTP loop %d : unable to read clock: bad response format",loop);
			continue;
	    }

	    flagleap = ((response[0] & 192) == 64);

		memcpy(test_time, response + 32, 16);
		test_flagleap = flagleap;

		GetOK = 1;
		
		break;
	}

	if( GetOK )
	{
		test(test_time,test_flagleap);
		printf("SNTP: SUCCESS\n");
		exit(0);
	}
	else
	{
		printf("SNTP: ERROR - unable to read clock\n");
		exit(1);
	}
}
