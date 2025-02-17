#include<lock.h>

/* linit.c - linit */
struct	lentry	locktable[NLOCKS];	/* lock table			*/
int nextlock;		/* next lock slot to use in lcreate	*/
void linit(){
    int i;
    struct lentry *lptr;
    for(i=0;i<NLOCKS;i++){
        lptr = &locktable[i];
		lptr->lstate = LFREE;
		lptr->lrqtail = 1 + (lptr->lrqhead = newqueue());
		lptr->lwqtail = 1 + (lptr->lwqhead = newqueue());
    }
    nextlock = 0;
    return;
}