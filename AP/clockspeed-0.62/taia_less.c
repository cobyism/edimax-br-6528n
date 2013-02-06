
#include "taia.h"

/* XXX: breaks tai encapsulation */

int taia_less(t,u)
struct taia *t;
struct taia *u;
{
	if (t->sec.x < u->sec.x) 
	{
//		printf(" t->sec.x < u->sec.x\n");
		return 1;
	}
	if (t->sec.x > u->sec.x) 
	{
//		printf(" t->sec.x > u->sec.x\n");
		return 0;
	}
	if (t->nano < u->nano) 
	{
//		printf(" t->nano < u->nano\n");
		return 1;
	}
	if (t->nano > u->nano) 
	{
//		printf(" t->nano < u->nano\n");
		return 0;
	}
  
	return t->atto < u->atto;
}
