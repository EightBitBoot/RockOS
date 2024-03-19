/**
** This source file contains two user programs: 'init' and 'shell'.
** This is done to simplify life when the shell is being used, as it
** makes use of the same "spawn table" as 'init'.
*/

#ifndef INIT_H_
#define INIT_H_

#include "users.h"

#include "ulib.h"

/**
** Initial process; it starts the other top-level user processes.
**
** Prints a message at startup, '+' after each user process is spawned,
** and '!' before transitioning to wait() mode to the SIO, and
** startup and transition messages to the console. It also reports
** each child process it collects via wait() to the console along
** with that child's exit status.
*/

/*
** For the test programs in the baseline system, command-line arguments
** follow these rules. The first two entries are as follows:
**
**	argv[0] the name used to "invoke" this process
**	argv[1] the "character to print" (identifies the process)
**
** Most user programs have one or more additional arguments.
**
** See the comment at the beginning of each user-code source file for
** information on the argument list that code expects.
*/

/*
** "Spawn table" process entry.
*/
typedef struct proc_s {
	int32_t (*entry)(int32_t,char**);	// entry point
	prio_t prio;			// process priority
	char select[3];			// identifying character, NUL, extra
	char *args[MAX_ARGS];	// argument vector strings
} proc_t;

/*
** Create a spawn table entry for a process with astring literal
** as its argument buffer.	We rely on the fact that the C standard
** ensures our array of pointers will be filled out with NULLs
*/
#define PROCENT(e, p, s, ...) { e, p, s, { __VA_ARGS__ , NULL } }

/*
** We have two spawn tables. The first one contains user-level
** processes that are started by 'init' but which are not available
** to be started by 'shell', such as 'idle' and 'shell' itself.
** These will started before the processes listed in the secondary
** table (below).
*/
static proc_t spawn_table_1[] = {

	// the idle process; it runs at Deferred priority,
	// so it will only be dispatched when there is
	// nothing else available to be dispatched
	PROCENT( idle, DeferredPrio, "!", "idle", "." ),

#ifdef SPAWN_SHELL
	// spawn a "test shell" process; it runs at System
	// priority, so it takes precedence over all other
	// user-level processes when it's not sleeping or
	// waiting for input
	PROCENT( shell, SysPrio, "@", "shell" ),
#endif

	// PROCENT( 0, 0, 0, 0 )
	{ 0 }
};

