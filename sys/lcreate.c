#include<lock.h>
#include<kernel.h>
#include <stdio.h>


int lcreate (void){
	STATWORD ps;    
	int	lock;
	int	i;

	disable(ps);

	for (i=0 ; i<NLOCKS ; i++) {
		lock=nextlock--;
		if (nextlock < 0)
            nextlock = NLOCKS-1;
		if (locktable[lock].lstate == LFREE) {
			locktable[lock].lstate = LUSE;
			locktable[lock].ltype = 0;
			locktable[lock].lcreationTime = ctr1000;
			locktable[lock].lnumreaders = 0;

			kprintf("[lcreate] Created lock. Current nextlock:%d", nextlock);
			kprintf("[lcreate] Creation time: %d", locktable[lock].lcreationTime);

			restore(ps);
			return(lock);
		}
	}

	kprintf("[lcreate] No free locks found.");
    restore(ps);
	return(SYSERR);
}
