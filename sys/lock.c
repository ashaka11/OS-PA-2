/* lock.c - lock */

#include <lock.h>
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * lock  --  make current process wait on a lock
 *------------------------------------------------------------------------
 */
SYSCALL lock (int ldes1, int type, int priority)
{
	kprintf("[lock: %d] [currpid: %d] Locking process %d\n lock-id: %d, type: %d, priority: %d",  ldes1, currpid, currpid, ldes1, type, priority);
	STATWORD ps;    
	struct	lentry	*lptr;
	struct	pentry	*pptr;
	
	disable(ps);
	if (isbadlock(ldes1) || (lptr= &locktable[ldes1])->lstate==LFREE || type!=READ || type!= WRITE) {
		kprintf("[lock: %d] [currpid: %d] Invalid lock request: lock %d requested by %d\n", ldes1, currpid, ldes1, currpid);
		restore(ps);
		return(SYSERR);
	}

	unsigned long lcreationTime= locktable[ldes1].lcreationTime;
	pptr = &proctab[currpid];

	// if the lock can be acquired by the current process
	if(lptr->lstate == LAVAIL){
		kprintf("[lock: %d] [currpid: %d] Lock is free\n",  ldes1, currpid);
		lptr->ltype = type;

		lptr->lcurracquired[currpid] = 1;
		kprintf("[lock: %d] [currpid: %d] Process %d added to list of processes currently holding the lock\n",  ldes1, currpid, currpid);
		
		pptr->plockcurr[ldes1] = 1;
		kprintf("[lock: %d] [currpid: %d] Lock %d added to list of locks currently held by process %d\n",  ldes1, currpid, ldes1, currpid);
		kprintf("[lock: %d] [currpid: %d] Process %d acquired lock %d\n",  ldes1, currpid, currpid, ldes1);

		if(type==READ){
			lptr->lnumreaders=1;
		}

		// return control back to the process
		restore(ps);
		return(OK);
	} else if(lptr->ltype == READ){		// if the lock is acquired by a reader
		
		kprintf("[lock: %d] [currpid: %d] Lock is acquired by a reader\n",  ldes1, currpid);

		if(type==READ){ // if the process is requesting for reader access
			int waitingwriterhighestprio;
			kprintf("[lock] Waiting writer with highest priority: %d\n", waitingwriterhighestprio);
			if (isempty(lptr->lwaitwriteqhead) || priority > (waitingwriterhighestprio = getlast(lptr->lwaitwriteqhead))){
				// if there are no waiting writers or curr process has higher lock priority than the highest priority writer

				kprintf("[lock: %d] [currpid: %d] No waiting writers or current process has higher lock priority than the highest priority writer\n",  ldes1, currpid);

				pptr->plockcurr[ldes1] = 1;
				lptr->lcurracquired[currpid] = 1;
				lptr->lnumreaders++;
				restore(ps);
				return(OK);
			}else{
				kprintf("[lock: %d] [currpid: %d] Waiting writer has higher priority. Process added to waiting readers queue\n",  ldes1, currpid);

				insert(currpid, lptr->lwaitreadqhead, priority);
				pptr->pstate = PRWAIT;
				pptr->pwaitret = OK;
				pptr->plockwaiting = ldes1;
				
				resched();

				kprintf("[lock: %d] [currpid: %d] Control restored. Checking if the lock was deleted by another process",  ldes1, currpid);

				// if locktable[ldes1].lcreationTime > lcreationTime return DELETED;
				if (locktable[ldes1].lcreationTime > lcreationTime ){
					kprintf("[lock : %d] [currpid: %d ]Lock was deleted\n", ldes1, currpid);
					restore(ps);
					return DELETED;
				}
				
				pptr->plockwaiting = -1;
				pptr->plockcurr[ldes1] = 1;
				lptr->lcurracquired[currpid] = 1;
				lptr->lnumreaders++;
				restore(ps);
				return OK;
			}
		}else{ // if the process is requesting for writer access
			kprintf("[lock: %d] [currpid: %d] Process is requesting for writer access\n",  ldes1, currpid);
			// add process in the list of waiting writers
			pptr->pstate = PRWAIT;
			insert(currpid, lptr->lwaitwriteqhead, priority);
			pptr->pwaitret = OK;
			pptr->plockwaiting = ldes1;
			
			kprintf("[lock: %d] [currpid: %d] Process blocked\n",  ldes1, currpid);

			resched();

			kprintf("[lock: %d] [currpid: %d] Control restored. Checking if the lock was deleted by another process",  ldes1, currpid);
			kprintf("locktable[ldes1].lcreationTime: %ld\n", locktable[ldes1].lcreationTime);	
			kprintf("lcreationTime: %ld\n", lcreationTime);	

			if (locktable[ldes1].lcreationTime > lcreationTime ){
				kprintf("Lock was deleted\n");
				restore(ps);
				return DELETED;
			}

			kprintf("Process %d acquired lock %d\n", currpid, ldes1);
			pptr->plockwaiting = -1;
			pptr->plockcurr[ldes1] = 1;	
			lptr->lcurracquired[currpid] = 1;
			
			restore(ps);
			return OK;
		}
	}else{ // lock is acquired by a writer
		pptr->pstate = PRWAIT;
		pptr->pwaitret = OK;
		pptr->plockwaiting = ldes1;

		// add process in appropriate queue
		// if the process is reader, add in reader queue
		if(type==READ){
			insert(currpid, lptr->lwaitreadqhead, priority);
		}else{
			// if the process is writer, add in writer queue
			insert(currpid, lptr->lwaitwriteqhead, priority);
		}

		kprintf("[lock: %d] [currpid: %d] Lock is held by writer. Process blocked",  ldes1, currpid);

		resched();

		kprintf("[lock: %d] [currpid: %d] Control restored. Checking if the lock was deleted by another process",  ldes1, currpid);
		kprintf("locktable[ldes1].lcreationTime: %ld\n", locktable[ldes1].lcreationTime);
		kprintf("lcreationTime: %ld\n", lcreationTime);

		if (locktable[ldes1].lcreationTime > lcreationTime ){
			kprintf("Lock was deleted\n");
			restore(ps);
			return DELETED;
		}
		kprintf("Process %d acquired lock %d\n", currpid, ldes1);
		pptr->plockwaiting = -1;
		pptr->plockcurr[ldes1] = 1;
		lptr->lcurracquired[currpid] = 1;

		if(type==READ){
			lptr->lnumreaders++;
		}
		restore(ps);
		return OK;
	}
}