/*
** The secondary table contains process that may be spawned by 'init'
** or by 'shell'. Some of these may also be spawned by other user
** processes.
*/
static proc_t spawn_table_2[] = {

	// Users A-C each run main1, which loops printing its character
#ifdef SPAWN_A
	PROCENT( main1, UserPrio, "A", "userA", "A", "30" ),
#endif
#ifdef SPAWN_B
	PROCENT( main1, UserPrio, "B", "userB", "B", "30" ),
#endif
#ifdef SPAWN_C
	PROCENT( main1, UserPrio, "C", "userC", "C", "30" ),
#endif

	// Users D and E run main2, which is like main1 but doesn't exit()
#ifdef SPAWN_D
	PROCENT( main2, UserPrio, "D", "userD", "D", "20" ),
#endif
#ifdef SPAWN_E
	PROCENT( main2, UserPrio, "E", "userE", "E", "20" ),
#endif

	// Users F and G run main3, which sleeps between write() calls
#ifdef SPAWN_F
	PROCENT( main3, UserPrio, "F", "userF", "F", "20" ),
#endif
#ifdef SPAWN_G
	PROCENT( main3, UserPrio, "G", "userG", "G", "10" ),
#endif

	// User H tests reparenting of orphaned children
#ifdef SPAWN_H
	PROCENT( userH, UserPrio, "H", "userH", "H", "4" ),
#endif

	// User I spawns several children, kills one, and waits for all
#ifdef SPAWN_I
	PROCENT( userI, UserPrio, "I", "userI", "I" ),
#endif

	// User J tries to spawn 2 * N_PROCS children
#ifdef SPAWN_J
	PROCENT( userJ, UserPrio, "J", "userJ", "J" ),
#endif

	// Users K and L iterate spawning userX and sleeping
#ifdef SPAWN_K
	PROCENT( main4, UserPrio, "K", "userK", "K", "8" ),
#endif
#ifdef SPAWN_L
	PROCENT( main4, UserPrio, "L", "userL", "L", "5" ),
#endif

	// Users M and N spawn copies of userW and userZ via main5
#ifdef SPAWN_M
	PROCENT( main5, UserPrio, "M", "userM", "M", "5", "f" ),
#endif
#ifdef SPAWN_N
	PROCENT( main5, UserPrio, "N", "userN", "N", "5", "t" ),
#endif

	// There is no user O

	// User P iterates, reporting system time and stats, and sleeping
#ifdef SPAWN_P
	PROCENT( userP, UserPrio, "P", "userP", "P", "3", "2" ),
#endif

	// User Q tries to execute a bad system call
#ifdef SPAWN_Q
	PROCENT( userQ, UserPrio, "Q", "userQ", "Q" ),
#endif

	// User R reports its PID, PPID, and sequence number; it
	// calls fork() but not exec(), with each child getting the
	// next sequence number, to a total of five copies
#ifdef SPAWN_R
	PROCENT( userR, UserPrio, "R", "userR", "R", "20", "1" ),
#endif

	// User S loops forever, sleeping 13 sec. on each iteration
#ifdef SPAWN_S
	PROCENT( userS, UserPrio, "S", "userS", "S", "13" ),
#endif

	// Users T-V run main6(); they spawn copies of userW
	//	 User T waits for any child
	//	 User U waits for each child by PID
	//	 User V kills each child
#ifdef SPAWN_T
	PROCENT( main6, UserPrio, "T", "userT", "T", "6", "w" ),
#endif
#ifdef SPAWN_U
	PROCENT( main6, UserPrio, "U", "userU", "U", "6", "W" ),
#endif
#ifdef SPAWN_V
	PROCENT( main6, UserPrio, "V", "userV", "V", "6", "k" ),
#endif
	
	// a dummy entry to use as a sentinel
	// PROCENT( 0, 0, 0, 0 )
	{ 0 }
};

/**
** process - spawn all user processes listed in the supplied table
**
** @param ch    character identifying the process who called this function
** @param table table containing the processes to be started
*/

static void process( char ch, proc_t *table )
{
	proc_t *next = table;
	char buf[128];

	while( next->entry != NULL ) {

		// kick off the process
		int32_t p = fork();
		if( p < 0 ) {

			// error!
			sprint( buf, "INIT: fork for 0x%08x failed\n",
					(uint32_t) (next->entry) );
			cwrites( buf );

		} else if( p == 0 ) {

			// child - first change the priority
			int32_t old = setdata( Prio, next->prio );
			if( old < 0 ) {
				sprint( buf, "INIT: set prio for %d to %d, code %d\n",
						p, next->prio, old );
				cwrites( buf );
			}

			// now, send it on its way
			exec( next->entry, next->args );

			// uh-oh - should never get here!
			sprint( buf, "INIT: exec(0x%08x) failed\n",
					(uint32_t) (next->entry) );
			cwrites( buf );

		} else {

			// parent just reports that another one was started
			swritech( ch );

		}

		++next;
	}
}

