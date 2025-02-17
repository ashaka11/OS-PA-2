/* lock.h */

#ifndef _LOCK_H_
#define _LOCK_H_

#ifndef	NLOCKS
#define	NLOCKS	50		/* maximum number of locks allowed	*/
#endif

#define	DELETED		 1	/*  lock has been deleted		*/
#define	READ		-1	/*  read lock	 */   
#define	WRITE		-1	/*  write lock   */

#define	LFREE	'\01'		/* this lock is free		*/
#define	LUSE	'\02'		/* this lock is in use		*/

struct	lentry	{		    /* lock table entry		            */
	char	lstate;		    /* the state LFREE or LUSE		    */
	int	    lrqhead;		/* q index of head of read list	    */
    int	    lrqtail;		/* q index of tail of read list	    */
	int	    lwqhead;		/* q index of head of write list    */
    int	    lwqtail;		/* q index of tail of write list	*/
};

extern	struct	lentry	locktable[];
extern	int	nextlock;

int lcreate (void);
int ldelete (int lockdescriptor);
int lock (int ldes1, int type, int priority);
int releaseall (int numlocks, int *ldes,...);
#endif
