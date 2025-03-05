#include <lock.h>
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>

void  wakeup_readers(int prio, struct  lentry *lptr){
	// if a reader is chosen to have a lock, then all the other waiting readers having priority greater than that of the highest-priority waiting writer for the same lock should also be admitted.
	kprintf("[wakeup_readers] Waking up readers with priority %d\n", prio);

	while(!nonempty(lptr->lwaitreadqhead) &&   lastkey(lptr -> lwaitreadqtail)>= prio){
		int pid = getlast(lptr->lwaitreadqhead);
		kprintf("Adding reader with priority %d\n", pid);
		ready(pid, RESCHNO);
	}
	resched();
}

void wakeup_highest_waiting_process(struct lentry *lptr){
	int highestreader, highestwriter; 
	Bool waitingreaders = FALSE, waitingwriters = FALSE;

	if ( nonempty(lptr->lwaitreadqhead)){
		highestreader = lastkey(lptr->lwaitreadqtail);
		waitingreaders = TRUE;
	}
	if ( nonempty(lptr->lwaitreadqhead)){
		highestwriter = lastkey(lptr->lwaitreadqtail);
		waitingwriters = TRUE;
	}

	if (waitingreaders && waitingwriters){
		kprintf("[wakeup_highest_waiting_process] Both readers and writers are waiting\n");
		if (highestreader > highestwriter){
			wakeup_readers(highestreader, lptr);
			// ready(getlast(lptr->lwaitreadqhead), RESCHYES);
		}else{
			// release the highest writer
			kprintf("[wakeup_highest_waiting_process] Releasing the highest writer\n");
			ready(getlast(lptr->lwaitwriteqhead), RESCHYES);
		}
	}else if(waitingreaders){
		wakeup_readers(highestreader, lptr);

		// ready(getlast(lptr->lwaitreadqhead), RESCHYES);}
	}else if(waitingwriters){
		kprintf("[wakeup_highest_waiting_process] Releasing the highest writer\n");
		ready(getlast(lptr->lwaitwriteqhead), RESCHYES);
	}else{
		// this was the last process holding the lock
		lptr->ltype = 0;
	}

}


int release(int ldes){
    STATWORD ps;    
	register struct	lentry	*lptr;

	disable(ps);
	if (isbadlock(ldes) || (lptr= &locktable[ldes])->lstate==LFREE || locktable[ldes].lcurracquired[currpid] == 0) {
		kprintf("[release] Invalid lock request: lock %d not currently held by %d\n", ldes, currpid);
		restore(ps);
		return(SYSERR);
	}

	lptr->lcurracquired[currpid] = 0;
	proctab[currpid].plockcurr[ldes] = 0;

	if(lptr->ltype == READ){
		lptr->lnumreaders--;
		if(lptr->lnumreaders == 0){
			// this was the last reader
			kprintf("[release lock-id: %d] [ currproc: %d] Last reader released the lock\n", ldes, currpid);

			// wakeup highest waiting reader or writer
			kprintf("[release lock-id: %d] [ currproc: %d] Waking up highest waiting process\n", ldes, currpid);
			wakeup_highest_waiting_process(lptr);
		}
		kprintf("[release lock-id: %d] [ currproc: %d] Other readers currently acquired the lock so not waking up anyone\n", ldes, currpid);


	}else if(lptr->ltype==WRITE){
		// writer releasing a lock
		kprintf("[release lock-id: %d] [ currproc: %d] Writer released the lock\n", ldes, currpid);

		// wakeup highest waiting reader or writer
		kprintf("[release lock-id: %d] [ currproc: %d] Waking up highest waiting process\n", ldes, currpid);
		wakeup_highest_waiting_process(lptr);
	}

	restore(ps);
	return(OK);

}

