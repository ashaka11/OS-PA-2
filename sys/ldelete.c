/* ldelete.c - ldelete */
#include<lock.h>

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * ldelete  --  delete a lock by releasing its table entry
 *------------------------------------------------------------------------
 */
int ldelete (int lockdescriptor)
{
	STATWORD ps;    
	int	pid, i;
	struct	lentry	*lptr;

	disable(ps);
	if (isbadlock(lockdescriptor) || locktable[lockdescriptor].lstate==LFREE) {
		kprintf("[Ldelete] Invalid lock descriptor or lock state %s !=LFREE\n",locktable[lockdescriptor].lstate );
		restore(ps);
		return(SYSERR);
	}

	lptr = &locktable[lockdescriptor];
	lptr->lstate = LFREE;
	for(i=0;i<NPROC;i++){
		lptr->lcurracquired[i] = 0;
	}

	if (nonempty(lptr->lwaitreadqhead) || nonempty(lptr->lwaitwriteqhead)) {
		while( (pid=getfirst(lptr->lwaitwriteqhead)) != EMPTY){
		    proctab[pid].pwaitret = DELETED;
			kprintf("[Ldelete] Process %d deleted from lrqhead\n",pid);
		    ready(pid,RESCHNO);
		}

		while( (pid=getfirst(lptr->lwaitreadqhead)) != EMPTY){
		    proctab[pid].pwaitret = DELETED;
			kprintf("[Ldelete] Process %d deleted from lwqhead\n",pid);
		    ready(pid,RESCHNO);
		}
		resched();
	}

	restore(ps);
	return(OK);
}
