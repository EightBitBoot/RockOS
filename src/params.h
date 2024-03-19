/**
** @file	params.h
**
** @author	CSCI-452 class of 20235
**
** @brief	System configuration settings
**
** This header file contains many of the "easily tunable" system
** settings, such as clock rate, number of simultaneous user
** processes, etc. This provides a sort of "one-stop shop" for
** things that might be tweaked frequently.
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

// Clock frequency (Hz)

#define	CLOCK_FREQUENCY	1000
#define	TICKS_PER_MS	1

#endif