/*
** The initial user process. Should be invoked with zero or one
** argument; if provided, the first argument should be the ASCII
** character 'init' will print to indicate the spawning of a process.
*/
USERMAIN( init )
{
	char ch;
	static int invoked = 0;
	char buf[128];

	if( invoked > 0 ) {
		cwrites( "*** INIT RESTARTED??? ***\n" );
		for(;;);
	}

	cwrites( "Init started\n" );
	++invoked;

	// there will always be an argv[1]
	if( argv[1] == NULL ) {
		ch = '+';
	} else {
		ch = argv[1][0];
	}

	// home up, clear on a TVI 925
	swritech( '\x1a' );

	// wait a bit
	DELAY(SHORT);

	// a bit of Dante to set the mood :-)
	swrites( "\n\nSpem relinquunt qui huc intrasti!\n\n\r" );

	/*
	** Start all the user processes
	*/

	cwrites( "INIT: starting user processes\n" );

	process( ch, spawn_table_1 );
	process( ch, spawn_table_2 );

	swrites( " !!!\r\n\n" );

	/*
	** At this point, we go into an infinite loop waiting
	** for our children (direct, or inherited) to exit.
	*/

	cwrites( "INIT: transitioning to wait() mode\n" );

	for(;;) {
		int32_t status;
		int32_t whom = waitpid( 0, &status );

		// PIDs must be positive numbers!
		if( whom <= 0 ) {
			cwrites( "INIT: waitpid() said 'no children'???\n" );
		} else {
			sprint( buf, "INIT: pid %d exit(%d)\n", whom, status );
			cwrites( buf );
		}
	}

	/*
	** SHOULD NEVER REACH HERE
	*/

	cwrites( "*** INIT IS EXITING???\n" );
	exit( 1 );

	return( 1 );  // shut the compiler up
}

#ifdef SPAWN_SHELL

/**
** run - look up and spawn the requested process
**
** performs case-independent comparisons
**
** @param which - character indicating which process to spawn
**
** @return the result from spawn(), or -90210 on error
*/
static int32_t run( char which )
{
	char buf[128];

	if( which >= 'a' && which <= 'z' ) {
		which &= 0xdf;	// map to uppercase
	}

	register proc_t *curr;
	for( curr = spawn_table_2; curr->entry != 0; ++curr ) {
		if( which == *(curr->select) ) {
			return spawn( curr->entry, curr->prio, curr->args );
		}
	}

	sprint( buf, "+++ Shell: unknown cmd '%c'\n", which );
	cwrites( buf );

	// unlikely to be an actual error code
	return( -90210 );
};


/**
** help - print the list of available commands
*/
static void help( void )
{

	swrites( "\nAvailable commands:\n" );
	swrites( " Spawn a process: " );

	proc_t *curr = spawn_table_2;
	while( curr->entry != NULL ) {
		swrites( curr->select );
		++curr;
	}
	swrites( "\n Help:  ?\n Exit:	*\n" );

}

/**
** shell - extremely simple shell for spawning test programs
*/
USERMAIN( shell )
{
	char buf[128];
	char ch;

	// keep the compiler happy
	(void) argc;
	(void) argv;

	// report that we're up and running
	cwrites( "+++ Shell is ready\n" );
	swritech( '@' );

	// loop forever
	for(;;) {

	   // prompt for the next command
	   swrites( "\n@ " );
	   int n = read( CHAN_SIO, &ch, 1 );
	   if( n < 1 ) {
		  sprint( buf, "+++ Shell input error, code %d\n", n );
		  cwrites( buf );
		  continue;
	   }

	   // process the current command
	   if( ch == '?' ) {

		  // "help"
		  help();

	   } else if( ch == '*' ) {

		  // "exit"
		  break;

	   } else {

		  // find the command character
		  int pid = run( ch );

		  if( pid == -90210 ) {

			 // bad command
			 help();

		  } else if( pid < 1 ) {

			 // spawn() failed
			 sprint( buf, "+++ Shell spawn %c failed, code %d\n", ch, pid );
			 cwrites( buf );

		  } else {

			 // wait for the process to terminate
			 int32_t status;
			 int32_t whom = waitpid( pid, &status );

			 // figure out the result
			 if( whom != pid ) {
				sprint( buf, "shell: waitpid() returned %d\n", whom );
			 } else {
				sprint( buf, "shell: PID %d exit status %d\n",
					   whom, status );
			 }
			 // report it
			 swrites( buf );
		  }

	   }

	}

	cwrites( "+++ Shell exiting\n" );
	exit( 0 );

	// keep the compiler happy
	return( 42 );
}

#endif
// SPAWN_SHELL

#endif
