#include <ptnetwork.h>
#include <stdint.h>

uint32_t InterlockedOr( uint32_t volatile *Destination,uint32_t Value)
{
	    return __sync_fetch_and_or(Destination,Value);
}

void bzero(void *p, int s)
{
	    memset(p,0,s);
}
