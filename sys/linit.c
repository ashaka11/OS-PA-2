#include<lock.h>
#include<q.h>
#include<stdio.h>

/* linit.c - linit */
struct	lentry	locktable[NLOCKS];	/* lock table			*/
int nextlock;		/* next lock slot to use in lcreate	*/
void linit(){
    int i, j;
    struct lentry *lptr;
    for(i=0;i<NLOCKS;i++){
        lptr = &locktable[i];
		lptr->lstate = LFREE;
        
        // list of waiting readers
		lptr->lwaitreadqtail = 1 + (lptr->lwaitreadqhead = newqueue());
        
        // list of waiting writers
		lptr->lwaitwriteqtail = 1 + (lptr->lwaitwriteqhead = newqueue());
    }
    nextlock = NLOCKS -1;

    kprintf("Lock table initialized\n");
    return;
}