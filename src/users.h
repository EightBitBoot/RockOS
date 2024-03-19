/**
** @file	users.h
**
** @author	CSCI-452 class of 20235
**
** @brief	"Userland" configuration information
*/

#ifndef USERS_H_
#define USERS_H_

#include "common.h"

/*
** General (C and/or assembly) definitions
**
** This section of the header file contains definitions that can be
** used in either C or assembly-language source code.
*/

// delay loop counts

#define DELAY_LONG		100000000
#define DELAY_MED		4500000
#define DELAY_SHORT		2500000

#define DELAY_STD		DELAY_SHORT

#ifndef SP_ASM_SRC

/*
** Start of C-only definitions
*/

// convenience macros

// a delay loop

#define DELAY(n)	do { \
        for(int _dlc = 0; _dlc < (DELAY_##n); ++_dlc) continue; \
    } while(0)

/*
** All user main() functions have the following prototype:
**
**	int32_t name( int32_t argc, char *argv[] );
**
** To simplify declaring them, we define a macro that expands into
** that header. This can be used both in the implementation (followed
** by the function body) and in places where we just need the prototype
** (following it with a semicolon).
*/

#define USERMAIN(f)	int32_t f( int32_t argc, char *argv[] )

/*
** System call matrix
**
** System calls in this system:   exit, sleep, read, write, wtpid,
**  getdata, setdata, kill, fork, exec.
**
** There is also a "bogus" system call which attempts to use an invalid
** system call code; this should be caught by the syscall handler and
** the process should be terminated.
**
** These are the system calls which are used in each of the user-level
** main functions.  Some main functions only invoke certain system calls
** when given particular command-line arguments.
**
** Note that some system calls are nested inside library functions - e.g.,
** cwrite*() and swrite*() perform write(), etc.
**
** main1 runs for userA, userB, and userC
** main2 runs for userD and userE
** main3 runs for userF and userG
** main4 runs for userK and userL
** main5 runs for userM and userN
** main6 runs for userT, userU, and userV
** all others run individual main functions
**
**
**                        baseline system calls in use
**  fcn   exit  sleep read  write wtpid gdata sdata kill  fork  exec  bogus
** -----  ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
** init     X     X     .     X     X     .     X     .     X     X     .
** idle     X     .     .     X     .     X     .     .     .     .     .
** shell    .     .     X     X     X     .     X     .     X     X     .
** -----  ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
** main1    X     .     .     X     .     .     .     .     .     .     .
** main2    .     .     .     X     .     .     .     .     .     .     .
** main3    X     X     .     X     .     .     .     .     .     .     .
** main4    X     X     .     X     .     .     X     .     X     X     .
** main5    X     .     .     X     .     .     X     .     X     X     .
** main6    X     X     .     X     X     .     X     X     X     X     .
** ........................................................................
** userH    X     X     .     X     .     .     X     .     X     X     .
** userI    X     X     .     X     X     .     X     X     X     X     .
** userJ    X     .     .     X     .     .     X     .     X     X     .
** userP    X     X     .     X     .     X     .     .     .     .     .
** userQ    X     .     .     X     .     .     .     .     .     .     X
** userR    X     X     .     X     .     X     .     .     X     .     .
** userS    X     X     .     X     .     .     .     .     .     .     .
** userW    X     X     .     X     .     X     .     .     .     .     .
** userX    X     .     .     X     .     X     .     .     .     .     .
** userY    X     .     .     X     .     X     .     .     .     .     .
** userZ    X     X     .     .     .     .     .     .     .     .     .
** ........................................................................
*/

/*
** User process controls.
**
** To spawn a specific user process from the initial process, uncomment
** its entry in this list.
**
** Generally, most of these will exit with a status of 0.  If a process
** returns from its main function when it shouldn't (e.g., if it had
** called exit() but continued to run), it will usually return a status
** of ?.
*/

#ifdef USER_SHELL
#define	SPAWN_SHELL
#endif

// uncomment these to spawn the various user processes directly
#define SPAWN_A
#define SPAWN_B
#define SPAWN_C
#define SPAWN_D
#define SPAWN_E
#define SPAWN_F
#define SPAWN_G
#define SPAWN_H
#define SPAWN_I
#define SPAWN_J
#define SPAWN_K
#define SPAWN_L
#define SPAWN_M
#define SPAWN_N
#define SPAWN_P
#define SPAWN_Q
#define SPAWN_R
#define SPAWN_S
#define SPAWN_T
#define SPAWN_U
#define SPAWN_V

//
// There is no userO.  Users W-Z are spawned from other
// processes; they should never be spawned directly by init().
//
// Not all users have 'userX' main functions
//

/*
** Prototypes for externally-visible routines
*/

/**
** init - initial user process
**
** Spawns the other user processes, then loops forever calling wait()
**
** Invoked as:  init
*/
USERMAIN(init);

/**
** idle - the idle process
**
** Reports itself, then loops forever delaying and printing a character.
**
** Invoked as:  idle
*/
USERMAIN(idle);

#endif
/* SP_ASM_SRC */

#endif
