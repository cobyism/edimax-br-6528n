#include "taia.h"

/* XXX: breaks tai encapsulation */

void taia_add(t,u,v)
struct taia *t;
struct taia *u;
struct taia *v;
{
	t->sec.x = u->sec.x + v->sec.x;
//	printf("taia_add t->sec.x= %lX\n",t->sec.x);
	
	t->nano = u->nano + v->nano;
//	printf("taia_add t->nano= %lX\n",t->nano);
	
	t->atto = u->atto + v->atto;
//	printf("taia_add t->atto= %lX\n",t->atto);

	if (t->atto > 999999999UL) 
	{
    	t->atto -= 1000000000UL;
	    ++t->nano;
	}
	if (t->nano > 999999999UL) 
	{
		t->nano -= 1000000000UL;
		++t->sec.x;
	}
}
