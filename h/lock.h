/* lock.h */

#ifndef _LOCK_H_
#define _LOCK_H_

#ifndef	NLOCKS
#define	NLOCKS	5		/* maximum number of locks allowed	*/
#endif

extern unsigned long	ctr1000;	/* counts in 1000ths of second 0-INF	*/

#define	READ		1	/*  read lock	 */   
#define	WRITE		2	/*  write lock   */

#define	LFREE	'\01'		/* this lock is free		*/
#define	LUSE	'\02'		/* this lock is in use		*/
#define	LAVAIL	'\03'		/* this lock is in use		*/

struct	lentry	{		    /* lock table entry		            */
	char			lstate;		    /* the state LFREE or LUSE		    */
	int	    		lwaitreadqhead;		/* q index of head of read waitlist	    */
    int	    		lwaitreadqtail;		/* q index of tail of read waitlist	    */
	int	    		lwaitwriteqhead;		/* q index of head of write waitlist    */
    int	    		lwaitwriteqtail;		/* q index of tail of write waitlist	*/
	int				ltype;     /* lock type 0 if lock is not acquired currently   1 for read and 2 for write */
	unsigned long 	lcreationTime;   /* time of creation of lock         */
    int	    		lcurracquired[50];		/* processes currently holding the lock will have value 1   */
	int 			lnumreaders;		/* number of readers holding the lock */
};

extern	struct	lentry	locktable[];
extern	int	nextlock;

int lcreate (void);
int ldelete (int lockdescriptor);
int lock (int ldes1, int type, int priority);
int releaseall (int numlocks, int *ldes,...);

#define	isbadlock(l)	(l<0 || l>=NLOCKS)

#endif
