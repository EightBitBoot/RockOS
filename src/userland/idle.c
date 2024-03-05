#ifndef IDLE_H_
#define IDLE_H_

#include "users.h"
#include "ulib.h"

/**
** Idle process:  write, getpid, getdata, exit
**
** Reports itself, then loops forever delaying and printing a character.
**
** Invoked as:	idle
*/

USERMAIN( idle )
{
	// this is the character we will repeatedly print
	char ch = '.';

	// ignore the command-line arguments
	(void) argc;
	(void) argv;

	// get some current information
	pid_t pid = (pid_t) getdata( Pid );
	uint32_t now = (uint32_t) getdata( Time );
	prio_t prio = (prio_t) getdata( Prio );

	char buf[128];
	sprint( buf, "Idle [%d], prio %d, started @ %u\n", pid, prio, now );
	cwrites( buf );
	
	// report our presence on the console
	cwrites( "Idle started\n" );

	write( CHAN_SIO, &ch, 1 );

	// idle() should never block - it must always be available
	// for dispatching when we need to pick a new current process

	for(;;) {
		DELAY(LONG);
		write( CHAN_SIO, &ch, 1 );
	}

	// we should never reach this point!
	now = (uint32_t) getdata( Time );
	sprint( buf, "Idle [%d] EXITING @ %u!?!?!\n", pid, now );
	cwrites( buf );

	exit( 1 );

	return( 42 );
}

#endif
