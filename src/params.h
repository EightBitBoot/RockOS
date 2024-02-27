/*
** @file	params.h
**
** @author	CSCI-452 class of 20235
**
** @brief	System configuration settings
*/

#ifndef PARAMS_H_
#define PARAMS_H_

/*
** General (C and/or assembly) definitions
*/

// Upper bound on the number of simultaneous user-level
// processes in the system (completely arbitrary)

#define N_PROCS		25

// PID of the initial user process

#define	PID_INIT	1

// First PID value assigned when processes are created
// at user-level (i.e., not created directly by the OS).

#define	FIRST_USER_PID	2

#ifndef SP_ASM_SRC

/*
** Start of C-only definitions
*/

/*
** Types
*/

/*
** Globals
*/

/*
** Prototypes
*/

#endif
// !SP_ASM_SRC

#endif
