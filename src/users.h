/**
** @file	users.h
**
** @author	CSCI-452 class of 20225
**
** @brief	"Userland" configuration information
*/

#ifndef USERS_H_
#define USERS_H_

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

// Entry point definition

#define USERMAIN(f)	int32_t f( uint32_t arglen, void *args )

/*
** System call matrix
**
** System calls in this system:   exit, spawn, read, write, sleep,
**  kill, waitpid.
**
** There is also a "bogus" system call which attempts to use an invalid
** system call code; this should be caught by the syscall handler and
** the process should be terminated.
**
** These are the system calls which are used in each of the user-level
** main functions.  Some main functions only invoke certain system calls
** when given particular command-line arguments (e.g., main6).
**
** Note that some system calls are nested inside library functions - e.g.,
** cwrite() performs write(), etc.
**
**                        baseline system calls in use
**  fcn   exit  read  write sleep ...
** -----  ----- ----- ----- -----
** main1    .     .     .     .
** main2    .     .     .     .
** main3    .     .     .     .
** main4    .     .     .     .
** main5    .     .     .     .
** main6    .     .     .     .
** ..............................
** userH    .     .     .     .
** userI    .     .     .     .
** userJ    .     .     .     .
** userP    .     .     .     .
** userQ    .     .     .     .
** userR    .     .     .     .
** userS    .     .     .     .
** userW    .     .     .     .
** user.    .     .     .     .
** userY    .     .     .     .
** userZ    .     .     .     .
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
int32_t init( uint32_t arglen, void *args );

/**
** idle - the idle process
**
** Reports itself, then loops forever delaying and printing a character.
**
** Invoked as:  idle
*/
int32_t idle( uint32_t arglen, void *args );

#endif
/* SP_ASM_SRC */

#endif
