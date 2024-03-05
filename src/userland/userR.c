#ifndef USER_R_H_
#define USER_R_H_

#include "users.h"
#include "ulib.h"

/**
** User function R:   exit, sleep, read, write
**
** Reports itself, then loops forever reading and printing SIO characters
**
** Invoked as:  userR  x  [ s ]
**	 where x is the ID character
**		   s is the initial delay time (defaults to 10)
*/

USERMAIN( userR ) {
	char ch = 'r';	  // default character to print
	int count = 10;	  // initial delay count
	char buf[128];
	char b2[8] = "r[?]";	// will replace 'r', '?' below

	// process the command-line arguments
	switch( argc ) {
	case 3:	count = str2int( argv[2], 10 );
			// FALL THROUGH
	case 2:	ch = argv[1][0];
			break;
	default:
			sprint( buf, "userR: argc %d, args: ", argc );
			cwrites( buf );
			for( int i = 0; i <= argc; ++i ) {
				sprint( buf, " %s", argv[argc] ? argv[argc] : "(null)" );
				cwrites( buf );
			}
			cwrites( "\n" );
	}

	// announce our presence
	b2[0] = ch;

	write( CHAN_SIO, b2, 1 );

	sleep( SEC_TO_MS(count) );

	for(;;) {
		int n = read( CHAN_SIO, &b2[2], 1 );
		if( n != 1 ) {
			sprint( buf, "!! %c, read returned %d\n", ch, n );
			cwrites( buf );
			if( n == -1 ) {
				// wait a bit
				sleep( SEC_TO_MS(1) );
			}
		} else {
			write( CHAN_SIO, b2, 4 );
		}
	}

	sprint( buf, "!! %c left its infinite loop?!?!?!?\n", ch );
	cwrites( buf );
	exit( 1 );

	return( 42 );  // shut the compiler up!

}

#endif
