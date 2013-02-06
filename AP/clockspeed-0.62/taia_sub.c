#include "taia.h"

/* XXX: breaks tai encapsulation */

void taia_sub(t,u,v)
struct taia *t;
struct taia *u;
struct taia *v;
{
	unsigned long unano = u->nano;
	unsigned long uatto = u->atto;
  
	t->sec.x = u->sec.x - v->sec.x;
//	printf("taia_add t->sec.x= %lX\n",t->sec.x);
  
	t->nano = unano - v->nano;
//	printf("taia_add t->nano= %lX\n",t->nano);
  
	t->atto = uatto - v->atto;
//	printf("taia_add t->atto= %lX\n",t->atto);
  
	if (t->atto > uatto) 
	{
  		t->atto += 1000000000UL;
		--t->nano;
	}
	if (t->nano > unano) 
	{
    	t->nano += 1000000000UL;
		--t->sec.x;
	}
}
